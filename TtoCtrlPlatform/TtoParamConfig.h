#pragma once
#include "afxwin.h"
#include "DB.h"

class CTtoParamConfig : public CDialog
{
	DECLARE_DYNAMIC(CTtoParamConfig)

public:
	CTtoParamConfig(int index,string deciceId,string taskId,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CTtoParamConfig();

// 对话框数据
	enum { IDD = IDD_DIALOG_CONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CButton m_ComBoxMedName;
	CButton m_ComBoxMedWeight;
	CButton m_ComBoxMedSpecial;
	CButton m_ComBoxMedPlace;
	CButton m_ComBoxMedBatch;
	CButton m_ComBoxMedDate;
	CButton m_ComBoxMedQRCode;

	string gDeviceId;
	string gTaskId;    
	int iDeviceId;//序列号
	afx_msg void SetDeviceConfigParamToDB();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
private:
	DB<CMySql> db;
	DB<CMySql>::const_iterator iter;
public:
	afx_msg void OnPaint();
	afx_msg void OnBnClickedButtonPzQuit();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	CButton m_ComBox1Code;
	CButton m_ComBoxMedDefine1;
	CButton m_ComBoxMedDefine2;
	CButton m_ComBoxMedDefine3;
	CButton m_ComBoxMedDataTo;
	CButton m_ComBoxMedOffdoc;
	CButton m_ComBoxMedStandard;
	CButton m_ComBoxCpyName;
};
