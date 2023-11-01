// MessageDlg.h : header file
//

#pragma once

#include "Dark.h"

class CMessageDlg : public CDialog
{
public:
		CMessageDlg( void );
		void	SetMessage( LPCTSTR pszMessage );

protected:
		CFont	m_font;

		CDark	m_dark;

	virtual	void	OnCancel( void );
	virtual	void	OnOK( void );

	DECLARE_MESSAGE_MAP()

		BOOL	Create( void );
		void	FitSize( LPCTSTR pszMessage );
};
