
#include "stdafx.h"
#include "Keyexe.h"
#include "KeyexeDlg.h"
#include "..\Keydll\Keydll.h"//Include this for functions in the DLL.
#include <tlhelp32.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_KEYSTROKE (WM_USER + 755)//This message is recieved when key is down/up
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
HWND txtinWND;
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
	//ON_MESSAGE(WM_KEYSTROKE, processkey)
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


// Find the HWND of the text input window
BOOL CALLBACK WindowEnumProc(HWND hWnd, LPARAM lParam)

{
	char className[256];

	GetClassName(hWnd, className,256);

	if((!_stricmp("TChatRichEdit", className)))
	{
		txtinWND=hWnd;
	}
	return TRUE;

}


HWND findTargetTopWindow(char * targetname, DWORD *pid )
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (_stricmp(entry.szExeFile, targetname) == 0)
			{  
				HWND hTopWnd = ::GetTopWindow(0 );
				while ( hTopWnd )
				{
					DWORD dwTheardId = ::GetWindowThreadProcessId(hTopWnd,pid);
					if ( *pid == entry.th32ProcessID )
					{
						char txt[256];
						int len=GetWindowText(hTopWnd,  txt, 256);

						//if(!_strnicmp(txt, targetname, strlen(targetname)-4))
						if(!_strnicmp(txt, targetname, 5))
						{
							//MessageBox(NULL, txt, "Skype Title", MB_OK);
							*pid=dwTheardId;
							CloseHandle(snapshot);
							return hTopWnd;
						}

					}
					hTopWnd = ::GetNextWindow( hTopWnd , GW_HWNDNEXT);
				}

			}
		}
	}

	CloseHandle(snapshot);

	return 0;
}



void CKeyexeDlg::OnHook()//Install or Remove keyboard Hook.
{
	// TODO: Add your control notification handler code here
	if(hooked==true)
	{
		if(removehook())
		{
			hooked = false;

			if (txtinWND != NULL)
			{

				CWnd *pWnd=CWnd::FromHandle(txtinWND); 

				if (pWnd->GetSafeHwnd()) 
				{ 

					pWnd->ShowWindow(SW_NORMAL); 
					pWnd->SetForegroundWindow(); 
					keybd_event(VK_RETURN, 0, 0, 0); 
					keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0); 
				}
			}

			m_hook.SetWindowText("Enable protection");		
		}
	}
	else
	{
		DWORD pid;
		HWND hTopWnd = findTargetTopWindow("Skype.exe", (DWORD *)&pid); 
		if(hTopWnd)
		{
			txtinWND=NULL;
			EnumChildWindows(hTopWnd,WindowEnumProc,NULL); 	//find text-in window by enuming all child windows
			if (txtinWND != NULL)
			{

				if(installhook(txtinWND,pid))
				{

					hooked = true;
					CWnd *pWnd=CWnd::FromHandle(txtinWND); 

					if (pWnd->GetSafeHwnd()) 
					{ 

						pWnd->ShowWindow(SW_NORMAL); 
						pWnd->SetForegroundWindow(); 
						keybd_event(VK_RETURN, 0, 0, 0); 
						keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0); 
					}

					m_hook.SetWindowText("Disable protection");
				}
			}
		}
	}

}


void CKeyexeDlg::OnCancel() //On exit, do cleanup(Close files, remove hook.).
{
	// TODO: Add extra cleanup here
	if(hooked==true)
	{
		removehook();
		CWnd *pWnd=CWnd::FromHandle(txtinWND); 

		if (pWnd->GetSafeHwnd()) 
		{ 

			pWnd->ShowWindow(SW_NORMAL); 
			pWnd->SetForegroundWindow(); 
			keybd_event(VK_RETURN, 0, 0, 0); 
			keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0); 
		}


	}

	CDialog::OnCancel();
}
