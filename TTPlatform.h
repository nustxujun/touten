#ifndef _TTPlatform_H_
#define _TTPlatform_H_

#ifdef TOUTEN_TTATIC_LIB
#	define ToutenExport
#else
#	ifdef TOUTEN_NONCLIENT_BUILD
#		define ToutenExport __declspec( dllexport )
#	else
#		define ToutenExport __declspec( dllimport )
#	endif
#endif

#pragma warning(disable:4251)

#endif