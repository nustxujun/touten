#include "TTStackBasedAssembler.h"

using namespace TT;


#define TTSBASSEMBLER_EXCPET(t) 
#define TTSBASSEMBLER_WARNING(t) 

StackBasedAssembler::StackBasedAssembler(StackBasedAssembler::Codes& c, ScopeManager& sm, ConstantPool& cp):
	mCodes(c), mScopeMgr(sm), mConstPool(cp), mInstruction(0)
{
}

void StackBasedAssembler::assemble(ASTNode::Ptr ast)
{
	ast->visit(this);

	auto i = mBackFill.begin(), endi = mBackFill.end();
	for (; i != endi; ++i)
	{
		size_t c = i->first->codes;
		std::for_each(i->second.begin(), i->second.end(), [this, c](size_t a){ mCodes[a] = c; });
	}
}

void StackBasedAssembler::visit(FileNode* node)
{
	mCurScope = mScopeMgr.enterScope();

	visit(node->defs);

	mScopeMgr.leaveScope();

}

void StackBasedAssembler::visit(VarNode* node)
{
	node->var->visit(this);
	
	if (mCurSymbol->symtype != ST_VARIABLE)
	{
		TTSBASSEMBLER_EXCPET("symbol redefine");
	}

	mCurSymbol->isdefine = true;

	if (mCurSymbol->actype == AT_SHARED)
	{
		//
	}
	else
	{
		addInstruction(LOAD, mCurSymbol->addrOffset);
	}
	if (!node->index.isNull())
	{
		node->index->visit(this);
		addInstruction(ADDR);
	}
}

void StackBasedAssembler::visit(AssginNode* node)
{
	size_t varcount = visit(node->left);

	node->right->visit(this);

	if (varcount > 1)
		addInstruction(ASSGIN_ARRAY, varcount);
	else
		addInstruction(ASSGIN);
}

void StackBasedAssembler::visit(VarListNode* node)
{
	visit(node->vars);
}

void StackBasedAssembler::visit(FunctionNode* node)
{

	Symbol* sym = mCurScope->getSymbol(node->name);

	if (sym != 0)
	{
		if (sym->symtype != ST_FUNCTION)
		{
			TTSBASSEMBLER_EXCPET("same name existed");
		}
		else if ( sym->isdefine)
		{
			TTSBASSEMBLER_EXCPET("function is defined");
		}
		sym->symtype = ST_FUNCTION;
	}
	else
	{
		sym = mCurScope->createSymbol(
			node->name, ST_FUNCTION, node->acctype);
	}

	sym->codes = mInstruction;
	sym->isdefine = true;

	enterScope(node->name);
	{
		ASTNodeList::Ptr sub = node->paras;
		size_t count = 0;
		while (!sub.isNull() && !sub->obj.isNull())
		{
			
			sub->obj->visit(this);
			mCurSymbol->isdefine = true;
			sub = sub->next;
			addInstruction(STORE, count);
			++count;
		}
	}
	node->body->visit(this);
	leaveScope();
}

void StackBasedAssembler::visit(FieldNode* node)
{
	Symbol* sym = mCurScope->getSymbol(node->name);

	if (!sym )
	{
		if (sym->symtype != ST_FIELD)
		{
			TTSBASSEMBLER_EXCPET("same name existed");
		}
		else if ( sym->isdefine)
		{
			TTSBASSEMBLER_EXCPET("field is defined");
		}
		sym->symtype = ST_FIELD;
	}
	else
	{
		sym = mCurScope->createSymbol(
			node->name, ST_FUNCTION, node->acctype);
	}
	sym->codes = mInstruction;
	sym->isdefine = true;

	mCurScope = mScopeMgr.enterScope();

	node->body->visit(this);

	mCurScope = mScopeMgr.leaveScope();
}


void StackBasedAssembler::visit(BlockNode* node)
{
	visit(node->stats);
}

void StackBasedAssembler::visit(ConstNode* node)
{
	size_t addr;
	switch (node->type)
	{
	case CT_NULL:
		addr = ConstantPool::Null;
		break;
	case CT_TRUE:
		addr = ConstantPool::True;
		break;
	case CT_FALSE:
		addr = ConstantPool::False;
		break;
	case CT_INTEGER:
		addr = mConstPool.add(node->type, node->value.i);
		break;
	case CT_DOUBLE:
		addr = mConstPool.add(node->type, node->value.d);
		break;
	case CT_STRING:
		addr = mConstPool.add(node->type, node->value.s);
		break;
	}

	addInstruction(LOAD_CONST, addr);
}

void StackBasedAssembler::visit(FuncCallNode* node)
{
	node->var->visit(this);

	Symbol* s = mCurSymbol;

	if (s->isdefine && s->symtype != ST_FUNCTION)
	{
		TTSBASSEMBLER_EXCPET("symbol is not function");
	}

	visit(node->paras);

	size_t addr = addInstruction(CALL);
	if (!s->isdefine)
		addbackfill(s, addr);

}

void StackBasedAssembler::visit(OperatorNode* node)
{
	node->exp1->visit(this);
	node->exp2->visit(this);

	addInstruction( Instruction(DAND + node->opt));
}

void StackBasedAssembler::visit(LoopNode* node)
{

}

void StackBasedAssembler::visit(CondIFNode* node)
{
}

void StackBasedAssembler::visit(ReturnNode* node)
{
	size_t count = visit(node->exprs);

	switch (count)
	{
	case 0:
		addInstruction(RETURN);
		break;
	case 1:
		addInstruction(RETURN_VALUE);
		break;
	default:
		addInstruction(RETURN_ARRAY, count);
	}


}

void StackBasedAssembler::visit(TT::NameNode * node)
{
	Symbol* s = mCurScope->getSymbol(node->name);
	if (s == 0 || node->forcedefine)
	{
		s = mCurScope->createSymbol(node->name,ST_VARIABLE, node->type);
	}
	mCurSymbol = s;
	mCurName = node->name;
}

size_t StackBasedAssembler::visit(ASTNodeList::Ptr list)
{
	ASTNodeList::Ptr sub = list;
	size_t count = 0;
	while (!sub.isNull() && !sub->obj.isNull())
	{
		++count;
		sub->obj->visit(this);
		sub = sub->next;
	}
	return count;
}

size_t StackBasedAssembler::addInstruction(Instruction instr)
{
	return addCode((char)instr);
}

size_t StackBasedAssembler::addInstruction(Instruction instr, Operand opra)
{
	size_t head = addCode(opra);
	addInstruction(instr);
	return head;
}

Scope::Ptr StackBasedAssembler::enterScope(const Char* name )
{
	if (name) return mCurScope = mScopeMgr.enterScope(name);
	else return mCurScope =  mScopeMgr.enterScope();
}

Scope::Ptr StackBasedAssembler::leaveScope()
{
	return mCurScope = mScopeMgr.leaveScope();
}

void StackBasedAssembler::addbackfill(Symbol* s, size_t instr)
{
	mBackFill[s].push_back(instr);
}
