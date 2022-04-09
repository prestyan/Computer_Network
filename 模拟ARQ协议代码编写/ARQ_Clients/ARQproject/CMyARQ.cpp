// CMyARQ.cpp: 实现文件
//

#include "pch.h"
#include "ARQproject.h"
#include "CMyARQ.h"
#include "afxdialogex.h"
#include <time.h>
#include <fstream>
#pragma warning(disable:4996)

// CMyARQ 对话框
std::fstream sf;
SOCKET soc1;
char buff1[256];
bool stop = false;
bool check = false;
int totalSeq;
IMPLEMENT_DYNAMIC(CMyARQ, CDialogEx)

CMyARQ::CMyARQ(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{

}

CMyARQ::~CMyARQ()
{
}

char *genStr(int byte=4)
{
	char *b = new char[byte + 1];
	for (int i = 0; i < byte; i++)
	{
		b[i] = sf.get();
	}
	b[byte] = '\0';
	return b;
}
DWORD WINAPI threadpro(LPVOID pPrama)
{
	SOCKET soc = soc1;
	if (soc == INVALID_SOCKET)
	{
		MessageBox(NULL, _T("No SOCKET!"), NULL, MB_ICONERROR);
	}
	CMyARQ* p = (CMyARQ *)pPrama;
	char buff[256];
	char rbuff[256];
	char *str = new char[2048];
	memset(str, 0, 2048); memset(buff, 0, 256);
	int seq = 1;
	char s[8]; itoa(seq, s, 10);
	buff[0] = seq; buff[1] = '\0'; strcat(buff, "Hello, this is ARQclient.\r\n");
	send(soc, buff, sizeof(buff), 0);
	strcat(str, "send to ARQServer:\r\nHello, this is ARQclient. seq:");
	strcat(str, s); strcat(str, "\r\n");
	p->recvinfo.SetWindowText(CString(str)); seq++;
	while (1)
	{
		strcpy(buff1, buff);
		SetTimer(p->m_hWnd, 1, 200, NULL);
		int num = recv(soc, rbuff, 256, 0);
		if (num>0)
		{
			KillTimer(p->m_hWnd, 1);
			if (rbuff[2] == 'N')
			{
				strcat(str, "receive from server:\r\nNAK sseq: ");
				char s[8]; itoa(rbuff[1], s, 10);
				strcat(str, s); strcat(str, "本次重传\r\n");
				p->recvinfo.SetWindowText(CString(str));
				send(soc, buff, sizeof(buff), 0);
			}
			else
				break;
		}
		Sleep(1020);
	}
	while (!stop)
	{
		memset(buff, 0, 256);
		char *b; b = genStr(); itoa(seq, s, 10);
		buff[0] = seq; buff[1] = '\0'; strcat(buff, b);
		send(soc, buff, sizeof(buff), 0);
		strcat(str, "send to ARQServer:\r\n");
		strcat(str, b); strcat(str, " seq: ");
		strcat(str, s); strcat(str, "\r\n");
		p->recvinfo.SetWindowText(CString(str)); seq++;
		delete b;
		while (1)
		{
			strcpy(buff1, buff);
			SetTimer(p->m_hWnd, 1, 200, NULL);
			int num = recv(soc, rbuff, 256, 0);
			if (num > 0)
			{
				KillTimer(p->m_hWnd, 1);
				if (rbuff[2] == 'N')
				{
					strcat(str, "receive from server:\r\nNAK sseq: ");
					char s[8]; itoa(rbuff[1], s, 10);
					strcat(str, s); strcat(str, "本次重传\r\n");
					send(soc, buff, sizeof(buff), 0);
					p->recvinfo.SetWindowText(CString(str)); //seq++;
				}
				else
				{
					strcat(str, "receive from server:\r\nACK sseq: ");
					char s[8]; itoa(rbuff[1], s, 10);
					strcat(str, s); strcat(str, "本次成功\r\n");
					p->recvinfo.SetWindowText(CString(str)); 
					break;
				}
			}
			p->recvinfo.LineScroll(p->recvinfo.GetLineCount());
			Sleep(2000);
		}
		p->recvinfo.LineScroll(p->recvinfo.GetLineCount());
		Sleep(2000);
	}
	totalSeq = seq;
	return 0;
}

//使用检验和的线程处理方式
//单独增加线程是为了减少判断次数
//以免每一次发送数据都要判断是否使用了校验和
DWORD WINAPI threadpro_Csum(LPVOID pPrama)
{
	SOCKET soc = soc1;
	if (soc == INVALID_SOCKET)
	{
		MessageBox(NULL, _T("No SOCKET!"), NULL, MB_ICONERROR);
	}
	CMyARQ* p = (CMyARQ *)pPrama;
	char buff[256];
	char rbuff[256];
	char *str = new char[2048];
	memset(str, 0, 2048); memset(buff, 0, 256);
	int seq = 1;
	char s[8]; itoa(seq, s, 10);
	buff[0] = seq; buff[1] = '\0'; strcat(buff, "USEA Hello, this is ARQclient.\r\n");
	send(soc, buff, sizeof(buff), 0);
	strcat(str, "send to ARQServer:\r\nUse checksum-Hello, this is ARQclient. seq:");
	strcat(str, s); strcat(str, "\r\n");
	p->recvinfo.SetWindowText(CString(str)); seq++;
	while (1)
	{
		strcpy(buff1, buff);
		SetTimer(p->m_hWnd, 1, 200, NULL);
		int num = recv(soc, rbuff, 256, 0);
		if (num > 0)
		{
			KillTimer(p->m_hWnd, 1);
			if (rbuff[3] == 'N')
			{
				strcat(str, "receive from server:\r\nNAK sseq: ");
				char s[8]; itoa(rbuff[1], s, 10);
				strcat(str, s); strcat(str, "本次重传\r\n");
				p->recvinfo.SetWindowText(CString(str));
				send(soc, buff, sizeof(buff), 0);
			}
			else
				break;
		}
		Sleep(1020);
	}
	while (!stop)
	{
		memset(buff, 0, 256);
		char *b; b = genStr(); itoa(seq, s, 10);
		char csum = 0;

		buff[0] = seq; 
		buff[1] = ' '; buff[2] = '\0'; strcat(buff, b);
		//计算校验和
		/*##################################
		#如果数据发送部分可变
		#需要更改这里计算校验和的循环次数
		####################################
		*/
		for (int i = 2; i < 6; i++)
		{
			csum += buff[i];
		}
		csum += seq;
		buff[1] = csum;
		//
		send(soc, buff, sizeof(buff), 0);
		strcat(str, "send to ARQServer:\r\n");
		strcat(str, b); strcat(str, " seq: ");
		strcat(str, s); strcat(str, "\r\n");
		//加上显示校验和的部分
		strcat(str, "本报文校验和:");
		itoa(csum, s, 10);
		strcat(str, s);
		//
		p->recvinfo.SetWindowText(CString(str)); seq++;
		delete b;
		while (1)
		{
			strcpy(buff1, buff);
			SetTimer(p->m_hWnd, 1, 200, NULL);
			int num = recv(soc, rbuff, 256, 0);
			if (num > 0)
			{
				KillTimer(p->m_hWnd, 1);
				//计算校验和是否正确
				char rsum = 0;
				for (int i = 3;i<6; i++)
				{
					if (rbuff[i] == '\0')
						break;
					rsum += rbuff[i];
				}
				rsum += rbuff[0];
				rsum += rbuff[1];
				//计算完毕
				//进行判断
				if (rsum != rbuff[2])
				{
					strcat(str, "\r\nCSUM ERROR, recv:");
					itoa(rbuff[2], s, 10); strcat(str, s); strcat(str, " cal:");
					itoa(rsum, s, 10); strcat(str, s);
					send(soc, buff, sizeof(buff), 0);
					p->recvinfo.SetWindowText(CString(str));
					p->recvinfo.LineScroll(p->recvinfo.GetLineCount());
					Sleep(2000);
					continue;
				}
				//
				if (rbuff[3] == 'N')
				{
					strcat(str, "receive from server:\r\nNAK sseq: ");
					char s[8]; itoa(rbuff[1], s, 10);
					strcat(str, s); strcat(str, "本次重传\r\n");
					send(soc, buff, sizeof(buff), 0);
					p->recvinfo.SetWindowText(CString(str)); //seq++;
				}
				else
				{
					strcat(str, "receive from server:\r\nACK sseq: ");
					char s[8]; itoa(rbuff[1], s, 10);
					strcat(str, s); strcat(str, "本次成功\r\n");
					p->recvinfo.SetWindowText(CString(str));
					break;
				}
			}
			p->recvinfo.LineScroll(p->recvinfo.GetLineCount());
			Sleep(2000);
		}
		p->recvinfo.LineScroll(p->recvinfo.GetLineCount());
		Sleep(2000);
	}
	totalSeq = seq;
	return 0;
}

void CMyARQ::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, recvinfo);
	DDX_Control(pDX, IDC_EDIT2, resend);
	this->SetWindowText(_T("ARQ接收端信息"));
	CRect temprect(0, 0, 800, 600);//设置对话框大小
	CWnd::SetWindowPos(NULL, 0, 0, temprect.Width(), temprect.Height(), SWP_NOZORDER | SWP_NOMOVE);
	CFont e,e1;
	e.CreatePointFont(200, _T("宋体"));
	GetDlgItem(IDC_STATIC)->SetFont(&e);
	e1.CreatePointFont(120, _T("宋体"));
	recvinfo.SetFont(&e1);
	sf.open("sinfo.txt", std::ios::in);
	if (!sf.is_open())
	{
		MessageBox(_T("文件打开失败！"), NULL, MB_ICONERROR);
		return;
	}
	
	soc1 = CMyARQ::soc;
}

BEGIN_MESSAGE_MAP(CMyARQ, CDialogEx)
	ON_BN_CLICKED(IDOK, &CMyARQ::OnBnClickedOk)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &CMyARQ::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMyARQ::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_CHECK1, &CMyARQ::OnBnClickedCheck1)
	ON_BN_CLICKED(IDCANCEL, &CMyARQ::OnBnClickedCancel)
END_MESSAGE_MAP()


// CMyARQ 消息处理程序


void CMyARQ::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	stop = false;
	CDialogEx::OnOK();
}


void CMyARQ::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
	case 1:
		//MessageBox(_T("lll"), NULL, MB_ICONERROR);
		send(soc1, buff1, sizeof(buff1), 0);
		char r[64];
		char s[8]; itoa(buff1[0], s, 10);
		strcpy(r, "本次超时\r\n进行重传\r\nseq:");
		strcat(r, s);
		resend.SetWindowText(CString(r));
		break;
	default:break;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CMyARQ::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	stop = true;
	
}


void CMyARQ::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	stop = false;
	if (!check)
	{
		HANDLE myh;
		DWORD nthread = 0;
		myh = (HANDLE)::CreateThread(NULL, 0, threadpro, (LPVOID)this, 0, &nthread);
	}
	else//使用校验和
	{
		HANDLE myh;
		DWORD nthread = 0;
		myh = (HANDLE)::CreateThread(NULL, 0, threadpro_Csum, (LPVOID)this, 0, &nthread);
	}
}


void CMyARQ::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (check == false)
		check = true;
	else
		check = false;
}


void CMyARQ::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	//点击取消，发送exit
	memset(buff1, 0, sizeof(buff1));
	if (check)
	{
		char t2 = 0;
		buff1[0] = totalSeq; buff1[1] = ' ';
		buff1[2] = '\0';
		strcat(buff1, "exit");
		for (int i = 2; i<6; i++)
			t2 += buff1[i];
		t2 += buff1[0];
		buff1[1] = t2;
		send(soc1, buff1, sizeof(buff1), 0);
	}
	else
	{
		buff1[0] = totalSeq;
		buff1[1] = '\0';
		strcat(buff1, "exit");
		send(soc1, buff1, sizeof(buff1), 0);
	}
	CDialogEx::OnCancel();
}
