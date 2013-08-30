// touten.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Touten.h"
#include <fstream>
#include <vector>
int _tmain(int argc, _TCHAR* argv[])
{
	TT::Touten t;
	t.loadFile(L"test.txt");
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
	ScopeManager scopemgr;
	ConstantPool constpool(1024);

	StackBasedAssembler assembler(codes, scopemgr, constpool);

	assembler.assemble(ast);

	StackBasedInterpreter interpreter(codes, constpool);
	
	if (assembler.hasMain())
		interpreter.execute(assembler.getMain());
	return true;
}

