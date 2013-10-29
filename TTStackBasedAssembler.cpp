#include "TTStackBasedAssembler.h"

using namespace TT;


#define TTSBASSEMBLER_EXCPET(t) assert(0);
#define TTSBASSEMBLER_WARNING(t) assert(0);

StackBasedAssembler::StackBasedAssembler(Codes& c, ScopeManager& sm, ConstantPool& cp):
	mCodes(c), mScopeMgr(sm), mConstPool(cp), mMain(0),
	mHasMain(false), mIsFuncall(false), mIsLeft(false)
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

	auto i = mBackFill.begin(), endi = mBackFill.end();
	for (; i != endi; ++i)
	{
		size_t c = i->first->addrOffset;
		if (c == -1)
		{
			 TTSBASSEMBLER_EXCPET("func undefine");
			 continue;
		}
		std::for_each(i->second.begin(), i->second.end(), [this, c](size_t a)
		{ 
			*(int*)(&mCodes[a]) = c;
		});
	}
}

void StackBasedAssembler::visit(FileNode* node)
{
	mCurScope = mScopeMgr.enterScope();

	visit(node->defs);

	auto s = mCurScope->getSymbol(L"main");
	if (s.sym && s.sym->symtype == ST_FUNCTION)
	{
		mHasMain = true;
		mMain = s.sym->addrOffset;
	}


	mCodes.switchToGlobal();
	addInstruction(HALT);
	mCodes.switchToCode();

	mScopeMgr.leaveScope();

}

void StackBasedAssembler::visit(VarNode* node)
{
	node->var->visit(this);
	bool hasindex = !(node->indexs.isNull() || node->indexs->obj.isNull());

	if (mCurSymbol != 0)
	{
		switch (mCurSymbol->symtype)
		{
		case ST_VARIABLE:
			{
					Instruction instr ;
					switch (node->type)
					{
					case AT_SHARED: instr = LOAD_SHARED; break;
					case AT_GLOBAL: instr = LOAD_GLOBAL; break;
					case AT_LOCAL:  instr = LOAD_LOCAL; break;
					}
					addInstruction(LOAD_STRING, stringToConstPool(mCurName.c_str()));
					addInstruction(instr);
					
			}
			break;
		case ST_CPP_FUNC:
			addInstruction(LOAD_CPP_FUNC, mCurSymbol->addrOffset);break;
		case ST_FUNCTION:
		case ST_FIELD:
			{
				size_t addr = addInstruction(LOAD_FUNC, mCurSymbol->addrOffset);
				if (!mCurSymbol->isdefine)
					addbackfill(mCurSymbol, addr );
			}
			break;
		}
	}
	else
	{
		if (hasindex | !mIsFuncall )
		{
			Instruction instr = LOAD;
			Scope::Ptr scope;
			switch (node->type)
			{
			case AT_SHARED: 
				instr = LOAD_SHARED; 
				break;
			case AT_GLOBAL: 
				instr = LOAD_GLOBAL; 
				break;
			case AT_LOCAL:  
				instr = LOAD_LOCAL;
				break;
			}

			mCurSymbol = mCurScope->createSymbol(mCurName, ST_VARIABLE, node->type);
			addInstruction(LOAD_STRING, stringToConstPool(mCurName.c_str()));
			//if (!mIsLeft)
				addInstruction(instr);
		}
		else
		{
			mCurSymbol = mScopeMgr.getGlobal()->createSymbol(mCurName,ST_FUNCTION, AT_GLOBAL);
			mCurSymbol->isdefine = false;

			size_t addr = addInstruction(LOAD_FUNC, -1);//temp
			addbackfill(mCurSymbol, addr );
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

	mIsLeft = true;
	size_t varcount = visit(node->left);
	mIsLeft = false;

	if (varcount > 1)
		addInstruction(STORE_ARRAY, varcount);
	else
	{
		addInstruction(STORE);
	}
}

void StackBasedAssembler::visit(VarListNode* node)
{
	mCodes.switchToGlobal();
	visit(node->vars);
	mCodes.switchToCode();
}

void StackBasedAssembler::visit(FunctionNode* node)
{

	auto sym = mCurScope->getSymbol(node->name);

	if (sym.sym != 0 )
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
		sym.sym = mScopeMgr.getGlobal()->createSymbol(
			node->name, ST_FUNCTION, AT_GLOBAL);
	}
	
	sym.sym->addrOffset = mCodes.size();//temp
	sym.sym->isdefine = true;

	enterScope(node->name);
	ASTNodeList::Ptr sub = node->paras;
	size_t count = 0;
	while (!sub.isNull() && !sub->obj.isNull())
	{

		sub->obj->visit(this);
		auto sym = mCurScope->getSymbol(mCurName);
		if (sym.sym == 0 || !sym.local)
		{
			mCurSymbol = mCurScope->createSymbol(mCurName, ST_VARIABLE, AT_LOCAL);
			mCurSymbol->isdefine = true;
			addInstruction(LOAD_STRING, stringToConstPool(mCurName.c_str()));
			addInstruction(STORE);
		}
		else
		{
			TTSBASSEMBLER_EXCPET("same name");
		}
		sub = sub->next;
		//addInstruction(STORE, count);
		++count;
	}

	FunctionValue fv;
	fv.paraCount = count;
	fv.codeAddr = sym.sym->addrOffset;

	sym.sym->addrOffset = mConstPool << fv;

	node->body->visit(this);

	addInstruction(RETURN);
	leaveScope();
}

void StackBasedAssembler::visit(FieldNode* node)
{
	auto sym = mCurScope->getSymbol(node->name);

	if (!sym.sym  )
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
		sym.sym = mScopeMgr.getGlobal()->createSymbol(
			node->name, ST_FUNCTION, AT_LOCAL);
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
	Object obj;
	switch (node->type)
	{
	case CT_NULL:
		addInstruction(LOAD_NULL);
		return;
	case CT_TRUE:
		addInstruction(LOAD_BOOL, 1);
		return;
	case CT_FALSE:
		addInstruction(LOAD_BOOL, 0);
		return;
	case CT_INTEGER:
		//addInstruction(LOAD_INT, mConstPool << node->value.i);
		addInstruction(LOAD_INT, node->value.i);//加速整型读取
		return;
	case CT_DOUBLE:
		addInstruction(LOAD_REAL, mConstPool << node->value.d);
		return;
	case CT_STRING:
		addInstruction(LOAD_STRING, stringToConstPool(node->value.s));
		return;
	}
}

void StackBasedAssembler::visit(FuncCallNode* node)
{
	//先推参数
	size_t argsnum = visit(node->paras);

	//再推符号
	mIsFuncall = true;
	node->var->visit(this);
	mIsFuncall = false;

	switch (mCurSymbol->symtype)
	{
	case ST_VARIABLE:
	case ST_FUNCTION:
		{
			//stack desc:[before] --> [after]
			//paras funcname callinstr argsnum --> paras
			addInstruction(CALL, argsnum);
		}
		break;
	case ST_CPP_FUNC:
		{
			addInstruction(CALL_HOST, argsnum);
		}
		break;
	default:
		TTSBASSEMBLER_EXCPET("symbol is not function");
	}


}

void StackBasedAssembler::visit(OperatorNode* node)
{
	node->exp1->visit(this);
	node->exp2->visit(this);

	addInstruction( Instruction(BINOP_BEG + node->opt));
}

void StackBasedAssembler::visit(LoopNode* node)
{
	size_t begin = mCodes.size();
	node->expr->visit(this);
	addInstruction(TEST);
	size_t condjmp = addInstruction(JZ,0);
	node->block->visit(this);
	addInstruction(JMP,begin);
	size_t end = mCodes.size();
	*(int*)(&mCodes[condjmp]) = end;

}

void StackBasedAssembler::visit(CondIFNode* node)
{
}

void StackBasedAssembler::visit(ReturnNode* node)
{
	size_t count = visit(node->exprs);

	addInstruction(RETURN);
}

void StackBasedAssembler::visit(TT::NameNode * node)
{
	auto s = mCurScope->getSymbol(node->name);
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
	return addCode(&instr, INSTR_SIZE);
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

size_t StackBasedAssembler::stringToConstPool(const Char* str)
{
	size_t charcount = 1;
	const Char* tmp = str;
	while (*tmp++) ++charcount;

	size_t addr = mConstPool << charcount;
	mConstPool.write(str, sizeof(Char) * charcount);
	return addr;
}

