// MemoView.h : interface of the CMemoView class
//

#pragma once
#include "FindDlg.h"
#include "FontDlg.h"

class	CMemoView : public CEdit
{
public:
	CMemoView( void );

#define	STAT_NOFILE	1
#define	STAT_EMPTY	2
#define	STAT_MODIFIED	4
#define	STAT_UNDOABLE	8
#define	STAT_REDOABLE	16
#define	STAT_SELECTED	32
#define	STAT_PASTEABLE	64
#define	STAT_FOUND	128
		DWORD	GetStatus( void );

protected:
	static	TCHAR*	m_apchEncode[];

	    CFindDlg	m_dlgFind;
	    CFontDlg	m_dlgFont;

		bool	m_bDiscardModified;
	      COLORREF	m_crText,
			m_crBack;
		HBRUSH	m_brBack;
		int	m_nFontHeight,
			m_nFontHeightOrg;
		CFont	m_font;
		CString	m_strFile;
		CString	m_strLines,
			m_strLinesLast,
			m_strLinesOrg;
		bool	m_bInsert;

		DWORD	m_cbBOM;
		DWORD	m_dwBOM;
		enum	Encode{
			unknown,
			ANSI,
			ShiftJIS,
			UTF8,
			UTF16BE,
			UTF16LE,
			UTF32BE,
			UTF32LE
		}	m_eEncode;
		int	m_cbEOL;

		bool	m_bFindUp,
			m_bFindCase,
			m_bReplace,
			m_bReplaceAll;
		int	m_nFound;
		CString	m_strFind,
			m_strReplace;
	class	CUndo{
	public:
			int	iChar;
			CString	strText;
		CUndo( void )
		{
			iChar = 0;
			strText.Empty();
		}
	};
		CArray	<CUndo, CUndo&>
			m_aUndo;
		INT_PTR	m_iUndo;
		bool	m_bUndoing;

	virtual	BOOL	PreTranslateMessage( MSG* pMsg );

	afx_msg	int	OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg	void	OnDestroy( void );
	afx_msg	void	OnDropFiles( HDROP hDropInfo );
	afx_msg	BOOL	OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );
	afx_msg	void	OnRButtonDown( UINT nFlags, CPoint point );
	afx_msg	void	OnTimer( UINT_PTR nIDEvent );
	afx_msg	HBRUSH	CtlColor( CDC* pDC, UINT nCtlColor );

	afx_msg	void	OnFileOpen( void );
	afx_msg	void	OnFileSave( void );
	afx_msg	void	OnFileSaveAs( void );
	afx_msg	void	OnFileClose( void );
	afx_msg	void	OnFileProperties( void );
	afx_msg	void	OnEditUndo( void );
	afx_msg	void	OnEditRedo( void );
	afx_msg	void	OnEditCut( void );
	afx_msg	void	OnEditCopy( void );
	afx_msg	void	OnEditPaste( void );
	afx_msg	void	OnEditClear( void );
	afx_msg	void	OnEditFind( void );
	afx_msg	void	OnEditFindNext( void );
	afx_msg	void	OnEditFindPrev( void );
	afx_msg	void	OnEditReplace( void );
	afx_msg	void	OnEditSelectAll( void );
	afx_msg	void	OnEditInsertUnicode( void );
	afx_msg	void	OnViewFont( void );
	afx_msg	void	OnViewZoomIn( void );
	afx_msg	void	OnViewZoomOut( void );
	afx_msg	void	OnViewZoomRestore( void );
	afx_msg	void	OnEditUpdate( void );
	afx_msg	void	OnEditChange( void );

	afx_msg LRESULT	OnFind( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT	OnFont( WPARAM wParam, LPARAM lParam );
	DECLARE_MESSAGE_MAP()

		bool	LoadFile( CString strFile );
		bool	SaveFile( CString strFile );
		void	ConfirmDiscard( void );
		DWORD	GetSizeOnFile( CString strFile, bool bModified = false );
		DWORD	GetSizeOnDisk( CString strFile, DWORD cbFile = 0 );
		void	GetTextEncode( BYTE* pbText, QWORD cbText );
		void	GetLowestEncode( Encode& eEncode );
		void	SetDefaultEncode( CFileDialog& dlg, Encode eEncode );
		void	GetSpecifiedEncode( CFileDialog& dlg );
		void	SetTitle( CString strFile );
		void	RenewTitle( void );
		void	TakeDiff( void );
		BOOL	CanUndo( void );
		BOOL	CanRedo( void );
		void	Undo( void );
		void	Redo( void );
		void	SetFont( LOGFONT* plf );
		CString	GetSelected( void );
		bool	IsPasteable( void );
		CString	CommaDigitsOf( int nValue );
};
