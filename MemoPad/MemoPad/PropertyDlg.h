// PropertyDlg.h : interface of the CProperyDlg class
//

#pragma once

class CPropertyDlg : public CDialog
{
	DECLARE_DYNAMIC( CPropertyDlg )

public:
		CPropertyDlg( CWnd* pParent = NULL );

		void	SetFile( CString strFile );
		void	SetText( CString strLines );
		void	SetModified( bool bModified );

protected:
	virtual	BOOL	OnInitDialog( void );

		CString	m_strFile;
		CString	m_strProperties;
		bool	m_bModified;
		CFont	m_fontPath;
		bool	m_bInPath;
		HCURSOR	m_hcArrow,
			m_hcHand;

	afx_msg	BOOL	OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message );
	afx_msg	void	OnSize( UINT nType, int cx, int cy );
	afx_msg	void	OnClickPath( void );
	afx_msg	LRESULT	OnCtlColorStatic( WPARAM wParam, LPARAM lParam );
	DECLARE_MESSAGE_MAP()

		CSize	GetValueSize( UINT uID );
		CPoint	GetUnitPoint( UINT uID );
};
