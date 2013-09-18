#include "TTInterpreterCommon.h"
#include "TTMemoryAllocator.h"
#include <memory.h>
using namespace TT;

TTString::TTString(size_t count)
{
	numChar = count;
	data = TT_NEW_ARRAY(Char, numChar);
	*data = 0;
}

TTString::TTString(const StringValue& sv)
{
	numChar = sv.size + 1;
	data = TT_NEW_ARRAY(Char, numChar * sizeof(Char));
	memcpy(data, sv.cont, numChar* sizeof(Char));
}

TTString::TTString(int i)
{

}

TTString::TTString(double d)
{

}

TTString::~TTString()
{
	TT_FREE(data);
}

TTString TTString::operator+(const TTString& str)
{
	size_t newsize = numChar + str.numChar;
	TTString tmp(newsize);

	memcpy(tmp.data , data, numChar * sizeof(Char));
	memcpy(tmp.data + numChar, str.data, str.numChar * sizeof(Char));
	return tmp;
}

TTString::operator bool()const
{
	return 0;

}

TTString::operator int()const
{
	return 0;
}

TTString::operator double()const
{
	return 0;
}

