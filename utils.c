/************************************************************************
 *																		*
 *							BEOS Deskbar Windows 9x						*
 *																		*
 *							   Utility Routines							*
 *																		*
 *						Written by Paul Robson (C) 2001					*
 *																		*
 ************************************************************************/

#include "beoswin.h"

/************************************************************************/
/*			       Lock the deskbar memory block						*/
/************************************************************************/

LPDESKBARMENU LockDeskbarMemory(HWND hWnd)
{
	LPDESKBARMENU lpMenu;
	ABORT(LockCount < 0);
	LockCount++;
	CHECK(GetWindowLong(hWnd,WC_HANDLE) != 0L);
	lpMenu = GlobalLock((HGLOBAL)GetWindowLong(hWnd,WC_HANDLE));
	ABORT(lpMenu == NULL);
	return(lpMenu);
}

/************************************************************************/
/*					Unlock the deskbar memory block						*/
/************************************************************************/

LPDESKBARMENU UnlockDeskbarMemory(HWND hWnd)
{
	LockCount--;
	ABORT(LockCount < 0);
	CHECK(GetWindowLong(hWnd,WC_HANDLE) != 0L);
	GlobalUnlock((HGLOBAL)GetWindowLong(hWnd,WC_HANDLE));
	return(NULL);
}

/************************************************************************/
/*						Draw a bevelled frame							*/
/************************************************************************/

void BevelFrame(HDC hDC,LPRECT rc,BOOL Raised)
{
	HPEN hOldPen;
	SelectObject(hDC,Raised ? GetStockObject(WHITE_PEN):hGreyPen);
	MoveToEx(hDC,rc->right,rc->top,NULL);
	LineTo(hDC,rc->left,rc->top); 
	LineTo(hDC,rc->left,rc->bottom);
	SelectObject(hDC,!Raised ? GetStockObject(WHITE_PEN):hGreyPen);
	LineTo(hDC,rc->right,rc->bottom);
	LineTo(hDC,rc->right,rc->top);
	SelectObject(hDC,hOldPen);
}

/************************************************************************/
/*					Draw Icon Associated with a file name				*/
/************************************************************************/

void DrawFilenameIcon(HDC hDC,HINSTANCE hInst,LPSTR lpFile,int x,int y,BOOL bSmall)
{
	SHFILEINFO FileInfo;
	HIMAGELIST hImgList;
											/* Get the baby icon */
	hImgList = (HIMAGELIST)SHGetFileInfo(lpFile,
		  								 _A_NORMAL | _A_SUBDIR,
										 &FileInfo,
									     sizeof(FileInfo),
										 SHGFI_ICON | (bSmall ? SHGFI_SMALLICON : SHGFI_LARGEICON) | SHGFI_SYSICONINDEX);
											/* and draw it */
	if (hImgList == NULL)
	{
		DrawIcon(hDC,x,y,LoadIcon(hInst,MAKEINTRESOURCE(bSmall ? IDI_DEF16:IDI_DEF32)));
		return;
	}
	ImageList_Draw(hImgList,FileInfo.iIcon,
								hDC,x,y,ILD_TRANSPARENT);

}

/************************************************************************/
/*						Profile access functions						*/
/************************************************************************/

char _Buffer[_MAX_PATH];

LPSTR GetStringParameter(LPSTR lpKey,LPSTR Default)
{
	GetProfileString(APPNAME,lpKey,Default,_Buffer,sizeof(_Buffer));
	#ifdef _DEBUG
	strcpy(_Buffer,Default);
	#endif
	WriteStringParameter(lpKey,_Buffer);
	return(_Buffer);
}

void WriteStringParameter(LPSTR lpKey,LPSTR lpValue)
{
	WriteProfileString(APPNAME,lpKey,lpValue);
}

int GetIntParameter(LPSTR lpKey,int DefValue)
{
	int n = GetProfileInt(APPNAME,lpKey,DefValue);
	#ifdef _DEBUG
	n = DefValue;
	#endif
	WriteIntParameter(lpKey,n);
	return(n);
}

void WriteIntParameter(LPSTR lpKey,int n)
{
	char _Buffer2[20];
	WriteProfileString(APPNAME,lpKey,itoa(n,_Buffer2,10));
}

/************************************************************************/
/*						Report an error									*/
/************************************************************************/

void ReportError(int nLine,LPSTR lpFile)
{
	char _Temp[64];
	sprintf(_Temp,"Error at %d of %s",nLine,lpFile);
	MessageBox(GetActiveWindow(),_Temp,"Alert !",MB_ICONHAND | MB_OK);
	strcpy(_Temp,"");
}


/************************************************************************/
/*				Convert control panel applet to full path				*/
/************************************************************************/

void GetAppletPath(LPSTR lpName,LPSTR lpPath)
{
	GetWindowsDirectory(lpPath,_MAX_PATH);
	strcat(lpPath,"\\system\\");
	strcat(lpPath,lpName);
}

/************************************************************************/
/*						Application Execution							*/
/************************************************************************/

void ExecuteApplication(HWND hWnd,LPSTR lpFile,LPSTR lpDir)
{
	HCURSOR hCur = SetCursor(LoadIcon(NULL,IDC_WAIT));
	ShellExecute(hWnd,NULL,lpFile,NULL,lpDir,SW_SHOWDEFAULT);
	SetCursor(hCur);
}

/************************************************************************/
/*							Startup Group								*/
/************************************************************************/

void StartupExecute(HANDLE hInstance)
{
	char szPath[_MAX_PATH],szCut[_MAX_PATH];
	long lHandle;
	struct _finddata_t FileInfo;


	GetWindowsDirectory(szPath,sizeof(szPath));
	strcat(szPath,"\\Start Menu\\Programs\\StartUp\\*.*");

	lHandle = _findfirst(szPath,&FileInfo);	/* Read the handle */
	while (lHandle != -1)					/* While more files... */
	{										/* Add entry if not junk */
		if (strcmp(FileInfo.name,".") != 0 &&
			strcmp(FileInfo.name,"..") != 0)
		{			
			GetWindowsDirectory(szCut,sizeof(szCut));
			strcat(szCut,"\\Start Menu\\Programs\\StartUp\\");
			strcat(szCut,FileInfo.name);
			ExecuteApplication(GetActiveWindow(),szCut,"");
		}
	if (_findnext(lHandle,					/* Look for next matching file */
						&FileInfo) < 0) lHandle = -1;
	}
}