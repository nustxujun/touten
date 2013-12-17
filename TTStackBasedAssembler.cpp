#include "TTStackBasedAssembler.h"
#include "TTTools.h"
using namespace TT;


#define TTSBASSEMBLER_EXCPET(t) assert(0);
#define TTSBASSEMBLER_WARNING(t) assert(0);

StackBasedAssembler::StackBasedAssembler(ScopeManager& sm, ConstantPool& cp):
	mScopeMgr(sm), mConstPool(cp),
	mIsFuncall(false), mIsLeft(false)
{
}

void StackBasedAssembler::assemble(ASTNode::Ptr ast)
{
	ast->visit(this);

	//auto i = mBackFill.begin(), endi = mBackFill.end();
	//for (; i != endi; ++i)
	//{
	//	size_t c = i->first->addrOffset;
	//	if (c == -1)
	//	{
	//		 TTSBASSEMBLER_EXCPET("func undefine");
	//		 continue;
	//	}
	//	std::for_each(i->second.begin(), i->second.end(), [this, c](size_t a)
	//	{ 
	//		*(int*)(&mCodes[a]) = c;
	//	});
	//}
}

void StackBasedAssembler::visit(FileNode* node)
{
	mCurScope = mScopeMgr.enterScope();
	visit(node->defs);

	addInstruction(HALT);

	Scope::SymbolObj g = mCurScope->getSymbol(GLOBAL_INIT_FUNC);
	if (g.sym == 0)
	{
		g.sym = mScopeMgr.getGlobal()->createSymbol(GLOBAL_INIT_FUNC, ST_FUNCTION, AT_GLOBAL);
	}
	FunctionValue fv;
	fv.codeAddr = mCurScope->getCode();
	g.sym->addrOffset = mConstPool << fv;

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
			{
				size_t addr = addInstruction(LOAD_FUNC, mCurSymbol->addrOffset);
				if (!mCurSymbol->isdefine)
					addbackfill(mCurSymbol, addr );
			}
			break;
		default:
			{
				TTSBASSEMBLER_EXCPET("Unknown type");
			}
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
			TTSBASSEMBLER_EXCPET("undefined symbol");
			//考虑到在这里无法分清是哪层空间的符号，所以不予回填

			//mCurSymbol = mScopeMgr.getGlobal()->createSymbol(mCurName,ST_FUNCTION, AT_GLOBAL);
			//mCurSymbol->isdefine = false;

			//size_t addr = addInstruction(LOAD_FUNC, -1);//temp
			//addbackfill(mCurSymbol, addr );
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
		addInstruction(STORE, node->isRef ? IP_STORE_REF : IP_STORE_COPY);
}

void StackBasedAssembler::visit(VarListNode* node)
{
	visit(node->vars);
}

void StackBasedAssembler::visit(FunctionNode* node)
{

	auto sym = mCurScope->getSymbol(node->name);
	bool anonymous = node->name == L"";

	if (sym.sym != 0 )
	{
		if (sym.sym->symtype != ST_FUNCTION)
		{
			TTSBASSEMBLER_EXCPET("same name existed");
			return;
		}
		else if ( sym.sym->isdefine)
		{
			TTSBASSEMBLER_EXCPET("function is defined");
			return;
		}
		sym.sym->symtype = ST_FUNCTION;
	}
	else
	{
		if (anonymous)
		{
			static int index = 1;
			const String anonymous = L"__anonymous_function";
			sym.sym = mCurScope->createSymbol(
				anonymous + Tools::toString(index++), ST_FUNCTION, AT_GLOBAL);
		}
		else
		{
			Scope::Ptr scope = mCurScope;
			switch (node->acctype)
			{
			case AT_DEFAULT:
			case AT_LOCAL:
				break;
			case AT_GLOBAL:
				scope = mScopeMgr.getGlobal();
				break;
			default:
				TTSBASSEMBLER_EXCPET("unknown type");
			}
			sym.sym = scope->createSymbol(
				node->name, ST_FUNCTION, node->acctype);
			//sym.sym = mScopeMgr.getGlobal()->createSymbol(
			//	node->name, ST_FUNCTION, AT_GLOBAL);
		}
	}
	
	sym.sym->isdefine = true;

	enterScope(node->name.c_str());
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
			addInstruction(LOAD_LOCAL);
			addInstruction(STORE, IP_STORE_COPY);
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
	fv.funcinfo = count;
	fv.codeAddr = mCurScope->getCode();

	sym.sym->addrOffset = mConstPool << fv;

	node->body->visit(this);

	addInstruction(RETURN);
	leaveScope();

	if (anonymous)
		addInstruction(LOAD_FUNC, sym.sym->addrOffset);
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
		{
			String str;
			str.reserve(node->value.c);

			const Char* read = node->value.s;
			for (size_t i = 0; i < node->value.c; )
			{
				const Char* cur = read + i;
				if (*cur == '\\')
				{
					++i; ++cur;
					switch (*cur)
					{
					case 'a': str.push_back('\a'); break;
					case 'b': str.push_back('\b'); break;
					case 'f': str.push_back('\f'); break;
					case 'n': str.push_back('\n'); break;
					case 'r': str.push_back('\r'); break;
					case 't': str.push_back('\t'); break;
					case 'v': str.push_back('\v'); break;
					case '\n':str.push_back('\n'); break;
					case '\r':str.push_back('\r'); break;
					default:
						//error
						break;
					}
				}
				else
					str.push_back(*cur);

				++i;
			}
			addInstruction(LOAD_STRING, stringToConstPool(str));
		}
		return;
	}
}

void StackBasedAssembler::visit(FuncCallNode* node)
{
	//先推参数
	size_t argsnum = visit(node->paras);

	//再推符号

	
	mIsFuncall = node->func.isNull();
	node->var->visit(this);
	mIsFuncall = false;

	if (!node->func.isNull())
	{
		addInstruction(ATTACH);
		mIsFuncall = true;
		node->func->visit(this);
		mIsFuncall = false;
	}
	switch (mCurSymbol->symtype)
	{
	case ST_VARIABLE:
	case ST_FUNCTION:
	case ST_CPP_FUNC:
		{
			//stack desc:[before] --> [after]
			//paras funcname callinstr argsnum --> paras
			addInstruction(CALL, argsnum | (node->needrets? FunctionValue::NEED_RETURN : 0));
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
	size_t begin = mCurScope->getOffset();
	node->expr->visit(this);
	//addInstruction(TEST);
	size_t condjmp = addInstruction(JZ,0);
	node->block->visit(this);
	addInstruction(JMP,(Operand)begin);
	size_t end = mCurScope->getOffset();
	*(int*)(mCurScope->getCode()->data() + condjmp) = (Operand)end;

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
	return mCurScope->writeCode(&instr, INSTR_SIZE);
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

size_t StackBasedAssembler::stringToConstPool(const String& str)
{
	auto ret = mConstStringMap.find(str.c_str());
	if (ret != mConstStringMap.end())
		return ret->second;


	size_t charcount = str.size() + 1;

	size_t addr = mConstPool << charcount;
	mConstPool.write(str.c_str(), sizeof(Char)* charcount);

	mConstStringMap.insert(ConstStringMap::value_type((const Char*)mConstPool[addr + 4], addr));

	return addr;
}

