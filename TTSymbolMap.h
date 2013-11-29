#ifndef _TTSymbolMap_H_
#define _TTSymbolMap_H_

#include "ToutenCommon.h"
#include "TTType.h"
#include <vector>

namespace TT
{

	enum SymbolType
	{
		//ST_FIELD,
		ST_FUNCTION,
		ST_VARIABLE,
		ST_CPP_FUNC,
	};

	class Symbol
	{
	public:
		AccessType actype;
		SymbolType symtype;
		size_t addrOffset;

		bool isdefine;
	};

	class SymbolMap
	{
	public :
		~SymbolMap();
		Symbol* createSymbol(const String& name, SymbolType st, AccessType at);
		Symbol* getSymbol(const String& name);

	private:
		Symbol* createSymbolImpl(SymbolType type);

	private:
		typedef TTMap<String, Symbol*> Symbols;
		Symbols mSymbols;
	};
}

#endif