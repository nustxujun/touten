#include "TTObject.h"
#include "TTMemoryAllocator.h"
#include "TTTools.h"

using namespace TT;


TTString::TTString(size_t count)
{
	numChar = count;
	data = TT_NEW_ARRAY(Char, numChar);
	*data = 0;
}

TTString::TTString(const StringValue& sv)
{
	numChar = sv.size ;
	data = Tools::cloneString(sv.cont, numChar);
}

TTString::TTString(const Char* str)
{
	data = Tools::cloneString(str, &numChar);
}

TTString::TTString(int i)
{
	String tmp = Tools::toString(i);
	numChar = tmp.size() + 1;
	data = Tools::cloneString(tmp.c_str(), numChar);
}

TTString::TTString(double d)
{

}

TTString::~TTString()
{
	TT_FREE(data);
}



bool TTString::operator==(const Char* str)
{
	return !Tools::less(data, str) && !Tools::less(str, data);
}

bool TTString::operator==(const TTString& str)
{
	return !Tools::less(data, str.data) && !Tools::less(str.data, data);
}

TTString::operator bool()const
{
	return !Tools::less(data, L"true") && !Tools::less(L"true", data);
}

TTString::operator int()const
{

	return 0;
}

TTString::operator double()const
{
	return 0;
}

TTString::operator const Char*()const
{
	return data;
}


Object::Object():type(OT_NULL)
{

}

Object::Object(const Object& obj):
	type(OT_NULL)
{
	*this = obj;
}

Object::Object(bool v):
	type( v? OT_TRUE: OT_FALSE)
{
}

Object::Object(int v):
	type(OT_INTEGER) 
{
	val.i = v;
}

Object::Object(double v):
	type(OT_DOUBLE)
{
	val.d = v;
}

Object::Object(const Char* str, size_t size):
	type(OT_STRING)
{
	val.str.size = size;
	val.str.cont = Tools::cloneString(str, size);
}

Object::Object(FunctionValue v):
	type(OT_FUNCTION)
{
	val.func = v;
}

void Object::swap(Object& obj)
{
	std::swap(val, obj.val);
	std::swap(type, obj.type);
}


Object::~Object()
{
	switch (type)
	{
	case OT_ARRAY:
		TT_DELETE(Array, val.arr);
		break;
	case OT_STRING:
		TT_FREE(val.str.cont);
		break;
	}
	
}

Object& Object::operator=(const Object& obj)
{
	this->~Object();
	switch (obj.type)
	{
	case OT_ARRAY:
		{
			val.arr = TT_NEW(Array)(false);
			*val.arr = *obj.val.arr;
		}
		break;
	case OT_STRING:
		{
			val.str.size = obj.val.str.size;
			val.str.cont = Tools::cloneString(obj.val.str.cont, val.str.size);
		}
		break;
	default:
		val = obj.val;
		break;
	}

	type = obj.type;
	return *this;
}

ObjectPtr::ObjectPtr(Object* obj ):
	mInst(obj), mAutoDel(false), mCount(0)
{
	if (!obj)
	{
		mInst = TT_NEW(Object)();
		mAutoDel = true;
		mCount = TT_NEW(size_t)(1);
	}
}
	
ObjectPtr::ObjectPtr(const Object& obj)
{
	mAutoDel = true;
	mInst = TT_NEW(Object)();
	*mInst = obj;
	mCount = TT_NEW(size_t)(1);
}

ObjectPtr::ObjectPtr(const ObjectPtr& obj)
{
	mAutoDel = obj.mAutoDel;
	mInst = obj.mInst;
	mCount = obj.mCount;
	if (obj.mAutoDel)
		++(*mCount);
}

Object& ObjectPtr::operator*() const
{
	return *mInst;
}

Object* ObjectPtr::operator->()const
{
	return mInst;
}


ObjectPtr::~ObjectPtr()
{
	if (mAutoDel )
	{
		if (*mCount == 1)
		{
			TT_DELETE(Object, mInst);
			TT_FREE(mCount);
		}
		else 
			--(*mCount);
	}
}

Array::Array(bool hash, size_t cap)
{
	mHash = hash;
	mLast = mHead = (Elem*)TT_MALLOC(cap * sizeof(Elem));
	mTail = mHead + cap;
	memset(mHead, 0, cap * sizeof(Elem));
}

Array::~Array()
{
	for (Elem* i = mHead; i < mTail; ++i)
	{
		if (i->key == 0) continue;
		TT_FREE(i->key);
		i->obj.~Object();
	}
	TT_FREE(mHead);
}

Object* Array::operator[](size_t index)
{
	if (!mHash)
	{
		if (!checkSize(index))
		{
			const int ARRAY_MAX_SIZE = 0xff;
			if (index < ARRAY_MAX_SIZE)
			{
				grow();
				return &mHead[index].obj;
			}
			else
				convertToHashMap();
		}
		else	
			return &mHead[index].obj;
	}

	Char key[IntkeyLength];
	convertKey(index, key);
	return operator[](key);
}

Object* Array::operator[](const Char* key)
{
	if (!mHash) convertToHashMap();

	size_t hashval = hash(key);
	Elem* elem = mHead + (hashval % (mTail - mHead));
	Elem* head = elem;
	while(elem->key != 0 )
	{
		if (comp(elem->key, key) || comp(key, elem->key))
		{
			++elem;
			if (elem == mTail) elem = mHead;
			if (elem == head) 
			{
				grow();
				return operator[](key);
			}
		}
		else//equal
			return &elem->obj;
	}	
	
	elem->key = Tools::cloneString(key);
	return &elem->obj;
}

Object* Array::get(size_t index) const
{
	if (!mHash)
	{
		if (checkSize(index))
			return &mHead[index].obj;
		else
			return 0;
		
	}

	Char key[IntkeyLength];
	convertKey(index, key);
	return get(key);
}

Object* Array::get(const Char* key) const
{
	if (!mHash) return 0;

	size_t hashval = hash(key);
	Elem* elem = &mHead[hashval % (mTail - mHead)];
	Elem* head = elem;
	while(elem->key != 0 )
	{
		if (comp(elem->key, key) || comp(key, elem->key))
		{
			++elem;
			if (elem == mTail) elem = mHead;
			if (elem == head) 
			{
				return 0;
			}
		}
		else
			return &elem->obj;
	}	
	
	return 0;
}

Array& Array::operator=(const Array& arr)
{
	Array tmp(arr.mHash, arr.mTail - arr.mHead);
	Elem* begt = tmp.mHead;
	const Elem* bega = arr.mHead;
	for (; begt < tmp.mTail; ++begt, ++bega)
	{
		if (!bega->key) continue;
		begt->obj = bega->obj;
		begt->key = Tools::cloneString(bega->key);
		
	}
	swap(tmp);
	return *this;
}


void Array::grow()
{
	size_t size = (mTail - mHead);
	size_t newcap = size << 1;
	if (mHash)
	{
		Array tmp(true, newcap);
		Elem* head = mHead;
		for (; head != mTail; ++head)
			if (head->key)
				*tmp[head->key] = head->obj;
		//置空指针
		memset(mHead, 0, sizeof(Elem) * size);
		swap(tmp);
	}
	else
	{
		mHead = (Elem*)TT_REALLOC(mHead, newcap * sizeof(Elem));
		mTail = mHead + newcap;
		memset(mHead + size, 0, (newcap - size) * sizeof(Elem));
	}

}


void Array::swap(Array& arr)
{
	std::swap(mHead, arr.mHead);
	std::swap(mTail, arr.mTail);
	std::swap(mLast, arr.mLast);
	std::swap(mHash, arr.mHash);
}

bool Array::comp(const Char* s1, const Char* s2)const
{
	return Tools::less(s1, s2);
}

size_t Array::convertKey(size_t i, Char* key)const
{
	int length = 0;
	size_t num = i;
	do
	{
		++length;
		num /= 10;
	}
	while( num != 0);
	if (key == 0) return length;
	num = i;
	for (int i = length - 1; i >= 0; --i)
	{
		key[i] = (num % 10) + '0';
		num /= 10;
	}
	key[length] = 0;
	return length;
}

bool Array::checkSize(size_t size)const
{
	return size < (mTail - mHead);
}

void Array::convertToHashMap()
{
	size_t size = mTail - mHead;
	Array tmp(true, size);
	Elem* head = mHead;

	const int defaultSize = 32 * sizeof(Char);
	size_t len = defaultSize;
	Char* key = (Char*)TT_MALLOC(len * sizeof(Char));

	for (size_t i = 0; head != mTail; ++head, ++i)
	{
		if (head->obj.type == OT_NULL) continue;
		size_t klen = convertKey(i, 0) + 1;
		if (klen > len)
		{
			len = klen;
			TT_FREE(key);
			key = (Char*)TT_MALLOC(len * sizeof(Char));
		}
		
		convertKey(i, key);
		*tmp[key] = head->obj;
	}

	TT_FREE(key);

	memset(mHead, 0, sizeof(Elem) * size);
	swap(tmp);
	mHash = true;
}

size_t Array::hash(const Char* key)const
{
	size_t nHash = 0;
	while (*key)
		nHash = (nHash << 5) + nHash + *key++;
	return nHash;

}

ObjectVector::ObjectVector()
{
	mLast = mHead = (Object*)TT_MALLOC( OBJECT_STACK_SIZE * sizeof(Object));
	mTail = mHead + OBJECT_STACK_SIZE;
}

ObjectVector::~ObjectVector()
{
	TT_DELETE_ARRAY(Object, mHead, mLast - mHead);
}

Object* ObjectVector::operator[](size_t index)
{
	if (index >= (mLast - mHead))	 assert(0);
	return &mHead[index];
}

Object* ObjectVector::begin()
{
	return mHead;
}

Object* ObjectVector::back()
{
	return mLast - 1;
}

Object* ObjectVector::end()
{
	return mLast;
}

Object* ObjectVector::push()
{
	if (mLast == mTail) assert(0 && "not enough obj");//这里直接返回的obj指针，所以不能随便修改容器大小
	new (mLast) Object();
	++mLast;
	return mLast - 1;
}

void ObjectVector::pop()
{
	assert(mHead <mLast);
	--mLast;
	mLast->~Object();
}

bool ObjectVector::empty()const
{
	return mLast == mHead;
}

void ObjectVector::resize(size_t size)
{
	if ( (mLast - mHead) >= size) return;
	if ( (mTail - mHead) < size) reserve(size * 2);

	size_t init = size - (mLast - mHead);
	for (size_t i = 0; i < size; ++i)
	{
		new (mLast++) Object();
	}
}

void ObjectVector::reserve(size_t size)
{
	size_t cap = mTail - mHead;
	if (size <= cap) return;
	size_t osize = mLast - mHead;
	mHead = (Object*)TT_REALLOC(mHead, size * sizeof(Object));
	mLast = mHead + osize;
	mTail = mHead + size;
}

ObjectSet::~ObjectSet()
{
	//assert(mObjs.empty());
	std::for_each(mObjs.begin(), mObjs.end(), [](Object* obj){TT_DELETE(Object, obj);});
}


void ObjectSet::erase(Object* obj)
{
	auto ret = mObjs.find(obj);
	if (ret == mObjs.end()) return;
	TT_DELETE(Object, obj);
	mObjs.erase(ret);
}

Object* ObjectSet::add()
{
	Object* obj = TT_NEW(Object);
	mObjs.insert(obj);
	return obj;
}

bool ObjectSet::empty()const
{
	return mObjs.empty();
}

