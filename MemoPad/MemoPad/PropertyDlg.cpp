// PropertyDlg.cpp : implementation file
//

#include "pch.h"
#include "PropertyDlg.h"
#include "MainFrm.h"
#include "resource.h"

IMPLEMENT_DYNAMIC( CPropertyDlg, CDialog )

CPropertyDlg::CPropertyDlg( CWnd* pParent )
	: CDialog( IDD_PROPERTIES, pParent )
{
	m_bModified = false;
	m_bInPath   = false;
	m_hcArrow = AfxGetApp()->LoadStandardCursor( IDC_ARROW );
	m_hcHand  = AfxGetApp()->LoadStandardCursor( IDC_HAND );
}

void
CPropertyDlg::SetFile( CString strFile )
{
	m_strFile = strFile;
}

void
CPropertyDlg::SetText( CString strLines )
{
	m_strProperties = strLines;
}

void
CPropertyDlg::SetModified( bool bModified )
{
	m_bModified = bModified;
}

BOOL
CPropertyDlg::OnInitDialog( void )
{
	CDialog::OnInitDialog();

	int	x = m_strFile.ReverseFind( '\\' );
	CString	strFile = m_strFile.Mid( x+1 );
	CString	strPath = m_strFile.Left( x+1 );
	GetDlgItem( IDC_STATIC_FILE )->SetWindowText( strFile );
	GetDlgItem( IDC_STATIC_PATH )->SetWindowText( strPath );

	LOGFONT	lf = {};
	CFont*	pFont = GetFont();
	pFont->GetLogFont( &lf );
	lf.lfUnderline = TRUE;
	m_fontPath.CreateFontIndirect( &lf );
	GetDlgItem( IDC_STATIC_PATH )->SetFont( &m_fontPath, FALSE );

	if	( !m_strFile.IsEmpty() ){
		CFileStatus	fs;
		CFile::GetStatus( m_strFile, fs );
		CString	str;
		str = fs.m_ctime.Format( _T("%Y/%m/%d %H:%M:%S") );
		GetDlgItem( IDC_STATIC_CREATED  )->SetWindowText( str );
		str = fs.m_mtime.Format( _T("%Y/%m/%d %H:%M:%S") );
		GetDlgItem( IDC_STATIC_MODIFIED )->SetWindowText( str );
	}

	GetDlgItem( IDC_STATIC_PROPERTY )->SetWindowText( m_strProperties );

	SupportDarkMode( GetSafeHwnd() );

	return	TRUE;
}

void
CPropertyDlg::OnOK( void )
{
	PostMessage( WM_SYSCOMMAND, SC_CLOSE, 0 );
}

BEGIN_MESSAGE_MAP( CPropertyDlg, CDialog )
	ON_WM_SETCURSOR()
	ON_STN_CLICKED( IDC_STATIC_PATH, OnClickPath )
	ON_MESSAGE( WM_CTLCOLORSTATIC, OnCtlColorStatic )
END_MESSAGE_MAP()

BOOL
CPropertyDlg::OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message )
{
	if	( pWnd->m_hWnd == GetDlgItem( IDC_STATIC_PATH )->m_hWnd ){
		if	( !m_bInPath ){
			SetCursor( m_hcHand );
			m_bInPath = true;
		}
	}
	else{
		if	( m_bInPath ){
			SetCursor( m_hcArrow );
			m_bInPath = false;
		}
	}

	return	TRUE;
}

void
CPropertyDlg::OnClickPath( void )
{
	CString	strPath;

	GetDlgItem( IDC_STATIC_PATH )->GetWindowText( strPath );
	if	( !strPath.IsEmpty() )
		ShellExecute( NULL, _T("open"), strPath, NULL, NULL, SW_SHOWNORMAL );
}

LRESULT
CPropertyDlg::OnCtlColorStatic( WPARAM wParam, LPARAM lParam )
{
	HDC	hDC  = (HDC)wParam;
	HWND	hWnd = (HWND)lParam;
	UINT	uID  = ::GetDlgCtrlID( hWnd );

	CDC*	pDC  = CDC ::FromHandle( hDC );
	CWnd*	pWnd = CWnd::FromHandle( hWnd );
	HBRUSH	hbr  = CDialog::OnCtlColor( pDC, pWnd, CTLCOLOR_STATIC );

	if	( uID == IDC_STATIC_MODIFY && m_bModified )
		SetTextColor( hDC, RGB( 255,   0,   0 ) );
	else if	( uID == IDC_STATIC_PATH )
		SetTextColor( hDC, (COLORREF)GetSysColor( COLOR_HOTLIGHT ) );

	return	(LRESULT)hbr;
}