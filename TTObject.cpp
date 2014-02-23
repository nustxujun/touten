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



ValuePtr::ValuePtr(ObjectType ot)
{
	mVal = (Value*)TT_MALLOC(sizeof(Value));
	mCount = TT_NEW(size_t)(1);

	memset(mVal, 0, sizeof(Value));
	mVal->type = ot;
}

ValuePtr::ValuePtr(const ValuePtr& ap)
{
	assert(*(((const int*)ap.mVal) - 1) > 0);
	mVal = ap.mVal;
	mCount = ap.mCount;
	++(*mCount);

}

ValuePtr::~ValuePtr()
{
	setNull();
}

void ValuePtr::copy(const ValuePtr& val)
{
	releaseVal();
	switch (val->type)
	{
	case OT_ARRAY:
		{
			mVal->arr = TT_NEW(Array)(false);
			*mVal->arr = *val->arr;
		}
		break;
	case OT_STRING:
		{
			mVal->str.size = val->str.size;
			mVal->str.cont = Tools::cloneString(val->str.cont, mVal->str.size);
		}
		break;
	default:
		*mVal = *(val);
		break;
	}

	mVal->type = val->type;
}

void ValuePtr::releaseVal()
{
	switch (mVal->type)
	{
	case OT_ARRAY:
		if (mVal->arr != nullptr)
		TT_DELETE(Array, mVal->arr);
		break;
	case OT_STRING:
		if (mVal->str.cont != nullptr)
		TT_FREE(mVal->str.cont);
		break;
	}
	memset(mVal, 0, sizeof(mVal));
	mVal->type = OT_NULL;

}

void ValuePtr::setNull()
{
	if (isNull()) return;
	if (*mCount == 1)
	{
		releaseVal();
		TT_FREE(mVal);
		TT_FREE(mCount);
		mCount = nullptr;
	}
	else
		--(*mCount);
}

bool ValuePtr::isNull()const
{
	return (mCount == nullptr);
}

void ValuePtr::operator = (const ValuePtr& ap)
{
	setNull();
	mVal = ap.mVal;
	mCount = ap.mCount;
	++(*mCount);
}


Value& ValuePtr::operator*()const
{
	return *mVal;
}

Value* ValuePtr::operator->()const
{
	return mVal;
}


Object::Object() :
	val(OT_NULL)
{
}

Object::Object(ObjectType ot):
	val(ot)
{
}

Object::Object(const Object& obj):
	val(OT_NULL)
{
	*this = obj;
}

Object::Object(bool v):
	val(v ? OT_TRUE : OT_FALSE)
{
}

Object::Object(int v):
	val(OT_INTEGER)
{
	val->i = v;
}

Object::Object(double v):
	val(OT_DOUBLE)
{
	val->d = v;
}

Object::Object(const Char* str) : 
	val(OT_STRING)
{
	val->str.cont = Tools::cloneString(str, &val->str.size);
}

Object::Object(const Char* str, size_t size):
	val(OT_STRING)
{
	val->str.size = size;
	val->str.cont = Tools::cloneString(str, size);
}

Object::Object(const FunctionValue& v):
	val(OT_FUNCTION)
{
	val->func = v;
}

void Object::swap(Object& obj)
{
	std::swap(val, obj.val);
}


Object::~Object()
{
	//switch (type)
	//{
	//case OT_ARRAY:
	//	TT_DELETE(Array, val->arr);
	//	break;
	//case OT_STRING:
	//	TT_FREE(val.str.cont);
	//	break;
	//}
	//type = OT_NULL;
	//
}

Object& Object::operator=(const Object& obj)
{
	//this->~Object();
	//switch (obj.type)
	//{
	//case OT_ARRAY:
	//	{
	//		val.arr = TT_NEW(Array)(false);
	//		*val.arr = *obj.val.arr;
	//	}
	//	break;
	//case OT_STRING:
	//	{
	//		val.str.size = obj.val.str.size;
	//		val.str.cont = Tools::cloneString(obj.val.str.cont, val.str.size);
	//	}
	//	break;
	//default:
	//	*val = *(obj.val);
	//	break;
	//}

	//type = obj.type;

	val.copy(obj.val);
	return *this;
}

void Object::reference(const Object& obj)
{
	val = obj.val;
	//switch (obj.type)
	//{
	//case OT_ARRAY:
	//	{
	//		this->~Object();
	//		val.arr = TT_NEW(ArrayPtr)(*obj.val.arr);
	//		type = obj.type;
	//	}
	//	break;
	//default:
	//	*this = obj;
	//}
}


ObjectPtr::ObjectPtr()
{
	mInst = nullptr;
	mCount = nullptr;
}

ObjectPtr::ObjectPtr(ObjectType ot)
{
	mInst = TT_NEW(Object)(ot);
	mCount = TT_NEW(size_t)(1);
}

ObjectPtr::ObjectPtr(const Object& obj)
{
	mInst = TT_NEW(Object)();
	*mInst = obj;
	mCount = TT_NEW(size_t)(1);
}

ObjectPtr::ObjectPtr(const ObjectPtr& obj)
{
	mInst = obj.mInst;
	mCount = obj.mCount;
	if (mCount)
		++(*mCount);
}

void ObjectPtr::swap(ObjectPtr& obj)
{
	std::swap(mInst, obj.mInst);
	std::swap(mCount, obj.mCount);
}


void ObjectPtr::operator = (const ObjectPtr& obj)
{
	setNull();

	mInst = obj.mInst;
	mCount = obj.mCount;
	if (mCount)
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

void ObjectPtr::setNull()
{
	if (isNull()) return;
	if (*mCount == 1)
	{
		TT_DELETE(Object, mInst);
		TT_FREE(mCount);
		mCount = nullptr;
	}
	else
		--(*mCount);
}

bool ObjectPtr::isNull() const
{
	return mCount == nullptr;
}

ObjectPtr::~ObjectPtr()
{
	setNull();
}

Array::Array(bool hash, size_t cap)
{
	mHash = hash;
	mHead = (Elem*)TT_MALLOC(cap * sizeof(Elem));
	mTail = mHead + cap;
	mLast = nullptr;
	memset(mHead, 0, cap * sizeof(Elem));
}

Array::~Array()
{
	for (Elem* i = mHead; i < mTail; ++i)
	{
		if (i->key != 0)
			TT_FREE(i->key);
		i->obj.setNull();
	}
	TT_FREE(mHead);
}

ObjectPtr& Array::operator[](size_t index)
{
	if (!mHash)
	{
		const int ARRAY_MAX_SIZE = 0xff;
		if (index < ARRAY_MAX_SIZE)
		{
			if (!checkSize(index)) grow();
			if (mHead[index].obj.isNull())
			{
				for (size_t i = index; i >= 0; --i)
				{
					if (!mHead[i].obj.isNull()) break;
					mHead[i].obj = Object();
				}
			}
			return mHead[index].obj;
		}
		else convertToHashMap();
	}

	String key = Tools::toString(index);
	return operator[](key.c_str());
}

ObjectPtr& Array::operator[](const Char* key)
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
			return elem->obj;
	}	
	
	elem->key = Tools::cloneString(key);
	if (elem->obj.isNull())
	{
		elem->obj = Object();
		elem->next = mLast;
		mLast = elem;
	}
	return elem->obj;
}

ObjectPtr Array::get(size_t index) const
{
	if (!mHash)
	{
		if (checkSize(index))
		{
			return mHead[index].obj;
		}
		else
			return ObjectPtr();
		
	}

	String key = Tools::toString(index);
	return get(key.c_str());
}

ObjectPtr Array::get(const Char* key) const
{
	if (!mHash) return ObjectPtr();

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
				return ObjectPtr();
			}
		}
		else
			return elem->obj;
	}	
	
	return ObjectPtr();
}

Array& Array::operator=(const Array& arr)
{
	Array tmp(arr.mHash, arr.mTail - arr.mHead);

	if (arr.mHash)
	{
		for (auto i = arr.mLast; i != nullptr; i = i->next)
			tmp[i->key] = i->obj;
	}
	else
	{
		Elem* begt = tmp.mHead;
		const Elem* bega = arr.mHead;
		for (; begt < tmp.mTail; ++begt, ++bega)
		{
			if (bega->obj.isNull()) continue;
			begt->obj = bega->obj;
		}
	}

	//Elem* begt = tmp.mHead;
	//const Elem* bega = arr.mHead;
	//for (; begt < tmp.mTail; ++begt, ++bega)
	//{
	//	//if (!bega->key) continue;
	//	//Array被创建时 ，内存置了0，因此obj虽然不合法但是 isNull的返回一定正确
	//	//因为如果不是hash表的情况下 key是 NULL
	//	if (bega->obj.isNull()) continue;
	//	begt->obj = bega->obj;
	//	if (bega->key)
	//		begt->key = Tools::cloneString(bega->key);
	//	
	//}
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
		swap(tmp);
	}
	else
	{
		mHead = (Elem*)TT_REALLOC(mHead, newcap * sizeof(Elem));
		mTail = mHead + newcap;
		memset(mHead + size, 0, (newcap - size) * sizeof(Elem));
	}

}

Array::iterator Array::begin()const
{
	//图方便，遍历顺序和插入顺序相反
	return iterator(mLast);
}

Array::iterator Array::end()const
{
	return iterator(nullptr);
}

bool Array::isArray()const
{	
	return !mHash;
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

bool Array::checkSize(size_t size)const
{
	return size < (mTail - mHead);
}

void Array::convertToHashMap()
{
	size_t size = mTail - mHead;
	Array tmp(true, size);
	Elem* head = mHead;

	for (size_t i = 0; head != mTail; ++head, ++i)
	{
		if (head->obj.isNull() || head->obj->val->type == OT_NULL) continue;
		String key = Tools::toString(i);

		tmp[key.c_str()] = head->obj;
	}

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

//ArrayPtr::ArrayPtr(bool bhash)
//{
//	mArray = TT_NEW(Array)(bhash);
//	mCount = TT_NEW(size_t)(1);
//}
//
//ArrayPtr::ArrayPtr(const ArrayPtr& ap)
//{
//	mArray = ap.mArray;
//	mCount = ap.mCount;
//	++(*mCount);
//
//}
//
//ArrayPtr::~ArrayPtr()
//{
//	setNull();
//}
//
//ObjectPtr ArrayPtr::operator[](size_t index)
//{
//	return (*mArray)[index];
//}
//
//ObjectPtr ArrayPtr::operator[](const Char* key)
//{
//	return (*mArray)[key];
//}
//
//ObjectPtr ArrayPtr::get(size_t index)const
//{
//	return mArray->get(index);
//}
//
//ObjectPtr ArrayPtr::get(const Char* key)const
//{
//	return mArray->get(key);
//}
//
//void ArrayPtr::setNull()
//{
//	if (isNull()) return;
//	if (*mCount == 1)
//	{
//		TT_DELETE(Array, mArray);
//		TT_FREE(mCount);
//		mCount = nullptr;
//	}
//	else
//		--(*mCount);
//}
//
//bool ArrayPtr::isNull()const
//{
//	return (mCount == nullptr);
//}
//
//void ArrayPtr::operator = (const ArrayPtr& ap)
//{
//	setNull();
//	mArray = ap.mArray;
//	mCount = ap.mCount;
//	++(*mCount);
//}
//
//void ArrayPtr::copy(const ArrayPtr& ap)
//{
//	*mArray = *ap.mArray;
//}
