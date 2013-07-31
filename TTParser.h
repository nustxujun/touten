#ifndef _TTParser_H_
#define _TTParser_H_

#include "TTLexer.h"
#include "TTASTree.h"

namespace TT
{


	class ParserInput
	{
	public:
		virtual Token next() = 0;
		virtual Token lookahead() = 0;
	};

	class ParserError
	{};

	class Parser
	{
		ASTNode::Ptr parse(ParserInput* input);

	private:
		ASTNode::Ptr parseFile(ParserInput* input);
		ASTNode::Ptr parseDef(ParserInput* input, TokenType type = TT_ERROR);
		ASTNode::Ptr parseVarlist(ParserInput* input, TokenType type = TT_ERROR);
		ASTNode::Ptr parseField(ParserInput* input, TokenType type = TT_ERROR);
		ASTNode::Ptr parseFunction(ParserInput* input, TokenType type = TT_ERROR);
		ASTNode::Ptr parseExpr(ParserInput* input);
		ASTNode::Ptr parseBlock(ParserInput* input);
		ASTNode::Ptr parseStat(ParserInput* input);
		ASTNode::Ptr parseLoop(ParserInput* input);
		ASTNode::Ptr parseCond(ParserInput* input);
		ASTNode::Ptr parseVar(ParserInput* input);
		ASTNode::Ptr parseAssgin(ParserInput* input, bool muli);
		ASTNode::Ptr parseUnop(ParserInput* input);

		bool checkOperator(const Token& t, Char c);
		void copyString(Char* d, const Char* s, size_t count);
	};
}

#endif 