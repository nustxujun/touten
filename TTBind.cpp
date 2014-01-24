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
	//��touten��ע�ắ��ʱ��constant pool �в���������Ŀǰ���ᱻɾ��
	//�������Ƚ�С ������Ӱ������
	auto ret = mFactors.find(name);
	if (ret == mFactors.end()) return;

	delete ret->second;
	mFactors.erase(ret);
}

Touten* Bind::getTouten()
{
	return mTT;
}

