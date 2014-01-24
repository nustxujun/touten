#ifndef _TTConstantPool_H_
#define _TTConstantPool_H_

#include "TTType.h"
#include "TTTools.h"

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

		template<>
		size_t operator<<(const String& str)
		{
			auto ret = mConstStringMap.find(str.c_str());
			if (ret != mConstStringMap.end())
				return ret->second;

			size_t charcount = str.size() + 1;

			size_t addr = *this << charcount;
			write(str.c_str(), sizeof(Char)* charcount);

			mConstStringMap.insert(ConstStringMap::value_type((const Char*)(*this)[addr + 4], addr));

			return addr;
		}

		template<>
		size_t operator<<(const Char*const& strval)
		{
			String str = strval;
			auto ret = mConstStringMap.find(strval);
			if (ret != mConstStringMap.end())
				return ret->second;

			size_t charcount = str.size() + 1;

			size_t addr = *this << charcount;
			write(str.c_str(), sizeof(Char)* charcount);

			mConstStringMap.insert(ConstStringMap::value_type((const Char*)(*this)[addr + 4], addr));

			return addr;
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


		class hash_compare
		{
		public:
			enum
			{	// parameters for hash table
				bucket_size = 1		// 0 < bucket_size
			};

			size_t operator()(const Char* key) const
			{
				size_t nHash = 0;
				while (*key)
					nHash = (nHash << 5) + nHash + *key++;
				return nHash;
			}

			bool operator()(const Char* k1, const Char* k2) const
			{
				return Tools::less(k1, k2);
			}
		};

		typedef std::hash_map<const Char*, size_t, hash_compare> ConstStringMap;
		ConstStringMap mConstStringMap;


	};
}

#endif