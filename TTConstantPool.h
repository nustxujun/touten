#ifndef _TTConstantPool_H_
#define _TTConstantPool_H_

#include "TTType.h"

namespace TT
{
	class ConstantPool
	{
	public :
		ConstantPool(size_t cap = 1024);
		~ConstantPool();

		size_t write(const void* buff, size_t size);
		void* get(size_t offset)const;

		template<class T>
		size_t operator<<(const T& val)
		{
			return write(&val, sizeof(T));
		}


		void* operator[](size_t offset)const
		{
			return get(offset);
		}

		size_t size()const;
		const void* data()const;

		void reserve(size_t size);
		void clear();

		void reset(int val);

	private:
		char* mData;
		char* mLast;
		char* mTail;

	};
}

#endif