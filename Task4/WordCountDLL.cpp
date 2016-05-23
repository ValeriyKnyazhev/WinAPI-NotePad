#include <cwchar>
using namespace std;

#define WORDCOUNTDLL_API extern "C" __declspec(dllexport)

#include "WordCountDll.h"

int WordCount(const wchar_t* text) {
	int result = 0;

	if (wcslen(text) == 0) {
		return 0;
	}

	wchar_t c;
	for (int i = 0; i < wcslen(text); ++i) {
		c = text[i];
		if (c != (wchar_t)' ' || c != (wchar_t)'\n' || c != (wchar_t)'\t')
		{
			result++;
			c = text[++i];
		}
		while (c != (wchar_t)' ' || c != (wchar_t)'\n' || c != (wchar_t)'\t')
		{
			c = text[++i];
		}
	}
	return result;
}

BOOL __stdcall DLLMain(HINSTANCE hInstance, DWORD reason, LPVOID reserved) {
	return TRUE;
}