#ifndef _TTType_H_
#define _TTType_H_

namespace TT
{
	enum OperatorType
	{//注意这里顺序影响计算时候的优先级
		OT_VOID,

		//binop
		OT_DAND,			// &&
		OT_DOR	,	// ||

		OT_AND,				// &
		OT_OR	,	// ||
		OT_XOR	,	// ^

		OT_EQ,				// ==
		OT_NE	,	// !=

		OT_GREAT,			// >
		OT_LESS	,	// <
		OT_GE	,	// >=
		OT_LE	,	// <=

		OT_ADD,				// +
		OT_SUB	,	// -

		OT_MUL,				// *
		OT_DIV	,	// /
		OT_MOD	,	// %

		//unop
		OT_EXC,				// !
		OT_NS	,	// - 
	};

	const int OptPre[] = 
	{
		0,
		1,1,
		2,2,2,
		3,3,
		4,4,4,4,
		5,5,
		6,6,6,
		7,7
	};

	enum AccessType
	{
		AT_DEFAULT,
		AT_GLOBAL,
		AT_SHARED,
		AT_LOCAL,
	};

	enum ConstantType
	{
		CT_NULL,
		CT_TRUE,
		CT_FALSE,
		CT_INTEGER,
		CT_DOUBLE,
		CT_STRING,		
	};
}

#endif