#pragma once
#include "afxwin.h"
#include "Definition.h"

class CModifyPassDlg : public CDialog
{
	DECLARE_DYNAMIC(CModifyPassDlg)

public:
	CModifyPassDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CModifyPassDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_MODIFY_PASS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	//CMysql ttoModifyPassSql;
	afx_msg void OnBnClickedButtonModifyOk();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
