// MessageDlg.cpp : implementation file
//

#include "pch.h"
#include "MessageDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef	UNICODE
#define	strcpy_s	wcscpy_s
#endif//UNICODE

#define	ID_MESSAGE	2

///////////////////////////////////////////////////////////////////////////////////////
// Constructor

CMessageDlg::CMessageDlg( void )
{
	Create();
}

///////////////////////////////////////////////////////////////////////////////////////
// Interface Functions

void
CMessageDlg::SetMessage( LPCTSTR pszMessage )
{
	int	nShow = SW_HIDE;

	CEdit*	pEdit = (CEdit*)GetDlgItem( ID_MESSAGE );
	if	( pEdit ){
		CString	strMessage = pszMessage;
		for	( int i = 0; ( i = strMessage.Find( '\n', i ) ) >= 0; )
			if	( i > 0 && strMessage[i-1] != '\r' ){
				strMessage.Insert( i, '\r' );
				i += 2;
			}

		if	( !strMessage.IsEmpty() ){
			FitSize( strMessage );
			pEdit->SetWindowText( strMessage );
			nShow = SW_SHOW;
			MessageBeep( MB_OK );
			GotoDlgCtrl( GetDlgItem( IDOK ) );
		}
	}

	ShowWindow( nShow );
}

///////////////////////////////////////////////////////////////////////////////////////
// Overridden Functions

void
CMessageDlg::OnCancel( void )
{
	OnOK();
}

void
CMessageDlg::OnOK( void )
{
	ShowWindow( SW_HIDE );
}

///////////////////////////////////////////////////////////////////////////////////////
// Message Handlers

BEGIN_MESSAGE_MAP( CMessageDlg, CDialog )
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////////////
// Specific Functions

BOOL
CMessageDlg::Create( void )
{
	WCHAR	awchTitle[] = L"Message";
	WCHAR	awchItem1[] = L"OK";
	int	nMargin = 10;
	int	xButton = 40;
	int	yButton = 16;

	struct{
		DLGTEMPLATE	temp;
		WORD		wMenuID;
		WORD		wClass;
		WCHAR		awchTitle[sizeof(awchTitle)/2];
		WORD		wPad;

		DLGITEMTEMPLATE	item1;
		WORD		wHItem1;
		WORD		wLItem1;
		WCHAR		awchItem1[sizeof(awchItem1)/2];
		WORD		wcbDataItem1;
		WORD		wPad1;

		DLGITEMTEMPLATE	item2;
		WORD		wHItem2;
		WORD		wLItem2;
		WCHAR		awchItem2[2];
		WORD		wcbDataItem2;
	}	stArg;

	memset( &stArg, 0, sizeof( stArg ) );

	stArg.temp.style = WS_POPUP | WS_BORDER | WS_SYSMENU | WS_CAPTION | DS_MODALFRAME;
	stArg.temp.cdit  =   2;
	stArg.temp.x     =   0;
	stArg.temp.y     =   0;
	stArg.temp.cx    = 225;
	stArg.temp.cy    = 100;
	memcpy( stArg.awchTitle, awchTitle, sizeof( awchTitle ) );

	stArg.item1.style = WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON;
	stArg.item1.dwExtendedStyle = 0;
	stArg.item1.x  = stArg.temp.cx - xButton - nMargin;
	stArg.item1.y  = stArg.temp.cy - yButton - nMargin;
	stArg.item1.cx = xButton;
	stArg.item1.cy = yButton;
	stArg.item1.id = IDOK;
	stArg.wHItem1  = 0xffff;
	stArg.wLItem1  = 0x0080;	// 0080: Button
	memcpy( stArg.awchItem1, awchItem1, sizeof( awchItem1 ) );

	stArg.item2.style = WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY;
	stArg.item2.dwExtendedStyle = 0;
	stArg.item2.x  = nMargin;
	stArg.item2.y  = nMargin;
	stArg.item2.cx = stArg.temp.cx - ( nMargin * 2 );
	stArg.item2.cy = stArg.item1.y - ( nMargin * 2 );
	stArg.item2.id = ID_MESSAGE;
	stArg.wHItem2  = 0xffff;
	stArg.wLItem2  = 0x0081;	// 0081: Edit

	BOOL	bDone =
	CreateIndirect( &stArg, NULL, NULL );

	if	( bDone ){
		CEdit*	pEdit = (CEdit*)GetDlgItem( ID_MESSAGE );
		if	( pEdit ){
			LOGFONT	lf;
			memset( &lf, 0, sizeof( lf ) );

			strcpy_s( lf.lfFaceName, _T("Arial") );
			lf.lfCharSet        = ANSI_CHARSET;
			lf.lfPitchAndFamily = VARIABLE_PITCH;
			lf.lfHeight  = 16;
			lf.lfWeight  = 400;
			m_font.DeleteObject();
			m_font.CreateFontIndirect( &lf );

			pEdit->SetFont( &m_font );
		}
		m_dark.RefreshNC( GetSafeHwnd() );
	}
	else{
		DWORD	dwError = GetLastError();
		CString	str;
		str.Format(  _T("MessageDlg: Failed to create with error %08x."), dwError );
		AfxMessageBox( str );
	}
		
	return	bDone;
}

void
CMessageDlg::FitSize( LPCTSTR pszMessage )
{
	CEdit*	pEdit = (CEdit*)GetDlgItem( ID_MESSAGE );
	DWORD	dwMargins = pEdit->GetMargins();
	int	xMargin = dwMargins&0xffff;
	int	yMargin = dwMargins>>16;
	CRect	rect, rectClient;

	CDC*	pDC = pEdit->GetDC();
	CFont*	pFont = pDC->SelectObject( &m_font );
	pDC->DrawText( pszMessage, &rect, DT_NOPREFIX | DT_CALCRECT );
	pDC->SelectObject( pFont );
	pEdit->ReleaseDC( pDC );

	int	xMinimum = 200;
	rect.InflateRect( xMargin*2, yMargin*2 );
	if	( rect.Width() < xMinimum )
		rect.right = rect.left + xMinimum;

	pEdit->SetWindowPos( NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOMOVE );
	pEdit->GetWindowRect( &rect );
	GetWindowRect( &rectClient );
	xMargin = rect.left - rectClient.left;
	yMargin = rect.top  - rectClient.top;

	int	x = xMargin + rect.Width();
	int	y = xMargin + rect.Height();

	CWnd*	pWnd = GetDlgItem( IDOK );
	pWnd->GetWindowRect( &rect );
	x -= rect.Width();
	pWnd->SetWindowPos( NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
	x += rect.Width()  + xMargin;
	y += rect.Height() + yMargin;
	SetWindowPos( &wndTopMost, 0, 0, x, y, SWP_NOMOVE );
	CenterWindow();
}