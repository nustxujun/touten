#include "TTMemoryAllocator.h"
#include <malloc.h>

using namespace TT;
MemoryAllocator::AllocMethod MemoryAllocator::allocMethod = 0;

void MemoryAllocator::setupMethod(AllocMethod method)
{
	allocMethod = method;
}

void* MemoryAllocator::alloc(size_t size)
{
	return allocMethod( 0, size);
}

void* MemoryAllocator::realloc(void* ptr, size_t nsize)
{
	return allocMethod(ptr, nsize);
}

void MemoryAllocator::free(void* ptr)
{
	allocMethod(ptr, 0);
}

void* MemoryAllocator::defaultAlloc(void* ptr, size_t size)
{
	return ::realloc(ptr, size);
}
