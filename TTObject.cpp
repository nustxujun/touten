#include "TTObject.h"
#include "TTMemoryAllocator.h"

using namespace TT;

Object::Object():type(OT_NULL)
{

}

Object::~Object()
{
	if (type == OT_ARRAY)
		TT_DELETE(Array, val.arr);
}

Object& Object::operator=(const Object& obj)
{
	val = obj.val;
	if (type == OT_ARRAY)
	{
		val.arr = TT_NEW(Array)(false);
		*val.arr = *obj.val.arr;
	}
	type = obj.type;
	return *this;
}

Object Array::NullObject;

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

Object& Array::operator[](size_t index)
{
	if (!mHash)
	{
		if (checkSize(index))
			return mHead[index].obj;
		else
			convertToHashMap();
		
	}

	Char key[IntkeyLength];
	convertKey(index, key);
	return operator[](key);
}

Object& Array::operator[](const Char* key)
{
	if (!mHash) convertToHashMap();

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
				grow();
				return operator[](key);
			}
		}
		else//equal
			return elem->obj;
	}	
	
	size_t len = 0;
	const Char* str = key;
	while (*str++ != 0) ++len;
	elem->key = (Char*)TT_MALLOC((len + 1)* sizeof(Char) );
	memcpy(elem->key, key, sizeof(Char) * (len + 1));
	return elem->obj;
}

Object& Array::get(size_t index) 
{
	if (!mHash)
	{
		if (checkSize(index))
			return mHead[index].obj;
		else
			return NullObject;
		
	}

	Char key[IntkeyLength];
	convertKey(index, key);
	return get(key);
}

Object& Array::get(const Char* key) 
{
	if (!mHash) return NullObject;

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
				return NullObject;
			}
		}
		else
			return elem->obj;
	}	
	
	return elem->obj;
}

Array& Array::operator=(const Array& arr)
{
	Array tmp(arr.mHash, arr.mTail - arr.mHead);
	Elem* begt = tmp.mHead;
	const Elem* bega = arr.mHead;
	while (begt < tmp.mTail)
	{
		begt->obj = bega->obj;
		++begt; ++bega;
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
				tmp[head->key] = head->obj;
		//置空指针
		memset(mHead, 0, sizeof(Elem) * size);
		swap(tmp);
	}
	else
	{
		mHead = (Elem*)TT_REALLOC(mHead, newcap * sizeof(Elem));
		mTail = mHead + newcap;
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
	while(*s1 != 0 && *s2 != 0)
	{
		if (*s1 < *s2) return true;
		if (*s2 < *s1) return false;
		++s1; ++s2;
	}
	return *s1 < *s2;
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
		tmp[key] = head->obj;
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


Object* ObjectStack::top()
{
	return mContainer.back();
}

Object* ObjectStack::push()
{
	return mContainer.push();
}

void ObjectStack::pop()
{
	mContainer.pop();
}

bool ObjectStack::empty()const
{
	return mContainer.empty();
}

void ObjectStack::reserve(size_t size)
{
	mContainer.reserve(size);
}


Object Caster::cast(bool val)
{
	return Object();
}

Object Caster::cast(TTString str)
{
	return Object();
}

Object Caster::cast(double val)
{
	return Object();
}

Object Caster::cast(int val)
{
	return Object();
}


void Caster::cast(Object& o, ObjectType otype)
{
	switch (otype)
	{
	case OT_NULL:
		castToNull(o); break;
	case OT_TRUE:
		castToBool(o); break;
	case OT_FALSE:
		castToBool(o); break;
	case OT_STRING:
		castToString(o); break;
	case OT_DOUBLE:
		castToReal(o); break;
	case OT_INTEGER:
		castToInt(o); break;
	case OT_FUNCTION:
		castToFunction(o); break;
	case OT_FIELD:
		castToField(o); break;
	case OT_ARRAY:
		castToArray(o); break;
	default:
		assert(0 && "unexpected type");
		castToNull(o);
	}
}

int Caster::castToNull(Object& o)
{
	o.~Object();
	o.type = OT_NULL;
	return 0;
}

bool Caster::castToBool(Object& o)
{
	switch (o.type)
	{
	case OT_INTEGER: case OT_DOUBLE: 
		o.type = (o.val.i == 0) ? OT_FALSE: OT_TRUE;
		break;
	case OT_TRUE: case OT_FALSE:
		break;
	default:
		o.~Object(); o.type = OT_NULL;
		break;
	}
	return o.type == OT_TRUE;
}

const Char* Caster::castToString(Object& o)
{
	return o.val.str.cont;
}

double Caster::castToReal(Object& o)
{
	switch (o.type)
	{
	case OT_INTEGER:
		o.val.d = o.val.i;
		break;
	case OT_DOUBLE:
		break;
	case OT_TRUE:
		o.val.d = 1;
		break;
	case OT_FALSE:
		o.val.d = 0;
		break;
	case OT_STRING:
		o.val.d = (TTString)o.val.str;
		break;
	default:
		o.~Object();
		o.type = OT_NULL;
		return 0;
	}
	o.type = OT_DOUBLE;
	return o.val.d;
}

int Caster::castToInt(Object& o)
{
	switch (o.type)
	{
	case OT_INTEGER:
		break;
	case OT_DOUBLE:
		o.val.i = (int)o.val.d;
		break;
	case OT_TRUE:
		o.val.i = 1;
		break;
	case OT_FALSE:
		o.val.i = 0;
		break;
	case OT_STRING:
		o.val.i = (TTString)o.val.str;
		break;
	default:
		o.~Object();
		o.type = OT_NULL;
		return 0;
	}
	o.type = OT_INTEGER;
	return o.val.i;
}

int Caster::castToFunction(Object& o)
{
	return o.val.func.codeAddr;
}

int Caster::castToField(Object& o)
{
	return o.val.func.codeAddr;
}

int Caster::castToArray(Object& o)
{
	if (o.type == OT_ARRAY) return 0;

	Array* arr = TT_NEW(Array)(false);
	(*arr)[(size_t)0] = o;
	o.~Object();
	o.type = OT_ARRAY;
	o.val.arr = arr;
	return 0;
}


