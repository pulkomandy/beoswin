/************************************************************************
 *																		*
 *							BEOS Deskbar Windows 9x						*
 *																		*
 *								 Include File							*
 *																		*
 *						Written by Paul Robson (C) 2001					*
 *																		*
 ************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <cpl.h>
#include <direct.h>
#include <commctrl.h>
#include "resource.h"

#define VERSION		1.03
#define DATE		"7-Mar-01"
											/* Parameter stuff */
#define APPNAME							"BEOS Deskbar"
#define PARAMINT(Name,Def)				GetIntParameter(Name,Def)
#define PARAMSTRING(Name,Def)			GetStringParameter(Name,Def)

#ifdef  _DEBUG								/* Asserting and Fatal Error macros */
#define CHECK(x)	if (!(x)) ReportError(__LINE__,__FILE__)
#else
#define CHECK(x)	{ }
#endif
#define ABORT(x)	if (x) ReportError(__LINE__,__FILE__)

#define DESKBARCLASS	"BEOS_Deskbar_Window_Class"
#define SHORTCUTCLASS	"BEOS_Deskbar_Shortcut_Class"

#define WC_MEMORY		(4)					/* Bytes required in each window */
#define WC_HANDLE		(0)					/* Handle to descriptor global memory */

#define MT_DESKBAR		(0)					/* Menu type (Deskbar) */
#define MT_MENU			(1)				
#define MT_SUBMENU		(2)

#define	ETT_HASARROW	(0x100)				/* Has arrow : popup type */
#define ETT_CANTSELECT	(0x200)				/* Can't be selected */
#define ETT_HASICON		(0x400)				/* Display associated icon */
#define ETT_HASBAR		(0x800)				/* Follow with bar */
#define ETT_NAME		(0x1000)			/* Look up name */
#define ETT_FULLWIDTH	(0x2000)			/* Always use full width */

#define SC_WIDTH		(96)				/* Shortcut icon size */
#define SC_HEIGHT		(64)
											/* Entry types */
#define ET_TITLE		(0 | ETT_HASBAR | ETT_FULLWIDTH)	
#define ET_CLOCK		(1 | ETT_CANTSELECT | ETT_HASBAR | ETT_FULLWIDTH)
#define ET_EMPTY		(2)
#define ET_ABOUT		(3)
#define ET_TASK			(4 | ETT_HASBAR | ETT_HASICON)
#define ET_MENUITEM		(5 | ETT_HASICON | ETT_NAME)
#define ET_SUBMENU		(6 | ETT_HASARROW | ETT_HASICON | ETT_NAME)
#define ET_PROGRAMS		(7 | ETT_HASARROW | ETT_HASICON)
#define ET_RESTART		(8)
#define ET_SHUTDOWN		(9 | ETT_HASBAR)
#define ET_PREFS		(10 | ETT_HASBAR | ETT_HASARROW | ETT_HASICON)
#define ET_PREFITEM		(11)
#define ET_CONFIGURE	(12)
#define ET_SETSHELL		(13 | ETT_HASBAR)

#define ET_ACTIVATE		(13 | ETT_HASBAR | ETT_HASICON)
#define ET_SHOW			(14)
#define ET_HIDE			(15)
#define ET_CLOSE		(16)

#define HASBAR(x)		(((x) & ETT_HASBAR) != 0)
#define FULLWIDTH(x)	(((x) & ETT_FULLWIDTH) != 0)
#define HASICON(x)		(((x) & ETT_HASICON) != 0)
#define HASARROW(x)		(((x) & ETT_HASARROW) != 0)
#define CANSELECT(x)	(((x) & ETT_CANTSELECT) != 0)
#define NAMELOOKUP(x)	(((x) & ETT_NAME) != 0)

#define MAX_ENTRIES		(80)				/* Max items in a Deskbar menu */
#define MAX_TASKS		(64)				/* Max tasks supported */
#define MAX_HIDDEN		(32)				/* Max no of hidden tasks */

#define SM_CLOSEALL		(0x00)				/* Close all menus level > lParam */
#define SM_CLOSESUBMENU	(0x01)				/* Close all sub menus */
#define SM_KILLMENU		(0x02)				/* Close all menus */
#define SM_DESELECTMAIN	(0x04)				/* Deselect main menu */
#define SM_CLOSESHORTCUTS (0x05)			/* Close all shortcuts */
#define SM_DESELECT		(0x06)				/* Deselect a short cut */

#define WMU_SETBAR	(WM_USER+0)				/* Update tick postion */
#define WARNTICK	(150)					/* Rate of warning movement */

typedef struct _TaskEntry					/* Record of a single task */
{
	LONG	aClass;							/* Class atom */
	HWND	hWnd;							/* Window Handle */
} TASKENTRY, *LPTASKENTRY;

typedef struct _TaskList					/* Record of all tasks */
{
	int			Count;						/* No of tasks */
	int			Checksum;					/* Checksum values */
	BOOL		Changed;					/* True if list invalid */
	TASKENTRY	Task[MAX_TASKS];			/* List of tasks */
} TASKLIST;

typedef struct _DeskbarEntry
{
	int Type;								/* Entry type */
	BOOL Selected;							/* Currently selected */
	BOOL IsSelectable;						/* Is Selectable ? */
	BOOL IsIcon;							/* Icon present ? */
	BOOL FollowingBar;						/* Followed by Separator */
	char Text[_MAX_FNAME+1];				/* Text Descriptor */
	char PanelApp[32];						/* Control Panel App Name */
	LPTASKENTRY lpTask;						/* Task information */
} ENTRY;

typedef struct _DeskbarMenu					/* Deskbar menu structure */
{
	int xSize;								/* Width */
	int ySize;								/* Height of one item */
	int Type;								/* Menu type */
	int TotalItems;							/* Total number of items */
	int VisCount;							/* Visible Count */
	int TopDisplayItem;						/* Topmost displayed item */
	int Selected;							/* Selected Item */
	int OpenItem;							/* Open Item (Deskbar only) */
	int Level;								/* Menu Level */
	int OverItem;							/* Item we are over */
	HWND hWndMain;							/* Owner window */
	BOOL TopArrow,BottomArrow;				/* Top/Bottom Arrow Present ? */
	HINSTANCE hInstance;					/* Application Instance */
	char  Directory[MAX_PATH+1];			/* Associated Directory */
	ENTRY Menu[MAX_ENTRIES];				/* Menu entries */

} DESKBARMENU,* LPDESKBARMENU;

LRESULT CALLBACK DeskbarWindowProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam);
HWND CreateDeskbarMenu(HANDLE hInst,int Type,int xRight,int y,char *Directory,int Level,HWND hWndMain);
LPDESKBARMENU LockDeskbarMemory(HWND hWnd);
LPDESKBARMENU UnlockDeskbarMemory(HWND hWnd);
int AddDeskbarItem(HWND hWnd,int Type);
void DrawDeskbarItem(HDC hDC,LPRECT rc,LPDESKBARMENU lpMem,int nDraw,BOOL IsLast);
void BevelFrame(HDC hDC,LPRECT rc,BOOL Raised);
void DrawFilenameIcon(HDC hDC,HINSTANCE hInst,LPSTR lpFile,int x,int y,BOOL bSmall);
void AppendDeskbarDirectory(HWND hWnd,LPSTR lpDirectory);
void RecalculateMenuSize(HWND hWnd);
BOOL SortMenuData(LPDESKBARMENU lpMenu);
int MenuCompare(ENTRY *e1,ENTRY *e2);
void DrawArrow(HDC hDC,LPRECT rc,BOOL IsUp);
LPRECT GetEntryRect(int i,LPRECT rc,LPDESKBARMENU lpMem);
void GetMenuPosition(LPPOINT pt,LPDESKBARMENU lpMem,HWND hWnd,int n);
int  GetIntParameter(LPSTR,int);
LPSTR GetStringParameter(LPSTR,LPSTR);
void WriteStringParameter(LPSTR lpKey,LPSTR lpValue);
void WriteIntParameter(LPSTR lpKey,int n);
void ReportError(int nLine,LPSTR lpFile);
void AddPanelApplets(HWND hWnd,LPSTR lpName,LPDESKBARMENU lpMenu);
void GetAppletPath(LPSTR lpName,LPSTR lpPath);
BOOL FAR PASCAL CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL CALLBACK WarningDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
BOOL ShutDownTest(HWND hWnd,BOOL IsShutDown);
void CollectTaskWindows(HANDLE hInst,BOOL bUpdate);
void UpdateDeskbarMenuTasks(HWND hWnd);
BOOL CALLBACK _EnumWindows(HWND hWnd,LPARAM lParam);
void Install(HANDLE hInstance,LPSTR lpCmdLine);
void SetShell(HANDLE hInstance);
BOOL FAR PASCAL CALLBACK SetShellDlg(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
void InitShortcut(HANDLE hInstance);     
void EndShortcut(void);
LRESULT CALLBACK ShortcutWindowProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam);
HWND CreateShortCut(HANDLE hInstance,LPSTR lpFile,int x,int y);
void CopyShortcuts(HANDLE hInstance);
void ExecuteApplication(HWND hWnd,LPSTR lpFile,LPSTR lpDir);
void FindHiddenTasks(HANDLE hInstance);
BOOL CALLBACK _EnumHiddenWindows(HWND hWnd,LPARAM lParam);
BOOL IsAppWindow(HWND hWnd);
void StartupExecute(HANDLE hInstance);

extern HPEN hGreyPen;
extern int  LockCount,AllocCount,WinCount;
extern int xDefSize,yDefSize,yArrowSize;
extern UINT msgControl;
extern TASKLIST Tasks;
extern HFONT hFont;
extern HWND hWndDeskbar;
extern HWND hWndTaskBar;