// touten.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Touten.h"
#include "TTBind.h"
#include <fstream>
#include <vector>
#include "TTObject.h"
#include "TTMemoryAllocator.h"
#include "TTCaster.h"
#include "TTLexer.h"
#include "TTParser.h"
#include "TTStackBasedAssembler.h"
#include "TTException.h"
#include <set>


//#define MEM_DEBUG

size_t memsize = 0;

struct REC
{
	int create;
	int release;
	std::set<void*> addr;
	REC() :create(0), release(0)
	{}
};
std::map<int ,REC > memrecord;

void* alloc(void* optr, size_t nsize)
{
#ifndef _DEBUG
	return ::realloc(optr, nsize);
#else
	if (nsize == 0)
	{
		int* s = (int*)optr - 1;
		memsize -= *s;
#ifdef MEM_DEBUG
		memrecord[*s].release += 1;
		memrecord[*s].addr.erase(optr);
		printf("total:%10d, current:- %d\n", memsize, *s);
#endif
		::free(s);
	}
	else
	{
		int* s = 0;
		if (optr)
		{
			s = (int*)optr - 1;
			memsize -= *s;
#ifdef MEM_DEBUG
			memrecord[*s].release += 1;
			memrecord[*s].addr.erase(optr);
			printf("total:%10d, current:- %d\n", memsize, *s);
#endif
		}
		s = (int*)::realloc(s, nsize + 4);
		*s = nsize;
		optr = s + 1;
		memsize += nsize;
#ifdef MEM_DEBUG
		memrecord[*s].create += 1;
		memrecord[*s].addr.insert(optr);
		printf("total:%10d, current:+ %d\n", memsize, *s);
#endif
	}
#endif
	return optr ;
}


void Print(TT::String str)
{
	wprintf(L"%s", str.c_str());
}

int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		TT::MemoryAllocator::setupMethod(alloc);
		TT::Touten t;
		TT::Bind b(&t);
		b.bind(L"print", &Print);
		//t.loadFile(L"test2.txt");
		t.loadFile(L"test.txt");
		//t.call(L"main");
		//int a = b.call<int>(L"f1", (int)3, (float)2, true);
		b.call<void>(L"main");
		getchar();
	}
	catch (const TT::Exception& e)
	{
		printf("\n[ line:%d ] %s", e.getLine(), e.getDesc().c_str());
	}
	assert(memsize == 0 && "leak detected");
	//_CrtDumpMemoryLeaks();
	return 0;
}



using namespace TT;

bool Touten::loadFile(const String& name)
{
	std::wfstream f(name.c_str(), std::ios_base::in);
	if (!f) return 0;

	f.seekg(0,std::ios_base::end);
	size_t size = f.tellg();
	f.seekg(0, std::ios_base::beg);
	TT::Char* b = new TT::Char[size+1]();
	f.read(b, size + 1);
	
	Lexer lexer(b);

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

	//mInterpreter.execute(mConstPool, mFuncTable, mCodes.data(false), 0);


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

