#pragma once


// CMyARQ 对话框

class CMyARQ : public CDialogEx
{
	DECLARE_DYNAMIC(CMyARQ)

public:
	SOCKET soc;
	CMyARQ(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CMyARQ();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CEdit recvinfo;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CEdit resend;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCancel();
};
