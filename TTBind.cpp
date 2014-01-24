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

void Bind::remove(const String& name)
{
	//在touten中注册函数时在constant pool 中产生的数据目前不会被删掉
	//数据量比较小 基本不影响性能
	auto ret = mFactors.find(name);
	if (ret == mFactors.end()) return;

	delete ret->second;
	mFactors.erase(ret);
}

Touten* Bind::getTouten()
{
	return mTT;
}

