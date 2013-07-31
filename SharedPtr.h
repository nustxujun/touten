#ifndef _SharedPtr_H_
#define _SharedPtr_H_

namespace TT
{
	template<class T>
	class SharedPtr
	{
	public :
		SharedPtr(): mInstance(0), mUseCount(0)
		{
		}

		SharedPtr(T* inst)
		{
			if (inst == 0)
			{
				mInstance = 0;
				mUseCount = 0;
			}
			else
			{
				mInstance = inst;
				mUseCount = new unsigned int(1);
			}
		}

		SharedPtr(const SharedPtr<T>& ptr)
		{
			mInstance = ptr.mInstance;
			mUseCount = ptr.mUseCount;
			addRef();
		}

		template<class S>
		SharedPtr(const SharedPtr<S> ptr)
		{
			mInstance = ptr.pointer();
			mUseCount = ptr.counter();
			addRef();
		}

		~SharedPtr()
		{
			release();
		}

		T* pointer()const
		{
			return mInstance;
		}

		unsigned int* counter()const
		{
			return mUseCount;
		}

		void addRef()
		{
			if (mUseCount)
				++(*mUseCount);
		}

		void release()
		{
			if (mUseCount != 0 && 
				--(*mUseCount) == 0)
			{
				delete mUseCount;
				delete mInstance;
				mUseCount = 0;
				mInstance = 0;
			}
		}

		bool isNull()const
		{
			return mInstance == 0;
		}

		void setNull()
		{
			release();
		}

		SharedPtr& operator= (const SharedPtr& ptr)
		{
			if (mInstance != ptr.mInstance )
			{
				release();
				mInstance = ptr.mInstance;
				mUseCount = ptr.mUseCount;
				addRef();
			}
			return *this;
		}

		template<class S>
		SharedPtr& operator= (const SharedPtr<S>& ptr)
		{
			if (mInstance != ptr.pointer() )
			{
				release();
				mInstance = ptr.pointer();
				mUseCount = ptr.counter();
				addRef();
			}
			return *this;
		}

		T* operator->()const
		{
			return mInstance;
		}

		T& operator*()const
		{
			return *mInstance;
		}

	private:
		T* mInstance;
		unsigned int* mUseCount;

	};

	template<class T, class U>
	bool operator <( const SharedPtr<T>& t, const SharedPtr<U> u)
	{
		return t.pointer() < u.pointer();
	}

	template<class T, class U>
	bool operator ==( const SharedPtr<T>& t, const SharedPtr<U> u)
	{
		return t.pointer() == u.pointer();
	}

	template<class T, class U>
	bool operator !=( const SharedPtr<T>& t, const SharedPtr<U> u)
	{
		return t.pointer() != u.pointer();
	}
}

#endif