#ifndef _TTInterpreterCommon_H_
#define _TTInterpreterCommon_H_

#include "ToutenCommon.h"

namespace TT
{
	enum Instruction
	{
		//var
		LOAD,
		LOAD_CONST,
		LOAD_CPP_FUNC,
		LOAD_PARENT,
		STORE,
		STORE_ARRAY,
		ADDR,

		//func
		CALL,
		CALL_HOST,
		//opt
	
		RETURN,
		RETURN_ARRAY,


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

	typedef __int32 Operand;
	const int INSTR_SIZE = 1;
	typedef std::vector<char> Codes;


}
#endif