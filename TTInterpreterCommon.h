#ifndef _TTInterpreterCommon_H_
#define _TTInterpreterCommon_H_

#include "ToutenCommon.h"

namespace TT
{
	enum Instruction
	{
		//var
		LOAD,
		LOAD_SHARED,
		LOAD_GLOBAL,
		LOAD_LOCAL,

		LOAD_NULL,
		LOAD_BOOL,
		LOAD_INT,
		LOAD_REAL,
		LOAD_STRING,
		LOAD_FUNC,
		LOAD_CPP_FUNC,

		STORE,
		STORE_ARRAY,
		ADDR,
		ATTACH,

		//func
		CALL,
		CALL_HOST,
		//opt
	
		RETURN,
		RETURN_ARRAY,

		//
		TEST,
		JMP,
		JZ,		//jump if zero

		//binop
		BINOP_BEG,
		DAND,
		DOR,
		AND,
		OR,
		XOR,
		EQ,
		NE,
		GREAT,
		LESS,
		GE,
		LE,
		ADD,
		SUB,
		MUL,
		DIV,
		MOD,

		//unop
		EXC,
		NS,

		HALT,
		VOID
	};

	enum InstructionParameter
	{
		IP_STORE_REF,
		IP_STORE_COPY,
	};

	typedef int Operand;
	const int INSTR_SIZE = 1;

	typedef std::vector<char> Codes;

	//class Codes
	//{

	//public :
	//	typedef std::vector<char> Code;
	//	typedef std::vector<Code*> CodeVec;


	//public :
	//	Codes()
	//	{
	//	}

	//	~Codes()
	//	{
	//		std::for_each(mCodeVec.begin(), mCodeVec.end(), [](Code* c){ delete c;});
	//	}

	//	void push_back(char c)
	//	{
	//		(*mCurrent)->push_back(c);
	//	}

	//	char& operator[](size_t i)
	//	{
	//		return (**mCurrent)[i];
	//	}

	//	const char* data()const 
	//	{
	//		return (*mCurrent)->data();
	//	}

	//	size_t size()const 
	//	{
	//		return (*mCurrent)->size();
	//	}
	//	
	//	size_t addCode()
	//	{
	//		mCodeVec.push_back(new Code());
	//		return mCodeVec.size() - 1;
	//	}

	//	Code* setCode(size_t i)
	//	{
	//		return mCodeVec[i];
	//	}	
	//private:
	//	CodeVec::iterator mCurrent;
	//	CodeVec mCodeVec;
	//};

}
#endif