#include "TTParser.h"
#include "TTASTree.h"
#include "TTException.h"
#include <memory.h>

using namespace TT;

#define TTPARSER_EXCEPT(t, l) TT_EXCEPT(ET_PARSING_FAILED, EL_NORMAL, t, l)

ASTNode::Ptr Parser::parse(ParserInput* input)
{
	return parseFile(input);
}


ASTNode::Ptr Parser::parseFile(ParserInput* input)
{
	FileNode* fn = new FileNode();
	ASTNode::Ptr f = fn;
	
	ASTNodeList::Ptr last = new ASTNodeList();
	fn->defs = last;

	while(true)
	{
		Token t = input->lookahead();
		AccessType ac = AT_LOCAL;
		switch (t.type)
		{
		case TT_GLOBAL: ac = AT_GLOBAL; input->next(); break;
		case TT_LOCAL: ac = AT_LOCAL; input->next(); break;
		case TT_NAME: ac = AT_GLOBAL;break;//变量为全局
		}
		ASTNode::Ptr o = parseDef(input, ac);
		if (o.isNull()) return f;
		last = last->setAndNext(o);

		if (checkDelimiter(input->lookahead(), ';'))
			input->next();
	}

	return f;
}

ASTNode::Ptr Parser::parseDef(ParserInput* input, AccessType at)
{
	switch (input->lookahead().type)
	{
	case TT_FUNCTION:
		return parseFunction(input, at);
		break;
	case TT_NAME:
		return parseVarlist(input, at);
		break;
	case TT_EOS:
		return 0;
	default:
		TTPARSER_EXCEPT("syntax error ", input->lookahead().lineNum);
	}
	return 0;
}

ASTNode::Ptr Parser::parseVarlist(ParserInput* input, AccessType at)
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
			TTPARSER_EXCEPT("syntax error", t.lineNum);
			return vl;
		}

		NameNode* nn = new NameNode();
		nn->name = String(t.string, t.size);

		t = input->lookahead();
		if (t.type  == TT_ASSGIN)
		{
			VarNode* vn = new VarNode;
			vn->type = at;
			vn->var = nn;
	
			ASTNode::Ptr a = parseAssgin(input, vn);

			last = last->setAndNext(a);
			t = input->lookahead();
			
		}
		else {last = last->setAndNext(nn);}

		
		if (!checkDelimiter(t, ',')) break;
		input->next();
	}
	

	return vl;
}

ASTNode::Ptr Parser::parseFunction(ParserInput* input, AccessType at)
{
	Token t = input->next();
	FunctionNode* fn = new FunctionNode();
	ASTNode::Ptr f = fn;
	fn->acctype = at;
	fn->isVariadic = false;

	t = input->next();
	if (t.type == TT_NAME)
	{
		fn->name = String(t.string, t.size);
		t = input->next();
	}
	
	if (!checkDelimiter(t, '('))
	{
		TTPARSER_EXCEPT("need (", t.lineNum);
		return 0;
	}

	ASTNodeList::Ptr last = new ASTNodeList();
	fn->paras = last;

	if ( !checkDelimiter(input->lookahead(), ')'))
	{
		bool terminate = false;
		do
		{
			t = input->next();
			switch (t.type)
			{
			case TT_NAME:
				{
					NameNode* nn = new NameNode;
					nn->name = String(t.string, t.size);

					last = last->setAndNext(nn);

					t = input->lookahead();
					if (!checkDelimiter(t, ',')) terminate = true;
					t = input->next();
				}
				break;
			case TT_ELLIPSIS:
				{
					fn->isVariadic = true;
					terminate = true;
					t = input->next();

				}
				break;
			default:
				{
				   TTPARSER_EXCEPT("syntax error", t.lineNum);
				   return 0;
				}
				break;
			}
		} while (!terminate);

		if (!checkDelimiter(t, ')'))
		{
			TTPARSER_EXCEPT("need , or )", t.lineNum);
			return 0;
		}
		//while (true)
		//{
		//	t = input->next();
		//	if (t.type != TT_NAME)
		//	{
		//		TTPARSER_EXCEPT("syntax error", t.lineNum);
		//		return 0;
		//	}

		//	NameNode* nn = new NameNode;
		//	nn->name = String(t.string, t.size);

		//	last = last->setAndNext(nn);

		//	t = input->next();
		//	if (!checkDelimiter(t, ','))
		//	{
		//		if (checkDelimiter(t, ')')) break;
		//		
		//		TTPARSER_EXCEPT("need , or )", t.lineNum);
		//		return 0;
		//	}
		//}
	}
	else
		input->next();

	if (!checkDelimiter(input->next(), '{' ))
	{
		TTPARSER_EXCEPT("need function body", input->lookahead().lineNum);
		return 0;
	}

	fn->body = parseBlock(input);

	if (!checkDelimiter(input->lookahead(), '}'))
	{
		TTPARSER_EXCEPT("need }", input->lookahead().lineNum);
		return 0;
	}
	input->next();
	return f;
}


ASTNode::Ptr Parser::parseBlock(ParserInput* input)
{
	BlockNode* bn = new BlockNode();
	ASTNode::Ptr b = bn;
	
	ASTNodeList::Ptr last = new ASTNodeList();
	bn->stats = last;

	while(true)
	{
		ASTNode::Ptr obj = parseStat(input);
		if (obj.isNull()) return b;

		last = last->setAndNext(obj);

		if(checkDelimiter(input->lookahead(), ';'))
			input->next();
	}
	
	
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
	case TT_SHARED: 
		return parseDef(input, AT_SHARED ); break;
	case TT_LOCAL: //definition
		return parseDef(input, AT_LOCAL); break;
	case TT_PRE_GL: 
	case TT_PRE_SA:
	case TT_NAME://assgin funcall
		{
			ASTNode::Ptr var = parseVar(input);
			t = input->lookahead();
			if (t.type == TT_ASSGIN ||
				t.type == TT_REF)
				return parseAssgin(input,var);
			else
				return parseFuncCall(input,false, var);

		}
		break;
	case TT_DELIMITER:
		{		
			//if (checkDelimiter(t, '{'))
			//	return parseField(input, AT_LOCAL);
			if (checkDelimiter(t, '['))
				return parseAssgin(input);
			else if ( checkDelimiter(t,'}'))
				return 0;

		}
		break;
	case TT_RETURN://return
		{
			input->next();
			ASTNodeList::Ptr last = new ASTNodeList();
			ASTNodeList::Ptr head = last;
			while (true)
			{
				ASTNode::Ptr expr = parseExpr(input);
				if (expr.isNull()) break;
				last = last->setAndNext(expr);
				if (checkDelimiter(input->lookahead(), ','))
					input->next();
				else
					break;
			}

			ReturnNode* rn = new ReturnNode;
			rn->exprs = head;
			return rn;
		}
		break;
	}

	TTPARSER_EXCEPT("syntax error", input->lookahead().lineNum);
	return 0;
}

ASTNode::Ptr Parser::parseVar(ParserInput* input, AccessType deftype)
{
	Token name = input->next();
	AccessType at = deftype;
	switch (name.type)
	{
	case TT_PRE_GL: at = AT_GLOBAL; name = input->next(); break;
	case TT_PRE_SA: at = AT_SHARED; name = input->next(); break;
	}
	NameNode* nn = new NameNode;
	nn->name = String(name.string, name.size);

	VarNode* vn = new VarNode;
	vn->type = at;
	vn->var = nn;
	ASTNodeList::Ptr last = (vn->indexs = new ASTNodeList);
	ASTNode::Ptr v = vn;

	while (true)
	{
		Token t = input->lookahead();
		if (checkDelimiter(t, '['))
		{
			input->next();
			ASTNode::Ptr index = parseExpr(input);
			if (index.isNull())
			{
				TTPARSER_EXCEPT("need index", input->lookahead().lineNum);
				return 0;
			}

			last = last->setAndNext(index);
			
			if (!checkDelimiter(input->next(),']'))
			{
				TTPARSER_EXCEPT("need ]", input->lookahead().lineNum);
				return 0;
			}		
		}
		else if (checkOperator(t, '.'))
		{
			input->next();
			t = input->next();
			if (t.type != TT_NAME)
			{
				TTPARSER_EXCEPT("syntax error", t.lineNum);
				return 0;
			}
			ConstNode* cn = new ConstNode;
			cn->type = CT_STRING;
			cn->value.s = t.string;
			cn->value.c = t.size;
			//copyString(cn->value.s, t.string, t.size);
			last = last->setAndNext(cn);
		}
		else
		{
			break;
		}
	}

	return v;
}

ASTNode::Ptr Parser::parseExpr(ParserInput* input,  BinopStack* stack)
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
			expr = parseConst(input);
		}
		break;
	case TT_PRE_GL:
	case TT_PRE_SA:
	case TT_NAME://var funcall 
		{
			expr = parseAccessSymbol(input);
		}
		break;
	case TT_FUNCTION://函数内暂时只能定义匿名函数
		expr = parseFunction(input, AT_LOCAL);
		break;
	case TT_DELIMITER:
		{
			if (checkDelimiter(look, '('))
			{
				input->next();
				expr = parseExpr(input);
				if (!checkDelimiter(input->next(), ')'))
				{
					TTPARSER_EXCEPT("need )", input->lookahead().lineNum);
					return 0;
				}
			}
			//else if (checkDelimiter(look, '{'))
			//	expr = parseField(input, AT_LOCAL);
			
			TTPARSER_EXCEPT("syntax error", input->lookahead().lineNum);
			return 0;
		}
		break;
	case TT_OPERATOR:
		expr = parseUnop(input);
		break;
	default:
		TTPARSER_EXCEPT("syntax error",look.lineNum);
		return 0;
	}

	Binop binop;
	binop.type = OT_VOID;
	binop.left = expr;


	//以下是推导二元运算
	Token op = input->lookahead();
	if ( op.type != TT_OPERATOR)
	{
		goto MAKEOP;
	}

	switch ( *op.string )
	{
	case '=':
		if (op.size == 1) goto MAKEOP;
		binop.type = OT_EQ;
		break;
	case '!':if (op.size == 1) goto MAKEOP;
		binop.type = OT_NE;
		break;
	case '&':binop.type = (op.size == 1)? OT_AND: OT_DAND;break;
	case '*':binop.type = OT_MUL;break;
	case '-':binop.type = OT_SUB;break;
	case '+':binop.type = OT_ADD;break;
	case '|':binop.type = (op.size == 1)? OT_OR: OT_DOR;break;
	case '<':binop.type = (op.size == 1)? OT_LESS: OT_LE;break;
	case '>':binop.type = (op.size == 1)? OT_GREAT: OT_GE;break;
	case '/':binop.type = OT_DIV;break;
	case '%':binop.type = OT_MOD;break;
	case '^':binop.type = OT_XOR;break;
	default :
		goto MAKEOP;
	}
	input->next();

MAKEOP:
	if (stack != 0)
	{
		while (!stack->empty())
		{
			Binop& top = stack->top();
			if (OptPre[binop.type] > OptPre[top.type]) break;

			OperatorNode* on = new OperatorNode;
			on->exp1 = top.left;
			on->exp2 = binop.left;
			on->opt = top.type;

			binop.left = on;
			stack->pop();
		}

		if (binop.type == OT_VOID)
			return binop.left;

		stack->push(binop);
		return parseExpr(input, stack);
	}
	else
	{
		if (binop.type == OT_VOID)
			return binop.left;

		BinopStack s;
		s.push(binop);
		return parseExpr(input, &s);
	}
}


ASTNode::Ptr Parser::parseAssgin(ParserInput* input, ASTNode::Ptr pre )
{
	AssginNode* an = new AssginNode();
	ASTNode::Ptr a = an;
	ASTNodeList::Ptr last = new ASTNodeList();
	an->left = last;

	if (!pre.isNull())
	{
		last->obj = pre;
	}
	else
	{
		Token t = input->lookahead();
		if (checkDelimiter(t, '[') )
		{
			input->next();
			while (true)
			{
				ASTNode::Ptr o = parseVar(input);
				if (o.isNull()) break;
				last = last->setAndNext(o);
				t = input->next();
				if (!checkDelimiter(t, ','))
				{
					if (checkDelimiter(t, ']')) break;
					TTPARSER_EXCEPT("need var between , and ]", input->lookahead().lineNum);
					return 0;
				}
			}
		}
		else
		{
			last->obj = parseVar(input);
			if (last->obj.isNull()) 
			{
				TTPARSER_EXCEPT("parse var failed, need var", input->lookahead().lineNum);
				return 0;
			}
		}
	}

	Token t = input->next();
	if (t.type == TT_ASSGIN)
		an->isRef = false;
	else if (t.type == TT_REF)
		an->isRef = true;
	else
	{
		TTPARSER_EXCEPT("need = or &=",t.lineNum);
		return 0;
	}

	an->right = parseExpr(input);
	if (an->right.isNull())
	{
		TTPARSER_EXCEPT("parse expr fail, need a expr", input->lookahead().lineNum);
		return 0;
	}

	return a;
}

ASTNode::Ptr Parser::parseUnop(ParserInput* input)
{
	Token op = input->next();

	OperatorType type;
	switch ( *op.string )
	{
	case '!':
		if (op.size == 2) return 0;//排除 "!=" 
		type = OT_EXC;
		break;
	case '-':
		type = OT_NS;
		break;
	default:
		TTPARSER_EXCEPT("syntax error", input->lookahead().lineNum);
		return 0;
	}
	ASTNode::Ptr expr = parseExpr(input);
	if (expr.isNull()) 
		return 0;	

	OperatorNode* bn = new OperatorNode();
	bn->exp1 = expr;
	bn->opt = type;
	return bn;
}

ASTNode::Ptr Parser::parseLoop(ParserInput* input)
{
	Token t = input->next();
	
	switch (t.type)
	{
	case TT_WHILE:
		{
			if (!checkDelimiter(input->next(), '('))
			{
				TTPARSER_EXCEPT("need (", input->lookahead().lineNum);
				return 0;
			}
			
			ASTNode::Ptr expr = parseExpr(input);
			if (expr.isNull())
			{
				TTPARSER_EXCEPT("fail to parse expr, need expr", input->lookahead().lineNum);
				return 0;
			}

			if (!checkDelimiter(input->next(), ')'))
			{
				TTPARSER_EXCEPT("need )", input->lookahead().lineNum);
				return 0;
			}

			ASTNode::Ptr block;
			if (checkDelimiter(input->next(), '{'))
			{
				block = parseBlock(input);

				if (!checkDelimiter(input->next(), '}'))
				{
					TTPARSER_EXCEPT("need }", input->lookahead().lineNum);
					return 0;
				}
			}
			else
			{
				block = parseExpr(input);
			}

			LoopNode* ln = new LoopNode();
			ln->expr = expr;
			ln->block = block;
			return ln;

		}
		break;
	case TT_FOR:
	case TT_DO:
		TTPARSER_EXCEPT("for and do isnot supported",t.lineNum);
		break;

	}
	return 0;
}

ASTNode::Ptr Parser::parseCond(ParserInput* input)
{

	CondIFNode* cin = new CondIFNode();
	ASTNode::Ptr ci = cin;
	Token t = input->lookahead();
	if (t.type != TT_IF && t.type != TT_SWITCH)
	{
		TTPARSER_EXCEPT("need if or switch", input->lookahead().lineNum);
		return 0;
	}

	while (true)
	{
		CondIFNode::Branch	bnc;
		Token t = input->next();
		Token head = t;

		if (t.type == TT_IF || t.type == TT_ELSEIF)
		{
			t = input->next();

			if (!checkDelimiter(t, '('))
			{
				TTPARSER_EXCEPT("need (", t.lineNum);
				return 0;
			}

			bnc.expr = parseExpr(input);
			if (bnc.expr.isNull())
			{
				TTPARSER_EXCEPT("parse expr failed, need expr", input->lookahead().lineNum);
				return 0;
			}

			t = input->next();
			if (!checkDelimiter(t, ')'))
			{
				TTPARSER_EXCEPT("need )", t.lineNum);
				return 0;
			}
		}
		else if (t.type == TT_ELSE);//skip
		else break;

		


		if (checkDelimiter(input->lookahead(), '{'))
		{
			input->next();
			bnc.block = parseBlock(input);
			t = input->next();
			if (!checkDelimiter(t, '}'))
			{
				TTPARSER_EXCEPT("need }", t.lineNum);
				return 0;
			}
		}
		else { bnc.block = parseExpr(input); }

		cin->branchs.push_back(bnc);

		t = input->lookahead();
		if (t.type != TT_ELSE && t.type != TT_ELSEIF)
			break;

		if (head.type == TT_ELSE)
			break;

	}

	if (cin->branchs.size() != 0)
		return ci;
	else
		return 0;


}

ASTNode::Ptr Parser::parseFuncCall(ParserInput* input, bool needret,  ASTNode::Ptr pre)
{
	ASTNode::Ptr var = pre.isNull() ? parseVar(input) : pre;

	FuncCallNode* fn = new FuncCallNode();
	ASTNode::Ptr f = fn;
	fn->var = var;
	fn->needrets = needret;
	fn->hasVariadic = false;

	if (checkDelimiter(input->lookahead(), ':'))
	{
		input->next();
		fn->func = parseVar(input);
	}

	if (!checkDelimiter(input->next(), '('))
	{
		TTPARSER_EXCEPT(" need  (", input->lookahead().lineNum);
		return 0;
	}

	if (!checkDelimiter(input->lookahead(), ')'))
	{

		ASTNodeList::Ptr last = new ASTNodeList();
		fn->paras = last;
		Token t;
		bool terminate = false;
		do
		{					
			t = input->lookahead();
			switch (t.type)
			{
			case TT_ELLIPSIS:
				{
					input->next();
					fn->hasVariadic = true;
					
					terminate = true;
					t = input->next();
				}
				break;
			default:
				{
					ASTNode::Ptr o = parseExpr(input);
					if (!o.isNull())
						last = last->setAndNext(o);

					t = input->lookahead();
					if (!checkDelimiter(t, ','))
					{
						terminate = true;
					}
					t = input->next();

				}
				break;
			}

		} while (!terminate);

		if (!checkDelimiter(t, ')'))
		{
			TTPARSER_EXCEPT(" need , or )", input->lookahead().lineNum);
			return 0;
		}
	}
	else
		input->next();

	return f;
}

ASTNode::Ptr Parser::parseAccessSymbol(ParserInput* input)
{
	ASTNode::Ptr var = parseVar(input);
	if (checkDelimiter(input->lookahead(), '('))
	{
		return parseFuncCall(input,true, var);
	}
	else
		return var;
}

ASTNode::Ptr Parser::parseConst(ParserInput* input)
{
	Token t = input->lookahead();
	ConstNode* cn = new ConstNode;
	ASTNode::Ptr c = cn;
	switch (t.type)
	{
	case TT_NULL: cn->type = CT_NULL; break;
	case TT_TRUE: cn->type = CT_TRUE; break;
	case TT_FALSE: cn->type = CT_FALSE; break;
	case TT_INTEGER: cn->type = CT_INTEGER; break;
	case TT_DOUBLE: cn->type = CT_DOUBLE; break;
	case TT_STRING:	cn->type = CT_STRING; break;
	default:
		TTPARSER_EXCEPT("need a constant",t.lineNum);
		return 0;
	}

	input->next();
	if (t.type == TT_STRING)
		cn->value.s = t.string, cn->value.c = t.size;

		//copyString(cn->value.s, t.val.s.b, t.val.s.s);
	else
		memcpy(&cn->value, &t.val, sizeof(t.val));	

	return c;
}


bool Parser::checkOperator(const Token& t, Char c)
{
	return t.type == TT_OPERATOR && c == *t.string && t.size == 1;
}

bool Parser::checkDelimiter(const Token& t, Char c)
{
	return t.type == TT_DELIMITER && c == *t.string && t.size == 1;
}

void Parser::copyString(Char* d, const Char* s, size_t count)
{
	memcpy(d, s, count * sizeof(Char));
	d[count] = 0;
}