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

	typedef int Operand;
	const int INSTR_SIZE = 1;

	class Codes
	{
		std::vector<char> global;
		std::vector<char> code;

		bool switcher;
	public :
		Codes():switcher(true){}

		void push_back(char c)
		{
			if (switcher)
				code.push_back(c);
			else
				global.push_back(c);
		}

		char& operator[](size_t i)
		{
			if (switcher)
				return code[i];
			else
				return global[i];
		}

		void switchToCode(){ switcher = true;}
		void switchToGlobal(){ switcher = false;}

		const char* data(bool s)const {return s? code.data(): global.data();}

		size_t size()const {return switcher? code.size(): global.size();}
	};

}
#endif