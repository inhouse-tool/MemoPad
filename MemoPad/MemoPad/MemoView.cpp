// MemoView.cpp : implementation of the CMemoView class
//

#include "pch.h"
#include "MemoView.h"
#include "MainFrm.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef	UNICODE
#define	strcpy_s	wcscpy_s
#define	strtol		wcstol
#endif//UNICODE

#define	TID_REPLACEALL	1
#define	TID_UNDOALL	2
#define	TID_REDOALL	3
#define	TID_SET_FONT	4
#define	TID_INDICATE	5

TCHAR*
CMemoView::m_apchEncode[]
	= { _T("ASCII"), _T("Shift JIS"),
	    _T("UTF-8"), _T("UTF-8 with BOM"), _T("UTF-16LE with BOM"), _T("UTF-16BE with BOM"),
	    _T("Unknown"), NULL };

///////////////////////////////////////////////////////////////////////////////////////
// Constructor

CMemoView::CMemoView( void )
{
	CWinApp*	pApp = AfxGetApp();
	m_bDiscardModified = pApp->GetProfileInt(    _T("Settings"), _T("DiscardModified"), 0 );
	m_crText           = pApp->GetProfileInt(    _T("Settings"), _T("ColorText"),      -1 );
	m_crBack           = pApp->GetProfileInt(    _T("Settings"), _T("ColorBack"),      -1 );
	m_nRepeatTime      = pApp->GetProfileInt(    _T("Settings"), _T("RepeatTime"),     32 );

	m_brBack = CreateSolidBrush( m_crBack );

	m_bInsert = true;

	m_nFontHeight    = 0;
	m_nFontHeightOrg = 0;

	m_cbBOM   = 0;
	m_dwBOM   = 0;
	m_eEncode = ASCII;
	m_cbEOL   = 0;

	m_bFindUp     = true;
	m_bFindCase   = false;
	m_bReplaceAll = false;
	m_nFound      = 0;
	m_xFirst      = -1;
	m_bWrapped    = false;
	m_iUndo       = 0;
	m_bNoDiff    = false;
}

///////////////////////////////////////////////////////////////////////////////////////
// Interface Functions

DWORD
CMemoView::GetStatus( void )
{
	DWORD	dwStatus = 0x0;

	if	( m_strFile.IsEmpty() )
		dwStatus |= STAT_NOFILE;
	if	( m_strLines.IsEmpty() )
		dwStatus |= STAT_EMPTY;
	if	( GetModify() )
		dwStatus |= STAT_MODIFIED;
	if	( CanUndo() )
		dwStatus |= STAT_UNDOABLE;
	if	( CanRedo() )
		dwStatus |= STAT_REDOABLE;
	int	xStart, xEnd;
	GetSel( xStart, xEnd );
	if	( xStart != xEnd )
		dwStatus |= STAT_SELECTED;
	if	( IsPasteable() )
		dwStatus |= STAT_PASTEABLE;
	if	( m_nFound > 0 )
		dwStatus |= STAT_FOUND;

	return	dwStatus;
}

///////////////////////////////////////////////////////////////////////////////////////
// Overridden Functions

BOOL
CMemoView::PreTranslateMessage( MSG* pMsg )
{
	if	( pMsg->message == WM_CHAR ){
		if	( !m_bInsert ){
			int	xStart, xEnd;
			GetSel( xStart, xEnd );
			if	( xEnd == xStart )
				SetSel( xStart, ++xEnd );
			ReplaceSel( _T("") );
		}
	}
	else if	( pMsg->message == WM_KEYDOWN ){

		// Insert key was hit: Toggle Insert.

		if	( pMsg->wParam == VK_INSERT ){
			m_bInsert = !m_bInsert;
			if	( m_bInsert )
				AfxGetMainWnd()->SetFocus();
			else{
				int	xStart, xEnd;
				GetSel( xStart, xEnd );
				SetSel( xStart, xEnd, FALSE );
				SetTimer( TID_INDICATE, 0, NULL );
			}
		}
	}
	else if	( pMsg->message == WM_KEYUP ||
		  pMsg->message == WM_LBUTTONUP ){

		// Just after the search has failed: Keep the indicator once.

		if	( m_nFound < 0 )
			m_nFound = 0;

		// Other cases: Renew indicator.

		else
			SetTimer( TID_INDICATE, 0, NULL );
	}
	else if( pMsg->message == WM_LBUTTONDBLCLK ){
		SelectWord();
		return	TRUE;
	}

	return	CEdit::PreTranslateMessage( pMsg );
}

///////////////////////////////////////////////////////////////////////////////////////
// Message Handlers

BEGIN_MESSAGE_MAP( CMemoView, CEdit )
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_DROPFILES()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_TIMER()
	ON_WM_CTLCOLOR_REFLECT()

	ON_COMMAND( ID_FILE_OPEN,    OnFileOpen )
	ON_COMMAND( ID_FILE_SAVE,    OnFileSave )
	ON_COMMAND( ID_FILE_SAVE_AS, OnFileSaveAs )
	ON_COMMAND( ID_FILE_CLOSE,   OnFileClose )
	ON_COMMAND( ID_FILE_PRINT,   OnFilePrint )
	ON_COMMAND( ID_FILE_UPDATE,  OnFileProperties )
	ON_COMMAND( ID_EDIT_UNDO,          OnEditUndo )
	ON_COMMAND( ID_EDIT_REDO,          OnEditRedo )
	ON_COMMAND( ID_EDIT_CUT,           OnEditCut )
	ON_COMMAND( ID_EDIT_COPY,          OnEditCopy )
	ON_COMMAND( ID_EDIT_PASTE,         OnEditPaste )
	ON_COMMAND( ID_EDIT_CLEAR,         OnEditClear )
	ON_COMMAND( ID_EDIT_FIND,          OnEditFind )
	ON_COMMAND( ID_NEXT_PANE,          OnEditFindNext )
	ON_COMMAND( ID_PREV_PANE,          OnEditFindPrev )
	ON_COMMAND( ID_EDIT_REPLACE,       OnEditReplace )
	ON_COMMAND( ID_EDIT_SELECT_ALL,    OnEditSelectAll )
	ON_COMMAND( ID_EDIT_PASTE_SPECIAL, OnEditInsertUnicode )
	ON_COMMAND( ID_FORMAT_FONT,    OnViewFont )
	ON_COMMAND( ID_WINDOW_CASCADE, OnViewZoomIn )
	ON_COMMAND( ID_WINDOW_ARRANGE, OnViewZoomOut )
	ON_COMMAND( ID_WINDOW_NEW,     OnViewZoomRestore )
	ON_CONTROL_REFLECT( EN_UPDATE, OnEditUpdate )
	ON_CONTROL_REFLECT( EN_CHANGE, OnEditChange )

	ON_MESSAGE( WM_FIND, OnFind )
	ON_MESSAGE( WM_FONT, OnFont )
END_MESSAGE_MAP()

int
CMemoView::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	CEdit::OnCreate( lpCreateStruct );

	// Create the attached modeless dialogs.

	m_dlgFind.SetOwner( this );
	m_dlgFind.Create( IDD_FIND, NULL );
	m_dlgFont.SetOwner( this );
	m_dlgFont.Create( IDD_FONT, NULL );

	// Initialize the edit view.

	SetMargins( 4, 4 );
	SetLimitText( 0 );

	PostMessage( WM_SETCURSOR, HTCLIENT, NULL );
	DragAcceptFiles();

	// When launched with argument: Open the file specified by the argument.

	CString	strArg = AfxGetApp()->m_lpCmdLine;
	if	( !strArg.IsEmpty() ){
		if	( strArg[0] == '"' ){
			strArg.Delete( 0, 1 );
			int	x = strArg.Find( '"' );
			if	( x >= 0 )
				strArg = strArg.Left( x );
		}
		LoadFile( strArg );
	}

	SetTimer( TID_INDICATE, 0, NULL );

	return	0;
}

void
CMemoView::OnDestroy( void )
{
	m_dlgFind.DestroyWindow();
	m_dlgFont.DestroyWindow();

	CEdit::OnDestroy();
}

void
CMemoView::OnDropFiles( HDROP hDropInfo )
{
	// Accept just one file to open.

	UINT	nFile = DragQueryFile( hDropInfo, -1, NULL, 0 );
	if	( nFile != 1 )
		return;

	TCHAR	szFile[MAX_PATH];
	DragQueryFile( hDropInfo, 0, szFile, MAX_PATH );
	LoadFile( szFile );
}

BOOL
CMemoView::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
	// Wheel with Ctrl key: Zoom in / out.

	if	( nFlags & MK_CONTROL ){
		if	( zDelta > 0 )
			OnViewZoomIn();
		else
			OnViewZoomOut();
		return	TRUE;
	}

	// Wheel without Ctrl key: Scroll as usual.

	else
		return	CEdit::OnMouseWheel( nFlags, zDelta, pt );
}

void
CMemoView::OnRButtonDown( UINT nFlags, CPoint point )
{
	CMenu menu;
	menu.LoadMenu( IDR_POPUP );
	CMenu* pPopupMenu = menu.GetSubMenu( 0 );

	DWORD	dwStatus = GetStatus();

	pPopupMenu->EnableMenuItem( ID_EDIT_UNDO,       MF_BYCOMMAND | ( ( dwStatus & STAT_UNDOABLE )? MF_ENABLED: MF_DISABLED ) );
	pPopupMenu->EnableMenuItem( ID_EDIT_CUT,        MF_BYCOMMAND | ( ( dwStatus & STAT_SELECTED )? MF_ENABLED: MF_DISABLED ) );
	pPopupMenu->EnableMenuItem( ID_EDIT_COPY,       MF_BYCOMMAND | ( ( dwStatus & STAT_SELECTED )? MF_ENABLED: MF_DISABLED ) );
	pPopupMenu->EnableMenuItem( ID_EDIT_PASTE,      MF_BYCOMMAND | ( ( dwStatus & STAT_PASTEABLE)? MF_ENABLED: MF_DISABLED ) );
	pPopupMenu->EnableMenuItem( ID_EDIT_CLEAR,      MF_BYCOMMAND | ( ( dwStatus & STAT_SELECTED )? MF_ENABLED: MF_DISABLED ) );
	pPopupMenu->EnableMenuItem( ID_EDIT_SELECT_ALL, MF_BYCOMMAND | ( ( dwStatus & STAT_EMPTY    )? MF_DISABLED: MF_ENABLED ) );

	ClientToScreen( &point );
	pPopupMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this );

	PostMessage( WM_NULL, 0, 0 );
}

void
CMemoView::OnTimer( UINT_PTR nIDEvent )
{
	if	( nIDEvent == TID_REPLACEALL ){
		KillTimer( nIDEvent );
		OnFind( FIND_COMMAND_ALL, 0 );
	}
	else if	( nIDEvent == TID_UNDOALL ){
		KillTimer( nIDEvent );
		Undo();
	}
	else if	( nIDEvent == TID_REDOALL ){
		KillTimer( nIDEvent );
		Redo();
	}

	else if	( nIDEvent == TID_SET_FONT ){
		KillTimer( nIDEvent );

		CFont*	pFont = GetFont();
		LOGFONT	lf;
		pFont->GetLogFont( &lf );

		lf.lfHeight  = m_nFontHeight;

		SetFont( &lf );
	}	
	else if	( nIDEvent == TID_INDICATE ){
		KillTimer( nIDEvent );

		// To renew cursor position: Calculate Line and Column.

		int	xStart, xEnd;
		GetSel( xStart, xEnd );
		int	x = xEnd;
		int	iChar = x;
		int	y  = LineFromChar( x );
		x -= LineIndex( y );

		// Indicate the Line and Column.

		CString	str;
		str.Format( _T("Lin %d, Col %d"), y+1, x+1 );
		SetIndicator( 0, str.GetBuffer() );

		// Not in insert mode: Show a square caret.

		if	( !m_bInsert ){
			CClientDC	dc( this );
			HFONT	hFont = (HFONT)SendMessage( WM_GETFONT, 0, 0 );
			HFONT	hFontOld = (HFONT)SelectObject( dc.m_hDC, hFont );

			TCHAR	ach[2] = {};
			ach[0] = m_strLines.GetAt( iChar );
			if	( ach[0] < ' ' )
				ach[0] = ' ';
			CSize	size = dc.GetTextExtent( ach );

			SelectObject( dc.m_hDC, hFontOld );
			CreateSolidCaret( size.cx, size.cy );
			ShowCaret();
		}
	}
	else
		CEdit::OnTimer( nIDEvent );
}

HBRUSH
CMemoView::CtlColor( CDC* pDC, UINT nCtlColor )
{
	// Not for Edit control: Do nothing.

	if	( nCtlColor != CTLCOLOR_EDIT )
		return	NULL;

	// Colors not customized: Do nothing.

	else if	( m_crText == -1 &&
		  m_crBack == -1 )
		return	NULL;

	// Colors customized: Apply the specified colors.

	else{
		pDC->SetTextColor( m_crText );
		pDC->SetBkColor(   m_crBack );

		return	m_brBack;
	}
}

void
CMemoView::OnFileOpen( void )
{
	// With unsaved modified file: Warn it.

	if	( !ConfirmDiscard() )
		return;

	// Open the file dialog.

	TCHAR	szFile[MAX_PATH] = {};

	CFileDialog	dlg( TRUE );
	dlg.m_ofn.lpstrFilter     = _T("Text file\0*.txt;*.log\0Any file\0*.*\0");
	dlg.m_ofn.lpstrFile       = szFile;
	dlg.m_ofn.lpstrInitialDir = NULL;
	dlg.m_ofn.lpstrTitle      = _T("Open");
	*szFile = '\0';

	// Quit unless OK.

	if	( dlg.DoModal() != IDOK )
		return;

	// Load the specified file.

	LoadFile( dlg.m_ofn.lpstrFile );
}

void
CMemoView::OnFileSave( void )
{
	// New file: Save new.

	if	( m_strFile.IsEmpty() )
		OnFileSaveAs();

	// Existing file: Save in it.

	else{
		Encode	eEncode;
		GetLowestEncode( eEncode );
		if	( eEncode > m_eEncode )
			m_eEncode = eEncode;
		if	( !SaveFile( m_strFile ) )
			return;
	}

	// Keep unmodified image.

	GetWindowText( m_strLinesOrg );
	SetModify( FALSE );

	// Remove modified mark.

	RenewTitle();
}

#define	IDC_COMBO_ENCODE	1

void
CMemoView::OnFileSaveAs( void )
{
	CString	strExt = _T("txt");
	LPCWSTR	pchFilter = _T("Text file\0*.txt\0Log file\0*.log\0Any file\0*.*\0");
	int	x = m_strFile.ReverseFind( '.' );
	if	( x >= 0 ){
		strExt = m_strFile.Mid( x+1 );
		if	( !strExt.CompareNoCase( _T("txt") ) )
			pchFilter = _T("Text file\0*.txt\0Log file\0*.log\0Any file\0*.*\0");
		else if	( !strExt.CompareNoCase( _T("log") ) )
			pchFilter = _T("Log file\0*.log\0Text file\0*.txt\0Any file\0*.*\0");
		else
			pchFilter = _T("Any file\0*.*\0Log file\0*.log\0Text file\0*.txt\0");
	}

	// Open the file dialog.

	TCHAR	szFile[MAX_PATH] = {};

	CFileDialog	dlg( FALSE );
	dlg.m_ofn.lpstrFilter     = pchFilter;
	dlg.m_ofn.lpstrFile       = szFile;
	dlg.m_ofn.lpstrDefExt     = strExt.GetBuffer();
	dlg.m_ofn.lpstrInitialDir = NULL;
	dlg.m_ofn.lpstrTitle      = _T("Save as");
	dlg.m_ofn.Flags          &= ~OFN_OVERWRITEPROMPT;
	*szFile = '\0';

	// Select the default encoding.

	Encode	eEncode;

	GetLowestEncode( eEncode );
	if	( m_strFile.IsEmpty() )
		m_eEncode = eEncode;
	SetDefaultEncode( dlg, eEncode );

	// Quit unless OK.

	if	( dlg.DoModal() != IDOK )
		return;

	// Save in the specified file.

	GetSpecifiedEncode( dlg );
	m_strFile = dlg.m_ofn.lpstrFile;
	if	( !SaveFile( m_strFile ) )
		return;

	// Keep unmodified image.

	GetWindowText( m_strLinesOrg );
	SetModify( FALSE );

	// Set window title.

	SetTitle( m_strFile );
}

void
CMemoView::OnFileClose( void )
{
	// With unsaved modified file: Warn it.

	if	( !ConfirmDiscard() )
		return;

	// Then close the application.

	AfxGetMainWnd()->PostMessage( WM_CLOSE, 0, 0 );
}

void
CMemoView::OnFilePrint( void )
{
	CPrintDlg	dlg;
	if	( dlg.DoModal() == IDOK )
		Print( dlg.m_param );
}

void
CMemoView::OnFileProperties( void )
{
	CString	str, strProperties;

	int	nLine = m_strLines.IsEmpty()? 0: GetLineCount();
	strProperties += CommaDigitsOf( nLine );
	strProperties += _T("\r\n" );

	DWORD	cchData = m_strLines.GetLength();
	if	( m_cbEOL == 1 )
		cchData -= nLine-1;
	strProperties += CommaDigitsOf( cchData );
	strProperties += _T("\r\n" );

	DWORD	cbFile = GetSizeOnFile( m_strFile, GetModify() );
	strProperties += CommaDigitsOf( cbFile );
	strProperties += _T("\r\n" );

	strProperties += CommaDigitsOf( GetSizeOnDisk( m_strFile, cbFile ) );
	strProperties += _T("\r\n" );

	strProperties += CommaDigitsOf( m_cbBOM );
	strProperties += _T("\r\n" );

	int	i;
	if	( m_eEncode == ASCII )
		i = 0;
	else if	( m_eEncode == ShiftJIS )
		i = 1;
	else if	( m_eEncode == UTF8 )
		i = 2;
	else if	( m_eEncode == UTF16LE )
		i = 4;
	else if	( m_eEncode == UTF16BE )
		i = 5;
	else
		i = 6;
	str.Format( _T("%s\n"), m_apchEncode[i] );
	if	( cchData == 0 )
		str = _T("Not");
	strProperties += str;

	CPropertyDlg	dlg;
	dlg.SetFile( m_strFile );
	dlg.SetText( strProperties );
	dlg.SetModified( GetModify() );
	dlg.SetOwner( this );
	dlg.DoModal();
}

void
CMemoView::OnEditUndo( void )
{
	Undo();
}

void
CMemoView::OnEditRedo( void )
{
	Redo();
}

void
CMemoView::OnEditCut( void )
{
	Cut();
}

void
CMemoView::OnEditCopy( void )
{
	Copy();
}

void
CMemoView::OnEditPaste( void )
{
	Paste();
}

void
CMemoView::OnEditClear( void )
{
	Clear();
}

void
CMemoView::OnEditFind( void )
{
	CRect	rectView, rectDlg;
	GetWindowRect( &rectView );
	m_dlgFind.GetWindowRect( &rectDlg );
	int	x = rectView.right - rectDlg.Width();
	int	y = rectView.bottom - rectDlg.Height();
	m_dlgFind.SetWindowPos( NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED );
	m_dlgFind.ShowDialog( 'f', GetSelected() );
}

void
CMemoView::OnEditFindNext( void )
{
	m_nFound = 0;
	OnFind( FIND_COMMAND_NEXT, 0 );
}

void
CMemoView::OnEditFindPrev( void )
{
	m_nFound = 0;
	m_bFindUp = !m_bFindUp;
	OnFind( FIND_COMMAND_NEXT, 0 );
	m_bFindUp = !m_bFindUp;
}

void
CMemoView::OnEditReplace( void )
{
	CRect	rectView, rectDlg;
	GetWindowRect( &rectView );
	m_dlgFind.GetWindowRect( &rectDlg );
	int	x = rectView.right - rectDlg.Width();
	int	y = rectView.bottom - rectDlg.Height();
	m_dlgFind.SetWindowPos( NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED );
	m_dlgFind.ShowDialog( 'r', GetSelected() );
}

void
CMemoView::OnEditSelectAll( void )
{
	SetSel( 0, -1, FALSE );
}

void
CMemoView::OnEditInsertUnicode( void )
{
	int	xStart, xEnd;
	GetSel( xStart, xEnd );

	if	( xEnd == xStart ){
		if	( !m_strLines[xEnd] )
			xStart = --xEnd;
		else if	(  m_strLines[xEnd] == '\r' )
			xStart = --xEnd;

		for	( ;; ){
			TCHAR	ch = m_strLines[xStart];
			if	( !ch )
				break;
			else if	( ( ch >= '0' && ch <= '9' ) ||
				  ( ch >= 'A' && ch <= 'F' ) ||
				  ( ch >= 'a' && ch <= 'f' )    )
				xStart--;
			else{
				xStart++;
				break;
			}
			if	( xEnd - xStart > 6 )
				return;
			if	( xStart <= 0 )
				break;
		}
	}
	if	( ( xEnd - xStart <= 3 ) || ( xEnd - xStart > 5 ) )
		return;

	CString	strHex = m_strLines.Mid( xStart, ( xEnd - xStart + 1 ) );
	DWORD	dwUnicode = (DWORD)strtol( strHex.GetBuffer(), NULL, 16 );
	if	( ( dwUnicode < 0x20 ) || ( dwUnicode > 0x10ffff ))
		return;

	SetSel( xStart, xEnd+1, TRUE );
	CString	strUnicode;
	int	nch = 1;

	// Codes 0x00000 - 0x0ffff: Set as a Unicode.

	if	( dwUnicode < 0x10000 )
		strUnicode.Format( _T("%c"), (TCHAR)dwUnicode );

	// Codes 0x10000 - 0x30000: Set as a surrogate pair.

	else{
		dwUnicode -= 0x10000;
		WORD	wL = (WORD)( dwUnicode & 0x03ff );
		wL += 0xdc00;
		WORD	wH = (WORD)( dwUnicode >> 10 );
		wH += 0xd800;
		strUnicode.Format( _T("%c%c"), (TCHAR)wH, (TCHAR)wL );
		nch = 2;
	}

	ReplaceSel( strUnicode, TRUE );

	SetSel( xStart+nch, xStart+nch, TRUE );
}

void
CMemoView::OnViewFont( void )
{
	CRect	rectView, rectDlg;
	GetWindowRect( &rectView );
	m_dlgFont.GetWindowRect( &rectDlg );
	int	x = rectView.right - rectDlg.Width();
	int	y = rectView.bottom - rectDlg.Height();
	m_dlgFont.SetWindowPos( NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED );
	m_dlgFont.ShowWindow( SW_SHOW );
}

void
CMemoView::OnViewZoomIn( void )
{
	m_nFontHeight -= 1;
	if	( m_nFontHeight < -800 )
		m_nFontHeight = -800;

	SetTimer( TID_SET_FONT, 20, NULL );
}

void
CMemoView::OnViewZoomOut( void )
{
	m_nFontHeight += 1;
	if	( m_nFontHeight > -8 )
		m_nFontHeight = -8;

	SetTimer( TID_SET_FONT, 20, NULL );
}

void
CMemoView::OnViewZoomRestore( void )
{
	CFont*	pFont = GetFont();
	LOGFONT	lf;
	pFont->GetLogFont( &lf );

	m_nFontHeight = m_nFontHeightOrg;
	lf.lfHeight = m_nFontHeight;

	SetTimer( TID_SET_FONT, 20, NULL );
}

void
CMemoView::OnEditUpdate( void )
{
	m_strLinesLast = m_strLines;
}

void
CMemoView::OnEditChange( void )
{
	GetWindowText( m_strLines );
	RenewTitle();
	TakeDiff();
}

LRESULT
CMemoView::OnFind( WPARAM wParam, LPARAM lParam )
{
	// Take parameters given from the dialog.

	FIND*	pFind = (FIND*)lParam;
	bool	bReplace = false;
	if	( pFind ){
		m_bFindCase   = pFind->bMacthCase;
		m_bFindUp     = pFind->bUpward;
		bReplace      = wParam == FIND_COMMAND_REPLACE;
		m_bReplaceAll = wParam == FIND_COMMAND_ALL;
		m_strFind     = pFind->strFindWhat;
		m_strReplace  = pFind->strReplaceWith;
		m_nFound      = 0;
		m_xFirst      = -1;
		m_bWrapped    = false;
	}

	// Do Replace now for the last found string.

	int	xStart, xEnd;
	if	( bReplace ){
		GetSel( xStart, xEnd );
		CString	strLast = m_strLines.Mid( xStart, ( xEnd - xStart ) );
		if	( strLast == m_strFind ){
			m_bNoDiff = true;
			ReplaceSel( m_strReplace, TRUE );
			SetSel( xStart, xStart+m_strReplace.GetLength(), FALSE );
			m_bNoDiff = false;

			CUndo	undo;
			undo.m_iChar = xStart;
			undo.m_strText.Format( _T("R%s\b%s"), strLast.GetBuffer(), m_strReplace.GetBuffer() );
			m_aUndo.Add( undo );
			m_iUndo = m_aUndo.GetCount();
		}
	}

	// Make lower case when 'match case' is not specified.

	CString	strLines = m_strLines;
	CString	strFind  = m_strFind;
	if	( !m_bFindCase ){
		strLines.MakeLower();
		strFind .MakeLower();
	}

	// Get current position to start search.

	GetSel( xStart, xEnd );
	int	x = m_bFindUp? xStart-1: xEnd;

	// Initialize other parameters.

	bool	bWrapped = false;
	if	( m_nFound < 0 )
		m_nFound = 0;
	int	cch = strFind.GetLength();

	// Search the specified text.

	for	( ;; ){
		if	( m_bFindUp ){
			for	( ; x >= 0; x-- )
				if	( strLines.Mid( x, cch ) == strFind )
					break;
		}
		else{
			x = strLines.Find( strFind, x );
		}

		if	( x >= 0 ){
			if	( m_xFirst >= 0 ){
				if	( !m_bWrapped )
					;
				else if	( m_bFindUp ){
					if	( x <= m_xFirst )
						x = -1;
				}
				else{
					if	( x >= m_xFirst )
						x = -1;
				}
			}
			else
				m_xFirst = x;
			if	( x >= 0 ){
				m_nFound++;
				if	( m_bReplaceAll ){
					if	( x == m_xFirst ){
						int	xOffset = m_strReplace.Find( m_strFind );
						if	( xOffset > 0 )
							m_xFirst += xOffset;
					}
					else if	( x <  m_xFirst )
						m_xFirst += m_strReplace.GetLength() - m_strFind.GetLength();
				}
			}
			break;
		}
		else if	( bWrapped ){
			CString	str;
			if	( m_bReplaceAll )
				str.Format( _T("%d replaced"), m_nFound );
			else
				str = _T("Not found");

			m_nFound = -1;
			SetIndicator( 0, str.GetBuffer() );
			break;
		}
		else if	( m_bFindUp ){
			bWrapped = true;
			if	( m_nFound > 0 )
				m_bWrapped = true;
			x = strLines.GetLength();
		}
		else{
			bWrapped = true;
			if	( m_nFound > 0 )
				m_bWrapped = true;
			x = 0;
		}
	}

	if	( x >= 0 ){

		// Replace or select the text hit.

		CString	strLast = m_strLines.Mid( x, m_strFind.GetLength() );
		if	( m_bReplaceAll ){
			m_bNoDiff = true;
			SetSel( x, x+m_strFind.GetLength(), FALSE );
			ReplaceSel( m_strReplace, TRUE );
			SetSel( x, x+m_strReplace.GetLength(), FALSE );
			m_bNoDiff = false;

			CUndo	undo;
			undo.m_iChar = x;
			undo.m_strText.Format( _T("%c%s\b%s"),
				( m_nFound == 1 )? 'R': 'r', strLast.GetBuffer(), m_strReplace.GetBuffer() );
			m_aUndo.Add( undo );
			m_iUndo = m_aUndo.GetCount();

			SetTimer( TID_REPLACEALL, m_nRepeatTime, NULL );
		}
		else
			SetSel( x, x+cch, FALSE );

		// Show line & column directly while the find dialog has caret.

		if	( lParam ){
			x += cch;
			int	y  = LineFromChar( x );
			x -= LineIndex( y );

			CString	str;
			str.Format( _T("Lin %d, Col %d"), y+1, x+1 );
			SetIndicator( 0, str.GetBuffer() );
		}
	}

	return	TRUE;
}

LRESULT
CMemoView::OnFont( WPARAM wParam, LPARAM lParam )
{
	LOGFONT	lf = *(LOGFONT*)lParam;
	m_nFontHeightOrg = m_nFontHeight = lf.lfHeight;

	SetFont( &lf );

	return	TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////
// Specific Functions

bool
CMemoView::LoadFile( CString strFile )
{
	CWaitCursor	wc;

	CFile	f;
	if	( f.Open( strFile, CFile::modeRead | CFile::shareDenyNone ) ){

		// First, read the specified file as binary image.

		QWORD	cbData = f.GetLength();
		BYTE*	pbData = new BYTE[cbData+2];

		f.Read( pbData, (DWORD)cbData );
		pbData[cbData+0] = '\0';
		pbData[cbData+1] = '\0';

		// Then, get encode type.

		m_cbBOM = 0;
		m_dwBOM = 0;
		m_eEncode = unknown;
		m_cbEOL   = 0;

		GetTextEncode( pbData, cbData );
		CHAR*	pbText = (CHAR*)( pbData + m_cbBOM );

		// ASCII or Shift JIS encoded: Take the image as ANSI text.

		if	( m_eEncode == ASCII || m_eEncode == ShiftJIS )
			SetWindowTextA( GetSafeHwnd(), pbText );

		// UTF-8 encoded: Take the image as UTF-8 encoded sequence.

		else if	( m_eEncode == UTF8 ){
			int	cwch =
			::MultiByteToWideChar( CP_UTF8, 0, pbText, -1, NULL, 0 );
			WCHAR*	pwch = new WCHAR[cwch+1];
			::MultiByteToWideChar( CP_UTF8, 0, pbText, -1, pwch, cwch );
			SetWindowText( pwch );
			delete[]	pwch;
		}

		// UTF-16 Little Endian encoded: Take the image as wide character string.

		else if	( m_eEncode == UTF16LE )
			SetWindowTextW( (WCHAR*)pbText );

		// UTF-16 Big Endian encoded: Swap bytes of the image and take it as wide character string.

		else if	( m_eEncode == UTF16BE ){
			for	( int i = 0; i < cbData; i += 2 ){
				BYTE	b = pbData[i+0];
				pbData[i+0] = pbData[i+1];
				pbData[i+1] = b;
			}
			SetWindowTextW( (WCHAR*)pbText );
		}

		// Others: Empty edit control.

		else
			SetWindowText( _T("") );

		delete[]	pbData;

		// Set window title.

		m_strFile = strFile;
		SetTitle( strFile );

		// Keep the original text.

		GetWindowText( m_strLinesOrg );
		if	( m_cbEOL == 1 ){
			m_strLinesOrg.Replace( _T("\n"), _T("\r\n") );
			SetWindowText( m_strLinesOrg.GetBuffer() );
		}
		GetWindowText( m_strLines );
	
		// Set no Undo/Redo is stacked.

		m_aUndo.RemoveAll();
		m_iUndo = 0;

		return	true;
	}
	else
		return	false;
}

bool
CMemoView::SaveFile( CString strFile )
{
	CWaitCursor	wc;

	CFile	f;
	if	( f.Open( strFile, CFile::modeCreate | CFile::modeWrite ) ){
		CString	strLines = m_strLines;
		if	( m_cbEOL == 1 )
			strLines.Replace( _T("\r\n"), _T("\n") );

		BYTE*	pbData = NULL;
		DWORD	cbData = 0;

		// ASCII or Shift JIS encoded: Get the image as ANSI text.

		if	( m_eEncode == ASCII || m_eEncode == ShiftJIS ){
			cbData = strLines.GetLength() * sizeof( WCHAR ) +1;
			pbData = new BYTE[cbData];
			memset( pbData, 0, cbData );
			GetWindowTextA( GetSafeHwnd(), (char*)pbData, (int)cbData );
			if	( m_cbEOL == 1 ){
				CStringA strLinesA = (char*)pbData;
				strLinesA.Replace( "\r\n", "\n" );
				memcpy( pbData, strLinesA.GetBuffer(), (size_t)strLinesA.GetLength()+1 );
			}
			cbData = (int)strlen( (char*)pbData );
		}

		// UTF-8 encoded: Get the image as UTF-8 sequence.

		else if	( m_eEncode == UTF8 ){
			WCHAR*	pwch = strLines.GetBuffer();
			cbData =
			::WideCharToMultiByte( CP_UTF8, 0, pwch, -1, NULL,          0,      NULL, NULL );
			pbData = new BYTE[cbData];
			::WideCharToMultiByte( CP_UTF8, 0, pwch, -1, (CHAR*)pbData, cbData, NULL, NULL );
		}

		// UTF-16 Little Endian encoded: Get the image as wide character string.

		else if	( m_eEncode == UTF16LE ){
			cbData = strLines.GetLength() * sizeof( WCHAR );
			pbData = new BYTE[cbData];
			memcpy( pbData, strLines.GetBuffer(), cbData );
		}

		// UTF-16 Big Endian encoded: Get the image and swap bytes.

		else if	( m_eEncode == UTF16BE ){
			cbData = strLines.GetLength() * sizeof( WCHAR );
			pbData = new BYTE[cbData];
			memcpy( pbData, strLines.GetBuffer(), cbData );
			for	( DWORD i = 0; i < cbData; i += 2 ){
				BYTE	b = pbData[i+0];
				pbData[i+0] = pbData[i+1];
				pbData[i+1] = b;
			}
		}

		// If BOMs are attached, insert them on the top of the image.

		if	( m_cbBOM ){
			DWORD	cbDataWithBOM = cbData + m_cbBOM;
			BYTE*	pbDataWithBOM = new BYTE[cbDataWithBOM];
			memcpy( pbDataWithBOM,         &m_dwBOM, m_cbBOM );
			memcpy( pbDataWithBOM+m_cbBOM, pbData,   cbData );
			delete[]	pbData;
			pbData = pbDataWithBOM;
			cbData = cbDataWithBOM;
		}

		// Write the image into the specified file.

		f.Write( pbData, cbData );
		f.Close();
		GetTextEncode( pbData, cbData );
		delete[]	pbData;

		return	true;
	}
	else{
		CString	str;
		str.Format( _T("The file could not be opened.\nRead only?") );
		AfxMessageBox( str, MB_ICONERROR );
		return	false;
	}
}

bool
CMemoView::ConfirmDiscard( void )
{
	// With unsaved modified file: Warn it.

	if	( GetModify() && !m_bDiscardModified ){
		CString	strQuestion;
		if	( m_strFile.IsEmpty() )
			strQuestion.Format( _T("Do you want to save this new document?") );
		else{
			CString	strFile = m_strFile;
			int	x = strFile.ReverseFind( '\\' );
			strFile.Delete( 0, x+1 );
			strQuestion.Format( _T("Do you want to save changes to %s?"), strFile.GetBuffer() );
		}
		
		int	iAnswer = AfxMessageBox( strQuestion, MB_ICONEXCLAMATION | MB_YESNOCANCEL | MB_DEFBUTTON3, 0 );

		if	( iAnswer == IDCANCEL )
			return	false;

		else if	( iAnswer == IDYES )
			OnFileSave();
	}

	return	true;
}

DWORD
CMemoView::GetSizeOnFile( CString strFile, bool bModified )
{
	// If the file is not modified, take size of the file.

	if	( !bModified )
		if	( strFile.IsEmpty() )
			return	0;
		else{
			CFile	f( strFile, CFile::modeRead );
			return	(DWORD)f.GetLength();
		}

	// If the file is modified, calculate the size.

	CString	strLines = m_strLines;

	if	( m_cbEOL == 1 )
		strLines.Replace( _T("\r\n"), _T("\n") );

	int	cbData = 0;

	if	( m_eEncode == ASCII || m_eEncode == ShiftJIS ){
		BYTE*	pbData = NULL;
		cbData = strLines.GetLength() * sizeof( WCHAR ) +1;
		pbData = new BYTE[cbData];
		memset( pbData, 0, cbData );
		GetWindowTextA( GetSafeHwnd(), (char*)pbData, (int)cbData );
		cbData = (int)strlen( (char*)pbData );
		delete[]	pbData;
	}
	else if	( m_eEncode == UTF8 ){
		WCHAR*	pwch = strLines.GetBuffer();
		cbData = ::WideCharToMultiByte( CP_UTF8, 0, pwch, -1, NULL, 0, NULL, NULL );
	}
	else if	( m_eEncode == UTF16LE ){
		cbData = strLines.GetLength() * sizeof( WCHAR );
	}
	else if	( m_eEncode == UTF16BE ){
		cbData = strLines.GetLength() * sizeof( WCHAR );
	}

	cbData += m_cbBOM;

	return	cbData;
}

DWORD
CMemoView::GetSizeOnDisk( CString strFile, DWORD cbFile )
{
	if	( !cbFile ){
		if	( strFile.IsEmpty() )
			return	0;

		CFile	f( strFile, CFile::modeRead );
		cbFile = (DWORD)f.GetLength();
		if	( !cbFile )
			return	0;
	}

	DWORD	dwSectorsPerCluster = 0,
		dwBytesPerSector    = 0,
		dwFreeClusters      = 0,
		dwTotalClusters     = 0;

	int	x = strFile.ReverseFind( '\\' );
	CString	strPath = ( x < 0 )? _T(""): strFile.Left( x+1 );
	GetDiskFreeSpace( strPath, &dwSectorsPerCluster, &dwBytesPerSector, &dwFreeClusters, &dwTotalClusters );
	DWORD	cbCluster = dwSectorsPerCluster * dwBytesPerSector;
	DWORD	nCluster = ( cbFile / cbCluster ) + ( ( cbFile % cbCluster )? 1: 0 );

	return	nCluster * cbCluster;
}

void
CMemoView::GetTextEncode( BYTE* pbData, QWORD cbData )
{
	BYTE*	pb = pbData;

	// If the data is empty, encoding is unknown.

	if	( cbData == 0 )
		m_eEncode = unknown;

	// If the BOM exists, encoding is written in BOM.

	else if	( pb[0] == 0xef && pb[1] == 0xbb && pb[2] == 0xbf ){
		m_cbBOM = 3;
		memcpy( &m_dwBOM, pb, m_cbBOM );
		m_eEncode = UTF8;
	}
	else if	( pb[0] == 0xff && pb[1] == 0xfe ){
		m_cbBOM = 2;
		memcpy( &m_dwBOM, pb, m_cbBOM );
		m_eEncode = UTF16LE;
	}
	else if	( pb[0] == 0xfe && pb[1] == 0xff ){
		m_cbBOM = 2;
		memcpy( &m_dwBOM, pb, m_cbBOM );
		m_eEncode = UTF16BE;
	}
	else if	( pb[0] == 0x00 && pb[1] == 0x00 && pb[2] == 0xfe && pb[3] == 0xff ){
		m_cbBOM = 4;
		memcpy( &m_dwBOM, pb, m_cbBOM );
		m_eEncode = UTF32BE;
	}
	else if	( pb[0] == 0xff && pb[1] == 0xfe && pb[2] == 0x00 && pb[3] == 0x00 ){
		m_cbBOM = 4;
		memcpy( &m_dwBOM, pb, m_cbBOM );
		m_eEncode = UTF32LE;
	}

	// If there's no BOM, judge encoding from some data.

	else{
		Encode	aeEncode[16] = {};
		int	nEncode = 0;

		for	( QWORD cb = cbData; cb > 0; ){
			if	( *pb < 0x7f ){
				cb--;
				pb++;
			}
			else if	( pb[0] >= 0xc2 && pb[0] <= 0xdf &&
				  pb[1] >= 0x80 && pb[1] <= 0xbf ){
				aeEncode[nEncode++] = UTF8;	// 11bit code
				cb -= 2;
				pb += 2;
			}
			else if	( pb[0] >= 0xe0 && pb[0] <= 0xef &&
				  pb[1] >= 0x80 && pb[1] <= 0xbf &&
				  pb[2] >= 0x80 && pb[2] <= 0xbf ){
				aeEncode[nEncode++] = UTF8;	// 16bit code
				cb -= 3;
				pb += 3;
			}
			else if	( pb[0] >= 0xf0 && pb[0] <= 0xf4 &&
				  pb[1] >= 0x80 && pb[1] <= 0xbf &&
				  pb[2] >= 0x80 && pb[2] <= 0xbf &&
				  pb[3] >= 0x80 && pb[3] <= 0xbf ){
				aeEncode[nEncode++] = UTF8;	// 21bit code
				cb -= 4;
				pb += 4;
			}
			else if	( ( ( pb[0] >= 0x81 && pb[0] <= 0x9f ) ||
				    ( pb[0] >= 0xe0 && pb[0] <= 0xef )    ) &&
				  ( ( pb[1] >= 0x40 && pb[1] <= 0x7e ) ||
				    ( pb[0] >= 0x80 && pb[0] <= 0xfc )    )    ){
				aeEncode[nEncode++] = ShiftJIS;
				cb -= 2;
				pb += 2;
			}
			else{
				aeEncode[nEncode++] = unknown;
				break;
			}
			if	( nEncode >= _countof( aeEncode ) )
				break;
		}
		{
			int	i;
			for	( i = 1; i < nEncode; i++ )
				if	( aeEncode[i] != aeEncode[0] )
					break;	
			m_eEncode =	( nEncode == 0 )?	ASCII:
					( i >= nEncode )?	aeEncode[0]:
								unknown;
		}
	}

	// Get byte count of End-of-line code.

	if	( m_eEncode == unknown )
		m_cbEOL = 0;
	else{
		pb  = pbData;
		for	( QWORD cb = cbData; cb > 0; cb--, pb++ ){
			if	( !m_cbEOL ){
				if	( pb[0] == '\r' && pb[1] == '\n' )
					m_cbEOL = 2;
				else if	( pb[0] == '\r' && pb[1] == '\0' && pb[2] == '\n' && pb[3] == '\0' )
					m_cbEOL = 2;
				else if	( pb[0] == '\0' && pb[1] == '\r' && pb[2] == '\0' && pb[3] == '\n' )
					m_cbEOL = 2;
				else if	( pb[1] == '\n' && pb[0] != '\r' && pb[0] != '\n' )
					m_cbEOL = 1;
			}
			else
				break;
		}
	}

	// Set the indicators.

	{
		CString	str;
		int	i;
		if	( m_eEncode == ASCII )
			i = 0;
		else if	( m_eEncode == ShiftJIS )
			i = 1;
		else if	( m_eEncode == UTF8 )
			i = 2;
		else if	( m_eEncode == UTF16LE )
			i = 4;
		else if	( m_eEncode == UTF16BE )
			i = 5;
		else
			i = 6;
		str = m_apchEncode[i];
		if	( cbData == 0 )
			str = _T("");
		str += _T(" ");
		str.Insert( 0, _T("\t") );
		SetIndicator( IndicatorOf( ID_INDICATOR_CODE ), str.GetBuffer() );
		str =
			( cbData == 0 )?	 _T("Empty"):
			( m_eEncode == unknown )?_T("Binary"):
			( m_cbEOL == 2 )?	 _T("Windows (CRLF)"):
			( m_cbEOL == 1 )?	 _T("UNIX (LF)"):
						 _T("");
		str.Insert( 0, _T("\t") );
		str +=  _T("\t");
		SetIndicator( IndicatorOf( ID_INDICATOR_TYPE ), str.GetBuffer() );
	}
}

void
CMemoView::GetLowestEncode( Encode& eEncode )
{
	// If every character is under 0x7f, ASCII encode is enough.

	int	cch = m_strLines.GetLength();
	int	i;
	for	( i = 0; i < cch; i++ )
		if	( m_strLines[i] > 0x7e )
			break;
	if	( i >= cch ){
		eEncode = ASCII;
		return;
	}

	// Try to convert them all into Shift JIS.

	BYTE	chError = '\xff';
	TCHAR*	pbUTF16 = m_strLines.GetBuffer();
	int	cbSJIS =
	::WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)pbUTF16, -1, NULL, 0, (CHAR*)&chError, NULL );
	BYTE*	pbSJIS = new BYTE[(cbSJIS*2)+1];
	::WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)pbUTF16, -1, (LPSTR)pbSJIS, cbSJIS, (CHAR*)&chError, NULL );
	pbSJIS[cbSJIS*2] = '\0';

	for	( i = 0; i < cbSJIS; i++ )
		if	( pbSJIS[i] == chError )
			break;

	delete[]	pbSJIS;

	// If all characters are not convertable, the text requires UTF8 encode.

	if	( i < cbSJIS )
		eEncode = UTF8;

	// If all characters are convertable, Shift JIS encode is enough.

	else
		eEncode = ShiftJIS;
}

void
CMemoView::SetDefaultEncode( CFileDialog& dlg, Encode eEncode )
{
	dlg.AddComboBox( IDC_COMBO_ENCODE );

	DWORD	dwLowest = ( eEncode == UTF8 )? 2: ( eEncode == ShiftJIS )? 1: 0;
	for	( DWORD dw = dwLowest; dw < 6; dw++ )
		dlg.AddControlItem( IDC_COMBO_ENCODE, dw, m_apchEncode[dw] );

	DWORD	dwDefault;
	if	( m_eEncode == ASCII )
		dwDefault = 0;
	else if	( m_eEncode == ShiftJIS )
		dwDefault = 1;
	else if	( m_eEncode == UTF8 )
		dwDefault = 2;
	else if	( m_eEncode == UTF16LE )
		dwDefault = 4;
	else if	( m_eEncode == UTF16BE )
		dwDefault = 5;
	else
		dwDefault = 0;

	dlg.SetSelectedControlItem( IDC_COMBO_ENCODE, dwDefault );
}

void
CMemoView::GetSpecifiedEncode( CFileDialog& dlg )
{
	DWORD	dwEncode = 0;
	dlg.GetSelectedControlItem( IDC_COMBO_ENCODE, dwEncode );

	if	( dwEncode == 0 ){
		m_eEncode = ASCII;
		m_cbBOM = 0;
		m_dwBOM = 0;
	}
	else if	( dwEncode == 1 ){
		m_eEncode = ShiftJIS;
		m_cbBOM = 0;
		m_dwBOM = 0;
	}
	else if	( dwEncode == 2 ){
		m_eEncode = UTF8;
		m_cbBOM = 0;
		m_dwBOM = 0;
	}
	else if	( dwEncode == 3 ){
		m_eEncode = UTF8;
		m_cbBOM = 3;
		m_dwBOM = 0xbfbbef;
	}
	else if	( dwEncode == 4 ){
		m_eEncode = UTF16LE;
		m_cbBOM = 2;
		m_dwBOM = 0xfeff;
	}
	else if	( dwEncode == 5 ){
		m_eEncode = UTF16BE;
		m_cbBOM = 2;
		m_dwBOM = 0xfffe;
	}
}

void
CMemoView::SetTitle( CString strFile )
{
	int	x = strFile.ReverseFind( '\\' );
	if	( x >= 0 )
		strFile = strFile.Mid( x+1 );

	CString	strTitle;
	(void)strTitle.LoadString( IDR_MAINFRAME );
	strTitle.Insert( 0, _T(" - ") );
	strTitle.Insert( 0, strFile );
	AfxGetMainWnd()->SetWindowText( strTitle );
}

void
CMemoView::RenewTitle( void )
{
	// Get current window title.

	CString	strTitle;
	AfxGetMainWnd()->GetWindowText( strTitle );

	// Add or remove modified mark of '*' in window title.

	if	( m_strLines == m_strLinesOrg &&
		  strTitle[0] == '*' ){
		strTitle.Delete( 0, 1 );
		SetModify( FALSE );
	}
	else if	( m_strLines != m_strLinesOrg &&
		  strTitle[0] != '*' ){
		strTitle.Insert( 0, '*' );
		SetModify( TRUE );
	}
	else
		return;

	// Set window title.

	AfxGetMainWnd()->SetWindowText( strTitle );
}

void
CMemoView::TakeDiff( void )
{
	if	( m_bNoDiff )
		return;

	int	nchNew = m_strLines    .GetLength();
	int	nchOld = m_strLinesLast.GetLength();
	int	iFore;

	// Seek the first difference from top.

	for	( iFore = 0; iFore < nchNew; iFore++ ){
		if	( iFore >= nchOld )
			break;
		if	( m_strLines[iFore] != m_strLinesLast[iFore] )
			break;
	}

	// Seek the first difference from bottom.

	int	iBackNew = nchNew-1;
	int	iBackOld = nchOld-1;
	for	( ; iBackNew >= 0 && iBackOld >= 0; iBackNew--, iBackOld-- ){
		if	( m_strLines[iBackNew] != m_strLinesLast[iBackOld] )
			break;
	}

	CString	strDiff;
	TCHAR	chMode = 0;

	// The difference of newer text is after older text: The action is 'Insert'.

	if	( iBackNew > iBackOld ){
		int	nch = iBackNew - iBackOld;
		chMode = 'I';
		strDiff = m_strLines.Mid( iFore, nch );
	}

	// Or the action is 'Delete'.

	else{
		int	nch = iBackOld - iBackNew;
		chMode = 'D';
		strDiff = m_strLinesLast.Mid( iFore, nch );
	}

	// The action was taken after some Redo: Cut Redo actions.

	{
		INT_PTR	nUndo = m_aUndo.GetCount();
		if	( m_iUndo < nUndo )
			m_aUndo.SetSize( m_iUndo );
	}

	do{
		if	( chMode == 'I' ){
			INT_PTR	iLast = m_aUndo.GetUpperBound();
			if	( iLast >= 0 ){
				CUndo&	undo = m_aUndo[iLast];

				if	( undo.m_iChar == iFore ){

					// Insert after Delete to the same column: Merge them into one Replace.

					if	( undo.m_strText[0] == 'D' ){
						undo.m_strText.Delete( 0, 1 );
						undo.m_strText.Insert( 0, ( m_bReplaceAll && m_nFound > 1 ) ? _T("r"): _T("R") );
						undo.m_strText += _T("\b");
						undo.m_strText += strDiff;
						break;
					}
				}

				else{
					// Get length of preceeding Insert.

					int	x = undo.m_strText.ReverseFind( '\b' );
					if	( x >= 0 )
						x = undo.m_strText.GetLength() - x - 1;
					else
						x = undo.m_strText.GetLength() - 1;

					// Continued Insert after the same column: Merge it into one operation.

					if	( undo.m_iChar + x == iFore ){
						undo.m_strText += strDiff;
						break;
					}
				}
			}
		}

		// Not merged, Add as a new action.

		{
			CUndo	undo;
			undo.m_iChar = iFore;
			undo.m_strText = chMode + strDiff;
			m_aUndo.Add( undo );
		}
	}while	( false );

	m_iUndo = m_aUndo.GetCount();
}

BOOL
CMemoView::CanUndo( void )
{
	return	m_iUndo > 0? TRUE: FALSE;
}

BOOL
CMemoView::CanRedo( void )
{
	return	m_iUndo < m_aUndo.GetCount()? TRUE: FALSE;
}

void
CMemoView::Undo( void )
{
	if	( !CanUndo() )
		return;

	// Set the falg to avoid retaking the difference.

	m_bNoDiff = true;

	{
		// Take the last action.

		INT_PTR	i = --m_iUndo;
		TCHAR	chMode  = m_aUndo[i].m_strText[0];
		CString	strText = m_aUndo[i].m_strText.Mid( 1 );
		int	iChar   = m_aUndo[i].m_iChar;
		int	iEnd    = iChar;

		// The action was 'Insert', delete it.

		if	( chMode == 'I' ){
			iEnd += strText.GetLength();
			SetSel( iChar, iEnd, FALSE );
			ReplaceSel( _T(""), FALSE );
		}

		// The action was 'Delete', insert it.

		else if	( chMode == 'D' ){
			iEnd += strText.GetLength();
			SetSel( iChar, iChar, FALSE );
			ReplaceSel( strText, FALSE );
			SetSel( iChar, iEnd, FALSE );
		}

		// The action was 'Replace', exchange it.

		else if	( chMode == 'R' ||
			  chMode == 'r'    ){
			int	x = strText.Find( '\b' );
			CString	strNew = strText.Mid( x+1 );
			strText = strText.Left( x );
			iEnd += strNew.GetLength();
			SetSel( iChar, iEnd, FALSE );
			ReplaceSel( strText, FALSE );
			SetSel( iChar, iChar+strText.GetLength(), FALSE );

			if	( chMode == 'r' )
				SetTimer( TID_UNDOALL, m_nRepeatTime, NULL );
		}
	}

	// Clear the falg to avoid retaking the difference.

	m_bNoDiff = false;

	// Just in case file image is left from / returned to the original, renew '*' of title.

	RenewTitle();
}

void
CMemoView::Redo( void )
{
	if	( !CanRedo() )
		return;

	// Set the falg to avoid retaking the difference.

	m_bNoDiff = true;

	{
		// Take the last Undo.

		INT_PTR	i = m_iUndo++;
		TCHAR	chMode  = m_aUndo[i].m_strText[0];
		CString	strText = m_aUndo[i].m_strText.Mid( 1 );
		int	iChar   = m_aUndo[i].m_iChar;
		int	iEnd    = iChar;

		// The action was 'Insert', do it again.

		if	( chMode == 'I' ){
			iEnd += strText.GetLength();
			SetSel( iChar, iChar, FALSE );
			ReplaceSel( strText, FALSE );
			SetSel( iChar, iEnd, FALSE );
		}

		// The action was 'Delete', do it again.

		else if	( chMode == 'D' ){
			iEnd += strText.GetLength();
			SetSel( iChar, iEnd, FALSE );
			ReplaceSel( _T(""), FALSE );
		}

		// The action was 'Replace', do it again.

		else if	( chMode == 'R' ||
			  chMode == 'r'    ){
			int	x = strText.Find( '\b' );
			CString	strNew = strText.Mid( x+1 );
			strText = strText.Left( x );
			iEnd += strText.GetLength();
			SetSel( iChar, iEnd, FALSE );
			ReplaceSel( strNew, FALSE );
			SetSel( iChar, iChar+strNew.GetLength(), FALSE );

			if	( i < m_aUndo.GetUpperBound() &&
				  m_aUndo[i+1].m_strText[0] == 'r' )
				SetTimer( TID_REDOALL, m_nRepeatTime, NULL );
		}
	}

	// Clear the falg to avoid retaking the difference.

	m_bNoDiff = false;

	// Just in case file image is left from / returned to the original, renew '*' of title.

	RenewTitle();
}

void
CMemoView::SelectWord( void )
{
	int	xStart, xEnd;
	GetSel( xStart, xEnd );

	UINT	uSel = TypeOfChar( m_strLines[xStart] );
	if	( uSel <= 0 )
		if	( m_strLines[xStart] == '\t' )
			;
		else if	( m_strLines[xStart] == '\r' )
			;
		else if	( m_strLines[xStart] == '\n' )
			;
		else if	( xStart > 0 )
			if	( TypeOfChar( m_strLines[xStart-1] ) > uSel )
				uSel = TypeOfChar( m_strLines[--xStart] );

	int	xSel = xStart;
	for	( ;; )
		if	( xStart <= 0 )
			break;
		else if	( TypeOfChar( m_strLines[xStart-1] ) == uSel )
			xStart--;
		else
			break;

	if	( m_strLines[xSel] == '\r' || m_strLines[xSel] == '\n' )
		;
	else
		for	( ;; )
			if	( xEnd >= m_strLines.GetLength()-1 )
				break;
			else if	( TypeOfChar( m_strLines[xEnd+1] ) == uSel )
				xEnd++;
			else
				break;

	SetSel( xStart, xEnd+1 );
}

UINT
CMemoView::TypeOfChar( TCHAR ch )
{
	if	( ch <= ' ' )
		return	0;
	else if	( ch <  '0' )
		return	1;
	else if( ch >= ':' &&
		 ch <= '@' )
		return	1;
	else if( ch >= '[' &&
		 ch <= '`' )
		return	1;
	else if( ch >= '{' &&
		 ch <= '~' )
		return	1;
	else{
		WORD	wType = 0;
		GetStringTypeW( CT_CTYPE1, &ch, 1, &wType );

		if	( wType & C1_SPACE )
			return	0;
		else if	( wType & C1_BLANK )
			return	0;
		else if	( wType & C1_CNTRL )
			return	0;
		else if	( wType & C1_PUNCT )
			return	1;
		else
			return	2;
	}
}

void
CMemoView::SetFont( LOGFONT* plf )
{
	plf->lfQuality = CLEARTYPE_QUALITY;

	m_font.DeleteObject();
	m_font.CreateFontIndirect( plf );
	CEdit::SetFont( &m_font );
	Invalidate( TRUE );

	int	n = m_nFontHeight;
	n *= 100;
	n /= m_nFontHeightOrg;
	CString	str;
	str.Format( _T("\t%d%%"), n );
	SetIndicator( IndicatorOf( ID_INDICATOR_ZOOM ), str.GetBuffer() );
}

CString
CMemoView::GetSelected( void )
{
	CString	strFind;

	int	xStart, xEnd;
	GetSel( xStart, xEnd );
	if	( xEnd - xStart > 0 )
		strFind = m_strLines.Mid( xStart, ( xEnd - xStart ) );

	return	strFind;
}

bool
CMemoView::IsPasteable( void )
{
	if	( IsClipboardFormatAvailable( CF_TEXT ) )
		return	true;

	if	( IsClipboardFormatAvailable( CF_OEMTEXT ) )
		return	true;

	if	( IsClipboardFormatAvailable( CF_UNICODETEXT ) )
		return	true;

	return	false;
}

CString
CMemoView::CommaDigitsOf( int nValue )
{
	CString	str;

	str.Format( L"%u", nValue );
	int	n = str.GetLength();
	for	( int i = n-3; i > 0; i -= 3 )
		str.Insert( i, L"," );

	return	str;
}

#include <Winspool.h>

bool
CMemoView::Print( CPrintParam& param )
{
	CWaitCursor	wc;

	bool	bDone = false;

	LPTSTR	szPrinter = param.m_strPrinter.GetBuffer();
	HANDLE	hPrinter = NULL;
	if	( !OpenPrinter( szPrinter, (LPHANDLE)&hPrinter, NULL ) )
		return	bDone;

	DWORD	cbProp = DocumentProperties( NULL, hPrinter, szPrinter, NULL, NULL, 0 );
	HGLOBAL	hDevMode = GlobalAlloc( GHND, cbProp );

	if	( hDevMode ){
		DEVMODE* pDevMode = (DEVMODE*)GlobalLock( hDevMode );
		if	( pDevMode ){
			memset( pDevMode, 0, sizeof( *pDevMode ) );

			LONG	lFlag = DocumentProperties( NULL, hPrinter, szPrinter, pDevMode, NULL, DM_OUT_BUFFER );

			pDevMode->dmPaperSize	= param.m_iPaperSize;
			pDevMode->dmOrientation	= param.m_bLandscape? DMORIENT_LANDSCAPE: DMORIENT_PORTRAIT;

			CDC	dc;
			if	( dc.CreateDC( _T("WINSPOOL"), szPrinter, NULL, pDevMode ) ){
				CString	strJob = MakeJobName();
				DOCINFO	di = {};
				di.cbSize = sizeof( di );
				di.lpszDocName  = strJob.GetBuffer();
				di.lpszOutput   = NULL;
				di.lpszDatatype = NULL;
				di.fwType       = 0;

				//NOTE:
				// StartDoc throws unfounded exception in debug session with VS2022.
				// ( https://github.com/wxWidgets/wxWidgets/issues/23850 )
				// Turn off C++ Exceptions / winrt::hresult_error by Exeption Settings.
				// ( Debug -> Windows -> Exception Settings )
				// And turn off [rethrow].
				// ( https://developercommunity.visualstudio.com/t/Cannot-catch-C-exception/1681838 )

				int	iJob = dc.StartDoc( &di );

				if	( iJob > 0 )
					PrintContent( &dc, param );

				dc.DeleteDC ();
				bDone = true;
			}
			GlobalUnlock( hDevMode );
		}
		GlobalFree( hDevMode );
	}

	ClosePrinter( hPrinter );

	return	bDone;
}

#define	mm2inch( value )	( MulDiv( value, 1000, 254 ) )

void
CMemoView::PrintContent( CDC* pDC, CPrintParam& param )
{
	// Intialize the device context with inch.

	pDC->SetMapMode( MM_LOENGLISH );
	pDC->SelectObject( m_font.m_hObject );
	pDC->SetTextColor( RGB( 0, 0, 0 ) );
	pDC->SetBkMode( TRANSPARENT );

	// Calculate sizes in inch.

	CRect	rectPage;
	CRect	rectTop;
	CRect	rectHeader,
		rectFooter;
	CPoint	ptLine( 0, 0 );
	{
		LOGFONT	lf = {};
		int	cx =  mm2inch( pDC->GetDeviceCaps( HORZSIZE ) );
		int	cy = -mm2inch( pDC->GetDeviceCaps( VERTSIZE ) );
		LONG	lMarginTop    = mm2inch( param.m_nMarginT );
		LONG	lMarginBottom = mm2inch( param.m_nMarginB );
		m_font.GetLogFont( &lf );

		rectPage.SetRect( 0, cy, cx, 0 );
		rectPage.left  += mm2inch( param.m_nMarginL );
		rectPage.right -= mm2inch( param.m_nMarginR );

		rectHeader = rectPage;
		rectFooter = rectPage;
		rectFooter.bottom = rectFooter.top    + lMarginTop;
		rectHeader.top    = rectHeader.bottom - lMarginBottom;

		rectPage.top    += lMarginTop;
		rectPage.bottom -= lMarginBottom;

		CSize	size = pDC->GetTextExtent( _T("W") );
		ptLine.Offset( 0, -size.cy );
		rectTop.SetRect( rectPage.left, rectPage.bottom, rectPage.right, rectPage.bottom + ptLine.y );
	}

	// Print out all lines.

	CString	strLines = m_strLines;
	strLines.Replace( _T("\r\r\n" ), _T("\r\n" ) );
	strLines += _T("\r\n");

	UINT	uPage = 0;
	int	xStart = 0, xEnd = 0;
	CString	strBroken;

	while	( xEnd >= 0 ){

		// Start a page.

		bool	bPageToPrint = IsPageToPrint( ++uPage, param.m_uaPages );
		if	( bPageToPrint ){
			pDC->StartPage();
			PrintMargin( pDC, rectHeader, param.m_strHeader, -(int)uPage );
		}

		CRect	rectLine = rectTop;

		while	( xEnd >= 0 ){

			CString	strLine;

			// For the original lines:
			// Get a line terminated with CR/LF.
			// ( and just in case single LF, not CR/LF, is left, remove it. )

			if	( strBroken.IsEmpty() ){
				xEnd = strLines.Find( _T("\r\n"), xStart );
				strLine = strLines.Mid( xStart, xEnd-xStart );
				strLine.Remove( '\n' );
			}

			// For the page-broken lines:
			// Get a line out of the broken lines.
			// ( and just after the broken lines has done, return to the original lines. )

			else{
				int	x = strBroken.Find( _T("\r\n"), 0 );
				strLine = strBroken.Left( x );
				strBroken.Delete( 0, x+2 );
				if	( strBroken.IsEmpty() )
					xStart = xEnd+2;
			}

			// Calculate the size of the line(s).
			// ( A line in the file may be expanded into multiple lines in DC. )

			CRect	rectDraw = rectLine;
			PrintLine( pDC, rectDraw, strLine, 0, param.m_nTab );

			// When the calculated size get out of page bounds:

			if	( rectDraw.bottom < rectPage.top ){
				if	( strBroken.IsEmpty() )
					strBroken = strLine + _T("\r\n");

				// There's a room for one line at least: Continue to enter devided line(s).

				if	( rectPage.top - rectLine.bottom < ptLine.y )
					continue;

				// There's no room for a line: Skip to the next page.

				else{
					if	( strBroken != strLine + _T("\r\n") )
						strBroken.Insert( 0, strLine + _T("\r\n") );
					break;
				}
			}

			// Print out the line(s).

			if	( bPageToPrint )
				PrintLine( pDC, rectDraw, strLine, ptLine.y, param.m_nTab );

			// Make a space for the empty line.

			if	( strLine.IsEmpty() )
				rectDraw = rectLine;

			// Every single line in a document is done: Break to finish the last page.

			if	( xEnd < 0 )
				break;

			// Skip to the next original line beyond CR/LF.

			if	( strBroken.IsEmpty() )
				xStart = xEnd+2;

			// Make a space for drawn line(s).

			int	nLine = rectDraw.Height() / rectLine.Height();
			for	( int i = 0; i < nLine; i++ )
				rectLine += ptLine;
		}

		// Finish the page.

		if	( bPageToPrint ){
			PrintMargin( pDC, rectFooter, param.m_strFooter, (int)uPage );
			pDC->EndPage();
		}
	}

	pDC->EndDoc();
}

void
CMemoView::PrintLine( CDC* pDC, CRect& rectDraw, CString& strLine, int cyLine, int nTab )
{
	CRect	rectLine = rectDraw;
	UINT	uFormat = DT_TOP | DT_LEFT | DT_NOPREFIX | DT_WORDBREAK;
	CString	strBroken;

	CSize	sizeLine;
	int	cxTab, cxSpace;
	{
		CString	strTab( 'W', nTab );
		CSize	sizeTab = pDC->GetTextExtent( strTab );
		cxTab = sizeTab.cx;
		sizeLine.cy = sizeTab.cy;
		CSize	sizeSpace = pDC->GetTextExtent( _T(" ") );
		cxSpace = sizeSpace.cx;
	}

	// Calculate the size and break line.

	if	( !cyLine ){

		int	cx = 0;
		CRect	rect;

		// Detab the line ( since DT_TABSTOP cannot be used with DT_CALCRECT ).

		CString	strUnbroken = strLine;
		for	( ;; ){
			int	x = strUnbroken.FindOneOf( _T(" \t") );
			if	( x < 0 )
				break;

			CString	strWord = strUnbroken.Left( x );

			rect = rectLine;
			pDC->DrawText( strWord, &rect, uFormat | DT_CALCRECT );

			if	( cx + rect.Width()  > rectLine.Width() ||
				       rect.Height() < rectLine.Height()   ){
				if	( cx )
					strBroken += _T("\r\n");
				rect = rectLine;
				cx = 0;
				BreakLine( pDC, rect, strWord, cx, uFormat );
			}
			else
				cx += rect.Width();

			strBroken += strWord;
			if	( strUnbroken[x] == '\t' ){
				cx = ( ( cx / cxTab )+1 ) * cxTab;
				strBroken += _T("\t");
				x++;
			}
			else if	( strUnbroken[x] == ' ' ){
				while	( strUnbroken[x] == ' ' ){
					cx += cxSpace;
					strBroken += _T(" ");
					x++;
				}
			}

			strUnbroken.Delete( 0, x );
		}

		// Break the left line.

		rect = rectLine;
		pDC->DrawText( strUnbroken, &rect, uFormat | DT_CALCRECT );

		if	( cx + rect.Width()  > rectLine.Width() ||
			       rect.Height() < rectLine.Height()   ){
			if	( cx )
				strBroken += _T("\r\n");
			rect = rectLine;
			cx = 0;
			BreakLine( pDC, rect, strUnbroken, cx, uFormat );
		}
		else
			cx += rect.Width();

		// Get size of broken line(s).

		strLine = strBroken + strUnbroken;
		rectDraw = rectLine;
		pDC->DrawText( strLine, &rectDraw, uFormat | DT_CALCRECT );
		rectDraw.right = rectLine.right;
	}

	// Draw broken line(s) into the calculated rectangle.

	else{
		CString	strBroken = strLine;
		int	xStart = 0;
		while	( xStart >= 0 ){
			CString	strRow = strBroken.Tokenize( _T("\r\n"), xStart );
			if	( strRow.IsEmpty() )
				break;

			// Print a row in the line(s).

			CRect	rect;
			int	cx = 0;
			for	( int x = 0; ( x = strRow.Find( '\t', 0 ) ) >= 0; ){

				// Print words before tab.

				CString	strTabbed = strRow.Left( x );

				rect = rectLine;
				rect.left += cx;
				pDC->DrawText( strTabbed, &rect, uFormat );
				pDC->DrawText( strTabbed, &rect, uFormat | DT_CALCRECT );

				// Skip tab width.

				cx += rect.Width();
				cx = ( ( cx / cxTab )+1 ) * cxTab;
				x++;

				strRow.Delete( 0, x );
			}

			// Print words out of tab.

			rect = rectLine;
			rect.left += cx;
			pDC->DrawText( strRow, &rect, uFormat );
			pDC->DrawText( strRow, &rect, uFormat | DT_CALCRECT );

			// Shift to the next row.

			rectLine.top += cyLine;
		}
	}
}

void
CMemoView::BreakLine( CDC* pDC, CRect& rectLine, CString& strLine, int& cx, UINT uFormat )
{
	CRect	rectDraw = rectLine;
	CString	strFull, strBroken;
	int	cxLast = 0;

	while	( !strLine.IsEmpty() ){
		for	( int i = 0; ; i++ ){
			strFull = strLine.Left( i );
			rectDraw = rectLine;
			rectDraw.left += cx;

			pDC->DrawText( strFull, &rectDraw, uFormat | DT_CALCRECT );

			if	( rectDraw.Width()  > rectLine.Width() ||
				  rectDraw.Height() < rectLine.Height() ){							 
				strFull = strLine.Left( --i );
				strLine.Delete( 0, i );
				strBroken += strFull + _T("\r\n");
				break;
			}
			else if	( i == strLine.GetLength() ){
				strBroken += strLine;
				rectDraw = rectLine;
				pDC->DrawText( strLine, &rectDraw, uFormat | DT_CALCRECT );
				cx = rectDraw.Width();
				strLine.Empty();
				break;
			}
		}
	}

	strLine = strBroken;
	pDC->DrawText( strLine, &rectLine, uFormat | DT_CALCRECT );
}

void
CMemoView::PrintMargin( CDC* pDC, CRect rectMargin, CString strMargin, int nPage )
{
	if	( strMargin.IsEmpty() )
		return;

	UINT	uFormat = DT_SINGLELINE | DT_VCENTER;
	bool	bHeader = false;

	if	( nPage < 0 ){
		nPage = -nPage;
		bHeader = true;
	}

	if	( strMargin.Left( 2 ) == _T("&l") ){
		strMargin.Delete( 0, 2 );
		uFormat |= DT_LEFT;
	}
	else if	( strMargin.Left( 2 ) == _T("&c") ){
		strMargin.Delete( 0, 2 );
		uFormat |= DT_CENTER;
	}
	else if	( strMargin.Left( 2 ) == _T("&r") ){
		strMargin.Delete( 0, 2 );
		uFormat |= DT_RIGHT;
	}
	else{
		uFormat |= DT_CENTER;
	}

	CString	str;
	int	x;

	x = strMargin.Find( _T("&d") );
	if	( x >= 0 ){
		strMargin.Delete( x, 2 );
		CTime	t = CTime::GetCurrentTime();
		str = t.Format( _T("%Y/%m/%d") );
		strMargin.Insert( x, str );
	}
	x = strMargin.Find( _T("&t") );
	if	( x >= 0 ){
		strMargin.Delete( x, 2 );
		CTime	t = CTime::GetCurrentTime();
		str = t.Format( _T("%H:%M:%S") );
		strMargin.Insert( x, str );
	}
	x = strMargin.Find( _T("&f") );
	if	( x >= 0 ){
		strMargin.Delete( x, 2 );
		int	i = m_strFile.ReverseFind( '\\' );
		str = m_strFile.Mid( i+1 );
		strMargin.Insert( x, str );
	}
	x = strMargin.Find( _T("&p") );
	if	( x >= 0 ){
		strMargin.Delete( x, 2 );
		str.Format( _T("%d"), nPage );
		strMargin.Insert( x, str );
	}
	x = strMargin.Find( _T("&b") );
	if	( x >= 0 ){
		strMargin.Delete( x, 2 );
		int	y = rectMargin.bottom-rectMargin.Height()/2;
		if	( bHeader )
			y -= rectMargin.Height()/4;
		else
			y += rectMargin.Height()/4;
		pDC->MoveTo( rectMargin.left,  y );
		pDC->LineTo( rectMargin.right, y );
	}

	pDC->DrawText( strMargin, &rectMargin, uFormat );
}

bool
CMemoView::IsPageToPrint( UINT uPage, CUIntArray& uaPages )
{
	INT_PTR	n = uaPages.GetCount();
	if	( !n )
		return	true;

	for	( INT_PTR i = 0; i < n; i++ )
		if	( uaPages[i] == uPage )
			return	true;

	for	( INT_PTR i = 0; i < n; i++ )
		if	( uaPages[i] == 0 )
			if	( uaPages[i-1] <= uPage &&
				  uaPages[i+1] >= uPage )
				return	true;

	return	false;
}

CString
CMemoView::MakeJobName( void )
{
	int	x = m_strFile.ReverseFind( '\\' );
	CString	strName = m_strFile.Mid( x+1 );
	CString	strUser;
	{
		TCHAR	achUser[UNLEN+1];
		DWORD	cchUser = UNLEN;
		GetUserName( achUser, &cchUser );
		strUser = achUser;
	}
	CString	strHost;
	{
		TCHAR	achHost[MAX_COMPUTERNAME_LENGTH+1];
		DWORD	cchHost = MAX_COMPUTERNAME_LENGTH;
		GetComputerName( achHost, &cchHost );
		strHost = achHost;
	}

	CString	strJob;
	strJob.Format( _T("%s - %s@%s"), strName.GetBuffer(), strUser.GetBuffer(), strHost.GetBuffer() );
	return	strJob;
}