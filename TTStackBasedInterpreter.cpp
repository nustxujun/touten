#include "TTStackBasedInterpreter.h"
#include "TTMemoryAllocator.h"
using namespace TT;

#define TTINTERPRETER_EXCEPT(t)

#define INT_OPT(x, y, op, ret) {\
	switch (op){\
	case MOD:	(ret) = (x) % (y); break;\
	case AND:	(ret) = (x) & (y); break;\
	case OR:	(ret) = (x) | (y); break;\
	case XOR:	(ret) = (x) ^ (y); break;\
	}\
}

#define CMP_OPT(x, y, op, ret) {\
	switch (op){\
	case EQ:	(ret) = (x) == (y); break;\
	case NE:	(ret) = (x) != (y); break;\
	case GREAT:	(ret) = (x) > (y); break;\
	case LESS:	(ret) = (x) < (y); break;\
	case GE:	(ret) = (x) >= (y); break;\
	case LE:	(ret) = (x) <= (y); break;\
	}\
}

#define BASE_OPT(x, y, op, ret) {\
	switch (op){\
	case ADD:	(ret) = (x) + (y); break;\
	case SUB:	(ret) = (x) - (y); break;\
	case MUL:	(ret) = (x) * (y); break;\
	case DIV:	(ret) = (x) / (y); break;\
	}\
}

CallStack::CallStack()
{
	mLast = mHead = (CallFrame*)TT_MALLOC(sizeof(CallFrame) * 8);
	mTail = mHead + 8;
}

CallStack::~CallStack()
{
	TT_DELETE_ARRAY(CallFrame, mHead, mLast - mHead);

}

CallFrame& CallStack::top()
{
	return *(mLast - 1);
}

CallFrame& CallStack::push()
{
	if (mLast == mTail) reserve();
	new (mLast) CallFrame();//call constructor
	++mLast;
	return *(mLast - 1);
}

void CallStack::pop()
{
	if (mLast > mHead) 
	{
		--mLast;
		mLast->~CallFrame();
	}
}

void CallStack::reserve()
{
	size_t cap = mTail - mHead;
	mHead = (CallFrame*)TT_REALLOC(mHead,cap * sizeof(CallFrame) * 2);
	mLast = mHead + cap;
	mTail = mHead + cap * 2;
}

bool CallStack::empty()const
{
	return mLast == mHead;
}



StackBasedInterpreter::StackBasedInterpreter(const Codes& c, const ConstantPool& cp, const CPPFunctionTable& ft):
	mCodes(c), mConstPool(cp), mFuncTable(ft)
{
	mTempObj.reserve(32);
}

void StackBasedInterpreter::execute(size_t offset)
{
	Object* begfunc = (Object*)mConstPool[offset];

	pushCallFrame(0, 0);

	auto begin = mCodes.begin();
	auto current = begin + begfunc->val.func.codeAddr;
	auto getopr = [&current]()->Operand
	{
		Operand opr = *(Operand*)(&*current);
		current += sizeof(Operand);
		return opr;
	};

	auto getArg = [this](size_t num)->Object*
	{
		Object* o = *((mOprStack._Get_container().end() - 1) - num);
		return o;
	};

	Caster caster;
	while (true)
	{
		Instruction instr = (Instruction)*current++;
		switch (instr)
		{
		case LOAD:
			{
				pushOpr(mCallStack.top().vars[getopr()]);
			}
			break;
		case LOAD_CONST:
			{
				pushOpr((Object*)(mConstPool[getopr()]));
			}
			break;
		case LOAD_CPP_FUNC:
			{
				pushOpr(mFuncTable[getopr()]);
			}
			break;
		case STORE:
			{
				Object* obj = getArg(0);
				Object* val = getArg(1);
				*obj = *val;
				popOpr();
				popOpr();
			}
			break;
		case STORE_ARRAY:
			{
				Operand opr = getopr();
				Object* arr = getArg(opr);
				caster.cast(*arr, OT_ARRAY);
				for (size_t i = 0; i < opr; ++i)
				{
					*getArg(0) = arr->val.arr->get(opr - i - 1);
					popOpr();
				}
				popOpr();//pop ret;
			}
			break;
		case ADDR:
			{
				Object* index = getArg(0);
				Object* obj = getArg(1);
				if (obj->type != OT_ARRAY && index->val.i == 0)
				{
						popOpr();//pop index
				}
				else
				{
					caster.cast(*obj, OT_ARRAY);
					Object* elem ;
					if (index->type == OT_STRING)
						elem = &(*obj->val.arr)[index->val.str.cont];
					else
						elem = &(*obj->val.arr)[index->val.i];

					popOpr();
					popOpr();
					pushOpr(elem);
				}
			}
			break;
		case CALL:
			{
				Object* func = getArg(0);
				size_t argsnum = getopr();
				size_t paraCount = func->val.func.paraCount;
				size_t codeaddr = func->val.func.codeAddr;
				popOpr();
				CallFrame& frame = pushCallFrame(current - begin, mOprStack.size() - argsnum);
				current = begin + codeaddr;

				int i = argsnum - 1;
				for ( ; i >=0 ; --i)
				{
					if (i < paraCount)
						*frame.vars[i] = *mOprStack.top();
					popOpr();
				}
			}
			break;
		case CALL_HOST:
			{
				Object* func = getArg(0);
				size_t argsnum = getopr();
				size_t paraCount = func->val.func.paraCount;
				TT_Function call = (TT_Function)func->val.func.codeAddr;
				popOpr();
				//
				CallFrame& frame = pushCallFrame(current - begin, mOprStack.size() - argsnum);
				int i = argsnum - 1;
				for ( ; i >=0 ; --i)
				{
					*frame.vars[i] = *mOprStack.top();
					popOpr();
				}			

				Object* callret = pushOpr();
				int ret = call(argsnum, frame.vars.begin() , callret);
				popCallFrame();
			}
			break;
		case RETURN:
			{
				current = begin + popCallFrame();
				if (mCallStack.empty())
					return;
			}
			break;
		case RETURN_ARRAY:
			{
				assert(0);
			}
			break;
			//binop
		case DAND:case DOR:case AND:case OR:
		case XOR:case EQ:case NE:case GREAT:
		case LESS:case GE:case LE:case ADD:
		case SUB:case MUL:case DIV:case MOD:
			autoOpt(getArg(1), getArg(0), instr);
			break;
		case HALT:
			return;
		}
	}

}

void StackBasedInterpreter::popOpr()
{
	Object* obj = mOprStack.top();
	mOprStack.pop();

	CallFrame& cf = mCallStack.top();
	if (mTempObj.top() == obj)
	{
		mTempObj.pop();
	}
}

Object* StackBasedInterpreter::pushOpr(Object* obj)
{
	CallFrame& cf = mCallStack.top();
	if (!obj) 
	{
		obj = mTempObj.push();
	}
	mOprStack.push(obj);
	return obj;
}

CallFrame& StackBasedInterpreter::pushCallFrame(size_t codepos, size_t resopr)
{
	CallFrame& cf = mCallStack.push();
	cf.vars.resize(8);
	cf.beginPos = codepos;
	cf.reserveOpr = resopr;
	return cf;
}

size_t StackBasedInterpreter::popCallFrame()
{
	CallFrame& cf = mCallStack.top();

	size_t oprcount = mOprStack.size();
	size_t retcount = oprcount - cf.reserveOpr;
	size_t pos = cf.beginPos;

	if (retcount == 0)
	{
		mCallStack.pop();
		pushOpr();		
	}
	else if (retcount == 1)
	{
		Object tmp = *mOprStack.top();
		popOpr();
		mCallStack.pop();
		Object* obj = pushOpr();	
		*obj = tmp;
	}
	else
	{
		Array* arr = TT_NEW(Array)(false);
		for (size_t i = cf.reserveOpr; i < oprcount; ++i)
		{
			(*arr)[i - cf.reserveOpr] = *mOprStack.top();
			popOpr();
		}
		
		mCallStack.pop();

		Object* obj = pushOpr();
		obj->type = OT_ARRAY;
		obj->val.arr = arr;

	}
	return pos;
}

void StackBasedInterpreter::boolOpt(Object* o1, Object* o2, Instruction instr, Object* o)
{
	bool ret = false;
	switch (instr)
	{
	case DAND:	ret = cast<int>(*o1) && cast<int>(*o2); break;
	case DOR:	ret = cast<int>(*o1) || cast<int>(*o2); break;
	}

	o->type = ret ? OT_TRUE: OT_FALSE;

}

void StackBasedInterpreter::autoOpt(Object* o1, Object* o2, Instruction instr)
{
	Object* o = mTempObj.push();

	switch (instr)
	{
	case DAND:
	case DOR:
		boolOpt(o1, o2, instr, o);
		break;
	case ADD:
	case SUB:
	case MUL:
	case DIV:
		normalOpt(o1, o2, instr, o);
		break;
	case EQ:
	case NE:
	case GREAT:
	case LESS:
	case GE:
	case LE:
		compareOpt(o1, o2, instr, o);
		break;
	case MOD:
	case AND:
	case OR:
	case XOR:
		{
			o->type = OT_INTEGER;
			INT_OPT(cast<int>(*o1), cast<int>(*o2), instr, o->val.i);
		}
	}



	popOpr();
	popOpr();
	pushOpr(o);	
}

void StackBasedInterpreter::compareOpt(Object* o1, Object* o2, Instruction instr, Object* o)
{
	bool type[OT_NUM] = {0};
	type[o1->type] = true;
	type[o2->type] = true;

	bool ret = false;

	if (type[OT_NULL] || type[OT_FUNCTION] || type[OT_FIELD])
	{
		o->type = OT_NULL;
	}
	else if (type[OT_STRING])
	{
		
	}
	else if (type[OT_DOUBLE])
	{
		CMP_OPT(cast<double>(*o1), cast<double>(*o2), instr, ret);
	}
	else if (type[OT_INTEGER] || type[OT_TRUE] || type[OT_FALSE])
	{
		CMP_OPT(cast<int>(*o1), cast<int>(*o2), instr, ret);
	}
	else
	{
		assert(0);
	}

	o->type = ret ? OT_TRUE: OT_FALSE;
}


void StackBasedInterpreter::normalOpt(Object* o1, Object* o2, Instruction instr, Object* o)
{
	bool type[OT_NUM] = {0};
	type[o1->type] = true;
	type[o2->type] = true;

	if (type[OT_NULL] || type[OT_FUNCTION] || type[OT_FIELD])
	{
		o->type = OT_NULL;
	}
	else if (type[OT_STRING])
	{
		
	}
	else if (type[OT_DOUBLE])
	{
		o->type = OT_DOUBLE;
		BASE_OPT(cast<double>(*o1), cast<double>(*o2), instr, o->val.d);
	}
	else if (type[OT_INTEGER] || type[OT_TRUE] || type[OT_FALSE])
	{
		o->type = OT_INTEGER;
		BASE_OPT(cast<int>(*o1), cast<int>(*o2), instr, o->val.i);
	}
	else
	{
		assert(0);
	}

}




template<class Type>
Type StackBasedInterpreter::cast(Object& o)
{
	switch(o.type)
	{
	case OT_STRING:
		return ((TTString)o.val.str);
	case OT_DOUBLE:
		return (Type)o.val.d;
	case OT_INTEGER:
		return (Type)o.val.i;
	case OT_TRUE:
		return (Type)1;
	case OT_FALSE:
		return (Type)0;
	default:
		TTINTERPRETER_EXCEPT("cant cast");
		return 0;
	}
}