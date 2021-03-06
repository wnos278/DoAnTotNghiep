// VirustotalScanerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VirustotalScaner.h"
#include "VirustotalScanerDlg.h"
#include "ManagerThread.h"
#include "SupportMain.h"
#include <Shlwapi.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CVirustotalScanerDlg dialog




CVirustotalScanerDlg::CVirustotalScanerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVirustotalScanerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVirustotalScanerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListCtrl);
	DDX_Control(pDX, IDC_EDIT1, m_edit_Path);
	DDX_Control(pDX, IDC_BUTTON1, m_btn_Browser);
	DDX_Control(pDX, IDC_BUTTON2, m_btn_Setting);
	DDX_Control(pDX, IDC_BUTTON3, m_btn_UnZip);
	DDX_Control(pDX, IDC_BUTTON4, m_btn_Help);
	DDX_Control(pDX, IDOK, m_btn_Scan);
}

BEGIN_MESSAGE_MAP(CVirustotalScanerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CVirustotalScanerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CVirustotalScanerDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CVirustotalScanerDlg message handlers

BOOL CVirustotalScanerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	// Init button
	m_btn_Setting.SetIcon(IDI_ICON_SETTING);
	m_btn_UnZip.SetIcon(IDI_ICON_UNZIP);
	m_btn_Help.SetIcon(IDI_ICON_HELP);

	// Init list
	LVCOLUMN lvColumn;

	lvColumn.cx = 300;
	lvColumn.mask = LVCF_WIDTH | LVCF_TEXT;
	lvColumn.pszText = _T("ImagePath");
	this->m_ListCtrl.InsertColumn(1, &lvColumn);

	lvColumn.cx = 250;
	lvColumn.mask = LVCF_WIDTH | LVCF_TEXT;
	lvColumn.pszText = _T("Md5");
	this->m_ListCtrl.InsertColumn(2, &lvColumn);

	lvColumn.cx = 100;
	lvColumn.mask = LVCF_WIDTH | LVCF_TEXT;
	lvColumn.pszText = _T("Status");
	this->m_ListCtrl.InsertColumn(3, &lvColumn);

	this->m_ListCtrl.SetExtendedStyle(m_ListCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	m_pThreadScan = NULL;
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVirustotalScanerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVirustotalScanerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVirustotalScanerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CVirustotalScanerDlg::OnBnClickedOk()     //button scan
{
	// TODO: Add your control notification handler code here
	CString sPath = _T("");
	
	m_ListCtrl.DeleteAllItems();
	// Check chose folder scan
	m_edit_Path.GetWindowText(sPath);
	if (sPath == _T(""))
	{
		AfxMessageBox(_T("Chưa chọn thư mục quét!"));
		return;
	}

	if (!PathIsDirectory(sPath))
	{
		AfxMessageBox(_T("Đường dẫn không đúng!"));
		return;
	}

	if (sPath.GetLength() + 50 > MAX_PATH)
	{
		AfxMessageBox(_T("Đường dẫn quá dài!"));
		return;
	}
	// Check connect to www.virustotal.com
	if(!InternetCheckConnection(L"https://www.virustotal.com",FLAG_ICC_FORCE_CONNECTION,0))
	{
		AfxMessageBox(_T("Kiểm tra kết nối mạng!"));
		return;
	}

	if (m_pThreadScan != NULL)
	{
		SAFE_RELEASE(m_pThreadScan);
	}

	m_pThreadScan = new CManagerThread(sPath);

	m_btn_Scan.EnableWindow(FALSE);

	m_pThreadScan->SetHwndDialogMain(&this->m_hWnd);
	m_pThreadScan->SetBtnScan(&m_btn_Scan);
	m_pThreadScan->SetListCrtl(&m_ListCtrl);
	m_pThreadScan->BeginThread();
	

//	OnOK();
}

void CVirustotalScanerDlg::OnBnClickedButton1()
{
	 //TODO: Add your control notification handler code here
	BROWSEINFO   bi; 
	ZeroMemory(&bi,   sizeof(bi)); 
	TCHAR   szDisplayName[MAX_PATH]; 
	//szDisplayName[0]    =   '';  

	bi.hwndOwner        =   this->m_hWnd; 
	bi.pidlRoot         =   NULL; 
	bi.pszDisplayName   =   szDisplayName; 
	bi.lpszTitle        =   _T("Please select a folder for storing received files :"); 
	bi.ulFlags          =   BIF_RETURNONLYFSDIRS;
	bi.lParam           =   NULL; 
	bi.iImage           =   0;  

	LPITEMIDLIST   pidl   =   SHBrowseForFolder(&bi);
	TCHAR   szPathName[MAX_PATH]; 
	if   (NULL   !=   pidl)
	{
		BOOL bRet = SHGetPathFromIDList(pidl,szPathName);
		if(FALSE == bRet)
			return;
		m_edit_Path.SetWindowText(szPathName);
	}

}
