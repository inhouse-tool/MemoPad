// MemoApp.h : main header file for the MemoPad application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"

class CMemoApp : public CWinApp
{
protected:
	virtual	BOOL	InitInstance( void );
	virtual	int	DoMessageBox( LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt );

	afx_msg	void	OnInfoOnWeb( void );
	afx_msg	void	OnLicense( void );
	afx_msg	void	OnAppAbout( void );
	DECLARE_MESSAGE_MAP()
};

extern	CMemoApp	theApp;
