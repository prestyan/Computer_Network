
// ARQprojectDlg.h: 头文件
//

#pragma once


// CARQprojectDlg 对话框
class CARQprojectDlg : public CDialogEx
{
// 构造
public:
	CARQprojectDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ARQPROJECT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnStnClickedStatictitle();
	CStatic title;
	afx_msg void OnEnChangeEdit1();
	CEdit ipinfo;
	afx_msg void OnEnChangeipedit();
	afx_msg void OnEnSetfocusipedit();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnEnChangeportedit();
	afx_msg void OnEnSetfocusportedit();
	CEdit portinfo;
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnChangeconnect();
	CEdit connectinfo;
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedCancel();
	CEdit resend;
	afx_msg void OnStnClickedStaticPicture();
	CStatic mp;
};
