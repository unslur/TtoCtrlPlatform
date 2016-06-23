#pragma once
#include "Definition.h"
#include "afxwin.h"
#include "DB.h"
// CCode1Dialog 对话框

class CCode1Dialog : public CDialog
{
	DECLARE_DYNAMIC(CCode1Dialog)

public:
	CCode1Dialog(string task,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CCode1Dialog();
	string inTaskId;

// 对话框数据
	enum { IDD = IDD_DIALOG_1CODE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClicked1codeOk();
	CEdit m_EditCode1;
	virtual BOOL OnInitDialog();
	DB<CMySql> db;
	DB<CMySql>::const_iterator iter;
	afx_msg void OnBnClickedBtn1codeQuit();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
