#ifndef _TTStackBasedInterpreter_H_
#define _TTStackBasedInterpreter_H_

#include "TTInterpreterCommon.h"
#include "TTConstantPool.h"
#include "TTObject.h"
#include "TTCPPFunctionTable.h"
#include <stack>

namespace TT
{
	struct CallFrame
	{
		ObjectVector vars;
		size_t beginPos;
		size_t reserveOpr;
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
	private:
		CallFrame* mHead;
		CallFrame* mLast;
		CallFrame* mTail;

	};

	class StackBasedInterpreter
	{
	public :
		StackBasedInterpreter(const Codes& c, const ConstantPool& cp, const CPPFunctionTable& ft);
		
		void execute(size_t offset);

	private:
		void popOpr();
		Object* pushOpr(Object* obj = 0);

		CallFrame& pushCallFrame(size_t codepos, size_t resopr);
		size_t popCallFrame();
		template<class Type>
		Type cast(Object& o);

		void compareOpt(Object* o1, Object* o2, Instruction instr, Object* o);
		void normalOpt(Object* o1, Object* o2, Instruction instr, Object* o);
		void boolOpt(Object* o1, Object* o2, Instruction instr, Object* o);
		void autoOpt(Object* o1, Object* o2, Instruction instr);
	private:
		const Codes& mCodes;
		const ConstantPool& mConstPool;
		const CPPFunctionTable& mFuncTable;

		typedef std::stack<Object*> OperandStack;
		OperandStack mOprStack;

		ObjectStack mTempObj;

		CallStack mCallStack;

	};



}

#endif