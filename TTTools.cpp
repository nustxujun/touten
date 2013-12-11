#include "TTTools.h"
#include "TTMemoryAllocator.h"
#include <sstream>
using namespace TT;
using namespace std;

String Tools::toString(int val)
{
	wstringstream ss;
	ss << val;
	String ret;
	ss >> ret;
	return ret;
}

String Tools::toString(double val)
{
	wstringstream ss;
	ss << val;
	String ret;
	ss >> ret;
	return ret;
}


int Tools::toInt(const String& val)
{
	wstringstream ss;
	ss << val;
	int ret;
	ss >> ret;
	return ret;
}

Char* Tools::cloneString(const Char* str, size_t* len )
{
	const Char* head = str;
	while (*head) ++head;

	if (len) *len = head - str + 1;

	size_t size = (head - str + 1) * sizeof(Char);
	Char* newstr = (Char*)TT_MALLOC( size );

	memcpy(newstr, str, size);
	return newstr;
}

Char* Tools::cloneString(const Char* str, size_t charlen)
{
	size_t size = charlen * sizeof(Char);
	Char* newstr = (Char*)TT_MALLOC(size);
	memcpy(newstr, str, size);
	return newstr;
}


bool Tools::less(const Char* s1, const Char* s2)
{
	while (*s1 != 0 && *s2 != 0)
	{
		if (*s1 < *s2) return true;
		if (*s2 < *s1) return false;
		++s1; ++s2;
	}
	return *s1 < *s2;
}