#include "TTObject.h"
#include "TTMemoryAllocator.h"

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
	data = (Char*)TT_MALLOC(numChar * sizeof(Char));
	memcpy(data, sv.cont, numChar* sizeof(Char));
}

TTString::TTString(const ConstString& cs)
{
	numChar = cs.size ;
	data = (Char*)TT_MALLOC(numChar * sizeof(Char));
	memcpy(data, cs.cont, numChar* sizeof(Char));
}

TTString::TTString(const Char* str)
{
	size_t len = 1;
	const Char* h = str;
	while(*h++ != 0) ++len;
	numChar = len ;
	data = (Char*)TT_MALLOC(numChar * sizeof(Char));
	memcpy(data, str, numChar* sizeof(Char));
}

TTString::TTString(int i)
{
	int length = 0;
	size_t num = i;
	do
	{
		++length;
		num /= 10;
	}
	while( num != 0);
	
	numChar = length + 1;
	data = (Char*)TT_MALLOC(numChar * sizeof(Char));

	num = i;
	for (int i = length - 1; i >= 0; --i)
	{
		data[i] = (num % 10) + '0';
		num /= 10;
	}
	data[length] = 0;
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
	return compareString(data, str);
}

bool TTString::operator==(const TTString& str)
{
	return compareString(data, str.data);
}

TTString::operator bool()const
{
	return compareString(data, L"true");
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


Object::Object():type(OT_NULL), isConst(false)
{

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
			val.str.cont = (Char*)TT_MALLOC(obj.val.str.size * sizeof(Char));
			val.str.size = obj.val.str.size;
			memcpy(val.str.cont, obj.val.str.cont, sizeof(Char) * obj.val.str.size);
		}
		break;
	case OT_CONST_STRING:
		{
			val.str.cont = (Char*)TT_MALLOC(obj.val.cstr.size * sizeof(Char));
			val.str.size = obj.val.cstr.size;	
			memcpy(val.str.cont, obj.val.cstr.cont, sizeof(Char) * obj.val.cstr.size);
			type = OT_STRING;
			return *this;
		}
		break;
	default:
		val = obj.val;
		break;
	}

	type = obj.type;
	return *this;
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
		if (checkSize(index))
			return &mHead[index].obj;
		else
			convertToHashMap();
		
	}

	Char key[IntkeyLength];
	convertKey(index, key);
	return operator[](key);
}

Object* Array::operator[](const Char* key)
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
			return &elem->obj;
	}	
	
	size_t len = 0;
	const Char* str = key;
	while (*str++ != 0) ++len;
	elem->key = (Char*)TT_MALLOC((len + 1)* sizeof(Char) );
	memcpy(elem->key, key, sizeof(Char) * (len + 1));
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
	
	return &elem->obj;
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
				*tmp[head->key] = head->obj;
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
	return compareString(s1, s2);
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

void Caster::cast(Object& o, ObjectType otype)
{
	assert(o.isConst == false);
	switch (otype)
	{
	case OT_NULL:
		castToNullObject(o); break;
	case OT_TRUE:
		castToBoolObject(o); break;
	case OT_FALSE:
		castToBoolObject(o); break;
	case OT_STRING:
		castToStringObject(o); break;
	case OT_DOUBLE:
		castToRealObject(o); break;
	case OT_INTEGER:
		castToIntObject(o); break;
	case OT_FUNCTION:
		castToFunctionObject(o); break;
	case OT_FIELD:
		castToFieldObject(o); break;
	case OT_ARRAY:
		castToArrayObject(o); break;
	default:
		assert(0 && "unexpected type");
		castToNullObject(o);
	}
}

void Caster::castToNullObject(Object& o)
{
	o.~Object();
	o.type = OT_NULL;
}

void Caster::castToBoolObject(Object& o)
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
}

void Caster::castToStringObject(Object& o)
{
	TTString* str;
	switch (o.type)
	{
	case OT_STRING:
	case OT_NULL:
		return;
	case OT_TRUE:
		str = TT_NEW(TTString)(L"true");
		break;
	case OT_FALSE:
		str = TT_NEW(TTString)(L"false");
		break;	
	case OT_DOUBLE:
		str = TT_NEW(TTString)(o.val.d);
		break;	
	case OT_INTEGER:
		str = TT_NEW(TTString)(o.val.i);
		break;
	case OT_CONST_STRING:
		str = TT_NEW(TTString)(o.val.cstr);
		break;
	case OT_FUNCTION:
	case OT_FIELD:
	case OT_ARRAY:
		{
			o.~Object();
			o.type = OT_NULL;
			return ;
		}

	}
	o.val.str.cont = str->data;
	o.val.str.size = str->numChar;
	o.type = OT_STRING;
}

void Caster::castToRealObject(Object& o)
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
		o.val.d = *((TTString*)&o.val.str);
		break;
	case OT_CONST_STRING:
		o.val.d = *((TTString*)&o.val.cstr);
		break;
	default:
		o.~Object();
		o.type = OT_NULL;
		return;
	}
	o.type = OT_DOUBLE;
}

void Caster::castToIntObject(Object& o)
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
		o.val.i = *((TTString*)&o.val.str);
		break;
	case OT_CONST_STRING:
		o.val.i = *((TTString*)&o.val.cstr);
		break;
	default:
		o.~Object();
		o.type = OT_NULL;
		return;
	}
	o.type = OT_INTEGER;
}

void Caster::castToFunctionObject(Object& o)
{
}

void Caster::castToFieldObject(Object& o)
{
}

void Caster::castToArrayObject(Object& o)
{
	if (o.type == OT_ARRAY) return;

	Array* arr = TT_NEW(Array)(false);
	*(*arr)[(size_t)0] = o;
	o.~Object();
	o.type = OT_ARRAY;
	o.val.arr = arr;
}



bool Caster::castToBool(const Object& o)
{
	switch (o.type)
	{
	case OT_INTEGER:
		return o.val.i != 0;
	}
	return o.type == OT_TRUE;
}

SharedPtr<TTString> Caster::castToString(const Object& o)
{
	TTString* str;
	switch (o.type)
	{
	case OT_NULL:	
	case OT_FUNCTION:
	case OT_FIELD:
		str = new TTString(L"null");
		break;
	case OT_ARRAY:
		return castToString(*o.val.arr->get((size_t)0));
	case OT_STRING:
		str = new TTString(o.val.str);
		break;
	case OT_CONST_STRING:
		str = new TTString(o.val.cstr);
		break;
	case OT_TRUE:
		str = new TTString (L"true");
		break;
	case OT_FALSE:
		str = new TTString (L"false");
		break;	
	case OT_DOUBLE:
		str = new TTString (o.val.d);
		break;	
	case OT_INTEGER:
		str = new TTString (o.val.i);
		break;
	}
	return str;
}

double Caster::castToReal(const Object& o)
{
	return cast<double>(o);
}

int Caster::castToInt(const Object& o)
{
	return cast<int>(o);
}

template<class Type>
Type Caster::cast(const Object& o)
{
	switch(o.type)
	{
	case OT_STRING:
		return *((TTString*)&o.val.str);
	case OT_CONST_STRING:
		{
			StringValue sv;
			sv.cont = (Char*)o.val.cstr.cont;
			sv.size = o.val.cstr.size;
			return *((TTString*)&sv);
		}
	case OT_DOUBLE:
		return (Type)o.val.d;
	case OT_INTEGER:
		return (Type)o.val.i;
	case OT_TRUE:
		return (Type)1;
	case OT_FALSE:
		return (Type)0;
	default:
		//TTINTERPRETER_EXCEPT("cant cast");
		return (Type)0;
	}
}	