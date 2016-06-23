#pragma once
#include "afxcmn.h"
#include "Definition.h"
#include <list>

// CMidPacketInfo 对话框
/////////////////////////////////


class CMidPacketInfo : public CDialog
{
	DECLARE_DYNAMIC(CMidPacketInfo)

public:
	CMidPacketInfo(string task,string tname,string tbatch,string tweight,string tspec,string tplace,string tsimplace,string tdate,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMidPacketInfo();

// 对话框数据
	enum { IDD = IDD_DIALOG_MID_PACKET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	string inTaskId;

	//要打印的参数;
	string midname;
	string midweight;
	string midspec;
	string midbatch;
	string middate;
	string midplace;
	string midsimplace;//简单产地

	string printerName;

	afx_msg void OnBnClickedButtonFile();
	afx_msg void OnBnClickedButtonPrint();
	afx_msg void OnBnClickedButtonMidCancel();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
