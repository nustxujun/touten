#ifndef _TTType_H_
#define _TTType_H_

namespace TT
{
	enum OperatorType
	{
		OT_VOID,

		//binop
		OT_DAND,			// &&
		OT_DOR	= OT_DAND,	// ||

		OT_AND,				// &
		OT_OR	= OT_AND,	// ||
		OT_XOR	= OT_AND,	// ^

		OT_EQ,				// ==
		OT_NE	= OT_EQ,	// !=

		OT_GREAT,			// >
		OT_LESS	= OT_GREAT,	// <
		OT_GE	= OT_GREAT,	// >=
		OT_LE	= OT_GREAT,	// <=

		OT_ADD,				// +
		OT_SUB	= OT_ADD,	// -

		OT_MUL,				// *
		OT_DIV	= OT_MUL,	// /
		OT_MOD	= OT_MUL,	// %

		//unop
		OT_EXC,				// !
		OT_NS	= OT_EXC,	// - 
	};

	enum VariableType
	{
		VT_GLOBAL,
		VT_SHARED,
		VT_LOCAL,
	};

	enum ConstantType
	{
		VT_NULL,
		VT_TRUE,
		VT_FALSE,
		VT_INTEGER,
		VT_DOUBLE,
		VT_STRING,		
	};
}

#endif