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

/************************************************************************/
/*					Window Procedure for the Deskbar					*/
/************************************************************************/

static int _ScanCount;

LRESULT CALLBACK DeskbarWindowProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
	LRESULT lRes = 0;
	PAINTSTRUCT ps;
	LPDESKBARMENU lpMem,lpNew;
	RECT rc,rcUpdate;
	POINT pt;
	HANDLE hInst;
	int i,x,y,n,nVisible;
	BOOL IsScrolling,PopDown;
	char _DirBuffer[_MAX_PATH],*s;
	HWND hWndNew,hUnderWnd,hWndMenu;
	FARPROC lpProc;
	
	if (iMessage == msgControl)				/* Internal messages */
	{
		lpMem = LockDeskbarMemory(hWnd);	/* Close submenus above certain depth */
		if (wParam == SM_CLOSEALL)
			if (lpMem->Level > lParam) 
				PostMessage(hWnd,WM_CLOSE,0,0L);
		if (wParam == SM_CLOSESUBMENU && 
			lpMem->Type == MT_SUBMENU)		/* Close all popups */
					PostMessage(hWnd,WM_CLOSE,0,0L);
		if (wParam == SM_KILLMENU)			/* Close all menus */
		{
			if (lpMem->Type == MT_DESKBAR)
			{
				lpMem->OpenItem = -1;
				InvalidateRect(hWnd,NULL,FALSE);
			}
			else
				PostMessage(hWnd,WM_CLOSE,0,0L);
		}
		if (wParam == SM_DESELECTMAIN &&	/* Deselect main menu */
					lpMem->Type == MT_MENU)
		{
			if (lpMem->Selected >= 0)		/* If selected, then deselect and redraw */
			{
				lpMem->Menu[lpMem->Selected].Selected = FALSE;
				lpMem->Selected = -1;
				InvalidateRect(hWnd,NULL,FALSE);
			}
		}
		lpMem = UnlockDeskbarMemory(hWnd);
		return(0L);
	}

	switch(iMessage)
	{
	case WM_CREATE:
		ABORT(SetTimer(hWnd,1,				/* Timer for scrolling and popdown */
				PARAMINT("ScrollRate",100),NULL) == 0);			
		WinCount++;							/* One more Deskbar window */
		break;

	#ifndef _DEBUG
	case WM_SYSCOMMAND:						/* In release mode, can't close via ALT+F4 */
		break;
	#endif

	case WM_LBUTTONDOWN:					/* Click */
		lpMem = LockDeskbarMemory(hWnd);		
		n = lpMem->OverItem;				/* Which one are we on ? */
		if (lpMem->Type == MT_DESKBAR &&	/* Deskbar menu behaviour ? */
					lpMem->Menu[n].IsSelectable)
		{
			PopDown = (lpMem->OpenItem>=0);	/* Closing only by clicking on open ? */
			SendMessage(HWND_BROADCAST,		/* Close the menu down */
								msgControl,SM_KILLMENU,0L);
			if (PopDown) n = -1;			/* Closing only, don't open ! */
			lpMem->OpenItem = n;			/* Currently open item */
			InvalidateRect(hWnd,NULL,FALSE);
		}

		if (n >= 0)							/* Menu item selected */
		{
			switch(lpMem->Menu[n].Type)		/* What's to do */
			{
			case ET_RESTART:				/* Restart/Shutdown call the function */
				if (ShutDownTest(hWnd,FALSE))
					ExitWindowsEx(EWX_REBOOT|EWX_LOGOFF,0);
				break;
			case ET_SHUTDOWN:
				if (ShutDownTest(hWnd,TRUE))
					ExitWindowsEx(EWX_SHUTDOWN|EWX_POWEROFF|EWX_LOGOFF,0);
				break;

			case ET_CONFIGURE:				/* Configure menu */
				strcpy(_DirBuffer,"explorer /e,");
				GetWindowsDirectory(_DirBuffer+strlen(_DirBuffer),sizeof(_DirBuffer));
				strcat(_DirBuffer,"\\Start Menu\\Programs");
				WinExec(_DirBuffer,SW_SHOWNORMAL);
				break;

			case ET_TASK:					/* Task selected */
				GetMenuPosition(&pt,lpMem,hWnd,n);
				hWndNew = CreateDeskbarMenu(lpMem->hInstance,
										MT_MENU,
										pt.x,pt.y,"",
										0,
										lpMem->hWndMain);
				CHECK(hWndNew != NULL);
											/* Build and display it */
				AddDeskbarItem(hWndNew,ET_ACTIVATE);
				AddDeskbarItem(hWndNew,ET_SHOW);
				AddDeskbarItem(hWndNew,ET_HIDE);
				AddDeskbarItem(hWndNew,ET_CLOSE);
											/* Now set it up */
				lpNew = LockDeskbarMemory(hWndNew);
											/* Copy the task details in */
				for (i = 0;i < lpNew->TotalItems;i++)
					lpNew->Menu[i].lpTask = lpMem->Menu[n].lpTask;
				s = lpMem->Menu[n].Text;	/* Set up the menu text */
				strcpy(lpNew->Menu[0].Text,s);
											/* All nicely set up */
				lpNew = UnlockDeskbarMemory(hWndNew);
				RecalculateMenuSize(hWndNew);
				ShowWindow(hWndNew,SW_SHOWNORMAL);
				break;

			case ET_TITLE:					/* Title will open or close the menu */
				GetMenuPosition(&pt,lpMem,hWnd,n);
				hWndNew = CreateDeskbarMenu(lpMem->hInstance,
										MT_MENU,
										pt.x,pt.y,"",
										0,
										lpMem->hWndMain);
				CHECK(hWndNew != NULL);
											/* Build and display it */
				AddDeskbarItem(hWndNew,ET_ABOUT);
				AddDeskbarItem(hWndNew,ET_CONFIGURE);
				AddDeskbarItem(hWndNew,ET_SETSHELL);
				AddDeskbarItem(hWndNew,ET_RESTART);
				AddDeskbarItem(hWndNew,ET_SHUTDOWN);
				AddDeskbarItem(hWndNew,ET_PROGRAMS);
				GetWindowsDirectory(_DirBuffer,sizeof(_DirBuffer));
				strcat(_DirBuffer,"\\BEOS Menu");
				AppendDeskbarDirectory(hWndNew,_DirBuffer);
				AddDeskbarItem(hWndNew,ET_PREFS);
				ShowWindow(hWndNew,SW_SHOWNORMAL);
				break;

			case ET_SETSHELL:
				SetShell((HANDLE)GetWindowLong(hWnd,GWL_HINSTANCE));
				break;

			case ET_ABOUT:					/* Display a little box */
				hInst = (HANDLE)GetWindowLong(hWnd,GWL_HINSTANCE);
				lpProc = MakeProcInstance((FARPROC)About, hInst);
				DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUT),hWnd,(DLGPROC)lpProc);
				FreeProcInstance(lpProc);
				break;

			case ET_PREFITEM:				/* Run a preferences item */
				strcpy(_DirBuffer,"rundll32.exe shell32.dll,Control_RunDLL ");
				strcat(_DirBuffer,lpMem->Menu[n].PanelApp);
				strcat(_DirBuffer,",");
				strcat(_DirBuffer,lpMem->Menu[n].Text);
				WinExec(_DirBuffer,SW_SHOWNORMAL);
				break;

			case ET_MENUITEM:				/* Clicked on menu items */
				strcpy(_DirBuffer,lpMem->Directory);
				strcat(_DirBuffer,lpMem->Menu[n].Text);
				ExecuteApplication(hWnd,_DirBuffer,lpMem->Directory);
				break;

			case ET_ACTIVATE:				/* Task control */
			case ET_SHOW:
			case ET_HIDE:
			case ET_CLOSE:
				hWndMenu = lpMem->Menu[n].lpTask->hWnd;
				if (lpMem->Menu[n].Type == ET_ACTIVATE) 
				{
					if (IsIconic(hWndMenu)) ShowWindow(hWndMenu,SW_RESTORE);
					ShowWindow(hWndMenu,SW_SHOW);
					BringWindowToTop(hWndMenu);
				}
				if (lpMem->Menu[n].Type == ET_SHOW)		ShowWindow(hWndMenu,SW_SHOW);
				if (lpMem->Menu[n].Type == ET_HIDE)		ShowWindow(hWndMenu,SW_HIDE);
				if (lpMem->Menu[n].Type == ET_CLOSE)	PostMessage(hWndMenu,WM_CLOSE,0,0L);
				PostMessage(hWnd,WM_CLOSE,0,0L);
				PostMessage(HWND_BROADCAST,msgControl,SM_KILLMENU,0L);
				break;
			}
		}
		lpMem = UnlockDeskbarMemory(hWnd);
		break;

	case WM_MOUSEMOVE:						/* Check for selection change */
		lpMem = LockDeskbarMemory(hWnd);
		x = LOWORD(lParam);					/* Position of mouse */
		y = HIWORD(lParam);
		n = (lpMem->TopArrow)?yArrowSize:0;	/* Amount of arrow space at top */
		if (y >= n &&						/* In the selection area */
			y < n+lpMem->ySize*lpMem->VisCount)
		{
			 n = (y - n) / lpMem->ySize +	/* Identify the item to be selected */
								lpMem->TopDisplayItem;
			 lpMem->OverItem = n;			/* Item we are over */

			 	
			 if (lpMem->Selected != n &&	/* if it has changed.... */
				 lpMem->Type != MT_DESKBAR)				 
			 {								
				SendMessage(HWND_BROADCAST,	/* Close all popups down the line */
									msgControl,SM_CLOSEALL,lpMem->Level);
				 							/* Deselect the item & redraw it */
				if (lpMem->Selected >= 0)
								lpMem->Menu[lpMem->Selected].Selected = FALSE;
				InvalidateRect(hWnd,
							   GetEntryRect(lpMem->Selected-lpMem->TopDisplayItem,&rc,lpMem),
							   FALSE);
											/* If item can be selected this way */
				if (lpMem->Menu[n].IsSelectable)
				{							/* Select it */
					lpMem->Menu[n].Selected = TRUE;
					lpMem->Selected = n;	/* Set current selection and redraw */
					InvalidateRect(hWnd,
								   GetEntryRect(lpMem->Selected-lpMem->TopDisplayItem,&rc,lpMem),
								   FALSE);
											/* If it is a submenu, pop it up */
					if (HASARROW(lpMem->Menu[n].Type))
					{
						GetMenuPosition(&pt,lpMem,hWnd,n);
											/* Build directory name */
						strcpy(_DirBuffer,lpMem->Directory);
											/* Submenu appends the submenu directory */
						if (lpMem->Menu[n].Type == ET_SUBMENU)
								strcat(_DirBuffer,lpMem->Menu[n].Text);
						if (lpMem->Menu[n].Type == ET_PROGRAMS)
						{
							GetWindowsDirectory(_DirBuffer,sizeof(_DirBuffer));
							strcat(_DirBuffer,"\\Start Menu\\Programs");
						}
											/* Prefs uses the [PANEL] fix */
						if (lpMem->Menu[n].Type == ET_PREFS)
								strcpy(_DirBuffer,"[PANEL]");

											/* Create the new submenu Deskbar */
						hWndNew = CreateDeskbarMenu(lpMem->hInstance,
													MT_SUBMENU,
													pt.x,pt.y,_DirBuffer,
													lpMem->Level+1,
													lpMem->hWndMain);
						CHECK(hWndNew != NULL);
											/* Add all the file entries and show it */
						AppendDeskbarDirectory(hWndNew,_DirBuffer);
						ShowWindow(hWndNew,SW_SHOWNORMAL);
					}
				}
			 }
		}	
		lpMem = UnlockDeskbarMemory(hWnd);
		break;

	case WM_TIMER:
		IsScrolling = FALSE;				/* Check if scrolling flag */
		lpMem = LockDeskbarMemory(hWnd);
		ABORT(!GetCursorPos(&pt));			/* Where is the cursor ? */
		ABORT(!ScreenToClient(hWnd,&pt));
		x = pt.x;y = pt.y;

		if (lpMem->BottomArrow)				/* Scrolling down maybe ? */
		{
			n = ((lpMem->TopArrow) ? yArrowSize : 0) + lpMem->ySize * lpMem->VisCount;
			if (y > n)						/* if cursor below line calculated above */		
			{
				lpMem->TopArrow = TRUE;		/* Must now be a top arrow */
				lpMem->TopDisplayItem++;	/* Scroll down one */
											/* May not be a bottom arrow */
				if (lpMem->TopDisplayItem+lpMem->VisCount == lpMem->TotalItems)
					lpMem->BottomArrow = FALSE;
				RecalculateMenuSize(hWnd);	/* Recalculate menu size and redraw */
				IsScrolling = TRUE;
			}
		}

		if (lpMem->TopArrow && y<yArrowSize)/* Scrolling up ? */
		{	
											/* One line up, remove top arrow if at top */
			if (--lpMem->TopDisplayItem == 0) lpMem->TopArrow = FALSE;
			lpMem->BottomArrow = TRUE;		/* Must be a bottom arrow */
			RecalculateMenuSize(hWnd);		/* Calculate menu sizes again */
			IsScrolling = TRUE;
		}

		_ScanCount++;
		if (lpMem->Type == MT_DESKBAR &&	/* Check task list when windows closed */
			lpMem->OpenItem < 0 &&
			_ScanCount % 5 == 0)
		{
			CollectTaskWindows(lpMem->hInstance,FALSE);
			if (Tasks.Changed)
					UpdateDeskbarMenuTasks(hWnd);	
		}

		lpMem = UnlockDeskbarMemory(hWnd);

		GetCursorPos(&pt);					/* Find out what's under the menu */
		hUnderWnd = WindowFromPoint(pt);

		PopDown = FALSE;
		if (hUnderWnd == NULL)				/* if desktop, close popups */
					PopDown = TRUE;
		else								/* if window, if not mine close popups */
			if (GetClassLong(hUnderWnd,GCW_ATOM) != GetClassLong(hWnd,GCW_ATOM))
																		PopDown = TRUE;


		if (PopDown)						/* close popups if required */
		{
			SendMessage(HWND_BROADCAST,msgControl,SM_CLOSESUBMENU,0L);
			SendMessage(HWND_BROADCAST,msgControl,SM_DESELECTMAIN,0L);
		}
		
		break;

	case WM_DISPLAYCHANGE:					/* Resolution changed */
		SendMessage(HWND_BROADCAST,msgControl,SM_CLOSESUBMENU,0L);
		SendMessage(HWND_BROADCAST,msgControl,SM_DESELECTMAIN,0L);
		lpMem = LockDeskbarMemory(hWnd);	/* Lock for redraw */
		hWndMenu = lpMem->hWndMain;			/* The top window */
		CHECK(hWndMenu != NULL);
		GetWindowRect(hWndMenu,&rc);		/* Get its size */
		lpMem = UnlockDeskbarMemory(hWnd);
		SetWindowPos(hWnd,NULL,				/* Reposition it appropriately */
					 LOWORD(lParam)-(rc.right-rc.left),0,
					 0,0,SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOZORDER);
		RecalculateMenuSize(hWndMenu);		/* Resize it */
		break;

	case WM_PAINT:
		lpMem = LockDeskbarMemory(hWnd);	/* Lock for redraw */
		nVisible = lpMem->VisCount;
		BeginPaint(hWnd,&ps);
		rc.left = rc.top = 0;				/* Calculate frame for menu */
		rc.right = lpMem->xSize-1;			/* WEHT GetClientRect ? */
		rc.bottom = nVisible*lpMem->ySize-1;
		if (lpMem->TopArrow) rc.bottom+= yArrowSize;
		if (lpMem->BottomArrow) rc.bottom+= yArrowSize;
		BevelFrame(ps.hdc,&rc,TRUE);

		rc.left = 1;rc.right=lpMem->xSize-1;/* Frame for top arrow */
		rc.top = 0;rc.bottom = yArrowSize;
		if (lpMem->TopArrow)				/* Draw if required */
		{
			DrawArrow(ps.hdc,&rc,TRUE);		
			rc.top = yArrowSize;			/* Offset for bottomarrow if toparrow */
		}
		rc.top += lpMem->ySize*nVisible;	/* Frame for bottom arrow */
		rc.bottom = rc.top + yArrowSize;
		if (lpMem->BottomArrow)				/* Draw if required */
					DrawArrow(ps.hdc,&rc,FALSE);		
			
		GetUpdateRect(hWnd,&rcUpdate,FALSE);

		for (i = 0;i < nVisible;i++)		/* Work through all entries redrawing them */
		{
			GetEntryRect(i,&rc,lpMem);
			DrawDeskbarItem(ps.hdc,&rc,lpMem,i+lpMem->TopDisplayItem,i == nVisible-1);
		}
		EndPaint(hWnd,&ps);
		lpMem = UnlockDeskbarMemory(hWnd);
		break;

	case WM_ACTIVATEAPP:					/* If Deskbar switch kill menu */
		lpMem = LockDeskbarMemory(hWnd);	/* Lock for redraw */
		if (!wParam && lpMem->Type == MT_DESKBAR)
			SendMessage(HWND_BROADCAST,msgControl,SM_KILLMENU,0L);
		lpMem = UnlockDeskbarMemory(hWnd);	
		break;

	case WM_DROPFILES:
		n = DragQueryFile((HDROP)wParam,0xFFFFFFFF,NULL,0);
		if (n == 1)
		{
			GetWindowRect(hWndDeskbar,&rc);
			DragQueryFile((HDROP)wParam,0,_DirBuffer,sizeof(_DirBuffer));
			CreateShortCut((HANDLE)GetWindowLong(hWnd,GWL_HINSTANCE),
						   _DirBuffer,
						   (rc.right+rc.left-SC_WIDTH)/2,
						   rc.bottom+16);
		}
		break;

	case WM_DESTROY:
		KillTimer(hWnd,1);					/* Kill timer on exit */
		if (--WinCount == 0)				/* Quit if last window closed */
				PostQuitMessage(0);
		GlobalFree((HGLOBAL)				/* Free up memory */
					GetWindowLong(hWnd,WC_HANDLE));
		AllocCount--;
		SetWindowLong(hWnd,WC_HANDLE,0);
		break;
	default:
		lRes = DefWindowProc(hWnd,iMessage,wParam,lParam);
		break;
	}
	return(lRes);
}

