#ifndef _TTLexer_H_
#define _TTLexer_H_

#include "ToutenCommon.h"
#include "TTStaticArea.h"

namespace TT
{
	enum TokenType
	{
		TT_ERROR,

		TT_OPERATOR,
		TT_ASSGIN,

		TT_INTEGER,
		TT_DOUBLE,
		TT_STRING,
		TT_NAME,

		TT_RESERVED_BEG,
		TT_GLOBAL = TT_RESERVED_BEG,
		TT_LOCAL,
		TT_SHARED,
		TT_FUNCTION,
		TT_FIELD,
		TT_WHILE,
		TT_DO,
		TT_FOR,
		TT_IF,
		TT_ELSEIF,
		TT_ELSE,
		TT_SWITCH,
		TT_NULL,
		TT_TRUE,
		TT_FALSE,
		TT_RETURN,

		TT_EOS,
	};

	union TokenValue
	{
		int i;
		double d;
		struct 
		{
			Char* b;
			size_t s;//×Ö·û¸öÊý
		}s;
	};

	struct Token
	{
		TokenType type;
		const Char* string;
		size_t size;
		size_t lineNum;

		TokenValue val;
	};


	enum LexerErrorType
	{
		LET_UNKNOWN,
	};

	class LexerError
	{
	public:
		LexerErrorType type;
		size_t lineNum;
		const Char* string;
		size_t size;

		~LexerError()
		{
		}
	};


	class Lexer
	{
		static const int eof = 0;
	public :
		Lexer(const Char* buff, StaticArea& sa);
		Token next();

	private:
		void incLineNum();
		void parse(const Char*& read, Token& token);
		void skipLine(const Char*& read);
		bool getVal(const Char*& read, Token& token);
		bool getNum(const Char*& read, Token& token);
		bool getString(const Char*& read, Token& token);
		bool getReserved(const Char*& read, Token& token);

		bool isChar(Char r);
		bool isDigit(Char r);
	private:
		const Char* mRead;
		size_t mLineNum;
		StaticArea& mStaticArea;
	};
}
#endif