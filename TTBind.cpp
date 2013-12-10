#include "TTBind.h"

using namespace TT;

Bind::Bind(Touten* tt):
	mTT(tt)
{
}

Bind::~Bind()
{
	std::for_each(mFactors.begin(), mFactors.end(), [](Factors::value_type& i){ delete i.second; });
}