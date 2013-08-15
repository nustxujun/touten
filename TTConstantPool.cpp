#include "TTConstantPool.h"
#include <memory.h>
using namespace TT;

ConstantPool::ConstantPool(size_t cap)
{
	mData = mLast = new char[cap];
	mTail = mData + cap;

	const ConstantType cts[] = 
	{
		CT_NULL, CT_TRUE, CT_FALSE
	};

	add(cts,sizeof(cts));

}

ConstantPool::~ConstantPool()
{
	delete mData;
	mData = mTail = mLast = 0;
}

size_t ConstantPool::add(const void* buff, size_t s)
{
	size_t offset = size();
	if (s + mLast >= mTail)
		reserve( (s + offset ) << 1);

	memcpy(mLast , buff, s);
	mLast += s;

	return offset;
}

void* ConstantPool::get(size_t offset)
{
	return mData + offset;
}

size_t ConstantPool::size()const
{
	return mLast - mData;
}

const void* ConstantPool::data()const
{
	return mData;
}

void ConstantPool::reserve(size_t s)
{
	char* temp = new char[s];
	size_t offset = size();
	memcpy(temp, mData, offset);
	delete mData;
	mData = temp;
	mLast = temp + offset;
	mTail = temp + s;
}