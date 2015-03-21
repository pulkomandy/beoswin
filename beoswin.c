/************************************************************************
 *																		*
 *							BEOS Deskbar Windows 9x						*
 *																		*
 *						Written by Paul Robson (C) 2001					*
 *																		*
 ************************************************************************/

// TODO:: Recent Documents menu
// TODO:: Recent Apps menu
// TODO:: Window Skinning Application.
// TODO:: Alternate Background ?

#include "beoswin.h"						/* All the definitions */

int		xDefSize,yDefSize;					/* Size of the menu items */
int     yArrowSize;							/* Size of arrow space */
int     WinCount = 0;						/* Window Count */
HWND	hWndDeskbar;						/* Deskbar window handle */
HPEN	hGreyPen;							/* Gray pen */
HFONT	hFont;								/* Text font */
UINT	msgControl;							/* Registered Message */
int     LockCount = 0,AllocCount = 0;		/* Lock/Alloc Checks */
TASKLIST Tasks;								/* Task list */
HWND	hWndTaskBar = NULL;					/* Explorer window handle ? */
int     HiddenCount;						/* Tasks hidden at start up */
LONG    HiddenTask[MAX_HIDDEN];				/* Hidden task instances */

/************************************************************************/
/*							  Main Window Program						*/
/************************************************************************/

int WINAPI WinMain(HANDLE hInstance,HANDLE hPrevInst,LPSTR lpCmdLine,int nCmdShow)
{
	MSG Msg;
	WNDCLASS wc;
	char *FontName;
	RECT rc,rcWorkArea;
	HWND hWndTask;

	if (hPrevInst != NULL) return(0);		/* Only one instance of application */
	Install(hInstance,lpCmdLine);
	hWndTask = FindWindow("Shell_TrayWnd",NULL);
	#ifndef _DEBUG
											/* Maybe hide the task bar */
	if (PARAMINT("HideExplorerTaskBar",1) != 0)
										hWndTaskBar = hWndTask;
	#endif
	if (hWndTaskBar != NULL)
		ShowWindow(hWndTaskBar,SW_HIDE);	/* Hide taskbar */

	rc.left = 0;							/* rc = whole screen - bit at top */
	rc.top = PARAMINT("TopSpace",16);		/* is kept free so you can see the menu ! */
	rc.right = GetSystemMetrics(SM_CXSCREEN);
	rc.bottom = GetSystemMetrics(SM_CYSCREEN);
	/* Save old and restore new : maybe */
	ABORT(!SystemParametersInfo(SPI_GETWORKAREA,0,&rcWorkArea,0));
	if (PARAMINT("DontResizeScreenArea",0) == 0)
			ABORT(!SystemParametersInfo(SPI_SETWORKAREA,0,&rc,0));

	if (hWndTask == NULL)
		StartupExecute(hInstance);

	msgControl =							/* Message used to talk in application */
			RegisterWindowMessage("Win95_BEOSDeskbar_Intercommunication");
	ABORT(msgControl == 0);

	InitCommonControls();					/* Load COMMDLG32.DLL */

	yDefSize = PARAMINT("MenuHeight",24);	/* Size of a menu entry */
	xDefSize = PARAMINT("MenuWidth",yDefSize*7);
	yArrowSize = PARAMINT("ArrowSize",16);	/* Size of arrow at top and bottom if reqd */

	wc.style = 0;							/* Register Window Class */
	wc.lpfnWndProc = DeskbarWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = WC_MEMORY;				/* Allocate some memory for the handle */
	wc.hInstance = hInstance;
	wc.hbrBackground = GetStockObject(LTGRAY_BRUSH);
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_APPICON));
	wc.lpszMenuName = NULL;
	wc.lpszClassName = DESKBARCLASS;
	ABORT(RegisterClass(&wc) == 0);

											/* This is the font that is used for menus */
	FontName = PARAMSTRING("Font","Arial");
	hFont = CreateFont(yDefSize*PARAMINT("FontScale",55)/100,0,		
					   0,0,PARAMINT("FontBold",FW_NORMAL),
					   FALSE,FALSE,FALSE,
					   ANSI_CHARSET,OUT_CHARACTER_PRECIS,
					   CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
					   FF_DONTCARE,FontName);

	hGreyPen = CreatePen(PS_SOLID,			/* Pen is used to draw bevel frames */
							1,RGB(64,64,64));
	ABORT(hGreyPen == NULL);

	hWndDeskbar =							/* Create the original deskbar window */
		CreateDeskbarMenu(hInstance,MT_DESKBAR,GetSystemMetrics(SM_CXSCREEN),0,NULL,0,NULL);
	ABORT(hWndDeskbar == NULL);
	AddDeskbarItem(hWndDeskbar,ET_TITLE); 
	AddDeskbarItem(hWndDeskbar,ET_CLOCK);
	FindHiddenTasks(hInstance);
	CollectTaskWindows(hInstance,TRUE);		/* Load up all the tasks */
	UpdateDeskbarMenuTasks(hWndDeskbar);	/* Add the tasks on */

	ShowWindow(hWndDeskbar,SW_SHOWNORMAL);	/* Show the original window */
	InitShortcut(hInstance);				/* Create all the shortcuts */

	while (GetMessage(&Msg,NULL,0,0))		/* Simple Message Pump */
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}	
	DeleteObject(hGreyPen);					/* Throw away objects */
	DeleteObject(hFont);

	if (hWndTaskBar != NULL)				/* Recall hidden taskbar and set work area */
		ShowWindow(hWndTaskBar,SW_SHOW);
	EndShortcut();							/* End all the short cuts */

	ABORT(!SystemParametersInfo(SPI_SETWORKAREA,0,&rcWorkArea,0));
	ABORT(LockCount != 0);					/* Check stuff */
	ABORT(AllocCount != 0);
	return(0);
}

/************************************************************************/
/*				  Create a new, empty deskbar object					*/
/************************************************************************/

HWND CreateDeskbarMenu(HANDLE hInst,int Type,int xRight,int y,
									char *Directory,int Level,HWND hWndMain)
{
	HWND hWnd;
	HGLOBAL hMem;
	LPDESKBARMENU lpMem;
	int xLeft;

	CHECK(hInst != NULL);

	hMem = GlobalAlloc(GMEM_MOVEABLE		/* Allocate memory */
							| GMEM_ZEROINIT,sizeof(DESKBARMENU));
	ABORT(hMem == NULL);
	AllocCount++;

	xLeft = xRight-xDefSize;				/* Calculate starting position */
	if (xLeft < 0) xLeft = 0;
	
	hWnd = CreateWindow(DESKBARCLASS,		/* Create the window */
						"Deskbar",
						Type == MT_DESKBAR ? WS_POPUP : WS_POPUP|WS_CHILD,
						xLeft,y,xDefSize,yDefSize,
						hWndMain,NULL,hInst,NULL);
	ABORT(hWnd == NULL);
	SetWindowLong(hWnd,WC_HANDLE,			/* Store the data handle */
							(LONG)hMem);
	
	lpMem = LockDeskbarMemory(hWnd);		/* Lock it to copy information */
	lpMem->Type = Type;
	lpMem->VisCount = 0;
	lpMem->hInstance = hInst;
	lpMem->TopArrow = 
			lpMem->BottomArrow = FALSE;
	lpMem->Selected = -1;
	lpMem->Level = Level;
	lpMem->hWndMain =						/* if NULL this is main window, else copy */
			(hWndMain == NULL ? hWnd : hWndMain);
	lpMem->OpenItem = -1;					/* Deskbar open flag = none */
	lpMem->xSize = xDefSize;
	lpMem->ySize = yDefSize;
	if (Directory != NULL)					
			strcpy(lpMem->Directory,Directory);
	lpMem = UnlockDeskbarMemory(hWnd);		/* Unlock it */
	return(hWnd);
}

/************************************************************************/
/*					Add a simple deskbar menu item						*/
/************************************************************************/

int AddDeskbarItem(HWND hWnd,int Type)
{
	LONG n;
	int  MaxMenu;
	LPDESKBARMENU lpDeskbar;
	RECT rc;
	CHECK(IsWindow(hWnd));
	lpDeskbar = LockDeskbarMemory(hWnd);	/* Lock deskbar memory */
	n = lpDeskbar->TotalItems++;			/* Bump total number of items */
	lpDeskbar->Menu[n].Type = Type;			/* Store the basic type */
	lpDeskbar->Menu[n].FollowingBar =		/* Entry underlined */
						HASBAR(Type);
	lpDeskbar->Menu[n].IsIcon =				/* Has an icon ? */
						HASICON(Type);
	lpDeskbar->Menu[n].IsSelectable =		/* Can select all except clock */
						!(CANSELECT(Type));

	switch(Type)
	{
	case ET_ABOUT:
			strcpy(lpDeskbar->Menu[n].Text,"About BEOS...");
			break;
	case ET_PROGRAMS:
			strcpy(lpDeskbar->Menu[n].Text,"Applications");
			GetWindowsDirectory(lpDeskbar->Directory,sizeof(lpDeskbar->Directory));
			strcat(lpDeskbar->Directory,"\\start menu\\programs");
			break;
	case ET_SHOW:
			strcpy(lpDeskbar->Menu[n].Text,"Show");
			break;
	case ET_HIDE:
			strcpy(lpDeskbar->Menu[n].Text,"Hide");
			break;
	case ET_CLOSE:
			strcpy(lpDeskbar->Menu[n].Text,"Close");
			break;
	case ET_PREFS:
			strcpy(lpDeskbar->Menu[n].Text,"Preferences");
			break;
	case ET_RESTART:
			strcpy(lpDeskbar->Menu[n].Text,"Restart");
			break;
	case ET_SHUTDOWN:
			strcpy(lpDeskbar->Menu[n].Text,"Shutdown");
			break;
	case ET_CONFIGURE:
			strcpy(lpDeskbar->Menu[n].Text,"Configure BEOS Menu...");
			break;
	case ET_SETSHELL:
			strcpy(lpDeskbar->Menu[n].Text,"Configure Windows Shell...");
			break;
	default:
			strcpy(lpDeskbar->Menu[n].Text,"cxxx");	
			break;
	}
	
	GetWindowRect(hWnd,&rc);
	MaxMenu =								/* How much can we fit ? */
		(GetSystemMetrics(SM_CYSCREEN)-rc.top)/lpDeskbar->ySize;
	n++;									/* No of items not index */
	if (n >= MaxMenu-1)						/* No more space on screen */
		lpDeskbar->BottomArrow = TRUE;		/* Bottom arrow on */
	else
		lpDeskbar->VisCount = n;			/* Else update visible total */
	RecalculateMenuSize(hWnd);				/* Work out size & pos */
	lpDeskbar = UnlockDeskbarMemory(hWnd);
	return(n-1);							/* Return index in array */
}

/************************************************************************/
/*					  Recalculate the menu size							*/
/************************************************************************/

void RecalculateMenuSize(HWND hWnd)
{
	LPDESKBARMENU lpDeskbar;
	int i,n,xOrg;
	HDC hDC;
	char *s;
	SIZE sz;
	HFONT hOldFont;
	RECT rc;

	CHECK(IsWindow(hWnd));
	GetWindowRect(hWnd,&rc);				/* Get position */
	lpDeskbar = LockDeskbarMemory(hWnd);	/* Lock the deskbar memory */
	n=lpDeskbar->ySize*lpDeskbar->VisCount;	/* Size of menu bit */
	if (lpDeskbar->TopArrow) n+= yArrowSize;/* Add for arrows */
	if (lpDeskbar->BottomArrow) n+= yArrowSize;

	if (lpDeskbar->Type != MT_DESKBAR)		/* Check menu width */
	{
		xOrg = lpDeskbar->xSize;
		hDC = GetDC(hWnd);					/* Get context for menu */
		ABORT(hDC == NULL);
		hOldFont = SelectObject(hDC,hFont);	/* Select working font */
		for (i = 0;							/* Work through all items */
				i < lpDeskbar->TotalItems;i++)
		{	
			s = lpDeskbar->Menu[i].Text;	/* Get the text */
			GetTextExtentPoint32(hDC,		/* Work out the size */
								s,strlen(s),&sz);
			sz.cx += 16+4+16+2+16;			/* Add the edges */
			if (lpDeskbar->xSize < sz.cx)	/* Bigger than the widest ? */
							lpDeskbar->xSize = sz.cx;
		}
		SelectObject(hDC,hOldFont);			/* Restore old font */
		ReleaseDC(hWnd,hDC);
		rc.left = rc.left -					/* Adjust left edge */
					(lpDeskbar->xSize-xOrg);
		if (rc.left < 0) rc.left = 0;		/* Can't go too far... */
	}
	SetWindowPos(hWnd,NULL,rc.left,rc.top,	/* Resize */
				lpDeskbar->xSize,n,SWP_NOZORDER);		
	InvalidateRect(hWnd,NULL,TRUE);			/* Force redraw */
	lpDeskbar = UnlockDeskbarMemory(hWnd);
}

/************************************************************************/
/*							Draw deskbar item							*/
/************************************************************************/

void DrawDeskbarItem(HDC hDC,LPRECT rc,LPDESKBARMENU lpMenu,int nDraw,BOOL IsLast)
{
	char s[_MAX_PATH],Name[_MAX_FNAME];
	BOOL bGreyMenu,bHighlight;
	HDC hdcBitmap;
	HBITMAP hBmp,hOldBmp;
	BITMAP BmInfo;
	HBRUSH hBrush,hOldBrush;
	HPEN hOldPen;
	HICON hIcon;
	SIZE sz;
	HFONT hOldFont;
	int n,x,y;
	SYSTEMTIME tm;
	RECT rcLine;
	POINT pt;
	HWND hWndTask;

	if (lpMenu->Menu[nDraw].FollowingBar &&	/* Draw following split bar if required */
										!IsLast)
	{
		rcLine = *rc;						/* Work out rectangle for split */
		rcLine.top = rcLine.bottom-2;
		rcLine.left += 5;
		rcLine.right -= 5;
		BevelFrame(hDC,&rcLine,TRUE);		/* Draw it */
		rc->bottom -= 2;					/* Adjust for unused space */
	}

	bHighlight = lpMenu->Menu[nDraw].Selected || lpMenu->OpenItem == nDraw;
	hOldBrush = SelectObject(hDC,			/* Set up to fill in the rectangle */
			GetStockObject(bHighlight ? GRAY_BRUSH:LTGRAY_BRUSH));
	hOldPen = SelectObject(hDC,GetStockObject(NULL_PEN));
	Rectangle(hDC,rc->left,rc->top,rc->right,rc->bottom);
	SelectObject(hDC,hOldPen);
	SelectObject(hDC,hOldBrush);

	if (lpMenu->Menu[nDraw].IsIcon)			/* If has an icon */
	{
		pt.x = rc->left+4;					/* Work out position */
		pt.y = (rc->top+rc->bottom-16)/2;
		switch(lpMenu->Menu[nDraw].Type)	/* Draw appropriate icon */
		{
		case ET_TASK:						/* Task, read from class data */
		case ET_ACTIVATE:
			hWndTask = lpMenu->Menu[nDraw].lpTask->hWnd;
			hIcon = (HICON)GetClassLong(hWndTask,GCL_HICON);
			if (hIcon != NULL) 
				DrawIconEx(hDC,pt.x,pt.y,hIcon,16,16,0,NULL,DI_NORMAL);
			break;
		case ET_SUBMENU:					/* Submenu, standard */
			DrawIcon(hDC,pt.x,pt.y,LoadIcon(lpMenu->hInstance,MAKEINTRESOURCE(IDI_FOLDER16)));
			break;
		case ET_PROGRAMS:					/* Programs, standard */
			DrawIcon(hDC,pt.x,pt.y,LoadIcon(lpMenu->hInstance,MAKEINTRESOURCE(IDI_APP16)));
			break;
		case ET_PREFS:						/* Preferences, standard */
			DrawIcon(hDC,pt.x,pt.y,LoadIcon(lpMenu->hInstance,MAKEINTRESOURCE(IDI_PREFS16)));
			break;
		case ET_MENUITEM:					/* Menu item, from file */
			strcpy(s,lpMenu->Directory);	/* Make a file name */
			if (s[0] != '\0')
			{
				if (s[strlen(s)-1] != '\\') strcat(s,"\\");
				strcat(s,lpMenu->Menu[nDraw].Text);
				DrawFilenameIcon(hDC,		/* Draw the icon */
								 lpMenu->hInstance,
								 s,pt.x,pt.y,TRUE);
			}
			break;
		default:
			break;
		}
	}

	n = lpMenu->Menu[nDraw].Type;			/* Reduce size of draw area for most types */
	if (!FULLWIDTH(n))	rc->left += 8 + 16;

	switch(lpMenu->Menu[nDraw].Type)		/* Drawing the menu entry */
	{
	case ET_TITLE:							/* Title */
		hdcBitmap = CreateCompatibleDC(hDC);/* DC for Bitmap */
		ABORT(hdcBitmap == NULL);
		hBmp = LoadBitmap(lpMenu->hInstance,/* Load in */
						MAKEINTRESOURCE(IDB_TITLEBAR));
		ABORT(hBmp == NULL);
		GetObject(hBmp,sizeof(BITMAP),		/* Sizing information */
								&BmInfo);
		hOldBmp=SelectObject(hdcBitmap,hBmp);/* Select into DC */
		x = (rc->right-rc->left)*35/100;	/* Where it will end up */
		y = (rc->bottom-rc->top)/10;
		StretchBlt(hDC,						/* Draw "BEOS" Bitmap onto area */
				   rc->left+x,rc->top+y,rc->right-rc->left-x*2,rc->bottom-rc->top-y*2,
				   hdcBitmap,0,0,BmInfo.bmWidth,BmInfo.bmHeight,
				   SRCCOPY);
		SelectObject(hdcBitmap,hOldBmp);	/* Restore DC bitmap and chuck it */
		DeleteDC(hdcBitmap);
		DeleteObject(hBmp);
		break;

	case ET_EMPTY:							/* Empty just prints that. */
		hOldFont = SelectObject(hDC,hFont);
		SetBkMode(hDC,TRANSPARENT);
		SetTextColor(hDC,RGB(0,0,0));
		DrawText(hDC,"(empty)",7,rc,DT_VCENTER | DT_SINGLELINE);		
		SelectObject(hDC,hOldFont);
		break;

	case ET_CLOCK:							/* Clock */
		rc->left += 3;rc->right -= 5;		/* Dimensions of inner frame */
		rc->top += 3;rc->bottom -= 3;
		hBrush =							/* Inside is dark grey */
			CreateSolidBrush(RGB(160,160,160));
		ABORT(hBrush == NULL);
		hOldBrush = SelectObject(hDC,hBrush);
		Rectangle(hDC,rc->left,				/* Draw it */
				rc->top,rc->right+1,rc->bottom);
		SelectObject(hDC,hOldBrush);
		DeleteObject(hBrush);
		BevelFrame(hDC,rc,FALSE);			/* Bevel it */
		GetLocalTime(&tm);					/* Get time, make into a string */
		sprintf(s,"%d:%02d %cM",tm.wHour %12,tm.wMinute,tm.wHour >= 12 ? 'P':'A');
		SetBkMode(hDC,TRANSPARENT);			/* Print it out */
		hOldFont = SelectObject(hDC,hFont);
		GetTextExtentPoint32(hDC,s,strlen(s),&sz);
		SetTextColor(hDC,RGB(0,0,0));
		TextOut(hDC,rc->right-sz.cx-3,(rc->top+rc->bottom-sz.cy)/2,s,strlen(s));
		SelectObject(hDC,hOldFont);
		break;

	default:
											/* Submenus have the arrow icon */
		if (HASARROW(lpMenu->Menu[nDraw].Type))
		{
			hIcon = LoadIcon(lpMenu->hInstance,MAKEINTRESOURCE(IDI_ARROW16));
			DrawIcon(hDC,rc->right-2-16,(rc->top+rc->bottom-16)/2,hIcon);
		}
		rc->right = rc->right-4-16;
											/* Select font in and get the name */
		hOldFont = SelectObject(hDC,hFont);
		_splitpath(lpMenu->Menu[nDraw].Text,s,s,Name,s);	
		bGreyMenu = FALSE;					/* Grey certain menus */
		if (lpMenu->Menu[nDraw].Type == ET_SHOW)
			if (IsWindowVisible(lpMenu->Menu[nDraw].lpTask->hWnd)) bGreyMenu = TRUE;
		if (lpMenu->Menu[nDraw].Type == ET_HIDE)
			if (!IsWindowVisible(lpMenu->Menu[nDraw].lpTask->hWnd)) bGreyMenu = TRUE;
		if (lpMenu->Menu[nDraw].Type == ET_EMPTY) bGreyMenu = TRUE;

		SetBkMode(hDC,TRANSPARENT);			/* Set up and draw it */
		SetTextColor(hDC,RGB(0,0,0));
		if (bGreyMenu)SetTextColor(hDC,RGB(128,128,128));
		DrawText(hDC,Name,strlen(Name),rc,DT_VCENTER | DT_SINGLELINE);		
		SelectObject(hDC,hOldFont);
		break;
	}
}

/************************************************************************/
/*			Fill the menu with items from the directory					*/
/************************************************************************/

void AppendDeskbarDirectory(HWND hWnd,LPSTR lpDirectory)
{
	LPDESKBARMENU lpMenu;
	struct _finddata_t FileInfo;
	long lHandle;
	int n;
	BOOL bCtrl = (strcmp(lpDirectory,"[PANEL]") == 0);
	lpMenu = LockDeskbarMemory(hWnd);		/* Start work */
	if (*lpDirectory == '\0') return;
	if (lpDirectory[strlen(lpDirectory)-1] != '\\')
									strcat(lpDirectory,"\\");
	strcpy(lpMenu->Directory,lpDirectory);	/* Set the current directory */
	strcat(lpDirectory,"*.*");
	if (bCtrl)								/* Special case if control panel */
	{
		GetWindowsDirectory(lpDirectory,_MAX_PATH);
		strcat(lpDirectory,"\\system\\*.cpl");
	}
	lHandle = _findfirst(lpDirectory,		/* Read the handle */
								&FileInfo);
	while (lHandle != -1)					/* While more files... */
	{										/* Add entry if not junk */
		if (strcmp(FileInfo.name,".") != 0 &&
			strcmp(FileInfo.name,"..") != 0)
		{			
			if (!bCtrl)						/* Handle normal */
			{
				n = AddDeskbarItem(hWnd,ET_MENUITEM);
				strcpy(lpMenu->Menu[n].Text,FileInfo.name);
				if (FileInfo.attrib & _A_SUBDIR)
					lpMenu->Menu[n].Type = ET_SUBMENU;
			}
			else							/* Handle control panel stuff */
			{
				AddPanelApplets(hWnd,FileInfo.name,lpMenu);
			}
		}
		if (_findnext(lHandle,				/* Look for next matching file */
					&FileInfo) < 0) lHandle = -1;
	}

	if (lpMenu->TotalItems > 1)				/* Sort it */
		while (!SortMenuData(lpMenu)) {};

	if (lpMenu->TotalItems == 0)			/* Special case if empty */
			AddDeskbarItem(hWnd,ET_EMPTY);

	lpMenu = UnlockDeskbarMemory(hWnd);
}

/************************************************************************/
/*							Sort the menu								*/
/************************************************************************/

BOOL SortMenuData(LPDESKBARMENU lpMenu)
{
	BOOL IsSorted = TRUE;
	int i;
	ENTRY Temp;								/* Simple multi pass sort */
	CHECK(lpMenu != NULL);
	for (i = 0;i < lpMenu->TotalItems-1;i++)
	{
		if (MenuCompare(&(lpMenu->Menu[i]),&(lpMenu->Menu[i+1])) > 0)
		{
			IsSorted = FALSE;
			Temp = lpMenu->Menu[i];
			lpMenu->Menu[i] = lpMenu->Menu[i+1];
			lpMenu->Menu[i+1] = Temp;
		}
	}
	return(IsSorted);
}

/************************************************************************/
/*						Compare two elements							*/
/************************************************************************/

#define ISSORTTYPE(x) ((x) == ET_SUBMENU || (x) == ET_MENUITEM || (x) == ET_PREFITEM || (x) == ET_TASK)

int MenuCompare(ENTRY *e1,ENTRY *e2)
{
	CHECK(e1 != NULL);
	CHECK(e2 != NULL);
	if (!ISSORTTYPE(e1->Type)) return(0);	/* Only sort menus/programs/prefs/tasks */
	if (!ISSORTTYPE(e2->Type)) return(0);
	if (e1->Type != e2->Type)
		return (e1->Type == ET_MENUITEM ? 1 : 0);
	return(stricmp(e1->Text,e2->Text));
}	

/************************************************************************/
/*					Draw the up down arrow thing						*/
/************************************************************************/

void DrawArrow(HDC hDC,LPRECT rc,BOOL IsUp)
{
	POINT ptArray[4];
	int x,y;
	HPEN hOldPen = SelectObject(hDC,		/* First draw the rectangle */
									GetStockObject(NULL_PEN));
	HBRUSH hOldBrush = SelectObject(hDC,GetStockObject(GRAY_BRUSH));
	Rectangle(hDC,rc->left,rc->top,rc->right,rc->bottom);
	x = rc->right-rc->left;y = rc->bottom-rc->top;

	ptArray[0].x = rc->left+x*45/100;		/* Work out polygon coordinates */
	ptArray[1].x = rc->left+x/2;
	ptArray[2].x = rc->left+x*55/100;	
											/* Fix for arrow direction */
	ptArray[0].y = rc->top + y * (IsUp ? 80:20)/100;
	ptArray[1].y = rc->top*2+y-ptArray[0].y;
	ptArray[2].y = ptArray[0].y;
	ptArray[3] = ptArray[0];
											/* Draw the polygon */
	SelectObject(hDC,GetStockObject(DKGRAY_BRUSH));
	Polygon(hDC,ptArray,4);
	SelectObject(hDC,hOldBrush);
	SelectObject(hDC,hOldPen);
}

/************************************************************************/
/*			Get rectangle for given position on menu window				*/
/************************************************************************/

LPRECT GetEntryRect(int i,LPRECT rc,LPDESKBARMENU lpMem)
{
	rc->left = 1;							/* horizontal, easy */
	rc->right = lpMem->xSize-1;
	rc->top = i*lpMem->ySize;				/* top start */
	if (lpMem->TopArrow)					/* adjust if a top arrow */
				rc->top += yArrowSize;
	rc->bottom = rc->top+lpMem->ySize;		/* bottom position */
	if (i == 0) rc->top++;					/* adjust for frame */
	if (i == lpMem->VisCount) rc->bottom--;
	return(rc);
}

/************************************************************************/
/*					Work out position of next menu						*/
/************************************************************************/

void GetMenuPosition(LPPOINT pt,LPDESKBARMENU lpMem,HWND hWnd,int n)
{
	int yMax;
	pt->x = PARAMINT("MenuXOverlap",0);		/* Offset x maybe */
	pt->y =									/* Fix Y for selection n */
			((lpMem->TopArrow) ? yArrowSize : 0)+(n-lpMem->TopDisplayItem)*lpMem->ySize;
	ClientToScreen(hWnd,pt);				/* Make them screen coordinates */
	yMax = GetSystemMetrics(SM_CYSCREEN)*2/3;
	if (pt->y > yMax) pt->y -= lpMem->ySize*2;
}

/************************************************************************/
/*			   Add Applets for given control panel item					*/
/************************************************************************/

void AddPanelApplets(HWND hWnd,LPSTR lpName,LPDESKBARMENU lpMenu)
{
	int i,n,Total;
	char _Path[_MAX_PATH];
	HINSTANCE hLib;
	APPLET_PROC CplCall;
	NEWCPLINFOA Info;
	CPLINFO	Info2;

	GetAppletPath(lpName,_Path);			/* Get full path name */
	hLib = LoadLibrary(_Path);				/* Load in the library */
	if (hLib == NULL) return;				/* Couldn't load it */

	CplCall = (APPLET_PROC)					/* Extract CPlApplet() function */
					GetProcAddress(hLib,"CPlApplet");
	if (CplCall == NULL)					/* Exit if not found */
	{
		FreeLibrary(hLib);
		return;
	}
	n = CplCall(NULL,CPL_INIT,0,0);			/* Initialise the *.CPL file */
	if (n == 0)
	{
		FreeLibrary(hLib);					/* Initialisation failed */
		return;
	}
	Total = CplCall(NULL,CPL_GETCOUNT,0,0);	/* How many are there ? */
	for (i = 0;i < Total;i++)				/* Get them all ? */
	{
		Info.szName[0] = '\0';
		if (CplCall(NULL,CPL_NEWINQUIRE,i,(LONG)&Info) != 0)
		{									/* Two methods ! */
			CplCall(NULL,CPL_INQUIRE,i,(LONG)&Info2);
			LoadStringA(hLib,Info2.idName,Info.szName,sizeof(Info.szName));
		}
		if (Info.szName[0] != '\0')			/* Add if found */
		{
			n = AddDeskbarItem(hWnd,ET_PREFITEM);
			strcpy(lpMenu->Menu[n].Text,Info.szName);
			strcpy(lpMenu->Menu[n].PanelApp,lpName);
		}
	}	
	CplCall(NULL,CPL_EXIT,0,0);				/* End the *.CPL file */
	FreeLibrary(hLib);						/* Free the library */
}

/************************************************************************/
/*						Collect all the tasks							*/
/************************************************************************/

static TASKLIST New;
    
void CollectTaskWindows(HANDLE hInst,BOOL bUpdate)
{
	int i;
	char _Buffer[_MAX_PATH],*s;

	New.Count = 0;						/* Reload all the tasks */
	EnumWindows(_EnumWindows,0L);
	New.Checksum = 0;
	Tasks.Changed = FALSE;

	for (i = 0;i < New.Count;i++)		/* Checksum the task information */
	{
		New.Checksum += (int)(New.Task[i].hWnd);
		New.Checksum += New.Task[i].aClass;
		GetWindowText(New.Task[i].hWnd,_Buffer,sizeof(_Buffer));
		s = _Buffer;
		while (*s != '\0')
			New.Checksum += (int)(*s++);
	}
	New.Changed = (Tasks.Checksum != New.Checksum) | bUpdate;
	if (New.Changed) Tasks = New;
}

/************************************************************************/
/*						Update tasks in the deskbar						*/
/************************************************************************/

void UpdateDeskbarMenuTasks(HWND hWnd)
{
	LPDESKBARMENU lpMem;
	int i,n,nMin;

	lpMem = LockDeskbarMemory(hWnd);		/* Access deskbar memory */
	SendMessage(HWND_BROADCAST,				/* Close all open menus */
				msgControl,SM_KILLMENU,0L);
	nMin = 2;
	lpMem->TotalItems = nMin;				/* Reset total items, visible count to that */
	lpMem->VisCount = nMin;
	for (i = 0;i < Tasks.Count;i++)			/* Add all the loaded items */
	{
		n = AddDeskbarItem(hWnd,ET_TASK);	/* Add a task */
		GetWindowText(Tasks.Task[i].hWnd,	/* Read the text descriptor */
					  lpMem->Menu[n].Text,sizeof(lpMem->Menu[n].Text));
											/* Store pointer to task info and set icon flag */
		lpMem->Menu[n].lpTask = &(Tasks.Task[i]);
		lpMem->Menu[n].IsIcon = TRUE;		
	}
	if (lpMem->TotalItems > 1)				/* Sort it */
		while (!SortMenuData(lpMem)) {};
	lpMem = UnlockDeskbarMemory(hWnd);		/* Tidy up */
	RecalculateMenuSize(hWnd);				/* Calculate the menu size again */
}

/************************************************************************/
/*					  Check the windows for tasks						*/
/************************************************************************/

BOOL CALLBACK _EnumWindows(HWND hWnd,LPARAM lParam)
{
	BOOL bAllow = TRUE;
	int n;

	bAllow = IsAppWindow(hWnd);				/* A useable application ? */

											/* Check if hidden tasks on the list from start */
	if (bAllow & !(GetWindowLong(hWnd,GWL_STYLE) & WS_VISIBLE))
		for (n = 0;n < HiddenCount;n++)
			if (GetWindowLong(hWnd,GWL_WNDPROC) == HiddenTask[n]) bAllow = FALSE;

	if (bAllow)								/* If allowed, add it to the list */
	{
		n = New.Count++;					/* One more */
		New.Task[n].hWnd = hWnd;			/* Save window handle,class atom */
		New.Task[n].aClass = GetClassLong(hWnd,GCW_ATOM);
	}
	return(New.Count < MAX_TASKS);
}

/************************************************************************/
/*				 Find all tasks hidden at this point					*/
/************************************************************************/

void FindHiddenTasks(HANDLE hInstance)
{	
	HiddenCount = 0;
	EnumWindows(_EnumHiddenWindows,0L);
}

/************************************************************************/
/*					Count all the hidden windows						*/
/************************************************************************/

BOOL CALLBACK _EnumHiddenWindows(HWND hWnd,LPARAM lParam)
{
	char _Temp[32];
	if (IsAppWindow(hWnd) && (GetWindowLong(hWnd,GWL_STYLE) & WS_VISIBLE) == 0)
	{
		GetWindowText(hWnd,_Temp,31);
				HiddenTask[HiddenCount++] = 
							GetWindowLong(hWnd,GWL_WNDPROC);
	}
	return(HiddenCount < MAX_HIDDEN);
}

/************************************************************************/
/*				Is window of the type to be displayed ?					*/
/************************************************************************/

BOOL IsAppWindow(HWND hWnd)
{
	LONG lStyle,lExStyle;
	char _Temp[16];
	BOOL bAllow = TRUE;

	lStyle = GetWindowLong(hWnd,GWL_STYLE);	/* Learn about windows */
	lExStyle = GetWindowLong(hWnd,GWL_EXSTYLE);

	GetWindowText(hWnd,_Temp,				/* Get enough of the name to know its empty */
						sizeof(_Temp)-1);
	if (_Temp[0] == '\0') bAllow = FALSE;	/* Needs name, min box, visible, not tool */
	if (!(lStyle & WS_MINIMIZEBOX)) bAllow = FALSE;
	if (lExStyle & WS_EX_TOOLWINDOW) bAllow = FALSE;
	
	return(bAllow);
}