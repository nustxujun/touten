#ifndef _TTStackBasedInterpreter_H_
#define _TTStackBasedInterpreter_H_

#include "TTInterpreterCommon.h"
#include "TTConstantPool.h"
#include "TTObject.h"
#include "TTCPPFunctionTable.h"
#include "TTEnvironment.h"
#include <stack>

namespace TT
{
	struct CallFrame
	{
		std::stack<ObjectPtr> vars;
		size_t beginPos;
		Object localenv;
		Object* sharedenv;
	};

	class CallStack
	{
	public :
		CallStack();
		~CallStack();
		CallFrame& top();
		CallFrame& push();
		void pop();
		bool empty()const;
		void reserve();
		void clear();

	private:
		CallFrame* mHead;
		CallFrame* mLast;
		CallFrame* mTail;

	};

	class StackBasedInterpreter
	{
	public :
		StackBasedInterpreter();
		
		void execute(const ConstantPool& cp, const CPPFunctionTable& ft, const char* codes, size_t offset = 0);

	private:
		void popOpr();
		Object& pushOpr(Object* obj);
		Object& pushOpr(const Object& obj);

		CallFrame& pushCallFrame(size_t codepos);
		size_t popCallFrame(Object& ret);

		void compareOpt(const Object& o1, const Object& o2, Instruction instr, Object& o);
		void normalOpt(const Object& o1, const Object& o2, Instruction instr, Object& o);
		void boolOpt(const Object& o1, const Object& o2, Instruction instr, Object& o);
		void autoOpt(const Object& o1, const Object& o2, Instruction instr);
	private:

		CallStack mCallStack;
		Object mGlobalEnv;
		Caster mCaster;
	};



}

#endif