#pragma once
#include "afxwin.h"
#include "Definition.h"
#include "Function.h"
#include "afxcmn.h"
#include "DB.h"

typedef struct SettagNMITEMACTIVATE
{
	NMHDR   hdr;
	int     iItem;
	int     iSubItem;
	UINT    uNewState;
	UINT    uOldState;
	UINT    uChanged;
	POINT   ptAction;
	LPARAM  lParam;
	UINT    uKeyFlags;
} SetNMITEMACTIVATE, *LSetPNMITEMACTIVATE;

class CTaskAndDeviceSetup : public CDialog
{
	DECLARE_DYNAMIC(CTaskAndDeviceSetup)

public:
	CTaskAndDeviceSetup(string sTask,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CTaskAndDeviceSetup();

// �Ի�������
	enum { IDD = IDD_DIALOG_TASK_AND_DEVICE_SETUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CComboBox cb_Device;
	string rTask;//�ӹ��캯����������taskid

	virtual BOOL OnInitDialog();
	BOOL GetDeviceNameFromDeviceList();
	BOOL TaskAndDeviceMatch(const char* deviceId);
	BOOL CheckTaskIdInMatchOrNot(string taskId);
	BOOL GetDeviceIdFromDeviceList(string deviceName,string &deviceId);
	BOOL GetTaskIdFromDeviceStatus(string deviceId,string& taskId);

	afx_msg void OnBnClickedButtonMatch();
private:
	DB<CMySql> db;
	DB<CMySql>::const_iterator iter;
public:
	afx_msg void OnBnClickedButtonFeipeiQuit();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
