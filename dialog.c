/************************************************************************
 *																		*
 *							BEOS Deskbar Windows 9x						*
 *																		*
 *							  Dialog Box Routines						*
 *																		*
 *						Written by Paul Robson (C) 2001					*
 *																		*
 ************************************************************************/

#include "beoswin.h"

/************************************************************************/
/*						About Box Dialog Function						*/
/************************************************************************/

BOOL FAR PASCAL CALLBACK About(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	BOOL bRet = TRUE;
	RECT rc;
	char szText[64];
	switch(iMessage)
	{
	case WM_INITDIALOG:
		sprintf(szText,						/* Set version and date stamp */
				"BEOS Deskbar for Windows\nVersion %.2f (%s)",VERSION,DATE);
		SetDlgItemText(hDlg,IDC_INFO,szText);
		GetWindowRect(hDlg,&rc);			/* Centre in display area */
		SetWindowPos(hDlg,NULL,
					 (GetSystemMetrics(SM_CXSCREEN)-(rc.right-rc.left))/2,
					 (GetSystemMetrics(SM_CYSCREEN)-(rc.bottom-rc.top))/2,
					 0,0,SWP_NOSIZE | SWP_NOZORDER);					 
		break;
	case WM_COMMAND:
		if (wParam == IDOK)
			EndDialog(hDlg,TRUE);
		break;
	default:
		bRet = FALSE;
	}
	return(bRet);
}

/************************************************************************/
/*						Test for legit shutdown							*/
/************************************************************************/

BOOL ShutDownTest(HWND hWnd,BOOL IsShutdown)
{
	HANDLE hInst;
	FARPROC lpProc;
	BOOL bRet;
	hInst = (HANDLE)GetWindowLong(hWnd,GWL_HINSTANCE);
	lpProc = MakeProcInstance((FARPROC)WarningDlgProc, hInst);
	bRet = DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_WARNING),hWnd,(DLGPROC)lpProc,IsShutdown);
	FreeProcInstance(lpProc);
	return(bRet);
}

/************************************************************************/
/*				Warning dialog for shutdown/restart						*/
/************************************************************************/

int nTime,nTargetTime;

BOOL FAR PASCAL CALLBACK WarningDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	BOOL bRet = TRUE;
	RECT rc;
	POINT pt;
	HWND hWarning;
	char _Msg[64];

	switch(iMessage)						
	{
	case WM_INITDIALOG:
		nTime = 0;							/* Initialise the clock */
		nTargetTime = PARAMINT("ShutdownDelay",5) * 100;
		SendMessage(hDlg,WMU_SETBAR,0,0L);	/* Reset the bar */
		sprintf(_Msg,"%s after %d seconds.",/* Set the text message */
				lParam ? "Shutting down" : "Restarting",
				nTargetTime / 100);
		SetDlgItemText(hDlg,IDC_MESSAGE,_Msg);
		SetTimer(hDlg,1,WARNTICK,NULL);		/* Start the timer */
		break;

	case WM_TIMER:							/* On timer tick */
		nTime = nTime + WARNTICK/10;		/* Advance the timer */
		SendMessage(hDlg,WMU_SETBAR,		/* Redraw the bar */
					nTime * 100 / nTargetTime,0L);
		if (nTime > nTargetTime)			/* On time out, end with "TRUE" */
				EndDialog(hDlg,TRUE);
		break;

	case WMU_SETBAR:						/* WM_USER : Draw the flag */
		hWarning = GetDlgItem(hDlg,			/* Identify dialog item surrounding it*/
								IDC_PROGRESS);
		GetWindowRect(hWarning,&rc);		/* Get rectangle */
		pt.x = rc.left;pt.y = rc.top;		/* Work out in Client terms */
		ScreenToClient(hDlg,&pt);			/* Reposition it */
		SetWindowPos(GetDlgItem(hDlg,IDC_BAR),	
					 NULL,
					 pt.x,pt.y,
					 (rc.right-rc.left)*wParam/100,rc.bottom-rc.top,
					 SWP_NOZORDER);
		break;

	case WM_DESTROY:						/* On exit kill the timer */
		KillTimer(hDlg,1);
		break;

	case WM_COMMAND:
		if (wParam == IDOK)
			EndDialog(hDlg,FALSE);
		break;
	default:
		bRet = FALSE;
	}
	return(bRet);
}

