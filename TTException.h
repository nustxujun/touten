#ifndef _TTException_H_
#define _TTException_H_

#include "ToutenCommon.h"

namespace TT
{
	enum ExceptionType
	{
		ET_UNKNOWN,
		ET_SCANING_FAILED,
		ET_PARSING_FAILED,
		ET_ASSEMBLING_FAILED,
	};

	enum ExceptionLevel
	{
		EL_NORMAL,
	};

	class ToutenExport Exception
	{
	public:
		Exception(ExceptionLevel lv, const std::string& de,  int l);

		const std::string getDesc()const;
		int getLine()const;
	private:
		ExceptionLevel level;
		int line;
		std::string desc;
	};

#define TT_DECL_EXCEPT(ExpName) class ToutenExport ExpName: public Exception{public : ExpName(ExceptionLevel lv, const std::string& desc, int l ):Exception(lv,desc,l){};};

	TT_DECL_EXCEPT(LexerException);
	TT_DECL_EXCEPT(ParserException);
	TT_DECL_EXCEPT(AssemblerException);

	class ExceptionFactory
	{
	public:
		template<class R>
		static R create(ExceptionLevel lv, const std::string& desc, int l)
		{
			return R(lv, desc, l);
		}
	};

	template<int Type>
	class ExceptionSelector;

	template<>struct ExceptionSelector<ET_UNKNOWN>
	{
		static Exception create(ExceptionLevel lv, const std::string& desc, int l)
		{
			return ExceptionFactory::create<Exception>(lv, desc, l);
		}
	};

	template<>struct ExceptionSelector<ET_SCANING_FAILED>
	{
		static LexerException create(ExceptionLevel lv, const std::string& desc, int l)
		{
			return ExceptionFactory::create<LexerException>(lv, desc, l);
		}
	};

	template<>struct ExceptionSelector<ET_PARSING_FAILED>
	{
		static ParserException create(ExceptionLevel lv, const std::string& desc, int l)
		{
			return ExceptionFactory::create<ParserException>(lv, desc, l);
		}
	};

	template<>struct ExceptionSelector<ET_ASSEMBLING_FAILED>
	{
		static AssemblerException create(ExceptionLevel lv, const std::string& desc, int l)
		{
			return ExceptionFactory::create<AssemblerException>(lv, desc, l);
		}
	};
#define TT_EXCEPT(type,level, desc, line) throw ExceptionSelector<type>::create(level, desc ,line);
}

#endif//_TTException_H_