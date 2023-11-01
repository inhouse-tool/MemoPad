// FindDlg.cpp : implementation file
//

#include "pch.h"
#include "resource.h"
#include "FindDlg.h"
#include "MainFrm.h"

#define	TID_CLICK	1

IMPLEMENT_DYNAMIC( CFindDlg, CDialog )

///////////////////////////////////////////////////////////////////////////////////////
// Constructor

CFindDlg::CFindDlg( void )
{
	m_bAlt = false;
	m_uClicked = 0;
	m_chMode = 'f';
	m_find = { 0 };
}

///////////////////////////////////////////////////////////////////////////////////////
// Interface Functions

void
CFindDlg::ShowDialog( char chMode, CString strFind )
{
	if	( chMode == 'f' )
		;
	else if	( chMode == 'r' )
		;
	else
		return;

	ShowWindow( SW_HIDE );
	m_chMode = chMode;
	GetDlgItem( IDC_EDIT_FIND )->SetWindowText( strFind );
	GotoDlgCtrl( GetDlgItem( IDC_EDIT_FIND ) );
	Invalidate( TRUE );
	ShowWindow( SW_SHOW );
}

///////////////////////////////////////////////////////////////////////////////////////
// Overridden Functions

BOOL
CFindDlg::OnInitDialog( void )
{
	CDialog::OnInitDialog();

	((CButton*)GetDlgItem( IDC_RADIO_DOWN ))->SetCheck( BST_CHECKED );

	SupportDarkMode( GetSafeHwnd() );

	return	TRUE;
}

void
CFindDlg::OnOK( void )
{
	CString	str;
	GetDlgItem( IDC_EDIT_FIND )->GetWindowText( str );
	if	( !str.IsEmpty() )
		OnClickButton( IDC_BUTTON_NEXT );

	ShowWindow( SW_HIDE );
}

///////////////////////////////////////////////////////////////////////////////////////
// Message Handlers

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP( CFindDlg, CDialog )
	ON_WM_SHOWWINDOW()
	ON_WM_SYSCOMMAND()
	ON_WM_TIMER()
	ON_CONTROL_RANGE( BN_CLICKED, IDC_BUTTON_NEXT, IDC_BUTTON_ALL, OnClickButton )
END_MESSAGE_MAP()

#pragma warning( pop )

void
CFindDlg::OnShowWindow( BOOL bShow, UINT nStatus )
{
	CDialog::OnShowWindow( bShow, nStatus );

	if	( !bShow )
		;
	else if	( m_chMode == 'f' ){
		SetWindowText( _T("Find") );
		(GetDlgItem( IDC_STATIC_REPLACE ))->ShowWindow( SW_HIDE );
		(GetDlgItem( IDC_EDIT_REPLACE )  )->ShowWindow( SW_HIDE );
		(GetDlgItem( IDC_BUTTON_REPLACE ))->ShowWindow( SW_HIDE );
		(GetDlgItem( IDC_BUTTON_ALL )    )->ShowWindow( SW_HIDE );
	}
	else if	( m_chMode == 'r' ){
		SetWindowText( _T("Replace") );
		(GetDlgItem( IDC_STATIC_REPLACE ))->ShowWindow( SW_SHOW );
		(GetDlgItem( IDC_EDIT_REPLACE )  )->ShowWindow( SW_SHOW );
		(GetDlgItem( IDC_BUTTON_REPLACE ))->ShowWindow( SW_SHOW );
		(GetDlgItem( IDC_BUTTON_ALL )    )->ShowWindow( SW_SHOW );
	}
}

void
CFindDlg::OnSysCommand( UINT nID, LPARAM lParam )
{
	if	( nID == SC_CLOSE )
		ShowWindow( SW_HIDE );
	else
		CDialog::OnSysCommand( nID, lParam );
}

void
CFindDlg::OnTimer( UINT_PTR nIDEvent )
{
	if	( nIDEvent == TID_CLICK ){
		KillTimer( nIDEvent );
		CWnd*	pwnd = GetDlgItem( m_uClicked );
		m_uClicked = 0;
		pwnd->Invalidate( FALSE );
	}
	else
		CDialog::OnTimer( nIDEvent );
}

void
CFindDlg::OnClickButton( UINT uID )
{
	m_uClicked = uID;
	SetTimer( TID_CLICK, 200, NULL );

	m_find.bMacthCase = ((CButton*)GetDlgItem( IDC_CHECK_CASE ))->GetCheck() == BST_CHECKED;
	m_find.bUpward    = ((CButton*)GetDlgItem( IDC_RADIO_UP )  )->GetCheck() == BST_CHECKED;
	GetDlgItem( IDC_EDIT_FIND    )->GetWindowText( m_find.strFindWhat );
	GetDlgItem( IDC_EDIT_REPLACE )->GetWindowText( m_find.strReplaceWith );

	WPARAM	wparam = ( (WPARAM)uID - IDC_BUTTON_NEXT ) + FIND_COMMAND_NEXT;
	CWnd*	pwnd = GetOwner();
	if	( pwnd )
		pwnd->PostMessage( WM_FIND, wparam, (LPARAM)&m_find );
}
