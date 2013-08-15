#ifndef _TTConstantPool_H_
#define _TTConstantPool_H_

#include "TTType.h"

namespace TT
{
	class ConstantPool
	{
	public :
		static const size_t Null = 0;
		static const size_t True = 4;
		static const size_t False = 8;
	public :
		ConstantPool(size_t cap);
		~ConstantPool();

		size_t add(const void* buff, size_t size);
		void* get(size_t offset);

		template<class T>
		size_t add(ConstantType ct, const T& obj)
		{
			size_t bg = add(&ct, sizeof(ConstantType)); 
			add(&obj, sizeof(T)); 
			return bg;
		}

		void* operator[](size_t offset)
		{
			return get(offset);
		}

		size_t size()const;
		const void* data()const;

	private:
		void reserve(size_t size);

	private:
		char* mData;
		char* mLast;
		char* mTail;

	};
}

#endif