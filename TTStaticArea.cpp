#include "TTStaticArea.h"
#include <memory.h>

using namespace TT;

StaticArea::StaticArea(size_t size ):
	mMemTotal(0)
{
	mLast = mDatas = createBuffer(size);
}

StaticArea::~StaticArea()
{
	delete mDatas;
}

StaticArea::Buffer* StaticArea::createBuffer(size_t size)
{
	Buffer* b = new Buffer;
	b->last = b->data = new char[size];
	b->tail = b->data + size;
	b->next = 0;
	mMemTotal += size;
	return b;
}

void* StaticArea::write(const void* b, size_t size)
{
	void* w = assgin(size);
	memcpy(w, b, size);
	return w;
}

void* StaticArea::assgin(size_t size)
{
	if (mLast->last + size >= mLast->tail)
		reserve(size + mMemTotal);
	void* w = mLast->last;
	mLast->last += size;
	return w;
}

void StaticArea::reserve(size_t s)
{
	Buffer* b = createBuffer(s);
	mLast->next = b;
	mLast = b;
}

size_t StaticArea::size()const
{
	return mDatas->size();
}
