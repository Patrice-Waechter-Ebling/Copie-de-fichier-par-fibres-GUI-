// Copie de fichier par fibres(GUI).cpp : Définit le point d'entrée de l'application.
//

#include "framework.h"
#include "Copie de fichier par fibres(GUI).h"
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <Commdlg.h>
#include <commctrl.h>

#define MAX_LOADSTRING 100
#define RetourValide 0
#define RetourFonction 1
#define RetourErreur 0x0D
#define Tampon 0x7FFF
#define nbFibres 3 
#define FibrePrimaire 0 
#define FibreLecture 1
#define FibreEcriture 2 

#pragma comment(lib,"Comctl32")
typedef struct
{
    DWORD dwParameter;
    DWORD dwResultat;
    HANDLE hFile;
    DWORD dwBitCompletes;
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
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

VOID __stdcall LireFibre(LPVOID Parametre);
VOID __stdcall EcrireFibre(LPVOID Parametre);
void AfficherInfoFibre(void);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hPrevInstance,_In_ LPWSTR    lpCmdLine,_In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_COPIEDEFICHIERPARFIBRESGUI, szWindowClass, MAX_LOADSTRING);
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COPIEDEFICHIERPARFIBRESGUI));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(RGB(128, 128, 255));
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_COPIEDEFICHIERPARFIBRESGUI);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm = wcex.hIcon;
    if (!RegisterClassEx(&wcex)) { return false; }
    hWnd = CreateWindowEx(WS_EX_DLGMODALFRAME|WS_EX_WINDOWEDGE,szWindowClass, szTitle, WS_OVERLAPPED |WS_CAPTION |WS_SYSMENU |WS_THICKFRAME,CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd){return FALSE;}
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    CreateStatusWindow(WS_CHILD | WS_VISIBLE, __FILE__, hWnd, 0x1770);
    HMENU mnu = GetSystemMenu(hWnd, 0);
    RemoveMenu(mnu, SC_SIZE, MF_BYCOMMAND);
    RemoveMenu(mnu, SC_MOVE, MF_BYCOMMAND);
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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(wcex.hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

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



int CopieFichier(CHAR* Source, CHAR* Cible)
{
    LPFIBERDATASTRUCT fs;
    char buff[0x64];
    fs = (LPFIBERDATASTRUCT)HeapAlloc(GetProcessHeap(), 0,sizeof(FIBERDATASTRUCT) * nbFibres);
    if (fs == NULL)
    {
        wsprintf(buff,"HeapAlloc error (%d)\n", GetLastError());
        return RetourErreur;
    }

    //
    // Allocate storage for the read/write buffer
    //
    g_Tampon = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, Tampon);
    if (g_Tampon == NULL)
    {
        printf("HeapAlloc error (%d)\n", GetLastError());
        return RetourErreur;
    }

    //
    // Open the source file
    //
    fs[FibreEcriture].hFile = CreateFile(
        argv[1],
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN,
        NULL
    );

    if (fs[FibreEcriture].hFile == INVALID_HANDLE_VALUE)
    {
        printf("CreateFile error (%d)\n", GetLastError());
        return RetourErreur;
    }

    //
    // Open the destination file
    //
    fs[FibreEcriture].hFile = CreateFile(
        argv[2],
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_NEW,
        FILE_FLAG_SEQUENTIAL_SCAN,
        NULL
    );

    if (fs[FibreEcriture].hFile == INVALID_HANDLE_VALUE)
    {
        printf("CreateFile error (%d)\n", GetLastError());
        return RetourErreur;
    }

    //
    // Convert thread to a fiber, to allow scheduling other fibers
    //
    g_Fibre[FibrePrimaire] = ConvertThreadToFiber(&fs[FibrePrimaire]);

    if (g_Fibre[FibrePrimaire] == NULL)
    {
        printf("ConvertThreadToFiber error (%d)\n", GetLastError());
        return RetourErreur;
    }

    //
    // Initialize the primary fiber data structure.  We don't use
    // the primary fiber data structure for anything in this sample.
    //
    fs[FibrePrimaire].dwParameter = 0;
    fs[FibrePrimaire].dwResultat = 0;
    fs[FibrePrimaire].hFile = INVALID_HANDLE_VALUE;

    //
    // Create the Read fiber
    //
    g_Fibre[FibreEcriture] = CreateFiber(0, LireFibre, &fs[FibreEcriture]);

    if (g_Fibre[FibreEcriture] == NULL)
    {
        printf("CreateFiber error (%d)\n", GetLastError());
        return RetourErreur;
    }

    fs[FibreEcriture].dwParameter = 0x12345678;

    //
    // Create the Write fiber
    //
    g_Fibre[FibreEcriture] = CreateFiber(0, EcrireFibre, &fs[FibreEcriture]);

    if (g_Fibre[FibreEcriture] == NULL)
    {
        printf("CreateFiber error (%d)\n", GetLastError());
        return RetourErreur;
    }

    fs[FibreEcriture].dwParameter = 0x54545454;

    //
    // Switch to the read fiber
    //
    SwitchToFiber(g_Fibre[FibreEcriture]);

    //
    // We have been scheduled again. Display results from the 
    // read/write fibers
    //
    printf("ReadFiber: result code is %lu, %lu bytes processed\n",
        fs[FibreEcriture].dwResultat, fs[FibreEcriture].dwBitCompletes);

    printf("WriteFiber: result code is %lu, %lu bytes processed\n",
        fs[FibreEcriture].dwResultat, fs[FibreEcriture].dwBitCompletes);

    //
    // Delete the fibers
    //
    DeleteFiber(g_Fibre[FibreEcriture]);
    DeleteFiber(g_Fibre[FibreEcriture]);

    //
    // Close handles
    //
    CloseHandle(fs[FibreEcriture].hFile);
    CloseHandle(fs[FibreEcriture].hFile);

    //
    // Free allocated memory
    //
    HeapFree(GetProcessHeap(), 0, g_Tampon);
    HeapFree(GetProcessHeap(), 0, fs);

    return RetourValide;
}

VOID __stdcall LireFibre(LPVOID Parametre)
{
    LPFIBERDATASTRUCT fds = (LPFIBERDATASTRUCT)Parametre;
    if (fds == NULL) {
        MessageBox(hWnd, "Une valeur nulle de fibre a été indiquée", szTitle, MB_ICONERROR | MB_OK);
        return;
    }
    AfficherInfoFibre();
    fds->dwBitCompletes = 0;
    while (1)
    {
        if (!ReadFile(fds->hFile, g_Tampon, Tampon,&dwBitsLus, NULL)){break;}
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
    DWORD dwBitEcrits;
    if (fds == NULL)
    {
        MessageBox(hWnd, "La fibre indiquée a une valeur nulle ", szTitle, MB_ICONERROR | MB_OK);
        return;
    }
    AfficherInfoFibre();
    fds->dwBitCompletes = 0;
    fds->dwResultat = ERROR_SUCCESS;
    while (1)
    {
        if (!WriteFile(fds->hFile, g_Tampon, dwBitsLus,&dwBitEcrits, NULL))
        {
            //
            // If an error occurred writing, break
            //
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
    LPVOID lpCurrentFiber = GetCurrentFiber();
    if (lpCurrentFiber == g_Fibre[FibreEcriture])
        printf("Read fiber entered");
    else
    {
        if (lpCurrentFiber == g_Fibre[FibreEcriture])
            printf("Write fiber entered");
        else
        {
            if (lpCurrentFiber == g_Fibre[FibrePrimaire])
                printf("Primary fiber entered");
            else
                printf("Unknown fiber entered");
        }
    }

    //
    // Display dwParameter from the current fiber data structure
    //
    printf(" (dwParameter is 0x%lx)\n", fds->dwParameter);
}