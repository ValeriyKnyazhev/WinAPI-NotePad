#pragma once
#include <Windows.h>

#ifdef WORDCOUNTDLL_API

#else
#define WORDCOUNTDLL_API __declspec(dllimport)
#endif

extern "C" WORDCOUNTDLL_API int WordCount(const wchar_t* text);