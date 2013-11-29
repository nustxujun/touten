#include "TTLexer.h"
#include <math.h>
using namespace TT;

#define TTLEXER_EXCEPT(t) { LexerError error;error.type = t; error.lineNum = mLineNum; throw error;}
#define TTLEXER_ASSERT(e,t) { if (!(e)) { TTLEXER_EXCEPT(t) } }

Lexer::Lexer(const Char* buff, StaticArea& sa):
	mRead(buff),mLineNum(0), mStaticArea(sa)
{
	
}

Token Lexer::next()
{
	Token t;
	parse(mRead, t);
	return t;
}

void Lexer::incLineNum()
{
	++mLineNum;
}

void Lexer::parse(const Char*& read, Token& t)
{
	while (true)
	{
		Char c = *read;
		switch (c)
		{
		case '\r':
		case '\n':
			{
				++read;
				incLineNum();
				continue;
			}
			break;
		case ' ':
		case '	':
			++read;
			continue;
			break;
		case eof:
			t.type = TT_EOS;
			return;
		case '/':
			{
				t.string = read;
				t.lineNum = mLineNum;
				
				++read;
				if ( *read != '/')
				{
					t.size = 1;
					t.type = TT_OPERATOR;
					return ;
				}

				skipLine(read);
			}
			break;
		case '~':case '%':case '^':	case '*':case '-':	case '+':case '.':
			{
				t.type = TT_OPERATOR;
				t.string = read++;
				t.size = 1;
				t.lineNum = mLineNum;
			}
			return;
		case '{':case '}':case '[':	case ']':case ';':case ',':case '(':case ')':
			{
				t.type = TT_DELIMITER;
				t.string = read++;
				t.size = 1;
				t.lineNum = mLineNum;
			}
			return;
		case '&':
		case '|':
			{
				t.type = TT_OPERATOR;
				t.string = read++;
				t.lineNum = mLineNum;
				if (*read == c)
					t.size = 2, ++read;
				else
					t.size = 1;
				return;
			}
			break;
		case '!':
		case '<':
		case '>':
			{
				t.type = TT_OPERATOR;
				t.string = read++;
				t.lineNum = mLineNum;

				if (*read == '=')
					t.size = 2,	++read;
				else
					t.size = 1;	
			}
			return ;
			break;
		case '=':
			{
				t.type = TT_ASSGIN;
				t.string = read++;
				t.lineNum = mLineNum;
				if (*read == '=')
				{
					t.type = TT_OPERATOR;
					t.size = 2;
					++read;
				}
				else
				{
					t.size = 1;
				}
				return;
			}
			break;
		case '@':
			t.type = TT_PRE_GL;
			++read;
			return ;
			break;
		case '$':
			t.type = TT_PRE_SA;
			++read;
			return ;
			break;
		//case '#':
		//	t.type = TT_PRE_FL;
		//	++read;
		//	return ;
		//	break;
		default:
			{
				if (getReserved(read ,t))
					return;
				if (getVal(read, t))
					return;
				if (getNum(read, t))
					return;
				if (getString(read, t))
					return;

				TTLEXER_EXCEPT(LET_UNKNOWN);
			}
			break;
		}
	}
}

void Lexer::skipLine(const Char*& read)
{
	while ( *read != '\n' && *read != '\r' && *read != eof)
		++read;
}

bool Lexer::getVal(const Char*& read, Token& token)
{
	
	if ( isChar(*read))
	{
		token.lineNum = mLineNum;
		token.type = TT_NAME;
		token.string = read;
		token.size = 1;
		++read;

		while (true)
		{
			if ( isChar(*read) || isDigit(*read) ||
				 *read == '_')
			{
				++token.size;
				++read;
			}
			else
				break;
		}
		return true;
	}

	return false;
}

bool Lexer::getNum(const Char*& read, Token& token)
{
	const Char* dotpos = 0;
	enum
	{
		ERR,
		DEX,
		BIN,
		HEX,
	};
	size_t type  = ERR;
	bool n = false;
	switch (*read)
	{
	case '-':
		n = true;
		++read;
		break;
	case '0':
		{
			++read;
			switch (*read)
			{
			case '.':
				{
					dotpos = read;
					++read;
					break;
				}
			case 'x':
				{
					++read;
					type = HEX;
					break;
				}
			case 'b':
				{
					++read;
					type = BIN;
					break;
				}
			default:
				type = DEX;
				break;
			}
		}
		break;
	case '1':case '2':case '3':
	case '4':case '5':case '6':
	case '7':case '8':case '9':
		{
			type = DEX;
		}
		break;
	default :
		return false;
	}

	size_t num = 0;
	while (true)
	{

		if( isDigit(*read))
		{
			size_t digit = *read - '0';
			switch (type)
			{
			case BIN:
				if (*read <= '1')
				{
					TTLEXER_EXCEPT(LET_UNKNOWN);
					return false;
				}
				
				num = (num << 1) + digit;
				break;
			case HEX:
				num = (num << 4) + digit;
				break;
			default:
				num = num * 10 + digit;
				break;

			}
		}
		else if (*read == '.')
		{
			if (dotpos == 0 && type == DEX)
			{
				TTLEXER_EXCEPT(LET_UNKNOWN);
				return false;
			}
			dotpos = read;
		}
		else if (type == HEX && 
			( (*read >= 'a' && *read <= 'f') || 
			(*read >= 'A' && *read <= 'F')))
		{
			const size_t hexCharCount = 6;
			const size_t hexA = 0x0a;
			size_t digit = *read - 'a';

			if (digit >= hexCharCount)
				digit = *read - 'A';

			num = (num << 4) + digit + hexA;
		}
		else
		{
			if ( isChar(*read) ||
				*read == '_')
			{
				//以后加科学计数
				TTLEXER_EXCEPT(LET_UNKNOWN); 
			}

			token.lineNum = mLineNum;
			if (dotpos == 0)
			{
				if (type == ERR)
				{
					TTLEXER_EXCEPT(LET_UNKNOWN); 
					return false;
				}

				token.type = TT_INTEGER;
				token.val.i = num;
				
			}
			else
			{
				token.type = TT_DOUBLE;
				token.val.d = (double)num /  pow((double)10, (read - dotpos - 1));
			}
			return true;
		}

		++read;
	}
}

bool Lexer::getString(const Char*& read, Token& token)
{
	size_t size = 0;
	Char dob = *read;
	if (dob == '"' || dob == '\'')
	{
		++read;
		token.string = read;
		token.lineNum = mLineNum;
		token.type = TT_STRING;
		
		while (true)
		{
			switch (*read)
			{
			case '"':
			case '\'':
				{
					if (dob != *read)
						break;

					token.val.s.s = size + 1;

					if (token.val.s.s > MAX_VAR_NAME_LEN)
					{
						TTLEXER_EXCEPT(LET_UNKNOWN);
						return false;
					}
					token.val.s.b = (Char*)mStaticArea.assgin( sizeof(Char) * token.val.s.s);

					const Char* begin = token.string;
					Char* write = token.val.s.b;
					while ( begin < read)
					{
						if(*begin == '\\')
						{
							switch (*(++begin))
							{
							case 'a': *write = '\a'; break;
							case 'b': *write = '\b'; break;
							case 'f': *write = '\f'; break;
							case 'n': *write = '\n'; break;
							case 'r': *write = '\r'; break;
							case 't': *write = '\t'; break;
							case 'v': *write = '\v'; break;
							case '\n':*write = '\n'; break;
							case '\r':*write = '\r'; break;
							default:
								TTLEXER_EXCEPT(LET_UNKNOWN); 
								return false;
							}
						}
						else
							*write =  *begin;

						++begin;
						++write;

					}
					*write = 0;
					++read;
					return true;
					
				}
				break;
			case '\n':
			case '\r':
			case eof:
				TTLEXER_EXCEPT(LET_UNKNOWN); 
				return false;
			default:
				break;
			}
			++size;
			++read;
		}
	}

	return false;
}

bool Lexer::getReserved(const Char*& read, Token& token)
{
	const Char* reserved[] = {//顺序和枚举关联
		L"global", 
		L"local", 
		L"shared",
		L"function",
		//L"field",
		L"while",
		L"do",
		L"for",
		L"if",
		L"elseif",
		L"else",
		L"switch",
		L"null",
		L"true",
		L"false",
		L"return"
	};

	

	for (size_t i = 0; i < (sizeof(reserved) / sizeof(const Char*)); ++i)
	{
		const Char* src = read;
		const Char* mod = reserved[i];

		while (true)
		{
			if (*src != 0 && *mod != 0 && *mod == *src ) 
			{
				++mod;++src;
				continue;
			}
			if ( *mod != 0 ) break;
			//注意 else elseif 或者 else if
			if (isChar(*src) || isDigit(*src) || *src == '_') break;
			
			token.lineNum = mLineNum;
			token.type = (TokenType)(i + TT_RESERVED_BEG);
			
			read = src;
			return true;
		}
	}

	return false;
}

bool Lexer::isChar(Char r)
{
	return ( r >= 'a' && r <='z') || (r >='A' && r <= 'Z');
}

bool Lexer::isDigit(Char r)
{
	return r >= '0' && r <= '9';
}