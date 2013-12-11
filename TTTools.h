#ifndef _TTTools_H_
#define _TTTools_H_

#include"ToutenCommon.h"

namespace TT
{
	class Tools
	{
	public :
		static String toString(int val);
		static String toString(double val);


		static int toInt(const String& val);

		static Char* cloneString(const Char* str, size_t* len = nullptr);
		static Char* cloneString(const Char* str, size_t charlen);


		static bool less(const Char* s1, const Char* s2);

	};
}
#endif//_TTTools_H_