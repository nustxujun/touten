#ifndef _TTObject_H_
#define _TTObject_H_

#include "ToutenCommon.h"
#include "SharedPtr.h"
#include <functional>

#define OBJECT_STACK_SIZE 1

namespace TT
{
	enum ObjectType
	{
		OT_NULL,
		OT_TRUE,
		OT_FALSE,
		OT_STRING,		
		OT_DOUBLE,
		OT_INTEGER,
		OT_FUNCTION,
		//OT_FIELD,
		OT_ARRAY,

		OT_NUM
	};

	struct FunctionValue
	{
		enum
		{
			PARA_COUNT = 0xffff,

			IS_VARIADIC = 0x20000000,
			IS_CPP_FUNC = 0x40000000,
			NEED_RETURN = 0x80000000,
		};

		void* codeAddr;
		size_t funcinfo;
	};

	struct StringValue
	{
		size_t size;	
		Char* cont;
	};

	class ToutenExport TTString
	{
	public :
		TTString(size_t count);
		TTString(const StringValue& sv);
		TTString(const Char* str);
		TTString(int i);
		TTString(double d);
		~TTString();
		bool operator==(const Char* str);
		bool operator==(const TTString& str);

		operator bool()const;
		operator int()const;
		operator double()const;
		operator const Char*()const;

		size_t numChar;
		Char* data;
	};


	class Array;
	struct Value
	{
		ObjectType type;
		union
		{
			int i;
			double d;

			StringValue str;
			FunctionValue func;

			Array* arr;
		} ;
	};


	class ToutenExport ValuePtr
	{
	public:
		ValuePtr(const ValuePtr& ap);
		ValuePtr(ObjectType ot);
		~ValuePtr();
		void operator=(const ValuePtr& ap);

		Value& operator*()const;
		Value* operator->()const;

		void releaseVal();
		void copy(const ValuePtr& val);
	private:
		void setNull();
		bool isNull()const;

	private:
		Value* mVal;
		size_t* mCount;
	};

	class ToutenExport Object
	{
	public:
		//ObjectType type;
		ValuePtr val;

		Object();
		Object(ObjectType ot); 
		Object(const Object& obj);
		Object(bool v);
		Object(int v);
		Object(double v);
		Object(const Char* str);
		Object(const Char* str, size_t size);

		Object(FunctionValue v);

		~Object();
		Object& operator=(const Object& obj);

		void reference(const Object& obj);

		void swap(Object& obj);
	};

	class ToutenExport ObjectPtr
	{
	public :
		ObjectPtr();
		ObjectPtr(ObjectType ot);
		ObjectPtr(const Object& obj);
		ObjectPtr(const ObjectPtr& obj);

		~ObjectPtr();

		void operator=(const ObjectPtr& obj);
		Object& operator*() const;
		Object* operator->()const;

		void setNull();
		bool isNull() const;

		void swap(ObjectPtr& obj);
	private:
		Object* mInst;
		size_t* mCount;
	};

	class ToutenExport Array
	{
		static const size_t ArrayLimit = 0xff;
		static const size_t IntkeyLength = 0xff;
		struct Elem
		{
			ObjectPtr obj;
			Char* key;
		};
	public :
		Array(bool hash, size_t cap = 8);
		~Array();

		ObjectPtr& operator[](size_t index);
		ObjectPtr& operator[](const Char* key);

		ObjectPtr get(size_t index)const;
		ObjectPtr get(const Char* key)const;

		Array& operator=(const Array& arr);

	private:
		void swap(Array& arr);
		void grow();
		void convertToHashMap();
		bool checkSize(size_t size)const;

		size_t hash(const Char* key)const;

		bool comp(const Char* s1, const Char* s2) const;
	private:
		Elem* mHead;
		Elem* mTail;
		Elem* mLast;
		bool mHash;
	};

}

#endif