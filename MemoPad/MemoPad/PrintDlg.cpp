// PrintDlg.cpp : implementation file
//

#include "pch.h"
#include "PrintDlg.h"
#include "MainFrm.h"
#include "resource.h"

#ifdef	UNICODE
#define	strtol		wcstol
#endif//UNICODE

#include <Winspool.h>
#define PRINTER_STATUS_CHECKING		0x40000000
#define PRINTER_STATUS_CONNECTING	0x80000000

IMPLEMENT_DYNAMIC( CPrintDlg, CDialog )

///////////////////////////////////////////////////////////////////////////////////////
// Constructor

CPrintDlg::CPrintDlg( CWnd* pParent )
	: CDialog( IDD_PRINT, pParent )
{
	m_pthCheck = NULL;
	m_hPrinter = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////
// Overridden Functions

BOOL
CPrintDlg::OnInitDialog( void )
{
	// Initialize the dialog.

	CDialog::OnInitDialog();

	SupportDarkMode( GetSafeHwnd() );

	// Load the last settings.

	CWinApp*	pApp = AfxGetApp();

	m_param.m_strPrinter = pApp->GetProfileString( _T("Print"), _T("Printer"),   _T("Microsoft Print to PDF") );
	m_param.m_bInch      = pApp->GetProfileInt(    _T("Print"), _T("Inch"),      0 );
	m_param.m_iPaperSize = pApp->GetProfileInt(    _T("Print"), _T("Size"),      DMPAPER_A4 );
	m_param.m_bLandscape = pApp->GetProfileInt(    _T("Print"), _T("Landscape"), 0 );
	m_param.m_strHeader  = pApp->GetProfileString( _T("Print"), _T("Header"),    _T("") );
	m_param.m_strFooter  = pApp->GetProfileString( _T("Print"), _T("Footer"),    _T("") );
	m_param.m_nMarginL   = pApp->GetProfileInt(    _T("Print"), _T("MarginL"),   20 );
	m_param.m_nMarginR   = pApp->GetProfileInt(    _T("Print"), _T("MarginR"),   20 );
	m_param.m_nMarginT   = pApp->GetProfileInt(    _T("Print"), _T("MarginT"),   25 );
	m_param.m_nMarginB   = pApp->GetProfileInt(    _T("Print"), _T("MarginB"),   25 );
	m_param.m_nTab       = pApp->GetProfileInt(    _T("Print"), _T("Tab"),        8 );

	m_paramOrg = m_param;
	m_strPrinter = m_param.m_strPrinter;
	m_strItem = GetItemFromPath( m_strPrinter );

	// Start the checking thread.

	GetDlgItem( IDOK )->EnableWindow( FALSE );

	m_pthCheck = AfxBeginThread( EnumPrintersTh, (LPVOID)this, 0, CREATE_SUSPENDED );
	m_pthCheck->m_bAutoDelete = TRUE;
	m_pthCheck->ResumeThread();

	// Initialize controls.

	UINT	uid;
	uid = m_param.m_bInch? IDC_RADIO_INCH: IDC_RADIO_MM;
	((CButton*)GetDlgItem( uid ))->SetCheck( BST_CHECKED );
	UpdatePaperSize( true );

	CComboBox*	pCombo;
	pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_SIZE );
	for	( int i = 0; i < pCombo->GetCount(); i++ )
		if	( (DWORD)pCombo->GetItemData( i ) == m_param.m_iPaperSize ){
			pCombo->SetCurSel( i );
			break;
		}

	CFont*	pFont = GetDlgItem( IDC_STATIC_ORIENT )->GetFont();
	LOGFONT	lf = {};
	pFont->GetLogFont( &lf );
	lf.lfHeight /= 2;
	m_fontSmall.CreateFontIndirect( &lf );
	GetDlgItem( IDC_STATIC_ORIENT )->SetFont( &m_fontSmall, TRUE );
	CRect	rect;
	GetDlgItem( IDC_STATIC_ORIENT )->GetWindowRect( &rect );
	ScreenToClient( &rect );
	m_rectPortrait = rect;
	int	dx = rect.Height() - rect.Width();
	rect.top    += dx;
	rect.right  += dx;
	m_rectLandscape = rect;

	uid = m_param.m_bLandscape? IDC_RADIO_LANDSCAPE: IDC_RADIO_PORTRAIT;
	((CButton*)GetDlgItem( uid ))->SetCheck( BST_CHECKED );
	UpdateOrientation();

	((CButton*)GetDlgItem( IDC_RADIO_PAGE_ALL ))->SetCheck( BST_CHECKED );

	return	TRUE;
}

BOOL
CPrintDlg::DestroyWindow( void )
{
#pragma warning( push )
#pragma warning( disable : 6258 )

	if	( m_pthCheck ){
		TerminateThread( m_pthCheck->m_hThread, 0 );
		delete	m_pthCheck;
		m_pthCheck = NULL;
	}
	if	( m_hPrinter ){
		ClosePrinter( m_hPrinter );
		m_hPrinter = NULL;
	}

	return	CDialog::DestroyWindow();

#pragma warning( pop )
}

void
CPrintDlg::OnOK( void )
{
	// When pages are specified, check them first.

	bool	bDone = true;
	if	( ((CButton*)GetDlgItem( IDC_RADIO_PAGE_SELECT ))->GetCheck() == BST_CHECKED ){
		CString	strPages;
		GetDlgItem( IDC_EDIT_PAGES )->GetWindowText( strPages );
		bDone = GetPages( strPages );
	}
	else
		m_param.m_uaPages.RemoveAll();

	// Save current settings.

	CComboBox*	pCombo;
	pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_PRINTERS );
	CString	strItem;
	pCombo->GetLBText( pCombo->GetCurSel(), strItem );
	m_param.m_strPrinter = GetPathFromItem( strItem );

	m_param.m_bInch = ((CButton*)GetDlgItem( IDC_RADIO_INCH ))->GetCheck() == BST_CHECKED;

	pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_SIZE );
	m_param.m_iPaperSize = (DWORD)pCombo->GetItemData( pCombo->GetCurSel() );

	m_param.m_bLandscape = ((CButton*)GetDlgItem( IDC_RADIO_LANDSCAPE ))->GetCheck() == BST_CHECKED;
	
	CWinApp*	pApp = AfxGetApp();

	if	( m_param.m_strPrinter != m_paramOrg.m_strPrinter )
		pApp->WriteProfileString( _T("Print"), _T("Printer"),   m_param.m_strPrinter );
	if	( m_param.m_bInch != m_paramOrg.m_bInch )
		pApp->WriteProfileInt(    _T("Print"), _T("Inch"),      m_param.m_bInch );
	if	( m_param.m_iPaperSize != m_paramOrg.m_iPaperSize )
		pApp->WriteProfileInt(    _T("Print"), _T("Size"),      m_param.m_iPaperSize );
	if	( m_param.m_bLandscape != m_paramOrg.m_bLandscape )
		pApp->WriteProfileInt(    _T("Print"), _T("Landscape"), m_param.m_bLandscape );
	if	( m_param.m_strHeader != m_paramOrg.m_strHeader )
		pApp->WriteProfileString( _T("Print"), _T("Header"),    m_param.m_strHeader );
	if	( m_param.m_strFooter != m_paramOrg.m_strFooter )
		pApp->WriteProfileString( _T("Print"), _T("Footer"),    m_param.m_strFooter );
	if	( m_param.m_nMarginL != m_paramOrg.m_nMarginL )
		pApp->WriteProfileInt(    _T("Print"), _T("MarginL"),   m_param.m_nMarginL );
	if	( m_param.m_nMarginR != m_paramOrg.m_nMarginR )
		pApp->WriteProfileInt(    _T("Print"), _T("MarginR"),   m_param.m_nMarginR );
	if	( m_param.m_nMarginT != m_paramOrg.m_nMarginT )
		pApp->WriteProfileInt(    _T("Print"), _T("MarginT"),   m_param.m_nMarginT );
	if	( m_param.m_nMarginB != m_paramOrg.m_nMarginB )
		pApp->WriteProfileInt(    _T("Print"), _T("MarginB"),   m_param.m_nMarginB );
	if	( m_param.m_nTab != m_paramOrg.m_nTab )
		pApp->WriteProfileInt(    _T("Print"), _T("Tab"),       m_param.m_nTab );

	// Close the dialog if setting is done.

	if	( bDone )
		CDialog::OnOK();
}

BOOL
CPrintDlg::PreTranslateMessage( MSG* pMsg )
{
	if	( pMsg->message == WM_CHAR ){
		UINT	uChar = (UINT)pMsg->wParam;
		if	( uChar == VK_RETURN )
			;
		else if	( uChar == VK_BACK )
			;
		else if( uChar >= '0' && uChar <= '9' )
			;
		else if( uChar == '-' )
			;
		else if( uChar == ' ' || uChar == ',' )
			;
		else
			return	TRUE;
	}

	return	CDialog::PreTranslateMessage( pMsg );
}

///////////////////////////////////////////////////////////////////////////////////////
// Message Handlers

BEGIN_MESSAGE_MAP( CPrintDlg, CDialog )
	ON_CBN_SELCHANGE( IDC_COMBO_PRINTERS, OnSelPrinters )
	ON_EN_SETFOCUS(   IDC_EDIT_PAGES,     OnFocusPages )
	ON_CONTROL_RANGE( BN_CLICKED, IDC_RADIO_MM,       IDC_RADIO_INCH,      OnRadioSize )
	ON_CONTROL_RANGE( BN_CLICKED, IDC_RADIO_PORTRAIT, IDC_RADIO_LANDSCAPE, OnRadioOrientation )
	ON_CBN_SELCHANGE( IDC_COMBO_SIZE,     OnSelSize )
	ON_BN_CLICKED(    IDC_BUTTON_MARGINS, OnClickMargins )
END_MESSAGE_MAP()

void
CPrintDlg::OnSelPrinters( void )
{
	CComboBox*	pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_PRINTERS );
	int	i = pCombo->GetCurSel();

	DWORD	dwStatus = (DWORD)pCombo->GetItemData( i );

	CString	strStatus;
	if	( dwStatus & PRINTER_STATUS_CHECKING )
		strStatus += _T("Checking... ");
	if	( dwStatus & PRINTER_STATUS_CONNECTING )
		strStatus += _T("Connecting... ");
	if	( dwStatus & PRINTER_STATUS_BUSY )
		strStatus += _T("Busy ");
	if	( dwStatus & PRINTER_STATUS_ERROR )
		strStatus += _T("Error ");
	if	( dwStatus & PRINTER_STATUS_INITIALIZING )
		strStatus += _T("Initializing ");
	if	( dwStatus & PRINTER_STATUS_NO_TONER )
		strStatus += _T("No toner ");
	if	( dwStatus & PRINTER_STATUS_OFFLINE )
		strStatus += _T("Offline ");
	if	( dwStatus & PRINTER_STATUS_NOT_AVAILABLE )
		strStatus += _T("Not available ");
	if	( dwStatus & PRINTER_STATUS_PAPER_JAM )
		strStatus += _T("Jammed ");
	if	( dwStatus & PRINTER_STATUS_PAPER_OUT )
		strStatus += _T("Out of paper ");
	if	( dwStatus & PRINTER_STATUS_PRINTING )
		strStatus += _T("Printing ");
	if	( dwStatus & PRINTER_STATUS_TONER_LOW )
		strStatus += _T("Low toner ");
	if	( dwStatus & PRINTER_STATUS_WARMING_UP )
		strStatus += _T("Warming up ");
	if	( dwStatus == 0 )
		strStatus = _T("Ready");

	CStatic*	pStatic = (CStatic*)GetDlgItem( IDC_STATIC_STATUS );
	pStatic->SetWindowText( strStatus );

	GetDlgItem( IDOK )->EnableWindow( dwStatus? FALSE: TRUE );
}

void
CPrintDlg::OnRadioSize( UINT uID )
{
	m_param.m_bInch = uID == IDC_RADIO_INCH;
	if	( m_param.m_bInch == m_paramOrg.m_bInch )
		m_param.m_iPaperSize = m_paramOrg.m_iPaperSize;

	UpdatePaperSize( true );
}

void
CPrintDlg::OnRadioOrientation( UINT uID )
{
	if	( uID == IDC_RADIO_PORTRAIT )
		m_param.m_bLandscape = false;
	else if	( uID == IDC_RADIO_LANDSCAPE )
		m_param.m_bLandscape = true;

	UpdateOrientation();
}

void
CPrintDlg::OnSelSize( void )
{
	UpdatePaperSize( false );
}

#include "MarginDlg.h"

void
CPrintDlg::OnClickMargins( void )
{
	CMarginDlg	dlg;

	dlg.m_param = m_param;

	if	( dlg.DoModal() == IDOK )
		m_param = dlg.m_param;
}

void
CPrintDlg::OnFocusPages( void )
{
	((CButton*)GetDlgItem( IDC_RADIO_PAGE_SELECT ))->SetCheck( BST_CHECKED );
	((CButton*)GetDlgItem( IDC_RADIO_PAGE_ALL )   )->SetCheck( BST_UNCHECKED );
}

///////////////////////////////////////////////////////////////////////////////////////
// Specific Functions

void
CPrintDlg::UpdatePaperSize( bool bWhole )
{
	struct	sizes{
		TCHAR*	pchName;
		TCHAR*	pchSize;
		DWORD	dwCode;
	}
		astmm[] = {
		{ _T("A3"),			_T("297 x 420"),		DMPAPER_A3		},
		{ _T("A4"),			_T("210 x 297"),		DMPAPER_A4		},
		{ _T("A5"),			_T("148 x 210"),		DMPAPER_A5		},
		{ _T("B4 (JIS)"),		_T("250 x 354"),		DMPAPER_B4		},
		{ _T("B5 (JIS)"),		_T("182 x 257"),		DMPAPER_B5		},
	//	{ _T("Quarto"),			_T("215 x 275"),		DMPAPER_QUARTO		},
	//	{ _T("B4 (ISO)"),		_T("250 x 353"),		DMPAPER_ISO_B4		},
		{ _T("Japanese Postcard"),	_T("100 x 148"),		DMPAPER_JAPANESE_POSTCARD	},
	//	{ _T("A4 Transverse"),		_T("210 x 297"),		DMPAPER_A4_TRANSVERSE	},
	//	{ _T("SuperA/SuperA/A4"),	_T("227 x 356"),		DMPAPER_A_PLUS		},
	//	{ _T("SuperB/SuperB/A3"),	_T("305 x 487"),		DMPAPER_B_PLUS		},
	//	{ _T("A4 Plus"),		_T("210 x 330"),		DMPAPER_A4_PLUS		},
	//	{ _T("A5 Transverse"),		_T("148 x 210"),		DMPAPER_A5_TRANSVERSE	},
	//	{ _T("B5 (JIS) Transverse"),	_T("182 x 257"),		DMPAPER_B5_TRANSVERSE	},
	//	{ _T("A3 Extra"),		_T("322 x 445"),		DMPAPER_A3_EXTRA	},
	//	{ _T("A5 Extra"),		_T("174 x 235"),		DMPAPER_A5_EXTRA	},
	//	{ _T("B5 (ISO) Extra"),		_T("201 x 276"),		DMPAPER_B5_EXTRA	},
	//	{ _T("A3 Transverse"),		_T("297 x 420"),		DMPAPER_A3_TRANSVERSE	},
	//	{ _T("A3 Extra Transverse"),	_T("322 x 445"),		DMPAPER_A3_EXTRA_TRANSVERSE	},
		{ NULL,				NULL,				0 }
	},
		astinch[] = {
		{ _T("Letter"),			_T("8\x00bd x 11"),		DMPAPER_LETTER		},
	//	{ _T("Letter Small"),		_T("8\x00bd x 11"),		DMPAPER_LETTERSMALL	},
		{ _T("Tabloid"),		_T("11 x 17"),			DMPAPER_TABLOID		},
	//	{ _T("Ledger"),			_T("17 x 11"),			DMPAPER_LEDGER		},
		{ _T("Legal"),			_T("8\x00bd x 14"),		DMPAPER_LEGAL		},
		{ _T("Statement"),		_T("5\x00bd x 8\x00bd"),	DMPAPER_STATEMENT	},
		{ _T("Executive"),		_T("7\x00bc x 10\x00bd"),	DMPAPER_EXECUTIVE	},
	//	{ _T("Folio"),			_T("8\x00bd x 13"),		DMPAPER_FOLIO		},
	//	{ _T("Note"),			_T("8\x00bd x 11"),		DMPAPER_NOTE		},
	//	{ _T("US Std Fanfold"),		_T("14\x215e x 11"),		DMPAPER_FANFOLD_US	},
	//	{ _T("German Std Fanfold"),	_T("8\x00bd x 12"),		DMPAPER_FANFOLD_STD_GERMAN	},
	//	{ _T("German Legal Fanfold"),	_T("8\x00bd x 13"),		DMPAPER_FANFOLD_LGL_GERMAN	},
	//	{ _T("Letter Extra"),		_T("9\x00bd x 12"),		DMPAPER_LETTER_EXTRA	},
	//	{ _T("Legal Extra"),		_T("9\x00bd x 15"),		DMPAPER_LEGAL_EXTRA	},
	//	{ _T("Tabloid Extra"),		_T("11.69 x 18"),		DMPAPER_TABLOID_EXTRA	},
	//	{ _T("A4 Extra"),		_T("9.27 x 12.69"),		DMPAPER_A4_EXTRA	},
	//	{ _T("Letter Transverse"),	_T("8\x00bd x 11"),		DMPAPER_LETTER_TRANSVERSE	},
	//	{ _T("Letter Extra Transverse"),_T("9\x00bd x 12"),		DMPAPER_LETTER_EXTRA_TRANSVERSE	},
	//	{ _T("Letter Plus"),		_T("8.5 x 12.69"),		DMPAPER_LETTER		},
		{ NULL,				NULL,				0 }
	};

	CComboBox*	pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_SIZE );
	struct	sizes*	pSizes =
		( ((CButton*)GetDlgItem( IDC_RADIO_MM ))->GetCheck() == BST_CHECKED )? astmm: astinch;

	// On change of mm/inch

	if	( bWhole ){
		pCombo->ResetContent();

		bool	bFirst = true;
		for	( int i = 0; pSizes[i].pchName; i++ ){
			pCombo->AddString( pSizes[i].pchName );
			pCombo->SetItemData( i, pSizes[i].dwCode );
			if	( pSizes[i].dwCode == m_param.m_iPaperSize )
				pCombo->SetCurSel( i );
			else if	( bFirst ){
				pCombo->SetCurSel( i );
				bFirst = false;
			}
		}

		UpdatePaperSize( false );
	}

	// On change of paper size

	else{
		int	i = pCombo->GetCurSel();
		DWORD	dwCode = (DWORD)pCombo->GetItemData( i );

		for	( i = 0; pSizes[i].pchName; i++ )
			if	( pSizes[i].dwCode == dwCode )
				break;

		GetDlgItem( IDC_STATIC_SIZE )->SetWindowText( pSizes[i].pchSize );
	}
}

void
CPrintDlg::UpdateOrientation( void )
{
	bool	bPortrait = ((CButton*)GetDlgItem( IDC_RADIO_PORTRAIT ))->GetCheck() == BST_CHECKED;

	CRect*	pRect = bPortrait? &m_rectPortrait: &m_rectLandscape;

	GetDlgItem( IDC_STATIC_ORIENT )->
		SetWindowPos( NULL, pRect->left, pRect->top, pRect->Width(), pRect->Height(), SWP_NOZORDER );

	GetDlgItem( IDC_STATIC_ORIENT )->SetWindowText( bPortrait?
								_T("   ... ...   \n")
								_T(" .. ... . .. \n")
								_T(" . ..... . . \n")
								_T(" ... .. . .. \n")
								_T(" ...         \n")
								_T("         ... \n")
							:
								_T("       ... ...       \n")
								_T(" .. ... . .. . ..... \n")
								_T(" . ... .. . .. ...   \n")
								_T("                 ... \n") );
}

bool
CPrintDlg::GetPages( CString strPages )
{
	m_param.m_uaPages.RemoveAll();

	TCHAR*	pch = strPages.GetBuffer();
	TCHAR*	pchNext;
	bool	bExtent = false;

	while	( *pch ){
		if	( *pch == ' ' )
			pch++;
		else if	( *pch == ',' )
			pch++;
		else if	( *pch == '-' ){
			pch++;
			bExtent = true;
		}
		else if	( isdigit( *pch ) ){
			UINT	uPage = strtol( pch, &pchNext, 10 );
			if	( (int)uPage <= 0 )
				return	false;

			if	( bExtent ){
				bExtent = false;
				INT_PTR	iPage = m_param.m_uaPages.GetUpperBound();
				if	( iPage < 0 )
					return	false;

				UINT	uStart = m_param.m_uaPages[iPage];
				if	( uStart > uPage )
					return	false;

				m_param.m_uaPages.Add( 0 );
			}
			m_param.m_uaPages.Add( uPage );
			pch = pchNext;
		}
		else
			return	false;
	}

	return	true;
}

UINT
CPrintDlg::EnumPrintersTh( LPVOID pParam )
{
	((CPrintDlg*)pParam)-> ListPrinters();
	((CPrintDlg*)pParam)->CheckPrinters();
	((CPrintDlg*)pParam)->m_pthCheck = NULL;

	return	0;	
}

void
CPrintDlg::ListPrinters( void )
{
	CComboBox*	pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_PRINTERS );
	pCombo->ResetContent();

	// Enumerate local printers first, then check remote printers.

	DWORD	adwFlags[2]  = { PRINTER_ENUM_LOCAL,      PRINTER_ENUM_CONNECTIONS };
	DWORD	adwStatus[2] = { PRINTER_STATUS_CHECKING, PRINTER_STATUS_CONNECTING };

	for	( int iTurn = 0; iTurn < 2; iTurn++ ){
		DWORD	dwFlags = adwFlags[iTurn];
		DWORD	cbBuf = 0;
		DWORD	nInfo = 0;

		// Enumerate the printers ( this may take 50s or more just after a reboot of the client ).

		EnumPrinters( dwFlags, NULL, 1, NULL, 0, &cbBuf, &nInfo );

		if	( cbBuf ){
			BYTE*	pbBuf = new BYTE[cbBuf];
			EnumPrinters( dwFlags, NULL, 1, pbBuf, cbBuf, &cbBuf, &nInfo );

			PRINTER_INFO_1*	pInfo = (PRINTER_INFO_1*)pbBuf;

			for	( DWORD iItem = 0; iItem < nInfo; iItem++ ){

				// Add the printer as an item of th combo box.

				CString	strItem = GetItemFromPath( pInfo->pName );
				pCombo->AddString( strItem );

				// Set the status of the printer as 'Connecting...'.

				int	i = GetComboIndexByItemText( pCombo, strItem );
				if	( i >= 0 ){
					pCombo->SetItemData( i, adwStatus[iTurn] );
					if	( strItem == m_strItem ){
						pCombo->SetCurSel( i );
						OnSelPrinters();
					}
				}

				pInfo++;
			}

			delete[]	pbBuf;
		}
	}
}

void
CPrintDlg::CheckPrinters( void )
{
	CComboBox*	pCombo = (CComboBox*)GetDlgItem( IDC_COMBO_PRINTERS );

	// Check local printers first, then check remote printers.

	for	( int iTurn = 0; iTurn < 2; iTurn++ ){

		DWORD	dwToBeChecked = ( iTurn == 0 )? PRINTER_STATUS_CHECKING: PRINTER_STATUS_CONNECTING;
		int	nItem = pCombo->GetCount();

		for	( int iItem = 0; iItem < nItem; iItem++ ){
			if	( pCombo->GetItemData( iItem ) == dwToBeChecked ){

				pCombo->GetLBText( iItem, m_strItem );
				m_strPrinter = GetPathFromItem( m_strItem );

				// Open the printer w/o cache ( this may take 80s or more when the server is down ).

				PRINTER_OPTIONS	opt = {};
				opt.cbSize = sizeof( opt );
				opt.dwFlags = PRINTER_OPTION_NO_CACHE;
				BOOL	bDone = OpenPrinter2( m_strPrinter, &m_hPrinter, NULL, &opt );

				// Get the latest information of the printer.

				if	( bDone ){
					DWORD	cbBuf = 0;
					bDone = GetPrinter( m_hPrinter, 2, NULL,      0, &cbBuf );
					BYTE*	pbBuf = new BYTE[cbBuf];
					bDone = GetPrinter( m_hPrinter, 2, pbBuf, cbBuf, &cbBuf );

					if	( bDone ){
						PRINTER_INFO_2*	pInfo = (PRINTER_INFO_2*)pbBuf;
						int	i = GetComboIndexByItemText( pCombo, m_strItem );
						if	( i >= 0 ){
							pCombo->SetItemData( i, pInfo->Status );
							if	( i == pCombo->GetCurSel() )
								OnSelPrinters();
						}
					}

					delete[]	pbBuf;
				}

				// Close the printer ( if it was opened ).

				if	( m_hPrinter ){
					ClosePrinter( m_hPrinter );
					m_hPrinter = NULL;
				}

				// Set the status of the printer as 'Offline' ( if could not get information of it ).

				if	( !bDone ){
					int	i = GetComboIndexByItemText( pCombo, m_strItem );
					if	( i >= 0 ){
						pCombo->SetItemData( i, PRINTER_STATUS_OFFLINE );
						if	( i == pCombo->GetCurSel() )
							OnSelPrinters();
					}
				}
			}
		}
	}
}

CString
CPrintDlg::GetItemFromPath( CString strPath )
{
	if	( strPath.Left( 2 ) == _T("\\\\") ){
		int	x = strPath.Find( _T("\\"), 2 );
		CString	strServer = strPath.Mid( 2, x-2 );
		CString	strDevice = strPath.Mid( x+1 );
		CString	strItem;
		strItem.Format( _T("%s on %s"), strDevice.GetBuffer(), strServer.GetBuffer() );
		return	strItem;
	}
	else
		return	strPath;
}

CString
CPrintDlg::GetPathFromItem( CString strItem )
{
	int	x = strItem.Find( _T(" on ") );
	if	( x > 0 ){
		CString	strServer = strItem.Mid( x+4 );
		CString	strDevice = strItem.Left( x );
		CString	strPath;
		strPath.Format( _T("\\\\%s\\%s"), strServer.GetBuffer(), strDevice.GetBuffer() );
		return	strPath;
	}
	else
		return	strItem;
}

int
CPrintDlg::GetComboIndexByItemText( CComboBox* pCombo, CString strItem )
{
	for	( int i = 0; i < pCombo->GetCount(); i++ ){
		CString	str;
		pCombo->GetLBText( i, str );
		if	( str == strItem )
			return	i;
	}

	return	-1;
}