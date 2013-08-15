#ifndef _TTSymbolMap_H_
#define _TTSymbolMap_H_

#include "ToutenCommon.h"
#include "TTType.h"
#include <vector>

namespace TT
{

	enum SymbolType
	{
		ST_FIELD,
		ST_FUNCTION,
		ST_VARIABLE,
	};

	class Symbol
	{
	public:
		AccessType actype;
		SymbolType symtype;
		size_t addrOffset;

		bool isdefine;
		size_t codes;
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