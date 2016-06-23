#pragma once
#include "Definition.h"
#include "afxwin.h"
#include "DB.h"
// CCode1Dialog �Ի���

class CCode1Dialog : public CDialog
{
	DECLARE_DYNAMIC(CCode1Dialog)

public:
	CCode1Dialog(string task,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CCode1Dialog();
	string inTaskId;

// �Ի�������
	enum { IDD = IDD_DIALOG_1CODE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

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
