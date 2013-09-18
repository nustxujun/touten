#ifndef _CPPFunctionTable_H_
#define _CPPFunctionTable_H_

#include "ToutenCommon.h"
#include "TTConstantPool.h"

#define MAX_KEY_LEN 256
#define DEFAULT_NUM_FUNC 8

namespace TT
{
	class CPPFunctionTable
	{
	public :
		CPPFunctionTable(size_t size = DEFAULT_NUM_FUNC);
		CPPFunctionTable(const void* buff, size_t size);
		~CPPFunctionTable();

		size_t insert(const Char* name, TT_Function func);
		Object* operator[](size_t index)const;
		const void* data()const;
	private:
		size_t hash(const Char* key)const;
		void grow();
		bool checkSize(size_t size)const;
		bool comp(const Char* s1, const Char* s2)const ;

	private:
		ConstantPool mConstPool;
		size_t mSize;
		struct Elem
		{
			Object obj;
			Char key[MAX_KEY_LEN];
		};

	};
}

#endif