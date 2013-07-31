#ifndef _TTStaticArea_H_
#define _TTStaticArea_H_

#define STATIC_AREA_DEFAULT_SIZE 1024

namespace TT
{
	class StaticArea
	{
	public :
		StaticArea(size_t size = STATIC_AREA_DEFAULT_SIZE);
		~StaticArea();

		void* write(const void* b, size_t size); 
		void* assgin(size_t size);

		size_t size()const;
	private:
		struct Buffer
		{
			char* data;
			char* last;
			char* tail;
			Buffer* next;

			size_t size()
			{
				size_t size = last - data;
				if (next) size += next->size();
				return size;
			}
			
			~Buffer()
			{
				delete data;
				if (next) delete next;
			}
		};

		Buffer* createBuffer(size_t size);
		void reserve(size_t s);

	private:


		Buffer* mDatas;
		Buffer* mLast;
		size_t mMemTotal;
	};
}

#endif