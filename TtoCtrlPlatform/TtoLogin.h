#pragma once

#include "afxwin.h"
#include "ModifyPassDlg.h"
// CTtoLogin 对话框

class CTtoLogin : public CDialog
{
	DECLARE_DYNAMIC(CTtoLogin)

public:
	CTtoLogin(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CTtoLogin();

// 对话框数据
	enum { IDD = IDD_DIALOG_LOGIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	bool passFlag;//验证通过否的标识
	afx_msg void OnBnClickedLogin();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnPaint();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
private:
	CEdit m_EditLoginName;
	CEdit m_EditLoginPass;
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	virtual BOOL OnInitDialog();
};
