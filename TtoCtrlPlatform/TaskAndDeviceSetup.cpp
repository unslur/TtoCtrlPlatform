// TaskAndDeviceSetup.cpp : 实现文件
//

#include "stdafx.h"
#include "TtoCtrlPlatform.h"
#include "TaskAndDeviceSetup.h"


IMPLEMENT_DYNAMIC(CTaskAndDeviceSetup, CDialog)

CTaskAndDeviceSetup::CTaskAndDeviceSetup(string sTask,CWnd* pParent /*=NULL*/)
	: CDialog(CTaskAndDeviceSetup::IDD, pParent),db(new CMySql(DB_IP,DB_NAME,DB_USER,DB_PASS))
{
	rTask = sTask;
	db.ExeSql("SET NAMES GBK");
}

CTaskAndDeviceSetup::~CTaskAndDeviceSetup()
{

}

void CTaskAndDeviceSetup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_SETUP_DEVICE, cb_Device);
}


BEGIN_MESSAGE_MAP(CTaskAndDeviceSetup, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_MATCH, &CTaskAndDeviceSetup::OnBnClickedButtonMatch)
	ON_BN_CLICKED(IDC_BUTTON_FEIPEI_QUIT, &CTaskAndDeviceSetup::OnBnClickedButtonFeipeiQuit)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


//取得设备ID
BOOL CTaskAndDeviceSetup::GetDeviceNameFromDeviceList()
{
	BOOL ret = FALSE;
	try{
		iter = db.ExeSql("select devicenickname from table_device");
		int i=0;
		string deviceName="";
		while(iter.HasMore()){
				deviceName = (string)iter.GetData(0);
				cb_Device.InsertString(i, deviceName.c_str());
				i++;
		}
		ret = TRUE;	
	}
	catch (...)
	{
		ret = FALSE;
	}
	cb_Device.SetCurSel(0);
	return ret;
}

BOOL CTaskAndDeviceSetup::OnInitDialog()
{
	CDialog::OnInitDialog();
	GetDeviceNameFromDeviceList();
	return TRUE;  // return TRUE unless you set the focus to a control
}

//查询任务号是否被某个设备占用
BOOL CTaskAndDeviceSetup::CheckTaskIdInMatchOrNot(string taskId)  
{
	BOOL ret = FALSE;
	iter = db.ExeSql("select CAST(tabletstorid as char) from table_device");
	while(iter.HasMore()){
		string tmpTaskId = iter.GetData(0);
		if (tmpTaskId == taskId)
		{
			ret = TRUE;
			break;
		}
	}
	return ret;
}

//分配
BOOL CTaskAndDeviceSetup::TaskAndDeviceMatch(const char* deviceId)  
{
	char sqlBuff[0x200] = {0};
	sprintf_s(sqlBuff,0x200,"update table_device SET tabletstorid = %s where devicetag='%s'",iter.FormatSqlStr(rTask).c_str(),deviceId);
	try{
		db.ExeSql(sqlBuff); 
	}
	catch(...){
		return FALSE;
	}
	return TRUE;
}

//根据devicenickname从数据库取设备号
BOOL CTaskAndDeviceSetup::GetDeviceIdFromDeviceList(string deviceName,string &deviceId)  
{ 
	char sqlBuff[0x200] = {0};
	sprintf_s(sqlBuff,0x200,"select devicetag from table_device where devicenickname='%s'",iter.FormatSqlStr(deviceName).c_str());
	try{
		iter = db.ExeSql(sqlBuff); 
		if (iter.HasMore()){
			deviceId = (string)iter.GetData(0);
		}
	}
	catch(...){
		return FALSE;
	}
	return TRUE;
}

//根据deviceid取taskId
BOOL CTaskAndDeviceSetup::GetTaskIdFromDeviceStatus(string deviceId,string& taskId)  
{
	char sqlBuff[0x200] = {0};
	sprintf_s(sqlBuff,0x200,"select CAST(tabletstorid as char) from table_device where devicetag='%s'",iter.FormatSqlStr(deviceId).c_str());
	try{
		iter = db.ExeSql(sqlBuff); 
		if (iter.HasMore()){
			taskId = (string)iter.GetData(0);
			if ("0" == taskId)
			{
				return FALSE;
			}
		}
	}
	catch(...){
		return FALSE;
	}
	return TRUE;
}

void CTaskAndDeviceSetup::OnBnClickedButtonMatch()
{
	string taskId = rTask;
	if (CheckTaskIdInMatchOrNot(taskId)){
		MessageBox("任务已经分配，如需重新分配，请先将任务原来分配的设备取消,步骤：右键该任务，点击\"取消设备\"！","任务和设备分配",MB_SYSTEMMODAL);
		return;
	}
	CString strDevice;
	int j=0;
	j=cb_Device.GetCurSel();
	if (-1 == j){
		MessageBox("抱歉，没有设备可以接收指定的任务！","任务和设备分配",MB_SYSTEMMODAL);
		return;
	}
	CString deviceName;
	cb_Device.GetLBText(j,deviceName);
	string sDeviceName="";
	sDeviceName = deviceName.GetBuffer(0);
	deviceName.ReleaseBuffer();
	if (sDeviceName.length() == 0){
		MessageBox("取设备名称失败！","任务和设备分配",MB_SYSTEMMODAL);
		return;
	}

	string deviceId = "";
	string oldTaskId = "";
	if (GetDeviceIdFromDeviceList(sDeviceName,deviceId)){
		if (!GetTaskIdFromDeviceStatus(deviceId,oldTaskId)){
			if(TaskAndDeviceMatch(deviceId.c_str())){
				MessageBox("任务分配成功！","任务分配提示：",MB_SYSTEMMODAL);
				OnCancel();
			}else MessageBox("任务分配失败！","任务分配提示：",MB_SYSTEMMODAL);
		}else MessageBox("该设备已分配,请重新选择设备！","任务分配提示：",MB_SYSTEMMODAL);
	}
}

void CTaskAndDeviceSetup::OnBnClickedButtonFeipeiQuit()
{
	OnCancel();
}

void CTaskAndDeviceSetup::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect   rect;   
	GetClientRect(&rect);   
	CDC   dcMem;   
	dcMem.CreateCompatibleDC(&dc);   
	CBitmap   bmpBackground;   
	bmpBackground.LoadBitmap(IDB_BITMAP_FP_BJ);
	BITMAP   bitmap;   
	bmpBackground.GetBitmap(&bitmap);   
	CBitmap   *pbmpOld=dcMem.SelectObject(&bmpBackground);   
	dc.StretchBlt(0,0,rect.Width(),rect.Height(),&dcMem,0,0,bitmap.bmWidth,bitmap.bmHeight,SRCCOPY); 
	((CButton *)GetDlgItem(IDC_BUTTON_MATCH))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_FP_OK))); 
	((CButton *)GetDlgItem(IDC_BUTTON_FEIPEI_QUIT))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_MAIN_QUIT))); 
	
}

void CTaskAndDeviceSetup::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonDown(nFlags, point);
		PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x,point.y));
}
