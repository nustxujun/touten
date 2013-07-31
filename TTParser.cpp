#include "TTParser.h"
#include <memory.h>

using namespace TT;

#define TTPARSER_EXCEPT(t) { ParserError error; throw error;}

ASTNode::Ptr Parser::parseFile(ParserInput* input)
{
	FileNode* fn = new FileNode();
	ASTNode::Ptr f = fn;
	
	ASTNodeList::Ptr last = new ASTNodeList();
	fn->defs = last;

	do
	{
		TokenType tt = TT_LOCAL;
		switch (input->lookahead().type)
		{
		case TT_GLOBAL:
		case TT_LOCAL:
			tt = input->next().type;
			break;
		}
		last = last->setAndNext(parseDef(input, tt));
	}
	while (!last.isNull());

	return f;
}

ASTNode::Ptr Parser::parseDef(ParserInput* input, TokenType type)
{


	switch (input->lookahead().type)
	{
	case TT_FUNCTION:
		return parseFunction(input, type);
		break;
	case TT_FIELD:
		return parseField(input, type);
		break;
	case TT_NAME:
		return parseVarlist(input, type);
		break;
	default:
		TTPARSER_EXCEPT(L"syntax error");
	}
	return 0;
}

ASTNode::Ptr Parser::parseVarlist(ParserInput* input, TokenType type)
{
	Token t;
	VarListNode* vln = new VarListNode();
	ASTNode::Ptr vl = vln;
	ASTNodeList::Ptr last = new ASTNodeList();
	vln->vars = last;

	while (true)
	{
		t = input->next();

		if (t.type != TT_NAME)
		{
			TTPARSER_EXCEPT("unexpected token");
			return vl;
		}

		VarNode* vn = new VarNode();
		vn->type = type;
		copyString(vn->name, t.string,  t.size);
		ASTNode::Ptr var = vn;

		if (input->lookahead().type == TT_ASSGIN)
		{
			input->next();
			AssginNode* an = new AssginNode();
			an->left = var;

			an->right = parseExpr(input);

			if (an->right.isNull()) return vl;

			last = last->setAndNext(an);
		}
		else {last = last->setAndNext(vn);}

		if (!checkOperator(t, ',')) break;
	}
	

	return vl;
}

ASTNode::Ptr Parser::parseFunction(ParserInput* input, TokenType type)
{
	Token t = input->next();
	FunctionNode* fn = new FunctionNode();
	ASTNode::Ptr f = fn;
	fn->name[0] = 0;//anonymous
	
	if (t.type == TT_FUNCTION)
	{
		t = input->next();
		if (t.type != TT_NAME)
		{
			TTPARSER_EXCEPT("function need name");
			return 0;
		}
		copyString(fn->name, t.string, t.size);
		t = input->next();
	}

	if (!checkOperator(t, '('))
	{
		TTPARSER_EXCEPT("need (");
		return 0;
	}

	ASTNodeList::Ptr last = new ASTNodeList();
	fn->paras = last;

	if ( !checkOperator(input->lookahead(), ')'))
	{
		while (true)
		{
			t = input->next();
			if (t.type != TT_NAME)
			{
				TTPARSER_EXCEPT("unexpected token");
				return 0;
			}

			VarNode* vn = new VarNode();
			vn->type = TT_LOCAL;
			copyString(vn->name, t.string,  t.size);
			last = last->setAndNext(vn);

			t = input->next();
			if (!checkOperator(t, ','))
			{
				if (checkOperator(t, ')')) break;
				
				TTPARSER_EXCEPT("unexpected token， need , or )");
				return 0;
			}
		}
	}

	if (!checkOperator(input->next(), '{' ))
	{
		TTPARSER_EXCEPT("need function body");
		return 0;
	}

	fn->body = parseBlock(input);

	if (fn->body.isNull())
		return 0;

	if (!checkOperator(t, '}'))
	{
		TTPARSER_EXCEPT("need }");
		return 0;
	}
	return f;
}

ASTNode::Ptr Parser::parseField(ParserInput* input, TokenType type)
{
	Token t = input->next();
	FieldNode* fn = new FieldNode();
	ASTNode::Ptr f = fn;
	*fn->name = 0;//anonymous
	
	if (t.type == TT_FIELD)
	{
		t = input->next();
		if (t.type != TT_NAME)
		{
			TTPARSER_EXCEPT("field need name");
			return 0;
		}
		copyString(fn->name, t.string, t.size);
		t = input->next();
	}

	if (!checkOperator(t, '{' ))
	{
		TTPARSER_EXCEPT("need function body");
		return 0;
	}

	fn->body = parseBlock(input);

	if (fn->body.isNull())
		return 0;

	if (!checkOperator(t, '}'))
	{
		TTPARSER_EXCEPT("need }");
		return 0;
	}
	return f;

}

ASTNode::Ptr Parser::parseBlock(ParserInput* input)
{
	BlockNode* bn = new BlockNode();
	ASTNode::Ptr b = bn;
	
	ASTNodeList::Ptr last = new ASTNodeList();
	bn->stats = last;

	do
	{
		last = last->setAndNext(parseStat(input));

		if(checkOperator(input->lookahead(), ';'))
			input->next();
	}
	while (!last.isNull());
	
	return b;
}

ASTNode::Ptr Parser::parseStat(ParserInput* input)
{
	Token t = input->lookahead();
	switch (t.type)
	{
	case TT_WHILE:
	case TT_DO:
	case TT_FOR:
		return parseLoop(input);
		break;
	case TT_IF:
	case TT_SWITCH:
		return parseCond(input);
		break;
	case TT_SHARED: case TT_LOCAL: //definition
		return parseDef(input, input->next().type);
		break;
	case TT_NAME://assgin
		return parseAssgin(input, false);
		break;
	case TT_OPERATOR://field
		{		
			if (checkOperator(t, '{'))
				return parseField(input, TT_LOCAL);
			if (checkOperator(t, '['))
				return parseAssgin(input, true);
		}
		break;
	}

	TTPARSER_EXCEPT("unexpect token");
	return 0;
}

ASTNode::Ptr Parser::parseVar(ParserInput* input)
{
	Token t = input->next();
	VarNode* vn = new VarNode();
	ASTNode::Ptr v = vn;
	copyString(vn->name, t.string, t.size);

	t = input->next();
	if (checkOperator(t, '['))
	{
		vn->index = parseExpr(input);
		if (!checkOperator(t,']'))
		{
			TTPARSER_EXCEPT("need ]");
			return 0;
		}
	}

	if (checkOperator(input->next(), '.'))
	{
		if (input->lookahead().type != TT_NAME)
		{
			TTPARSER_EXCEPT("unexpect token");
			return 0;
		}
		vn->member = parseVar(input);
		if (vn->member.isNull())
			return 0;
	}

	return v;
}

ASTNode::Ptr Parser::parseExpr(ParserInput* input)
{
	Token look = input->lookahead();
	ASTNode::Ptr expr;
	switch (look.type)
	{
	//constant
	case TT_NULL:
	case TT_TRUE:
	case TT_FALSE:
	case TT_INTEGER:
	case TT_DOUBLE:
	case TT_STRING:
		{
			Token t = input->next();
			ConstNode* cn = new ConstNode();
			cn->type = t.type;

			if (t.type == TT_STRING)
				copyString(cn->value, t.val.s.b, t.val.s.s);
			else
				memcpy(cn->value, &t.val, sizeof(t.val));

			expr = cn;
		}
		break;
	case TT_NAME://var funcall
		{
			ASTNode::Ptr var = parseVar(input);
			if (checkOperator(input->next(), '('))
			{
				FuncCallNode* fn = new FuncCallNode();
				fn->var = var;
				expr = fn;
				
				if (checkOperator(input->lookahead(), ')')) break;
				
				ASTNodeList::Ptr last = new ASTNodeList();
				fn->paras = last;

				while (true)
				{					
					last = last->setAndNext(parseExpr(input));
					Token t = input->next();
					if (!checkOperator(t, ','))
					{
						if (checkOperator(t, ')')) break;

						TTPARSER_EXCEPT("unexpected token， need , or )");
						return 0;
					}
				}
			}
			else { expr = var;}
		}
		break;
	case TT_OPERATOR:
		{
			if (checkOperator(look, '('))
				expr = parseFunction(input, TT_LOCAL);
			else if (checkOperator(look, '{'))
				expr = parseField(input, TT_LOCAL);
			else 
				expr = parseUnop(input);
		}
		break;
	default:
		TTPARSER_EXCEPT("unexpected token");
		return 0;
	}


	//以下是推导二元运算
	Token op = input->lookahead();
	if ( op.type != TT_OPERATOR)
		return expr;

	switch ( *op.string )
	{
	case '=':
		if (op.size == 1) return expr;//需要 "=="
	case '&':case '*':case '-':
	case '+':case '|':case '<':
	case '>':case '/':case '%':
		{
			input->next();
			ASTNode::Ptr e2 = parseExpr(input);
			if (e2.isNull()) 
				return expr;	
			
			BinopNode* bn = new BinopNode();
			bn->exp1 = expr;
			bn->exp2 = e2;
			assert(op.size <= BinopNode::OP_MAX_LEN);
			copyString(bn->opt, op.string, op.size);
			bn->opt[op.size] = 0;
			return bn;
		}
		break;
	}
	return expr;
}

ASTNode::Ptr Parser::parseAssgin(ParserInput* input, bool muli = false)
{
	AssginNode* an = new AssginNode();
	ASTNode::Ptr a = an;
	ASTNodeList::Ptr last = new ASTNodeList();
	an->left = last;

	input->next();
	
	if (muli)
	{
		while (true)
		{
			Token t = input->next();
			last = last->setAndNext(parseVar(input));
			t = input->next();
			if (!checkOperator(t, ','))
			{
				if (checkOperator(t, ']')) break;
				TTPARSER_EXCEPT("unexpected token");
				return 0;
			}
		}
	}
	else
	{
		last->obj = parseVar(input);
		if (last->obj.isNull()) 
		{
			TTPARSER_EXCEPT("cant parse var");
			return 0;
		}
	}

	if (!checkOperator(input->next(), '='))
	{
		TTPARSER_EXCEPT("need =");
		return 0;
	}

	an->right = parseExpr(input);
	if (an->right.isNull())
	{
		TTPARSER_EXCEPT("parse expr fail");
		return 0;
	}

	return a;
}


bool Parser::checkOperator(const Token& t, Char c)
{
	return t.type == TT_OPERATOR && *t.string == c;
}

void Parser::copyString(Char* d, const Char* s, size_t count)
{
	memcpy(d, s, count * sizeof(Char));
}