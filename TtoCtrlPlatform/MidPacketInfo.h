#pragma once
#include "afxcmn.h"
#include "Definition.h"
#include <list>

// CMidPacketInfo �Ի���
/////////////////////////////////


class CMidPacketInfo : public CDialog
{
	DECLARE_DYNAMIC(CMidPacketInfo)

public:
	CMidPacketInfo(string task,string tname,string tbatch,string tweight,string tspec,string tplace,string tsimplace,string tdate,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CMidPacketInfo();

// �Ի�������
	enum { IDD = IDD_DIALOG_MID_PACKET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	string inTaskId;

	//Ҫ��ӡ�Ĳ���;
	string midname;
	string midweight;
	string midspec;
	string midbatch;
	string middate;
	string midplace;
	string midsimplace;//�򵥲���

	string printerName;

	afx_msg void OnBnClickedButtonFile();
	afx_msg void OnBnClickedButtonPrint();
	afx_msg void OnBnClickedButtonMidCancel();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
