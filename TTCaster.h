#ifndef _TTCaster_H_
#define _TTCaster_H_

#include "TTObject.h"

namespace TT
{

	class Caster
	{
	public:
		static void cast(Object& o, ObjectType otype);

		static bool castToBool(const Object& o);
		//static SharedPtr<TTString> castToString(const Object& o);
		static double castToReal(const Object& o);
		static int castToInt(const Object& o);
		static String castToString(const Object& o);
		template<class Type>
		static Type cast(const Object& o)
		{
			assert(0 && "unexpected type");
			return Type();
		}

		template<> static int cast(const Object& o){ return castToInt(o); }
		template<> static double cast(const Object& o){ return castToReal(o); }
		template<> static bool cast(const Object& o){ return castToBool(o); }
		//template<> static SharedPtr<TTString> cast(const Object& o){ return castToString(o); }
		template<> static String cast(const Object& o){ return castToString(o); }
			

	private:
		static void castToNullObject(Object& o);
		static void castToBoolObject(Object& o);
		static void castToStringObject(Object& o);
		static void castToRealObject(Object& o);
		static void castToIntObject(Object& o);
		static void castToFunctionObject(Object& o);
		static void castToFieldObject(Object& o);
		static void castToArrayObject(Object& o);

		template<class Type>
		static Type castToCommon(const Object& o);

	};

}

#endif