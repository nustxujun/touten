#ifndef _TTMemoryAllocator_H_
#define _TTMemoryAllocator_H_

#include "TTPlatform.h"

namespace TT
{
	class ToutenExport MemoryAllocator
	{
	public:
		//optr : 原内存	nsize : 新大小
		typedef void* (*AllocMethod) (void* optr, size_t nsize);

	public :
		static void setupMethod(AllocMethod method );

		static void* alloc(size_t size);
		static void* realloc(void* ptr, size_t nsize);
		static void free(void* ptr);

		static void* defaultAlloc(void* ptr, size_t size);
	private:
		static AllocMethod allocMethod;
	};

}

#define TT_MALLOC(size) MemoryAllocator::alloc(size)

#define TT_REALLOC(ptr, size) MemoryAllocator::realloc(ptr, size)

#define TT_FREE(ptr) MemoryAllocator::free(ptr)

#define TT_NEW(T) new (TT_MALLOC(sizeof(T))) T

#define TT_NEW_ARRAY(T, count) new (TT_MALLOC(sizeof(T) * (count))) T[count]

#define TT_DELETE(T, ptr) {(ptr)->~T(); TT_FREE(ptr);}

#define TT_DELETE_ARRAY(T,ptr, count) {T* tmp = (ptr); for (int i = 0; i < count; ++i, ++tmp) tmp->~T(); TT_FREE(ptr);}



#endif