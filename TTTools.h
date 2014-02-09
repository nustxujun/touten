#ifndef _TTTools_H_
#define _TTTools_H_

#include"ToutenCommon.h"
#include <sstream>

namespace TT
{
	class ToutenExport Tools
	{
	public :
		template<class T>
		static String toString(T val)
		{
			std::wstringstream ss;
			ss << val;
			String ret;
			ss >> ret;
			return ret;
		}

		static int toInt(const String& val);

		static Char* cloneString(const Char* str, size_t* len = nullptr);
		static Char* cloneString(const Char* str, size_t charlen);


		static bool less(const Char* s1, const Char* s2);

	};
}
#endif//_TTTools_H_