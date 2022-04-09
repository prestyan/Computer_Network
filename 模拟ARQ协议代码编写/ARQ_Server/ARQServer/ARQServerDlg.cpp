
// ARQServerDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "ARQServer.h"
#include "ARQServerDlg.h"
#include "afxdialogex.h"
#include <WinSock2.h>
#include <time.h>
#include<fstream>
#define MAX_CONNECTIONS 3
#pragma warning (disable:4996)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int ports = -1;
int presentCon = 0, lastcon = -1;
int prevCSeq = -1, prevsit = 0;
SOCKET m_Server[MAX_CONNECTIONS+1];
SOCKET soc;
char hostname[256];
bool cs = false;//是否使用校验和

DWORD WINAPI threadpro2(LPVOID pParam);
DWORD WINAPI threadpro(LPVOID pParam)//线程1
{
	CARQServerDlg *p = (CARQServerDlg*)pParam;
	sockaddr_in serveraddrfrom;
	int len = sizeof(serveraddrfrom);
	while (1)
	{
		listen(soc, 0);
		m_Server[presentCon] = accept(soc, (sockaddr*)&serveraddrfrom, &len);
		if (m_Server[presentCon] != INVALID_SOCKET)
		{
			char s[256];
			strcpy(s, "This is ARQServer:");
			strcat(s, hostname);
			send(m_Server[presentCon], s, sizeof(s), 0);
			p->connectinfo.SetWindowText(_T("连接到ARQ客户端：") + CString(inet_ntoa(serveraddrfrom.sin_addr)));
			if (presentCon == MAX_CONNECTIONS)
			{
				send(m_Server[presentCon], "服务器忙，暂时拒绝连接。", sizeof("服务器忙，暂时拒绝连接。"), 0);
				continue;
			}
			presentCon++;
			HANDLE myh;
			DWORD nthred = 0;
			myh = (HANDLE)::CreateThread(NULL, 0, threadpro2, p, 0, &nthred);
		}
	}

	return 0;
}

DWORD WINAPI threadpro2(LPVOID pParam)//线程2
{
	std::fstream rf;
	rf.open("recvFile.txt", std::ios::out);
	char buff[256];
	char *str = new char[2048*3]; str[0] = '\0';
	int seq = 1;
	CARQServerDlg* p = (CARQServerDlg*)pParam;
	SOCKET soc = m_Server[presentCon - 1];
	while (1)
	{
		int num = recv(soc, buff, 256, 0);
		if (num > 0)
		{
			strcat(str, "从客户端收到消息：\r\ncseq: ");
			int cseq = buff[0]; buff[0] = '|'; char s2[8]; itoa(cseq, s2, 10);
			strcat(str, s2); char t1;
			if (cs)
			{
				t1 = buff[1];
				buff[1] = '-';
			}
			strcat(str, buff);
			if (cs)	buff[1] = t1;
			if (cs)//增加校验和部分
			{
				itoa(buff[1], s2, 10);
				strcat(str, " 报文校验和:");
				strcat(str, s2); strcat(str, " | ");
				char rsum = 0;
				for (int i = 2; ; i++)
				{
					if (buff[i] == '\0')
						break;
					rsum += buff[i];
				}
				rsum += cseq;
				strcat(str, "计算出校验和:");
				itoa(rsum, s2, 10); strcat(str, s2);
				if (rsum != buff[1])
				{
					//if not right, send error
					//do nothing ,drop the packet.
					strcat(str, "CSUM ERROR!");
					p->connectinfo.SetWindowText(CString(str));
					p->connectinfo.LineScroll(p->connectinfo.GetLineCount());
					continue;
					//
				}
			}
			p->connectinfo.SetWindowText(CString(str));
			if (cs&&buff[2] == 'e'&&buff[3] == 'x'&&buff[4] == 'i'&&buff[5] == 't')
			{
				send(soc, "exit\r\nACK\r\n\0", sizeof("exit\r\nACK\r\n\0"), 0);
				strcat(str, "exit\r\nACK\r\n客户机退出");
				p->connectinfo.SetWindowText(CString(str));
				p->connectinfo.LineScroll(p->connectinfo.GetLineCount());
				closesocket(soc);
				presentCon--;
				break;
			}
			if (strcmp(buff, "|exit")==0)
			{
				send(soc, "exit\r\nACK\r\n\0", sizeof("exit\r\nACK\r\n\0"), 0);
				strcat(str, "exit\r\nACK\r\n客户机退出");
				p->connectinfo.SetWindowText(CString(str));
				p->connectinfo.LineScroll(p->connectinfo.GetLineCount());
				closesocket(soc);
				presentCon--; 
				break;
			}
			buff[0] = cseq;
			//增加的校验和部分
			if (buff[1] == 'U'&&buff[2] == 'S'&&buff[3] == 'E'&&buff[4] == 'A')
			{
				strcat(str, "\r\n客户机要求使用校验和");
				p->connectinfo.SetWindowText(CString(str));
				p->connectinfo.LineScroll(p->connectinfo.GetLineCount());
				cs = true;

				buff[1] = seq; seq++;
				buff[3] = '\0';//clear
				strcat(buff, "ACK\r\n");
				int rrsum = 0;
				for (int i = 3;; i++)
				{
					if (buff[i] == '\0')
						break;
					rrsum += buff[i];
				}
				rrsum += buff[0]; rrsum += buff[1];
				buff[2] = rrsum;
				send(soc, buff, sizeof(buff), 0); prevsit = 0;

				continue;
			}
			//
			if (prevCSeq == cseq && prevsit == 3)//重复
			{
				strcat(str, "此处由于'确认'丢失，客户机重传\r\n");
				p->connectinfo.SetWindowText(CString(str));
				p->connectinfo.LineScroll(p->connectinfo.GetLineCount());
				seq--;
			}

			//选择随机状态
			int rn,situation;
			time_t t;
			time(&t);
			srand(t);
			rn = rand();
			situation = rn % 20;
			if (prevsit == 3)
				situation = 1;
			if (!cs)
			{
				if (situation >= 0 && situation <= 15)//正常
				{
					strcat(str, "\r\n--->0(ok) send:ACK sseq:");
					char s[8]; itoa(seq, s, 10);
					strcat(str, s); strcat(str, "\r\n");
					p->connectinfo.SetWindowText(CString(str));
					for (int i = 1;; i++)
					{
						if (buff[i] == '\0')
							break;
						rf.put(buff[i]);
					}
						
					buff[1] = seq; seq++;
					buff[2] = '\0';//clear
					strcat(buff, "ACK\r\n");
					send(soc, buff, sizeof(buff), 0); prevsit = 0;
				}
				else if (situation > 15 && situation <= 17)//1出错
				{
					strcat(str, "\r\n--->1(error) send:NAK sseq:");
					char s[8]; itoa(seq, s, 10);
					strcat(str, s); strcat(str, "\r\n");
					p->connectinfo.SetWindowText(CString(str));
					buff[1] = seq; seq++;
					buff[2] = '\0';//clear
					strcat(buff, "NAK\r\n");
					send(soc, buff, sizeof(buff), 0); prevsit = 1;
				}
				else if (situation == 18)//2丢失
				{
					strcat(str, "\r\n--->2(loss) send:NAK sseq:");
					char s[8]; itoa(seq, s, 10);
					strcat(str, s); strcat(str, "\r\n");
					p->connectinfo.SetWindowText(CString(str));
					buff[1] = seq; seq++;
					buff[2] = '\0';//clear
					strcat(buff, "NAK\r\n");
					send(soc, buff, sizeof(buff), 0); prevsit = 2;
				}
				else//3回复出错
				{
					strcat(str, "\r\n--->3(ACKloss) send:-- keep_sseq:");
					char s[8]; itoa(seq, s, 10);
					strcat(str, s); strcat(str, "\r\n");
					p->connectinfo.SetWindowText(CString(str));
					seq++; prevsit = 3;
					//continue;
				}
				p->connectinfo.LineScroll(p->connectinfo.GetLineCount());
				prevCSeq = cseq;
			}
			else//use checkSum
			{
				if (situation >= 0 && situation <= 15)//正常
				{
					strcat(str, "\r\n--->0(ok) send:ACK sseq:");
					char s[8]; itoa(seq, s, 10);
					strcat(str, s); strcat(str, "\r\n");
					p->connectinfo.SetWindowText(CString(str));
					for (int i = 2;; i++)
					{
						if (buff[i] == '\0')
							break;
						rf.put(buff[i]);
					}

					buff[1] = seq; seq++;//server seq
					buff[2] = ' ';
					buff[3] = '\0';//clear
					strcat(buff, "ACK\r\n");
					//计算校验和
					int sum = 0;
					for (int i = 3; i < 6; i++)
						sum += buff[i];
					sum += buff[1]; sum += buff[0];
					buff[2] = sum;
					//
					send(soc, buff, sizeof(buff), 0); prevsit = 0;

				}
				else if (situation > 15 && situation <= 17)//1出错
				{
					strcat(str, "\r\n--->1(error) send:NAK sseq:");
					char s[8]; itoa(seq, s, 10);
					strcat(str, s); strcat(str, "\r\n");
					p->connectinfo.SetWindowText(CString(str));
					buff[1] = seq; seq++; buff[2] = ' ';
					buff[3] = '\0';//clear
					strcat(buff, "NAK\r\n");
					//计算校验和
					int sum = 0;
					for (int i = 3; i < 6; i++)
						sum += buff[i];
					sum += buff[1]; sum += buff[0];
					buff[2] = sum;
					//
					send(soc, buff, sizeof(buff), 0); prevsit = 1;
				}
				else if (situation == 18)//2丢失
				{
					strcat(str, "\r\n--->2(loss) send:NAK sseq:");
					char s[8]; itoa(seq, s, 10);
					strcat(str, s); strcat(str, "\r\n");
					p->connectinfo.SetWindowText(CString(str));
					buff[1] = seq; seq++; buff[2] = ' ';
					buff[3] = '\0';//clear
					strcat(buff, "NAK\r\n");
					//计算校验和
					char sum = 0;
					for (int i = 3; i < 6; i++)
						sum += buff[i];
					sum += buff[0]; sum += buff[1];
					buff[2] = sum;
					//
					send(soc, buff, sizeof(buff), 0); prevsit = 2;
				}
				else//3回复出错
				{
					strcat(str, "\r\n--->3(ACKloss) send:-- keep_sseq:");
					char s[8]; itoa(seq, s, 10);
					strcat(str, s); strcat(str, "\r\n");
					p->connectinfo.SetWindowText(CString(str));
					seq++; prevsit = 3;
					//continue;
				}
				p->connectinfo.LineScroll(p->connectinfo.GetLineCount());
				prevCSeq = cseq;
			}
		}
	}
	return 0;
}

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


// CARQServerDlg 对话框



CARQServerDlg::CARQServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ARQSERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CARQServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_portedit, portinfo);
	DDX_Control(pDX, IDC_connectedit, connectinfo);
}

BEGIN_MESSAGE_MAP(CARQServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_SETFOCUS(IDC_portedit, &CARQServerDlg::OnEnSetfocusportedit)
	ON_BN_CLICKED(IDC_BUTTON1, &CARQServerDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CARQServerDlg::OnBnClickedButton2)
	ON_WM_TIMER()
	ON_EN_CHANGE(IDC_connectedit, &CARQServerDlg::OnEnChangeconnectedit)
END_MESSAGE_MAP()


// CARQServerDlg 消息处理程序

BOOL CARQServerDlg::OnInitDialog()
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
	this->SetWindowText(_T("服务器端ARQ协议模拟"));
	connectinfo.SetWindowTextW(_T("未绑定\r\n请先绑定本机"));
	CFont c,p;
	c.CreatePointFont(360, _T("Arial"));
	p.CreatePointFont(150, _T("宋体"));
	GetDlgItem(IDC_STATIC)->SetFont(&c);
	portinfo.SetWindowText(_T("请输入端口号"));


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CARQServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CARQServerDlg::OnPaint()
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
HCURSOR CARQServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CARQServerDlg::OnEnSetfocusportedit()
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


void CARQServerDlg::OnBnClickedButton1()
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


void CARQServerDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	if (soc != INVALID_SOCKET)
		closesocket(soc);
	if (ports == -1)
	{
		MessageBox(_T("请输入端口号！"), NULL, MB_ICONERROR);
		return;
	}
	connectinfo.SetWindowText(_T("Binding host..."));
	//SOCKET soc;
	sockaddr_in serveraddr;

	serveraddr.sin_family = AF_INET;//设置服务器地址
	serveraddr.sin_port = htons(ports);//设置端口号
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	soc = socket(AF_INET, SOCK_STREAM, 0);
	int i = bind(soc, (sockaddr*)&serveraddr, sizeof(serveraddr));
	if (i == -1)
	{
		connectinfo.SetWindowText(_T("Binding host...\r\nBinding failed!"));
		MessageBox(_T("绑定失败，请检查！"), NULL, MB_ICONEXCLAMATION);
		return;
	}
	//成功
	//char hostname[256]
	CString p; p.Format(L"%d", ports);
	gethostname(hostname, sizeof(hostname));
	hostent *host = gethostbyname(hostname);
	strcpy_s(hostname,sizeof(hostname),inet_ntoa(*(in_addr*)host->h_addr_list[2]));//*(struct in_addr*)*host->h_addr_list[0])
	connectinfo.SetWindowTextW(_T("Binding successfully.\r\nhostIP:") + CString(hostname) + _T("\r\nhostPort:") + p+_T("\r\nListening..."));
	HANDLE myh;
	DWORD nthred = 0;
	myh = (HANDLE)::CreateThread(NULL, 0, threadpro, this, 0, &nthred);
	//SetTimer(1, 1000, NULL);
}


void CARQServerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (presentCon == lastcon)
	{
		return;
	}
	else
		lastcon++;
	sockaddr_in serveraddrfrom;
	int len = sizeof(serveraddrfrom);
	switch (nIDEvent)
	{
	case 1:
		listen(soc, 0);
		m_Server[presentCon] = accept(soc, (sockaddr*)&serveraddrfrom, &len);
		char s[256];
		strcpy(s, "This is ARQServer:");
		strcat(s, hostname);
		if (m_Server[presentCon] != INVALID_SOCKET)
		{
			send(m_Server[presentCon], s, sizeof(s), 0);
			connectinfo.SetWindowText(_T("连接到ARQ客户端：")+CString(inet_ntoa(serveraddrfrom.sin_addr)));
			if (presentCon == MAX_CONNECTIONS)
			{
				send(m_Server[presentCon], "服务器忙，暂时拒绝连接。", sizeof("服务器忙，暂时拒绝连接。"),0);
				break;
			}
			presentCon++;
			//thred
		}
		break;
	default:
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}



void CARQServerDlg::OnEnChangeconnectedit()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
