// touten.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Touten.h"
#include <fstream>
#include <vector>
#include "TTObject.h"
#include "TTMemoryAllocator.h"


size_t memsize = 0;

void* alloc(void* optr, size_t nsize)
{
	if (nsize == 0)
	{
		int* s = (int*)optr - 1;
		memsize -= *s;
		printf("total:%10d, current:- %d\n", memsize, *s);

		::free(s);
	}
	else
	{
		int* s = 0;
		if (optr)
		{
			s = (int*)optr - 1;
			memsize -= *s;
			printf("total:%10d, current:- %d\n", memsize, *s);
		}
		s = (int*)::realloc(s, nsize + 4);
		*s = nsize;
		optr = s + 1;
		memsize += nsize;

		printf("total:%10d, current:+ %d\n", memsize, *s);
	}
	return optr ;
}

int print(int num, TT::Object* obj, TT::Object* ret)
{
	TT::Caster caster;
	for (int i = 0; i < num; ++i)
		wprintf(L"%s ", caster.castToString(*(obj + i)));
	return 1;
}

int _tmain(int argc, _TCHAR* argv[])
{
	TT::MemoryAllocator::setupMethod(alloc);

	{
		TT::Touten t;
		t.registerFunction(L"print", print);
	
		t.loadFile(L"test.txt");
	}

	assert(memsize == 0);
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
	
	StaticArea sa;
	Lexer lexer(b, sa);

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


	Codes codes;
	
	ConstantPool constpool(1024);

	StackBasedAssembler assembler(codes, mScopemgr, constpool);

	assembler.assemble(ast);

	StackBasedInterpreter interpreter(codes, constpool, mFuncTable);
	
	if (assembler.hasMain())
		interpreter.execute(assembler.getMain());


	return true;
}

void Touten::registerFunction(const String& name, TT_Function func)
{
	Symbol* sym = mScopemgr.getGlobal()->createSymbol(name, ST_CPP_FUNC, AT_GLOBAL);
	sym->addrOffset = mFuncTable.insert(name.c_str(), func);
	mFuncTable[sym->addrOffset]->val.func.codeAddr = (size_t)func;
}

