// header.h : fichier Include pour les fichiers Include système standard,
// ou les fichiers Include spécifiques aux projets
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclure les en-têtes Windows rarement utilisés
#define WC_PROGRESS "msctls_progress32"
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>
#include <Commdlg.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>
#include "resource.h"

#pragma comment(lib,"Shell32")
#pragma comment(lib,"Comctl32")
#pragma warning(disable:6001) //évite les warning de convertion 
#pragma warning(disable:4311) //warning tronquage cast
#pragma warning(disable:4312) // convertion INT en HMENU 
#pragma warning(disable:4302) // warning tonquage de valeur
#pragma warning(disable:4172) // warning retour variable locale