// FontDlg.cpp : implementation file
//

#include "pch.h"
#include "resource.h"
#include "FontDlg.h"
#include "MainFrm.h"

#ifdef	UNICODE
#define	strcpy_s	wcscpy_s
#define	atoi		_wtoi
#endif//UNICODE

IMPLEMENT_DYNAMIC( CFontDlg, CDialog )

///////////////////////////////////////////////////////////////////////////////////////
// Constructor

CFontDlg::CFontDlg( void )
{
	m_bAlt = false;
	m_uClicked = 0;

	m_nCharSet = 0;
	m_bPitch   = 0;
	m_nPoint   = 0;

	memset( &m_lf, 0, sizeof( m_lf ) );
}

///////////////////////////////////////////////////////////////////////////////////////
// Overridden Functions

BOOL
CFontDlg::OnInitDialog( void )
{
	CDialog::OnInitDialog();

	InitControls();

	SupportDarkMode( GetSafeHwnd() );

	LoadSettings();

	return	TRUE;
}

void
CFontDlg::OnOK( void )
{
	SaveSettings();

	ShowWindow( SW_HIDE );
}

///////////////////////////////////////////////////////////////////////////////////////
// Message Handlers

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP( CFontDlg, CDialog )
	ON_WM_SHOWWINDOW()
	ON_WM_SYSCOMMAND()
	ON_CBN_SELCHANGE( IDC_COMBO_CHARSET, OnSelectCharset )
	ON_CBN_SELCHANGE( IDC_COMBO_NAME,    OnSelectName )
	ON_CBN_SELCHANGE( IDC_COMBO_SIZE,    OnSelectSize )
	ON_BN_CLICKED( IDC_CHECK_PROPORTIONAL, OnClickWidth )
	ON_BN_CLICKED( IDC_CHECK_FIXED,        OnClickWidth )
END_MESSAGE_MAP()

#pragma warning( pop )

void
CFontDlg::OnShowWindow( BOOL bShow, UINT nStatus )
{
	CDialog::OnShowWindow( bShow, nStatus );

	GotoDlgCtrl( GetDlgItem( IDC_COMBO_NAME ) );
	GetDlgItem( IDC_COMBO_NAME )->SetFocus();
}

void
CFontDlg::OnSysCommand( UINT nID, LPARAM lParam )
{
	if	( nID == SC_CLOSE )
		ShowWindow( SW_HIDE );
	else
		CDialog::OnSysCommand( nID, lParam );
}

void
CFontDlg::OnSelectCharset( void )
{
	EnumFontGroup();
}

void
CFontDlg::OnSelectName( void )
{
	SeekFontByName();
}

void
CFontDlg::OnSelectSize( void )
{
	CComboBox*	pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_SIZE );
	m_lf.lfHeight = (int)pCombo->GetItemData( pCombo->GetCurSel() );

	SelectFont();
}

void
CFontDlg::OnClickWidth( void )
{
	EnumFontGroup();
}

///////////////////////////////////////////////////////////////////////////////////////
// Specific Functions

#define	LOGICALDPI	72	// It's seems to be fixed at 72dpi ( not 96dpi ) for the compatibility with Notepad.exe.

void
CFontDlg::InitControls( void )
{
	CComboBox*	pCombo;

	struct{
		TCHAR*	pchName;
		DWORD	dwData;
	}	astCharset[] = {
		{	_T("ANSI"),		ANSI_CHARSET		},
		{	_T("Baltic"),		BALTIC_CHARSET		},
		{	_T("Chinese"),		CHINESEBIG5_CHARSET	},
		{	_T("East Europe"),	EASTEUROPE_CHARSET	},
		{	_T("GB2312"),		GB2312_CHARSET		},
		{	_T("Greek"),		GREEK_CHARSET		},
		{	_T("Hangul"),		HANGUL_CHARSET		},
		{	_T("Mac"),		MAC_CHARSET		},
		{	_T("OEM"),		OEM_CHARSET		},
		{	_T("Russian"),		RUSSIAN_CHARSET		},
		{	_T("Shift JIS"),	SHIFTJIS_CHARSET	},
		{	_T("Symbol"),		SYMBOL_CHARSET		},
		{	_T("Turkish"),		TURKISH_CHARSET		},
		{	_T("Vietnamese"),	VIETNAMESE_CHARSET	},
		{	_T("Johab"),		JOHAB_CHARSET		},
		{	_T("Arabic"),		ARABIC_CHARSET		},
		{	_T("Hebrew"),		HEBREW_CHARSET		},
		{	_T("Thai"),		THAI_CHARSET		},
		{	NULL,			0			}
	};

	pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_CHARSET );
	for	( int i = 0; astCharset[i].pchName; i++ ){
		pCombo->AddString( astCharset[i].pchName );
		pCombo->SetItemData( i, astCharset[i].dwData );
	}

	static	int	anPoint[] = { 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 74, 96, 0 };

	pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_SIZE );
	HDC	hDC = GetDC()->m_hDC;
	for	( int i = 0; anPoint[i]; i++ ){
		CString	str;
		str.Format( _T("%d"), anPoint[i] );
		pCombo->AddString( str );
		pCombo->SetItemData( i, -MulDiv( anPoint[i], GetDeviceCaps( hDC, LOGPIXELSY ), LOGICALDPI ) );
	}

	GotoDlgCtrl( GetDlgItem( IDC_COMBO_NAME ) );
}

void
CFontDlg::LoadSettings( void )
{
	// Load 'CharSet' and 'Pitch' to filter 'Font name's.

	m_nCharSet = AfxGetApp()->GetProfileInt(     _T("Font"), _T("CharSet"), ANSI_CHARSET );
	m_bPitch   = AfxGetApp()->GetProfileInt(     _T("Font"), _T("Pitch"), FIXED_PITCH );
	((CButton*)GetDlgItem( IDC_CHECK_PROPORTIONAL ))->SetCheck( ( m_bPitch & VARIABLE_PITCH )? BST_CHECKED: BST_UNCHECKED );
	((CButton*)GetDlgItem( IDC_CHECK_FIXED )       )->SetCheck( ( m_bPitch & FIXED_PITCH    )? BST_CHECKED: BST_UNCHECKED );

	int	n;
	CComboBox*	pCombo;
	pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_CHARSET );
	n = pCombo->GetCount();
	for	( int i = 0; i < n; i++ )
		if	( pCombo->GetItemData( i ) == m_nCharSet ){
			pCombo->SetCurSel( i );
			break;
		}

	OnSelectCharset();

	// Load 'Point' and 'Font name' to select one font.

	m_nPoint   = AfxGetApp()->GetProfileInt(     _T("Font"), _T("Point"),   11 );

	pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_SIZE );
	n = pCombo->GetCount();
	for	( int i = 0; i < n; i++ ){
		CString	str;
		pCombo->GetLBText( i, str );
		int	nPt = atoi( str );
		if	( nPt == m_nPoint ){
			pCombo->SetCurSel( i );
			m_lf.lfHeight = (int)pCombo->GetItemData( i );
			break;
		}
	}

	m_strFaceName    = AfxGetApp()->GetProfileString(  _T("Font"), _T("Name"),    _T("Consolas") );
	pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_NAME );
	pCombo->SelectString( 0, m_strFaceName );

	OnSelectName();
}

void
CFontDlg::SaveSettings( void )
{
	CComboBox*	pCombo;

	pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_CHARSET );
	int	i = pCombo->GetCurSel();
	int	nCharSet = (int)pCombo->GetItemData( i );

	BYTE	bPitch = 0;
	if	( ((CButton*)GetDlgItem( IDC_CHECK_PROPORTIONAL ))->GetCheck() == BST_CHECKED )
		bPitch |= VARIABLE_PITCH;
	if	( ((CButton*)GetDlgItem( IDC_CHECK_FIXED )       )->GetCheck() == BST_CHECKED )
		bPitch |= FIXED_PITCH;

	CString	str;
	pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_SIZE );
	pCombo->GetLBText( pCombo->GetCurSel(), str );
	int	nPoint = atoi( str );

	if	( m_nCharSet != nCharSet )
		AfxGetApp()->WriteProfileInt(    _T("Font"), _T("CharSet"), nCharSet );
	if	( m_bPitch != bPitch )
		AfxGetApp()->WriteProfileInt(    _T("Font"), _T("Pitch"),   bPitch );
	if	( m_nPoint != nPoint )
		AfxGetApp()->WriteProfileInt(    _T("Font"), _T("Point"),   nPoint );
	if	( m_strFaceName != m_lf.lfFaceName )
		AfxGetApp()->WriteProfileString( _T("Font"), _T("Name"), m_lf.lfFaceName );
}

void
CFontDlg::EnumFontGroup( void )
{
	CComboBox*	pCombo;

	// Specify a 'CharSet'.

	pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_CHARSET );
	int	iCharSet = pCombo->GetCurSel();
	m_lf.lfCharSet = (BYTE)pCombo->GetItemData( iCharSet );

	// Unspecify 'Font name'.

	m_strFaceName.Empty();
	strcpy_s( m_lf.lfFaceName, m_strFaceName.GetBuffer() );

	// Enumerate the fonts filtered by the condition.

	pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_NAME );
	pCombo->ResetContent();
	EnumFontFamiliesEx( GetDC()->m_hDC, &m_lf, EnumFontProc, (LPARAM)this, 0 );

	// Select the first 'Font name' as default.

	if	( pCombo->GetCount() > 0 ){
		pCombo->SetCurSel( 0 );
		SeekFontByName();
	}
}

void
CFontDlg::SeekFontByName( void )
{
	// Get the selected 'Font name'.

	CComboBox*	pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_NAME );
	int	iName = pCombo->GetCurSel();
	if	( iName >= 0 )
		pCombo->GetLBText( iName, m_strFaceName );

	// Apply the font with 'Font name'.

	if	( !m_strFaceName.IsEmpty() ){
		strcpy_s( m_lf.lfFaceName, m_strFaceName.GetBuffer() );
		EnumFontFamiliesEx( GetDC()->m_hDC, &m_lf, EnumFontProc, (LPARAM)this, 0 );
	}
}

int	CALLBACK
CFontDlg::EnumFontProc( const LOGFONT* plf, const TEXTMETRIC* ptm, DWORD dwType, LPARAM lparam )
{
	BOOL	bContinue = TRUE;
	CFontDlg*	pdlg = (CFontDlg*)lparam;

	if	( plf->lfFaceName[0] == '@' )
		;
	else if	( dwType & TRUETYPE_FONTTYPE )
		bContinue = pdlg->OnEnumFont( plf );

	return	bContinue;
}

BOOL
CFontDlg::OnEnumFont( const LOGFONT* plf )
{
	// When the 'Font name' is not specified, collect the fonts for the specified conditions.

	if	( m_strFaceName.IsEmpty() ){
		BYTE	bPitch = 0;
		if	( ((CButton*)GetDlgItem( IDC_CHECK_PROPORTIONAL ))->GetCheck() == BST_CHECKED )
			bPitch |= VARIABLE_PITCH;
		if	( ((CButton*)GetDlgItem( IDC_CHECK_FIXED )       )->GetCheck() == BST_CHECKED )
			bPitch |= FIXED_PITCH;
		if	( ( plf->lfPitchAndFamily & bPitch ) )
			((CComboBox*)GetDlgItem( IDC_COMBO_NAME ))->AddString( plf->lfFaceName );
		return	TRUE;
	}

	// When the 'Font name' is specified, seek the LOGFONT of the specified font and select it.

	else{
		if	( m_strFaceName == plf->lfFaceName ){
			m_strFaceName.Empty();
			int	nHeight = m_lf.lfHeight;
			int	nWidth  = m_lf.lfWidth;
			*(&m_lf) = *plf;
			m_lf.lfHeight = nHeight;
			m_lf.lfWidth  = 0;
			SelectFont();
			return	FALSE;
		}
		else
			return	TRUE;
	}
}

void
CFontDlg::SelectFont( void )
{
	CWnd*	pwnd = GetOwner();
	if	( pwnd )
		pwnd->PostMessage( WM_FONT, 0, (LPARAM)&m_lf );

	CString	str, strProp;
	str =
		( ( m_lf.lfPitchAndFamily & 0x03 ) == FIXED_PITCH )?	"Fixed width":
		( ( m_lf.lfPitchAndFamily & 0x03 ) == VARIABLE_PITCH )?	"Proportional width":
									"Default width";
	strProp += str;
	strProp += " ";
	str =
		( ( m_lf.lfPitchAndFamily & 0xf0 ) == FF_DECORATIVE )?	"decorative":
		( ( m_lf.lfPitchAndFamily & 0xf0 ) == FF_MODERN )?	"without serifs":
		( ( m_lf.lfPitchAndFamily & 0xf0 ) == FF_ROMAN )?	"with serifs":
		( ( m_lf.lfPitchAndFamily & 0xf0 ) == FF_SCRIPT )?	"script":
		( ( m_lf.lfPitchAndFamily & 0xf0 ) == FF_SWISS )?	"without serifs":
									"default";
	strProp += str;

	GetDlgItem( IDC_STATIC_PROPERTIES )->SetWindowText( strProp );
}
