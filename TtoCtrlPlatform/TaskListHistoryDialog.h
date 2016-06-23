#pragma once
#include "afxcmn.h"
#include "Definition.h"
#include "Function.h"
#include "afxwin.h"
#include <map>
#include "DB.h"


class CTaskListHistoryDialog : public CDialog
{
	DECLARE_DYNAMIC(CTaskListHistoryDialog)

public:
	CTaskListHistoryDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CTaskListHistoryDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_TASK_HISTORY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	int curPage;
	int totalNum;
	CListCtrl ListTaskHistory;
	virtual BOOL OnInitDialog();
	BOOL GetTotalNumFromDB(int& totalNum);
	BOOL GetCurPageFromDB(int curPage);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CComboBox cbCurPerTotal;
	afx_msg void OnBnClickedButtonShouye();
	afx_msg void OnBnClickedButtonShangye();
	afx_msg void OnBnClickedButtonXiaye();
	afx_msg void OnBnClickedButtonMoye();
	afx_msg void OnCbnSelchangeComboCurPerTotal();
private:
	DB<CMySql> db;
	DB<CMySql>::const_iterator iter;
	map<int,string> taskStats;
public:
	afx_msg void OnBnClickedButtonHistoryQuit();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
