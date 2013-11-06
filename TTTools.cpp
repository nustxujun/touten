#include "TTTools.h"
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

int Tools::toInt(const String& val)
{
	wstringstream ss;
	ss << val;
	int ret;
	ss >> ret;
	return ret;
}