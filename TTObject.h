#ifndef _TTObject_H_
#define _TTObject_H_

#include "ToutenCommon.h"

#define OBJECT_STACK_SIZE 1

namespace TT
{
	enum ObjectType
	{
		OT_NULL,
		OT_TRUE,
		OT_FALSE,
		OT_BOOL,//×ª»»Ê±ºòÓÃ
		OT_STRING,		
		OT_DOUBLE,
		OT_INTEGER,
		OT_FUNCTION,
		OT_FIELD,
		OT_ARRAY,

		OT_NUM
	};

	struct FunctionValue
	{
		size_t codeAddr;
		size_t paraCount;
	};

	struct StringValue
	{
		const Char* cont;
		size_t size;	
	};

	class TTString
	{
	public :
		TTString(size_t count);
		TTString(const StringValue& sv);
		TTString(int i);
		TTString(double d);
		~TTString();
		TTString operator+(const TTString& str);


		operator bool()const;
		operator int()const;
		operator double()const;

	private:
		Char* data;
		size_t numChar;

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
		Value val;
		ObjectType type;
		Object();
		~Object();
		Object& operator=(const Object& obj);
	};

	class Array
	{
		static const size_t ArrayLimit = 0xff;
		static const size_t IntkeyLength = 0xff;
		static Object NullObject;
		struct Elem
		{
			Object obj;
			Char* key;
		};
	public :
		Array(bool hash, size_t cap = 8);
		~Array();

		Object& operator[](size_t index);
		Object& operator[](const Char* key);

		Object& get(size_t index) ;
		Object& get(const Char* key) ;

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

	class ObjectStack
	{
	public :
		Object* operator[](size_t index);
		Object* top();
		Object* push();
		void pop();
		bool empty()const;
		void reserve(size_t size);
	private:
		ObjectVector mContainer;
	};


	class Caster
	{
	public:
		Object cast(bool val);
		Object cast(TTString str);
		Object cast(double val);
		Object cast(int val);
		
		void cast(Object& o, ObjectType otype);
		
		int castToNull(Object& o);
		bool castToBool(Object& o);
		const Char* castToString(Object& o);
		double castToReal(Object& o);
		int castToInt(Object& o);
		int castToFunction(Object& o);
		int castToField(Object& o);
		int castToArray(Object& o);
	};

	typedef int (*TT_Function)(int argnum, Object* args, Object* ret);

}

#endif