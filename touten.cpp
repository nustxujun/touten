// touten.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "TTLexer.h"
#include <fstream>
int _tmain(int argc, _TCHAR* argv[])
{
	std::wfstream f(L"test.txt", std::ios_base::in);
	if (!f) return 0;

	f.seekg(0,std::ios_base::end);
	size_t size = f.tellg();
	f.seekg(0, std::ios_base::beg);
	TT::Char* b = new TT::Char[size+1]();
	f.read(b, size + 1);
	TT::Lexer l(b);

	//try
	//{
	while(l.next().type != TT::TT_EOS);
	//}
	//catch(...)
	//{};
	return 0;
}

