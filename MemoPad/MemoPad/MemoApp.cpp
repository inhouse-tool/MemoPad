// MemoPad.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "MemoApp.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMemoApp	theApp;

///////////////////////////////////////////////////////////////////////////////////////
// Overridden Functions

BOOL
CMemoApp::InitInstance( void )
{
	INITCOMMONCONTROLSEX	InitCtrls;
	InitCtrls.dwSize = sizeof( InitCtrls );
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx( &InitCtrls );

	CWinApp::InitInstance();

	EnableTaskbarInteraction( FALSE );

	SetRegistryKey( _T("In-house Tool") );

	CFrameWnd*	pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

	pFrame->LoadFrame( IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL );

	pFrame->ShowWindow( SW_SHOW );
	pFrame->UpdateWindow();

	return	TRUE;
}

int
CMemoApp::DoMessageBox( LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt )
{
#ifdef	MessageDlg	// a switch to compare with the original AfxMessageBox.
	return	(int)MessageDlg( lpszPrompt, nType );
#else
	return	CWinApp::DoMessageBox( lpszPrompt, nType, nIDPrompt );
#endif
}
