// MarginDlg.cpp : implementation file
//

#include "pch.h"
#include "MarginDlg.h"
#include "MainFrm.h"
#include "resource.h"

#ifdef	UNICODE
#define	atoi	_wtoi
#define	atof	_wtof
#endif//UNICODE

IMPLEMENT_DYNAMIC( CMarginDlg, CDialog )

///////////////////////////////////////////////////////////////////////////////////////
// Constructor

CMarginDlg::CMarginDlg( CWnd* pParent )
	: CDialog( IDD_MARGINS, pParent )
{
}

///////////////////////////////////////////////////////////////////////////////////////
// Overridden Functions

BOOL
CMarginDlg::OnInitDialog( void )
{
	CDialog::OnInitDialog();

	SupportDarkMode( GetSafeHwnd() );

	GetDlgItem( IDC_EDIT_HEADER )->SetWindowText( m_param.m_strHeader );
	GetDlgItem( IDC_EDIT_FOOTER )->SetWindowText( m_param.m_strFooter );

	SetValue( IDC_EDIT_TOP,    m_param.m_nMarginT );
	SetValue( IDC_EDIT_LEFT,   m_param.m_nMarginL );
	SetValue( IDC_EDIT_RIGHT,  m_param.m_nMarginR );
	SetValue( IDC_EDIT_BOTTOM, m_param.m_nMarginB );

	GetDlgItem( IDC_STATIC_UNIT )->SetWindowText( m_param.m_bInch? _T("inch"): _T("mm") );

	return	TRUE;
}

void
CMarginDlg::OnOK( void )
{
	GetDlgItem( IDC_EDIT_HEADER )->GetWindowText( m_param.m_strHeader );
	GetDlgItem( IDC_EDIT_FOOTER )->GetWindowText( m_param.m_strFooter );

	m_param.m_nMarginT = GetValue( IDC_EDIT_TOP    );
	m_param.m_nMarginL = GetValue( IDC_EDIT_LEFT   );
	m_param.m_nMarginR = GetValue( IDC_EDIT_RIGHT  );
	m_param.m_nMarginB = GetValue( IDC_EDIT_BOTTOM );

	CDialog::OnOK();
}

BOOL
CMarginDlg::PreTranslateMessage( MSG* pMsg )
{
	if	( pMsg->message == WM_CHAR ){
		CWnd*	pCtrl = GetFocus();
		if	( pCtrl == GetDlgItem( IDC_EDIT_HEADER ) ||
			  pCtrl == GetDlgItem( IDC_EDIT_FOOTER ) )
			;
		else{
			UINT	uChar = (UINT)pMsg->wParam;
			if	( uChar == VK_RETURN )
				;
			else if	( uChar == VK_BACK )
				;
			else if	( uChar == VK_DELETE )
				;
			else if( uChar >= '0' && uChar <= '9' )
				;
			else if( uChar == '.' && m_param.m_bInch )
				;
			else
				return	TRUE;
		}
	}

	return	CDialog::PreTranslateMessage( pMsg );
}

///////////////////////////////////////////////////////////////////////////////////////
// Message Handlers

BEGIN_MESSAGE_MAP( CMarginDlg, CDialog )
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////////////
// Specific Functions

int
CMarginDlg::GetValue( UINT uID )
{
	int	nValue;

	CString	strValue;
	GetDlgItem( uID )->GetWindowText( strValue );
	if	( m_param.m_bInch ){
		double	dValue = atof( strValue.GetBuffer() );
		nValue = (int)( dValue * 25.4 );
	}
	else{
		nValue = atoi( strValue.GetBuffer() );
	}

	return	nValue;
}

void
CMarginDlg::SetValue( UINT uID, int nValue )
{
	CString	strValue;
	if	( m_param.m_bInch ){
		double	dValue = (double)nValue;
		strValue.Format( _T("%.2f"), dValue / 25.4 );
	}
	else{
		strValue.Format( _T("%d"), nValue );
	}
	GetDlgItem( uID )->SetWindowText( strValue );
}
