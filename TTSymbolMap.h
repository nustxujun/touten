#ifndef _TTSymbolMap_H_
#define _TTSymbolMap_H_

#include "ToutenCommon.h"

namespace TT
{

	enum SymbolType
	{
		ST_FIELD,
		ST_FUNCTION,
		ST_VARIABLE,


		ST_GLOBAL  = 1 << 4,
		ST_SHARED  = 1 << 5,
		ST_LOCAL   = 1 << 6,
	};

	class Symbol
	{
	public:
		SymbolType type;
		String name;
	};

	class SymbolMap
	{
	public :
		Symbol* createSymbol(const String& name);
		Symbol* getSymbol(const String& name);
	};
}

#endif