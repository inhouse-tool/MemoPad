// FindDlg.h : interface of the CProperyDlg class
//

#pragma once

#define	WM_FIND		(WM_APP+200)
// wParam:
#define	FIND_COMMAND_NEXT	1
#define	FIND_COMMAND_REPLACE	2
#define	FIND_COMMAND_ALL	3
// lParam:
typedef	struct	tagFIND{
		bool	bMacthCase;
		bool	bUpward;
		CString	strFindWhat;
		CString	strReplaceWith;
}	FIND;

class CFindDlg : public CDialog
{
	DECLARE_DYNAMIC( CFindDlg )

		CFindDlg( void );

		void	ShowDialog( char chMode, CString strFind );

protected:
		FIND	m_find;
		char	m_chMode;
		bool	m_bAlt;
		UINT	m_uClicked;

	virtual	BOOL	OnInitDialog( void );
	virtual	void	OnOK( void );

	afx_msg	void	OnShowWindow( BOOL bShow, UINT nStatus );
	afx_msg	void	OnSysCommand( UINT nID, LPARAM lParam );
	afx_msg	void	OnTimer( UINT_PTR nIDEvent );
	afx_msg void	OnClickButton( UINT uID );
	DECLARE_MESSAGE_MAP()
};
