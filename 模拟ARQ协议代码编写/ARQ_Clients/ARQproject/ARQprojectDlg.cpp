
// ARQprojectDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "ARQproject.h"
#include "ARQprojectDlg.h"
#include "afxdialogex.h"
#include "CMyARQ.h"
#include "winsock2.h"
#include <ws2tcpip.h>
#include<string.h>
#include <regex>

#pragma warning(disable:4996)
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
int ports=-1;
char *add = new char[100]; 
SOCKET client_soc;
bool jumpEN = false;//跳转允许信号

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CARQprojectDlg 对话框



CARQprojectDlg::CARQprojectDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ARQPROJECT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CARQprojectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_title, title);
	DDX_Control(pDX, IDC_ipedit, ipinfo);
	DDX_Control(pDX, IDC_portedit, portinfo);
	DDX_Control(pDX, IDC_connect, connectinfo);
	//DDX_Control(pDX, IDC_EDIT1, resend);
	DDX_Control(pDX, IDC_STATIC_PICTURE, mp);
}

BEGIN_MESSAGE_MAP(CARQprojectDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_STN_CLICKED(IDC_STATIC_title, &CARQprojectDlg::OnStnClickedStatictitle)
	ON_EN_CHANGE(IDC_ipedit, &CARQprojectDlg::OnEnChangeipedit)
	ON_EN_SETFOCUS(IDC_ipedit, &CARQprojectDlg::OnEnSetfocusipedit)
	ON_BN_CLICKED(IDC_BUTTON1, &CARQprojectDlg::OnBnClickedButton1)
	ON_EN_CHANGE(IDC_portedit, &CARQprojectDlg::OnEnChangeportedit)
	ON_EN_SETFOCUS(IDC_portedit, &CARQprojectDlg::OnEnSetfocusportedit)
	ON_BN_CLICKED(IDC_BUTTON2, &CARQprojectDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDOK, &CARQprojectDlg::OnBnClickedOk)
	ON_EN_CHANGE(IDC_connect, &CARQprojectDlg::OnEnChangeconnect)
	ON_BN_CLICKED(IDC_BUTTON3, &CARQprojectDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDCANCEL, &CARQprojectDlg::OnBnClickedCancel)
	ON_STN_CLICKED(IDC_STATIC_PICTURE, &CARQprojectDlg::OnStnClickedStaticPicture)
END_MESSAGE_MAP()


// CARQprojectDlg 消息处理程序

BOOL CARQprojectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	WSADATA wsd;//定义WSADATA对象
	WSAStartup(MAKEWORD(2, 2), &wsd);

	add[0] = '\0';
	CFont t,e;
	t.CreatePointFont(300, _T("宋体"));
	title.SetWindowText(_T("请填入主机信息"));
	title.SetFont(&t);

	e.CreatePointFont(150,_T("宋体"));
	ipinfo.SetWindowText(_T("请输入ipv4地址"));
	ipinfo.SetFont(&e);

	CEdit *port = (CEdit*)GetDlgItem(IDC_portedit);
	port->SetWindowText(_T("请输入端口号"));
	port->SetFont(&e);

	this->SetWindowText(_T("发送端ARQ协议模拟"));

	connectinfo.SetWindowTextW(_T("未建立连接"));

	CRect rect;
	GetDlgItem(IDC_STATIC_PICTURE)->GetWindowRect(&rect);           //IDC_WAVE_DRAW为Picture Control的ID
	ScreenToClient(&rect);
	GetDlgItem(IDC_STATIC_PICTURE)->MoveWindow(rect.left, rect.top, 60, 220, true);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CARQprojectDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CARQprojectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CARQprojectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CARQprojectDlg::OnStnClickedStatictitle()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CARQprojectDlg::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CARQprojectDlg::OnEnChangeipedit()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CARQprojectDlg::OnEnSetfocusipedit()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(1);
	CString str;
	ipinfo.GetWindowText(str);
	if (str != "请输入ipv4地址")
		return;
	ipinfo.SetWindowText(_T(""));
	UpdateData(0);
}


void CARQprojectDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	CString ip;
	ipinfo.GetWindowText(ip);
	USES_CONVERSION;
	char *cip = T2A(ip);
	std::regex r("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}");
	bool ret = std::regex_match(cip,r);
	if (ret)
	{
		MessageBox(_T("服务器ip:") + ip, NULL, MB_ICONINFORMATION);
		strcpy_s(add, strlen(cip)+1, cip);
	}
	else
		MessageBox(_T("请输入正确格式的ipv4地址"), NULL, MB_ICONEXCLAMATION);
}


void CARQprojectDlg::OnEnChangeportedit()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CARQprojectDlg::OnEnSetfocusportedit()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	UpdateData(1);
	portinfo.GetWindowText(str);
	if (str != "请输入端口号")
		return;
	portinfo.SetWindowText(_T(""));
	UpdateData(0);
}


void CARQprojectDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	portinfo.GetWindowText(str);
	int h = _tstoi(str);
	if (h >= 0 && h <= 65535)
	{
		ports = h;
		MessageBox(_T("目的端口号:") + str, NULL, MB_ICONINFORMATION);
	}
	else
		MessageBox(_T("请输入正确的端口号！") + str, NULL, MB_ICONEXCLAMATION);
}


void CARQprojectDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!jumpEN)
	{
		MessageBox(_T("未建立连接！"), NULL, MB_ICONEXCLAMATION);
		return;
	}
	CMyARQ myarq;
	myarq.soc = client_soc;
	myarq.DoModal();
	//CDialogEx::OnOK();
}


void CARQprojectDlg::OnEnChangeconnect()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	
}


void CARQprojectDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	/*CBitmap bitmap;
	bitmap.LoadBitmap(2);
	CStatic *p = (CStatic *)GetDlgItem(IDC_STATIC_PICTURE);
	p->ModifyStyle(0xf, SS_BITMAP | SS_CENTERIMAGE);
	p->SetBitmap(bitmap);*/
	if (client_soc != INVALID_SOCKET)
	{
		send(client_soc, "exit", sizeof("exit"), 0);
		closesocket(client_soc);
	}
	if (add[0] == '\0' || ports == -1)
	{
		MessageBox(_T("客户端信息不正确"), NULL, MB_ICONEXCLAMATION);
		return;
	}
	//建立连接
	connectinfo.SetWindowText(_T("Connecting to server start..."));
	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(ports);
	serveraddr.sin_addr.S_un.S_addr = inet_addr(add);
	//inet_pton(AF_INET,add,(void*)&serveraddr.sin_addr.S_un.S_addr) ;
	client_soc = socket(AF_INET, SOCK_STREAM, 0);
	connectinfo.SetWindowText(_T("Connecting to server start...\r\nConstly trying now..."));
	int i = connect(client_soc,(sockaddr*)&serveraddr,sizeof(serveraddr));
	if (i == -1)
	{
		connectinfo.SetWindowText(_T("Connecting to server start...\r\nConstly trying now...\r\nFail to connect."));
		MessageBox(_T("连接失败，请检查"), NULL, MB_ICONEXCLAMATION);
		return;
	}
	connectinfo.SetWindowText(_T("Connect successfully!"));
	char buff[256];
	recv(client_soc, buff, 256, 0);
	connectinfo.SetWindowTextW(_T("Connect successfully!\r\n")+CString(buff));
	jumpEN = true;
}


void CARQprojectDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
	WSACleanup();
	delete add;
}


void CARQprojectDlg::OnStnClickedStaticPicture()
{
	// TODO: 在此添加控件通知处理程序代码
}
