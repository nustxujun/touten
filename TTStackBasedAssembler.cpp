#include "TTStackBasedAssembler.h"

using namespace TT;


#define TTSBASSEMBLER_EXCPET(t) 
#define TTSBASSEMBLER_WARNING(t) 

StackBasedAssembler::StackBasedAssembler(Codes& c, ScopeManager& sm, ConstantPool& cp):
	mCodes(c), mScopeMgr(sm), mConstPool(cp), mInstruction(0),	mMain(0), mHasMain(false)
{
}

bool StackBasedAssembler::hasMain()const 
{
	return mHasMain;
}

size_t StackBasedAssembler::getMain()const
{
	return mMain;
}


void StackBasedAssembler::assemble(ASTNode::Ptr ast)
{
	ast->visit(this);

	//auto i = mBackFill.begin(), endi = mBackFill.end();
	//for (; i != endi; ++i)
	//{
	//	//size_t c = i->first->codes;
	//	//std::for_each(i->second.begin(), i->second.end(), [this, c](size_t a){ mCodes[a] = c; });
	//}
}

void StackBasedAssembler::visit(FileNode* node)
{
	mCurScope = mScopeMgr.enterScope();

	visit(node->defs);

	auto s = mCurScope->getSymbol(L"main");
	if (s.sym && s.sym->symtype == ST_FUNCTION)
	{
		mHasMain = true;
		mMain = s.sym->ex;
	}

	mScopeMgr.leaveScope();

}

void StackBasedAssembler::visit(VarNode* node)
{
	node->var->visit(this);
	bool hasindex = !node->indexs->obj.isNull();

	if (mCurSymbol->actype == AT_SHARED)
	{
		//return;
	}


	if (node->left)
	{
		if (hasindex)
		{
			addInstruction(LOAD_REF, mCurSymbol->addrOffset);
		}
		
		
	}
	else
	{
		if (mCurSymbol->isdefine)
		{
			if (mCurSymbol->symtype == ST_VARIABLE)
				addInstruction(LOAD, mCurSymbol->addrOffset);
			else 
			{
				addInstruction(LOAD_CONST, mCurSymbol->ex);
			}
		}
		else
		{
			if (hasindex)
			{
				mCurSymbol->isdefine = true;
				addInstruction(LOAD, mCurSymbol->addrOffset);
			}
			else
				mBackFill[mCurSymbol].push_back(addInstruction(LOAD, mCurSymbol->addrOffset));
		}	
	}

	if (hasindex)
	{
		ASTNodeList::Ptr sub = node->indexs;
		while (!sub.isNull() && !sub->obj.isNull())
		{
			sub->obj->visit(this);
			addInstruction(ADDR);
			sub = sub->next;
		}
	}
}

void StackBasedAssembler::visit(AssginNode* node)
{
	node->right->visit(this);

	size_t varcount = visit(node->left);

	if (varcount > 1)
		addInstruction(STORE_ARRAY, varcount);
	else
		addInstruction(STORE, mCurSymbol->addrOffset);
}

void StackBasedAssembler::visit(VarListNode* node)
{
	visit(node->vars);
}

void StackBasedAssembler::visit(FunctionNode* node)
{

	auto sym = mCurScope->getSymbol(node->name);

	if (sym.sym != 0 && sym.local)
	{
		if (sym.sym->symtype != ST_FUNCTION)
		{
			TTSBASSEMBLER_EXCPET("same name existed");
		}
		else if ( sym.sym->isdefine)
		{
			TTSBASSEMBLER_EXCPET("function is defined");
		}
		sym.sym->symtype = ST_FUNCTION;
	}
	else
	{
		sym.sym = mCurScope->createSymbol(
			node->name, ST_FUNCTION, node->acctype);
	}
	
	sym.sym->ex = mInstruction;//temp
	sym.sym->isdefine = true;

	enterScope(node->name);
	ASTNodeList::Ptr sub = node->paras;
	size_t count = 0;
	while (!sub.isNull() && !sub->obj.isNull())
	{

		sub->obj->visit(this);
		mCurSymbol->isdefine = true;
		sub = sub->next;
		//addInstruction(STORE, count);
		++count;
	}

	Object fo;
	fo.val.func.paraCount = count;
	fo.val.func.codeAddr = sym.sym->ex;
	fo.type = OT_FUNCTION;
	sym.sym->ex = mConstPool << fo;

	node->body->visit(this);

	addInstruction(HALT);
	leaveScope();
}

void StackBasedAssembler::visit(FieldNode* node)
{
	auto sym = mCurScope->getSymbol(node->name);

	if (!sym.sym && sym.local )
	{
		if (sym.sym->symtype != ST_FIELD)
		{
			TTSBASSEMBLER_EXCPET("same name existed");
		}
		else if ( sym.sym->isdefine)
		{
			TTSBASSEMBLER_EXCPET("field is defined");
		}
		sym.sym->symtype = ST_FIELD;
	}
	else
	{
		sym.sym = mCurScope->createSymbol(
			node->name, ST_FUNCTION, node->acctype);
	}
	//sym->ex = mInstruction;
	//sym->isdefine = true;

	//mCurScope = mScopeMgr.enterScope();

	//node->body->visit(this);

	//mCurScope = mScopeMgr.leaveScope();
}


void StackBasedAssembler::visit(BlockNode* node)
{
	visit(node->stats);
}

void StackBasedAssembler::visit(ConstNode* node)
{
	size_t addr;
	Object obj;
	switch (node->type)
	{
	case CT_NULL:
		{
			obj.type = OT_NULL;
			static size_t null = mConstPool << obj;
			addr = null;
		}
		break;
	case CT_TRUE:
		{
			obj.type = OT_TRUE;
			static size_t tr = mConstPool << obj;
			addr = tr;
		}
		break;
	case CT_FALSE:
		{
			obj.type = OT_FALSE;
			static size_t fa = mConstPool << obj;
			addr = fa;
		}
		break;
	case CT_INTEGER:
		obj.type = OT_INTEGER;
		obj.val.i = node->value.i;
		addr = mConstPool << obj;
		break;
	case CT_DOUBLE:
		obj.type = OT_INTEGER;
		obj.val.i = node->value.i;
		addr = mConstPool << obj;
		break;
	case CT_STRING:
		{
			obj.type = OT_STRING;
			obj.val.str.size = 0;
			const Char* str = node->value.s;
			while (*str++) ++obj.val.str.size;
			addr = mConstPool.write(node->value.s, sizeof(Char) * (obj.val.str.size + 1));

			obj.val.str.cont = (Char*)mConstPool[addr];
			addr = mConstPool << obj;
		}
		break;
	}

	addInstruction(LOAD_CONST, addr);
}

void StackBasedAssembler::visit(FuncCallNode* node)
{
	//先推参数
	visit(node->paras);
	//再推符号
	node->var->visit(this);

	Symbol* s = mCurSymbol;

	if (s->isdefine && s->symtype != ST_FUNCTION)
	{
		TTSBASSEMBLER_EXCPET("symbol is not function");
	}

	//stack desc:[before] --> [after]
	//paras funcname callinstr --> paras
	size_t addr = addInstruction(CALL);
	if (!s->isdefine)
		addbackfill(s, addr);

}

void StackBasedAssembler::visit(OperatorNode* node)
{
	node->exp1->visit(this);
	node->exp2->visit(this);

	addInstruction( Instruction(BINOP_BEG + node->opt));
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
	auto s = mCurScope->getSymbol(node->name);
	if (s.sym == 0 || !s.local)
	{
		s.sym = mCurScope->createSymbol(node->name,ST_VARIABLE, node->type);
	}
	mCurSymbol = s.sym;
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
	addInstruction(instr);
	return addCode(opra);
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
