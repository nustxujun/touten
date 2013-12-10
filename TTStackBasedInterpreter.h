#ifndef _TTStackBasedInterpreter_H_
#define _TTStackBasedInterpreter_H_

#include "TTInterpreterCommon.h"
#include "TTConstantPool.h"
#include "TTObject.h"
#include "TTEnvironment.h"
#include "TTCaster.h"
#include <stack>

namespace TT
{
	struct CallFrame
	{
		std::vector<ObjectPtr> vars;
		const char* beginPos;
		const char* curPos;
		Object localenv;
		Object* sharedenv;
		bool needret;
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
		
		void execute(const ConstantPool& cp, const char* codes, size_t parasCount, const ObjectPtr* paras, Object* obj);

	private:
		void popOpr();
		void pushOpr(Object* obj);
		void pushOpr(const Object& obj);

		CallFrame& pushCallFrame(const char* begin, const char* current, bool bret);
		void popCallFrame(Object& ret);

		void compareOpt(const Object& o1, const Object& o2, Instruction instr, Object& o);
		void normalOpt(const Object& o1, const Object& o2, Instruction instr, Object& o);
		void boolOpt(const Object& o1, const Object& o2, Instruction instr, Object& o);
		void autoOpt(const Object& o1, const Object& o2, Instruction instr);
	private:

		CallStack mCallStack;
		Object mGlobalEnv;
	};



}

#endif