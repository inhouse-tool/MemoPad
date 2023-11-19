// MarginDlg.h : interface of the CMarginDlg class
//

#pragma once

#include "PrintDlg.h"

class CMarginDlg : public CDialog
{
	DECLARE_DYNAMIC( CMarginDlg )

public:
	CMarginDlg( CWnd* pParent = NULL );

	    CPrintParam	m_param;

protected:

	virtual	BOOL	OnInitDialog( void );
	virtual	void	OnOK( void );
	virtual	BOOL	PreTranslateMessage( MSG* pMsg );

	DECLARE_MESSAGE_MAP()

		int	GetValue( UINT uID );
		void	SetValue( UINT uID, int nValue );
};
