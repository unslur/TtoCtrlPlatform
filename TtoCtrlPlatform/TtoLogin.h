#pragma once

#include "afxwin.h"
#include "ModifyPassDlg.h"
// CTtoLogin �Ի���

class CTtoLogin : public CDialog
{
	DECLARE_DYNAMIC(CTtoLogin)

public:
	CTtoLogin(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CTtoLogin();

// �Ի�������
	enum { IDD = IDD_DIALOG_LOGIN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	bool passFlag;//��֤ͨ����ı�ʶ
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
