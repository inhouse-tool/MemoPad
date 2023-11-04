// Dark.cpp : implementation file
//

#include "pch.h"
#include "Dark.h"

///////////////////////////////////////////////////////////////////////////////////////
// Definitions out of the Class

// Undocumented APIs in uxtheme.dll.
// See https://github.com/ysc3839/win32-darkmode for below.

enum	PreferredAppMode
{
	Default,
	AllowDark,
	ForceDark,
	ForceLight,
	Max
};

enum	WINDOWCOMPOSITIONATTRIB
{
	WCA_UNDEFINED = 0,
	WCA_NCRENDERING_ENABLED = 1,
	WCA_NCRENDERING_POLICY = 2,
	WCA_TRANSITIONS_FORCEDISABLED = 3,
	WCA_ALLOW_NCPAINT = 4,
	WCA_CAPTION_BUTTON_BOUNDS = 5,
	WCA_NONCLIENT_RTL_LAYOUT = 6,
	WCA_FORCE_ICONIC_REPRESENTATION = 7,
	WCA_EXTENDED_FRAME_BOUNDS = 8,
	WCA_HAS_ICONIC_BITMAP = 9,
	WCA_THEME_ATTRIBUTES = 10,
	WCA_NCRENDERING_EXILED = 11,
	WCA_NCADORNMENTINFO = 12,
	WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
	WCA_VIDEO_OVERLAY_ACTIVE = 14,
	WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
	WCA_DISALLOW_PEEK = 16,
	WCA_CLOAK = 17,
	WCA_CLOAKED = 18,
	WCA_ACCENT_POLICY = 19,
	WCA_FREEZE_REPRESENTATION = 20,
	WCA_EVER_UNCLOAKED = 21,
	WCA_VISUAL_OWNER = 22,
	WCA_HOLOGRAPHIC = 23,
	WCA_EXCLUDED_FROM_DDA = 24,
	WCA_PASSIVEUPDATEMODE = 25,
	WCA_USEDARKMODECOLORS = 26,
	WCA_LAST = 27
};

struct	WINDOWCOMPOSITIONATTRIBDATA
{
	WINDOWCOMPOSITIONATTRIB	Attrib;
	PVOID	pvData;
	SIZE_T	cbData;
};

	static	HTHEME	(WINAPI *OpenNcThemeData)( HWND hWnd, LPCWSTR pszClassList );
	static	void	(WINAPI *RefreshImmersiveColorPolicyState)( void );
	static	bool	(WINAPI *ShouldAppsUseDarkMode)( void );
	static	void	(WINAPI *AllowDarkModeForWindow)( HWND hWnd, bool bAllow );
	static	void	(WINAPI *SetPreferredAppMode)( PreferredAppMode nMode );
	static	void	(WINAPI *SetWindowCompositionAttribute)( HWND hWnd, WINDOWCOMPOSITIONATTRIBDATA* pComposition );

// Undocumented window messages for the menu bar.
// See https://github.com/adzm/win32-custom-menubar-aero-theme for below.

#define WM_UAHDRAWMENU		0x0091	// lParam is UAHMENU

typedef	struct	tagUAHMENU
{
	HMENU		hmenu;
	HDC		hdc;
	DWORD		dwFlags;		// no idea what these mean, in my testing it's either 0x00000a00 or sometimes 0x00000a10
}	UAHMENU;

#define WM_UAHDRAWMENUITEM	0x0092	// lParam is UAHDRAWMENUITEM

typedef	union	tagUAHMENUITEMMETRICS
{
	struct{
		DWORD cx;
		DWORD cy;
	}	rgsizeBar[2];
	struct{
		DWORD cx;
		DWORD cy;
	}	rgsizePopup[4];
}	UAHMENUITEMMETRICS;

typedef	struct	tagUAHMENUPOPUPMETRICS
{
	DWORD		rgcx[4];
	DWORD		fUpdateMaxWidths: 2;	// from kernel symbols, padded to full dword
}	UAHMENUPOPUPMETRICS;

typedef	struct	tagUAHMENUITEM
{
	int		iPosition;		// 0-based position of menu item in menubar
    UAHMENUITEMMETRICS	umim;
    UAHMENUPOPUPMETRICS	umpm;
}	UAHMENUITEM;

typedef	struct	UAHDRAWMENUITEM
{
	DRAWITEMSTRUCT	dis;			// itemID looks uninitialized
	UAHMENU		um;
	UAHMENUITEM	umi;
}	UAHDRAWMENUITEM;

#define	WM_UAHINITMENU		0x0093

#define WM_UAHMEASUREMENUITEM	0x0094	// lParam is UAHMEASUREMENUITEM

typedef	struct	tagUAHMEASUREMENUITEM
{
      MEASUREITEMSTRUCT	mis;
	UAHMENU		um;
	UAHMENUITEM	umi;
}	UAHMEASUREMENUITEM;

///////////////////////////////////////////////////////////////////////////////////////
// Constructor & Destructor

CDark::CDark( void )
{
	m_bSupported  = false;
	m_bDarken     = false;
	m_dwWinVer    = 0;
	m_hUX         = NULL;
	m_hUser       = NULL;

	m_llGDI       = 0;
	m_hWndClicked = NULL;
	m_crFore      = 0;
	m_crBack      = 0;
	m_crGray      = 0;
	m_brBack      = NULL;
	m_hPenEdge    = NULL;
	m_hPenGray    = NULL;
	m_crCombo     = 0;
	m_brCombo     = NULL;
	m_crMenu      = 0;
	m_crMenuHot   = 0;
	m_brMenu      = NULL;
	m_brMenuHot   = NULL;
	m_hFontMenu   = NULL;
	m_hFontMenu   = NULL;
	m_crEdit      = 0;
	m_brEdit      = NULL;
	m_brScrollBar = NULL;
	m_hWndSel     = NULL;

	if	( AfxGetApp()->GetProfileInt( _T("Settings"), _T("EnableDark"), 1 ) ){
		m_dwWinVer = GetWinVer();

		m_hUX   = LoadLibraryEx( _T("uxtheme.dll"), NULL, LOAD_LIBRARY_SEARCH_SYSTEM32 );
		m_hUser = LoadLibraryEx( _T("user32.dll"),  NULL, LOAD_LIBRARY_SEARCH_SYSTEM32 );
		GetDarkThemeAPIs();
		GetDarkThemeSupport();

		Gdiplus::GdiplusStartupInput	si = { 0 };
		si.GdiplusVersion = 1;
		Gdiplus::GdiplusStartup( &m_llGDI, &si, NULL );

		SetColors();
	}
}

CDark::~CDark( void )
{
	if	( m_hUX ){
		FreeLibrary( m_hUX );
		m_hUX = NULL;
	}
	if	( m_hUser ){
		FreeLibrary( m_hUser );
		m_hUser = NULL;
	}

	if	( m_llGDI )
		Gdiplus::GdiplusShutdown( m_llGDI );

	if	( m_brBack )
		DeleteObject( m_brBack );
	if	( m_hPenGray )
		DeleteObject( m_hPenGray );
	if	( m_brCombo )
		DeleteObject( m_brCombo );
	if	( m_brMenu )
		DeleteObject( m_brMenu );
	if	( m_brMenuHot )
		DeleteObject( m_brMenuHot );
	if	( m_brEdit )
		DeleteObject( m_brEdit );
	if	( m_brScrollBar )
		DeleteObject( m_brScrollBar );
}

///////////////////////////////////////////////////////////////////////////////////////
// Interface Functions for Application

void
CDark::Initialize( HWND hWnd )
{
	if	( !m_bSupported )
		return;

	SetDarkThemeEnable( hWnd );

	HookWinProc( hWnd );
}

bool
CDark::IsDarkMode( void )
{
	return	m_bDarken;
}

LRESULT
CDark::PopUp( WPARAM wParam, LPARAM lParam )
{
	CDarkBox	dlg;
	dlg.SetUp( wParam, lParam );

	return	dlg.DoModal();
}

///////////////////////////////////////////////////////////////////////////////////////
// Internal Functions for Dark Theme

DWORD
CDark::GetWinVer( void )
{
	DWORD	dwBuild = 0;
	HMODULE	hNT = LoadLibraryEx( _T("ntdll.dll"), NULL, LOAD_LIBRARY_SEARCH_SYSTEM32 );
	if	( hNT ){
		void	(WINAPI *RtlGetNtVersionNumbers)( LPDWORD major, LPDWORD minor, LPDWORD build ) =
			(void (WINAPI *)( LPDWORD major, LPDWORD minor, LPDWORD build ))
			GetProcAddress( hNT, "RtlGetNtVersionNumbers" );
		DWORD	dwBuild = 0;
		DWORD	dwMajor, dwMinor;
		RtlGetNtVersionNumbers( &dwMajor, &dwMinor, &dwBuild );
		dwBuild &= ~0xF0000000;
		FreeLibrary( hNT );
	}

	DWORD	dwWinVer = 0;
	if	( dwBuild >= 22000 )
		dwWinVer = 11;
	else if	( dwBuild >= 10000 )
		dwWinVer = 10;
	else if	( dwBuild >=  9200 )
		dwWinVer =  8;
	else if	( dwBuild >=  7600 )
		dwWinVer =  7;

	return	dwWinVer;
}

void
CDark::GetDarkThemeAPIs( void )
{
	if	( m_hUX ){
		OpenNcThemeData =
			(HTHEME (WINAPI *)( HWND hWnd, LPCWSTR pszClassList ))
			GetProcAddress( m_hUX, MAKEINTRESOURCEA(49) );
		RefreshImmersiveColorPolicyState =
			(void (WINAPI *)( void ))
			GetProcAddress( m_hUX, MAKEINTRESOURCEA(104) );
		ShouldAppsUseDarkMode =
			(bool (WINAPI *)( void ))
			GetProcAddress( m_hUX, MAKEINTRESOURCEA(132) );
		AllowDarkModeForWindow =
			(void (WINAPI *)( HWND hWnd, bool bAllow ))
			GetProcAddress( m_hUX, MAKEINTRESOURCEA(133) );
		SetPreferredAppMode =
			(void (WINAPI *)( PreferredAppMode nMode ))
			GetProcAddress( m_hUX, MAKEINTRESOURCEA(135) );
	}

	if	( m_hUser ){
		SetWindowCompositionAttribute =
			(void (WINAPI *)( HWND hWnd, WINDOWCOMPOSITIONATTRIBDATA* pComposition ))
			GetProcAddress( m_hUser, "SetWindowCompositionAttribute" );
	}
}

void
CDark::GetDarkThemeSupport( void )
{
	m_bSupported = false;
	m_bDarken    = false;

	if	( OpenNcThemeData && RefreshImmersiveColorPolicyState && ShouldAppsUseDarkMode &&
		  AllowDarkModeForWindow && SetPreferredAppMode &&
		  SetWindowCompositionAttribute ){
		m_bSupported = true;

		SetPreferredAppMode( AllowDark );
		RefreshImmersiveColorPolicyState();

		m_bDarken = ShouldAppsUseDarkMode();

		HookScrollBar();
	}
}

#pragma comment( lib, "Dwmapi.lib" )

void
CDark::SetDarkThemeEnable( HWND hWnd )
{
	BOOL	bValue = TRUE;
	DwmSetWindowAttribute( hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &bValue, sizeof( bValue ) );

	AllowDarkModeForWindow( hWnd, true );

	BOOL	bDark = m_bDarken? TRUE: FALSE;
	WINDOWCOMPOSITIONATTRIBDATA data = { WCA_USEDARKMODECOLORS, &bDark, sizeof( bDark ) };
	SetWindowCompositionAttribute( hWnd, &data );
}

void
CDark::HookScrollBar( void )
{
	HMODULE	hComCtrl = LoadLibraryEx( _T("comctl32.dll"), NULL, LOAD_LIBRARY_SEARCH_SYSTEM32 );
	if	( hComCtrl ){
		IMAGE_THUNK_DATA*	pThunk = FindThunk( hComCtrl, "uxtheme.dll", 49 ); // OpenNcThemeData
		if	( pThunk ){
			DWORD	dwProtectOld;
			if	( VirtualProtect( pThunk, sizeof( IMAGE_THUNK_DATA ), PAGE_READWRITE, &dwProtectOld ) ){
				pThunk->u1.Function = (ULONG_PTR)HookedClassProc;
				VirtualProtect( pThunk, sizeof( IMAGE_THUNK_DATA ), dwProtectOld, &dwProtectOld );
			}
		}
		FreeLibrary( hComCtrl );
	}
}

HTHEME	WINAPI
CDark::HookedClassProc( HWND hWnd, LPCWSTR pszClassList )
{
	if	( !wcscmp( pszClassList, L"ScrollBar" ) ){
		hWnd = NULL;
		pszClassList = L"Explorer::ScrollBar";
	}
	else{//DBG
		CString	str;
		str.Format( _T("CDark: HookedClassProc got unknown class '%ls'."), pszClassList );
		AfxMessageBox( str );
	}

	return	OpenNcThemeData( hWnd, pszClassList );
}

IMAGE_THUNK_DATA	*
CDark::FindThunk( void* pBase, char* pchDLL, WORD wOrdinal )
{
	IMAGE_DOS_HEADER*		pDOS = (IMAGE_DOS_HEADER*)pBase;
	IMAGE_NT_HEADERS*		pNT  = (IMAGE_NT_HEADERS*)( (ULONG_PTR)pBase + pDOS->e_lfanew );
	IMAGE_DATA_DIRECTORY*		pDir = pNT->OptionalHeader.DataDirectory;
	DWORD				dwVA = pDir[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress;
	IMAGE_DELAYLOAD_DESCRIPTOR*	pImport = (IMAGE_DELAYLOAD_DESCRIPTOR*)( (ULONG_PTR)pBase + dwVA );

	for	( ; pImport->DllNameRVA; pImport++ ){
		char*	pchName = (char *)( (ULONG_PTR)pBase + pImport->DllNameRVA );
		if	( _stricmp( pchName, pchDLL ) )
			continue;

		IMAGE_THUNK_DATA*	pName = (IMAGE_THUNK_DATA*)( (ULONG_PTR)pBase + pImport->ImportNameTableRVA );
		IMAGE_THUNK_DATA*	pAddr = (IMAGE_THUNK_DATA*)( (ULONG_PTR)pBase + pImport->ImportAddressTableRVA );

		for	( ; pName->u1.Ordinal; pName++, pAddr++ ){
			if	( IMAGE_SNAP_BY_ORDINAL( pName->u1.Ordinal ) &&
				  IMAGE_ORDINAL( pName->u1.Ordinal ) == wOrdinal )
				return	pAddr;
		}
	}

	return	NULL;
}

///////////////////////////////////////////////////////////////////////////////////////
// Internal Functions to Darken Controls

void
CDark::SetColors( void )
{
	m_crFore      = m_bDarken? RGB( 247, 247, 247 ): RGB(  40,  40,  40 );
	m_crBack      = m_bDarken? RGB(  40,  40,  40 ): RGB( 243, 243, 243 );
	m_crGray      = m_bDarken? RGB( 127, 127, 127 ): RGB( 127, 127, 127 );
	m_crCombo     = m_crBack;
	m_crMenu      = m_bDarken? RGB(  31,  31,  31 ): RGB( 249, 249, 249 );
	m_crMenuHot   = m_bDarken? RGB(  44,  44,  44 ): RGB( 240, 240, 240 );
	m_crEdit      = m_bDarken? RGB(  33,  33,  33 ): RGB( 249, 249, 249 );

	DeleteObject( m_brBack );
	DeleteObject( m_hPenGray );
	DeleteObject( m_hPenEdge );
	DeleteObject( m_brMenu );
	DeleteObject( m_brMenuHot );
	DeleteObject( m_brEdit );
	DeleteObject( m_brScrollBar );

	m_brBack      = CreateSolidBrush( m_crBack );
	m_hPenGray    = CreatePen( PS_SOLID, 1, m_crGray );
	m_hPenEdge    = CreatePen( PS_SOLID, 5, m_crEdit );
	m_brCombo     = NULL;
	m_brMenu      = CreateSolidBrush( m_crMenu );
	m_brMenuHot   = CreateSolidBrush( m_crMenuHot );
	m_brEdit      = CreateSolidBrush( m_crEdit );
	m_brScrollBar = CreateSolidBrush( RGB(  23,  23,  23 ) );
}

void
CDark::HookWinProc( HWND hWnd )
{
	// Hook the top window.

	HookWindow( hWnd, OnWindow );
	SetWndUIState( hWnd, UISF_ACTIVE | UISF_HIDEACCEL | UISF_HIDEFOCUS );

	// Get the class name of the window.

	TCHAR	achClass[_MAX_PATH];
	GetClassName( hWnd, achClass, _MAX_PATH );
	CString	strClass = achClass;

	// Hook controls on the dialog class.

	HWND	hWndChild;

	if	( strClass == _T("#32770") ){

		for	( hWndChild = NULL;; ){
			hWndChild = FindWindowEx( hWnd, hWndChild, _T("Button"), NULL );
			if	( hWndChild )
				HookWindow( hWndChild, OnButton );
			else
				break;
		}

		for	( hWndChild = NULL;; ){
			hWndChild = FindWindowEx( hWnd, hWndChild, _T("ComboBox"), NULL );
			if	( hWndChild )
				HookWindow( hWndChild, OnComboBox );
			else
				break;
		}

		for	( hWndChild = NULL;; ){
			hWndChild = FindWindowEx( hWnd, hWndChild, _T("SysDateTimePick32"), NULL );
			if	( hWndChild ){
				HookWindow( hWndChild, OnDateTime );
				HWND	hWndSub = NULL;
				hWndSub = FindWindowEx( hWndChild, hWndSub, _T("msctls_updown32"), NULL );
				if	( hWndSub )
					HookWindow( hWndSub, OnSpin );
			}
			else
				break;
		}

		for	( hWndChild = NULL;; ){
			hWndChild = FindWindowEx( hWnd, hWndChild, _T("Edit"), NULL );
			if	( hWndChild )
				HookWindow( hWndChild, OnEdit );
			else
				break;
		}

		for	( hWndChild = NULL;; ){
			hWndChild = FindWindowEx( hWnd, hWndChild, _T("Static"), NULL );
			if	( hWndChild )
				HookWindow( hWndChild, OnStatic );
			else
				break;
		}

		for	( hWndChild = NULL;; ){
			hWndChild = FindWindowEx( hWnd, hWndChild, _T("msctls_updown32"), NULL );
			if	( hWndChild )
				HookWindow( hWndChild, OnSpin );
			else
				break;
		}
	}

	// Hook subwindows on the other classes.

	else{

		for	( hWndChild = NULL;; ){
			hWndChild = FindWindowEx( hWnd, hWndChild, _T("msctls_statusbar32"), NULL );
			if	( hWndChild )
				HookWindow( hWndChild, OnStatusBar );
			else
				break;
		}
		for	( hWndChild = NULL;; ){
			hWndChild = FindWindowEx( hWnd, hWndChild, NULL, NULL );
			if	( hWndChild ){
				DWORD	dwStyle = GetWindowLong( hWndChild, GWL_STYLE );
				if	( dwStyle & ( WS_VSCROLL | WS_HSCROLL ) )
					HookWindow( hWndChild, OnScrollBars );
			}
			else
				break;
		}
	}
}

void
CDark::HookWindow( HWND hWnd, SUBCLASSPROC pfnWnd )
{
	CHook	hook;
	hook.hWnd = hWnd;
	hook.pfnWnd = pfnWnd;
	m_aHook.Add( hook );

	HWND	hwndParent = GetAncestor( hWnd, GA_PARENT );
	if	( hwndParent == GetDesktopWindow() ){
		CState	state;
		state.hWnd = hWnd;
		m_aState.Add( state );
	}

	SetWindowSubclass( hWnd, pfnWnd, 0, (DWORD_PTR)this );
}

void
CDark::UnhookWindow( HWND hWnd )
{
	INT_PTR	n;
	
	n = m_aHook.GetCount();
	for	( INT_PTR i = n-1; i >= 0; i-- ){
		CHook&	hook = m_aHook[i];
		if	( hWnd == hook.hWnd )
			;
		else if	( hWnd == GetAncestor( hook.hWnd, GA_PARENT ) )
			;
		else if	( hWnd == GetAncestor( hook.hWnd, GA_ROOTOWNER ) )
			;
		else
			continue;

		RemoveWindowSubclass( hook.hWnd, hook.pfnWnd, 0 );
		m_aHook.RemoveAt( i );
	}

	n = m_aState.GetCount();
	for	( INT_PTR i = n-1; i >= 0; i-- )
		if	( m_aState[i].hWnd == hWnd ){
			m_aState.RemoveAt( i );
			break;
		}
}

LRESULT
CDark::OnWindow( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef )
{
	CDark*	that = (CDark*)dwRef;
	return	that->OnMsgWindow( hWnd, uMsg, wParam, lParam );
}

LRESULT
CDark::OnMsgWindow( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//	TraceMsg( _T("Window"), hWnd, uMsg );//DBG

	if	( uMsg == WM_CHANGEUISTATE ){

		// Get the states from WPARAM.

		WORD	wAction = (WORD)(   wParam         & 0xffff );
		WORD	wStyles = (WORD)( ( wParam >> 16 ) & 0xffff );

		// Write the states into the window's long.

		DWORD dwState =
		GetWndUIState( hWnd );
		if	( wAction == UIS_INITIALIZE )
			dwState  = UISF_ACTIVE | UISF_HIDEACCEL | UISF_HIDEFOCUS;
		else if	( wAction == UIS_CLEAR )
			dwState &= ~wStyles;
		else if	( wAction == UIS_SET )
			dwState |=  wStyles;
		SetWndUIState( hWnd, dwState );
	}
	else if	( uMsg == WM_CLOSE ){
		UnhookWindow( hWnd );
	}
	else if	( uMsg == WM_CTLCOLOREDIT ){
		HDC	hDC = (HDC)wParam;
		HBRUSH	hBrushDef = GetSysColorBrush( COLOR_WINDOW );
		HBRUSH	hBrushRet = (HBRUSH)DefSubclassProc( hWnd, uMsg, wParam, lParam );

		// When WM_CTLCOLOREDIT is not handled, overwrite with darken colors.
		if	( hBrushRet == hBrushDef ){
			hBrushRet        = m_brEdit;
			SetTextColor( hDC, m_crFore );
			SetBkColor(   hDC, m_crEdit );
		}

		return	(LRESULT)hBrushRet;
	}
	else if	( uMsg == WM_CTLCOLORSTATIC ){
		HDC	 hDC = (HDC)wParam;
		HBRUSH	 hBrushDef = GetSysColorBrush( COLOR_3DFACE );
		COLORREF crTextDef = GetSysColor( COLOR_WINDOWTEXT );
		COLORREF crBackDef = GetSysColor( COLOR_3DFACE );
		HBRUSH	 hBrushRet = (HBRUSH)DefSubclassProc( hWnd, uMsg, wParam, lParam );
		COLORREF crTextRet = GetTextColor( hDC );
		COLORREF crBackRet = GetBkColor(   hDC );

		// When WM_CTLCOLORSTATIC is not handled, overwrite with darken colors.
		if	( hBrushRet == hBrushDef )
			hBrushRet = m_brBack;
		if	( crTextRet == crTextDef )
			SetTextColor( hDC, m_crFore );
		if	( crBackRet == crBackDef )
			SetBkColor(   hDC, m_crBack );

		return	(LRESULT)hBrushRet;
	}
	else if	( uMsg == WM_ERASEBKGND ){
		CRect	rect;
		GetClientRect( hWnd, &rect );
		HDC	hDC = (HDC)wParam;
		CDC*	pDC = CDC::FromHandle( hDC );
		pDC->FillSolidRect( &rect, m_crBack );

		return	0;
	}
	else if	( uMsg == WM_NCPAINT ||
		  uMsg == WM_NCACTIVATE ){

		// Do as normal FIRST.

		LRESULT	lr = DefSubclassProc( hWnd, uMsg, wParam, lParam );

		// THEN overwrite the annoying line above the client area.

		CRect	rectLine, rectWindow;
		GetClientRect( hWnd, &rectLine );
		GetWindowRect( hWnd, &rectWindow );
		MapWindowPoints( hWnd, HWND_DESKTOP, (POINT*)&rectLine, 2 );
		OffsetRect( &rectLine, -rectWindow.left, -rectWindow.top );
		rectLine.bottom = rectLine.top;
		rectLine.top -= 3;

		HDC	hDC = GetWindowDC( hWnd );
		bool	bFocused = GetForegroundWindow() == hWnd;
		FillRect( hDC, &rectLine, bFocused? m_brMenu: m_brMenuHot );
		rectLine.top = rectLine.bottom-1;
		FillRect( hDC, &rectLine, bFocused? m_brMenuHot: m_brMenu );
		ReleaseDC( hWnd, hDC );

		return	lr;
	}
	else if	( uMsg == WM_SYSCOLORCHANGE ){
		GetDarkThemeSupport();
		SetColors();
		RedrawWindow( hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN );
		RedrawWindow( hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN );
		//YET: NC_ACTIVATE? No, You need to click the scroll bar. I don't know why.
	}
	else if	( uMsg == WM_UAHDRAWMENU ){

		// Get parameters from LPARAM.

		UAHMENU*	pMenu = (UAHMENU*)lParam;
		MENUBARINFO	mbi = {};
		mbi.cbSize = sizeof( mbi );
		GetMenuBarInfo( hWnd, OBJID_MENU, 0, &mbi );

		// Draw the menu bar.

		CRect	rect = mbi.rcBar;
		CRect	rectWindow;
		GetWindowRect( hWnd, &rectWindow );
		OffsetRect( &rect, -rectWindow.left, -rectWindow.top );
		bool	bFocused = GetForegroundWindow() == hWnd;
		FillRect( pMenu->hdc, &rect, bFocused? m_brMenu: m_brMenuHot );

		return	0;
	}
	else if	( uMsg == WM_UAHDRAWMENUITEM ){

		// Get parameters from LPARAM.

		UAHDRAWMENUITEM*	pItem = (UAHDRAWMENUITEM*)lParam;
		CRect	rect  = pItem->dis.rcItem;
		HDC	hDC   = pItem->um.hdc;
		HMENU	hMenu = pItem->um.hmenu;

		// Get the name of menu item.

		TCHAR	achName[_MAX_PATH] = { 0 };
		MENUITEMINFO	mii = {};
		mii.cbSize = sizeof( mii );
		mii.fMask  = MIIM_STRING;
		mii.fType  = MFT_STRING;
		mii.cch    = _MAX_PATH;
		mii.dwTypeData = achName;
		GetMenuItemInfo( hMenu, pItem->umi.iPosition, TRUE, &mii );
		CString	strName = achName;

		// Draw the name of the menu item.

		CDC*	pDC = CDC::FromHandle( hDC );
		int	nState = pItem->dis.itemState;
		DWORD	dwDT = DT_SINGLELINE | DT_CENTER | DT_BOTTOM;
		bool	bFocused = GetForegroundWindow() == hWnd;
		pDC->FillSolidRect( &rect, ( nState & ( ODS_HOTLIGHT | ODS_SELECTED ) )? m_crMenuHot: bFocused? m_crMenu: m_crMenuHot );
		if	( nState & ODS_NOACCEL )
			strName.Replace( _T("&"), _T("") );

		pDC->SetTextColor( m_crFore );
		pDC->DrawText( strName, &rect, dwDT );

		m_hFontMenu = ::GetCurrentObject( hDC, OBJ_FONT );
		return	0;
	}

	return	DefSubclassProc( hWnd, uMsg, wParam, lParam );
}

LRESULT
CDark::OnButton( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef )
{
	CDark*	that = (CDark*)dwRef;
	return	that->OnMsgButton( hWnd, uMsg, wParam, lParam );
}

LRESULT
CDark::OnMsgButton( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//	TraceMsg( _T("Button"), hWnd, uMsg );//DBG

	if	( uMsg == WM_CTLCOLOR ){
		HDC	hDC = GetDC( hWnd );
		SetBkColor(   hDC, m_crBack );
		SetTextColor( hDC, m_crFore );
		ReleaseDC( hWnd, hDC );

		return	(LRESULT)m_brBack;
	}
	else if	( uMsg == WM_ENABLE ){
		RedrawWindow( hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW );

		return	0;
	}
	else if	( uMsg == WM_KILLFOCUS ){
		CRect	rect;
		GetClientRect( hWnd, &rect );
		HDC	hDC = GetDC( hWnd );
		FillRect( hDC, &rect, m_brBack );
		ReleaseDC( hWnd, hDC );
	}
	else if	( uMsg == WM_LBUTTONDOWN )
		m_hWndClicked = hWnd;

	else if	( uMsg == WM_LBUTTONUP )
		m_hWndClicked = NULL;

	else if	( uMsg == WM_PAINT ){
		PAINTSTRUCT	ps;
		HDC		hDC = BeginPaint( hWnd, &ps );
		CRect	rect;
		GetClientRect( hWnd, &rect );
		OnPaintButton( hWnd, hDC, rect );
		EndPaint( hWnd, &ps );

		return	0;
	}
	else if	( uMsg == WM_UPDATEUISTATE ){	// once to draw focus rect

		// Get the states from WPARAM.

		WORD	wAction = (WORD)(   wParam         & 0xffff );
		WORD	wStyles = (WORD)( ( wParam >> 16 ) & 0xffff );

		// Write the states into the window's long.

		DWORD dwState =
		GetWndUIState( hWnd );
		if	( wAction == UIS_INITIALIZE )
			dwState  = UISF_ACTIVE | UISF_HIDEACCEL | UISF_HIDEFOCUS;
		else if	( wAction == UIS_CLEAR )
			dwState &= ~wStyles;
		else if	( wAction == UIS_SET )
			dwState |=  wStyles;
		SetWndUIState( hWnd, dwState );

		// Redraw the window with/without underlines for ALT shortcut.

		RedrawWindow( hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW );

		return	0;
	}

	return	DefSubclassProc( hWnd, uMsg, wParam, lParam );
}

void
CDark::OnPaintButton( HWND hWnd, HDC hDC, CRect rect )
{
	DWORD	dwStyle = GetWindowLong( hWnd, GWL_STYLE );
	UINT	uStyle  = dwStyle & BS_TYPEMASK;

	HFONT	hFont = (HFONT)SendMessage( hWnd, WM_GETFONT, 0, 0 );
	SelectObject( hDC, hFont );

	if	( uStyle == BS_CHECKBOX ||
		  uStyle == BS_AUTOCHECKBOX )
		OnPaintCheckBox( hWnd, hDC, rect );
	
	else if	( uStyle == BS_PUSHBUTTON ||
		  uStyle == BS_DEFPUSHBUTTON )
		OnPaintPushButton( hWnd, hDC, rect );

	else if	( uStyle == BS_RADIOBUTTON ||
		  uStyle == BS_AUTORADIOBUTTON )
		OnPaintRadioButton( hWnd, hDC, rect );

	else if	( uStyle == BS_GROUPBOX )
		OnPaintGroupBox( hWnd, hDC, rect );
}

void
CDark::OnPaintCheckBox( HWND hWnd, HDC hDC, CRect rect )
{
	HWND	hWndFocus = GetFocus();

	CPoint	pt;
	GetCursorPos( &pt );
	ScreenToClient( hWnd, &pt );

	CWnd*	pWnd     = CWnd::FromHandle( hWnd );
	bool	bChecked = ((CButton*)pWnd)->GetCheck() == BST_CHECKED;
	bool	bHot     = rect.PtInRect( pt );
	bool	bFocus   = hWndFocus == hWnd;

	COLORREF	crInner;
	COLORREF	crEdge;
	if	( bChecked )
		if	( m_bDarken )
			if	( bHot ){
				crInner = RGB( 153, 152, 148 );
				crEdge  = crInner;
			}
			else{
				crInner = RGB( 166, 165, 161 );
				crEdge  = crInner;
			}
		else
			if	( bHot ){
				crInner = RGB(  84,  82,  80 );
				crEdge  = crInner;
			}
			else{
				crInner = RGB(  65,  63,  61 );
				crEdge  = crInner;
			}
	else
		if	( m_bDarken )
			if	( bHot ){
				crInner = RGB(  52,  52,  52 );
				crEdge  = RGB( 160, 160, 160 );
			}
			else{
				crInner = RGB(  39,  39,  39 );
				crEdge  = RGB( 158, 158, 158 );
			}
		else
			if	( bHot ){
				crInner = RGB( 236, 236, 236 );
				crEdge  = RGB( 135, 135, 135 );
			}
			else{
				crInner = RGB( 245, 245, 245 );
				crEdge  = RGB( 137, 137, 137 );
			}

	bool	bTab, bAlt;
	GetWndTabAlt( hWnd, bTab, bAlt );

	CString	strTitle;
	pWnd->GetWindowText( strTitle );
	if	( !bAlt )
		strTitle.Replace( _T("&"), _T("") );

	SetTextColor( hDC, m_crFore );
	SetBkColor(   hDC, m_crBack );
	SetBkMode( hDC, OPAQUE );

	CRect	rectMark = rect;
	rectMark.right = rectMark.left + rectMark.Height();
	rectMark.bottom--;
	FillRect( hDC, &rectMark, m_brBack );
	GdipRoundRect( hDC, rectMark, CPoint( 6, 6 ), crInner, crEdge );

	if	( bChecked )
		GdipCheckMark( hDC, rectMark, m_crBack );

	rect.left += rect.Height() + rect.Height()/4;
	rect.top  += rect.Height()/8;

	DrawText( hDC, strTitle.GetBuffer(), strTitle.GetLength(), &rect, DT_SINGLELINE );

	if	( bTab )
		if	( bFocus )
			DrawFocusRect( hDC, &rect );
}

void
CDark::OnPaintPushButton( HWND hWnd, HDC hDC, CRect rect )
{
	CPoint	pt;
	GetCursorPos( &pt );
	ScreenToClient( hWnd, &pt );

	HWND	hWndFocus = GetFocus();
	DWORD	dwStyle   = GetWindowLong( hWnd, GWL_STYLE );
	bool	bClicked  = m_hWndClicked == hWnd;
	bool	bHot      = rect.PtInRect( pt );
	bool	bFocus    = hWndFocus == hWnd;
	bool	bDefault  = dwStyle & BS_DEFPUSHBUTTON;
	bool	bDisabled = dwStyle & WS_DISABLED;

	COLORREF	crInner;
	COLORREF	crEdge;
	COLORREF	crText = bDisabled? m_crGray: m_crFore;
	if	( bClicked )
		if	( m_bDarken )
			if	( bDisabled ){
				crInner = RGB(  36,  36,  36 );
				crEdge  = RGB(  40,  40,  40 );
			}
			else if	( bHot ){
				crInner = RGB(  39,  39,  39 );
				crEdge  = RGB(  48,  48,  48 );
			}
			else{
				crInner = RGB(  39,  39,  39 );
				crEdge  = RGB(  48,  48,  48 );
			}
		else
			if	( bDisabled ){
				crInner = RGB( 250, 250, 250 );
				crEdge  = RGB( 240, 240, 240 );
			}
			else if	( bHot ){
				crInner = RGB( 245, 245, 245 );
				crEdge  = RGB( 229, 229, 229 );
			}
			else{
				crInner = RGB( 245, 245, 245 );
				crEdge  = RGB( 229, 229, 229 );
			}
	else
		if	( m_bDarken )
			if	( bDisabled ){
				crInner = RGB(  36,  36,  36 );
				crEdge  = RGB(  40,  40,  40 );
			}
			else if	( bHot ){
				crInner = RGB(  50,  50,  50 );
				crEdge  = RGB(  53,  53,  53 );
			}
			else{
				crInner = RGB(  45,  45,  45 );
				crEdge  = RGB(  53,  53,  53 );
			}
		else
			if	( bDisabled ){
				crInner = RGB( 250, 250, 250 );
				crEdge  = RGB( 240, 240, 240 );
			}
			else if	( bHot ){
				crInner = RGB( 246, 246, 246 );
				crEdge  = RGB( 204, 204, 204 );
			}
			else{
				crInner = RGB( 251, 251, 251 );
				crEdge  = RGB( 204, 204, 204 );
			}

	bool	bTab, bAlt;
	GetWndTabAlt( hWnd, bTab, bAlt );

	if	( !bTab && bDefault )
		crEdge = (COLORREF)GetSysColor( COLOR_HOTLIGHT );

	CWnd*	pWnd = CWnd::FromHandle( hWnd );

	CString	strTitle;
	pWnd->GetWindowText( strTitle );
	if	( !bAlt )
		strTitle.Replace( _T("&"), _T("") );

	SetTextColor( hDC, crText );
	SetBkMode( hDC, TRANSPARENT );

	rect.right--;
	rect.bottom--;
	GdipRoundRect( hDC, rect, CPoint( 4, 4 ), crInner, crEdge );

	DrawText( hDC, strTitle.GetBuffer(), strTitle.GetLength(), &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER );

	if	( bTab )
		if	( bFocus ){
			rect.left   += 3;
			rect.top    += 3;
			rect.right  -= 2;
			rect.bottom -= 2;
			HDC	hDCn = GetDC( hWnd );
			DrawFocusRect( hDCn, &rect );
			ReleaseDC( hWnd, hDC );
		}
}

void
CDark::OnPaintRadioButton( HWND hWnd, HDC hDC, CRect rect )
{
	HWND	hWndFocus = GetFocus();

	CPoint	pt;
	GetCursorPos( &pt );
	ScreenToClient( hWnd, &pt );

	CWnd*	pWnd = CWnd::FromHandle( hWnd );
	bool	bChecked = ((CButton*)pWnd)->GetCheck() == BST_CHECKED;
	bool	bHot = rect.PtInRect( pt );
	bool	bFocus = hWndFocus == hWnd;

	CRect	rectMark = rect;
	rectMark.right = rectMark.left + rectMark.Height();
	FillRect( hDC, &rectMark, m_brBack );
	rectMark.right--;
	rectMark.bottom--;

	COLORREF	crInner;
	COLORREF	crEdge;
	int		nThickness;
	if	( bChecked )
		if	( m_bDarken )
			if	( bHot ){
				crInner = RGB(   0,   0,   0 );
				crEdge  = RGB( 152, 151, 147 );
				nThickness = 3;
			}
			else{
				crInner = RGB(   0,   0,   0 );
				crEdge  = RGB( 166, 165, 161 );
				nThickness = 4;
			}
		else
			if	( bHot ){
				crInner = RGB( 255, 255, 255 );
				crEdge  = RGB(  81,  79,  77 );
				nThickness = 3;
			}
			else{
				crInner = RGB( 255, 255, 255 );
				crEdge  = RGB(  65,  63,  61 );
				nThickness = 4;
			}
	else
		if	( m_bDarken )
			if	( bHot ){
				crInner = RGB(  42,  42,  42 );
				crEdge  = RGB( 155, 155, 155 );
				nThickness = 1;
			}
			else{
				crInner = RGB(  29,  29,  29 );
				crEdge  = RGB( 153, 153, 153 );
				nThickness = 1;
			}
		else
			if	( bHot ){
				crInner = RGB( 229, 229, 229 );
				crEdge  = RGB( 139, 139, 139 );
				nThickness = 1;
			}
			else{
				crInner = RGB( 237, 237, 237 );
				crEdge  = RGB( 135, 135, 135 );
				nThickness = 1;
			}

	bool	bTab, bAlt;
	GetWndTabAlt( hWnd, bTab, bAlt );

	CString	strTitle;
	pWnd->GetWindowText( strTitle );
	if	( !bAlt )
		strTitle.Replace( _T("&"), _T("") );

	SetTextColor( hDC, m_crFore );
	SetBkColor(   hDC, m_crBack );
	SetBkMode( hDC, OPAQUE );

	GdipCircle( hDC, rectMark, nThickness, crInner, crEdge );

	rect.left += rect.Height() + rect.Height()/8;
	rect.top  += rect.Height()/8;

	DrawText( hDC, strTitle.GetBuffer(), strTitle.GetLength(), &rect, DT_SINGLELINE );

	if	( bTab )
		if	( bFocus )
			DrawFocusRect( hDC, &rect );
}

void
CDark::OnPaintGroupBox( HWND hWnd, HDC hDC, CRect rect )
{
	SelectObject( hDC, GetStockObject( m_bDarken? WHITE_PEN: BLACK_PEN ) );
	SelectObject( hDC, GetStockObject( NULL_BRUSH ) );

	rect.top    += 6;
	rect.right  -= 1;
	rect.bottom -= 1;

	GdipRoundRect( hDC, rect, CPoint( 8, 8 ), -1, m_crGray );
}

LRESULT
CDark::OnComboBox( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef )
{
	CDark*	that = (CDark*)dwRef;
	return	that->OnMsgComboBox( hWnd, uMsg, wParam, lParam );
}

LRESULT
CDark::OnMsgComboBox( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//	TraceMsg( _T("ComboBox"), hWnd, uMsg );//DBG

	if	( uMsg == WM_CTLCOLOREDIT ||
		  uMsg == WM_CTLCOLORLISTBOX )
		return	OnCtlColorEdit( hWnd, wParam );

	else if	( uMsg == WM_KILLFOCUS ){
		RedrawWindow( hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW );
	}
	else if	( uMsg == WM_LBUTTONDOWN )
		m_hWndClicked = hWnd;

	else if	( uMsg == WM_LBUTTONUP )
		m_hWndClicked = NULL;

	else if	( uMsg == WM_PAINT ){
		PAINTSTRUCT	ps;
		HDC		hDC = BeginPaint( hWnd, &ps );
		CRect	rect;
		GetClientRect( hWnd, &rect );
		OnPaintComboBox( hWnd, hDC, rect );
		EndPaint( hWnd, &ps );
		return	0;
	}

	return	DefSubclassProc( hWnd, uMsg, wParam, lParam );
}

void
CDark::OnPaintComboBox( HWND hWnd, HDC hDC, CRect rect )
{
	HWND	hWndFocus = GetFocus();
	HWND	hWndEdit  = GetWindow( hWnd, GW_CHILD );

	CPoint	pt;
	GetCursorPos( &pt );
	ScreenToClient( hWnd, &pt );

	bool	bHot = rect.PtInRect( pt );
	bool	bFocus = hWndFocus == hWnd;

	CWnd*	pWnd = CWnd::FromHandle( hWnd );
	bool	bDrop = ((CComboBox*)pWnd)->GetDroppedState()? true: false;

	COLORREF	crInner;
	COLORREF	crEdge;
	COLORREF	crMark;
	{
		if	( m_bDarken )
			if	( bHot ){
				crInner = RGB(  60,  60,  60 );
				crEdge  = RGB(  63,  63,  63 );
				crMark  = RGB( 185, 185, 185 );
			}
			else{
				crInner = RGB(  57,  57,  57 );
				crEdge  = RGB(  65,  65,  65 );
				crMark  = RGB( 185, 185, 185 );
			}
		else
			if	( bHot ){
				crInner = RGB( 250, 250, 250 );
				crEdge  = RGB( 236, 236, 236 );
				crMark  = RGB(  70,  70,  70 );
			}
			else{
				crInner = RGB( 254, 254, 254 );
				crEdge  = RGB( 236, 236, 236 );
				crMark  = RGB(  70,  70,  70 );
			}
	}

	rect.bottom--;
	rect.right--;
	GdipRoundRect( hDC, rect, CPoint( 8, 8 ), crInner, crEdge );
	GdipArrow( hDC, rect, crMark, bDrop );

	m_crCombo = crInner;
	DeleteObject( m_brCombo );
	m_brCombo = CreateSolidBrush( m_crCombo );

	UINT	uStyle = ((CButton*)pWnd)->GetButtonStyle() & BS_TYPEMASK;
	if	( uStyle == CBS_DROPDOWNLIST ){
		CString	strSel;
		{
			CWnd*	pWnd = CWnd::FromHandle( hWnd );
			CComboBox*	pCombo = (CComboBox*)pWnd;
			int	iSel = pCombo->GetCurSel();
			pCombo->GetLBText( iSel, strSel );
		}
		HFONT	hFont = (HFONT)SendMessage( hWnd, WM_GETFONT, 0, 0 );
		SelectObject( hDC, hFont );
		SetTextColor( hDC, m_crFore );
		SetBkMode( hDC, TRANSPARENT );
		rect.left += 5;
		DrawText( hDC, strSel.GetBuffer(), strSel.GetLength(), &rect, DT_SINGLELINE | DT_VCENTER );

		bool	bTab, bAlt;
		GetWndTabAlt( hWnd, bTab, bAlt );

		if	( bTab )
			if	( bFocus ){
				rect.left   -= 2;
				rect.right  -= rect.Height();
				rect.top    += 3;
				rect.bottom -= 2;
				HDC	hDCn = GetDC( hWnd );
				DrawFocusRect( hDCn, &rect );
				ReleaseDC( hWnd, hDC );
			}
	}
}

LRESULT
CDark::OnDateTime( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef )
{
	CDark*	that = (CDark*)dwRef;
	return	that->OnMsgDateTime( hWnd, uMsg, wParam, lParam );
}

LRESULT
CDark::OnMsgDateTime( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//	TraceMsg( _T("DateTime"), hWnd, uMsg );//DBG

	int	xMargin = 2;

	if	( uMsg == WM_ERASEBKGND ){
		return	0;
	}
	else if	( uMsg == WM_LBUTTONDOWN ){
		OnClickDateTime( hWnd, wParam, lParam );
	}
	else if	( uMsg == WM_KILLFOCUS ){
		if	( hWnd == m_hWndSel ){
			m_hWndSel = NULL;
			m_rectSel.SetRectEmpty();
		}
	}
	else if	( uMsg == WM_PAINT ){
		PAINTSTRUCT	ps;
		HDC		hDC = BeginPaint( hWnd, &ps );
		bool	bFocus = GetFocus() == hWnd;

		CRect	rect;
		GetClientRect( hWnd, &rect );
		FillRect( hDC, &rect, m_brEdit );

		HFONT	hFont = (HFONT)SendMessage( hWnd, WM_GETFONT, 0, 0 );
		SelectObject( hDC, hFont );
		SetTextColor( hDC, m_crFore );
		SetBkMode( hDC, TRANSPARENT );
		rect.left += xMargin;

		CWnd*	pWnd = CWnd::FromHandle( hWnd );
		CString	str;
		pWnd->GetWindowText( str );
		DrawText( hDC, str.GetBuffer(), str.GetLength(), &rect, DT_SINGLELINE | DT_VCENTER );

		GdipArrow( hDC, rect, m_crFore, false );

		if	( hWnd == m_hWndSel ){
			SelectObject( hDC, m_hPenGray );
			SelectObject( hDC, GetStockObject( NULL_BRUSH ) );
			Rectangle( hDC, m_rectSel.left, m_rectSel.top, m_rectSel.right, m_rectSel.bottom );
		}

		EndPaint( hWnd, &ps );
	}
	else if	( uMsg == WM_UPDATEUISTATE ){
		HWND	hWndSpin  = GetWindow( hWnd, GW_CHILD );
		if	( hWndSpin ){
			CRect	rectSpin;
			GetClientRect(  hWndSpin, rectSpin );
			InvalidateRect( hWndSpin, rectSpin, FALSE );
		}
	}

	return	DefSubclassProc( hWnd, uMsg, wParam, lParam );
}

void
CDark::OnClickDateTime( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	int	xMargin = 2;
	int	cyDigit, cxDigit, cxSep;

	{
		HDC	hDC = GetDC( hWnd );
		HFONT	hFont = (HFONT)SendMessage( hWnd, WM_GETFONT, 0, 0 );
		SelectObject( hDC, hFont );

		CSize	size;
		GetTextExtentPoint32( hDC, _T("0"),   1, &size );
		cyDigit = size.cy;
		cxDigit = size.cx;
		GetTextExtentPoint32( hDC, _T("/"),   1, &size );
		cxSep   = size.cx;

		ReleaseDC( hWnd, hDC );
	}

	CRect	rectClient;
	GetClientRect( hWnd, &rectClient );
	int	y = ( rectClient.Height() - cyDigit ) / 2;
	int	x = xMargin;

	CPoint	pt( LOWORD( lParam ), HIWORD( lParam ) );

	HWND	hWndSpin  = GetWindow( hWnd, GW_CHILD );

	// Time

	if	( hWndSpin ){
		CRect	rectHH, rectMM;
		rectHH.SetRect( x, y, x+(cxDigit*2), y+cyDigit );
		x += rectHH.Width() + cxSep;
		rectMM.SetRect( x, y, x+(cxDigit*2), y+cyDigit );

		if	( rectHH.PtInRect( pt ) )
			m_rectSel = rectHH;
		else if	( rectMM.PtInRect( pt ) )
			m_rectSel = rectMM;
	}

	// Date

	else{
		CRect	rectYY, rectMM, rectDD;

		rectYY.SetRect( x, y, x+(cxDigit*4), y+cyDigit );
		x += rectYY.Width() + cxSep;
		rectMM.SetRect( x, y, x+(cxDigit*2), y+cyDigit );
		x += rectMM.Width() + cxSep;
		rectDD.SetRect( x, y, x+(cxDigit*2), y+cyDigit );

		if	( rectYY.PtInRect( pt ) )
			m_rectSel = rectYY;
		else if	( rectMM.PtInRect( pt ) )
			m_rectSel = rectMM;
		else if	( rectDD.PtInRect( pt ) )
			m_rectSel = rectDD;
		else
			m_rectSel.SetRectEmpty();
	}

	m_hWndSel = hWnd;
	InvalidateRect( hWnd, &m_rectSel, FALSE );
}

LRESULT
CDark::OnEdit( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef )
{
	CDark*	that = (CDark*)dwRef;
	return	that->OnMsgEdit( hWnd, uMsg, wParam, lParam );
}

LRESULT
CDark::OnMsgEdit( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//	TraceMsg( _T("Edit"), hWnd, uMsg );//DBG

	if	( uMsg == WM_CTLCOLOR ){
		HDC	hDC = GetDC( hWnd );
		SetBkColor(   hDC, m_crEdit );
		SetTextColor( hDC, m_crFore );
		ReleaseDC( hWnd, hDC );

		return	(LRESULT)m_brEdit;
	}
	else if	( uMsg == WM_ERASEBKGND ){
		CRect	rect;
		GetClientRect( hWnd, &rect );
		HDC	hDC = (HDC)wParam;
		FillRect( hDC, &rect, m_brEdit );

		return	0;
	}
	else if	( uMsg == WM_KEYDOWN ){
		RedrawWindow( hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW );

		return	0;
	}
	else if	( uMsg == WM_KILLFOCUS ){
		RedrawWindow( hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW );
	}
	else if	( uMsg == WM_NCPAINT ){
		// Do nothing for WM_NCPAINT since it just flickers.
		return	0;
	}
	else if	( uMsg == WM_PAINT ){
		LRESULT	lr = DefSubclassProc( hWnd, uMsg, wParam, lParam );

		// Replace client edge for edit box with spin.

		DWORD	dwExStyle = GetWindowLong( hWnd, GWL_EXSTYLE );
		if	(  dwExStyle & WS_EX_CLIENTEDGE ){
			HWND	hWndNext  = GetWindow( hWnd, GW_HWNDNEXT );
			if	( hWndNext ){
				TCHAR	achClass[_MAX_PATH];
				GetClassName( hWndNext, achClass, _MAX_PATH );
				CString	strClass = achClass;
				if	( strClass == _T("msctls_updown32") ){
					CRect	rect;
					GetClientRect( hWnd, &rect );
					HDC	hDC = GetDC( hWnd );
					SelectObject( hDC, m_hPenEdge );
					SelectObject( hDC, GetStockObject( NULL_BRUSH ) );
					Rectangle( hDC, rect.left, rect.top, rect.right, rect.bottom );
#ifdef	EDITEDGE
					SelectObject( hDC, m_hPenGray );
					rect.left   -= 2;
					rect.top    -= 2;
					rect.bottom += 2;
					Rectangle( hDC, rect.left, rect.top, rect.right, rect.bottom );
					ReleaseDC( hWnd, hDC );
#endif
				}
			}
		}

		// Draw edge for edgeless window.

		else{
#ifdef	EDITEDGE
			CRect	rect;
			GetClientRect( hWnd, &rect );
			HDC	hDC = GetDC( hWnd );
			SelectObject( hDC, m_hPenGray );
			SelectObject( hDC, GetStockObject( NULL_BRUSH ) );
			Rectangle( hDC, rect.left-1, rect.top-1, rect.right+1, rect.bottom-1 );
			ReleaseDC( hWnd, hDC );
#endif
		}
		return	lr;
	}

	return	DefSubclassProc( hWnd, uMsg, wParam, lParam );
}

LRESULT
CDark::OnStatic( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef )
{
	CDark*	that = (CDark*)dwRef;
	return	that->OnMsgStatic( hWnd, uMsg, wParam, lParam );
}

LRESULT
CDark::OnMsgStatic( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//	TraceMsg( _T("Static"), hWnd, uMsg );//DBG

	if	( uMsg == WM_ERASEBKGND ){
		CRect	rect;
		GetClientRect( hWnd, &rect );
		HDC	hDC = (HDC)wParam;
		CDC*	pDC = CDC::FromHandle( hDC );
		pDC->FillSolidRect( &rect, m_crBack );

		return	0;
	}
	else if	( uMsg == WM_PAINT ){
		DWORD	dwStyle   = GetWindowLong( hWnd, GWL_STYLE );
		if	( !( dwStyle & SS_ICON ) ){
			PAINTSTRUCT	ps;
			HDC		hDC = BeginPaint( hWnd, &ps );

			SetTextColor( hDC, m_crFore );
			SetBkColor(   hDC, m_crBack );

			HWND	hWndParent = GetAncestor( hWnd, GA_PARENT );
			HBRUSH	hBrush = (HBRUSH)SendMessage( hWndParent, WM_CTLCOLORSTATIC, (WPARAM)hDC, (LPARAM)hWnd );

			CRect	rect;
			GetClientRect( hWnd, &rect );
			FillRect( hDC, &rect, hBrush );//TMP
			OnPaintStatic( hWnd, hDC, rect );
			EndPaint( hWnd, &ps );

			return	0;
		}
	}
	else if	( uMsg == WM_SETTEXT ){
		RedrawWindow( hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW );
	}
	
	return	DefSubclassProc( hWnd, uMsg, wParam, lParam );
}

void
CDark::OnPaintStatic( HWND hWnd, HDC hDC, CRect rect )
{
	UINT	uDT = 0;
	DWORD	dwStyle   = GetWindowLong( hWnd, GWL_STYLE );
	if	( dwStyle & SS_RIGHT )
		uDT |= DT_RIGHT;
	if	( dwStyle & SS_CENTER )
		uDT |= DT_CENTER;
	if	( !( dwStyle & SS_ENDELLIPSIS ) ||
		  !( dwStyle & SS_PATHELLIPSIS ) ||
		  !( dwStyle & SS_WORDELLIPSIS ) )
		uDT |= DT_WORDBREAK;

	bool	bTab, bAlt;
	GetWndTabAlt( hWnd, bTab, bAlt );

	CWnd*	pWnd = CWnd::FromHandle( hWnd );
	CString	strTitle;
	pWnd->GetWindowText( strTitle );
	if	( !bAlt )
		strTitle.Replace( _T("&"), _T("") );

	HFONT	hFont = (HFONT)SendMessage( hWnd, WM_GETFONT, 0, 0 );
	SelectObject( hDC, hFont );

	SetBkMode( hDC, TRANSPARENT );
	DrawText( hDC, strTitle.GetBuffer(), strTitle.GetLength(), &rect, uDT );
}

LRESULT
CDark::OnSpin( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef )
{
	CDark*	that = (CDark*)dwRef;
	return	that->OnMsgSpin( hWnd, uMsg, wParam, lParam );
}

LRESULT
CDark::OnMsgSpin( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//	TraceMsg( _T("Spin"), hWnd, uMsg );//DBG

	if	( uMsg == WM_ERASEBKGND ){
		CRect	rect;
		GetClientRect( hWnd, &rect );
		HDC	hDC = (HDC)wParam;
		CDC*	pDC = CDC::FromHandle( hDC );
		pDC->FillSolidRect( &rect, m_crBack );

		return	0;
	}
	else if	( uMsg == WM_PAINT ){
		PAINTSTRUCT	ps;
		HDC		hDC = BeginPaint( hWnd, &ps );

		SetTextColor( hDC, m_crFore );
		SetBkColor(   hDC, m_crEdit );

		CRect	rect;
		GetClientRect( hWnd, &rect );
		FillRect( hDC, &rect, m_brEdit );
		OnPaintSpin( hWnd, hDC, rect );
		EndPaint( hWnd, &ps );

		return	0;
	}

	return	DefSubclassProc( hWnd, uMsg, wParam, lParam );
}

void
CDark::OnPaintSpin( HWND hWnd, HDC hDC, CRect rect )
{
	CRect	rectArrow, rectButton = rect;

	rectButton.bottom -= rectButton.Height()/2;
	rectArrow = rectButton;
	rectArrow -= CPoint( 3, 0 );
	GdipArrow( hDC, rectArrow, m_crFore, true );

	rectButton.top = rectButton.bottom;
	rectButton.bottom = rect.bottom;
	rectArrow = rectButton;
	rectArrow -= CPoint( 3, 0 );
	GdipArrow( hDC, rectArrow, m_crFore, false );
#ifdef	EDITEDGE
	SelectObject( hDC, GetStockObject( NULL_BRUSH ) );
	SelectObject( hDC, m_hPenGray );
	Rectangle( hDC, rect.left, rect.top, rect.right, rect.bottom );
#endif
}

LRESULT
CDark::OnStatusBar( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef )
{
	CDark*	that = (CDark*)dwRef;
	return	that->OnMsgStatusBar( hWnd, uMsg, wParam, lParam );
}

LRESULT
CDark::OnMsgStatusBar( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//	TraceMsg( _T("StatusBar"), hWnd, uMsg );//DBG

	if	( uMsg == WM_CTLCOLOREDIT ||
		  uMsg == WM_CTLCOLORLISTBOX )
		return	OnCtlColorEdit( hWnd, wParam );

	else if	( uMsg == WM_ERASEBKGND ){
		CRect	rect;
		GetClientRect( hWnd, &rect );
		HDC	hDC = (HDC)wParam;
		CDC*	pDC = CDC::FromHandle( hDC );
		pDC->FillSolidRect( &rect, m_crBack );

		return	0;
	}
	else if	( uMsg == WM_PAINT ){
		PAINTSTRUCT	ps;
		HDC		hDC = BeginPaint( hWnd, &ps );
		CRect	rect;
		GetClientRect( hWnd, &rect );
		OnPaintStatusBar( hWnd, hDC, rect );
		EndPaint( hWnd, &ps );

		return	0;
	}
	else if	( uMsg == WM_SIZE ){
		RedrawWindow( hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW );
	}

	return	DefSubclassProc( hWnd, uMsg, wParam, lParam );
}

void
CDark::OnPaintStatusBar( HWND hWnd, HDC hDC, CRect rect )
{
	FillRect( hDC, &rect, m_brBack );

	CWnd*	pWnd = CWnd::FromHandle( hWnd );
	CFont*	pFont = pWnd->GetFont();
	if	( pFont )
		SelectObject( hDC, pFont->m_hObject );

	SetTextColor( hDC, m_crFore );
	SetBkColor( hDC, m_crMenu );
	SetBkMode( hDC, OPAQUE );

	SelectObject( hDC, m_hFontMenu );
	CStatusBar*	pBar = (CStatusBar*)pWnd;
	for	( int i = 0; ; i++ ){
		pBar->GetItemRect( i, &rect );
		if	( rect.IsRectEmpty() )
			break;

		UINT	uDT = DT_SINGLELINE;
		CString	str = pBar->GetPaneText( i );
		if	( i == 0 )
			rect.left += 5;
		if	( str.Left( 1 ) == _T("\t") ){
			if	( str.Right( 1 ) == _T("\t") )
				uDT |= DT_CENTER;
			else
				uDT |= DT_RIGHT;
		}
		FillRect( hDC, &rect, m_brMenu );
		DrawText( hDC, str.GetBuffer(), str.GetLength(), &rect, uDT );
	}
}

LRESULT
CDark::OnScrollBars( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef )
{
	CDark*	that = (CDark*)dwRef;
	return	that->OnMsgScrollBars( hWnd, uMsg, wParam, lParam );
}

LRESULT
CDark::OnMsgScrollBars( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//	TraceMsg( _T("ScrollBars"), hWnd, uMsg );//DBG

	LRESULT	lr = DefSubclassProc( hWnd, uMsg, wParam, lParam );

	if	( uMsg == WM_NCPAINT )
		FillGap( hWnd );

	return	lr;
}

LRESULT
CDark::OnCtlColorEdit( HWND hWnd, WPARAM wParam )
{
	HDC	hDC = (HDC)wParam;

	SetTextColor( hDC, m_crFore );
	SetBkColor(   hDC, m_crCombo );

	return	(LRESULT)m_brCombo;
}

void
CDark::FillGap( HWND hWnd )
{//TMP: In case of a window with both vertical and horizontal scroll bars, a square gap appears on the edge. I don't know why.
 //	This function it to fill the gap.
	if	( m_bDarken ){
		CRect	rectGap, rectWindow;
		GetClientRect( hWnd, &rectGap );
		GetWindowRect( hWnd, &rectWindow );
		MapWindowPoints( hWnd, HWND_DESKTOP, (POINT*)&rectGap, 2 );
		OffsetRect( &rectGap, -rectWindow.left, -rectWindow.top );
		rectGap.top = rectGap.bottom;
		rectGap.bottom = rectGap.top +18;
		rectGap.left = rectGap.right;
		rectGap.right = rectGap.left +18;

		HDC	hDC = GetWindowDC( hWnd );
		FillRect( hDC, &rectGap, m_brScrollBar );
		ReleaseDC( hWnd, hDC );
	}
}

void
CDark::GetWndTabAlt( HWND hWnd, bool& bTab, bool& bAlt )
{
	hWnd = GetAncestor( hWnd, GA_PARENT );
	DWORD	dwState = GetWndUIState( hWnd );
	bTab = !( dwState & UISF_HIDEFOCUS );
	bAlt = !( dwState & UISF_HIDEACCEL );
}

DWORD
CDark::GetWndUIState( HWND hWnd )
{
	INT_PTR	n = m_aState.GetCount();
	for	( INT_PTR i = 0; i < n; i++ )
		if	( m_aState[i].hWnd == hWnd )
			return	m_aState[i].dwUIState;

	return	0;
}

void
CDark::SetWndUIState( HWND hWnd, DWORD dwUIState )
{
	INT_PTR	n = m_aState.GetCount();
	for	( INT_PTR i = 0; i < n; i++ )
		if	( m_aState[i].hWnd == hWnd ){
			m_aState[i].dwUIState = dwUIState;
			break;
		}
}

///////////////////////////////////////////////////////////////////////////////////////
// Internal Functions for GDI+

void
CDark::GdipRoundRect( HDC hDC, CRect rect, CPoint ptEdge, COLORREF crInner, COLORREF crEdge )
{
	Gdiplus::Graphics	g( hDC );
	g.SetSmoothingMode( Gdiplus::SmoothingModeAntiAlias );
	g.SetPageUnit( Gdiplus::UnitPixel );

	Gdiplus::GraphicsPath	path;
	path.AddArc( rect.left,           rect.top,             ptEdge.x, ptEdge.y, 180,  90 );
	path.AddArc( rect.right-ptEdge.x, rect.top,             ptEdge.x, ptEdge.y, 270,  90 );
	path.AddArc( rect.right-ptEdge.x, rect.bottom-ptEdge.y, ptEdge.x, ptEdge.y,   0,  90 );
	path.AddArc( rect.left,           rect.bottom-ptEdge.y, ptEdge.x, ptEdge.y,  90,  90 );
	path.CloseFigure();

	if	( crInner != -1 ){
		Gdiplus::Color	cInner;
		cInner.SetFromCOLORREF( crInner );
		Gdiplus::SolidBrush	bInner( cInner );
		g.FillPath( &bInner, &path );
	}

	if	( crEdge != -1 ){
		Gdiplus::Color	cEdge;
		cEdge.SetFromCOLORREF( crEdge );
		Gdiplus::Pen	pen( cEdge, 1.0f );
		g.DrawPath( &pen, &path );
	}
}

void
CDark::GdipCircle( HDC hDC, CRect rect, int nThickness, COLORREF crInner, COLORREF crEdge )
{
	Gdiplus::Graphics	g( hDC );
	g.SetSmoothingMode( Gdiplus::SmoothingModeAntiAlias );
	g.SetPageUnit( Gdiplus::UnitPixel );

	Gdiplus::Color	cInner;
	cInner.SetFromCOLORREF( crInner );
	Gdiplus::SolidBrush	bInner( cInner );
	Gdiplus::Color	cEdge;
	cEdge.SetFromCOLORREF( crEdge );

	Gdiplus::Pen	pen( cEdge, (float)nThickness );
	Gdiplus::Rect	rc( rect.left+nThickness/2, rect.top+nThickness/2, rect.Width()-nThickness, rect.Height()-nThickness );

	g.FillEllipse( &bInner, rc );
	g.DrawEllipse( &pen,    rc );
}

void
CDark::GdipCheckMark( HDC hDC, CRect rect, COLORREF crMark )
{
	Gdiplus::Graphics	g( hDC );
	g.SetSmoothingMode( Gdiplus::SmoothingModeAntiAlias );
	g.SetPageUnit( Gdiplus::UnitPixel );

	Gdiplus::Color	cMark;
	cMark.SetFromCOLORREF( crMark );
	Gdiplus::Pen	pen( cMark, 1.6f );

	int	cx = rect.Width();
	int	cy = rect.Height();

	Gdiplus::Point	pt1( rect.left+(cx* 5/20), rect.top+(cy*10/20) );
	Gdiplus::Point	pt2( rect.left+(cx* 8/20), rect.top+(cy*13/20) );
	Gdiplus::Point	pt3( rect.left+(cx*14/20), rect.top+(cy* 7/20) );

	Gdiplus::GraphicsPath	path;
	path.AddLine( pt1, pt2 );
	path.AddLine( pt2, pt3 );

	g.DrawPath( &pen, &path );
}

void
CDark::GdipArrow( HDC hDC, CRect rect, COLORREF crMark, bool bUp )
{
	rect.left = rect.right - rect.Height();

	Gdiplus::Graphics	g( hDC );
	g.SetSmoothingMode( Gdiplus::SmoothingModeAntiAlias );
	g.SetPageUnit( Gdiplus::UnitPixel );

	Gdiplus::Color	cMark;
	cMark.SetFromCOLORREF( crMark );
	Gdiplus::Pen	pen( cMark, 1.6f );

	int	x = rect.left;
	int	y = rect.top;
	int	cx = rect.Width();
	int	cy = rect.Height();
	int	dx = cx*20/100;
	int	dy = cy*10/100;

	Gdiplus::GraphicsPath	path;

	if	( bUp ){
		Gdiplus::Point	pt1( x+(cx/2)-dx, y+(cy/2)+dy );
		Gdiplus::Point	pt2( x+(cx/2)   , y+(cy/2)-dy );
		Gdiplus::Point	pt3( x+(cx/2)+dx, y+(cy/2)+dy );
		path.AddLine( pt1, pt2 );
		path.AddLine( pt2, pt3 );
	}
	else{
		Gdiplus::Point	pt1( x+(cx/2)-dx, y+(cy/2)-dy );
		Gdiplus::Point	pt2( x+(cx/2)   , y+(cy/2)+dy );
		Gdiplus::Point	pt3( x+(cx/2)+dx, y+(cy/2)-dy );
		path.AddLine( pt1, pt2 );
		path.AddLine( pt2, pt3 );
	}

	g.DrawPath( &pen, &path );
}

#ifdef	_DEBUG

extern	char*	DecodeMsg( UINT uMsg );
#include "DecodeMsg.cpp"

void
CDark::TraceMsg( CString strWindow, HWND hWnd, UINT uMsg )
{
	if	( uMsg == WM_NCHITTEST )
		return;
	if	( uMsg == WM_SETCURSOR )
		return;
	if	( uMsg == WM_MOUSEMOVE )
		return;
	if	( uMsg == WM_NCMOUSEMOVE )
		return;

	CString	str;
	DWORD	dwTime = (DWORD)GetTickCount64();
static	DWORD	dwTimeLast;
	if	( dwTime != dwTimeLast ){
		OutputDebugString( _T("\n") );
		dwTimeLast = dwTime;
	}
	int	nMs = dwTime % 1000;
	dwTime /= 100;
	int	nSec = dwTime % 60;
	dwTime /= 60;
	int	nMin = dwTime % 60;
	dwTime /= 60;
	int	nHour = dwTime % 100;
	CString	strMsg( DecodeMsg( uMsg ) );
	str.Format( _T("%02d:%02d:%02d.%03d %06llx %s %s\n"),
		nHour, nMin, nSec, nMs, (QWORD)hWnd, strWindow.GetBuffer(), strMsg.GetBuffer() );
	OutputDebugString( str );
}

#endif//_DEBUG

///////////////////////////////////////////////////////////////////////////////////////
// Interface Functions for Application

#define	IDR_MAINFRAME	128

#define	NMARGIN		 9
#define	CXBUTTON	45
#define	CYBUTTON	16
#define	CXICON		24

void
CDarkBox::SetUp( WPARAM wParam, LPARAM lParam )
{
	// Make a Dialog Template memory area.

	DWORD	cbTemp = 0;
	cbTemp += FillDialog(  NULL, NULL, wParam, lParam );
	cbTemp += FillIcon(    NULL, NULL, wParam, lParam );
	cbTemp += FillText(    NULL, NULL, wParam, lParam );
	cbTemp += FillButtons( NULL, NULL, wParam, lParam );
	m_pbTemp = new BYTE[cbTemp];
	memset( m_pbTemp, 0, cbTemp );

	// Make the Dialog template.

	BYTE*	pb = m_pbTemp;
	DLGTEMPLATE*	pTemp = (DLGTEMPLATE*)pb;
	pb += FillDialog(  pb, pTemp, wParam, lParam );
	pb += FillIcon(    pb, pTemp, wParam, lParam );
	pb += FillText(    pb, pTemp, wParam, lParam );
	pb += FillButtons( pb, pTemp, wParam, lParam );

	// Set the Dialog template.

	InitModalIndirect( (DLGTEMPLATE*)m_pbTemp, NULL, NULL );
}

CDarkBox::CDarkBox( void )
{
	m_pbTemp = NULL;
	m_uDefButton = 0;
}

CDarkBox::~CDarkBox( void )
{
	delete[]	m_pbTemp;
}

///////////////////////////////////////////////////////////////////////////////////////
// Overridden Functions

BOOL
CDarkBox::OnInitDialog( void )
{
	CDialog::OnInitDialog();

	m_dark.Initialize( GetSafeHwnd() );

	return	TRUE;
}

BOOL
CDarkBox::OnCommand( WPARAM wParam, LPARAM lParam )
{
	EndDialog( LOWORD( wParam ) );

	return	TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////
// Message Handlers

BEGIN_MESSAGE_MAP( CDarkBox, CDialog )
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

void
CDarkBox::OnShowWindow( BOOL bShow, UINT nStatus )
{
	CDialog::OnShowWindow( bShow, nStatus );

	if	( m_uDefButton ){
		SetDefID( m_uDefButton );
		GetDlgItem( m_uDefButton )->SetFocus();
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// Specific Functions

DWORD
CDarkBox::FillDialog( BYTE* pb, DLGTEMPLATE* pTemp, WPARAM wParam, LPARAM lParam )
{
	BYTE*	pbStart = pb;
	bool	bFill = true;
	if	( !pb ){
		bFill = false;
		pbStart = pb = new BYTE[4096];
		pTemp = (DLGTEMPLATE*)pb;
		memset( pTemp, 0, sizeof( *pTemp ) );
	}

	// Select the icon.

	UINT	uIcon = wParam & MB_ICONMASK;
	LPWSTR	pIcon = SelectIcon( uIcon );

	// Select the buttons.

	CString	strButton[4];
	UINT	uIdButton[4] = {};

	UINT	uButton = wParam & MB_TYPEMASK;
	int	nButton = SelectButtons( uButton, uIdButton, strButton );

	pTemp->style = WS_POPUP | WS_CAPTION | DS_SETFONT;
	pTemp->dwExtendedStyle = 0;
	pTemp->cdit = 1 + ( pIcon? 1: 0 ) + nButton;
	pTemp->x  =   0;
	pTemp->y  =   0;
	pTemp->cx = 240;
	pTemp->cy =  64;

	int	cyFont = 10;

	// Get the caption and message text.

	CString	strCaption;
	(void)strCaption.LoadString( IDR_MAINFRAME );

	WORD*	pw = (WORD*)( pTemp+1 );
	*pw++ = 0;			// Menu ID
	*pw++ = 0;			// Window Class
	DWORD	cb = ( strCaption.GetLength()+1 ) * (int)sizeof( WCHAR );
	memcpy( pw, strCaption.GetBuffer(), cb );
	pw += strCaption.GetLength()+1;
	*pw++ = cyFont;			// Font size
	memcpy( pw, L"Arial", 6 * sizeof( WCHAR ) );
	pw += 6;
	pb = (BYTE*)pw;
	pb = Align( pb );

	DWORD	cbFilled = (DWORD)( pb - pbStart );
	if	( !bFill )
		delete[]	pbStart;

	return	cbFilled;
}

DWORD
CDarkBox::FillIcon( BYTE* pb, DLGTEMPLATE* pTemp, WPARAM wParam, LPARAM lParam )
{
	BYTE*	pbStart = pb;
	bool	bFill = true;
	if	( !pb ){
		bFill = false;
		pbStart = pb = new BYTE[4096];
		pTemp = (DLGTEMPLATE*)pb;
		memset( pTemp, 0, sizeof( *pTemp ) );
	}

	// Select the icon.

	UINT	uIcon = wParam & MB_ICONMASK;
	LPWSTR	pIcon = SelectIcon( uIcon );

	if	( pIcon ){
		DLGITEMTEMPLATE*	pItem = (DLGITEMTEMPLATE*)pb;

		pItem->style = WS_CHILD | WS_VISIBLE | SS_ICON;
		pItem->dwExtendedStyle = 0;
		pItem->x = NMARGIN;
		pItem->y = NMARGIN;
		pItem->cx = CXICON;
		pItem->cy = CXICON;
		pItem->id = 0;
		WORD*	pw = (WORD*)( pItem+1 );
		*pw++ = 0xffff;		// HItem: Control specified below
		*pw++ = 0x0082;		// LItem: Static Control
		*pw++ = 0xffff;		// Title: ID specified below
		*pw++ = (WORD)(QWORD)pIcon;
		*pw++ = 0;	// cbDataItem
		pb = (BYTE*)pw;
		pb = Align( pb );
	}

	DWORD	cbFilled = (DWORD)( pb - pbStart );
	if	( !bFill )
		delete[]	pbStart;

	return	cbFilled;
}

DWORD
CDarkBox::FillText( BYTE* pb, DLGTEMPLATE* pTemp, WPARAM wParam, LPARAM lParam )
{
	BYTE*	pbStart = pb;
	bool	bFill = true;
	if	( !pb ){
		bFill = false;
		pbStart = pb = new BYTE[4096];
		pTemp = (DLGTEMPLATE*)pb;
		memset( pTemp, 0, sizeof( *pTemp ) );
	}

	DLGITEMTEMPLATE*	pItem = (DLGITEMTEMPLATE*)pb;

	UINT	uIcon = wParam & MB_ICONMASK;
	LPWSTR	pIcon = SelectIcon( uIcon );
	CString	strText = (WCHAR*)lParam;
	int	yButton  = pTemp->cy - 20;

	pItem->style = WS_CHILD | WS_VISIBLE | SS_LEFT;
	pItem->dwExtendedStyle = 0;
	pItem->x = NMARGIN + ( pIcon? CXICON: 0 );
	pItem->y = NMARGIN;
	pItem->cx = pTemp->cx - (NMARGIN*2) - ( pIcon? CXICON: 0 );
	pItem->cy = yButton   - (NMARGIN*2);
	pItem->id = 1;
	WORD*	pw = (WORD*)( pItem+1 );
	*pw++ = 0xffff;			// HItem: Control specified below
	*pw++ = 0x0082;			// LItem: Static Control
	DWORD	cb = ( strText.GetLength()+1 ) * (int)sizeof( WCHAR );
	memcpy( pw, strText.GetBuffer(), cb );
	pw += strText.GetLength()+1;
	*pw++ = 0;	// cbDataItem
	pb = (BYTE*)pw;
	pb = Align( pb );

	DWORD	cbFilled = (DWORD)( pb - pbStart );
	if	( !bFill )
		delete[]	pbStart;

	return	cbFilled;
}

DWORD
CDarkBox::FillButtons( BYTE* pb, DLGTEMPLATE* pTemp, WPARAM wParam, LPARAM lParam )
{
	BYTE*	pbStart = pb;
	bool	bFill = true;
	if	( !pb ){
		bFill = false;
		pbStart = pb = new BYTE[4096];
		pTemp = (DLGTEMPLATE*)pb;
		memset( pTemp, 0, sizeof( *pTemp ) );
	}

	// Select the buttons.

	CString	strButton[4];
	UINT	uIdButton[4] = {};

	UINT	uButton = wParam & MB_TYPEMASK;
	int	nButton = SelectButtons( uButton, uIdButton, strButton );
	int	yButton  = pTemp->cy - 20;
	UINT	uDefault = wParam & MB_DEFMASK;

	int	x = pTemp->cx - ( ( CXBUTTON + NMARGIN ) * nButton );
	for	( int i = 0; i < nButton; i++ ){
		DLGITEMTEMPLATE*	pItem = (DLGITEMTEMPLATE*)pb;

		pItem->style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON;
		if	( ( ( uDefault == MB_DEFBUTTON1 ) && ( i == 0 ) ) ||
			  ( ( uDefault == MB_DEFBUTTON2 ) && ( i == 1 ) ) ||
			  ( ( uDefault == MB_DEFBUTTON3 ) && ( i == 2 ) )    )
			m_uDefButton = uIdButton[i];
		pItem->dwExtendedStyle = 0;
		pItem->x  = x;
		pItem->y  =  yButton;
		pItem->cx = CXBUTTON;
		pItem->cy = CYBUTTON;
		pItem->id = uIdButton[i];
		WORD*	pw = (WORD*)( pItem+1 );
		*pw++ = 0xffff;		// HItem: Control specified below
		*pw++ = 0x0080;		// LItem: Button Control
		DWORD	cb = ( strButton[i].GetLength()+1 ) * (int)sizeof( WCHAR );
		memcpy( pw, strButton[i].GetBuffer(), cb );
		pw += strButton[i].GetLength()+1;
		*pw++ = 0;		// cbDataItem
		pb = (BYTE*)pw;
		pb = Align( pb );
		x += CXBUTTON + NMARGIN;
	}

	DWORD	cbFilled = (DWORD)( pb - pbStart );
	if	( !bFill )
		delete[]	pbStart;

	return	cbFilled;
}

int
CDarkBox::SelectButtons( UINT uType, UINT* puIdButton, CString* pstrButton )
{
	int	nButton = 0;

	if	( uType == MB_OK ){
		nButton = 1;
		puIdButton[0] = IDOK;
	}
	else if	( uType == MB_OKCANCEL ){
		nButton = 2;
		puIdButton[0] = IDOK;
		puIdButton[1] = IDCANCEL;
	}
	else if	( uType == MB_ABORTRETRYIGNORE ){
		nButton = 3;
		puIdButton[0] = IDABORT;
		puIdButton[1] = IDRETRY;
		puIdButton[2] = IDIGNORE;
	}
	else if	( uType == MB_YESNOCANCEL ){
		nButton = 3;
		puIdButton[0] = IDYES;
		puIdButton[1] = IDNO;
		puIdButton[2] = IDCANCEL;
	}
	else if	( uType == MB_YESNO ){
		nButton = 2;
		puIdButton[0] = IDYES;
		puIdButton[1] = IDNO;
	}
	else if	( uType == MB_RETRYCANCEL ){
		nButton = 2;
		puIdButton[0] = IDRETRY;
		puIdButton[1] = IDCANCEL;
	}
	else if	( uType == MB_CANCELTRYCONTINUE ){
		nButton = 3;
		puIdButton[0] = IDCANCEL;
		puIdButton[1] = IDTRYAGAIN;
		puIdButton[2] = IDCONTINUE;
	}
	else{
		nButton = 1;
		puIdButton[0] = IDOK;
	}

	static	WCHAR*	apchButtons[] = {
			_T(""),			// 0
			_T("OK"),		// IDOK
			_T("Cancel"),		// IDCANCEL
			_T("&Abort"),		// IDABORT
			_T("&Retry"),		// IDRETRY
			_T("&Ignore"),		// IDIGNORE
			_T("&Yes"),		// IDYES
			_T("&No"),		// IDNO
			_T("&Close"),		// IDCLOSE
			_T("&Help"),		// IDHELP
			_T("&Tray Again"),	// IDTRYAGAIN
			_T("&Continue")		// IDCONTINUE
	};

	for	( int i = 0; i < nButton; i++ )
		pstrButton[i] = apchButtons[puIdButton[i]];

	return	nButton;
}

LPWSTR
CDarkBox::SelectIcon( UINT uIcon )
{
	LPWSTR	pIcon = NULL;

	if	( uIcon == MB_ICONHAND )
		pIcon = IDI_HAND;
	else if	( uIcon == MB_ICONQUESTION )
		pIcon = IDI_QUESTION;
	else if	( uIcon == MB_ICONEXCLAMATION )
		pIcon = IDI_EXCLAMATION;
	else if	( uIcon == MB_ICONASTERISK )
		pIcon = IDI_ASTERISK;

	return	pIcon;
}

BYTE*
CDarkBox::Align( BYTE* pb )
{
	if	( (QWORD)pb % sizeof( DWORD ) )
		pb += sizeof( DWORD ) - ( (QWORD)pb % sizeof( DWORD ) );
	return	pb;
}