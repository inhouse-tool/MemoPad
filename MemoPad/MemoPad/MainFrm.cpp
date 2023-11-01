// MainFrm.cpp : implementation of the CMainFrame class
//

#include "pch.h"
#include "MainFrm.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC( CMainFrame, CFrameWnd )

///////////////////////////////////////////////////////////////////////////////////////
// Constructor

CMainFrame::CMainFrame( void )
{
	m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
	m_hAccel = NULL;

	CWinApp*	pApp = AfxGetApp();
	m_bWrap = pApp->GetProfileInt( _T("Settings"), _T("WordWrap"),  1 );
	m_bMenu = pApp->GetProfileInt( _T("Settings"), _T("MenuBar"),   1 );
	m_bSBar = pApp->GetProfileInt( _T("Settings"), _T("StatusBar"), 1 );
}

///////////////////////////////////////////////////////////////////////////////////////
// Overridden Functions

BOOL
CMainFrame::PreCreateWindow( CREATESTRUCT& cs )
{
	// Register a unique class for this program.

	TCHAR*	pszClass = _T("MemoPad");
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc   = AfxWndProc;
	wc.hInstance     = AfxGetInstanceHandle();
	wc.lpszClassName = pszClass;
	AfxRegisterClass( &wc );

	// Set window size and window class.

	int	cx = GetSystemMetrics( SM_CXSCREEN );
	int	cy = GetSystemMetrics( SM_CYSCREEN );

	cx /= 2;
	cy *= 9;
	cy /= 10;

	cs.cx = cx;
	cs.cy = cy;
	cs.x = -8;
	cs.y = 1;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = pszClass;

	// Do the PreCreateWindow as usual.

	CFrameWnd::PreCreateWindow( cs );

	return	TRUE;
}

BOOL
CMainFrame::PreTranslateMessage( MSG* pMsg )
{
	// Closing messages are caught by the View to preceed WM_CLOSE finish Dark mode.

	if	(   pMsg->hwnd == m_hWnd &&
		  ( pMsg->message == WM_NCLBUTTONDOWN &&
		    pMsg->wParam  == HTCLOSE ) ||
		  ( pMsg->message == WM_SYSCOMMAND &&
		    pMsg->wParam  == SC_CLOSE ) ){
		m_wndView.PostMessage( WM_COMMAND, ID_FILE_CLOSE, NULL );
		return	TRUE;
	}

	// Accelerator messagess are handled by the accelerator.

	else if	( TranslateAccelerator( GetSafeHwnd(), m_hAccel, pMsg ) )
		return	TRUE;

	// Other messagess are handled as usual.

	else
		return	CFrameWnd::PreTranslateMessage( pMsg );
}

BOOL
CMainFrame::OnCmdMsg( UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo )
{
	// View's commands are handled by the View.

	if	( m_wndView.OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) )
		return	TRUE;

	// Other commands are handled as usual.

	else
		return	CFrameWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}

///////////////////////////////////////////////////////////////////////////////////////
// Message Handlers

BEGIN_MESSAGE_MAP( CMainFrame, CFrameWnd )
	ON_WM_CREATE()
	ON_WM_INITMENUPOPUP()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SYSCOMMAND()
	ON_COMMAND( ID_WINDOW_TILE_HORZ, OnViewWordWrap )
	ON_COMMAND( ID_WINDOW_TILE_VERT, OnViewMenuBar )
	ON_COMMAND( ID_VIEW_STATUS_BAR, OnViewStatusBar )
	ON_UPDATE_COMMAND_UI_RANGE( ID_FILE_NEW, ID_FORMAT_FONT, OnUpdateCommand )
	ON_MESSAGE( WM_DARK_SUPPORT, OnSupportDarkMode )
	ON_MESSAGE( WM_MESSAGEDLG,   OnMessageDlg )
	ON_MESSAGE( WM_INDICATOR,    OnIndicator )
END_MESSAGE_MAP()

int
CMainFrame::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	CFrameWnd::OnCreate( lpCreateStruct );

	SetIcon( m_hIcon, TRUE  );
	SetIcon( m_hIcon, FALSE );
	m_hAccel = ::LoadAccelerators( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDR_MAINFRAME ) );

	CString	strTitle;
	if	( strTitle.LoadString( IDR_MAINFRAME ) ){
		strTitle.Insert( 0, _T(" - ") );
		strTitle.Insert( 0, _T("Untitled") );
		AfxGetMainWnd()->SetWindowText( strTitle );
	}

	CreateClient();

	CMenu* pSysMenu = GetSystemMenu( FALSE );
	if	( pSysMenu ){
		pSysMenu->InsertMenu( 5, MF_BYPOSITION, MF_SEPARATOR );

		CMenu*	pSubMenu;

		pSubMenu = GetMenu()->GetSubMenu( 0 );
		pSysMenu->InsertMenu( 6,  MF_BYPOSITION | MF_POPUP, (UINT_PTR)pSubMenu->m_hMenu, L"&File" );

		pSubMenu = GetMenu()->GetSubMenu( 1 );
		pSysMenu->InsertMenu( 7,  MF_BYPOSITION | MF_POPUP, (UINT_PTR)pSubMenu->m_hMenu, L"&Edit" );

		pSubMenu = GetMenu()->GetSubMenu( 2 );
		pSysMenu->InsertMenu( 8,  MF_BYPOSITION | MF_POPUP, (UINT_PTR)pSubMenu->m_hMenu, L"&View" );
	}

	return	0;
}

void
CMainFrame::OnInitMenuPopup( CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu )
{
	CFrameWnd::OnInitMenuPopup( pPopupMenu, nIndex, bSysMenu );

	if	( bSysMenu ){

		// Enable / disable 'File' menu items.

		if	( nIndex == 6 ){
			DWORD	dwStatus = m_wndView.GetStatus();

			pPopupMenu->EnableMenuItem( ID_FILE_SAVE,       MF_BYCOMMAND | ( ( dwStatus & STAT_EMPTY    )? MF_DISABLED: MF_ENABLED ) );
			pPopupMenu->EnableMenuItem( ID_FILE_SAVE_AS,    MF_BYCOMMAND | ( ( dwStatus & STAT_EMPTY    )? MF_DISABLED: MF_ENABLED ) );
			pPopupMenu->EnableMenuItem( ID_FILE_UPDATE,     MF_BYCOMMAND | ( ( dwStatus & STAT_NOFILE   )? MF_DISABLED: MF_ENABLED ) );
		}

		// Enable / disable 'Edit' menu items.

		if	( nIndex == 7 ){
			DWORD	dwStatus = m_wndView.GetStatus();

			pPopupMenu->EnableMenuItem( ID_EDIT_UNDO,       MF_BYCOMMAND | ( ( dwStatus & STAT_UNDOABLE )? MF_ENABLED: MF_DISABLED ) );
			pPopupMenu->EnableMenuItem( ID_EDIT_CUT,        MF_BYCOMMAND | ( ( dwStatus & STAT_SELECTED )? MF_ENABLED: MF_DISABLED ) );
			pPopupMenu->EnableMenuItem( ID_EDIT_COPY,       MF_BYCOMMAND | ( ( dwStatus & STAT_SELECTED )? MF_ENABLED: MF_DISABLED ) );
			pPopupMenu->EnableMenuItem( ID_EDIT_PASTE,      MF_BYCOMMAND | ( ( dwStatus & STAT_PASTEABLE)? MF_ENABLED: MF_DISABLED ) );
			pPopupMenu->EnableMenuItem( ID_EDIT_CLEAR,      MF_BYCOMMAND | ( ( dwStatus & STAT_SELECTED )? MF_ENABLED: MF_DISABLED ) );
			pPopupMenu->EnableMenuItem( ID_EDIT_FIND,       MF_BYCOMMAND | ( ( dwStatus & STAT_EMPTY    )? MF_DISABLED: MF_ENABLED ) );
			pPopupMenu->EnableMenuItem( ID_NEXT_PANE,       MF_BYCOMMAND | ( ( dwStatus & STAT_FOUND    )? MF_ENABLED: MF_DISABLED ) );
			pPopupMenu->EnableMenuItem( ID_PREV_PANE,       MF_BYCOMMAND | ( ( dwStatus & STAT_FOUND    )? MF_ENABLED: MF_DISABLED ) );
			pPopupMenu->EnableMenuItem( ID_EDIT_REPLACE,    MF_BYCOMMAND | ( ( dwStatus & STAT_EMPTY    )? MF_DISABLED: MF_ENABLED ) );
			pPopupMenu->EnableMenuItem( ID_EDIT_SELECT_ALL, MF_BYCOMMAND | ( ( dwStatus & STAT_EMPTY    )? MF_DISABLED: MF_ENABLED ) );
		}

		// Enable / disable 'View' menu items.

		if	( nIndex == 8 ){
			DWORD	dwStatus = m_wndView.GetStatus();

			pPopupMenu->EnableMenuItem( ID_WINDOW_CASCADE,  MF_BYCOMMAND | ( ( dwStatus & STAT_EMPTY    )? MF_DISABLED: MF_ENABLED ) );
			pPopupMenu->EnableMenuItem( ID_WINDOW_ARRANGE,  MF_BYCOMMAND | ( ( dwStatus & STAT_EMPTY    )? MF_DISABLED: MF_ENABLED ) );
			pPopupMenu->EnableMenuItem( ID_WINDOW_NEW,      MF_BYCOMMAND | ( ( dwStatus & STAT_EMPTY    )? MF_DISABLED: MF_ENABLED ) );
			pPopupMenu->CheckMenuItem(  ID_WINDOW_TILE_HORZ,MF_BYCOMMAND | ( ( m_bWrap                  )? MF_CHECKED:  MF_UNCHECKED ) );
			pPopupMenu->CheckMenuItem(  ID_WINDOW_TILE_VERT,MF_BYCOMMAND | ( ( m_bMenu                  )? MF_CHECKED:  MF_UNCHECKED ) );
			pPopupMenu->CheckMenuItem(  ID_VIEW_STATUS_BAR, MF_BYCOMMAND | ( ( m_wndStatusBar.IsWindowVisible() )? MF_CHECKED:  MF_UNCHECKED ) );
		}
	}
}

void
CMainFrame::OnSetFocus( CWnd* pOldWnd )
{
	if	( m_wndView.m_hWnd )
		m_wndView.SetFocus();
}

void
CMainFrame::OnSettingChange( UINT uFlags, LPCTSTR lpszSection )
{
	CFrameWnd::OnSettingChange( uFlags, lpszSection );

	CString	strSection = lpszSection;
	if	( strSection == _T("ImmersiveColorSet") )
		PostMessage( WM_SYSCOLORCHANGE, 0, 0 );
}

void
CMainFrame::OnSysColorChange( void )
{
	CFrameWnd::OnSysColorChange();

	CreateClient();
}

void
CMainFrame::OnSysCommand( UINT nID, LPARAM lParam )
{
	if	( nID == ID_VIEW_STATUS_BAR ||
		  nID == ID_WINDOW_TILE_VERT || 
		  nID == ID_WINDOW_TILE_HORZ   )
		PostMessage( WM_COMMAND, nID, 0 );

	else if	( nID >= ID_FILE_NEW &&
		  nID <= ID_RECORD_PREV )
		m_wndView.PostMessage( WM_COMMAND, nID, 0 );

	else
		CFrameWnd::OnSysCommand( nID, lParam );
}

void
CMainFrame::OnViewWordWrap( void )
{
	m_bWrap = !m_bWrap;
	AfxGetApp()->WriteProfileInt( _T("Settings"), _T("WordWrap"), m_bWrap? 1: 0 );

	CreateClient();
}

void
CMainFrame::OnViewMenuBar( void )
{
	m_bMenu = !m_bMenu;
	AfxGetApp()->WriteProfileInt( _T("Settings"), _T("MenuBar"), m_bMenu? 1: 0 );

	SetMenuBarState( m_bMenu? AFX_MBS_VISIBLE: AFX_MBS_HIDDEN );
}

void
CMainFrame::OnViewStatusBar( void )
{
	m_bSBar = !m_bSBar;
	AfxGetApp()->WriteProfileInt( _T("Settings"), _T("StatusBar"), m_bSBar? 1: 0 );

	ShowControlBar( &m_wndStatusBar, m_bSBar, FALSE );
}

void
CMainFrame::OnUpdateCommand( CCmdUI* pCmdUI )
{
	DWORD	dwStatus = m_wndView.GetStatus();

	if	( pCmdUI->m_nID == ID_FILE_SAVE )
		pCmdUI->Enable( ( dwStatus & STAT_EMPTY  )? FALSE: TRUE );
	else if	( pCmdUI->m_nID == ID_FILE_SAVE_AS )
		pCmdUI->Enable( ( dwStatus & STAT_EMPTY  )? FALSE: TRUE );
	else if	( pCmdUI->m_nID == ID_FILE_UPDATE )
		pCmdUI->Enable( ( dwStatus & STAT_NOFILE )? FALSE: TRUE );

	else if	( pCmdUI->m_nID == ID_EDIT_UNDO )
		pCmdUI->Enable( ( dwStatus & STAT_UNDOABLE )? TRUE: FALSE );
	else if	( pCmdUI->m_nID == ID_EDIT_CUT )
		pCmdUI->Enable( ( dwStatus & STAT_SELECTED )? TRUE: FALSE );
	else if	( pCmdUI->m_nID == ID_EDIT_COPY )
		pCmdUI->Enable( ( dwStatus & STAT_SELECTED )? TRUE: FALSE );
	else if	( pCmdUI->m_nID == ID_EDIT_PASTE )
		pCmdUI->Enable( ( dwStatus & STAT_PASTEABLE)? TRUE: FALSE );
	else if	( pCmdUI->m_nID == ID_EDIT_CLEAR )
		pCmdUI->Enable( ( dwStatus & STAT_SELECTED )? TRUE: FALSE );
	else if	( pCmdUI->m_nID == ID_EDIT_FIND )
		pCmdUI->Enable( ( dwStatus & STAT_EMPTY    )? FALSE: TRUE );
	else if	( pCmdUI->m_nID == ID_NEXT_PANE )
		pCmdUI->Enable( ( dwStatus & STAT_FOUND    )? TRUE: FALSE );
	else if	( pCmdUI->m_nID == ID_PREV_PANE )
		pCmdUI->Enable( ( dwStatus & STAT_FOUND    )? TRUE: FALSE );
	else if	( pCmdUI->m_nID == ID_EDIT_REPLACE )
		pCmdUI->Enable( ( dwStatus & STAT_EMPTY    )? FALSE: TRUE );
	else if	( pCmdUI->m_nID == ID_EDIT_SELECT_ALL )
		pCmdUI->Enable( ( dwStatus & STAT_EMPTY    )? FALSE: TRUE );

	else if	( pCmdUI->m_nID == ID_WINDOW_CASCADE )
		pCmdUI->Enable( ( dwStatus & STAT_EMPTY    )? FALSE: TRUE );
	else if	( pCmdUI->m_nID == ID_WINDOW_ARRANGE )
		pCmdUI->Enable( ( dwStatus & STAT_EMPTY    )? FALSE: TRUE );
	else if	( pCmdUI->m_nID == ID_WINDOW_NEW )
		pCmdUI->Enable( ( dwStatus & STAT_EMPTY    )? FALSE: TRUE );
	else if	( pCmdUI->m_nID == ID_WINDOW_TILE_HORZ )
		pCmdUI->SetCheck( ( m_bWrap                )? 1:     0 );
	else if	( pCmdUI->m_nID == ID_WINDOW_TILE_VERT )
		pCmdUI->SetCheck( ( m_bMenu                )? 1:     0 );
}

LRESULT
CMainFrame::OnSupportDarkMode( WPARAM wParam, LPARAM lParam )
{
	m_dark.Initialize( (HWND)lParam );
	return	0;
}

LRESULT
CMainFrame::OnMessageDlg( WPARAM wParam, LPARAM lParam )
{
#if	1	// a switch to compare with the original AfxMessageBox.
	return	m_dark.PopUp( wParam, lParam );
#else
	CString	strCaption;
	(void)strCaption.LoadString( IDR_MAINFRAME );
	return	CWnd::MessageBox( (LPCTSTR)lParam, strCaption, (UINT)wParam );
#endif
}

LRESULT
CMainFrame::OnIndicator( WPARAM wParam, LPARAM lParam )
{
	int	iPane = (int)wParam;
	LPCTSTR	lpszText = (LPCTSTR)lParam;

	if	( IsWindow( m_wndStatusBar.m_hWnd ) )
		m_wndStatusBar.SetPaneText( iPane, lpszText, TRUE );

	if	( iPane == 0 )
		m_strIdle = lpszText;

	return	0;
}

void
CMainFrame::CreateClient( void )
{
	static UINT indicators[] =
	{
		ID_SEPARATOR,
		ID_INDICATOR_CODE,
		ID_INDICATOR_TYPE,
		ID_INDICATOR_ZOOM
	};

	int	nPane = sizeof( indicators ) / sizeof( *indicators );
	CStringArray	saPane;
	saPane.SetSize( nPane );

	if	( IsWindow( m_wndStatusBar.m_hWnd ) ){
		for	( int i = 0; i < nPane; i++ )
			m_wndStatusBar.GetPaneText( i, saPane[i] );

		m_wndStatusBar.SendMessage( WM_CLOSE, 0, 0 );
	}
	{
		UINT	nID, nStyle;
		int	cx;

		m_wndStatusBar.Create( this );
		m_wndStatusBar.SetIndicators( indicators, sizeof( indicators ) / sizeof( *indicators ) );
		m_wndStatusBar.GetPaneInfo( 0, nID, nStyle, cx );
	//	nStyle &= ~SBPS_STRETCH;
		m_wndStatusBar.SetPaneInfo( 0, nID, nStyle, 250 );	// for the longest string

		for	( int i = 0; i < nPane; i++ )
			m_wndStatusBar.SetPaneText( i, saPane[i] );

		PostMessage( WM_COMMAND, ID_VIEW_STATUS_BAR, 0 );
		PostMessage( WM_COMMAND, ID_VIEW_STATUS_BAR, 0 );
	}

	CString	strLines;

	if	( IsWindow( m_wndView.m_hWnd ) ){
		m_wndView.GetWindowText( strLines );
		m_wndView.SendMessage( WM_CLOSE, 0, 0 );
	}
	{
		CRect	rect;
		GetClientRect( &rect );
		DWORD	dwStyle = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_NOHIDESEL;
		if	( !m_bWrap )
			dwStyle |= WS_HSCROLL;
		m_wndView.Create( dwStyle, rect, this, AFX_IDW_PANE_FIRST );
		if	( !strLines.IsEmpty() )
			m_wndView.SetWindowText( strLines );
	}

	m_dark.Initialize( GetSafeHwnd() );
}
