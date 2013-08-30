#ifndef _TTConstantPool_H_
#define _TTConstantPool_H_

#include "TTType.h"

namespace TT
{
	class ConstantPool
	{
	public :
		ConstantPool(size_t cap);
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

	private:
		void reserve(size_t size);

	private:
		char* mData;
		char* mLast;
		char* mTail;

	};
}

#endif