// touten.cpp : 定义控制台应用程序的入口点。
//

#include "Touten.h"
#include "TTLexer.h"
#include "TTParser.h"
#include <fstream>
#include "TTStackBasedAssembler.h"
#include "TTException.h"
#include "TTScope.h"
#include "TTStackBasedInterpreter.h"
#include "TTFunctor.h"
#include <windows.h>

using namespace TT;

Touten::Touten()
{
	mScopemgr = new ScopeManager();
	mInterpreter = new StackBasedInterpreter();
	mConstPool = new ConstantPool();

	initInternalFunction();
}

Touten::~Touten()
{
	delete mConstPool;
	delete mInterpreter;
	delete mScopemgr;
}


bool Touten::loadFile(const String& name)
{
	std::ifstream f(name, std::ios_base::in);
	if (!f) return 0;

	f.seekg(0,std::ios_base::end);
	size_t size = f.tellg();
	f.seekg(0, std::ios_base::beg);
	std::vector<char> b(size + 1);
	std::vector<Char> wb(size + 1);
	f.read(b.data(), size);
	b[size] = 0;

	DWORD sizeWChar = MultiByteToWideChar(CP_ACP, 0, b.data(), -1, NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, b.data(), -1, wb.data(), sizeWChar);

	Lexer lexer((Char*)wb.data());

	struct Input: public ParserInput 
	{
		Lexer* l;
		Token next()
		{
			Token t = lookahead(0);
			lookaheads.erase(lookaheads.begin());
			return t;
		}

		Token lookahead(size_t index)
		{
			size_t diff = (index < lookaheads.size())? 0: index + 1;
			for (size_t i = 0; i < diff; ++i)
			{
				lookaheads.push_back(l->next());
			}
			return lookaheads[index];
		}

		std::vector<Token> lookaheads;
	}i;

	i.l = &lexer;

	Parser parser;
	ASTNode::Ptr ast;
	ast = parser.parse(&i);


	StackBasedAssembler assembler( *mScopemgr, (*mConstPool));

	assembler.assemble(ast);

	call(GLOBAL_INIT_FUNC);

	return true;
}

void Touten::registerFunction(const String& name, Functor* func)
{
	Symbol* sym = mScopemgr->getGlobal()->createSymbol(name, ST_CPP_FUNC, AT_GLOBAL);
	FunctionValue fv;
	fv.codeAddr = func;
	fv.funcinfo = FunctionValue::IS_CPP_FUNC;
	sym->addrOffset =  (*mConstPool) << fv;
}

void Touten::registerOrRetrieveFunction(const String& name, Functor* func)
{
	auto global = mScopemgr->getGlobal();
	auto sym = global->getSymbol(name);
	if (sym.sym != nullptr)
	{
		if (sym.sym->symtype != ST_CPP_FUNC)
		{
			TT_EXCEPT(ET_UNKNOWN, EL_NORMAL, "invalid symbol", 0);
			return;
		}

		FunctionValue* fv = (FunctionValue*)(*mConstPool)[sym.sym->addrOffset];
		fv->codeAddr = func;
		return;
	}

	registerFunction(name, func);
}

bool Touten::isValidFunction(const String& name)
{
	Symbol* sym = mScopemgr->getGlobal()->getSymbol(name).sym;
	if (!sym) return false;

	if (sym->symtype != ST_FUNCTION &&
		sym->symtype != ST_CPP_FUNC)
		return false;
	return sym->actype == AT_GLOBAL;
}


bool Touten::call(const String& name, size_t parasCount, const ObjectPtr* paras, Object* ret)
{
	Scope::SymbolObj sym = mScopemgr->getGlobal()->getSymbol(name);
	if (sym.sym == 0 || sym.sym->symtype != ST_FUNCTION) return false;

	TT::FunctionValue* begin = (FunctionValue*)(*mConstPool)[sym.sym->addrOffset];

	mInterpreter->execute((*mConstPool), ((Codes*)begin->codeAddr)->data(), 
		begin->funcinfo, parasCount, paras, ret);
	return true;
}

void Touten::initInternalFunction()
{
	static struct : public Functor
	{
		ScopeManager* sm;
		ConstantPool* cp;
		void operator()(const ObjectPtr* paras, int paracount, Object* ret)
		{
			if (paracount == 0 || (*paras)->val->type != OT_STRING) return;
			String name = Caster::cast<String>(**paras);
			Scope::SymbolObj sym = sm->getGlobal()->getSymbol(name);
			if (!sym.sym || !(sym.sym->symtype == ST_FUNCTION || sym.sym->symtype == ST_CPP_FUNC)) return;

			*ret = *(FunctionValue*)((*cp)[sym.sym->addrOffset]);

		}
	} getglobalfunciton;
	getglobalfunciton.sm = mScopemgr;
	getglobalfunciton.cp = mConstPool;
	registerFunction(L"__GetGlobalFunction", &getglobalfunciton);


}

