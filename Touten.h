#ifndef _Touten_H_
#define _Touten_H_



#include "ToutenCommon.h"
#include "TTStaticArea.h"
#include "TTLexer.h"
#include "TTParser.h"
#include "TTStackBasedAssembler.h"
#include "TTStackBasedInterpreter.h"

namespace TT
{
	class Touten
	{
	public :
		bool loadFile(const String& name);
	};
}

#endif