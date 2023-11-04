// PropertyDlg.cpp : implementation file
//

#include "pch.h"
#include "PropertyDlg.h"
#include "MainFrm.h"
#include "resource.h"

IMPLEMENT_DYNAMIC( CPropertyDlg, CDialog )

///////////////////////////////////////////////////////////////////////////////////////
// Constructor

CPropertyDlg::CPropertyDlg( CWnd* pParent )
	: CDialog( IDD_PROPERTIES, pParent )
{
	m_bModified = false;
	m_bInPath   = false;
	m_hcArrow = AfxGetApp()->LoadStandardCursor( IDC_ARROW );
	m_hcHand  = AfxGetApp()->LoadStandardCursor( IDC_HAND );
}

///////////////////////////////////////////////////////////////////////////////////////
// Interface Functions

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

///////////////////////////////////////////////////////////////////////////////////////
// Overridden Functions

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

	CWnd*	pWnd = GetOwner();
	if	( pWnd ){
		CRect	rectView, rectDlg;
		pWnd->GetWindowRect( &rectView );
		GetWindowRect( &rectDlg );
		int	x = rectView.left;
		int	y = rectView.bottom - rectDlg.Height();
		int	cx = rectView.Width();
		int	cy = rectDlg.Height();
		SetWindowPos( NULL, x, y, cx, cy, SWP_NOZORDER | SWP_FRAMECHANGED );
	}

	return	TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////
// Message Handlers

BEGIN_MESSAGE_MAP( CPropertyDlg, CDialog )
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
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
CPropertyDlg::OnSize( UINT nType, int cx, int cy )
{
	CDialog::OnSize( nType, cx, cy );

	CWnd*	pCtrl = GetDlgItem( IDC_STATIC_CREATE );
	if	( !pCtrl )
		return;

	CRect	rectCtrl;
	pCtrl->GetClientRect( &rectCtrl );

	int	cyCtrl = rectCtrl.Height();
	int	cxCtrl = ( cx / 2 ) - 16;
	CSize	size;
	size = GetValueSize( IDC_STATIC_CREATE );
	GetDlgItem( IDC_STATIC_CREATE   )->SetWindowPos( NULL, 0, 0, size.cx, size.cy, SWP_NOZORDER | SWP_NOMOVE );
	size = GetValueSize( IDC_STATIC_MODIFY );
	GetDlgItem( IDC_STATIC_MODIFY   )->SetWindowPos( NULL, 0, 0, size.cx, size.cy, SWP_NOZORDER | SWP_NOMOVE );
	size = GetValueSize( IDC_STATIC_PROPERTY );
	GetDlgItem( IDC_STATIC_PROPERTY )->SetWindowPos( NULL, 0, 0, size.cx, size.cy, SWP_NOZORDER | SWP_NOMOVE );

	CPoint	pt;
	pt = GetUnitPoint( IDC_STATIC_CREATE );
	GetDlgItem( IDC_STATIC_CREATED  )->SetWindowPos( NULL, pt.x, pt.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
	pt = GetUnitPoint( IDC_STATIC_MODIFY );
	GetDlgItem( IDC_STATIC_MODIFIED )->SetWindowPos( NULL, pt.x, pt.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
	pt = GetUnitPoint( IDC_STATIC_PROPERTY );
	GetDlgItem( IDC_STATIC_UNITS    )->SetWindowPos( NULL, pt.x, pt.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
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

///////////////////////////////////////////////////////////////////////////////////////
// Specific Functions

CSize
CPropertyDlg::GetValueSize( UINT uID )
{
	CWnd*	pCtrl = GetDlgItem( uID );

	CRect	rectCtrl, rectDlg;
	pCtrl->GetClientRect( &rectCtrl );
	GetClientRect( &rectDlg );

	int	cx = ( rectDlg.Width() / 2 ) -16;

	return	CSize( cx, rectCtrl.Height() );
}

CPoint
CPropertyDlg::GetUnitPoint( UINT uID )
{
	CWnd*	pCtrl = GetDlgItem( uID );

	CRect	rectWindow, rectDlg;
	pCtrl->GetWindowRect( &rectWindow );

	ScreenToClient( &rectWindow );

	GetClientRect( &rectDlg );
	int	x = ( rectDlg.Width() / 2 ) -10;
	OffsetRect( &rectWindow, x, 0 );

	return	*(CPoint*)&rectWindow;
}