#pragma once
#include "afxwin.h"
#include "Definition.h"
#include "Function.h"
#include "afxcmn.h"
#include "DB.h"
#include "afxdtctl.h"
// CWriteSimplePlaceDlg 对话框

class CWriteSimplePlaceDlg : public CDialog
{
	DECLARE_DYNAMIC(CWriteSimplePlaceDlg)

public:
	CWriteSimplePlaceDlg(string task,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CWriteSimplePlaceDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_MODIFY_PLACE_SIMPLE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_EditSimplePlace;
	string inTaskId;
	//BOOL WriteSimplePlace2DB(string sPlace);
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonSpQuit();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
private:
	DB<CMySql> db;
	DB<CMySql>::const_iterator iter;
public:
	CEdit m_Ecode1;
	CEdit m_EdateTo;
	CEdit m_EoffDocNum;
	CEdit m_Estandard;
	CEdit m_Edef1;
	CEdit m_Edef2;
	CEdit m_Edef3;
	CDateTimeCtrl m_SyDate;
	//CDateTimeCtrl m_SyTime;
	afx_msg void OnDtnDatetimechangeDatetimepickerSyDate(NMHDR *pNMHDR, LRESULT *pResult);
};
