#ifndef _ToutenCommon_H_
#define _ToutenCommon_H_

#include <assert.h>

#include <algorithm>
#include <string>
#include <map>
#include <vector>
namespace TT
{
	typedef wchar_t Char;
	typedef std::wstring String;
	
#define TTMap ::std::map
	
}

#include "TTObject.h"

#define MAX_VAR_NAME_LEN 128

#endif