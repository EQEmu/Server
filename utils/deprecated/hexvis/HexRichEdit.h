#if !defined(AFX_HEXRICHEDIT_H__29F129EE_0086_45A7_8A61_7ACCB6507BCE__INCLUDED_)
#define AFX_HEXRICHEDIT_H__29F129EE_0086_45A7_8A61_7ACCB6507BCE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HexRichEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// HexRichEdit window

class CHexvisDlg;

class HexRichEdit : public CRichEditCtrl
{
// Construction
public:
	HexRichEdit();

// Attributes
public:
	CHexvisDlg *parent;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(HexRichEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~HexRichEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(HexRichEdit)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HEXRICHEDIT_H__29F129EE_0086_45A7_8A61_7ACCB6507BCE__INCLUDED_)
