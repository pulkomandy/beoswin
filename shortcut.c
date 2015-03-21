/************************************************************************
 *																		*
 *							BEOS Deskbar Windows 9x						*
 *																		*
 *							Main Window Procedure						*
 *																		*
 *						Written by Paul Robson (C) 2001					*
 *																		*
 ************************************************************************/

#include "beoswin.h"

static BOOL bShortcutActive;					/* TRUE if shortcuts working */

#define  WMEM_SELECT	(0)

/************************************************************************/
/*					Initialise desktop shortcuts						*/
/************************************************************************/

void InitShortcut(HANDLE hInstance)
{
	WNDCLASS wc;
	int i,n;
	char _Temp[_MAX_PATH],_Key[16];

	bShortcutActive = FALSE;

	#ifndef _DEBUG
	if (hWndTaskBar != NULL) return;		/* No shortcuts if explorer running */
	#endif

	wc.style = CS_DBLCLKS;					/* Register Window Class */
	wc.lpfnWndProc = ShortcutWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(LONG);			/* Allocate some memory for the handle */
	wc.hInstance = hInstance;
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_APPICON));
	wc.lpszMenuName = NULL;
	wc.lpszClassName = SHORTCUTCLASS;
	ABORT(RegisterClass(&wc) == 0);

	DragAcceptFiles(hWndDeskbar,TRUE);		/* Deskbar window accepts files */

	bShortcutActive = TRUE;					/* Shortcut now active */
	n = GetIntParameter("ShortcutCount",0);	/* Read them in */
	for (i = 0;i < n;i++)
	{
		sprintf(_Key,"Shortcut%02d",i);
		strcpy(_Temp,GetStringParameter(_Key,""));
		if (*_Temp != '\0')					/* Then create them */
		{
			_Temp[5] = '\0';
			_Temp[11] = '\0';
			CreateShortCut(hInstance,_Temp+12,atoi(_Temp),atoi(_Temp+6));
		}

	}
											/* If no shortcut pinch explorers to start with */
	if (FindWindow(SHORTCUTCLASS,NULL) == NULL)
									CopyShortcuts(hInstance);
}


/************************************************************************/
/*					  Terminate desktop shortcuts						*/
/************************************************************************/

static int _CloseCount;

void EndShortcut(void)
{
	#ifndef _DEBUG
	if (hWndTaskBar != NULL) return;		/* No save/close if explorer running */
	#endif
	if (!bShortcutActive) return;			/* Shortcuts not working */
	_CloseCount = 0;
	SendMessage(HWND_BROADCAST,msgControl,SM_CLOSESHORTCUTS,0L);
	WriteIntParameter("ShortcutCount",_CloseCount);
}

/************************************************************************/
/*				Copy shortcuts from Explorer to Desktop					*/
/************************************************************************/

#define XEDGE	(32)

void CopyShortcuts(HANDLE hInstance)
{
	char szPath[_MAX_PATH],szCut[_MAX_PATH];
	int  x,y;
	long lHandle;
	struct _finddata_t FileInfo;


	x = y = XEDGE;
	GetWindowsDirectory(szPath,sizeof(szPath));
	strcat(szPath,"\\Desktop\\*.*");

	lHandle = _findfirst(szPath,&FileInfo);	/* Read the handle */
	while (lHandle != -1)					/* While more files... */
	{										/* Add entry if not junk */
		if (strcmp(FileInfo.name,".") != 0 &&
			strcmp(FileInfo.name,"..") != 0)
		{			
			GetWindowsDirectory(szCut,sizeof(szCut));
			strcat(szCut,"\\Desktop\\");
			strcat(szCut,FileInfo.name);
			CreateShortCut(hInstance,szCut,x,y);
			x = x + SC_WIDTH*7/6;
			if (x > GetSystemMetrics(SM_CXSCREEN)*2/3)
			{
				x = XEDGE;y = y + SC_HEIGHT * 7/6;
			}
		}
	if (_findnext(lHandle,					/* Look for next matching file */
						&FileInfo) < 0) lHandle = -1;
	}
}

/************************************************************************/
/*						Create a new shortcut							*/
/************************************************************************/

HWND CreateShortCut(HANDLE hInstance,LPSTR lpFile,int x,int y)
{
	HWND hWnd;
	hWnd = CreateWindowEx(0,
						  SHORTCUTCLASS,lpFile,
						  WS_POPUP | WS_VISIBLE,
						  x,y,
						  SC_WIDTH,SC_HEIGHT,
						  NULL,NULL,hInstance,NULL);
	ShowWindow(hWnd,SW_SHOW);
	return(hWnd);
}

/************************************************************************/
/*					  Shortcut Window Procedure							*/
/************************************************************************/

LRESULT CALLBACK ShortcutWindowProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
	LONG lRet = 0L;
	PAINTSTRUCT ps;
	char _File[_MAX_PATH],_Name[_MAX_FNAME],_Junk[_MAX_PATH];
	RECT rc;
	HFONT hOldFont;
	POINT pt;
	static DWORD lFirstMove = 0L;
	int n;

	if (iMessage == msgControl &&			/* App terminating now.... */
		wParam == SM_CLOSESHORTCUTS)
	{
		sprintf(_Name,"Shortcut%02d",_CloseCount++);
		GetWindowText(hWnd,_File,sizeof(_File));
		GetWindowRect(hWnd,&rc);
		sprintf(_Junk,"%05d:%05d:%s",rc.left,rc.top,_File);
		WriteStringParameter(_Name,_Junk);
		PostMessage(hWnd,WM_CLOSE,0,0L);
	}

	if (iMessage == msgControl &&			/* Deselect an application */
		wParam == SM_DESELECT)
	{
		SetWindowLong(hWnd,WMEM_SELECT,0);
		InvalidateRect(hWnd,NULL,TRUE);
	}

	switch(iMessage)
	{
	case WM_CREATE:
		SendMessage(HWND_BROADCAST,msgControl,SM_DESELECT,0L);		
		break;

	case WM_LBUTTONDOWN:
		n = 1 - GetWindowLong(hWnd,WMEM_SELECT);
		SendMessage(HWND_BROADCAST,msgControl,SM_DESELECT,0L);
		SetWindowLong(hWnd,WMEM_SELECT,n);
		InvalidateRect(hWnd,NULL,TRUE);
		lFirstMove = GetTickCount()+200;
		break;

	case WM_LBUTTONUP:
		lFirstMove = 0L;
		break;

	case WM_MOUSEMOVE:
		if (wParam & MK_LBUTTON && GetTickCount() > lFirstMove)
		{
			GetCursorPos(&pt);				/* Move to its new home */
			pt.x-=SC_WIDTH/2;
			pt.y-=SC_HEIGHT/2;
			MoveWindow(hWnd,pt.x,pt.y,SC_WIDTH,SC_HEIGHT,TRUE);
			GetCursorPos(&pt);				/* Close if mouse in deskbar rectangle */
			GetWindowRect(hWndDeskbar,&rc);
			if (PtInRect(&rc,pt)) PostMessage(hWnd,WM_CLOSE,0,0L);
		}
		break;

	case WM_LBUTTONDBLCLK:					/* Execute application or whatever */
		GetWindowText(hWnd,_File,sizeof(_File));
		ExecuteApplication(hWnd,_File,"");
		break;

	case WM_PAINT:							/* Paint the icon */
		BeginPaint(hWnd,&ps);
		GetClientRect(hWnd,&rc);			/* Text starts here */
		rc.top = 36;
		GetWindowText(hWnd,_File,			/* Get the actual filename */
							sizeof(_File));
		DrawFilenameIcon(ps.hdc,			/* Draw the associated icon */
						 (HANDLE)GetWindowLong(hWnd,GWL_HINSTANCE),
						 _File,
						 SC_WIDTH/2-16,0,FALSE);
											/* Get the name */
		_splitpath(_File,_Junk,_Junk,_Name,_Junk);	
											/* Pepare to write */
		hOldFont = SelectObject(ps.hdc,hFont);
		n = GetWindowLong(hWnd,WMEM_SELECT);
											/* Set for selected/unselected */
		SetBkMode(ps.hdc,n == 0 ? TRANSPARENT:OPAQUE);
		SetBkColor(ps.hdc,RGB(0,0,0));
		SetTextColor(ps.hdc,RGB(255,255,255));
		DrawText(ps.hdc,					/* Write the text */
				 _Name,strlen(_Name),
				 &rc,DT_CENTER | DT_WORDBREAK);
		SelectObject(ps.hdc,hOldFont);
		EndPaint(hWnd,&ps);
		break;

	default:
		lRet = DefWindowProc(hWnd,iMessage,wParam,lParam);
		break;
	}
	return(lRet);
}
		

