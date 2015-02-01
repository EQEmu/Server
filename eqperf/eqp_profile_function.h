#pragma once

#ifndef __PRETTY_FUNCTION__
	#ifdef _MSC_VER
		#define __PRETTY_FUNCTION__ __FUNCSIG__
	#else
		#define __PRETTY_FUNCTION__ __FUNCTION__
	#endif
#endif

#ifdef _MSC_VER
//#define EQP_EXPORT __declspec(dllexport)
#define EQP_EXPORT
#else
#define EQP_EXPORT
#endif
