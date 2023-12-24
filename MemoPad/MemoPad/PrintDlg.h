// PrintDlg.h : interface of the CPrintDlg class
//

#pragma once

class CPrintParam
{
public:
		CString	m_strPrinter;
		bool	m_bInch;
		int	m_iPaperSize;
		bool	m_bLandscape;
		CString	m_strHeader,
			m_strFooter;
		int	m_nMarginL,
			m_nMarginR,
			m_nMarginT,
			m_nMarginB,
			m_nTab;
	     CUIntArray	m_uaPages;

	     CPrintParam( void )
	     {
			m_bInch      = false;
			m_iPaperSize = 0;
			m_bLandscape = false;
			m_nMarginL   = 0;
			m_nMarginR   = 0;
			m_nMarginT   = 0;
			m_nMarginB   = 0;
			m_nTab = 0;
	     }
	     void	operator = ( const CPrintParam& src )
	     {
			m_strPrinter = src.m_strPrinter;
			m_bInch      = src.m_bInch;
			m_iPaperSize = src.m_iPaperSize;
			m_bLandscape = src.m_bLandscape;
			m_strHeader  = src.m_strHeader;
			m_strFooter  = src.m_strFooter;
			m_nMarginL   = src.m_nMarginL;
			m_nMarginR   = src.m_nMarginR;
			m_nMarginT   = src.m_nMarginT;
			m_nMarginB   = src.m_nMarginB;
			m_nTab       = src.m_nTab;
	     }
};

class CPrintDlg : public CDialog
{
	DECLARE_DYNAMIC( CPrintDlg )

public:
	CPrintDlg( CWnd* pParent = NULL );

	    CPrintParam	m_param;

protected:
	    CPrintParam	m_paramOrg;

		CFont	m_fontSmall;
		CRect	m_rectPortrait,
			m_rectLandscape;

	    CWinThread*	m_pthCheck;
		HANDLE	m_hPrinter;
		CString	m_strPrinter;
		CString	m_strItem;

	virtual	BOOL	OnInitDialog( void );
	virtual	BOOL	DestroyWindow( void );
	virtual	void	OnOK( void );
	virtual	BOOL	PreTranslateMessage( MSG* pMsg );

	afx_msg	void	OnSelPrinters( void );
	afx_msg	void	OnRadioSize( UINT uID );
	afx_msg	void	OnRadioOrientation( UINT uID );
	afx_msg	void	OnSelSize( void );
	afx_msg	void	OnClickMargins( void );
	afx_msg	void	OnFocusPages( void );
	DECLARE_MESSAGE_MAP()

		void	UpdatePaperSize( bool bWhole );
		void	UpdateOrientation( void );
		bool	GetPages( CString strPages );

	static	UINT	EnumPrintersTh( LPVOID pParam );
		void	ListPrinters( void );
		void	CheckPrinters( void );
		CString	GetItemFromPath( CString strPath );
		CString	GetPathFromItem( CString strItem );
		int	GetComboIndexByItemText( CComboBox* pCombo, CString strItem );
};
