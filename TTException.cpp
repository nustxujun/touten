#include "TTException.h"

using namespace TT;

Exception::Exception(ExceptionLevel lv, const std::string& de, int l) :
level(lv),desc(de), line(l)
{

}

const std::string Exception::getDesc()const
{
	return desc;
}



int Exception::getLine()const
{
	return line;
}
