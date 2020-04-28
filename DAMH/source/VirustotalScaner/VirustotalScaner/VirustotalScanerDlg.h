// VirustotalScanerDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "BtnST.h"
#include "ManagerThread.h"

// CVirustotalScanerDlg dialog
class CVirustotalScanerDlg : public CDialog
{
// Construction
public:
	CVirustotalScanerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_VIRUSTOTALSCANER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CListCtrl m_ListCtrl;
	CEdit m_edit_Path;
	CButton	  m_btn_Browser;
	CButtonST m_btn_Setting;
	CButtonST m_btn_UnZip;
	CButtonST m_btn_Help;
	afx_msg void OnBnClickedButton1();
	CButton m_btn_Scan;

	CManagerThread* m_pThreadScan;
};
