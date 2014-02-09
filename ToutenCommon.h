#ifndef _ToutenCommon_H_
#define _ToutenCommon_H_

#include <assert.h>

#include <algorithm>
#include <string>
#include <map>
#include <hash_map>
#include <vector>
#include <set>
#include <stack>

namespace TT
{
	typedef wchar_t Char;
	typedef std::wstring String;
	
#define TTMap ::std::hash_map
	

	class ConstantPool;

	class Functor;

	class Object;
	class ObjectPtr;

	class ScopeManager;
	class StackBasedInterpreter;
	class Symbol;
}

#include "TTPlatform.h"

#endif