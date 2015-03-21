#include "windows.h"

#define WCLASS	"BEOSWindowClass"

LRESULT CALLBACK MenuWindowProc(HWND hWnd,UINT iMessage,
																WPARAM wParam,LPARAM lParam);
LONG ChangeMessagesToBEOS(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam);
void RepaintCaption(HWND,UINT,WPARAM,LPARAM);


int PASCAL WinMain(HANDLE hInst,HANDLE hPrevInst,LPSTR lpCmdLine,int nCmdShow)
{
MSG Msg;
HWND hWndMain;
WNDCLASS wc;

if (hPrevInst == NULL) 								/* Only one instance allowed */
	{
    wc.style = 	CS_DBLCLKS;							/* Fill up class register structure */
	wc.lpfnWndProc = MenuWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = GetStockObject(GRAY_BRUSH);
	wc.lpszMenuName = "MENU";
	wc.lpszClassName = WCLASS;
	RegisterClass(&wc);								/* Register class */
	}

hWndMain = CreateWindow(WCLASS,
								"[Title Bar]",
								WS_OVERLAPPEDWINDOW | WS_VISIBLE,
								120,120,256,256,
								NULL,NULL,hInst,NULL);
ShowWindow(hWndMain,nCmdShow);

while (GetMessage(&Msg, NULL, 0, 0))			/* Standard Message Pump */
		{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
		}
return 0;

}


LRESULT CALLBACK MenuWindowProc(HWND hWnd,UINT iMessage,
													WPARAM wParam,LPARAM lParam)
{
LONG lRet = 0L;

lRet = ChangeMessagesToBEOS(hWnd,iMessage,wParam,lParam);

if (lRet != 0) return(lRet);

switch(iMessage)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		lRet = DefWindowProc(hWnd,iMessage,wParam,lParam);
		break;
	}
return(lRet);
}

LONG ChangeMessagesToBEOS(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
	LONG lStyle;
	LONG lRet = TRUE;
	LPWINDOWPOS lpWin;
	switch(iMessage)
	{
		case WM_SETTEXT:
			lStyle = GetWindowLong(hWnd,GWL_STYLE);
			SetWindowLong(hWnd,GWL_STYLE,lStyle & (~WS_VISIBLE));
			DefWindowProc(hWnd,iMessage,wParam,lParam);
			SetWindowLong(hWnd,GWL_STYLE,lStyle);
			break;

		case WM_NCHITTEST:
			lRet = DefWindowProc(hWnd,iMessage,wParam,lParam);
			if (lRet == HTZOOM || lRet == HTREDUCE) lRet = HTCAPTION;
			if (lRet == HTSYSMENU) lRet = HTCAPTION;
			break;
	
		case WM_NCACTIVATE:
			break;

		case WM_MOVE:
		case WM_SIZE:
			InvalidateRect(NULL,NULL,TRUE);
			lRet = 0L;
			break;

		case WM_NCPAINT:
			lStyle = GetWindowLong(hWnd,GWL_STYLE);
			if ((lStyle & WS_CAPTION) && (lStyle & WS_VISIBLE))
				RepaintCaption(hWnd,iMessage,wParam,lParam);
			else
				DefWindowProc(hWnd,iMessage,wParam,lParam);
			break;

		case WM_WINDOWPOSCHANGING:
			lpWin = (LPWINDOWPOS)lParam;
			lpWin->flags |= SWP_NOCOPYBITS;
			break;

		default:
			lRet = 0L;
			break;
	}
return(lRet);
}


void RepaintCaption(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
	LONG lStyle;
	RECT rc;
	HRGN hRegion;
	HDC hDC;
	int  y,CapWidth;
	HPEN hOldPen;
	HBRUSH hOldBrush;

	lStyle = GetWindowLong(hWnd,GWL_STYLE);
	GetWindowRect(hWnd,&rc);
	y = 0;
	if (lStyle && WS_BORDER) y = 1;
	if (lStyle && WS_THICKFRAME) y = GetSystemMetrics(SM_CYFRAME);
	if (lStyle && WS_DLGFRAME) y = GetSystemMetrics(SM_CYDLGFRAME);
	y = y + GetSystemMetrics(SM_CYCAPTION);
	hRegion = CreateRectRgn(rc.left,rc.top+y,rc.right,rc.bottom);
	DefWindowProc(hWnd,iMessage,(WPARAM)hRegion,lParam);
	DeleteObject(hRegion);
	CapWidth = (rc.right-rc.left)/2;
	hDC = GetDC(NULL);
	hOldPen = SelectObject(hDC,GetStockObject(NULL_PEN));
	hOldBrush = SelectObject(hDC,GetSysColorBrush(COLOR_ACTIVECAPTION));
	Rectangle(hDC,rc.left,rc.top,rc.left+CapWidth,rc.top+y+1);
	SelectObject(hDC,hOldPen);
	SelectObject(hDC,hOldBrush);
	ReleaseDC(NULL,hDC);
}