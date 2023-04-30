// Copie de fichier par fibres(GUI).cpp : Définit le point d'entrée de l'application.
//

#include "framework.h"
#include "Copie de fichier par fibres(GUI).h"

#define MAX_LOADSTRING 100
#define RetourValide 0
#define RetourFonction 1
#define RetourErreur 0x0D
#define Tampon 0x7FFF
#define nbFibres 3 
#define FibrePrimaire 0 
#define FibreLecture 1
#define FibreEcriture 2 
typedef struct
{
    DWORD dwParametre;
    DWORD dwResultat;
    HANDLE hFichier;
    DWORD dwBitCompletes;
    CHAR Source[MAX_PATH];
    CHAR Cible[MAX_PATH];
} FIBERDATASTRUCT, * PFIBERDATASTRUCT, * LPFIBERDATASTRUCT;

// Variables globales :
CHAR szTitle[MAX_LOADSTRING];
CHAR szWindowClass[MAX_LOADSTRING]; 
WNDCLASSEX wcex;
HWND hWnd;

LPVOID g_Fibre[nbFibres];
LPBYTE g_Tampon;
DWORD dwBitsLus;

// Déclarations anticipées des fonctions incluses dans ce module de code :
LRESULT CALLBACK    ProcedurePrincipale(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ProcedureInfoApp(HWND, UINT, WPARAM, LPARAM);
VOID __stdcall LireFibre(LPVOID Parametre);
VOID __stdcall EcrireFibre(LPVOID Parametre);
void AfficherInfoFibre(void);
int CreerInterface(HINSTANCE hInstance);

//imports depuis utilitaires.cpp
char* ChargerFichier();
int CopieFichier(CHAR* Source, CHAR* Cible);
void CreerElement(const char* szWindowClass,const char* szTitle, DWORD Style, int x, int y, int l, int L, int szID);
void CreerElement(char* szWindowClass, char* szTitle, DWORD Style, int x, int y, int l, int L, int szID);
void CreerElement(char* szWindowClass, char* szTitle, DWORD Style, RECT rc, int szID);
void CreerElement(HWND hDlg, char* szWindowClass, char* szTitle, DWORD Style, int x, int y, int l, int L, int szID);
void initUtilitaires(HWND hWnd, WNDCLASSEX wcex);
char* ParcourrirDossiers();
int MsgBox(char* lpszText, char* lpszCaption, DWORD dwStyle, int lpszIcon);
int MsgBox(const char* lpszText, const char* lpszCaption, DWORD dwStyle, int lpszIcon);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hPrevInstance,_In_ LPWSTR    lpCmdLine,_In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_COPIEDEFICHIERPARFIBRESGUI, szWindowClass, MAX_LOADSTRING);
    if (!CreerInterface(hInstance)) 
    { 
        EndDialog(hWnd, false);
        MsgBox("Le montage de l'interface utilisateur a connu une erreur\nFin du programme", szTitle, MB_OK | MB_ICONERROR,IDI_ICON1); 
        PostQuitMessage(true);
    }
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_COPIEDEFICHIERPARFIBRESGUI));
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int) msg.wParam;
}
LRESULT CALLBACK ProcedurePrincipale(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC=reinterpret_cast<HDC>(wParam);
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case 0x3E9: SetDlgItemText(hWnd, 0x3E8, ChargerFichier()); break;
            case 0x3EB: SetDlgItemText(hWnd, 0x3EA, ChargerFichier()); break;
            case IDM_ABOUT:
                DialogBox(wcex.hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, ProcedureInfoApp);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_DROPFILES: {
        HDROP hDropInfo = (HDROP)wParam;
        int nb, taille, i;
        taille = 0;
        nb = 0;
        nb = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
        char fic[500];
        for (i = 0; i < nb; i++) {
            taille = DragQueryFile(hDropInfo, i, NULL, 0) + 1;
            DragQueryFile(hDropInfo, i, fic, taille);
            SetDlgItemText(hWnd,0x3E8,fic);
        }
        DragFinish(hDropInfo);
    }	break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_CTLCOLORSTATIC: 
        SetBkMode(hDC, TRANSPARENT);
        SetTextColor((HDC)wParam, RGB(128, 255, 128));
        return (LONG)wcex.hbrBackground;
    case WM_CTLCOLOREDIT:
        if (GetDlgItem(hWnd, 0x3E8) == reinterpret_cast<HWND>(lParam)) SetTextColor(hDC, RGB(0,255,  0));
        if (GetDlgItem(hWnd, 0x3EA) == reinterpret_cast<HWND>(lParam)) SetTextColor(hDC, RGB(255, 0, 25));
        SetBkMode((HDC)wParam, TRANSPARENT); 
        return (LONG)CreateSolidBrush(RGB(128, 128, 255));

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
INT_PTR CALLBACK ProcedureInfoApp(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hDC = reinterpret_cast<HDC>(wParam);
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_CTLCOLORSTATIC:
        SetBkMode(hDC, TRANSPARENT);
        SetTextColor((HDC)wParam, RGB(128,  255,0));
        return (LONG)CreateSolidBrush(RGB(128, 128, 255));
    case WM_CTLCOLORDLG:return (LONG)CreateSolidBrush(RGB(128, 128, 255));


    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
int CreerInterface(HINSTANCE hInstance) 
{
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = ProcedurePrincipale;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COPIEDEFICHIERPARFIBRESGUI));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(RGB(0, 0x7F, 0x7F));
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_COPIEDEFICHIERPARFIBRESGUI);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm = wcex.hIcon;
    if (!RegisterClassEx(&wcex)) { return false; }
    hWnd = CreateWindowEx(WS_EX_DLGMODALFRAME|WS_EX_WINDOWEDGE,szWindowClass, szTitle, WS_OVERLAPPED |WS_CAPTION |WS_SYSMENU |WS_THICKFRAME,CW_USEDEFAULT, CW_USEDEFAULT, 640, 410, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd){return FALSE;}
    initUtilitaires(hWnd, wcex);
    ShowWindow(hWnd, 0x05);
    UpdateWindow(hWnd);
    CreateStatusWindow(WS_CHILD | WS_VISIBLE, __FILE__, hWnd, 0x1770);
    CreerElement(WC_STATIC, "Fichier source a copier", WS_CHILD | WS_VISIBLE, 32, 62, 150, 19, 0);
    CreerElement(WC_EDIT, "", WS_CHILD | WS_VISIBLE, 32, 82, 450, 21, 0x3E8);
    CreerElement(WC_BUTTON, "Ouvrir Source", WS_CHILD | WS_VISIBLE, 488, 82, 100, 21, 0x3E9);
    CreerElement(WC_STATIC, "Fichier cible a creer", WS_CHILD | WS_VISIBLE, 32, 132, 150, 19, 0);
    CreerElement(WC_EDIT, "", WS_CHILD | WS_VISIBLE, 32, 152, 450, 21, 0x3EA);
    CreerElement(WC_BUTTON, "Ouvrir Cible", WS_CHILD | WS_VISIBLE, 488, 152, 100, 21, 0x3EB);
    CreerElement(WC_STATIC, "Progression", WS_CHILD | WS_VISIBLE|BS_CENTER, 32, 182, 90, 19, 0);
    CreerElement(WC_PROGRESS, "", WS_CHILD | WS_VISIBLE| PBS_MARQUEE | WS_BORDER, 132, 182, 350, 21, 0x3ED);
    CreerElement(WC_BUTTON, "Copier", WS_CHILD | WS_VISIBLE, 385, 302, 100, 21, IDOK);
    CreerElement(WC_BUTTON, "?", WS_CHILD | WS_VISIBLE, 487, 302, 25, 21, IDM_ABOUT);
    CreerElement(WC_BUTTON, "&Quitter", WS_CHILD | WS_VISIBLE, 515, 302, 90, 21, IDM_EXIT);
    HMENU mnu = GetSystemMenu(hWnd, 0);
    RemoveMenu(mnu, SC_SIZE, MF_BYCOMMAND);
    RemoveMenu(mnu, SC_MOVE, MF_BYCOMMAND);
    DragAcceptFiles(hWnd,true);
    return true;
}
int CopieFichier(CHAR* Source, CHAR* Cible)
{
    LPFIBERDATASTRUCT Fibre;
    char buff[0x64];
    Fibre = (LPFIBERDATASTRUCT)HeapAlloc(GetProcessHeap(), 0,sizeof(FIBERDATASTRUCT) * nbFibres);
    if (Fibre == NULL)
    {
        wsprintf(buff,"Erreur d'allocation Stack (%d)\n", GetLastError());
        MsgBox( buff, szTitle, MB_ICONERROR | MB_OK,0x83);
        return RetourErreur;
    }
    g_Tampon = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, Tampon);
    if (g_Tampon == NULL)
    {
        wsprintf(buff, "Erreur d'allocation tampon (%d)\n", GetLastError());
        MsgBox( buff, szTitle, MB_ICONERROR | MB_OK, 0x83);
        return RetourErreur;
    }
    Fibre[FibreEcriture].hFichier = CreateFile(Source,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    if (Fibre[FibreEcriture].hFichier == INVALID_HANDLE_VALUE)
    {
        wsprintf(buff, "Erreur d'acces au fichier (%d)\n%s", GetLastError(),Source);
        MsgBox( buff, szTitle, MB_ICONERROR | MB_OK, 0x83);
        return RetourErreur;
    }
    Fibre[FibreEcriture].hFichier = CreateFile(Cible,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    if (Fibre[FibreEcriture].hFichier == INVALID_HANDLE_VALUE)
    {
        wsprintf(buff, "Erreur d'acces au fichier (%d)\n%s", GetLastError(), Cible);
        MsgBox( buff, szTitle, MB_ICONERROR | MB_OK, 0x83);
        return RetourErreur;
    }
    g_Fibre[FibrePrimaire] = ConvertThreadToFiber(&Fibre[FibrePrimaire]);
    if (g_Fibre[FibrePrimaire] == NULL)
    {
        wsprintf(buff, "Erreur de convertion fil vers fibre (%d)\n", GetLastError());
        MsgBox( buff, szTitle, MB_ICONERROR | MB_OK, 0x83);
        return RetourErreur;
    }
    Fibre[FibrePrimaire].dwParametre = 0;
    Fibre[FibrePrimaire].dwResultat = 0;
    Fibre[FibrePrimaire].hFichier = INVALID_HANDLE_VALUE;
    g_Fibre[FibreEcriture] = CreateFiber(0, LireFibre, &Fibre[FibreEcriture]);
    if (g_Fibre[FibreEcriture] == NULL)
    {
        wsprintf(buff, "Erreur d'allocation pour la fibre de lecture (%d)\n", GetLastError());
        MsgBox( buff, szTitle, MB_ICONERROR | MB_OK, 0x83);
        return RetourErreur;
    }
    Fibre[FibreEcriture].dwParametre = 0x12345678;
    g_Fibre[FibreEcriture] = CreateFiber(0, EcrireFibre, &Fibre[FibreEcriture]);
    if (g_Fibre[FibreEcriture] == NULL)
    {
        wsprintf(buff, "Erreur d'allocation pour la fibre d'écriture (%d)\n", GetLastError());
        MsgBox( buff, szTitle, MB_ICONERROR | MB_OK, 0x83);
        return RetourErreur;
    }
    Fibre[FibreEcriture].dwParametre = 0x54545454;
    SwitchToFiber(g_Fibre[FibreEcriture]);
    wsprintf(buff,"Statut: %lu/%lu\n",Fibre[FibreEcriture].dwBitCompletes, Fibre[FibreEcriture].dwBitCompletes);
    SetDlgItemText(hWnd, 6000, buff);
    DeleteFiber(g_Fibre[FibreEcriture]);
    DeleteFiber(g_Fibre[FibreEcriture]);
    CloseHandle(Fibre[FibreEcriture].hFichier);
    CloseHandle(Fibre[FibreEcriture].hFichier);
    HeapFree(GetProcessHeap(), 0, g_Tampon);
    HeapFree(GetProcessHeap(), 0, Fibre);
    return RetourValide;
}

VOID __stdcall LireFibre(LPVOID Parametre)
{
    LPFIBERDATASTRUCT fds = (LPFIBERDATASTRUCT)Parametre;
    if (fds == NULL) {
        MsgBox( "Une valeur nulle de fibre a été indiquée", szTitle, MB_ICONERROR | MB_OK, 0x83);
        return;
    }
    AfficherInfoFibre();
    fds->dwBitCompletes = 0;
    while (1)
    {
        if (!ReadFile(fds->hFichier, g_Tampon, Tampon,&dwBitsLus, NULL)){break;}
        if (dwBitsLus == 0) break;
        fds->dwBitCompletes += dwBitsLus;
        SwitchToFiber(g_Fibre[FibreEcriture]);
    } 
    fds->dwResultat = GetLastError();
    SwitchToFiber(g_Fibre[FibrePrimaire]);
}

VOID __stdcall EcrireFibre(LPVOID Parametre)
{
    LPFIBERDATASTRUCT fds = (LPFIBERDATASTRUCT)Parametre;
    DWORD dwBitEcrits = 0;
    if (fds == NULL)
    {
        MsgBox( "La fibre indiquée a une valeur nulle ", szTitle, MB_ICONERROR | MB_OK, 0x83);
        return;
    }
    AfficherInfoFibre();
    fds->dwBitCompletes = 0;
    fds->dwResultat = ERROR_SUCCESS;
    SendDlgItemMessage(hWnd, 0x3ED,PBM_SETRANGE, 0, (LPARAM)dwBitEcrits);
    while (1)
    {
        if (!WriteFile(fds->hFichier, g_Tampon, dwBitsLus,&dwBitEcrits, NULL))
        {
            SendDlgItemMessage(hWnd,0x3ED , PBM_SETPOS, (WPARAM)fds->dwBitCompletes, 0);
            break;
        }
        fds->dwBitCompletes += dwBitEcrits;
        SwitchToFiber(g_Fibre[FibreEcriture]);
    }
    fds->dwResultat = GetLastError();
    SwitchToFiber(g_Fibre[FibrePrimaire]);
}
void AfficherInfoFibre(void)
{
    LPFIBERDATASTRUCT fds = (LPFIBERDATASTRUCT)GetFiberData();
    SetDlgItemInt(hWnd,6000, fds->dwParametre,1);
}