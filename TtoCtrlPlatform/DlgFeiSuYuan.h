#pragma once
#include "Definition.h"
#include "afxwin.h"
#include "DB.h"
#include "afxdtctl.h"
// CDlgFeiSuYuan �Ի���

class CDlgFeiSuYuan : public CDialog
{
	DECLARE_DYNAMIC(CDlgFeiSuYuan)

public:
	CDlgFeiSuYuan(int editType,string taskId,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgFeiSuYuan();

// �Ի�������
	enum { IDD = IDD_DIALOG_FEI_SUYUAN };
	string fName;
	string fNum;
	string fWeight;
	string fSpec;
	string fPlace;
	string fBatch;
	string fDate;
	string f1Code;
	string fDateTo;//��Ч����
	string fOffdoc;//���ĺ�
	string fStandard;//ִ�б�׼
	string fDefine1Value;
	string fDefine2Value;
	string fDefine3Value;

	bool writeDbFlag;
	int gEditType;
	string gTaskId;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	DB<CMySql> db;
	DB<CMySql>::const_iterator iter;
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	CDateTimeCtrl m_DtPicker;
	CDateTimeCtrl m_DtPickerTime;
	afx_msg void OnDtnDatetimechangeDatetimepickerFsy(NMHDR *pNMHDR, LRESULT *pResult);

	
	afx_msg void OnDtnDatetimechangeDatetimepickerFsyTime(NMHDR *pNMHDR, LRESULT *pResult);
	CDateTimeCtrl mDateToCtrl;
	afx_msg void OnDtnDatetimechangeDatetimepickerFsyDateTo(NMHDR *pNMHDR, LRESULT *pResult);
};
