#ifndef _TTParser_H_
#define _TTParser_H_

#include "TTLexer.h"
#include "TTASTNode.h"
#include "TTType.h"
#include <stack>

namespace TT
{
	/*
		因为用的是LL(2)所以　input的lookahead至少实现到2
	*/
	class ParserInput
	{
	public:
		virtual Token next() = 0;
		virtual Token lookahead(size_t index = 0) = 0;
	};

	class ParserError
	{};

	class Parser
	{
	public:
		ASTNode::Ptr parse(ParserInput* input);

	private:
		ASTNode::Ptr parseFile(ParserInput* input);
		ASTNode::Ptr parseDef(ParserInput* input, AccessType at);
		ASTNode::Ptr parseVarlist(ParserInput* input, AccessType at );
		ASTNode::Ptr parseField(ParserInput* input, AccessType at );
		ASTNode::Ptr parseFunction(ParserInput* input, AccessType at );
		ASTNode::Ptr parseBlock(ParserInput* input);
		ASTNode::Ptr parseStat(ParserInput* input);
		ASTNode::Ptr parseLoop(ParserInput* input);
		ASTNode::Ptr parseCond(ParserInput* input);
		ASTNode::Ptr parseVar(ParserInput* input);
		ASTNode::Ptr parseAssgin(ParserInput* input, ASTNode::Ptr pre = 0);
		ASTNode::Ptr parseUnop(ParserInput* input);
		ASTNode::Ptr parseFuncCall(ParserInput* input, bool needret, ASTNode::Ptr pre = 0);
		ASTNode::Ptr parseAccessSymbol(ParserInput* input);
		ASTNode::Ptr parseConst(ParserInput* input);
		struct Binop
		{
			OperatorType type;
			ASTNode::Ptr left;
		};

		typedef std::stack<Binop> BinopStack;
		ASTNode::Ptr parseExpr(ParserInput* input, BinopStack* stack = 0);



		//只检测一个字符的操作符
		bool checkOperator(const Token& t,  Char c);
		bool checkDelimiter(const Token& t, Char c);
		void copyString(Char* d, const Char* s, size_t count);
	};
}

#endif 