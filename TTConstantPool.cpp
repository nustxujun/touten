#include "TTConstantPool.h"
#include "TTMemoryAllocator.h"
#include <memory.h>
using namespace TT;

ConstantPool::ConstantPool(size_t cap)
{
	mData = mLast = (char*)TT_MALLOC(cap);
	mTail = mData + cap;
}

ConstantPool::~ConstantPool()
{
	TT_FREE( mData);
	mData = mTail = mLast = 0;
}

size_t ConstantPool::write(const void* buff, size_t s)
{
	size_t offset = size();
	if (s + mLast >= mTail)
		reserve( (s + offset ) << 1);

	memcpy(mLast , buff, s);
	mLast += s;

	return offset;
}

void* ConstantPool::get(size_t offset)const
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
	size_t offset = size();
	char* temp = (char*)TT_REALLOC(mData, s);
	mData = temp;
	mLast = temp + offset;
	mTail = temp + s;
}

void ConstantPool::clear()
{
	mLast = mData;
}

void ConstantPool::reset(int val)
{
	clear();
	memset(mData, val, mTail - mData);
}
