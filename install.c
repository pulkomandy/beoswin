/************************************************************************
 *																		*
 *							BEOS Deskbar Windows 9x						*
 *																		*
 *							Automatic Installer							*
 *																		*
 *						Written by Paul Robson (C) 2001					*
 *																		*
 ************************************************************************/

#include "beoswin.h"

/************************************************************************/
/*						   It installs itself							*/
/************************************************************************/

void Install(HANDLE hInstance,LPSTR lpCmdLine)
{
	char Current[_MAX_PATH],Windows[_MAX_PATH];
	FILE *fIn,*fOut;
	int n;

	#ifdef _DEBUG
	return;
	#endif
	
	GetModuleFileName(hInstance,			/* File name ran, check if WinDir */
						Current,sizeof(Current));
	GetWindowsDirectory(Windows,			/* Windows directory */
						sizeof(Windows));
	strcat(Windows,"\\BEOSWIN.EXE");		/* Full name of what we are looking for */
	if (stricmp(Current,Windows) == 0)		/* Already installed in Windows Directory */
									return;

	if (MessageBox(GetActiveWindow(),		/* Confirm installation */
				   "To install BEOS Deskbar for Windows I will first copy the executable file to your Windows directory. If you do not wish to do this, please press 'CANCEL'",
				   "Install BEOS Desktop",
				   MB_OKCANCEL | MB_ICONQUESTION) == IDCANCEL) return;

	fIn = fopen(Current,"rb");				/* Copy the executable */
	ABORT(fIn == NULL);
	fOut = fopen(Windows,"wb");
	ABORT(fOut == NULL);
	while (n = fgetc(fIn),n != EOF) fputc(n,fOut);
	fclose(fIn);
	fclose(fOut);

	GetWindowsDirectory(Windows,			/* Create the BEOS Menu and Demos directories */
						sizeof(Windows));
	strcat(Windows,"\\BEOS Menu");
	_mkdir(Windows);
	strcat(Windows,"\\Demos");
	_mkdir(Windows);

	SetShell(hInstance);					/* Run the "set the shell" dialogue */
}

/************************************************************************/
/*						  Set the shell function						*/
/************************************************************************/

void SetShell(HANDLE hInstance)
{
	FARPROC lpProc;
	lpProc = MakeProcInstance((FARPROC)SetShellDlg, hInstance);
	DialogBox(hInstance,MAKEINTRESOURCE(IDD_SHELL),GetActiveWindow(),(DLGPROC)lpProc);
	FreeProcInstance(lpProc);
	return;
}

/************************************************************************/
/*						About Box Dialog Function						*/
/************************************************************************/

BOOL FAR PASCAL CALLBACK SetShellDlg(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	BOOL bRet = TRUE;
	RECT rc;
	char szShellApp[_MAX_PATH],szText[_MAX_PATH];
	
	switch(iMessage)
	{
	case WM_INITDIALOG:
		GetWindowRect(hDlg,&rc);			/* Centre the dialog */
		SetWindowPos(hDlg,NULL,
					 (GetSystemMetrics(SM_CXSCREEN)-(rc.right-rc.left))/2,
					 (GetSystemMetrics(SM_CYSCREEN)-(rc.bottom-rc.top))/2,
					 0,0,SWP_NOSIZE | SWP_NOZORDER);
		GetPrivateProfileString("boot",		/* Read current shell */
								"shell",
								"explorer.exe",
								szShellApp,sizeof(szShellApp),"system.ini");
		sprintf(szText,						/* Set the Dialog item text */
				"Shell currently is set to \"%s\"",szShellApp);
		SetDlgItemText(hDlg,IDC_CURRENT,szText);
		CheckRadioButton(hDlg,				/* Check if it's explorer and set radio button */
						 IDC_SETBEOS,IDC_SETEXP,
						 stricmp(szShellApp,"explorer.exe") == 0 ? IDC_SETEXP : IDC_SETBEOS);
		break;
	case WM_COMMAND:
		if (wParam == IDOK)					/* On OK */
		{									/* Set Shell to explorer.exe or beoswin.exe */
			strcpy(szShellApp,"explorer.exe");
			if (IsDlgButtonChecked(hDlg,IDC_SETBEOS))
								strcpy(szShellApp,"beoswin.exe");
			WritePrivateProfileString("boot","shell",szShellApp,"system.ini");
			EndDialog(hDlg,TRUE);
		}
		if (wParam == IDCANCEL)
			EndDialog(hDlg,FALSE);
		break;
	default:
		bRet = FALSE;
	}
	return(bRet);
}
