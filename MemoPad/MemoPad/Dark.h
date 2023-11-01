// Dark.h : interface of the CDark class
//

#pragma once

class CDark
{
public:
		CDark( void );
		~CDark( void );

		void	Initialize( HWND hWnd );
		bool	IsDarkMode( void );
		LRESULT	PopUp( WPARAM wParam, LPARAM lParam );

protected:
		bool	m_bSupported;
		bool	m_bDarken;
		DWORD	m_dwWinVer;
		HMODULE	m_hUX;
		HMODULE	m_hUser;

		DWORD	GetWinVer( void );
		void	GetDarkThemeAPIs( void );
		void	GetDarkThemeSupport( void );
		void	SetDarkThemeEnable( HWND hWnd );
		void	HookScrollBar( void );
	static	HTHEME	WINAPI
			HookedClassProc( HWND hWnd, LPCWSTR pszClassList );
     IMAGE_THUNK_DATA*	FindThunk( void* pBase, char* pchDLL, WORD wOrdinal );

	      ULONG_PTR	m_llGDI;
		HWND	m_hWndClicked;
	    COLORREF	m_crFore,
			m_crBack,
			m_crGray;
	    HBRUSH	m_brBack;
	    HGDIOBJ	m_hPenEdge;
	    HGDIOBJ	m_hPenGray;
	    COLORREF	m_crCombo;
	    HBRUSH	m_brCombo;
	    COLORREF	m_crMenu,
			m_crMenuHot;
	    HBRUSH	m_brMenu,
			m_brMenuHot;
	    HGDIOBJ	m_hFontMenu;
	    COLORREF	m_crEdit;
	    HBRUSH	m_brEdit;
	    HBRUSH	m_brScrollBar;
	    HWND	m_hWndSel;
	    CRect	m_rectSel;

	class CHook
	{//		for each window including controls like buttons, comboboxes...
	public:
		HWND	hWnd;
	SUBCLASSPROC	pfnWnd;
		CHook( void )
		{
			hWnd = NULL;
			pfnWnd = NULL;
		}
	};
		CArray	<CHook, CHook&>
			m_aHook;
	class CState
	{//		for each desktop window like frame windows and dialogs
	public:
		HWND	hWnd;
		DWORD	dwUIState;
		CState( void )
		{
			hWnd = NULL;
			dwUIState = 0;
		}
	};
		CArray	<CState, CState&>
			m_aState;

		void	SetColors( void );
		void	HookWinProc( HWND hWnd );
		void	HookWindow( HWND hWnd, SUBCLASSPROC pfnWnd );
		void	UnhookWindow( HWND hWnd );

	static	LRESULT	OnWindow(       HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef );
		LRESULT	OnMsgWindow(    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static	LRESULT	OnButton(       HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef );
		LRESULT	OnMsgButton(    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
		void	OnPaintButton(  HWND hWnd, HDC hDC, CRect rect );
		void	OnPaintCheckBox(    HWND hWnd, HDC hDC, CRect rect );
		void	OnPaintPushButton(  HWND hWnd, HDC hDC, CRect rect );
		void	OnPaintRadioButton( HWND hWnd, HDC hDC, CRect rect );
		void	OnPaintGroupBox(    HWND hWnd, HDC hDC, CRect rect );
	static	LRESULT	OnComboBox(     HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef );
		LRESULT	OnMsgComboBox(  HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
		void	OnPaintComboBox(    HWND hWnd, HDC hDC, CRect rect );
	static	LRESULT	OnDateTime(     HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef );
		LRESULT	OnMsgDateTime(  HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
		void	OnClickDateTime( HWND hWnd, WPARAM wParam, LPARAM lParam );
	static	LRESULT	OnEdit(         HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef );
		LRESULT	OnMsgEdit(      HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static	LRESULT	OnStatic(       HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef );
		LRESULT	OnMsgStatic(    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
		void	OnPaintStatic(      HWND hWnd, HDC hDC, CRect rect );
	static	LRESULT	OnSpin(         HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef );
		LRESULT	OnMsgSpin(      HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
		void	OnPaintSpin(        HWND hWnd, HDC hDC, CRect rect );
	static	LRESULT	OnStatusBar(    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef );
		LRESULT	OnMsgStatusBar( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
		void	OnPaintStatusBar(   HWND hWnd, HDC hDC, CRect rect );
	static	LRESULT	OnScrollBars(    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef );
		LRESULT	OnMsgScrollBars( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

		LRESULT	OnCtlColorEdit( HWND hWnd, WPARAM wParam );
		void	FillGap( HWND hWnd );

		void	GetWndTabAlt( HWND hWnd, bool& bTab, bool& bAlt );
		DWORD	GetWndUIState( HWND hWnd );
		void	SetWndUIState( HWND hWnd, DWORD dwUIState );

		void	GdipRoundRect( HDC hDC, CRect rect, CPoint ptEdge, COLORREF crInner, COLORREF crEdge );
		void	GdipCircle( HDC hDC, CRect rect, int nThickness, COLORREF crInner, COLORREF crEdge );
		void	GdipCheckMark( HDC hDC, CRect rect, COLORREF crMark );
		void	GdipArrow( HDC hDC, CRect rect, COLORREF crMark, bool bUp );
#ifdef	_DEBUG
		void	TraceMsg( CString strWindow, HWND hWnd, UINT uMsg );
#endif//_DEBUG
};

class CDarkBox : public CDialog
{
public:
		void	SetUp( WPARAM wParam, LPARAM lParam );
		CDarkBox( void );
		~CDarkBox( void );
protected:
		BYTE*	m_pbTemp;
		UINT	m_uDefButton;
		CDark	m_dark;

	virtual	BOOL	OnInitDialog( void );
	virtual	BOOL	OnCommand( WPARAM wParam, LPARAM lParam );

	afx_msg	void	OnShowWindow( BOOL bShow, UINT nStatus );
	DECLARE_MESSAGE_MAP()

		DWORD	FillDialog(  BYTE* pb, DLGTEMPLATE* pTemp, WPARAM wParam, LPARAM lParam );
		DWORD	FillIcon(    BYTE* pb, DLGTEMPLATE* pTemp, WPARAM wParam, LPARAM lParam );
		DWORD	FillText(    BYTE* pb, DLGTEMPLATE* pTemp, WPARAM wParam, LPARAM lParam );
		DWORD	FillButtons( BYTE* pb, DLGTEMPLATE* pTemp, WPARAM wParam, LPARAM lParam );

		int	SelectButtons( UINT uType, UINT* puIdButton, CString* pstrButton );
		LPWSTR	SelectIcon( UINT uIcon );
		BYTE*	Align( BYTE* pb );
};