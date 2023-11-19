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
	INITCOMMONCONTROLSEX	InitCtrls = {};
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

BEGIN_MESSAGE_MAP( CMemoApp, CWinApp )
	ON_COMMAND( ID_HELP_INDEX, OnInfoOnWeb )
	ON_COMMAND( ID_HELP_USING, OnLicense )
	ON_COMMAND( ID_APP_ABOUT,  OnAppAbout )
END_MESSAGE_MAP()

void
CMemoApp::OnInfoOnWeb( void )
{
	CString	strPath = _T("https://github.com/inhouse-tool/MemoPad");
	ShellExecute( NULL, _T("open"), strPath, NULL, NULL, SW_SHOWNORMAL );
}

void
CMemoApp::OnLicense( void )
{
	TCHAR*	pchText =
		_T("MIT License\n")
		_T("\n")
		_T("Copyright (c) 2023 In-house Tool\n")
		_T("\n")
		_T("Permission is hereby granted, free of charge, to any person obtaining a copy\n")
		_T("of this software and associated documentation files (the \"Software\"), to deal\n")
		_T("in the Software without restriction, including without limitation the rights\n")
		_T("to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n")
		_T("copies of the Software, and to permit persons to whom the Software is\n")
		_T("furnished to do so, subject to the following conditions:\n")
		_T("\n")
		_T("The above copyright notice and this permission notice shall be included in all\n")
		_T("copies or substantial portions of the Software.\n")
		_T("\n")
		_T("THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n")
		_T("IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n")
		_T("FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n")
		_T("AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n")
		_T("LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n")
		_T("OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n")
		_T("SOFTWARE.\n")
		;

	DoMessageBox( pchText, 0, MB_OK );
}

#pragma comment( lib, "version.lib" )

void
CMemoApp::OnAppAbout( void )
{
	CString	strCaption = AfxGetApp()->m_pszAppName;
	strCaption.Insert( 0, _T("About ") );
	CString	strText;

	{
		unsigned	uLen;
		TCHAR		achPath[_MAX_PATH];
		GetModuleFileName( NULL, achPath, _countof( achPath ) );

		DWORD	dwHandle;
		DWORD	dwLen = GetFileVersionInfoSize( achPath, &dwHandle );
		char*	pchVerInfo = new char[dwLen];

		GetFileVersionInfo( achPath, 0, dwLen, (LPVOID)pchVerInfo );
		VS_FIXEDFILEINFO*	info;
		VerQueryValue( pchVerInfo, _T("\\"), (LPVOID*)&info, &uLen );

		CString	str, strSubBlock;
		LPVOID	pValue;

		VerQueryValue( pchVerInfo, _T("\\VarFileInfo\\Translation"), &pValue, &uLen );
		strSubBlock.Format( _T("\\StringFileInfo\\%04x%04x\\"), *(int*)pValue & 0xffff, *(int*)pValue>>16 );

		str = strSubBlock + _T("ProductName");
		VerQueryValue( pchVerInfo, str, &pValue, &uLen );
		strText = (LPCTSTR)pValue;
		strText += _T("\n");

		str = strSubBlock + _T("FileDescription");
		VerQueryValue( pchVerInfo, str, &pValue, &uLen );
		strText += (LPCTSTR)pValue;

		str.Format( _T("\n\nProgram Version %d.%d.%d"),
				( info->dwProductVersionMS >> 16 ),
				( info->dwProductVersionMS &  0xffff ),
				( info->dwProductVersionLS >> 16 ) );
		strText += str;
		strText += _T("\n");

		str = strSubBlock + _T("LegalCopyright");
		VerQueryValue( pchVerInfo, str, &pValue, &uLen );
		strText += (LPCTSTR)(char*)pValue;
		strText.Replace( _T( "(C)" ), _T("\x00a9") );

		CTime	tNow = CTime::GetCurrentTime();
		CString	strYear = tNow.Format( _T("-%Y ") );
		strText.Replace( _T("-XXXX"), strYear );

		delete[]	pchVerInfo;
	}
#ifdef	MessageDlg
	DoMessageBox( strText.GetBuffer(), MB_ICONMASK, MB_OK );
#else
	MSGBOXPARAMS	mbp;

	mbp.cbSize = sizeof( MSGBOXPARAMSA );
	mbp.hwndOwner    = AfxGetMainWnd()->m_hWnd;
	mbp.hInstance    = AfxGetApp()->m_hInstance;
	mbp.lpszText     = strText.GetBuffer();
	mbp.lpszCaption  = strCaption.GetBuffer();
	mbp.dwStyle      = MB_OK | MB_USERICON;
	mbp.lpszIcon     = MAKEINTRESOURCE( IDR_MAINFRAME );
	mbp.dwContextHelpId    = 0;
	mbp.lpfnMsgBoxCallback = NULL;
	mbp.dwLanguageId = MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US );

	MessageBoxIndirect( &mbp );
#endif
}
