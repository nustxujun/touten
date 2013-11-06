#ifndef _TTTools_H_
#define _TTTools_H_

#include"ToutenCommon.h"

namespace TT
{
	class Tools
	{
	public :
		static String toString(int val);

		static int toInt(const String& val);
	};
}
#endif//_TTTools_H_