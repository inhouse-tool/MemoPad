// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include "MemoView.h"
#include "Dark.h"

#define	WM_DARK_SUPPORT	(WM_APP+100)
#define	WM_MESSAGEDLG	(WM_APP+101)
#define	WM_INDICATOR	(WM_APP+102)

#define	SupportDarkMode( hWnd )		AfxGetMainWnd()->SendMessage( WM_DARK_SUPPORT, 0,   (LPARAM)hWnd )
#define	MessageDlg( lpszPrompt, nType )	AfxGetMainWnd()->SendMessage( WM_MESSAGEDLG, nType, (LPARAM)lpszPrompt )
#define	SetIndicator( iPane, lpszText )	AfxGetMainWnd()->SendMessage( WM_INDICATOR,  iPane, (LPARAM)lpszText )
#define	IndicatorOf( n )		(n-ID_INDICATOR_CODE+1)

class	CMainFrame : public CFrameWnd
{
public:
	CMainFrame( void );

protected:
	DECLARE_DYNAMIC( CMainFrame )

		HICON	m_hIcon;
		HACCEL	m_hAccel;

		CDark	m_dark;
	    CMemoView	m_wndView;
	   CStatusBar	m_wndStatusBar;
	   CString	m_strIdle;
		bool	m_bWrap;
		bool	m_bMenu;
		bool	m_bSBar;

	virtual	BOOL	PreCreateWindow( CREATESTRUCT& cs );
	virtual	BOOL	PreTranslateMessage( MSG* pMsg );
	virtual	BOOL	OnCmdMsg( UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo );

	afx_msg	int	OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg	void	OnInitMenuPopup( CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu );
	afx_msg	void	OnSetFocus( CWnd *pOldWnd );
	afx_msg	void	OnSettingChange( UINT uFlags, LPCTSTR lpszSection );
	afx_msg	void	OnSysColorChange( void );
	afx_msg	void	OnSysCommand( UINT nID, LPARAM lParam );
	afx_msg	void	OnViewWordWrap( void );
	afx_msg	void	OnViewMenuBar( void );
	afx_msg	void	OnViewStatusBar( void );
	afx_msg void	OnUpdateCommand( CCmdUI* pCmdUI );

	afx_msg	LRESULT	OnSupportDarkMode(  WPARAM wParam, LPARAM lParam );
	afx_msg	LRESULT	OnMessageDlg( WPARAM wParam, LPARAM lParam );
	afx_msg	LRESULT	OnIndicator( WPARAM wParam, LPARAM lParam );
	DECLARE_MESSAGE_MAP()

		void	CreateClient( void );
};
