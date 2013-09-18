#include "TTCPPFunctionTable.h"

using namespace TT;



CPPFunctionTable::CPPFunctionTable(size_t size):
	mSize(size), mConstPool(sizeof(Elem) * size)
{
	mConstPool.reset(0);
}

CPPFunctionTable::CPPFunctionTable(const void* buff, size_t size):
	mSize(size / sizeof(Elem)), mConstPool(size)
{
	mConstPool.reset(0);

	assert((size % sizeof(Elem)) == 0);
	mConstPool.write(buff, size);
}

CPPFunctionTable::~CPPFunctionTable()
{
}


size_t CPPFunctionTable::insert(const Char* key, TT_Function func)
{
	size_t hashval = hash(key);
	size_t pos = hashval % mSize;
	Elem* tail = (Elem*)mConstPool[0] + mSize;
	Elem* elem = (Elem*)mConstPool[pos];
	Elem* head = elem;
	while (*elem->key != 0)
	{
		if (comp(elem->key, key) || comp(key, elem->key))
		{
			++elem;
			pos += sizeof(Elem);
			if (elem == tail) 
			{
				elem = (Elem*)mConstPool[0];
				pos = 0;
			}
			if (elem == head)
				return 0;
		}
	}

	String str(key);
	if (str.size() >= MAX_KEY_LEN) assert(0 && "function name is too long");
	memcpy(elem->key, key, str.size() * sizeof(Char));
	elem->key[str.size()] = 0;
	elem->obj.type = OT_FUNCTION;

	return pos;
}

Object* CPPFunctionTable::operator[](size_t index)const
{
	Elem* elem = (Elem*)mConstPool[index];
	return &(elem->obj);
}

const void* CPPFunctionTable::data()const
{
	return mConstPool.data();
}


size_t CPPFunctionTable::hash(const Char* key)const
{
	size_t nHash = 0;
	while (*key)
		nHash = (nHash << 5) + nHash + *key++;
	return nHash;
}

void CPPFunctionTable::grow()
{
	size_t size = (mSize << 1) * sizeof(Elem);
	CPPFunctionTable table(size);
	Elem* head = (Elem*)mConstPool[0];
	Elem* tail = head + mSize;
	for (; head != tail; ++head)
	{
		if (head->key) table.insert(head->key, (TT_Function)head->obj.val.func.codeAddr);
	}
	mConstPool.reserve(size);
	mConstPool.write(table.data(), size);
	
}



bool CPPFunctionTable::comp(const Char* s1, const Char* s2)const
{
	while(*s1 != 0 && *s2 != 0)
	{
		if (*s1 < *s2) return true;
		if (*s2 < *s1) return false;
		++s1; ++s2;
	}
	return *s1 < *s2;
}
