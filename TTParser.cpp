#include "TTParser.h"
#include "TTASTree.h"
#include <memory.h>

using namespace TT;

#define TTPARSER_EXCEPT(t) { ParserError error; throw error;}

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
		TokenType tt = TT_LOCAL;
		switch ( input->lookahead().type)
		{
		case TT_GLOBAL:
		case TT_LOCAL:
			tt = input->next().type;
			break;
		}
		ASTNode::Ptr o = parseDef(input, tt);
		if (o.isNull()) return f;
		last = last->setAndNext(o);

		if (checkOperator(input->lookahead(), ';'))
			input->next();
	}

	return f;
}

ASTNode::Ptr Parser::parseDef(ParserInput* input, TokenType keyword)
{
	switch (input->lookahead().type)
	{
	case TT_FUNCTION:
		return parseFunction(input, keyword);
		break;
	case TT_FIELD:
		return parseField(input, keyword);
		break;
	case TT_NAME:
		return parseVarlist(input, keyword);
		break;
	case TT_EOS:
		return 0;
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

		NameNode* nn = new NameNode();
		switch (type)
		{
		case TT_GLOBAL: nn->type = AT_GLOBAL; break;
		case TT_SHARED: nn->type = AT_SHARED; break;
		case TT_LOCAL: nn->type = AT_LOCAL; break;
		}
		copyString(nn->name, t.string,  t.size);
		nn->forcedefine = true;

		t = input->lookahead();
		if (t.type  == TT_ASSGIN)
		{
			input->next();

			VarNode* vn = new VarNode;
			vn->var = nn;

			ASTNode::Ptr a = parseAssgin(input, vn);
			
			last = last->setAndNext(a);
			t = input->lookahead();
		}
		else {last = last->setAndNext(nn);}

		
		if (!checkOperator(t, ',')) break;
		input->next();
	}
	

	return vl;
}

ASTNode::Ptr Parser::parseFunction(ParserInput* input, TokenType type)
{
	Token t = input->next();
	FunctionNode* fn = new FunctionNode();
	ASTNode::Ptr f = fn;
	fn->name[0] = 0;//anonymous
	switch (type)
	{
	case TT_GLOBAL:
		fn->acctype = AT_GLOBAL;
		break;
	case TT_LOCAL:
		fn->acctype = AT_LOCAL;
	default:
		fn->acctype = AT_DEFAULT;
	}
	
	t = input->next();
	if (t.type == TT_NAME)
	{
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

			NameNode* nn = new NameNode;
			nn->type = AT_LOCAL;
			copyString(nn->name, t.string,  t.size);
			nn->forcedefine = true;

			last = last->setAndNext(nn);

			t = input->next();
			if (!checkOperator(t, ','))
			{
				if (checkOperator(t, ')')) break;
				
				TTPARSER_EXCEPT("unexpected token�� need , or )");
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

	if (!checkOperator(input->next(), '}'))
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
	switch (type)
	{
	case TT_GLOBAL:
		fn->acctype = AT_GLOBAL;
		break;
	case TT_LOCAL:
		fn->acctype = AT_LOCAL;
	default:
		fn->acctype = AT_DEFAULT;
	}
	
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

	while(true)
	{
		ASTNode::Ptr obj = parseStat(input);
		if (obj.isNull()) return b;

		last = last->setAndNext(obj);

		if(checkOperator(input->lookahead(), ';'))
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
	case TT_SHARED: case TT_LOCAL: //definition
		return parseDef(input, input->next().type );
		break;
	case TT_NAME://assgin funcall
		{
			ASTNode::Ptr var = parseVar(input);
			if (input->lookahead().type == TT_ASSGIN)
				return parseAssgin(input,var);
			else
				return parseFuncCall(input, var);

		}
		break;
	case TT_OPERATOR://field
		{		
			if (checkOperator(t, '{'))
				return parseField(input, TT_LOCAL);
			if (checkOperator(t, '['))
				return parseAssgin(input);
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
			}

			ReturnNode* rn = new ReturnNode;
			rn->exprs = head;
			return rn;
		}
		break;
	}

	return 0;
}

ASTNode::Ptr Parser::parseVar(ParserInput* input)
{
	Token name = input->next();
	NameNode* nn = new NameNode;
	copyString(nn->name,name.string,name.size);
	nn->type = AT_LOCAL;
	nn->forcedefine = false;

	VarNode* vn = new VarNode;
	vn->var = nn;
	ASTNode::Ptr v = vn;

	while (true)
	{
		Token t = input->lookahead();
		if (checkOperator(t, '['))
		{
			input->next();
			vn->index = parseExpr(input);
			if (vn->index.isNull())
			{
				TTPARSER_EXCEPT("need index");
				return 0;
			}
			
			if (!checkOperator(input->next(),']'))
			{
				TTPARSER_EXCEPT("need ]");
				return 0;
			}		
		}
		else if (checkOperator(t, '.'))
		{
			input->next();
			Token t = input->next();
			if (t.type != TT_NAME)
			{
				TTPARSER_EXCEPT("unexpect token");
				return 0;
			}
			ConstNode* cn = new ConstNode;
			copyString(cn->value.s, t.string, t.size);
			vn->index = cn;
		}
		else
			break;

		VarNode* next = new VarNode;
		next->var = v;
		v = next;
	}

	return v;



	

	//if ( index.isNull() && member.isNull())
	//{
	//	NameNode* nn = new NameNode;
	//	copyString(nn->name, name.string, name.size);
	//	return nn;
	//}
	//else
	//{
	//	VarNode* vn = new VarNode;
	//	vn->type = AT_DEFAULT;
	//	copyString(vn->name, name.string, name.size);
	//	vn->index = index;
	//	vn->member = member;
	//	return vn;
	//}

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
			Token t = input->next();
			ConstNode* cn = new ConstNode;
			switch (look.type)
			{
			case TT_NULL: cn->type = CT_NULL; break;
			case TT_TRUE: cn->type = CT_TRUE; break;
			case TT_FALSE: cn->type = CT_FALSE; break;
			case TT_INTEGER: cn->type = CT_INTEGER; break;
			case TT_DOUBLE: cn->type = CT_DOUBLE; break;
			case TT_STRING:	cn->type = CT_STRING; break;
			}

			if (t.type == TT_STRING)
				copyString(cn->value.s, t.val.s.b, t.val.s.s);
			else
				memcpy(&cn->value, &t.val, sizeof(t.val));

			expr = cn;
		}
		break;
	case TT_NAME://var funcall 
		{
			expr = parseAccessSymbol(input);
		}
		break;
	case TT_FUNCTION:
		expr = parseFunction(input, TT_LOCAL);
		break;
	case TT_OPERATOR:
		{
			if (checkOperator(look, '('))
			{
				input->next();
				expr = parseExpr(input);
				if (!checkOperator(input->next(), ')'))
				{
					TTPARSER_EXCEPT("need )");
					return 0;
				}
			}
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

	Binop binop;
	binop.type = OT_VOID;
	binop.left = expr;


	//�������Ƶ���Ԫ����
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
		if (checkOperator(t, '[') )
		{
			input->next();
			while (true)
			{
				Token t = input->next();
				ASTNode::Ptr o = parseVar(input);
				if (o.isNull()) break;
				last = last->setAndNext(o);
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
	}

	if (input->next().type != TT_ASSGIN)
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

ASTNode::Ptr Parser::parseUnop(ParserInput* input)
{
	Token op = input->next();

	OperatorType type;
	switch ( *op.string )
	{
	case '!':
		if (op.size == 2) return 0;//�ų� "!=" 
		type = OT_EXC;
		break;
	case '-':
		type = OT_NS;
		break;
	default:
		TTPARSER_EXCEPT("unexpected token");
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
			if (!checkOperator(input->next(), '('))
			{
				TTPARSER_EXCEPT("need (");
				return 0;
			}
			
			ASTNode::Ptr expr = parseExpr(input);
			if (expr.isNull())
			{
				TTPARSER_EXCEPT("fail to parse expr");
				return 0;
			}

			if (!checkOperator(input->next(), ')'))
			{
				TTPARSER_EXCEPT("need )");
				return 0;
			}

			ASTNode::Ptr block;
			if (checkOperator(input->next(), '{'))
			{
				block = parseBlock(input);

				if (!checkOperator(input->next(), '}'))
				{
					TTPARSER_EXCEPT("need }");
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
		TTPARSER_EXCEPT("for and do isnot supported");
		break;

	}
	return 0;
}

ASTNode::Ptr Parser::parseCond(ParserInput* input)
{
	Token h = input->next();
	Token t = h;

	if (t.type == TT_IF || t.type == TT_ELSEIF || t.type == TT_ELSE)
	{
		CondIFNode* cin = new CondIFNode();
		ASTNode::Ptr ci = cin;

		if (h.type != TT_ELSE)
		{//condition
			if (!checkOperator(input->next(), '('))
			{
				TTPARSER_EXCEPT("need (");
				return 0;		
			}

			cin->expr = parseExpr(input);

			if (cin->expr.isNull() ) 
			{
				TTPARSER_EXCEPT("need expr");
				return 0;
			}

			if (!checkOperator(input->next(), ')'))
			{
				TTPARSER_EXCEPT("need )");
				return 0;
			}
		}

		if (checkOperator(input->lookahead(), '{'))
		{
			input->next();
			cin->block = parseBlock(input);
			if (!checkOperator(input->next(), '}'))
			{
				TTPARSER_EXCEPT("need }");
				return 0;
			}
		}
		else {cin->block = parseExpr(input); }

		t = input->lookahead();

		if (h.type != TT_ELSE)
		{
			switch (t.type)
			{
			case TT_ELSEIF:
			case TT_ELSE:
				cin->elseif = parseCond(input);			
				break;
			}
		}

		return ci;

	}
	else if (t.type == TT_SWITCH)
	{
		TTPARSER_EXCEPT("switch isnot supported");
	}
	return 0;
}

ASTNode::Ptr Parser::parseFuncCall(ParserInput* input, ASTNode::Ptr pre)
{
	ASTNode::Ptr var = pre.isNull() ? parseVar(input) : pre;

	if (!checkOperator(input->next(), '('))
	{
		TTPARSER_EXCEPT("unexpected token�� need  )");
		return 0;
	}

	FuncCallNode* fn = new FuncCallNode();
	ASTNode::Ptr f = fn;
	fn->var = var;

	if (!checkOperator(input->lookahead(), ')'))
	{

		ASTNodeList::Ptr last = new ASTNodeList();
		fn->paras = last;

		while (true)
		{					
			ASTNode::Ptr o = parseExpr(input);
			if (!o.isNull()) 
				last = last->setAndNext(o);
			Token t = input->next();
			if (!checkOperator(t, ','))
			{
				if (checkOperator(t, ')')) break;

				TTPARSER_EXCEPT("unexpected token�� need , or )");
				return 0;
			}
		}
	}
	return f;
}

ASTNode::Ptr Parser::parseAccessSymbol(ParserInput* input)
{
	ASTNode::Ptr var = parseVar(input);
	if (checkOperator(input->lookahead(), '('))
	{
		return parseFuncCall(input, var);
	}
	else
		return var;
}



bool Parser::checkOperator(const Token& t, Char c)
{
	return t.type == TT_OPERATOR && c == *t.string && t.size == 1;
}

void Parser::copyString(Char* d, const Char* s, size_t count)
{
	memcpy(d, s, count * sizeof(Char));
	d[count] = 0;
}