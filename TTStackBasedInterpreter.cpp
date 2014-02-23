#include "TTStackBasedInterpreter.h"
#include "TTMemoryAllocator.h"
#include "TTException.h"
#include "TTFunctor.h"

using namespace TT;

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
	for (size_t i = 0; i < count; ++i, ++tmp) 
		tmp->~CallFrame(); 

	mLast = mHead;
}

bool CallStack::empty()const
{
	return mLast == mHead;
}



StackBasedInterpreter::StackBasedInterpreter()
{
	Caster::cast(mGlobalEnv, OT_ARRAY);
}

void StackBasedInterpreter::execute(const ConstantPool& constpool, const char* codes, size_t functionInfo, size_t parasCount, const ObjectPtr* paras, Object* callret)
{
	CallStack callstack;

	auto popOpr = [&callstack]()
	{
		callstack.top().vars.pop_back();
	};

	auto pushOpr = [&callstack](const Object& obj)
	{
		callstack.top().vars.push_back(obj);
	};

	auto pushOprPtr = [&callstack](const ObjectPtr& obj)
	{
		callstack.top().vars.push_back(obj);
	};

	auto popCallFrame = [&callstack, popOpr](ObjectPtr& ret)
	{
		auto& os = callstack.top().vars;
		size_t retcount = os.size();

		if (retcount == 0)
			callstack.pop();
		else if (retcount == 1)
		{
			ret = os.back();
			callstack.pop();
		}
		else
		{
			Array* arr = TT_NEW(Array)(false);
			for (size_t i = 0; !os.empty(); ++i)
			{
				*(*arr)[i] = *os.back();
				popOpr();
			}

			callstack.pop();
			(*ret).val->type = OT_ARRAY;
			(*ret).val->arr = arr;
		}
	};

	auto pushCallFrame = [&callstack](const char* begin, const char* current, bool bret)->CallFrame&
	{
		CallFrame& cf = callstack.push();
		cf.vars.reserve(8);
		cf.beginPos = begin;
		cf.curPos = current;
		cf.needret = bret;
		Caster::cast(cf.localenv, OT_ARRAY);
		return cf;
	};

	auto getArg = [&callstack](size_t num)->ObjectPtr&
	{
		auto& vec = callstack.top().vars;
		return *((vec.data() + vec.size() - 1) - num);
	};


	auto begin = codes;
	auto current = codes;//begfunc->codeAddr;
	auto getopr = [&current]()->Operand
	{
		Operand opr = *(Operand*)current;
		current += sizeof(Operand);
		return opr;
	};

	CallFrame& framebegin = pushCallFrame(0, 0, false);
	Object* localenv = &framebegin.localenv;
	framebegin.sharedenv = localenv;
	Object* sharedenv = localenv;
	Object* curenv = 0;
	
	size_t argsCount = functionInfo & FunctionValue::PARA_COUNT;
	{
		bool v = (functionInfo & FunctionValue::IS_VARIADIC) != 0;
		ObjectPtr va(OT_ARRAY);
		va->val->arr = TT_NEW(Array)(false);
		for (size_t i = 0;; ++i)
		{
			if (argsCount > i)
			{ 
				if (parasCount > i)
					pushOprPtr(paras[parasCount - i - 1]);
				else
					pushOpr(Object());
			}
			else if (v && parasCount > i)
				(*va->val->arr)[i - argsCount] = paras[parasCount - i - 1];
			else
				break;
		}
	
		if (v) pushOprPtr(va);
	}

	while (true)
	{
		Instruction instr = (Instruction)*current++;
		switch (instr)
		{
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
				const Object& name = *getArg(0);
				Caster::cast(*env, OT_ARRAY);
				ObjectPtr obj = (*env->val->arr)[name.val->str.cont];
				popOpr();
				pushOprPtr(obj);
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

				Object& obj = *getArg(0); 

				obj.swap(Object(str, size));//reduce copying
			}
			break;
		case LOAD_FUNC:
			pushOpr(*(FunctionValue*)(constpool[getopr()]));
			break;
		case LOAD_CPP_FUNC:
			pushOpr(*(FunctionValue*)(constpool[getopr()]));
			break;
		//case LOAD_VARIADIC:
		//	{
		//		ObjectPtr arr = localenv->val->arr->get(L"__Variadic");
		//		if (arr->val->type == OT_NULL) break;
		//		for (int i = 0;; ++i)
		//		{
		//			ObjectPtr o = arr->val->arr->get(i);
		//			if (o.isNull()) break;

		//			pushOprPtr(o);
		//		}
		//	}
		//	break;
		case STORE:
			{
				Operand opr = getopr();
				ObjectPtr& obj = getArg(0);
				ObjectPtr& val = getArg(1);
				switch (opr)
				{
				case IP_STORE_REF:
					obj->reference(*val); break;
				default:
					*obj = *val;
				}
				popOpr();
				popOpr();
			}
			break;
		case STORE_ARRAY:
			{
				Operand count = getopr();
				const Object& arr = *getArg(count);
				if (arr.val->type == OT_ARRAY)
				{
					for (int i = 0; i < count; ++i)
					{
						const Object& name = *getArg(0);
						*(*localenv->val->arr)[name.val->str.cont] = 
							*arr.val->arr->get(i);
						popOpr();
					}					
				}
				else
				{
					const Object& name = *getArg(count - 1);
					*(*localenv->val->arr)[name.val->str.cont] = arr;
					for (int i = 0; i < count; ++i)
						popOpr();
				}

				popOpr();//pop ret;
			}
			break;
		case ADDR:
			{
				const Object& index = *getArg(0);
				Object& obj = *getArg(1);

				curenv = &obj;

				Caster::cast(obj, OT_ARRAY);
			
				ObjectPtr elem ;
				if (index.val->type == OT_STRING)
					elem = (*obj.val->arr)[index.val->str.cont];
				else
					elem = (*obj.val->arr)[index.val->i];

				popOpr();
				popOpr();
				pushOprPtr(elem);

			}
			break;
		case ATTACH:
			{
				Object& obj = *getArg(0);
				curenv = &obj;
			}
			break;
		case CALL:
			{
				const Object& func = *getArg(0);
				size_t opr = getopr();
				size_t argsnum = FunctionValue::PARA_COUNT & opr;
				size_t bret = FunctionValue::NEED_RETURN & opr;
				bool hasVariadic = (FunctionValue::IS_VARIADIC & opr) != 0; 
				if (func.val->type != OT_FUNCTION)
				{
					popOpr();
					if (bret)
						pushOpr(Object());
					break;
				}

				size_t paraCount = FunctionValue::PARA_COUNT & func.val->func.funcinfo;
				bool isVariadic = (FunctionValue::IS_VARIADIC & func.val->func.funcinfo) != 0;

				if (FunctionValue::IS_CPP_FUNC & func.val->func.funcinfo)//c++ function
				{
					Functor* call = (Functor*)func.val->func.codeAddr;
					popOpr();

					auto& os = callstack.top().vars;
					size_t oprnum = os.size();
					int realnum = std::min(oprnum, argsnum);
					auto args = &*(os.data() + (oprnum - realnum));

					Object ret;
					(*call)(args, realnum, &ret);
					for (size_t i = 0; i < argsnum; ++i)
						popOpr();
					if (bret)
						pushOpr(ret);
				}
				else
				{
					Codes* codeaddr = (Codes*)func.val->func.codeAddr;
					popOpr();

					auto& os = callstack.top().vars;
					CallFrame& frame = pushCallFrame(begin, current, bret != 0);
					begin = current = codeaddr->data();
					int diff = (int)argsnum - (int)paraCount;
					if (diff == 0)
					{
						if (isVariadic && hasVariadic)
						{
							int variadicCount = 0;
							Array* arr = os.back()->val->arr;
							for (;; ++variadicCount)
								if (arr->get(variadicCount).isNull()) break;
							for (int i = 0; i < variadicCount; ++i)
								pushOpr(*arr->get(i));

							os.pop_back();
						}
					}
					else if (diff < 0)
					{
						if (hasVariadic)
						{ 
							int variadicCount = 0;
							Array* arr = os.back()->val->arr;
							for (;; ++variadicCount)
								if (arr->get(variadicCount).isNull()) break;

							diff = variadicCount + diff;
							if (diff <= 0 || isVariadic)
							{
								for (int i = 0; i < -diff; ++i)
									pushOpr(Object());

								for (int i = 0; i < variadicCount; ++i)
									pushOpr(*arr->get(i));

							}
							else if (0 < diff)
							{	
								for (int i = 0; i < diff; ++i)
									pushOpr(*arr->get(i));
							}
							os.pop_back();

							paraCount = argsnum;
						}
		
					}
					else if (isVariadic && hasVariadic)
					{ 
						ObjectPtr variadic = os.back();
						Array* arr = os.back()->val->arr;
						os.pop_back();
						int variadicCount = 0;
						for (;; ++variadicCount)
						if (arr->get(variadicCount).isNull()) break;

						for (int i = 0; i < variadicCount; ++i)
						for (int i = 0; i < 0; --i)
							pushOpr(*arr->get(i));

					}

					size_t maxcount = std::max(paraCount, argsnum);
					size_t pc = maxcount - paraCount;
					size_t an = maxcount - argsnum;
					ObjectPtr variadic;
					if (isVariadic)
					{
						variadic = ObjectPtr(OT_ARRAY);
						variadic->val->arr = TT_NEW(Array)(false);
						pushOprPtr(variadic);
					}
					for (size_t i = 0; i < maxcount; ++i)
					{
						if (i < pc)
						{
							if (isVariadic)
							{
								(*variadic->val->arr)[i] = os.back();
							}
							os.pop_back();
						}
						else if (i < an)
							pushOpr(Object());
						else
						{
							pushOprPtr(os.back());
							os.pop_back();
						}
					}

					localenv = &frame.localenv;
					frame.sharedenv = sharedenv;
					sharedenv = curenv;
					curenv = 0;
				}
				

			}
			break;
		case RETURN:
			{
				CallFrame& old = callstack.top();
				begin = old.beginPos;
				current = old.curPos;
				sharedenv = old.sharedenv;

				bool bret = old.needret;
				ObjectPtr ret(OT_NULL);
				popCallFrame(ret);
				if (callstack.empty())
				{
					if (callret)
						*callret = *ret;
					return;
				}
				if (bret)
					pushOprPtr(ret);

				CallFrame& frame = callstack.top();
				localenv = &frame.localenv;
			}
			break;
		case JZ:
			{
				size_t jumpos = getopr();
				const Object& obj = *getArg(0);
				if (!Caster::castToBool(obj))
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
			{
				const Object& o1 = *getArg(1);
				const Object& o2 = *getArg(0);
				Object o;

				switch (instr)
				{
				case DAND:case DOR:
					boolOpt(o1, o2, instr, o);
					break;
				case ADD: case SUB: case MUL: case DIV:
					normalOpt(o1, o2, instr, o);
					break;
				case EQ: case NE: case GREAT: case LESS: case GE: case LE:
					compareOpt(o1, o2, instr, o);
					break;
				case MOD: case AND: case OR: case XOR:
					{
						o.val->type = OT_INTEGER;
						INT_OPT(Caster::castToInt(o1), Caster::castToInt(o2), instr, o.val->i);
					}
				}


				popOpr();
				popOpr();
				pushOpr(o);
			}
			break;
		case HALT:
			return;
		default:
			TT_EXCEPT(ET_UNKNOWN, EL_NORMAL, "unknown instruction",0);
		}
	}

}

void StackBasedInterpreter::boolOpt(const Object& o1, const Object& o2, Instruction instr, Object& o)
{
	bool ret = false;
	switch (instr)
	{
	case DAND:	ret = Caster::castToInt(o1) && Caster::castToInt(o2); break;
	case DOR:	ret = Caster::castToInt(o1) || Caster::castToInt(o2); break;
	}

	o.val->type = ret ? OT_TRUE: OT_FALSE;

}


void StackBasedInterpreter::compareOpt(const Object& o1, const Object& o2, Instruction instr, Object& o)
{
	bool type[OT_NUM] = {0};
	type[o1.val->type] = true;
	type[o2.val->type] = true;

	bool ret = false;

	if (type[OT_NULL] || type[OT_FUNCTION] /*|| type[OT_FIELD]*/)
		o.val->type = OT_NULL;
	else if (type[OT_STRING])
	{
		CMP_OPT(Caster::castToString(o1), Caster::castToString(o2), instr, ret);
	}
	else if (type[OT_DOUBLE])
	{
		CMP_OPT(Caster::castToReal(o1), Caster::castToReal(o2), instr, ret);
	}
	else if (type[OT_INTEGER] || type[OT_TRUE] || type[OT_FALSE])
	{
		CMP_OPT(Caster::castToInt(o1), Caster::castToInt(o2), instr, ret);
	}
	else
	{
		TT_EXCEPT(ET_UNKNOWN, EL_NORMAL,"type cannt compare", 0);
	}

	o.val->type = ret ? OT_TRUE: OT_FALSE;
}


void StackBasedInterpreter::normalOpt(const Object& o1, const Object& o2, Instruction instr, Object& o)
{
	bool type[OT_NUM] = {0};
	type[o1.val->type] = true;
	type[o2.val->type] = true;

	if (type[OT_NULL] || type[OT_FUNCTION] /*|| type[OT_FIELD]*/)
	{
		o.val->type = OT_NULL;
	}
	else if (type[OT_STRING])
	{
		
	}
	else if (type[OT_DOUBLE])
	{
		o.val->type = OT_DOUBLE;
		BASE_OPT(Caster::castToReal(o1), Caster::castToReal(o2), instr, o.val->d);
	}
	else if (type[OT_INTEGER] || type[OT_TRUE] || type[OT_FALSE])
	{
		o.val->type = OT_INTEGER;
		BASE_OPT(Caster::castToInt(o1), Caster::castToInt(o2), instr, o.val->i);
	}
	else
	{
		TT_EXCEPT(ET_UNKNOWN, EL_NORMAL, "type cannt cal", 0);
	}

}
