#ifndef _TTObject_H_
#define _TTObject_H_

#include "ToutenCommon.h"
#include "SharedPtr.h"

#define OBJECT_STACK_SIZE 1

namespace TT
{
	enum ObjectType
	{
		OT_NULL,
		OT_TRUE,
		OT_FALSE,
		OT_STRING,		
		OT_CONST_STRING,
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
		size_t size;	
		Char* cont;
	};

	struct ConstString
	{
		size_t size;
		Char cont[2];
	};

	class TTString
	{
	public :
		TTString(size_t count);
		TTString(const StringValue& sv);
		TTString(const ConstString& cs);
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
		ConstString cstr;
		FunctionValue func;

		Array* arr;
	};

	struct Object
	{
		ObjectType type;
		bool isConst;
		Value val;//考虑const string的问题，value一定放最后

		Object();
		~Object();
		Object& operator=(const Object& obj);
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


	class Caster
	{
	public:
		void cast(Object& o, ObjectType otype);


		bool castToBool(const Object& o);
		SharedPtr<TTString> castToString(const Object& o);
		double castToReal(const Object& o);
		int castToInt(const Object& o);

	private:
		void castToNullObject(Object& o);
		void castToBoolObject(Object& o);
		void castToStringObject(Object& o);
		void castToRealObject(Object& o);
		void castToIntObject(Object& o);
		void castToFunctionObject(Object& o);
		void castToFieldObject(Object& o);
		void castToArrayObject(Object& o);

		template<class Type>
		Type cast(const Object& o);

	};

	typedef int (*TT_Function)(const std::vector<Object*>& paras, Object* ret);

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