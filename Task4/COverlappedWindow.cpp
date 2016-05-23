#include "COverlappedWindow.h"
#include <Windows.h>
#include <iostream>
#include <array>
#include <CommCtrl.h>
#include "resource.h"

typedef int(__cdecl *LPFNDLLFUNC)(const wchar_t*);
HINSTANCE hinstLib;
LPFNDLLFUNC ProcAdd;

COverlappedWindow::COverlappedWindow()
{
	
}

COverlappedWindow::~COverlappedWindow()
{

}

BOOL __stdcall COverlappedWindow::dialogProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {

	COverlappedWindow* window = reinterpret_cast<COverlappedWindow*>(::GetWindowLongPtr(::GetParent(handle),
		GWLP_USERDATA));

	switch (message) {
	case WM_INITDIALOG:
	{
		window->OnDlgInit(handle);
		return TRUE;
	}

	//простое сообщение, которое приходит при бездействии, например      WM_ENTERIDLE
	//
	case WM_HSCROLL:
	{
		window->OnDlgScroll(handle);
		return TRUE;
	}

	case WM_COMMAND:
	{
		switch (LOWORD(wParam)) {
		case IDCANCEL:
		{
			window->OnDlgCancel(handle);
			return TRUE;
		}
		case IDOK:
		{
			window->OnDlgOk(handle);
			return TRUE;
		}
		case IDC_BUTTON1:
		{
			window->OnDlgChangeColor(handle, IDC_BUTTON1);
			return TRUE;
		}
		case IDC_BUTTON2:
		{
			window->OnDlgChangeColor(handle, IDC_BUTTON2);
			return TRUE;
		}
		case IDC_CHECK1:
		{
			window->OnDlgCheck(handle);
			return TRUE;
		}

		}	
	}
	}

	return FALSE;
}

int COverlappedWindow::GetCurrentFontSize(HWND handle, HWND handleEditControl)
{
	HFONT font = (HFONT)::SendMessage(handleEditControl, WM_GETFONT, 0, 0);
	LOGFONT logfont;
	::GetObject(font, sizeof(LOGFONT), &logfont);
	return logfont.lfHeight;
}

void COverlappedWindow::setFont(HWND handleEditControl, int fontSize) {
	HFONT font = (HFONT)::SendMessage(handleEditControl, WM_GETFONT, 0, 0);
	if (!font) {
		font = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
	}
	LOGFONT logfont;
	::GetObject(font, sizeof(LOGFONT), &logfont);
	logfont.lfHeight = fontSize;
	DeleteObject(font);
	font = ::CreateFontIndirect(&logfont);
	::SendMessage(handleEditControl, WM_SETFONT, (WPARAM)font, true);
}

void COverlappedWindow::setAlpha(HWND handle, int alpha) {
	SetWindowLong(handle, GWL_EXSTYLE, GetWindowLong(handle, GWL_EXSTYLE) | WS_EX_LAYERED);
	::SetLayeredWindowAttributes(handle, 0, alpha, LWA_ALPHA);
}

void COverlappedWindow::OnDlgInit(HWND handle)
{
	COverlappedWindow* window = reinterpret_cast<COverlappedWindow*>(::GetWindowLongPtr(GetParent(handle),
		GWLP_USERDATA));
	//::SendMessage(GetParent(handle), WM_SETFOCUS, (WPARAM)handle, 0);
	HWND handleAlphaSlider = ::GetDlgItem(handle, IDC_SLIDER2);
	HWND handleFontSizeSlider = ::GetDlgItem(handle, IDC_SLIDER1);
	HWND handlePreviewCheckBox = ::GetDlgItem(handle, IDC_CHECK1);

	::SendMessage(handleAlphaSlider, TBM_SETRANGE, TRUE, MAKELONG(minAlpha, maxAlpha));
	::SendMessage(handleFontSizeSlider, TBM_SETRANGE, TRUE, MAKELONG(minFontSize, maxFontSize));
	//::SendDlgItemMessage(handle, IDC_SLIDER1, TBM_SETPOS, TRUE, 44);
	::SendMessage(handleAlphaSlider, TBM_SETPOS, TRUE, window->currentParametres.alpha);
	::SendMessage(handleFontSizeSlider, TBM_SETPOS, TRUE, window->currentParametres.fontSize);

	window->templateParametres.fontSize = GetCurrentFontSize(handle, window->handleEdit);

	if (window->checkedPreview) {
		::SendMessage(handlePreviewCheckBox, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		::SendMessage(handlePreviewCheckBox, BM_SETCHECK, BST_UNCHECKED, 0);
	}

}

void COverlappedWindow::OnDlgScroll(HWND handle) {
	COverlappedWindow* window = reinterpret_cast<COverlappedWindow*>(::GetWindowLongPtr(GetParent(handle),
		GWLP_USERDATA));

	int positionScrollFontSize = ::SendMessage(::GetDlgItem(handle, IDC_SLIDER1), TBM_GETPOS, 0, 0);
	int positionScrollAlpha = ::SendMessage(::GetDlgItem(handle, IDC_SLIDER2), TBM_GETPOS, 0, 0);

	window->templateParametres.fontSize = positionScrollFontSize;
	window->templateParametres.alpha = positionScrollAlpha;

	if (window->checkedPreview) {
		setFont(window->handleEdit, positionScrollFontSize);
		setAlpha(window->handle, positionScrollAlpha);
	}
}

void COverlappedWindow::OnDlgOk(HWND handle)
{
	COverlappedWindow* window = reinterpret_cast<COverlappedWindow*>(::GetWindowLongPtr(GetParent(handle),
		GWLP_USERDATA));

	window->currentParametres = window->templateParametres;

	setAlpha(window->handle, window->currentParametres.alpha);
	setFont(window->handleEdit, window->currentParametres.fontSize);
	
	//::EndDialog(handle, 0);
	DestroyWindow(window->handleDialog);
	window->handleDialog = NULL;
}

void COverlappedWindow::OnDlgCancel(HWND handle)
{
	COverlappedWindow* window = reinterpret_cast<COverlappedWindow*>(::GetWindowLongPtr(GetParent(handle),
		GWLP_USERDATA));

	setAlpha(window->handle, window->currentParametres.alpha);
	setFont(window->handleEdit, window->currentParametres.fontSize);

	//::EndDialog(handle, 0);
	DestroyWindow(window->handleDialog);
	window->handleDialog = NULL;
}

void COverlappedWindow::OnDlgChangeColor(HWND handle, UINT buttonID)
{
	COverlappedWindow* window = reinterpret_cast<COverlappedWindow*>(::GetWindowLongPtr(GetParent(handle),
		GWLP_USERDATA));

	CHOOSECOLOR cc;

	static COLORREF arrayCustomColor[16];

	ZeroMemory(&cc, sizeof(cc));
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = handle;
	cc.lpCustColors = (LPDWORD)arrayCustomColor;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;

	if (::ChooseColor(&cc) == TRUE)
	{
		switch (buttonID) {
		case IDC_BUTTON1:
		{
			window->templateParametres.colorBackground = cc.rgbResult;
			break;
		}
		case IDC_BUTTON2:
		{
			window->templateParametres.colorFont = cc.rgbResult;
			break;
		}
		}
	}

	if (window->checkedPreview) {
		::InvalidateRect(window->handleEdit, NULL, 1);
	}

}

void COverlappedWindow::OnDlgCheck(HWND handle)
{
	COverlappedWindow* window = reinterpret_cast<COverlappedWindow*>(::GetWindowLongPtr(GetParent(handle),
		GWLP_USERDATA)); 
	
	int positionScrollFontSize;
	int positionScrollAlpha;

	HWND previewCheckBox = ::GetDlgItem(handle, IDC_CHECK1);
	window->checkedPreview = (BOOL) ::SendMessage(previewCheckBox, BM_GETCHECK, 0, 0);
	DeleteObject(previewCheckBox);
	if (window->checkedPreview) {
		positionScrollFontSize = window->templateParametres.fontSize;
		positionScrollAlpha = window->templateParametres.alpha;
	}
	else {
		positionScrollFontSize = window->currentParametres.fontSize;
		positionScrollAlpha = window->currentParametres.alpha;
	}
	setFont(window->handleEdit, positionScrollFontSize);
	setAlpha(window->handle, positionScrollAlpha);
	::InvalidateRect(window->handleEdit, NULL, 1);
}

LRESULT __stdcall COverlappedWindow::windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{

	if (message == WM_NCCREATE) {
		COverlappedWindow* window =
			reinterpret_cast<COverlappedWindow*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
		::SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG>(window));
		window->handle = handle;
		return DefWindowProc(handle, message, wParam, lParam);
	}

	COverlappedWindow* window = reinterpret_cast<COverlappedWindow*>(::GetWindowLongPtr(handle, GWLP_USERDATA));

	switch (message) {
	case WM_CREATE:
	{
		window->OnCreate();
		return DefWindowProc(handle, message, wParam, lParam);
	}
	case WM_CLOSE:
	{
		if (window->OnClose())
			return DefWindowProc(handle, message, wParam, lParam);
		return 0;
	}
	case WM_CTLCOLOREDIT:
	{
		COLORREF colorFont, colorBackground;

		if (window->checkedPreview) {
			colorBackground = window->templateParametres.colorBackground;
			colorFont = window->templateParametres.colorFont;
		} else {
			colorBackground = window->currentParametres.colorBackground;
			colorFont = window->currentParametres.colorFont;
		}

		::SetTextColor((HDC)wParam, colorFont);
		::SetBkColor((HDC)wParam, colorBackground);
		DeleteObject(window->brushBackground);
		window->brushBackground = ::CreateSolidBrush(colorBackground);
		return (LRESULT)window->brushBackground;
	}
	case WM_SIZE:
	{
		window->OnSize();
		return 0;// DefWindowProc(handle, message, wParam, lParam);
	}
	
	case WM_COMMAND:
	{
		switch (HIWORD(wParam)) { // ID control
		case EN_CHANGE:
		{
			window->changedText = true;
			if (!window->firstChangedText) {
				window->firstChangedText = true;
				::SetWindowText(window->handleEdit, L"");
			}
			return 0;
		}
		case 1:
		{
			switch (LOWORD(wParam)) {
			case ID_MYACCELERATOR:
			{
				::PostQuitMessage(0);
				return 0;
			}
			}
			break;
		}
		case 0:
		{
			switch (LOWORD(wParam)) {
			case ID_FILE_SAVE:
			{
				if (window->changedText) {
					window->SaveFile();
				}
				break;
			}
			case ID_FILE_EXIT:
			{
				if (window->OnClose()) {
					PostQuitMessage(0);
				}
				break;
			}
			case ID_VIEW_SETTINGS:
			{
				//::DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_DIALOG1), handle, COverlappedWindow::dialogProc);
				if (window->handleDialog == NULL) {
					window->handleDialog = ::CreateDialog(::GetModuleHandle(0), MAKEINTRESOURCE(IDD_DIALOG1),
						window->handle, dialogProc);
					::ShowWindow(window->handleDialog, SW_SHOW);
					//::SetFocus(window->handleDialog);
				}
				//::SetFocus(window->handleDialog);
				break;
			}
			case ID_VIEW_WORDCOUNT:
			{
				DWORD len = SendMessage(window->handleEdit, WM_GETTEXTLENGTH, 0, 0);
				window->buffer = new wchar_t[len];
				::SendMessage(window->handleEdit, WM_GETTEXT, (WPARAM)len + 1, (LPARAM)window->buffer);
				wchar_t b[32];
				int k = (ProcAdd)(window->buffer);
				_itow_s(k, b, 10);
				MessageBox(NULL,
					b,
					L"WordCount",
					NULL);
				break;
			}
			}
			break;
		}
		default:
			return 0;
		}
	}
	case WM_SETFOCUS: {
		window->OnSetFocus();
		return 0;
	}
	case WM_DESTROY:
	{
		window->OnDestroy();
		return 0;
	}
	default:
		return DefWindowProc(handle, message, wParam, lParam);
	}
	return 0;

}

bool COverlappedWindow::RegisterClass()
{
	WNDCLASSEX windowClass;

	::ZeroMemory(&windowClass, sizeof(windowClass));
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = COverlappedWindow::windowProc;
	windowClass.hInstance = ::GetModuleHandle(0);
	windowClass.lpszClassName = L"COverlappedWindow";
	windowClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	windowClass.hIcon = (HICON) ::LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICONCSKA),
		IMAGE_ICON, 128, 128, 0);
	windowClass.hIconSm = (HICON) ::LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICONCSKA),
		IMAGE_ICON, 16, 16, 0);

	return ::RegisterClassEx(&windowClass) != 0;
}

bool COverlappedWindow::Create()
{
	wchar_t windowName[16];
	::LoadString(GetModuleHandle(0), IDS_MYSTRING, windowName, 16 * sizeof(wchar_t));

	hMenu = ::LoadMenu(GetModuleHandle(0), MAKEINTRESOURCE(IDR_MENU1));
	/*::EnableMenuItem(hMenu, ID_FILE_EXIT, MF_DISABLED);
	DrawMenuBar(handle);*/

	COverlappedWindow::handle = ::CreateWindowEx(0,
		L"COverlappedWindow",
		windowName,
		WS_OVERLAPPEDWINDOW/* | WS_DLGFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZE*/,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		hMenu,
		::GetModuleHandle(0),
		this);
	
	hinstLib = LoadLibrary(TEXT("WordCountDll.dll"));
	ProcAdd = (LPFNDLLFUNC)GetProcAddress(hinstLib, "WordCount");

	return COverlappedWindow::handle != 0;
}

void COverlappedWindow::OnCreate()
{
	RECT rect;
	::GetClientRect(handle, &rect);


	COverlappedWindow::handleEdit = ::CreateWindowEx(0,
		L"EDIT",  // Predefined class; Unicode assumed 
		0,      // Edit text 
		ES_MULTILINE | WS_VISIBLE | WS_VSCROLL |
		ES_LEFT | WS_CHILD | ES_AUTOVSCROLL | WS_BORDER,  // Styles 
		rect.left,         // x position 
		rect.top,         // y position 
		rect.right - rect.left,        // width
		rect.bottom - rect.top,        // height
		handle,     // Parent window
		0,       // Menu.
		(HINSTANCE)GetModuleHandle(0),
		0);

	HRSRC myresource = ::FindResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_MYRESOURCE), L"MYRESOURCE");
	HGLOBAL globalResource = ::LoadResource(GetModuleHandle(0), myresource);
	LPVOID lpResource = ::LockResource(globalResource);
	DWORD sizeresource = SizeofResource(GetModuleHandle(0), myresource);

	wchar_t* text = (wchar_t*)lpResource;
	SetWindowText(handleEdit, text);

	haccel = LoadAccelerators(GetModuleHandle(0), MAKEINTRESOURCE(ID_MYACCELERATOR));
}

void COverlappedWindow::OnSize()
{
	RECT rect;
	::GetClientRect(handle, &rect);
	SetWindowPos(handleEdit, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0);
	::DeleteObject(&rect);
}

bool COverlappedWindow::getFileName(HWND handle, LPTSTR fileName) {
	OPENFILENAME ofn;
	::ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = handle;
	ofn.hInstance = GetModuleHandle(0);
	ofn.nMaxFile = MAX_PATH;//размер буфера lpstrFile
	ofn.lpstrFile = fileName;//Указатель на буфер, содержит имя файла, чтобы инициализировать поле редактирования
	ofn.lpstrDefExt = L"txt";//расширение по умолчанию
	//ofn.lpstrFileTitle = L"1.txt";
	//ofn.nMaxFileTitle = sizeof(wchar_t) * 6;
	ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_EXTENSIONDIFFERENT | OFN_OVERWRITEPROMPT;

	return GetSaveFileName(&ofn);
}

bool writeFile(LPTSTR fileName, int lenText, LPTSTR fileString) {
	HANDLE handleFile = ::CreateFile(
		fileName,
		GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ,
		0,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		0);

	if (handleFile == INVALID_HANDLE_VALUE) {
		return false;
	}

	LPDWORD writtenBytes = 0;
	DWORD encoding = 0xFEFF;
	::WriteFile(handleFile, &encoding, 2, writtenBytes, 0);
	if (!::WriteFile(handleFile, fileString, lenText * sizeof(wchar_t), writtenBytes, 0)) {
		return false;
	}

	::CloseHandle(handleFile);

	return true;
}

bool COverlappedWindow::OnClose()
{
	bool exit = false;

	if (changedText) {
		
		int messageBoxID = ::MessageBox(handleEdit, L"Сохранить документ?", L"Fucking window", MB_YESNOCANCEL);

		switch (messageBoxID)
		{
		case IDYES:
		{
			exit = SaveFile();
			break;
		}
		case IDNO:
			exit = true;
			break;
		case IDCANCEL:
			exit = false;
			break;
		}
	} else {
		exit = true;
	}
	
	return exit;
}

void COverlappedWindow::OnSetFocus()
{
	SetFocus(COverlappedWindow::handleEdit);
}

void COverlappedWindow::OnDestroy()
{
	DeleteObject(brushBackground);
	FreeLibrary(hinstLib);
	::PostQuitMessage(0);
}

void COverlappedWindow::Show(int cmdShow)
{
	::ShowWindow(handle, cmdShow);
	::ShowWindow(handleEdit, cmdShow);
	
}

bool COverlappedWindow::SaveFile()
{
	DWORD textLength = ::SendMessage(handleEdit, WM_GETTEXTLENGTH, 0, 0);

	if (textLength == 0) {
		return true;
	}

	LPTSTR fileString = new wchar_t[sizeof(wchar_t) * textLength + 1];

	if (!fileString) {
		return false;
	}

	DWORD text = ::SendMessage(handleEdit, WM_GETTEXT, textLength * sizeof(wchar_t), reinterpret_cast<LPARAM>(fileString));

	wchar_t fileName[MAX_PATH] = L"";
	if (!getFileName(handle, fileName))
	{
		delete fileString;
		return false;
	}

	if (!writeFile(fileName, textLength, fileString)) {
		delete fileString;
		return false;
	}

	delete fileString;
	changedText = false;

	return true;
}

HWND COverlappedWindow::GetHandle() const
{
	return handle;
}

HWND COverlappedWindow::GetHandleDialog() const
{
	return handleDialog;
}

HACCEL COverlappedWindow::GetHaccel() const
{
	return haccel;
}