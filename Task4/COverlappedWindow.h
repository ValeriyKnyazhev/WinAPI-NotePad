#pragma once
#include <Windows.h>
#include <array>

class COverlappedWindow {
public:
	COverlappedWindow();
	~COverlappedWindow();

	//Зарегистрировать класс окна
	static bool RegisterClass();

	//Создать экземпляр окна
	bool Create();

	//Показать окно
	void Show(int cmdShow);

	HWND GetHandle() const;

	HWND GetHandleDialog() const;

	HACCEL GetHaccel() const;

protected:
	void OnDestroy();
	void OnCreate();
	void OnSize();
	bool OnClose();
	void OnSetFocus();
	
	static BOOL __stdcall dialogProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

	void OnDlgInit(HWND handle);
	void OnDlgOk(HWND handle);
	void OnDlgCancel(HWND handle);
	void OnDlgChangeColor(HWND handle, UINT buttonID);
	void OnDlgCheck(HWND handle);
	void OnDlgScroll(HWND handle);

	int GetCurrentFontSize(HWND handle, HWND handleEditControl);
	void setFont(HWND handleEditControl, int fontSize);
	void setAlpha(HWND handle, int alpha);

private:
	HWND handle;// хэндл окна
	HWND handleEdit;
	HMENU hMenu;
	HWND handleDialog;
	bool changedText = false;
	bool firstChangedText = false;
	//static HINSTANCE instance;
	bool getFileName(HWND handle, LPTSTR fileName);
	bool SaveFile();
	HACCEL haccel;

	wchar_t* buffer;

	int minFontSize = 8, maxFontSize = 72;
	BYTE minAlpha = 0, maxAlpha = 255;
	struct Parametres {
		COLORREF colorBackground = RGB(255, 255, 255), colorFont = RGB(0, 0, 0);
		UINT fontSize = 18;
		BYTE alpha = 255;
	} currentParametres, templateParametres;
	bool checkedPreview = false;
	HBRUSH brushBackground;

	static LRESULT __stdcall windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
};