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
		void* codeAddr;
		size_t paraCount;
	};

	struct StringValue
	{
		size_t size;	
		Char* cont;
	};

	class TTString
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
	union Value
	{
		int i;
		double d;

		StringValue str;
		FunctionValue func;

		Array* arr;
	};

	struct Object
	{
		ObjectType type;
		Value val;

		Object();
		Object(const Object& obj);
		Object(bool v);
		Object(int v);
		Object(double v);
		Object(const Char* str, size_t size);
		Object(FunctionValue v);

		~Object();
		Object& operator=(const Object& obj);

		void swap(Object& obj);
	};

	class ObjectPtr
	{
	public :
		ObjectPtr(Object* obj = 0);
		ObjectPtr(const Object& obj);
		Object& operator*() const;
		Object* operator->()const;
		
		~ObjectPtr();
		ObjectPtr(const ObjectPtr& obj);

	private:
		void operator=(const ObjectPtr&);
	private:
		Object* mInst;
		bool mAutoDel;
		size_t* mCount;
	};

	class ObjectVector
	{
	public :
		ObjectVector();
		~ObjectVector();
		Object* operator[](size_t index);
		Object* begin();
		Object* back();
		Object* end();

		Object* push();
		void pop();
		bool empty()const;

		void resize(size_t size);
		void reserve(size_t size);
	private:
		Object* mHead;
		Object* mTail;
		Object* mLast;		
	};

	class Array
	{
		static const size_t ArrayLimit = 0xff;
		static const size_t IntkeyLength = 0xff;
		struct Elem
		{
			Object obj;
			Char* key;
		};
	public :
		Array(bool hash, size_t cap = 8);
		~Array();

		Object* operator[](size_t index);
		Object* operator[](const Char* key);

		Object* get(size_t index)const ;
		Object* get(const Char* key)const ;

		Array& operator=(const Array& arr);

	private:
		void swap(Array& arr);
		void grow();
		void convertToHashMap();
		size_t convertKey(size_t i, Char* key)const;
		bool checkSize(size_t size)const;

		size_t hash(const Char* key)const;

		bool comp(const Char* s1, const Char* s2) const;
	private:
		Elem* mHead;
		Elem* mTail;
		Elem* mLast;
		bool mHash;
		
	};

	class ObjectSet
	{
	public :
		~ObjectSet();
		Object* add();
		void erase(Object* obj);
		bool empty()const;
	private:
		std::set<Object*> mObjs;
	};


	class Functor
	{
	public :
		virtual void operator()(const ObjectPtr* paras, int paracount, Object* ret) = 0;
	};

	//typedef int (*TT_Function)(const ObjectPtr* paras, int paracount, Object* ret);

	static bool compareString(const Char* s1, const Char* s2)
	{
		while(*s1 != 0 && *s2 != 0)
		{
			if (*s1 < *s2) return true;
			if (*s2 < *s1) return false;
			++s1; ++s2;
		}
		return *s1 < *s2;
	}
}

#endif