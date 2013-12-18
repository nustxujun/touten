// touten.cpp : 定义控制台应用程序的入口点。
//

#include "Touten.h"
#include "TTLexer.h"
#include "TTParser.h"
#include <fstream>
#include "TTStackBasedAssembler.h"


using namespace TT;

bool Touten::loadFile(const String& name)
{
	std::wfstream f(name.c_str(), std::ios_base::in);
	if (!f) return 0;

	f.seekg(0,std::ios_base::end);
	size_t size = f.tellg();
	f.seekg(0, std::ios_base::beg);
	std::vector<Char> b(size+1);
	f.read(b.data(), size + 1);
	
	Lexer lexer(b.data());

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


	StackBasedAssembler assembler( mScopemgr, mConstPool);

	assembler.assemble(ast);

	call(GLOBAL_INIT_FUNC);

	return true;
}

void Touten::registerFunction(const String& name, Functor* func)
{
	Symbol* sym = mScopemgr.getGlobal()->createSymbol(name, ST_CPP_FUNC, AT_GLOBAL);
	FunctionValue fv;
	fv.codeAddr = func;
	fv.funcinfo = FunctionValue::IS_CPP_FUNC;
	sym->addrOffset =  mConstPool << fv;
}

void Touten::call(const String& name, size_t parasCount, const ObjectPtr* paras, Object* ret)
{
	Scope::SymbolObj sym = mScopemgr.getGlobal()->getSymbol(name);
	if (sym.sym == 0 || sym.sym->symtype != ST_FUNCTION) return ;

	TT::FunctionValue* begin = (FunctionValue*)mConstPool[sym.sym->addrOffset];
	mInterpreter.execute(mConstPool, ((Codes*)begin->codeAddr)->data(), 
		std::min(begin->funcinfo & FunctionValue::PARA_COUNT , parasCount), paras, ret);

}

