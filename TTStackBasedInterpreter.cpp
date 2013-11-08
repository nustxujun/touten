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
	new (mLast) CallFrame();//call cotr
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

void CallStack::clear()
{
	CallFrame* tmp = mHead; 
	size_t count = mLast - mHead;
	for (int i = 0; i < count; ++i, ++tmp) 
		tmp->~CallFrame(); 

	mLast = mHead;
}

bool CallStack::empty()const
{
	return mLast == mHead;
}



StackBasedInterpreter::StackBasedInterpreter()
{
	mCaster.cast(mGlobalEnv, OT_ARRAY);
}

void StackBasedInterpreter::execute(const ConstantPool& constpool,const char* codes)
{
	mCallStack.clear();
	CallFrame& framebegin = pushCallFrame(0, 0, false);
	Object* localenv = &framebegin.localenv;
	framebegin.sharedenv = localenv;
	Object* sharedenv = localenv;
	Object* curenv = 0;
	
	//FunctionValue* begfunc = (FunctionValue*)constpool[offset];

	auto begin = codes;
	auto current = codes;//begfunc->codeAddr;
	auto getopr = [&current]()->Operand
	{
		Operand opr = *(Operand*)current;
		current += sizeof(Operand);
		return opr;
	};

	auto getArg = [this](size_t num)->Object&
	{
		return **((mCallStack.top().vars.end() - 1) - num);
	};

	
	while (true)
	{
		Instruction instr = (Instruction)*current++;
		switch (instr)
		{
		case LOAD:
			{
				assert(0);
			}
			break;
		case LOAD_SHARED:
		case LOAD_GLOBAL:
		case LOAD_LOCAL:
			{
				Object* env = localenv;
				switch (instr)
				{
				case LOAD_SHARED: env = sharedenv ;break;
				case LOAD_GLOBAL: env = &mGlobalEnv;break;
				}
				const Object& name = getArg(0);
				Object& obj = *(*env->val.arr)[name.val.str.cont];
				popOpr();
				pushOpr(&obj);
			}
			break;

		case LOAD_NULL:
			pushOpr(Object());
			break;
		case LOAD_BOOL:
			pushOpr(Object(getopr() != 0));
			break;
		case LOAD_INT:
			//pushOpr(*(int*)(mConstPool[getopr()]));
			pushOpr(getopr());
			break;
		case LOAD_REAL:
			pushOpr(*(double*)(constpool[getopr()]));
			break;
		case LOAD_STRING:
			{
				Operand opr = getopr();
				size_t size = *(size_t*)(constpool[opr]);
				const Char* str= (const Char*)(constpool[opr + 4]);
				pushOpr(Object());

				Object& obj = getArg(0); 

				obj.swap(Object(str, size));//reduce copying
			}
			break;
		case LOAD_FUNC:
			pushOpr(*(FunctionValue*)(constpool[getopr()]));
			break;
		case LOAD_CPP_FUNC:
			pushOpr(*(FunctionValue*)(constpool[getopr()]));
			break;
		case STORE:
			{
				Object& obj = getArg(0);
				const Object& val = getArg(1);
				//*(*localenv->val.arr)[name.val.str.cont] = val;
				obj = val;
				popOpr();
				popOpr();
			}
			break;
		case STORE_ARRAY:
			{
				Operand count = getopr();
				const Object& arr = getArg(count);
				if (arr.type == OT_ARRAY)
				{
					for (size_t i = 0; i < count; ++i)
					{
						const Object& name = getArg(0);
						*(*localenv->val.arr)[name.val.str.cont] = 
							*arr.val.arr->get(i);
						popOpr();
					}					
				}
				else
				{
					const Object& name = getArg(count - 1);
					*(*localenv->val.arr)[name.val.str.cont] = arr;
					for (size_t i = 0; i < count; ++i)
						popOpr();
				}

				popOpr();//pop ret;
			}
			break;
		case ADDR:
			{
				const Object& index = getArg(0);
				Object& obj = getArg(1);

				curenv = &obj;

				mCaster.cast(obj, OT_ARRAY);
			
				Object* elem ;
				if (index.type == OT_STRING)
					elem = (*obj.val.arr)[index.val.str.cont];
				else
					elem = (*obj.val.arr)[index.val.i];

				popOpr();
				popOpr();
				pushOpr(elem);

			}
			break;
		case CALL:
			{
				const Object& func = getArg(0);
				size_t opr = getopr();
				size_t argsnum = 0x7fffffff & opr;
				size_t bret = 0x80000000 & opr;
				if (func.type != OT_FUNCTION)
				{
					popOpr();
					if (bret)
						pushOpr(Object());
					break;
				}

				size_t paraCount = func.val.func.paraCount;
				Codes* codeaddr = (Codes*)func.val.func.codeAddr;
				popOpr();

				auto& os = mCallStack.top().vars;
				CallFrame& frame = pushCallFrame(begin, current, bret != 0);
				begin = current = codeaddr->data();

				int i = argsnum - 1;
				for ( ; i >=0 ; --i)
				{
					if (i < paraCount)
					{
						pushOpr(*os.back() );
						os.pop_back();
					}
					
				}
					
				localenv = &frame.localenv;
				frame.sharedenv = sharedenv;
				sharedenv = curenv;
				curenv = 0;
			}

			break;
		case CALL_HOST:
			{
				const Object& func = getArg(0);
				size_t opr = getopr();
				size_t argsnum = 0x7fffffff & opr;
				size_t bret = 0x80000000 & opr;
				size_t paraCount = func.val.func.paraCount;
				TT_Function call = (TT_Function)func.val.func.codeAddr;
				popOpr();

				auto& os = mCallStack.top().vars;
				size_t oprnum = os.size();
				int realnum = (oprnum < argsnum) ? oprnum : argsnum;
				auto i = &*(os.begin() + (oprnum - realnum));
			
				Object callret ;
				int ret = call( i , realnum, &callret);
				for (int i = 0; i < argsnum; ++i)
					popOpr();
				if (bret)
					pushOpr(callret);
			}
			break;
		case RETURN:
			{
				CallFrame& old = mCallStack.top();
				begin = old.beginPos;
				current = old.curPos;
				bool bret = old.needret;
				Object ret;
				popCallFrame(ret);
				if (mCallStack.empty())
					return;
				if (bret)
					pushOpr(ret);

				CallFrame& frame = mCallStack.top();
				localenv = &frame.localenv;
				sharedenv = frame.sharedenv;
			}
			break;
		case RETURN_ARRAY:
			{
				assert(0);
			}
			break;
		case TEST:
			{
				
			}
			break;
		case JZ:
			{
				size_t jumpos = getopr();
				const Object& obj = getArg(0);
				if (!mCaster.castToBool(obj))
					current = begin + jumpos;
				popOpr();

			}
			break;
		case JMP:
			{
				size_t jumpos = getopr();
				current = begin + jumpos;
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
	mCallStack.top().vars.pop_back();
}

void StackBasedInterpreter::pushOpr(Object* obj)
{
	mCallStack.top().vars.push_back(obj);
}

void StackBasedInterpreter::pushOpr(const Object& obj)
{
	mCallStack.top().vars.push_back(obj);
}


CallFrame& StackBasedInterpreter::pushCallFrame(const char* begin, const char* current, bool bret)
{
	CallFrame& cf = mCallStack.push();
	cf.vars.reserve(8);
	cf.beginPos = begin;
	cf.curPos = current;
	cf.needret = bret;
	mCaster.cast(cf.localenv, OT_ARRAY);
	return cf;
}

void StackBasedInterpreter::popCallFrame(Object& ret)
{
	auto& os = mCallStack.top().vars;

	size_t retcount = os.size();
	//const char* pos = mCallStack.top().beginPos;

	if (retcount == 0)
	{
		mCallStack.pop();
		//pushOpr(Object());
	}
	else if (retcount == 1)
	{
		ret = *os.back();
		mCallStack.pop();
		//pushOpr(tmp);	
	}
	else
	{
		Array* arr = TT_NEW(Array)(false);
		for (size_t i = 0; !os.empty(); ++i)
		{
			*(*arr)[i] = *os.back();
			popOpr();
		}
		
		mCallStack.pop();

		ret.type = OT_ARRAY;
		ret.val.arr = arr;

	}
	//return pos;
}

void StackBasedInterpreter::boolOpt(const Object& o1, const Object& o2, Instruction instr, Object& o)
{
	bool ret = false;
	switch (instr)
	{
	case DAND:	ret = mCaster.castToInt(o1) && mCaster.castToInt(o2); break;
	case DOR:	ret = mCaster.castToInt(o1) || mCaster.castToInt(o2); break;
	}

	o.type = ret ? OT_TRUE: OT_FALSE;

}

void StackBasedInterpreter::autoOpt(const Object& o1, const Object& o2, Instruction instr)
{
	Object o;

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
			o.type = OT_INTEGER;
			INT_OPT(mCaster.castToInt(o1), mCaster.castToInt(o2), instr, o.val.i);
		}
	}



	popOpr();
	popOpr();
	pushOpr(o);	
}

void StackBasedInterpreter::compareOpt(const Object& o1, const Object& o2, Instruction instr, Object& o)
{
	bool type[OT_NUM] = {0};
	type[o1.type] = true;
	type[o2.type] = true;

	bool ret = false;

	if (type[OT_NULL] || type[OT_FUNCTION] || type[OT_FIELD])
	{
		o.type = OT_NULL;
	}
	else if (type[OT_STRING])
	{
		
	}
	else if (type[OT_DOUBLE])
	{
		CMP_OPT(mCaster.castToReal(o1), mCaster.castToReal(o2), instr, ret);
	}
	else if (type[OT_INTEGER] || type[OT_TRUE] || type[OT_FALSE])
	{
		CMP_OPT(mCaster.castToInt(o1), mCaster.castToInt(o2), instr, ret);
	}
	else
	{
		assert(0);
	}

	o.type = ret ? OT_TRUE: OT_FALSE;
}


void StackBasedInterpreter::normalOpt(const Object& o1, const Object& o2, Instruction instr, Object& o)
{
	bool type[OT_NUM] = {0};
	type[o1.type] = true;
	type[o2.type] = true;

	if (type[OT_NULL] || type[OT_FUNCTION] || type[OT_FIELD])
	{
		o.type = OT_NULL;
	}
	else if (type[OT_STRING])
	{
		
	}
	else if (type[OT_DOUBLE])
	{
		o.type = OT_DOUBLE;
		BASE_OPT(mCaster.castToReal(o1), mCaster.castToReal(o2), instr, o.val.d);
	}
	else if (type[OT_INTEGER] || type[OT_TRUE] || type[OT_FALSE])
	{
		o.type = OT_INTEGER;
		BASE_OPT(mCaster.castToInt(o1), mCaster.castToInt(o2), instr, o.val.i);
	}
	else
	{
		assert(0);
	}

}
