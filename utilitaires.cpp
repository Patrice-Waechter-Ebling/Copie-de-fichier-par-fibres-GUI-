#include "framework.h"

	HWND hWND;
	WNDCLASSEX wcex2;

int CALLBACK ProcedureParcourrirDossiers(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData);
char* ParcourrirDossiers();

void initUtilitaires(HWND hWnd, WNDCLASSEX wcex) 
{
	hWND = hWnd;
	wcex2 = wcex;
}
char* ChargerFichier()
{
    OPENFILENAME ofn ;
	char szFile[0x64]{};
	ZeroMemory( &ofn , sizeof( ofn));
	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL  ;
	ofn.lpstrFile = szFile ;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( szFile );
	ofn.lpstrFilter = "Tous les fichiers\0*.*\0\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL ;
	ofn.nMaxFileTitle = 0 ;
	ofn.lpstrInitialDir=NULL ;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST ;
	GetOpenFileName( &ofn );
	return ofn.lpstrFile;
}
int MsgBox( char* lpszText, char* lpszCaption, DWORD dwStyle, int lpszIcon) {
	MSGBOXPARAMS lpmbp{};
	lpmbp.hInstance = wcex2.hInstance;
	lpmbp.cbSize = sizeof(MSGBOXPARAMS);
	lpmbp.hwndOwner = hWND;
	lpmbp.dwLanguageId = MAKELANGID(0x0800, 0x0800); //par defaut celui du systeme
	lpmbp.lpszText = lpszText;
	lpmbp.lpszCaption = lpszCaption;
	lpmbp.dwStyle = dwStyle | 0x00000080L;
	lpmbp.lpszIcon = (LPCTSTR)lpszIcon;
	lpmbp.lpfnMsgBoxCallback = 0;
	return  MessageBoxIndirect(&lpmbp);
}
int MsgBox(const char* lpszText,const char* lpszCaption, DWORD dwStyle, int lpszIcon) {
	MSGBOXPARAMS lpmbp{};
	lpmbp.hInstance = wcex2.hInstance;
	lpmbp.cbSize = sizeof(MSGBOXPARAMS);
	lpmbp.hwndOwner = hWND;
	lpmbp.dwLanguageId = MAKELANGID(0x0800, 0x0800); 
	lpmbp.lpszText = lpszText;
	lpmbp.lpszCaption = lpszCaption;
	lpmbp.dwStyle = dwStyle | 0x00000080L;
	lpmbp.lpszIcon = (LPCTSTR)lpszIcon;
	lpmbp.lpfnMsgBoxCallback = 0;
	return  MessageBoxIndirect(&lpmbp);
}
void CreerElement(const char* szWindowClass, const char* szTitle, DWORD Style, int x, int y, int l, int L, int szID)
{
	CreateWindow(szWindowClass, szTitle, Style, x, y, l, L, hWND, (HMENU)szID, wcex2.hInstance, nullptr);
}

void CreerElement(char* szWindowClass, char* szTitle, DWORD Style, int x, int y, int l, int L, int szID) {
    CreateWindow(szWindowClass, szTitle, Style, x, y, l, L, hWND, (HMENU)szID, wcex2.hInstance, nullptr);
}
void CreerElement(char* szWindowClass, char* szTitle, DWORD Style, RECT rc, int szID) {
    CreateWindow(szWindowClass, szTitle, Style, rc.left, rc.top, rc.right,rc.bottom, hWND, (HMENU)szID, wcex2.hInstance, nullptr);
}
void CreerElement(HWND hDlg,char* szWindowClass, char* szTitle, DWORD Style, int x, int y, int l, int L, int szID) {
    CreateWindow(szWindowClass, szTitle, Style, x, y, l, L, hDlg, (HMENU)szID, wcex2.hInstance, nullptr);
}

char* ParcourrirDossiers() {
	BROWSEINFO bi{};
	char szPath[MAX_PATH + 1];
	LPITEMIDLIST pidl;
	BOOL bResult = FALSE;
	LPSTR lpszFolder = nullptr;
	LPMALLOC pMalloc;
	if (SUCCEEDED(SHGetMalloc(&pMalloc))) {
		bi.hwndOwner = hWND;
		bi.pidlRoot = NULL;
		bi.pszDisplayName = NULL;
		bi.lpszTitle = "Sélectionnez le dossier ou creer la copie";
		bi.ulFlags = BIF_STATUSTEXT | BIF_EDITBOX;
		bi.lpfn = ProcedureParcourrirDossiers;
		bi.lParam = (LPARAM)lpszFolder;
		pidl = SHBrowseForFolder(&bi);
		if (pidl) {
			if (SHGetPathFromIDList(pidl, szPath)) {
				bResult = TRUE;	SetDlgItemText(hWND, 6000, szPath);
			}
			pMalloc->Free(pidl);
			pMalloc->Release();
		}
	}
	return szPath;
}
int CALLBACK ProcedureParcourrirDossiers(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
	char szPath[MAX_PATH];
	HDC hDc = reinterpret_cast<HDC>(lp);

	switch (uMsg)
	{
	case WM_CTLCOLOREDIT:SetTextColor((HDC)lp, RGB(0xA0, 0xFF, 0xff)); SetBkMode((HDC)lp, TRANSPARENT); return (LONG)CreateSolidBrush(RGB(255, 255, 0));
	case WM_CTLCOLORLISTBOX:SetTextColor((HDC)lp, RGB(255, 0, 255)); SetBkMode((HDC)lp, TRANSPARENT); return (LONG)CreateSolidBrush(RGB(255, 255, 0));
	case WM_CTLCOLORBTN: {
		SetBkMode((HDC)lp, TRANSPARENT);
		if (GetDlgItem(hwnd, IDOK) == reinterpret_cast<HWND>(pData)) SetTextColor(hDc, RGB(0, 255, 0));
		if (GetDlgItem(hwnd, IDCANCEL) == reinterpret_cast<HWND>(pData)) SetTextColor(hDc, RGB(255, 0, 0));
		return (LONG)wcex2.hbrBackground;
	}
	case WM_CTLCOLORMSGBOX:return (LONG)CreateSolidBrush(RGB(255, 255, 0));
	case BFFM_INITIALIZED:
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;

	case BFFM_SELCHANGED:
		if (SHGetPathFromIDList((LPITEMIDLIST)lp, szPath))
		{
			SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szPath);

		}
		break;
	}

	return 0;
}