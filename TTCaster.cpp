#include "TTCaster.h"
#include "TTTools.h"
#include "TTMemoryAllocator.h"
#include "TTException.h"

using namespace TT;

void Caster::cast(Object& o, ObjectType otype)
{
	switch (otype)
	{
	case OT_NULL:
		castToNullObject(o); break;
	case OT_TRUE:
		castToBoolObject(o); break;
	case OT_FALSE:
		castToBoolObject(o); break;
	case OT_STRING:
		castToStringObject(o); break;
	case OT_DOUBLE:
		castToRealObject(o); break;
	case OT_INTEGER:
		castToIntObject(o); break;
	case OT_FUNCTION:
		castToFunctionObject(o); break;
		//case OT_FIELD:
		//	castToFieldObject(o); break;
	case OT_ARRAY:
		castToArrayObject(o); break;
	default:
		TT_EXCEPT(ET_UNKNOWN, EL_NORMAL, "unexpected type", 0);
		castToNullObject(o);
	}
}

void Caster::castToNullObject(Object& o)
{
	o.val.releaseVal();
}

void Caster::castToBoolObject(Object& o)
{
	switch (o.val->type)
	{
	case OT_INTEGER: case OT_DOUBLE:
		o.val->type = (o.val->i == 0) ? OT_FALSE : OT_TRUE;
		break;
	case OT_TRUE: case OT_FALSE:
		break;
	default:
		o.val.releaseVal();
		break;
	}
}

void Caster::castToStringObject(Object& o)
{
	TTString* str;
	switch (o.val->type)
	{
	case OT_STRING:
	case OT_NULL:
		return;
	case OT_TRUE:
		str = TT_NEW(TTString)(L"true");
		break;
	case OT_FALSE:
		str = TT_NEW(TTString)(L"false");
		break;
	case OT_DOUBLE:
		str = TT_NEW(TTString)(o.val->d);
		break;
	case OT_INTEGER:
		str = TT_NEW(TTString)(o.val->i);
		break;
	case OT_FUNCTION:
		//case OT_FIELD:
	case OT_ARRAY:
	{
					 o.val.releaseVal();
					 return;
	}

	}
	o.val->str.cont = str->data;
	o.val->str.size = str->numChar;
	o.val->type = OT_STRING;
}

void Caster::castToRealObject(Object& o)
{
	switch (o.val->type)
	{
	case OT_INTEGER:
		o.val->d = o.val->i;
		break;
	case OT_DOUBLE:
		break;
	case OT_TRUE:
		o.val->d = 1;
		break;
	case OT_FALSE:
		o.val->d = 0;
		break;
	case OT_STRING:
		o.val->d = *((TTString*)&o.val->str);
		break;
	default:
		o.val.releaseVal();
		return;
	}
	o.val->type = OT_DOUBLE;
}

void Caster::castToIntObject(Object& o)
{
	switch (o.val->type)
	{
	case OT_INTEGER:
		break;
	case OT_DOUBLE:
		o.val->i = (int)o.val->d;
		break;
	case OT_TRUE:
		o.val->i = 1;
		break;
	case OT_FALSE:
		o.val->i = 0;
		break;
	case OT_STRING:
		o.val->i = *((TTString*)&o.val->str);
		break;
	default:
		o.val.releaseVal();
		return;
	}
	o.val->type = OT_INTEGER;
}

void Caster::castToFunctionObject(Object& o)
{
}

void Caster::castToFieldObject(Object& o)
{
}

void Caster::castToArrayObject(Object& o)
{
	if (o.val->type == OT_ARRAY) return;

	Array* arr = TT_NEW(Array)(false);
	*(*arr)[(size_t)0] = o;
	o.val.releaseVal();
	o.val->type = OT_ARRAY;
	o.val->arr = arr;
}



bool Caster::castToBool(const Object& o)
{
	switch (o.val->type)
	{
	case OT_INTEGER:
		return o.val->i != 0;
	}
	return o.val->type == OT_TRUE;
}

//SharedPtr<TTString> Caster::castToString(const Object& o)
//{
//	TTString* str;
//	switch (o.val->type)
//	{
//	case OT_NULL:
//	case OT_FUNCTION:
//		//case OT_FIELD:
//		str = new TTString(L"null");
//		break;
//	case OT_ARRAY:
//		return castToString(*o.val->arr->get((size_t)0));
//	case OT_STRING:
//		str = new TTString(o.val->str);
//		break;
//	case OT_TRUE:
//		str = new TTString(L"true");
//		break;
//	case OT_FALSE:
//		str = new TTString(L"false");
//		break;
//	case OT_DOUBLE:
//		str = new TTString(o.val->d);
//		break;
//	case OT_INTEGER:
//		str = new TTString(o.val->i);
//		break;
//	}
//	return str;
//}

String Caster::castToString(const Object& o)
{
		switch (o.val->type)
		{
		case OT_ARRAY:
			return castToString(*o.val->arr->get((size_t)0));
		case OT_STRING:
			return o.val->str.cont;
			break;
		case OT_TRUE:
			return L"true";
			break;
		case OT_FALSE:
			return L"false";
			break;
		case OT_DOUBLE:
			return Tools::toString(o.val->d);
			break;
		case OT_INTEGER:
			return Tools::toString(o.val->i);
			break;
		default:
			return L"null";
		}
}


double Caster::castToReal(const Object& o)
{
	return castToCommon<double>(o);
}

int Caster::castToInt(const Object& o)
{
	return castToCommon<int>(o);
}

template<class Type>
Type Caster::castToCommon(const Object& o)
{
	switch (o.val->type)
	{
	case OT_STRING:
		return*((TTString*)&o.val->str);
	case OT_DOUBLE:
		return (Type)o.val->d;
	case OT_INTEGER:
		return (Type)o.val->i;
	case OT_TRUE:
		return (Type)1;
	case OT_FALSE:
		return (Type)0;
	default:
		//TTINTERPRETER_EXCEPT("cant cast");
		return (Type)0;
	}
}