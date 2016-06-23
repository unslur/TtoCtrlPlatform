#include "stdafx.h"
#include <stdio.h>   
#include <stdlib.h>  
#include <WinSock2.h> 
#include <windows.h>
#include <time.h>
#include "TtoCtrlPlatform.h"
#include "TtoCtrlPlatformDlg.h"
#include "Definition.h"
#include "TtoLogin.h"
#include "TaskListHistoryDialog.h"
#include "TaskAndDeviceSetup.h"
#include "WriteSimplePlaceDlg.h"
#include "TtoParamConfig.h"
#include "DlgFeiSuYuan.h"
#include "MidPacketInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "USBKeyDLL.lib")

//MARKEM
char CheckFinishMarkem[14] = {0x02,0x00,0x7e,0x00,0x44,0x00,0x43,0x00,0x31,0x00,0x7c,0x00,0x03,0x00};
char statusWrongResMarkem[14] = {0x02,0x00,0x7e,0x00,0x44,0x00,0x53,0x00,0x30,0x00,0x7c,0x00,0x30,0x00};
char PrintEndResMarkem[18]={0x02,0x00,0x7e,0x00,0x44,0x00,0x56,0x00,0x30,0x00,0x7c,0x00,0x30,0x00,0x7c,0x00,0x03,0x00};
//WDJ
char boxOpenResWDJ[8] = {0x33,0x00,0x30,0x00,0x30,0x00,0x33,0x00};//故障ERR
char statusWrongResWDJ[10] = {0x45,0x00,0x52,0x00,0x53,0x00,0x7C,0x00,0x32,0x00};
char statusWrongRes2WDJ[6] = {0x45,0x00,0x52,0x00,0x52,0x00};
char PrintFullResWDJ[6]={0x45,0x00,0x52,0x00,0x52,0x00};
char PrintEndResWDJ[6]={0x50,0x00,0x52,0x00,0x53,0x00};//PRS 
char PrintEndRes2WDJ[9]={0x00,0x0d,0x00,0x50,0x00,0x52,0x00,0x53,0x00};//PRS 
char ClearErrOkWDJ[6]={0x41,0x00,0x43,0x00,0x4B,0x00};//ACK

map<int,DeviceArr*> CTtoCtrlPlatformDlg::deviceArr;
bool CTtoCtrlPlatformDlg::workFlag = false; //退出为true

Queue<string> CTtoCtrlPlatformDlg::EXESQL;
Queue<string> CTtoCtrlPlatformDlg::UIQUEUE;
HANDLE CTtoCtrlPlatformDlg::exeSqlH;
HANDLE CTtoCtrlPlatformDlg::UiHandle;
string allUseNameSim="";

unsigned int __stdcall CTtoCtrlPlatformDlg::exeSql( void* p ){
  DB<CMySql> dbh(new CMySql(DB_IP, DB_NAME, DB_USER, DB_PASS));
  dbh.ExeSql("SET NAMES 'GBK'");
  
  while (true){
    if (EXESQL.qsize() > 0){
      try{
        string sql = EXESQL.get();
        dbh.ExeSql(sql.c_str());
      }catch(const exception& e){
		  ((CTtoCtrlPlatformDlg*)p)->MessageBox(e.what(),"温馨提示:",MB_SYSTEMMODAL);
      }
    }
	else{	
		if (workFlag) break;
		Sleep(100);
	}
  }
  return 0;
}

unsigned int __stdcall CTtoCtrlPlatformDlg::uiQueue( void* p ){
	string curentDevice="";
    string strCode="";
	string strCode2="";
	string strTotal="";
	string strPrinted="";
	list<string> showList;
	list<string>::iterator it;
    while (true){
		if (UIQUEUE.qsize() > 0){
			try{
				string showStr = UIQUEUE.get();
			    showList = splitPrintStrN(showStr);
				int n=1;
				for ( it=showList.begin() ; it != showList.end(); it++ )
				{
					if (1 == n)
					{
						curentDevice = *it;
					}
					else if (2 == n)
					{
						strCode = *it;
					}
					else if (3 == n)
					{
						strCode2 = *it;
					}
					else if (4 == n)
					{
						strTotal = *it;
					}
					else if (5 == n)
					{
						strPrinted = *it;
					}
					n++;
				}
				if (atoi(curentDevice.c_str()) == ((CTtoCtrlPlatformDlg*)p)->gDevice)
				{
					((CTtoCtrlPlatformDlg*)p)->m_Edit2dCode.SetWindowText(strCode.c_str());
					((CTtoCtrlPlatformDlg*)p)->m_EditCode2.SetWindowText(strCode2.c_str());
					((CTtoCtrlPlatformDlg*)p)->m_printTotal.SetWindowText(strTotal.c_str());
					((CTtoCtrlPlatformDlg*)p)->m_printFinishedNum.SetWindowText(strPrinted.c_str());
				}           
			}catch(const exception& e){
				((CTtoCtrlPlatformDlg*)p)->MessageBox(e.what(),"温馨提示:",MB_SYSTEMMODAL);
			}
		}
		else{	
			if (workFlag) break;
			Sleep(100);
		}
	}
	return 0;
}

CTtoCtrlPlatformDlg::CTtoCtrlPlatformDlg(CWnd* pParent /*=NULL*/)
: CDialog(CTtoCtrlPlatformDlg::IDD, pParent),db(new CMySql(DB_IP,DB_NAME,DB_USER,DB_PASS)){
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	gTask=-1;
	gDevice=-1;
	hideFlag = FALSE;
	deviceBtnShowFlag = true;
	db.ExeSql("SET NAMES GBK");
	taskStats[0] = "未下载";
	taskStats[1] = "正在下载";
	taskStats[2] = "已经下载";
	taskStats[3] = "下载完成";
	taskStats[4] = "下载失败";
	taskStats[5] = "任务非法";
	taskStats[100] = "非溯源";

	deviceStats[0] = "空闲";
	deviceStats[1] = "正在打印";
	memset(usbId,0,sizeof(usbId));
	memset(usbAuth,0,sizeof(usbAuth));
	hInstMaster = NULL;
	gCpyNameSim="";
}

BOOL CTtoCtrlPlatformDlg::initUsb()
{
	//读取加密锁ID  如果失败 就弹提示框 并退出去  
	hInstMaster = LoadLibrary("USBKeyDLL.dll");
	if(!hInstMaster){	
		MessageBox("加载USBKeyDLL.dll失败！","提示：",MB_SYSTEMMODAL); 
		return FALSE;
	}
	KeyOpen = (USBKeyOpen_T*)GetProcAddress(hInstMaster, "USBKeyOpen");
	KeyClose = (USBKeyClose_T*)GetProcAddress(hInstMaster, "USBKeyClose");
	KeyID = (USBKeyID_T*)GetProcAddress(hInstMaster, "USBKeyID");
	KeyAuth = (USBKeyAuth_T*)GetProcAddress(hInstMaster, "USBKeyAuth");
	if (NULL == KeyOpen || NULL == KeyClose || NULL == KeyID || NULL == KeyAuth)
	{
		FreeLibrary(hInstMaster);
		hInstMaster = NULL;
		return FALSE;
	}
	bool openFlag = false;
    
	if (!KeyOpen(comPort,115200)) {
		for(int i=1;i<255;i++)
		{
			if (KeyOpen(i,115200)) {
				memset(usbId,0,sizeof(usbId));
				if (KeyID(usbId)){
					memset(sqlBuff,0,sizeof(sqlBuff));
					sprintf_s(sqlBuff,512,"update table_login set comPort = %d",i);
					db.ExeSql(sqlBuff);
					openFlag = true;
					break;
				}else{
					KeyClose();
				}
			}
		}
	}
	else{
		if (!KeyID(usbId)){
			KeyClose();
			for(int i=1;i<255;i++)
			{
				if (KeyOpen(i,115200)) {
					memset(usbId,0,sizeof(usbId));
					if (KeyID(usbId)){
						memset(sqlBuff,0,sizeof(sqlBuff));
						sprintf_s(sqlBuff,512,"update table_login set comPort = %d",i);
						db.ExeSql(sqlBuff);
						openFlag = true;
						break;
					}else{
						KeyClose();
					}
				}
			}
		}else{
			openFlag = true;
		}
	}
	if (!openFlag)
	{
		KeyClose();
		FreeLibrary(hInstMaster);
		hInstMaster = NULL;
	}

	return openFlag;
}

void CTtoCtrlPlatformDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DEVICE, ListDevice);
	DDX_Control(pDX, IDC_LIST_TASK, ListTask);
	DDX_Control(pDX, IDC_EDIT_PACKET_TOTAL, m_printTotal);
	DDX_Control(pDX, IDC_EDIT_PACKET_PRINTED, m_printFinishedNum);
	DDX_Control(pDX, IDC_EDIT_2D_CODE, m_Edit2dCode);
	DDX_Control(pDX, IDC_EDIT_CODE2, m_EditCode2);
}

BEGIN_MESSAGE_MAP(CTtoCtrlPlatformDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CTtoCtrlPlatformDlg::OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, &CTtoCtrlPlatformDlg::OnBnClickedButtonUpdateDevice)
	ON_MESSAGE(WM_SYSTEMTRAY,&CTtoCtrlPlatformDlg::OnSystemTray)
ON_BN_CLICKED(IDC_BUTTON_TASK_HISTORY, &CTtoCtrlPlatformDlg::OnBnClickedButtonTaskHistory)
ON_BN_CLICKED(IDC_BUTTON_UPDATE_TASK, &CTtoCtrlPlatformDlg::OnBnClickedButtonUpdateTask)
ON_BN_CLICKED(IDC_BUTTON_MIN, &CTtoCtrlPlatformDlg::OnBnClickedButtonMin)
ON_WM_SYSCOMMAND()
ON_BN_CLICKED(IDC_BUTTON_CHANGE_PASS, &CTtoCtrlPlatformDlg::OnBnClickedButtonChangePass)
ON_NOTIFY(NM_RCLICK, IDC_LIST_TASK, &CTtoCtrlPlatformDlg::OnNMRClickListTask)
ON_COMMAND(ID_TASK_FEN_PEI, &CTtoCtrlPlatformDlg::OnTaskFenPei)
ON_COMMAND(ID_TASK_DOWNLOAD, &CTtoCtrlPlatformDlg::OnTaskDownload)
ON_COMMAND(ID_DEVICE_PRINT_START, &CTtoCtrlPlatformDlg::OnDevicePrintStart)
ON_COMMAND(ID_DEVICE_PRINT_STOP, &CTtoCtrlPlatformDlg::OnDevicePrintStop)
ON_NOTIFY(NM_RCLICK, IDC_LIST_DEVICE, &CTtoCtrlPlatformDlg::OnNMRClickListDevice)
ON_COMMAND(ID_TASK_GUAN_BI, &CTtoCtrlPlatformDlg::OnTaskGuanBi)
ON_COMMAND(ID_TASK_QU_XIAO, &CTtoCtrlPlatformDlg::OnTaskQuXiao)
ON_COMMAND(ID_MODEL_SET, &CTtoCtrlPlatformDlg::OnModelSet)
ON_NOTIFY(NM_CLICK, IDC_LIST_DEVICE, &CTtoCtrlPlatformDlg::OnNMClickListDevice)
ON_NOTIFY(NM_CLICK, IDC_LIST_TASK, &CTtoCtrlPlatformDlg::OnNMClickListTask)
ON_WM_CTLCOLOR()
ON_WM_LBUTTONDOWN()
ON_COMMAND(ID_QUXIAO_TASK, &CTtoCtrlPlatformDlg::OnQuxiaoTask)
ON_BN_CLICKED(IDC_BUTTON_ADD_COMM_TASK, &CTtoCtrlPlatformDlg::OnBnClickedButtonAddCommTask)
ON_COMMAND(ID_EDIT_TASK, &CTtoCtrlPlatformDlg::OnEditTask)
ON_COMMAND(ID_MID_PACK, &CTtoCtrlPlatformDlg::OnMidPack)
ON_COMMAND(ID_OUT_SYSTEM, &CTtoCtrlPlatformDlg::OnOutSystem)
ON_COMMAND(ID_BACK_SYSTEM, &CTtoCtrlPlatformDlg::OnBackSystem)
END_MESSAGE_MAP()

BOOL CTtoCtrlPlatformDlg::CheckTaskIdInUseOrNot(const char* taskId)  
{ 
	if (NULL == taskId)
	{
		return FALSE;
	}
	iter = db.ExeSql("select tabletstorid from table_device");
	while(iter.HasMore()){
		if ((int)iter.GetData(0) == atoi(taskId))
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CTtoCtrlPlatformDlg::Get2dcodeTotalAndPrintNum(const char* taskId,int& totalNum,int& printNum)  
{   
	if (NULL == taskId)
	{
		return FALSE;
	}
	memset(sqlBuff,0,sizeof(sqlBuff));
	sprintf_s(sqlBuff,512,"select tabletboxnum,tabletPrintedNum from table_task where tabletstorid  = %s",taskId);
	iter = db.ExeSql(sqlBuff);
	if(!iter.HasMore()){
		MessageBox("取溯源码数和已经打印的数目失败！","温馨提示：",MB_SYSTEMMODAL);   
		return FALSE;
	}
	totalNum = (int)iter.GetData(0);
	printNum = (int)iter.GetData(1);
	return TRUE;
}


BOOL CTtoCtrlPlatformDlg::GetTotalNumAndFinishNum(const string& taskId,string& totalNum,string& finishNum)
{
	if ("" == taskId)
	{
		return false;
	}
	memset(sqlBuff,0,sizeof(sqlBuff));
	sprintf_s(sqlBuff,512,"select cast(tabletboxnum as char),cast(tabletPrintedNum as char) from table_task where tabletstorid =%d",atoi(taskId.c_str()));
	iter = db.ExeSql(sqlBuff);
	if(iter.HasMore()){
		totalNum=(const char*)iter.GetData(0);
		finishNum = (const char*)iter.GetData(1);
		return TRUE;
	}
	return FALSE;
}

BOOL CTtoCtrlPlatformDlg::GetTaskFromDB(){
	try{
		iter = db.ExeSql("select CAST(tabletstorid as char),taskFlag,tabletname,CAST(tabletnumweight as char),tabletspec,medaddress,tabletnumber,createtime,CAST(tabletboxnum as char),medPlaceSimple,tablet1code,tabletitemnum from table_task order by tabletstorid asc");
		int iTask = 0;
		char tmpTaskId[16]={0};
		int fsyTaskNum=0;
		while(iter.HasMore()){
			memset(tmpTaskId,0,sizeof(tmpTaskId));
				sprintf_s(tmpTaskId,16,"%s",(const char*)iter.GetData(0));
				if ( '-' == tmpTaskId[0]){
					fsyTaskNum++;
				}
				int nrow = ListTask.InsertItem(iTask++, tmpTaskId);//任务号
				ListTask.SetItemText(nrow, 1, taskStats[(int)iter.GetData(1)].c_str());//设置数据//状态
				ListTask.SetItemText(nrow, 2, (const char *)iter.GetData(2));//品名
				ListTask.SetItemText(nrow, 3, (const char *)iter.GetData(6));//批次
				string itemnum = (string)iter.GetData(11);//区分中包字段
				if (atoi(itemnum.c_str()) != 0)//中包显示
				{
					ListTask.SetItemText(nrow, 4, itemnum.c_str());//有中包数的情况
				}
				else//小包显示
					ListTask.SetItemText(nrow, 4, (const char *)iter.GetData(8));//打印包数

				ListTask.SetItemText(nrow, 5, (const char *)iter.GetData(3));//重量
				ListTask.SetItemText(nrow, 6, (const char *)iter.GetData(4));//规格
				ListTask.SetItemText(nrow, 7, (const char *)iter.GetData(5));//产地
				ListTask.SetItemText(nrow, 8, (const char *)iter.GetData(9));//产地简称			
				ListTask.SetItemText(nrow, 9, (const char *)iter.GetData(7));//日期
				ListTask.SetItemText(nrow, 10, (const char *)iter.GetData(10));//条形码	
		}		
		if (0 == fsyTaskNum)
		{
			iter = db.ExeSql("update table_login set fsyNum=0");
		}
	}catch(...){
		return false;
	}
	return true;
} 

bool CTtoCtrlPlatformDlg::ParseDeviceJson(HWND hwind,const char* jsonBuf,int jsonLen)
{
	size_t outLen = 0;
	if(NULL == jsonBuf || jsonLen <= 2) return false;
	cJSON *jsonArray = cJSON_Parse(jsonBuf);
	if (NULL == jsonArray){
		MessageBox("获取设备列表所有数据失败!","温馨提示：",MB_SYSTEMMODAL);
		return false;
	}
	int jNum = cJSON_GetArraySize(jsonArray);
	if (0 == jNum){
		MessageBox("没有数据","温馨提示：",MB_SYSTEMMODAL);
		if (NULL != jsonArray){
			cJSON_Delete(jsonArray);
		}
		return false;
	}
	cJSON *root = NULL;
	iter = db.ExeSql("select IFNULL(max(updateFlag),0)+1 from table_device");
	int updateFlag = 0;
	if( iter.HasMore() ){
		updateFlag = iter.GetData(0);
	}
	for (int i = 0;i < jNum;i++){	
		root = cJSON_GetArrayItem(jsonArray,i);
		if (NULL == root){
			MessageBox("获取设备列表单条记录失败 !","温馨提示：",MB_SYSTEMMODAL);
			break;
		}
		string deviceId = cJSON_GetObjectItem(root,"devicetag")->valuestring;  
		if (deviceId.length() == 0){
			MessageBox("获取设备列表deviceid失败 !","温馨提示：",MB_SYSTEMMODAL);
			break;
		}
		string deviceIp = cJSON_GetObjectItem(root,"deviceip")->valuestring;  
		if (deviceIp.length() == 0){
			MessageBox("获取设备列表deviceip失败 !","温馨提示：",MB_SYSTEMMODAL);
			break;
		}
		string devicePort = cJSON_GetObjectItem(root,"deviceport")->valuestring;  
		if (devicePort.length() == 0){
			MessageBox("获取设备列表deviceport失败 !","温馨提示：",MB_SYSTEMMODAL);
			break;
		}
		int deviceType = cJSON_GetObjectItem(root,"devicetype")->valueint;  
		if (deviceType <= 0){
			MessageBox("获取设备列表deviceType失败 !","温馨提示：",MB_SYSTEMMODAL);
			break;
		}
		string tmpdevicenickname = cJSON_GetObjectItem(root,"devicenickname")->valuestring;  
		if (0 == tmpdevicenickname.length()){
			MessageBox("获取设备列表devicenickname失败 !","温馨提示：",MB_SYSTEMMODAL);
			break;
		}
		string devicenickname="";
		UTF8ToGB2132(tmpdevicenickname,devicenickname);
		memset(sqlBuff,0,sizeof(sqlBuff));
    	sprintf_s(sqlBuff,512,"insert into table_device(devicetag,ip,port,devicenickname,devicetype,updateFlag) values ('%s','%s',%s,'%s',%d,%d) on duplicate key update ip='%s',port=%s,devicenickname='%s',devicetype=%d,updateFlag=%d ",
			deviceId.c_str(),
			deviceIp.c_str(),
			devicePort.c_str(),		
			devicenickname.c_str(),
			deviceType,
			updateFlag,
			deviceIp.c_str(),
			devicePort.c_str(),		
			devicenickname.c_str(),
			deviceType,
			updateFlag);
		if( -1 == (int)db.ExeSql(sqlBuff) ){
			MessageBox("设备列表插入失败！","温馨提示：",MB_SYSTEMMODAL);   
			return FALSE;
		}
		int nrow = 0;
        nrow = ListDevice.InsertItem(i, deviceId.c_str());//设备ID
		ListDevice.SetItemText(nrow, 1, devicenickname.c_str());//设备名称
		char strTaskId[32]={0};
		int taskId = 0;
		if (Device_Task(deviceId,taskId)){
			memset(strTaskId,0,sizeof(strTaskId));
			sprintf_s(strTaskId,32,"%d",taskId);
			ListDevice.SetItemText(nrow, 2, strTaskId);//打印任务
		}
		if(0 != taskId){
			ListDevice.SetItemText(nrow, 3, TASK_HAVE_MATCH);//已分配
		}
		else
			ListDevice.SetItemText(nrow, 3, TASK_NOT_MATCH);//未分配
		ListDevice.SetItemText(nrow, 4, deviceStats[0].c_str());//设置数据		
	}
	if (NULL != jsonArray){
		cJSON_Delete(jsonArray);
	}
	memset(sqlBuff,0,sizeof(sqlBuff));
	sprintf_s(sqlBuff,512,"delete from table_device where updateFlag<%d",updateFlag);
	if( -1 == (int)db.ExeSql(sqlBuff) ){
		MessageBox("删除没有使用的设备列表项失败！","温馨提示：",MB_SYSTEMMODAL);   
		return FALSE;
	}	
	return true;
}

BOOL CTtoCtrlPlatformDlg::DelOneFromTaskList( const char* taskId ){
	if (NULL == taskId)
	{
		return FALSE;
	}
	memset(sqlBuff,0,sizeof(sqlBuff));
	sprintf_s(sqlBuff,512,"delete from table_task where tabletstorid  = %s",taskId);	
	return (-1 != (int)db.ExeSql(sqlBuff));
}

BOOL CTtoCtrlPlatformDlg::GetDeviceListFromDB()
{
	try{
		iter = db.ExeSql("select devicetag,devicenickname,cast(tabletstorid as char) from table_device");
		int i=0;
		int nrow = -1;
		while(iter.HasMore()){
			nrow = ListDevice.InsertItem(i++, (const char *)iter.GetData(0));//设备ID
			ListDevice.SetItemText(nrow, 1, (const char *)iter.GetData(1));//设备名称
			
			string taskId = iter.GetData(2);
			if( taskId != "0" ){
				ListDevice.SetItemText(nrow, 2, taskId.c_str());//打印任务
				ListDevice.SetItemText(nrow,3,TASK_HAVE_MATCH);//任务分配
			}
			else{
				ListDevice.SetItemText(nrow,3,TASK_NOT_MATCH);//任务分配
			}
			ListDevice.SetItemText(nrow, 4, "空闲");//打印状态
		} 
	}
	catch (...){return FALSE;}
	return TRUE;
}

BOOL CTtoCtrlPlatformDlg::OnInitDialog(){
	CDialog::OnInitDialog();
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	ModifyStyle(0,WS_SYSMENU);

    CTtoLogin ttoLogin;
    ttoLogin.DoModal();
	
	if(!ttoLogin.passFlag){
		OnCancel();
		return FALSE;
	}
	//===================================================
	iter = db.ExeSql("select orgId,taskServerIp,taskServerPort,httpServerIp,httpServerPort,comPort,cpyNameSim,timeType,weightType from table_login");
	if(iter.HasMore()){
		orgid = (const char*)iter.GetData(0);
		taskIP= (const char*)iter.GetData(1);
		taskPort = (int)iter.GetData(2);
		deviceIP= (const char*)iter.GetData(3);
		devicePort = (int)iter.GetData(4);
		comPort = (int)iter.GetData(5);
		gCpyNameSim = (const char*)iter.GetData(6);
		timeType = (int)iter.GetData(7);
		weightType = (int)iter.GetData(8);
		allUseNameSim = gCpyNameSim;
		if ("" == orgid || "" == taskIP || 0 == taskPort || "" == deviceIP 
			/*|| 0 == devicePort*/ || "" == gCpyNameSim || 1 > timeType || 3 < timeType || weightType < 0 ||weightType > 2)
		{
			MessageBox("请先用TtoConfigTool进行基本信息配置，比如：orgid、IP、PORT等！","温馨提示:",MB_SYSTEMMODAL);
			OnCancel(); 
			return FALSE;
		}
	}else{	
		MessageBox("打开数据库表table_login失败！","温馨提示:",MB_SYSTEMMODAL);
		OnCancel(); 
		return FALSE;
	}
////////////////////////////////
	//if (!initUsb())
	//{
	//	MessageBox("初始化加密锁失败！","温馨提示:",MB_SYSTEMMODAL);
	//	OnCancel();
	//	return FALSE;
	//}
////////////////////////////////cyy		
	//托盘处理
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof( NOTIFYICONDATA );
	nid.hWnd = m_hWnd; // handle of window that will receive
	nid.uID = IDR_MAINFRAME; // id for this icon
	nid.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP;
	nid.uCallbackMessage = WM_SYSTEMTRAY;
	nid.hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
	strncpy_s(nid.szTip, "溯源打码控制平台",strlen("溯源打码控制平台"));
	::Shell_NotifyIcon( NIM_ADD,&nid );
	::SetWindowLong ( GetSafeHwnd () , GWL_EXSTYLE , WS_EX_TOOLWINDOW );

	m_brush.CreateSolidBrush(RGB(MY_BACK_START,MY_BACK_MID,MY_COLOR_END)); // 生成一刷子
	//设备
	LONG lStyle;
	lStyle = GetWindowLong(ListDevice.m_hWnd, GWL_STYLE);//获取当前窗口style
	lStyle &= ~LVS_TYPEMASK; //清除显示方式位
	lStyle |= LVS_REPORT; //设置style
	SetWindowLong(ListDevice.m_hWnd, GWL_STYLE, lStyle);//设置style

	DWORD dwStyle = ListDevice.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl） 
	dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl） 
	ListDevice.SetExtendedStyle(dwStyle); //设置扩展风格

	ListDevice.SetBkColor(RGB(247,247,255)); 
	ListDevice.SetTextColor(RGB(0,0,255));
	ListDevice.SetTextBkColor(RGB(247,247,255)); 
	
	SetWindowLong(ListTask.m_hWnd, GWL_STYLE, lStyle);
	dwStyle = ListTask.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;
	dwStyle |= LVS_EX_GRIDLINES;
	ListTask.SetExtendedStyle(dwStyle); 

 	ListTask.SetBkColor(RGB(247,247,255)); 
 	ListTask.SetTextColor(RGB(0,0,255)); 
 	ListTask.SetTextBkColor(RGB(247,247,255)); 

	m_Fontbs.CreateFont(16,7,0,0,FW_HEAVY,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,"黑体");
	m_printTotal.SetFont(&m_Fontbs,FALSE);
	m_printFinishedNum.SetFont(&m_Fontbs,FALSE);
	m_Edit2dCode.SetFont(&m_Fontbs,FALSE);
	m_EditCode2.SetFont(&m_Fontbs,FALSE);

	ListTask.InsertColumn( 0, "任务编号", LVCFMT_RIGHT,0 );
	ListTask.InsertColumn( 1, "任务状态", LVCFMT_LEFT, 60 );
	ListTask.InsertColumn( 2, "品名", LVCFMT_LEFT, 60);
	ListTask.InsertColumn( 3, "批次", LVCFMT_LEFT, 80 );
	ListTask.InsertColumn( 4, "包数", LVCFMT_LEFT, 50 );
	ListTask.InsertColumn( 5, "重量(g)", LVCFMT_LEFT, 60 );
	ListTask.InsertColumn( 6, "规格", LVCFMT_LEFT, 80 );
	ListTask.InsertColumn( 7, "产地详情", LVCFMT_LEFT, 170 );
	ListTask.InsertColumn( 8, "产地简称", LVCFMT_LEFT, 60 );	
	ListTask.InsertColumn( 9, "日期", LVCFMT_LEFT, 137 );
	ListTask.InsertColumn( 10, "商品码", LVCFMT_LEFT, 100 );
	
	ListDevice.InsertColumn( 0, "设备编号", LVCFMT_LEFT,0 );
	ListDevice.InsertColumn( 1, "设备名称", LVCFMT_LEFT,170 );
	ListDevice.InsertColumn( 2, "任务编号",LVCFMT_LEFT,0);
	ListDevice.InsertColumn( 3, "分配任务",LVCFMT_LEFT,140);
	ListDevice.InsertColumn( 4, "设备状态", LVCFMT_LEFT, 507);

	if(!GetTaskFromDB()) MessageBox("从数据库读取任务列表信息失败!","提示信息：",MB_SYSTEMMODAL);
	if (!GetDeviceListFromDB())	MessageBox("从数据库读取设备列表信息失败!","提示信息：",MB_SYSTEMMODAL);
	exeSqlH = (HANDLE)_beginthreadex(NULL, 0, exeSql, this, 0, 0);
	UiHandle = (HANDLE)_beginthreadex(NULL, 0,uiQueue,this, 0, 0);

    
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CTtoCtrlPlatformDlg::OnPaint()
{
	CPaintDC dc(this); 
	CRect   rect;   
	GetClientRect(&rect);   
	CDC   dcMem;   
	dcMem.CreateCompatibleDC(&dc);   
	CBitmap   bmpBackground;   
	bmpBackground.LoadBitmap(IDB_BITMAP_MAIN);
	BITMAP   bitmap;   
	bmpBackground.GetBitmap(&bitmap);   
	CBitmap   *pbmpOld=dcMem.SelectObject(&bmpBackground);   
	dc.StretchBlt(0,0,rect.Width(),rect.Height(),&dcMem,0,0,bitmap.bmWidth,bitmap.bmHeight,SRCCOPY); 

	((CButton *)GetDlgItem(IDC_BUTTON_MIN))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_MAIN_SMALL)));  
	((CButton *)GetDlgItem(IDC_BUTTON_CANCEL))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_MAIN_QUIT))); 
	((CButton *)GetDlgItem(IDC_BUTTON_TASK_HISTORY))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_CX_HISTORY)));  
	((CButton *)GetDlgItem(IDC_BUTTON_UPDATE_TASK))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_TASK)));  
	((CButton *)GetDlgItem(IDC_BUTTON_ADD_COMM_TASK))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_ADD_PT_TASK)));
	((CButton *)GetDlgItem(IDC_BUTTON_UPDATE_DEVICE))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_DEVICE)));
	((CButton *)GetDlgItem(IDC_BUTTON_CHANGE_PASS))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_XG_PASS)));

}

HCURSOR CTtoCtrlPlatformDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTtoCtrlPlatformDlg::DeleteTray()
{
	NOTIFYICONDATA nid;
	nid.cbSize=(DWORD)sizeof(NOTIFYICONDATA);
	nid.hWnd=this->m_hWnd;
	nid.uID=IDR_MAINFRAME;
	nid.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP ;
	nid.uCallbackMessage=WM_SYSTEMTRAY;
	nid.hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));
	strncpy_s(nid.szTip,"溯源打码控制平台",strlen("溯源打码控制平台")); 
	Shell_NotifyIcon(NIM_DELETE,&nid); 
}
template<class T,typename C>  
std::vector<T> SplitChar(T& str,C c)  
{  
	std::vector<T> temp;  
	typedef std::basic_stringstream<C, std::char_traits<C>,std::allocator<C> > mstringstream;  
	mstringstream mss;  
	mss << str;  
	T s;  
	while(getline(mss,s,c))  
	{  
		temp.push_back(s);  
	}  
	return temp;  
} 
std::vector<std::string> split(std::string str,std::string pattern)  
{  
	std::string::size_type pos;  
	std::vector<std::string> result;  
	str+=pattern;//扩展字符串以方便操作  
	int size=str.size();  

	for(int i=0; i<size; i++)  
	{  
		pos=str.find(pattern,i);  
		if(pos<size)  
		{  
			std::string s=str.substr(i,pos-i);  
			result.push_back(s);  
			i=pos+pattern.size()-1;  
		}  
	}  
	return result;  
} 

void CTtoCtrlPlatformDlg::OnBnClickedButtonUpdateDevice()
{
	if (!deviceBtnShowFlag)
	{
		return;
	}
	deviceBtnShowFlag=false;
	((CButton *)GetDlgItem(IDC_BUTTON_UPDATE_DEVICE))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_DEVICE_YINCANG)));

	if(MessageBox("是否确定更新设备列表？","设备更新提示：",MB_YESNO) == IDNO) return;
	////////////////////////////////////////////
Socket so;
	
	so.CInit(taskIP,taskPort);
	so.CCreateSocket();
	string sendcommand="GetDevice "+orgid;
	char recvBuff[0x100]={0};
	try{
		if(!so.CConnect()) throw exception("网络不通");
		if(!so.Send(sendcommand)) throw exception("请求失败");
		
		int recvInt = 0,allRecvLen = 0;
		/*while(true){
			if ( !so.Recv(recvBuff+allRecvLen,0x400,recvInt) ) break;
			allRecvLen += recvInt;
		}*/
		so.Recv(recvBuff,0x400);
		//recvBuff[allRecvLen]=0x00;
		so.CShutdownSocket();
		so.CCloseSocket();
	}catch(const exception& e){
		so.CShutdownSocket();
		so.CCloseSocket();
		MessageBox(e.what()) ;
	}
	int updateFlag = 0;
	if (strlen(recvBuff)>20){
		ListDevice.DeleteAllItems();
		string recvString=recvBuff;
		iter = db.ExeSql("select IFNULL(max(updateFlag),0)+1 from table_device");
		
		if( iter.HasMore() ){
			updateFlag = iter.GetData(0);
		}
		std::vector<std::string> arr1=split(recvString,"|");
		for(int i = 0;i < arr1.size();++i)  
		{ 
			if (arr1[i].length()<1)
			{
				break;
			}
			std::vector<std::string> arr2=split(arr1[i],"^");
			sprintf_s(sqlBuff,512,"insert into table_device(devicetag,ip,port,devicenickname,devicetype,updateFlag) values ('%s','%s',%s,'%s',%d,%d) on duplicate key update ip='%s',port=%s,devicenickname='%s',devicetype=%d,updateFlag=%d ",
				arr2[0].c_str(),
				arr2[1].c_str(),
				arr2[2].c_str(),		
				arr2[3].c_str(),
				atoi(arr2[4].c_str()),
				updateFlag,
				arr2[1].c_str(),
				arr2[2].c_str(),		
				arr2[3].c_str(),
				atoi(arr2[4].c_str()),
				updateFlag);
			if( -1 == (int)db.ExeSql(sqlBuff) ){
				MessageBox("设备列表插入失败！","温馨提示：",MB_SYSTEMMODAL);   
				//return FALSE;
			}
			int nrow = 0;
			nrow = ListDevice.InsertItem(i, arr2[0].c_str());//设备ID
			ListDevice.SetItemText(nrow, 1, arr2[3].c_str());//设备名称
			char strTaskId[32]={0};
			int taskId = 0;
			if (Device_Task(arr2[0],taskId)){
				memset(strTaskId,0,sizeof(strTaskId));
				sprintf_s(strTaskId,32,"%d",taskId);
				ListDevice.SetItemText(nrow, 2, strTaskId);//打印任务
			}
			if(0 != taskId){
				ListDevice.SetItemText(nrow, 3, TASK_HAVE_MATCH);//已分配
			}
			else
				ListDevice.SetItemText(nrow, 3, TASK_NOT_MATCH);//未分配
			ListDevice.SetItemText(nrow, 4, deviceStats[0].c_str());//设置数据		

			
		}  
	}
	else{
		MessageBox("更新设备列表失败 !","温馨提示：",MB_SYSTEMMODAL);
		if (deviceArr.size() == 0){
			deviceBtnShowFlag=true;
			((CButton *)GetDlgItem(IDC_BUTTON_UPDATE_DEVICE))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_DEVICE)));
		}
		return;
	}
	sprintf_s(sqlBuff,512,"delete from table_device where updateFlag<%d",updateFlag);
	if( -1 == (int)db.ExeSql(sqlBuff) ){
		MessageBox("删除没有使用的设备列表项失败！","温馨提示：",MB_SYSTEMMODAL);   
		return;
	}	
	
	MessageBox("更新设备列表完成 !","温馨提示：",MB_SYSTEMMODAL);
	if (deviceArr.size() == 0){
		deviceBtnShowFlag=true;
		((CButton *)GetDlgItem(IDC_BUTTON_UPDATE_DEVICE))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_DEVICE)));
	}
	return;
	////////////////////////////////////////////
	char sendBuf[0x200]={0};
	sprintf_s(sendBuf,0x200,"http://%s:%d/zycbs/swap!swapCommands.shtml?request={\"commandid\":\"1\",\"orgid\":\"%s\"}",deviceIP.c_str(),devicePort,orgid.c_str());
	Sleep(5);
	string recvbuf = "";
	recvbuf = UrlProcess(sendBuf);
	if ( "" == recvbuf )  {
		memset(sendBuf,0,sizeof(sendBuf));
		sprintf_s(sendBuf,0x200,"连接%s:%d失败,请检查网络或服务器是否开启!",deviceIP.c_str(),devicePort);
		MessageBox(sendBuf,"连接服务器提示：",MB_SYSTEMMODAL);
		if (deviceArr.size() == 0){
			deviceBtnShowFlag=true;
			((CButton *)GetDlgItem(IDC_BUTTON_UPDATE_DEVICE))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_DEVICE)));
		}
		return ;
	}
	if ( "[]" == recvbuf )  {
		MessageBox("向服务器请求数据失败！","数据请求：",MB_SYSTEMMODAL);
		if (deviceArr.size() == 0){
			deviceBtnShowFlag=true;
			((CButton *)GetDlgItem(IDC_BUTTON_UPDATE_DEVICE))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_DEVICE)));
		}
		return ;
	}
	ListDevice.DeleteAllItems();
	bool retParse = ParseDeviceJson(NULL,recvbuf.c_str(),recvbuf.length());
	if (!retParse)
	{
		MessageBox("更新设备列表失败 !","温馨提示：",MB_SYSTEMMODAL);
	}
	MessageBox("更新设备列表完成 !","温馨提示：",MB_SYSTEMMODAL);
	if (deviceArr.size() == 0){
		deviceBtnShowFlag=true;
		((CButton *)GetDlgItem(IDC_BUTTON_UPDATE_DEVICE))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_DEVICE)));
	}
}

void CTtoCtrlPlatformDlg::OnBnClickedButtonTaskHistory()
{
	CTaskListHistoryDialog taskHistoryDlg;
	taskHistoryDlg.DoModal();
}

string CTtoCtrlPlatformDlg::testsend(string& taskIP,int taskPort,char* usbId)
{
	string retString = "";
	size_t bufferLen=0;
	unsigned char* buffer = (unsigned char*)malloc(0x400);//解压后的空间
	if( NULL == buffer ){
		return "";
	}    
	sprintf((char*)buffer,"GETRANDOM %s",usbId);//新接口
	retString = GetDownData((char *)buffer,taskIP.c_str(),taskPort,&buffer,bufferLen);	
	if ( retString != "OK" || 0 == bufferLen){
		free(buffer);
		buffer = NULL;
		MessageBox(retString.c_str(),"提示：",MB_SYSTEMMODAL);
		return "";
	}
	buffer[bufferLen]=0x00;
	char tmpAuth[128]={0};
	strncpy_s(tmpAuth,(char *)buffer,127);
	retString = tmpAuth;
	free(buffer);
	buffer = NULL;
	return retString;
}

void CTtoCtrlPlatformDlg::OnBnClickedButtonUpdateTask(){
	if(MessageBox("是否确定更新任务列表？","任务更新提示：",MB_YESNO) == IDNO) return;

	string taskId="12345";
	string onewayPass="";

	//onewayPass = SentUsbIdToServer(taskIP,taskPort,taskId,usbId);//通过USBID请求pass 
	////////////////////
	//if ("" == onewayPass){
	//	return;
	//}
	//if (!KeyAuth(usbAuth,onewayPass.c_str())){
	//	MessageBox("加密锁认证失败！","提示：",MB_SYSTEMMODAL); 
	//	return ;
	//}
	/////////////////////
	unsigned char* unzip_buffer = (unsigned char*)malloc(0x400*0x400*5); //接收源数据
	if( NULL == unzip_buffer ){
		MessageBox("内存不足","温馨提示：",MB_SYSTEMMODAL);
		return;
	}

	size_t bufferLen=0;
	//sprintf((char*)unzip_buffer,"GETTASKS %s %s %s",orgid.c_str(),usbId,usbAuth);
	sprintf((char*)unzip_buffer,"GETTASKS %s 555 15156156",orgid.c_str(),usbId,usbAuth);
	string retString = GetDownData((char*)unzip_buffer,taskIP.c_str(),taskPort,&unzip_buffer,bufferLen);
	if ( retString != "OK" || 0 == bufferLen){
		if (0 != bufferLen){
			MessageBox(retString.c_str(),"温馨提示：",MB_SYSTEMMODAL);
		}
		else
			MessageBox("没有新任务！","温馨提示：",MB_SYSTEMMODAL);
		free(unzip_buffer);
		unzip_buffer = NULL;
		return;
	}
	unsigned char* gbkBuffer = (unsigned char*)malloc(bufferLen*2);
	if( NULL == gbkBuffer ){
		MessageBox("转码空间不足！","温馨提示：",MB_SYSTEMMODAL);
		free(unzip_buffer);
		unzip_buffer = NULL;
		return;
	}

	int nRet = UTF8ToGBK(unzip_buffer,gbkBuffer,bufferLen*2);
	free(unzip_buffer);
	unzip_buffer = NULL;
	if( 0 == nRet ){
		free(gbkBuffer);
		gbkBuffer = NULL;
		MessageBox("转码失败！","温馨提示：",MB_SYSTEMMODAL);
		return;
	}
	*(gbkBuffer+nRet)=0x00;
	//============================================
	char* p = (char *)gbkBuffer;
	char* q = NULL;
	const char *sqlSour = "insert into table_task(tabletstorid,tabletname,tabletnumweight,tabletboxnum,tabletitemnum,tabletnumber,tabletspec,createtime,medaddress) values (%s)";
	char sqlBuff[0x400];
	while(true){
		char sqlDest[0x400]={0};
		char* m = sqlDest;
		int i = 0;
		char tmpNum[32] = {0};
		for( i = 0; i < 9; i++ ){
			q = strstr(p,"|");
			if( NULL == q ) break;
			int rl = q - p;
			if ( 0 == i || 2 == i || 3 == i){
				if (2 == i)
				{
					memcpy(tmpNum,p,rl);
					int n = atoi(tmpNum);
					n = n/10;
					sprintf(tmpNum,"%d",n);
					memcpy(m,tmpNum,strlen(tmpNum));
					m += strlen(tmpNum);
				}
				else if (3 == i)
				{
					memset(tmpNum,0,sizeof(tmpNum));
					memcpy(tmpNum,p,rl);
					memcpy(m,tmpNum,strlen(tmpNum));
					m += strlen(tmpNum);
				}
				else{
					memcpy(m,p,rl);
					m += rl;
				}
				*m++ = ',';
			}else{
				*m++ = '\'';
				char tmpitemnum[128]={0};
				memcpy(tmpitemnum,p,rl);
				if (4 == i && atoi(tmpitemnum) != 0)
				{
					sprintf(tmpNum+strlen(tmpNum),"*%s",tmpitemnum);
					int len = strlen(tmpNum);
					memcpy(m,tmpNum,len);
					m+=len;
				}
				else
				{
					memcpy(m,p,rl);
					m += rl;
				}
				*m++ = '\'';
				*m++ = ',';
			}
			p += rl + 1;
		}
		if( i < 9 ) break;
		*(--m) = 0;
		memset(sqlBuff,0,sizeof(sqlBuff));
		sprintf_s(sqlBuff,0x400,sqlSour,sqlDest);
		db.ExeSql(sqlBuff);
	}
	free(gbkBuffer);
	gbkBuffer = NULL;
	ListTask.DeleteAllItems();
	GetTaskFromDB();
}

BOOL CTtoCtrlPlatformDlg::SetTaskListFlag(const char* taskId,int taskFinishFlag)
{
	memset(sqlBuff,0,sizeof(sqlBuff));
	sprintf_s(sqlBuff,512,"update table_task SET taskFlag = %d where tabletstorid = %s",taskFinishFlag,taskId);
	return ( -1 != (int)db.ExeSql(sqlBuff) );
}

BOOL CTtoCtrlPlatformDlg::DisMatchTaskFromDevice(string taskId)
{
	if ("" == taskId)
	{
		return FALSE;
	}
	memset(sqlBuff,0,sizeof(sqlBuff));
	sprintf_s(sqlBuff,512,"update table_device set tabletstorid = 0 where tabletstorid = %s",taskId.c_str());
	if (-1 == (int)db.ExeSql(sqlBuff)){
		MessageBox("取消设备任务失败！","温馨提示：",MB_SYSTEMMODAL); 
		return FALSE;
	}
	return TRUE;
}

string CTtoCtrlPlatformDlg::GetDownData(IN const char* sendCommand, IN const char* ip, IN int port, OUT unsigned char** buffer, OUT size_t& bufferLen) const{
	Socket so;
	so.CInit(ip,port);
	so.CCreateSocket();
	try{
		if(!so.CConnect()) throw exception("网络不通");
		if(!so.Send(sendCommand)) throw exception("请求失败");
		char recvBuff[0x400*0x200];
		int recvInt = 0,allRecvLen = 0;
		while(true){
			if ( !so.Recv(recvBuff+allRecvLen,0x400,recvInt) ) break;
			allRecvLen += recvInt;
		}
		recvBuff[allRecvLen]=0x00;
		if (allRecvLen <= 36){
			if ( strcmp(recvBuff,"1") == 0){
				throw exception("已经下载");
			}else if (strcmp(recvBuff,"0") == 0){
				throw exception("任务非法");
			}else if (36 == allRecvLen){
				memcpy(*buffer,recvBuff,allRecvLen);
				bufferLen = allRecvLen;
				throw exception("OK");
			}else if (strstr(recvBuff,"1234567890") != NULL){
				memcpy(*buffer,recvBuff,allRecvLen-8);
				bufferLen = allRecvLen-8;
				throw exception("OK");
			}else if (9 == allRecvLen){
				throw exception("请先检查设备列表是否更新或者加密锁是否配置正确,如有疑问,请联系服务商!");
			}else if (0 == allRecvLen){
				throw exception("程序调整中，请联系服务商！");
			}
			else if (strcmp("password ok",recvBuff) == 0)
			{
				throw exception("OK");
			}
			else if (strcmp("password error",recvBuff) == 0)
			{
				throw exception("Ukey验证失败！");
			}
			else throw exception("下载失败");
		}
		LodePNG_DecompressSettings setting;
		setting.ignoreAdler32 = 0;
		int ret = LodePNG_zlib_decompress(buffer, &bufferLen, (unsigned char*)recvBuff, allRecvLen, &setting);
		if( ret != 0 ) throw exception("解压失败");
		*(*buffer+bufferLen) = 0;

		so.CShutdownSocket();
		so.CCloseSocket();
	}catch(const exception& e){
		so.CShutdownSocket();
		so.CCloseSocket();
		return e.what();
	}
	return "OK";
}

void CTtoCtrlPlatformDlg::OnBnClickedButtonMin()
{
	ShowWindow(SW_HIDE);
	hideFlag = TRUE;
}

void CTtoCtrlPlatformDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if(nID == SC_MINIMIZE) {
		this->ShowWindow(FALSE);
	}
	else{
		CDialog::OnSysCommand(nID, lParam);
	}
}

LRESULT CTtoCtrlPlatformDlg::OnSystemTray(WPARAM wParam, LPARAM lParam)
{
	if ( wParam == IDR_MAINFRAME ){
		switch( lParam )
		{
		case WM_LBUTTONDOWN:            //左键点击托盘图标显示窗口
			{
				if (hideFlag){
					this->ShowWindow(SW_NORMAL);
					hideFlag = FALSE;
				}
				else{
					this->ShowWindow(SW_MINIMIZE);
					hideFlag = TRUE;
				}
				break;
			}
		case WM_RBUTTONDOWN:
			{
				CMenu menu;   //定义右键菜单对象
				POINT pt;
				GetCursorPos(&pt);   //获取当前鼠标位置
				menu.LoadMenu(IDR_TUOPAN_MENU);   //载入右键快捷菜单
				SetForegroundWindow();//放置在前面
				CMenu* pmenu;    //定义右键菜单指针
				pmenu=menu.GetSubMenu(0);      //该函数取得被指定菜单激活的下拉式菜单或子菜单的句柄
				ASSERT(pmenu!=NULL);      
				pmenu-> TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN,pt.x,pt.y,this);   //在指定位置显示右键快捷菜单
				HMENU hmenu=pmenu -> Detach();  
				pmenu ->DestroyMenu(); 
				break;
			}
		default:
			break;
		}
	}
	return 1;
}

BOOL CTtoCtrlPlatformDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg -> message == WM_KEYDOWN)
	{
		if(pMsg -> wParam == VK_ESCAPE)
			return TRUE;
		if(pMsg -> wParam == VK_RETURN)
			return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CTtoCtrlPlatformDlg::OnBnClickedButtonChangePass()
{
	CModifyPassDlg ModifyDlg;
	ModifyDlg.DoModal();
}

void CTtoCtrlPlatformDlg::OnNMRClickListTask(NMHDR *pNMHDR, LRESULT *pResult)
{
	LMyPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LMyPNMITEMACTIVATE>(pNMHDR);	
	if(pNMItemActivate->iItem != -1)
	{
		ListTask.SetItemState(-1, 0, -1); 
		ListDevice.SetItemState(-1, 0, -1);
		ListTask.SetItemState(pNMItemActivate->iItem,LVIS_DROPHILITED, LVIS_DROPHILITED); 

		string sTask = (string)ListTask.GetItemText(pNMItemActivate->iItem,0);
		if (sTask.length() == 0){
			MessageBoxA("读取任务失败!","温馨提示：",MB_SYSTEMMODAL);
		}
		string totalNum = "";
		string finishNum="";
		m_Edit2dCode.SetWindowText("");
		m_EditCode2.SetWindowText("");

		string deviceId = "";
		int i=0;
		if(Device_Task(atoi(sTask.c_str()),deviceId) && (0 != deviceId.length())){		
			while(1){
				string sDevice = (string)ListDevice.GetItemText(i,0);
				if ("" == sDevice){
					break;
				}
				if (sDevice == deviceId){
						gDevice = i;	
						ListDevice.SetItemState(i, LVIS_DROPHILITED, LVIS_DROPHILITED); 
					break;
				}
				i++;
			}
		}
		else
		{
	        gDevice = -1;
	    }
		if (deviceArr.find((const int)i) != deviceArr.end())
		{
			char tmpNum[64]={0};
			_itoa_s(deviceArr[i]->medPacketsNum,tmpNum,64,10);
			m_printTotal.SetWindowText(tmpNum); 
			memset(tmpNum,0,sizeof(tmpNum));
			_itoa_s(deviceArr[i]->printedNum,tmpNum,64,10);
			m_printFinishedNum.SetWindowText(tmpNum);    
		}
		else
		{
			if (GetTotalNumAndFinishNum(sTask,totalNum,finishNum)){
				m_printTotal.SetWindowText(totalNum.c_str());
				m_printFinishedNum.SetWindowText(finishNum.c_str());
			}
			else{
				m_printTotal.SetWindowText("");
				m_printFinishedNum.SetWindowText("");
			}
		}
		gTask = pNMItemActivate->iItem;	
		DWORD dwPos = GetMessagePos();
		CPoint point( LOWORD(dwPos), HIWORD(dwPos) );
		CMenu menu;
		VERIFY( menu.LoadMenu( IDR_MENU_TASK ) );     
		CMenu* popup = menu.GetSubMenu(0);
		ASSERT( popup != NULL );
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this );	
	}

	*pResult = 0;
}

void CTtoCtrlPlatformDlg::OnTaskFenPei()
{
	CString tmpTaskId = ListTask.GetItemText(gTask,0);
	string sTask = tmpTaskId.GetBuffer(0);
	tmpTaskId.ReleaseBuffer();
	if (0 == atoi(sTask.c_str())){
		return;
	}
	string simplePlace = "";
	string sCode1="";
	if (!GetSimplePlaceFromDB(sTask,simplePlace,sCode1) || "" == simplePlace){
		MessageBoxA("分配任务前请先将产地简称等信息配置完成!","温馨提示：",MB_SYSTEMMODAL);
		CWriteSimplePlaceDlg placeDlg(sTask);
		placeDlg.DoModal();

		string sPlace = "";
		GetSimplePlaceFromDB(sTask,sPlace,sCode1);
		ListTask.SetItemText(gTask, 8, sPlace.c_str());//产地简称
		if ("" == sPlace){
			return;
		}
	}
	CTaskAndDeviceSetup taskAndDeviceMatchDlg(sTask);
	taskAndDeviceMatchDlg.DoModal();
	ListDevice.DeleteAllItems();
	GetDeviceListFromDB();
} 

string CTtoCtrlPlatformDlg::SentUsbIdToServer(string& taskIP,int taskPort,string& taskId,char* usbId)
{
	string retString = "";

	int deviceTotal = 0;
	iter = db.ExeSql("select count(1) from table_device");
	if(iter.HasMore()){
		deviceTotal = (int)iter.GetData(0);
	}
	size_t bufferLen=0;
	unsigned char* buffer = (unsigned char*)malloc( 0x400);//解压后的空间
	if( NULL == buffer ){
		return "";
	}    
	sprintf((char*)buffer,"GETTASKS %s %s %d",taskId.c_str(),usbId,deviceTotal);//新接口
	retString = GetDownData((char *)buffer,taskIP.c_str(),taskPort,&buffer,bufferLen);	
	if ( retString != "OK" || 0 == bufferLen){
		free(buffer);
		buffer = NULL;
		MessageBox(retString.c_str(),"提示：",MB_SYSTEMMODAL);
		return "";
	}
	buffer[bufferLen]=0x00;
	char tmpAuth[128]={0};
	strncpy_s(tmpAuth,(char *)buffer,127);
	retString = tmpAuth;
	free(buffer);
	buffer = NULL;
	return retString;
}


void CTtoCtrlPlatformDlg::OnTaskDownload(){	
	string tmptaskstatus = (const char*)ListTask.GetItemText(gTask,1);
	if ("非溯源" == tmptaskstatus)
	{
		MessageBox("非溯源打码任务不需要下载！","提示：",MB_SYSTEMMODAL); 
		return;
	}
	string taskId = (const char*)ListTask.GetItemText(gTask,0);
	string onewayPass="";
	////////////////
	//onewayPass = SentUsbIdToServer(taskIP,taskPort,taskId,usbId);//通过USBID请求pass 
	//if ("" == onewayPass)
	//{
	//	return;
	//}

	//if (!KeyAuth(usbAuth,onewayPass.c_str())){
	//	MessageBox("加密锁认证失败！","提示：",MB_SYSTEMMODAL); 
	//	return ;
	//}
///////////////////////////
	ListTask.SetItemText(gTask,1,"正在下载");
	memset(sqlBuff,0,sizeof(sqlBuff));
	sprintf_s(sqlBuff,512,"select taskFlag from table_task where tabletstorid =%s",taskId.c_str());
	iter = db.ExeSql(sqlBuff);
	if(!iter.HasMore()){
		MessageBox("获取任务状态失败","温馨提示：",MB_SYSTEMMODAL); 
		ListTask.SetItemText(gTask,1,"没有任务");
		return;
	}
	int taskFlag = iter.GetData(0);
	if( 0 != taskFlag && 4 != taskFlag && 5 != taskFlag){
		MessageBox("已经下载！","温馨提示：",MB_SYSTEMMODAL);
		ListTask.SetItemText(gTask,1,"已经下载");
		return;
	}
	size_t bufferLen=0;
	unsigned char* unzip_buffer = (unsigned char*)malloc(0x400*0x400*5);//解压后的空间
	if( NULL == unzip_buffer ){
		MessageBox("下载二维码时，分配空间失败！","温馨提示：",MB_SYSTEMMODAL);
		ListTask.SetItemText(gTask,1,"未下载");
		return;
	}
	SetTaskListFlag(taskId.c_str(),1);
	ListTask.SetItemText(gTask,1,"正在下载");

	sprintf((char*)unzip_buffer,"GETCODES %s ss ssdss",taskId.c_str(),usbId,usbAuth);
	string retString = GetDownData((char *)unzip_buffer,taskIP.c_str(),taskPort,&unzip_buffer,bufferLen);
	if ( retString != "OK" ){
		ListTask.SetItemText(gTask, 1, retString.c_str());//设置数据
		if( retString == "已经下载"){
			DelOneFromTaskList(taskId.c_str());
		}
		else if( retString == "任务非法"){
			SetTaskListFlag(taskId.c_str(),5);
		}
		else{
			SetTaskListFlag(taskId.c_str(),4);	
		}
		free(unzip_buffer);
		unzip_buffer = NULL;
		return;
	}
	db.ExeSql("BEGIN");
	char* p = (char *)unzip_buffer;
	char* q = NULL;
	size_t j=0;
	char sqlSour[128]={0};
	sprintf_s(sqlSour,128,"insert into table_midcode (tabletstorid,midcode) values (%s,'%%s')",taskId.c_str());
	while(q = strstr(p,",")){
		*q = 0;
		memset(sqlBuff,0,sizeof(sqlBuff));
		sprintf_s(sqlBuff,512,sqlSour,p);
		p = q + 1;
		db.ExeSql(sqlBuff);
		if( ++j%500 == 0 ){
			db.ExeSql("COMMIT");
			db.ExeSql("BEGIN");
		}
	}
	db.ExeSql("COMMIT");
	db.ExeSql("BEGIN");
	size_t i=0;//小包数量
	memset(sqlSour,0,sizeof(sqlSour));
	sprintf_s(sqlSour,128,"insert into table_2dcode (tabletstorid,code2d) values (%s,'%%s')",taskId.c_str());
	while(q = strstr(p,"|")){
		*q = 0;
		memset(sqlBuff,0,sizeof(sqlBuff));
		sprintf_s(sqlBuff,512,sqlSour,p);
		p = q + 1;
		db.ExeSql(sqlBuff);
		if( ++i%2000 == 0 ){
			db.ExeSql("COMMIT");
			db.ExeSql("BEGIN");
		}
	}
	db.ExeSql("COMMIT");
	memset(sqlBuff,0,sizeof(sqlBuff));
	sprintf_s(sqlBuff,512,"select tabletboxnum,tabletitemnum from table_task where tabletstorid =%d",atoi(taskId.c_str()));
	iter = db.ExeSql(sqlBuff);
    int boxnum = 0;
	string itemnum="";
	if(iter.HasMore()){
		itemnum = (string)iter.GetData(1);
		if (atoi(itemnum.c_str()) > 0)
		{
			boxnum = (int)iter.GetData(0);
			memset(sqlBuff,0,sizeof(sqlBuff));
			sprintf_s(sqlBuff,512,"update table_task SET tabletboxnum = %d where tabletstorid = %d",i,atoi(taskId.c_str()));
			db.ExeSql(sqlBuff);
		}
	}
	free(unzip_buffer);
	unzip_buffer = NULL;
	SetTaskListFlag(taskId.c_str(),3);
	ListTask.SetItemText(gTask, 1, taskStats[3].c_str());//设置数据
}


bool CTtoCtrlPlatformDlg::Device_Task( const string& deviceid, int& taskid ){
	memset(sqlBuff,0,sizeof(sqlBuff));
	sprintf_s(sqlBuff,512,"select tabletstorid from table_device where devicetag='%s'",deviceid.c_str());
	iter = db.ExeSql(sqlBuff);
	if(iter.HasMore()){
		taskid = (int)iter.GetData(0);
		return true;
	}
	return false;
}

bool CTtoCtrlPlatformDlg::Device_Task(const int& taskid,string& deviceid){
	memset(sqlBuff,0,sizeof(sqlBuff));
	sprintf_s(sqlBuff,512,"select devicetag from table_device where tabletstorid=%d",taskid);
	iter = db.ExeSql(sqlBuff);
	if(iter.HasMore()){
		deviceid = (const char*)iter.GetData(0);
		return true;
	}
	return false;
}

bool CTtoCtrlPlatformDlg::Device_Task(const string& taskid,string& deviceid)//非溯源用
{
	memset(sqlBuff,0,sizeof(sqlBuff));
	sprintf_s(sqlBuff,512,"select devicetag from table_device where tabletstorid=%s",taskid.c_str());
	iter = db.ExeSql(sqlBuff);
	if(iter.HasMore()){
		deviceid = (const char*)iter.GetData(0);
		return true;
	}
	return false;
}

void CTtoCtrlPlatformDlg::OnDevicePrintStart(){
	if( -1 == gDevice )	return;
	if (deviceArr.find(gDevice) != deviceArr.end()){
		MessageBoxA("线程创建中，请稍候。。。","温馨提示：",0);
		return ;
	}
	ListDevice.SetItemText(gDevice,4,deviceStats[1].c_str());
	deviceBtnShowFlag = false;
	((CButton *)GetDlgItem(IDC_BUTTON_UPDATE_DEVICE))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_DEVICE_YINCANG)));

	DeviceArr* ttoArr = NULL;
	try{
		ttoArr = new DeviceArr;
		ttoArr->DeviceId = (const char*)ListDevice.GetItemText(gDevice,0);
		memset(sqlBuff,0,sizeof(sqlBuff));
		sprintf_s(sqlBuff,512,"select ip,port,devicetype,printFlag,CAST(tabletstorid as char) from table_device where devicetag = '%s'",ttoArr->DeviceId .c_str());
		iter = db.ExeSql(sqlBuff);
		if(!iter.HasMore())	throw exception("读取设备基本信息失败！");
		unsigned int printFlag = (int)iter.GetData(3);
		if( 0 == printFlag ){
			string taskId = (const char*)ListDevice.GetItemText(gDevice,2);
			CTtoParamConfig ttoConfig(gDevice,ttoArr->DeviceId,taskId);
			ttoConfig.DoModal();			 
			throw exception("");
		}
		if (printFlag&0x4000) ttoArr->InmedDef3 = MED_DEF3;
		if (printFlag&0x2000) ttoArr->InmedDef2 = MED_DEF2;
		if (printFlag&0x1000) ttoArr->InmedDef1 = MED_DEF1;
		if (printFlag&0x800) ttoArr->InCmpNameSim = CPY_NAME;
		if (printFlag&0x400) ttoArr->InmedStandard = MED_STANDARD;
		if (printFlag&0x200) ttoArr->InmedPiwenNum = MED_OFFDOC;
		if (printFlag&0x100) ttoArr->InmedDateTo = MED_DATE_TO;
		if (printFlag&0x80)	ttoArr->Inmed1Code = MED_1CODE;
		if (printFlag&0x40)	ttoArr->InmedName = MED_NAME;
		if (printFlag&0x20)	ttoArr->InmedWeight = MED_WEIGHT;
		if (printFlag&0x10)	ttoArr->InmedSpecial = MED_SPEC;
		if (printFlag&0x08)	ttoArr->InmedPlace =MED_ORIGIN;
		if (printFlag&0x04)	ttoArr->InmedBatch = MED_NUMBER;
		if (printFlag&0x02)	ttoArr->InmedDate = MED_DATE;
		if (printFlag&0x01)	ttoArr->InmedCode2d = MED_QRCODE;

		ttoArr->med1Code = (string)ListTask.GetItemText(gTask,10);

		ttoArr->TaskId = (const char*)iter.GetData(4);
		ttoArr->ip = (const char*)iter.GetData(0);
		ttoArr->port = (int)iter.GetData(1);
		ttoArr->deviceType = (int)iter.GetData(2);

		iter = db.ExeSql("select timeType,weightType from table_login");
		if(!iter.HasMore())	throw exception("读取时间类型、重量类型信息失败！");
		timeType = (int)iter.GetData(0);
		weightType = (int)iter.GetData(1);

		if ( "下载完成" != (string)ListTask.GetItemText(gTask,1) && "已经下载" != (string)ListTask.GetItemText(gTask,1) && "非溯源" != (string)ListTask.GetItemText(gTask,1)) throw exception("任务未下载完成！");
		if ( "0" == ttoArr->TaskId ) throw exception("任务未分配！");
		if ( 0 == ttoArr->deviceType ) throw exception("设备类型未设置！");
		if( ttoArr->ip.length() == 0 || 0 == ttoArr->port ) throw exception("读取设备IP,端口号失败！");
	    if (deviceArr.find(gDevice) != deviceArr.end()) throw exception("当前设备正在打印，请稍候。。。");
		
		ttoArr->oneStopFlag = false;
		deviceArr[gDevice] = ttoArr;
		
	}catch(const exception& e){
		if (strlen(e.what()) != 0)	MessageBox(e.what(),"温馨提示：",MB_SYSTEMMODAL);
		ListDevice.SetItemText(gDevice,4,deviceStats[0].c_str());
		delete ttoArr;
		ttoArr = NULL;
		if ( deviceArr.size() == 0 ){
			deviceBtnShowFlag=true;
			((CButton *)GetDlgItem(IDC_BUTTON_UPDATE_DEVICE))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_DEVICE)));
		}
		return;
	}  
	deviceArr[gDevice]->hStatusThread = (HANDLE)_beginthreadex(NULL, 0, PrintThread,  (LPVOID)this, 0, NULL); 
	if (NULL == ttoArr->hStatusThread)  {  
		ListDevice.SetItemText(gDevice,4,"设备开始打印失败！");
	}
}

void CTtoCtrlPlatformDlg::OnDevicePrintStop(){
	if(-1 == gDevice) return;
	string sDevice = (const char*)ListDevice.GetItemText(gDevice,0);
	if ("" == sDevice)
	{
		return;
	}
	if( deviceArr.find(gDevice) != deviceArr.end()){
		deviceArr[gDevice]->oneStopFlag = true;
		ListDevice.SetItemText(gDevice,4,"空闲");
	}else
		ListDevice.SetItemText(gDevice,4,"当前设备处于未打印状态！");
}

void CTtoCtrlPlatformDlg::OnNMRClickListDevice(NMHDR *pNMHDR, LRESULT *pResult)
{
	LMyPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LMyPNMITEMACTIVATE>(pNMHDR);
	if(pNMItemActivate->iItem != -1){

		ListDevice.SetItemState(-1, 0, -1);
		ListTask.SetItemState(-1, 0, -1); 
		ListDevice.SetItemState(pNMItemActivate->iItem, LVIS_DROPHILITED, LVIS_DROPHILITED); 	

		string sDevice = (string)ListDevice.GetItemText(pNMItemActivate->iItem,0);
		if ("" == sDevice)
		{
			return;
		}
		string sTask="";
		int taskId = 0;		
		if (Device_Task(sDevice,taskId) && (0 != taskId)){
			int i=0;
			while(1){
				sTask = (string)ListTask.GetItemText(i,0);
				if (sTask.length() == 0){
					break;
				}
				if (atoi(sTask.c_str()) == taskId){
					ListTask.SetItemState(i, LVIS_DROPHILITED, LVIS_DROPHILITED); 
					gTask = i;
					break;
				}
				i++;
			}
		}
		gDevice = pNMItemActivate->iItem;

		if (deviceArr.find((const int)gDevice) != deviceArr.end()){
			char tmpTotal[32]={0}; 
			sprintf_s(tmpTotal,32,"%d",deviceArr[gDevice]->medPacketsNum); 
			m_printTotal.SetWindowText(tmpTotal);
			memset(tmpTotal,0,sizeof(tmpTotal));
			sprintf_s(tmpTotal,32,"%d",deviceArr[gDevice]->printedNum); 
			m_printFinishedNum.SetWindowText(tmpTotal);
		}else {
			if ("" != sTask){
				string totalNum = "";
				string finishNum="";
				m_Edit2dCode.SetWindowText("");
				m_EditCode2.SetWindowText("");
				if (GetTotalNumAndFinishNum(sTask,totalNum,finishNum)){
					m_printTotal.SetWindowText(totalNum.c_str());
					m_printFinishedNum.SetWindowText(finishNum.c_str());
				}
				else{
					m_printTotal.SetWindowText("");
					m_printFinishedNum.SetWindowText("");
				}
			}else{
				m_printTotal.SetWindowText("");
				m_printFinishedNum.SetWindowText("");
			}
		}
		DWORD dwPos = GetMessagePos();
		CPoint point( LOWORD(dwPos), HIWORD(dwPos) );
		CMenu menu;
		VERIFY( menu.LoadMenu( IDR_MENU_DEVICE ) );     
		CMenu* popup = menu.GetSubMenu(0);
		ASSERT( popup != NULL );
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this );
	}
	*pResult = 0;
}

void CTtoCtrlPlatformDlg::OnTaskGuanBi(){
	if (CheckTaskIdInUseOrNot(ListTask.GetItemText(gTask,0))){
		MessageBox("该任务已分配，请先在该任务上右键选择\"取消设备\"，再\"关闭任务\"！","温馨提示：",MB_SYSTEMMODAL);
		return;
	}
	if (MessageBox("关闭任务之后，任务会从任务列表清除，是否确定关闭任务？","任务列表操作提示：",MB_YESNO) == IDNO)	return;
    string tmpTaskStatus = ListTask.GetItemText(gTask,1);
	if ("" != tmpTaskStatus)
	{
		int printedNum=0;
		string taskId = ListTask.GetItemText(gTask,0);
		memset(sqlBuff,0,sizeof(sqlBuff));
		sprintf_s(sqlBuff,512,"select tabletPrintedNum from table_task where tabletstorid =%s",taskId.c_str());
		iter = db.ExeSql(sqlBuff);
		if(iter.HasMore()) printedNum = (int)iter.GetData(0);
        if (tmpTaskStatus != taskStats[0])
        {
			int i=0;
			while (1){
				if (tmpTaskStatus == taskStats[i]){
					break;
				}
				i++;
			}
			GuanbiTaskAndInsertIntoHistroy(
				(const char*)ListTask.GetItemText(gTask,0),
				i,
				(const char*)ListTask.GetItemText(gTask,2),
				(const char*)ListTask.GetItemText(gTask,5),
				(const char*)ListTask.GetItemText(gTask,6),
				(const char*)ListTask.GetItemText(gTask,7),
				(const char*)ListTask.GetItemText(gTask,3),
				(const char*)ListTask.GetItemText(gTask,9),
				(const char*)ListTask.GetItemText(gTask,4),
				(const char*)ListTask.GetItemText(gTask,8),
				printedNum,
				(const char*)ListTask.GetItemText(gTask,10));	
			memset(sqlBuff,0,sizeof(sqlBuff));
			sprintf_s(sqlBuff,512,"delete from table_2dcode where tabletstorid =%s",taskId.c_str());
			db.ExeSql(sqlBuff);
        }

		DelOneFromTaskList((const char*)ListTask.GetItemText(gTask,0));
		ListTask.DeleteAllItems();
		GetTaskFromDB();
	}
}

void CTtoCtrlPlatformDlg::OnTaskQuXiao()
{
	string sTask = ListTask.GetItemText(gTask,0);
	if ("" == sTask)
	{
		MessageBox("请正确选择任务！","任务列表操作提示：",MB_YESNO);
		return;
	}
	string sDeviceId="";
	if (Device_Task(sTask,sDeviceId) && sDeviceId !="")
	{
		int i=0;
		while(1){
			string sTmpDeviceId="";
			sTmpDeviceId = (string)ListDevice.GetItemText(i,0);//界面取得
			if (sTmpDeviceId.length() == 0){
				break;//查找到最后
			}
			if (sTmpDeviceId == sDeviceId){
				//tmpDevice = i;
				string sRunStatus = ListDevice.GetItemText(i,4);
				if("正在打印" == sRunStatus)
				{
					MessageBox("该任务对应的设备处于打印状态，请先在该设备上停止打印后再取消任务！","任务列表操作提示：",MB_YESNO);
					return;
				}
				break;
			}
			i++;
		}
	}
    else
	{
		MessageBox("该任务没有对应的设备号！","任务列表操作提示：",MB_YESNO);
		return;
	}
	if (MessageBox("是否确定取消设备？","任务列表操作提示：",MB_YESNO) == IDNO){
		return;
	}

	if (CheckTaskIdInUseOrNot(sTask.c_str())){
		DisMatchTaskFromDevice(sTask);
		//刷新设备列表
		ListDevice.DeleteAllItems();
		GetDeviceListFromDB();
	}
}

BOOL CTtoCtrlPlatformDlg::GetSimplePlaceFromDB(string inTaskId,string &sPlace,string &sCode1)
{
	memset(sqlBuff,0,sizeof(sqlBuff));
	sprintf_s(sqlBuff,512,"select medPlaceSimple,tablet1code from table_task where tabletstorid =%d",atoi(inTaskId.c_str()));
	iter = db.ExeSql(sqlBuff);
	if(iter.HasMore()){
		sPlace = (string)iter.GetData(0);
		sCode1 = (string)iter.GetData(1);
		return TRUE;
	}
	return FALSE;
}

BOOL CTtoCtrlPlatformDlg::GetSimplePlaceFromDB(string inTaskId,string &sPlace,string &sCode1,string &sPec)
{
	memset(sqlBuff,0,sizeof(sqlBuff));
	sprintf_s(sqlBuff,512,"select medPlaceSimple,tablet1code,tabletspec from table_task where tabletstorid =%d",atoi(inTaskId.c_str()));
	iter = db.ExeSql(sqlBuff);
	if(iter.HasMore()){
		sPlace = (string)iter.GetData(0);
		sCode1 = (string)iter.GetData(1);
		sPec = (string)iter.GetData(2);
		return TRUE;
	}
	return FALSE;
}

void CTtoCtrlPlatformDlg::OnModelSet()
{
	string sDevice = ListDevice.GetItemText(gDevice,0);
	string sTask = ListDevice.GetItemText(gDevice,2);
    if ("" == sDevice || "" == sTask)
    {
		return;
    }
	CTtoParamConfig ttoConfig(gDevice,sDevice,sTask);
	ttoConfig.DoModal();
}

void CTtoCtrlPlatformDlg::OnNMClickListDevice(NMHDR *pNMHDR, LRESULT *pResult)
{
	LMyPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LMyPNMITEMACTIVATE>(pNMHDR);
	if(pNMItemActivate->iItem == -1){
		*pResult = 0;
		return;
	}
	gDevice = pNMItemActivate->iItem;
	ListDevice.SetItemState(-1, 0, -1);
	ListTask.SetItemState(-1, 0, -1); 
	ListDevice.SetItemState(pNMItemActivate->iItem, LVIS_DROPHILITED, LVIS_DROPHILITED); 	

	CString tmpDeviceId = ListDevice.GetItemText(pNMItemActivate->iItem,0);
	string sDeviceId = tmpDeviceId.GetBuffer(0);
	tmpDeviceId.ReleaseBuffer();
	if ("" == sDeviceId)
	{
		return;
	}
	int taskNum = -1;
	string sTask = "";//来自task界面   比对
	int taskId=0;//根据device从数据库取得  
	if (Device_Task(sDeviceId,taskId) && (0 != taskId)){
		int i=0;
		while(1){
			CString tmpTaskId = ListTask.GetItemText(i,0);
			sTask = tmpTaskId.GetBuffer(0);
			tmpTaskId.ReleaseBuffer();
			if (sTask.length() == 0){
				break;//查找到最后
			}
			if (atoi(sTask.c_str()) == taskId){
				taskNum = i;
				break;
			}
			i++;
		}
		if (taskNum != -1)
			ListTask.SetItemState(taskNum, LVIS_DROPHILITED, LVIS_DROPHILITED); 
	}
	//更改显示实时信息的gtask gdevice
	gTask = taskNum;
	if (deviceArr.find((const int)gDevice) != deviceArr.end()){
		char tmpTotal[32]={0}; 
		//总数
		sprintf_s(tmpTotal,32,"%d",deviceArr[pNMItemActivate->iItem]->medPacketsNum); 
		m_printTotal.SetWindowText(tmpTotal);
		//已打数目
		memset(tmpTotal,0,sizeof(tmpTotal));
		sprintf_s(tmpTotal,32,"%d",deviceArr[pNMItemActivate->iItem]->printedNum); 
		m_printFinishedNum.SetWindowText(tmpTotal);
	}
	else {
		//取总的和已打的数目
		string totalNum = "";
		string finishNum="";
		m_Edit2dCode.SetWindowText("");
		m_EditCode2.SetWindowText("");
		if (GetTotalNumAndFinishNum(sTask,totalNum,finishNum)){
			m_printTotal.SetWindowText(totalNum.c_str());
			m_printFinishedNum.SetWindowText(finishNum.c_str());
		}
		else{
			m_printTotal.SetWindowText("");
			m_printFinishedNum.SetWindowText("");
		}
	}
	*pResult = 0;
}

void CTtoCtrlPlatformDlg::OnNMClickListTask(NMHDR *pNMHDR, LRESULT *pResult)
{	
	LMyPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LMyPNMITEMACTIVATE>(pNMHDR);
	if(pNMItemActivate->iItem == -1){
		*pResult = 0;
		return;
	}
	gTask = pNMItemActivate->iItem;
    ListTask.SetItemState(-1, 0, -1); 
    ListDevice.SetItemState(-1, 0, -1);
	ListTask.SetItemState(pNMItemActivate->iItem,LVIS_DROPHILITED, LVIS_DROPHILITED); 

	string sTask = (string)ListTask.GetItemText(pNMItemActivate->iItem,0);
	string totalNum = "";
	string finishNum="";
	m_Edit2dCode.SetWindowText("");
	m_EditCode2.SetWindowText("");

	string deviceId = "";
	if (!Device_Task(atoi(sTask.c_str()),deviceId) || (0 == deviceId.length())){
		*pResult = 0;

		if (GetTotalNumAndFinishNum(sTask,totalNum,finishNum)){
			m_printTotal.SetWindowText(totalNum.c_str());
			m_printFinishedNum.SetWindowText(finishNum.c_str());
		}
		else{
			m_printTotal.SetWindowText("");
			m_printFinishedNum.SetWindowText("");
		}
		gDevice = -1;
		return;
	}
	int deviceNum = -1;
	int i=0;
	while(1){
		CString tmpDeviceId = ListDevice.GetItemText(i,0);
		string sDevice = tmpDeviceId.GetBuffer(0);
		tmpDeviceId.ReleaseBuffer();
		if ("" == sDevice){
			break;//查找到最后
		}
		if (sDevice == deviceId){
			deviceNum = i;
			break;
		}
		i++;
	}
	ListDevice.SetItemState(deviceNum, LVIS_DROPHILITED, LVIS_DROPHILITED); 
	
	if (deviceArr.find((const int)deviceNum) != deviceArr.end())
	{
		char tmpNum[64]={0};
		_itoa_s(deviceArr[deviceNum]->medPacketsNum,tmpNum,64,10);
		m_printTotal.SetWindowText(tmpNum); 
		memset(tmpNum,0,sizeof(tmpNum));
        _itoa_s(deviceArr[deviceNum]->printedNum,tmpNum,64,10);
		m_printFinishedNum.SetWindowText(tmpNum);    
	}
	else
	{
		if (GetTotalNumAndFinishNum(sTask,totalNum,finishNum)){
			m_printTotal.SetWindowText(totalNum.c_str());
			m_printFinishedNum.SetWindowText(finishNum.c_str());
		}
		else{
			m_printTotal.SetWindowText("");
			m_printFinishedNum.SetWindowText("");
		}
	}
	gDevice = deviceNum;
	*pResult = 0;
}

HBRUSH CTtoCtrlPlatformDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	if (GetDlgItem(IDC_STATIC_CODE_2D) == pWnd
		||GetDlgItem(IDC_EDIT_2D_CODE) == pWnd
		||GetDlgItem(IDC_EDIT_CODE2) == pWnd
		||GetDlgItem(IDC_STATIC_TOTAL_PAKETS) == pWnd
		||GetDlgItem(IDC_EDIT_PACKET_TOTAL) == pWnd
		||GetDlgItem(IDC_STATIC_PRINTED_PACKETS) == pWnd
		||GetDlgItem(IDC_EDIT_PACKET_PRINTED) == pWnd)
	{
		pDC->SetTextColor(RGB(255, 255, 255));
	}

	if(nCtlColor == MY_CTLCOLOR_DLG || nCtlColor == MY_CTLCOLOR_STATIC){
		COLORREF bkColor = RGB(MY_BACK_START,MY_BACK_MID,MY_COLOR_END);      
		CRect rect;         
		pWnd->GetClientRect(&rect); 
		CBrush br;          
		br.CreateSolidBrush(bkColor);       		     
		pDC->SetBkColor(bkColor);
		return m_brush;  
	}
	return hbr;
}

DWORD CTtoCtrlPlatformDlg::UTF8ToUTF16(UTF8* pUTF8Start, UTF8* pUTF8End, UTF16* pUTF16Start, UTF16* pUTF16End)  
{  
	UTF16* pTempUTF16 = pUTF16Start;  
	UTF8* pTempUTF8 = pUTF8Start; 
	DWORD len=0;

	while (pTempUTF8 < pUTF8End && pTempUTF16+1 < pUTF16End)  
	{  
		if (*pTempUTF8 >= 0xE0 && *pTempUTF8 <= 0xEF)//是3个字节的格式  
		{  
			*pTempUTF16 |= ((*pTempUTF8++ & 0xEF) << 12);  
			*pTempUTF16 |= ((*pTempUTF8++ & 0x3F) << 6);  
			*pTempUTF16 |= (*pTempUTF8++ & 0x3F);
		}  
		else if (*pTempUTF8 >= 0xC0 && *pTempUTF8 <= 0xDF)//是2个字节的格式  
		{  
			*pTempUTF16 |= ((*pTempUTF8++ & 0x1F) << 6);  
			*pTempUTF16 |= (*pTempUTF8++ & 0x3F);  
		}  
		else if(*pTempUTF8 >= 0 && *pTempUTF8 <= 0x7F)//是1个字节的格式  
		{  
			*pTempUTF16 = *pTempUTF8++; 
		}  
		else {  
			break;  
		}  
		pTempUTF16++;  
		len+=2;
	}  
	*pTempUTF16 = 0; 
	return len;
}  

int CTtoCtrlPlatformDlg::UTF16ToUTF8(UTF16* pUTF16Start, UTF16* pUTF16End, UTF8* pUTF8Start, UTF8* pUTF8End)
{
	UTF16* pTempUTF16 = pUTF16Start;
	UTF8* pTempUTF8 = pUTF8Start;

	while (pTempUTF16 < pUTF16End)
	{
		if (*pTempUTF16 <= UTF8_ONE_END
			&& pTempUTF8 + 1 < pUTF8End)
		{
			//0000 - 007F  0xxxxxxx
			*pTempUTF8++ = (UTF8)*pTempUTF16;
		}
		else if(*pTempUTF16 >= UTF8_TWO_START && *pTempUTF16 <= UTF8_TWO_END
			&& pTempUTF8 + 2 < pUTF8End) 
		{
			//0080 - 07FF 110xxxxx 10xxxxxx
			*pTempUTF8++ = (*pTempUTF16 >> 6) | 0xC0;
			*pTempUTF8++ = (*pTempUTF16 & 0x3F) | 0x80;
		}
		else if(*pTempUTF16 >= UTF8_THREE_START && *pTempUTF16 <= UTF8_THREE_END
			&& pTempUTF8 + 3 < pUTF8End)
		{
			//0800 - FFFF 1110xxxx 10xxxxxx 10xxxxxx
			*pTempUTF8++ = (*pTempUTF16 >> 12) | 0xE0;
			*pTempUTF8++ = ((*pTempUTF16 >> 6) & 0x3F) | 0x80;
			*pTempUTF8++ = (*pTempUTF16 & 0x3F) | 0x80;
		}
		else
		{
			break;
		}
		pTempUTF16++;
	}
	*pTempUTF8 = 0;
    return pTempUTF8 - pUTF8Start;
}

BOOL CTtoCtrlPlatformDlg::Gb2312ToUtf8(const char *pcGb2312,char* pcUtf8,int &utf8Len)
{
	if (NULL == pcGb2312 || NULL == pcUtf8)
	{
		return FALSE;
	}
	int nUnicodeLen = MultiByteToWideChar(CP_ACP, 0, pcGb2312, -1, NULL, 0);

	wchar_t pcUnicode[1024]={0}; 
	MultiByteToWideChar(CP_ACP, 0, pcGb2312, -1, pcUnicode, nUnicodeLen);

	int nUtf8Len = WideCharToMultiByte(CP_UTF8, 0, pcUnicode, -1, NULL, 0, NULL, NULL);
	if (nUtf8Len >= utf8Len)
	{
		return FALSE;
	}
	WideCharToMultiByte(CP_UTF8, 0, pcUnicode, -1, pcUtf8, nUtf8Len, NULL, NULL);
	utf8Len = nUtf8Len;
	return TRUE;
}

BOOL CTtoCtrlPlatformDlg::UTF8ToGB2312(const void * pIn,int inLen,char *pOut,int *outLen )
{
	ASSERT( ( (NULL==pIn) ||( NULL==pOut) ) );
	int len=MultiByteToWideChar(CP_UTF8, 0, (LPCTSTR)pIn, -1, NULL,0);
	wchar_t *pWchar = new wchar_t[len+1];
	if (NULL==pWchar)
		return FALSE;

	memset(pWchar,0,(len+1)* sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, (LPCTSTR)pIn,inLen, pWchar, len);
	CString str=CString(pWchar);
	ASSERT(str.GetLength()>(*outLen));//not enough output buff
	*outLen = str.GetLength();
	memcpy(pOut,str.GetBuffer(0),*outLen);

	delete[]pWchar;
	return TRUE;
}

bool CTtoCtrlPlatformDlg::MarkemGetPrintInfo(const char* GetStr,const int len,string& conferStr)
{
	UTF16 utf16[0x400] = {0};
	memcpy(utf16,GetStr+2,len-4);
	UTF8 utf8[1024]={0};
	int utf8Len = UTF16ToUTF8(&utf16[0],&utf16[len-4-1],&utf8[0],&utf8[1023]);
    if (utf8Len > 0)
    {
		conferStr = (char *)utf8+4;
		return true;
    }
    return false;
}

bool CTtoCtrlPlatformDlg::MarkemSendPrintInfo( const Socket& so,const string& sendStr,int &recvStrLen,char* recvStr)
{
	UTF8 utf8[1024]={0};
	int utf8Len = sizeof(utf8);
	if(Gb2312ToUtf8(sendStr.c_str(),(char *)utf8,utf8Len)){
		UTF16 utf16[0x400] = {0};  
		DWORD inlen = 0;
		inlen = UTF8ToUTF16(utf8, utf8 + strlen((char *)utf8), utf16, utf16+1024);
		char Command[0x400]={0};
		Command[0]=0x02;
		Command[1]=0x00;
		memcpy(&Command[2],utf16,inlen);
		Command[inlen+2]=0x03;
		Command[inlen+3]=0x00;
		so.Send(Command,inlen+4);
		return so.Recv(recvStr,recvStrLen);
	}
	return false;
}

bool CTtoCtrlPlatformDlg::MarkemSendPrintStatus( const Socket& so,const string& sendStr){
	UTF8 utf8[1024]={0};
	int utf8Len = sizeof(utf8);
	if(Gb2312ToUtf8(sendStr.c_str(),(char *)utf8,utf8Len)){
		UTF16 utf16[0x400] = {0};  
		DWORD inlen = 0;
		inlen = UTF8ToUTF16(utf8, utf8 + strlen((char *)utf8), utf16, utf16+1024);
		char Command[0x400]={0};
		Command[0]=0x02;
		Command[1]=0x00;
		memcpy(&Command[2],utf16,inlen);
		Command[inlen+2]=0x03;
		Command[inlen+3]=0x00;
		return so.Send(Command,inlen+4);
	}
	return false;
}

bool CTtoCtrlPlatformDlg::WDJSendPrintStatus( const Socket& so,const string& sendStr){
	UTF8 utf8[1024]={0};
	int utf8Len = sizeof(utf8);
	if(Gb2312ToUtf8(sendStr.c_str(),(char *)utf8,utf8Len)){
		UTF16 utf16[0x400] = {0};  
		DWORD inlen = 0;
		inlen = UTF8ToUTF16(utf8, utf8 + strlen((char *)utf8), utf16, utf16+1024);
		char Command[0x400]={0};
		memcpy(Command,utf16,inlen);
		return so.Send(Command,inlen);
	}
	return false;
}

BOOL CTtoCtrlPlatformDlg::WDJPrint(const char *code2d,int index,bool firstFlag,int& retFlag){
	string allCode;
	char buff[0x200];
	if (firstFlag){ 
		allCode="JDI|1|";
		if (deviceArr[index]->InmedCode2d != "")
		{
			allCode+= deviceArr[index]->InmedCode2d;
			allCode+="=";
			allCode+=code2d;
			allCode+="|";
		}
		if (deviceArr[index]->Inmed1Code != ""){
			allCode+=deviceArr[index]->Inmed1Code;
			allCode+="=";
			allCode+=deviceArr[index]->med1Code;
			allCode+="|";
		}
		if (deviceArr[index]->InmedName != ""){
			allCode+=deviceArr[index]->InmedName;
			allCode+="=";
			allCode+=deviceArr[index]->medName;
			allCode+="|";
		}
		if (deviceArr[index]->InmedWeight != ""){
			allCode+=deviceArr[index]->InmedWeight;
			allCode+="=";
			allCode+=deviceArr[index]->medWeight;
			allCode+="|";
		}
		if (deviceArr[index]->InmedSpecial != ""){
			allCode+=deviceArr[index]->InmedSpecial;
			allCode+="=";
			allCode+=deviceArr[index]->medSpecial;
			allCode+="|";
		}
		if (deviceArr[index]->InmedPlace != ""){
			allCode+=deviceArr[index]->InmedPlace;
			allCode+="=";
			allCode+=deviceArr[index]->medPlace;
			allCode+="|";
		}
		if (deviceArr[index]->InmedBatch != ""){
			allCode+=deviceArr[index]->InmedBatch;
			allCode+="=";
			allCode+=deviceArr[index]->medBatch;
			allCode+="|";
		}
		if (deviceArr[index]->InmedDate != ""){
			allCode+=deviceArr[index]->InmedDate;
			allCode+="=";
			allCode+=deviceArr[index]->medPrintDate;
			allCode+="|";
		}

		if (deviceArr[index]->InmedDateTo != ""){
			allCode+=deviceArr[index]->InmedDateTo;
			allCode+="=";
			allCode+=deviceArr[index]->medDateTo;
			allCode+="|";
		}
		if (deviceArr[index]->InmedPiwenNum != ""){
			allCode+=deviceArr[index]->InmedPiwenNum;
			allCode+="=";
			allCode+=deviceArr[index]->medPiwenNum;
			allCode+="|";
		}
		if (deviceArr[index]->InmedStandard != ""){
			allCode+=deviceArr[index]->InmedStandard;
			allCode+="=";
			allCode+=deviceArr[index]->medStandard;
			allCode+="|";
		}
		if (deviceArr[index]->InCmpNameSim != ""){
			allCode+=deviceArr[index]->InCmpNameSim;
			allCode+="=";
			allCode+=allUseNameSim;
			allCode+="|";
		}
		if (deviceArr[index]->InmedDef1 != ""){
			allCode+=deviceArr[index]->InmedDef1;
			allCode+="=";
			allCode+=deviceArr[index]->medDef1;
			allCode+="|";
		}
		if (deviceArr[index]->InmedDef2 != ""){
			allCode+=deviceArr[index]->InmedDef2;
			allCode+="=";
			allCode+=deviceArr[index]->medDef2;
			allCode+="|";
		}
		if (deviceArr[index]->InmedDef3 != ""){
			allCode+=deviceArr[index]->InmedDef3;
			allCode+="=";
			allCode+=deviceArr[index]->medDef3;
			allCode+="|";
		}

		allCode+="\r";
		if(!WDJSendPrintStatus(deviceArr[index]->so,"CQI\r")) return false;
		while (!deviceArr[index]->oneStopFlag && !workFlag){
			if ( deviceArr[index]->so.CanRecv()){
				if(!deviceArr[index]->so.Recv(buff,0x100)) return false;
				break;
			}
		}
		if(!WDJSendPrintStatus(deviceArr[index]->so,"SST|3|\r")) return false;		
		while (!deviceArr[index]->oneStopFlag && !workFlag){
			if ( deviceArr[index]->so.CanRecv()){
				if(!deviceArr[index]->so.Recv(buff,0x100)) return false;
				break;
			}
		}
	}else{
		allCode ="JDI|1|";
		if (deviceArr[index]->InmedCode2d != "")
		{
			allCode+=deviceArr[index]->InmedCode2d;
			allCode+="=";
			allCode+=code2d;
			allCode+="|\r";
		}
		else
		{
			allCode+=deviceArr[index]->InmedName;
			allCode+="=";
			allCode+=deviceArr[index]->medName;
			allCode+="|\r";
		}
	}

	if(!WDJSendPrintStatus(deviceArr[index]->so,allCode)) return false;
	int countReSend = 0;
	DWORD dret;
	while (!deviceArr[index]->oneStopFlag && !workFlag){
		int count = 0;
		dret = 0;
		while (!deviceArr[index]->oneStopFlag && !workFlag){
			if ( deviceArr[index]->so.CanRecv() )
			{
				if(!deviceArr[index]->so.Recv(buff,0x100)) return false;
				int i=0;
				while (i < 20)
				{
					if (buff[i] == 'P' && buff[i+2] == 'R' && buff[i+4] == 'S')
					{
						dret = 1;
						break;
					}
					i+=2;
				}
				if (1 == dret)
				{
					break;
				}
				else if (memcmp(buff,statusWrongResWDJ,6) == 0
					|| memcmp(buff,statusWrongRes2WDJ,6) == 0){
						WDJSendPrintStatus(deviceArr[index]->so,"GFT\r");
						dret = 2;
						break;
				}
				else if (memcmp(buff,PrintFullResWDJ,6) == 0)
				{
					dret = 3;
					break;
				}
				else if (memcmp(buff,ClearErrOkWDJ,6) == 0)
				{
					dret = 4;
					WDJSendPrintStatus(deviceArr[index]->so,allCode);
					Sleep(100);
					break;
				}
			}
			count++;
			if (count > 200)
			{
				WDJSendPrintStatus(deviceArr[index]->so,"SST|3|\r");
				while (!deviceArr[index]->oneStopFlag && !workFlag){
					if ( deviceArr[index]->so.CanRecv()){
						if(!deviceArr[index]->so.Recv(buff,0x100)) return false;
						count = 0;
						break;
					}
				}
			}
		}
		if (1 == dret){
			break;
		}
		else if(2 == dret)
		{
			WDJSendPrintStatus(deviceArr[index]->so,allCode);
			WDJSendPrintStatus(deviceArr[index]->so,"CAF\r");
		}
	}
	return true;
}

BOOL CTtoCtrlPlatformDlg::MarkemPrint(const char *code2d,int index,bool firstFlag){
	string allCode;
	char buff[0x200];
	int count = 0;
	if (firstFlag){ 
		allCode="~JU1||1|";
		if (deviceArr[index]->InmedCode2d != "")
		{
			allCode+= deviceArr[index]->InmedCode2d;
			allCode+="|";
			allCode+=code2d;
			allCode+="|";
		}	
		if (deviceArr[index]->InmedName != ""){
			allCode+=deviceArr[index]->InmedName;
			allCode+="|";
			allCode+=deviceArr[index]->medName;
			allCode+="|";
		}
		if (deviceArr[index]->InmedWeight != ""){
			allCode+=deviceArr[index]->InmedWeight;
			allCode+="|";
			allCode+=deviceArr[index]->medWeight;
			allCode+="|";
		}
		if (deviceArr[index]->InmedSpecial != ""){
			allCode+=deviceArr[index]->InmedSpecial;
			allCode+="|";
			allCode+=deviceArr[index]->medSpecial;
			allCode+="|";
		}
		if (deviceArr[index]->InmedPlace != ""){
			allCode+=deviceArr[index]->InmedPlace;
			allCode+="|";
			allCode+=deviceArr[index]->medPlace;
			allCode+="|";
		}
		if (deviceArr[index]->InmedBatch != ""){
			allCode+=deviceArr[index]->InmedBatch;
			allCode+="|";
			allCode+=deviceArr[index]->medBatch;
			allCode+="|";
		}
		if (deviceArr[index]->InmedDate != ""){
			allCode+=deviceArr[index]->InmedDate;
			allCode+="|";
			allCode+=deviceArr[index]->medPrintDate;
			allCode+="|";
		}
		if (deviceArr[index]->Inmed1Code != ""){
			allCode+=deviceArr[index]->Inmed1Code;
			allCode+="|";
			allCode+=deviceArr[index]->med1Code;
			allCode+="|";
		}
		if (deviceArr[index]->InmedDateTo != ""){
			allCode+=deviceArr[index]->InmedDateTo;
			allCode+="=";
			allCode+=deviceArr[index]->medDateTo;
			allCode+="|";
		}
		if (deviceArr[index]->InmedPiwenNum != ""){
			allCode+=deviceArr[index]->InmedPiwenNum;
			allCode+="=";
			allCode+=deviceArr[index]->medPiwenNum;
			allCode+="|";
		}
		if (deviceArr[index]->InmedStandard != ""){
			allCode+=deviceArr[index]->InmedStandard;
			allCode+="=";
			allCode+=deviceArr[index]->medStandard;
			allCode+="|";
		}
		if (deviceArr[index]->InCmpNameSim != ""){
			allCode+=deviceArr[index]->InCmpNameSim;
			allCode+="=";
			allCode+=allUseNameSim;
			allCode+="|";
		}
		if (deviceArr[index]->InmedDef1 != ""){
			allCode+=deviceArr[index]->InmedDef1;
			allCode+="|";
			allCode+=deviceArr[index]->medDef1;
			allCode+="|";
		}
		if (deviceArr[index]->InmedDef2 != ""){
			allCode+=deviceArr[index]->InmedDef2;
			allCode+="|";
			allCode+=deviceArr[index]->medDef2;
			allCode+="|";
		}
		if (deviceArr[index]->InmedDef3 != ""){
			allCode+=deviceArr[index]->InmedDef3;
			allCode+="|";
			allCode+=deviceArr[index]->medDef3;
			allCode+="|";
		}
		if(!MarkemSendPrintStatus(deviceArr[index]->so,"~ST|06|")) return false;
		while (!deviceArr[index]->oneStopFlag && !workFlag){
			if ( deviceArr[index]->so.CanRecv()){
				deviceArr[index]->so.Recv(buff,0x100);
				break;
			}
		}
	}else{
		allCode ="~JU1||1|";
		if (deviceArr[index]->InmedCode2d != "")
		{
			allCode+=deviceArr[index]->InmedCode2d;
			allCode+="=";
			allCode+=code2d;//RIGHT
			allCode+="|\r";
		}
		else
		{
			allCode+=deviceArr[index]->InmedName;
			allCode+="=";
			allCode+=deviceArr[index]->medName;//RIGHT
			allCode+="|\r";
		}
	}
	if(!MarkemSendPrintStatus(deviceArr[index]->so,allCode)) return false;
	while (!deviceArr[index]->oneStopFlag && !workFlag){
		if (deviceArr[index]->so.CanRecv()){
			if(!deviceArr[index]->so.Recv(buff,0x100)) return false;
			break;
		}
	}
	if(!deviceArr[index]->so.Send(CheckFinishMarkem,14)) return false;
	while (!deviceArr[index]->oneStopFlag && !workFlag){
		if (deviceArr[index]->so.CanRecv()){
			if(!deviceArr[index]->so.Recv(buff,0x100)) return false;
			break;
		}
	}
	if (memcmp(buff,PrintEndResMarkem,18) != 0) return false;
	return true;
}

void CTtoCtrlPlatformDlg::OnBnClickedButtonCancel(){  
	if(MessageBox("是否确定退出？","溯源打码控制平台",MB_YESNO) == IDNO) return;
	workFlag = true;	
	for (map<int,DeviceArr*>::iterator iter = deviceArr.begin();iter != deviceArr.end();++iter){
		if( WAIT_OBJECT_0 == WaitForSingleObject(iter->second->hStatusThread, 60*1000)) {
		iter->second->so.CShutdownSocket();
		iter->second->so.CCloseSocket();
		CloseHandle(iter->second->hStatusThread);
		delete iter->second;
		iter->second = NULL; 	    
		}
	}
	deviceArr.erase(deviceArr.begin(), deviceArr.end());
	if( WAIT_OBJECT_0 == WaitForSingleObject(exeSqlH, 60*1000)) TRACE("exesql thread quit\n");
	CloseHandle(exeSqlH); 
	if( WAIT_OBJECT_0 == WaitForSingleObject(UiHandle, 60*1000)) TRACE("uihandle thread quit\n");
	CloseHandle(UiHandle);
	
	if (NULL != hInstMaster)
	{
		KeyClose();
		FreeLibrary(hInstMaster);
		hInstMaster = NULL;
	}
	DeleteTray();
    OnCancel();
}

Mutex CTtoCtrlPlatformDlg::m_Mutex;
BOOL CTtoCtrlPlatformDlg::GetCodeAndPrint(int index,CTtoCtrlPlatformDlg* tto){
	int printFlag=0;
	int retFlag;
	char sqlBuff[512]={0};
	while (!deviceArr[index]->oneStopFlag && !workFlag && deviceArr[index]->printedNum < deviceArr[index]->medPacketsNum)
	{
		memset(sqlBuff,0,sizeof(sqlBuff));
		sprintf_s(sqlBuff,512,"select code2d,id from table_2dcode where tabletstorid=%s LIMIT 300",deviceArr[index]->TaskId.c_str());// limit 300
		deviceArr[index]->iter = deviceArr[index]->db.ExeSql(sqlBuff);
		if(!workFlag) tto->ListDevice.SetItemText(index,4,tto->deviceStats[1].c_str());
		retFlag=0;		
		while (deviceArr[index]->iter.HasMore())
		{
			if (deviceArr[index]->medPacketsNum == deviceArr[index]->printedNum)
			{
				break;
			}
			if (deviceArr[index]->oneStopFlag||workFlag){
				memset(sqlBuff,0,sizeof(sqlBuff));
				sprintf_s(sqlBuff,512,"update table_task Set tabletPrintedNum = %d where tabletstorid =%s",deviceArr[index]->printedNum,deviceArr[index]->TaskId.c_str());
				EXESQL.put(sqlBuff);
				printFlag = -1; 
				break;
			}
			strcpy(sqlBuff,(const char*)deviceArr[index]->iter.GetData(0));		
			if( 1 == deviceArr[index]->deviceType ){
				if(!MarkemPrint(sqlBuff,index,printFlag++==0)){
					if (!workFlag) tto->ListDevice.SetItemText(index, 4, "马肯依玛士设备打印异常！");
					printFlag = -2;
					memset(sqlBuff,0,sizeof(sqlBuff));
					sprintf_s(sqlBuff,512,"update table_task Set tabletPrintedNum = %d where tabletstorid =%s",deviceArr[index]->printedNum,deviceArr[index]->TaskId.c_str());
					EXESQL.put(sqlBuff);
					break;
				}
			}
			else if ( 2 == deviceArr[index]->deviceType ){
				if(!WDJPrint(sqlBuff,index,printFlag++==0,retFlag)){
					if (!workFlag) {
						if(1 == retFlag) tto->ListDevice.SetItemText(index, 4, "伟迪捷设备打印异常,请检查模板参数配置！");
						else if(2 == retFlag) tto->ListDevice.SetItemText(index, 4, "伟迪捷设备打印超时！");
					}
					printFlag = -2;
					memset(sqlBuff,0,sizeof(sqlBuff));
					sprintf_s(sqlBuff,512,"update table_task Set tabletPrintedNum = %d where tabletstorid =%s",deviceArr[index]->printedNum,deviceArr[index]->TaskId.c_str());
					EXESQL.put(sqlBuff);
					break;
				}
			}
			++deviceArr[index]->printedNum;
			if (index == tto->gDevice && !workFlag ){
				string uiShow="";
				char strcode1[32] = {0};
				char strcode2[32] = {0};
                strcpy(strcode2,&sqlBuff[12]);
				sqlBuff[12]=0;
				strcpy(strcode1,sqlBuff);

				char strIndex[16]={0};
				sprintf(strIndex,"%d",index);
                uiShow = strIndex;
				uiShow += "\n";
				uiShow += strcode1;
				uiShow += "\n";
				uiShow += strcode2;
				uiShow += "\n";
				memset(strIndex,0,sizeof(strIndex));
				sprintf(strIndex,"%d",deviceArr[index]->medPacketsNum);
				uiShow += strIndex;
				uiShow += "\n";
				memset(strIndex,0,sizeof(strIndex));
				sprintf(strIndex,"%d",deviceArr[index]->printedNum);
				uiShow += strIndex;

				UIQUEUE.put(uiShow);
			}
			memset(sqlBuff,0,sizeof(sqlBuff));
			sprintf_s(sqlBuff,512,"delete from table_2dcode where id=%d",(int)deviceArr[index]->iter.GetData(1));
			EXESQL.put(sqlBuff);
		}
		if (printFlag <= -1)
		{
			break;
		}
	}
	//--------------------------------
	if(0 != printFlag){
		if ( 1 == deviceArr[index]->deviceType ){
			MarkemSendPrintStatus(deviceArr[index]->so,"~ST|04|");
		}
		else if ( 2 == deviceArr[index]->deviceType ){
			WDJSendPrintStatus(deviceArr[index]->so,"SST|4|\r");
		}
	}
	//---------------------------------		
	Guard<Mutex> mutex(m_Mutex);
	if(!workFlag) {
		if(0 == retFlag) tto->ListDevice.SetItemText(index,4,tto->deviceStats[0].c_str());
	}
	memset(sqlBuff,0,sizeof(sqlBuff));
	sprintf_s(sqlBuff,512,"update table_device SET finishFlag=0 where devicetag = '%s'",deviceArr[index]->DeviceId.c_str());
	deviceArr[index]->db.ExeSql(sqlBuff);
	memset(sqlBuff,0,sizeof(sqlBuff));
	sprintf_s(sqlBuff,512,"update table_task Set tabletPrintedNum = %d where tabletstorid =%s",deviceArr[index]->printedNum,deviceArr[index]->TaskId.c_str());
	EXESQL.put(sqlBuff);

	if ( printFlag > 0 ){
		memset(sqlBuff,0,sizeof(sqlBuff));
		sprintf_s(sqlBuff,512,"insert into table_task_history(tabletstorid,taskFlag,tabletname,tabletnumweight,tabletspec,medaddress,tabletnumber,createtime,tabletboxnum,tabletitemnum,medPlaceSimple,tabletPrintedNum,tablet1code) values(%d,3,'%s','%s','%s','%s','%s','%s',%d,'%s','%s',%d,'%s')",
			atoi(deviceArr[index]->TaskId.c_str()),
			deviceArr[index]->medName.c_str(),
			deviceArr[index]->medWeight.c_str(),
			deviceArr[index]->medSpecial.c_str(),
			deviceArr[index]->medPlaceAll.c_str(),
			deviceArr[index]->medBatch.c_str(),
			deviceArr[index]->medDate.c_str(),
			deviceArr[index]->medPacketsNum,
            deviceArr[index]->meditemnum.c_str(),
			deviceArr[index]->medPlace.c_str(),
			deviceArr[index]->printedNum,
			deviceArr[index]->med1Code.c_str());
		deviceArr[index]->db.ExeSql(sqlBuff);
		memset(sqlBuff,0,sizeof(sqlBuff));
		sprintf_s(sqlBuff,512,"delete from table_task where tabletstorid  = %s",deviceArr[index]->TaskId.c_str());	
		deviceArr[index]->db.ExeSql(sqlBuff);
		memset(sqlBuff,0,sizeof(sqlBuff));
		sprintf_s(sqlBuff,512,"update table_device SET tabletstorid=0 where devicetag = '%s'",deviceArr[index]->DeviceId.c_str());
		EXESQL.put(sqlBuff);

		if (!workFlag){
			tto->ListDevice.SetItemText(index, 3, "未分配");
			tto->ListDevice.SetItemText(index, 2, "");
			tto->ListTask.DeleteAllItems();
			tto->GetTaskFromDB();
		}
	}
	
	return true;
}

BOOL CTtoCtrlPlatformDlg::GetCodeAndPrintFsy(int index,CTtoCtrlPlatformDlg* tto){
	char sqlBuff[512]={0};
	if(!workFlag) tto->ListDevice.SetItemText(index,4,tto->deviceStats[1].c_str());
	int printFlag = 0;
	int retFlag = 0;
	int totalNum = deviceArr[index]->medPacketsNum  -  deviceArr[index]->printedNum;
	while (totalNum-- > 0){
		if (deviceArr[index]->oneStopFlag||workFlag){
			memset(sqlBuff,0,sizeof(sqlBuff));
			sprintf_s(sqlBuff,512,"update table_task Set tabletPrintedNum = %d where tabletstorid =%s",deviceArr[index]->printedNum,deviceArr[index]->TaskId.c_str());
			EXESQL.put(sqlBuff);
			printFlag = -1; 
			break;
		}	
		if( 1 == deviceArr[index]->deviceType ){
			if(!MarkemPrint("",index,printFlag++==0)){
				if (!workFlag) tto->ListDevice.SetItemText(index, 4, "马肯依玛士设备打印异常！");
				printFlag = -2;
				memset(sqlBuff,0,sizeof(sqlBuff));
				sprintf_s(sqlBuff,512,"update table_task Set tabletPrintedNum = %d where tabletstorid =%s",deviceArr[index]->printedNum,deviceArr[index]->TaskId.c_str());
				EXESQL.put(sqlBuff);
				break;
			}
		}
		else if ( 2 == deviceArr[index]->deviceType ){
			if(!WDJPrint("",index,printFlag++==0,retFlag)){
				if (!workFlag) {
					if(1 == retFlag) tto->ListDevice.SetItemText(index, 4, "伟迪捷设备打印异常,请检查模板参数配置！");
					else if(2 == retFlag) tto->ListDevice.SetItemText(index, 4, "伟迪捷设备打印超时！");
				}
				printFlag = -2;
				memset(sqlBuff,0,sizeof(sqlBuff));
				sprintf_s(sqlBuff,512,"update table_task Set tabletPrintedNum = %d where tabletstorid =%s",deviceArr[index]->printedNum,deviceArr[index]->TaskId.c_str());
				EXESQL.put(sqlBuff);
				break;
			}
		}
		++deviceArr[index]->printedNum;
		if (index == tto->gDevice && !workFlag ){
			memset(sqlBuff,0,sizeof(sqlBuff));
			_itoa_s(deviceArr[index]->medPacketsNum,sqlBuff,512,10);
			tto->m_printTotal.SetWindowText(sqlBuff);
			memset(sqlBuff,0,sizeof(sqlBuff));
			_itoa_s(deviceArr[index]->printedNum,sqlBuff,512,10);
			tto->m_printFinishedNum.SetWindowText(sqlBuff);
		}

	}
	//--------------------------------
	if(0 != printFlag){
		if ( 1 == deviceArr[index]->deviceType ){
			MarkemSendPrintStatus(deviceArr[index]->so,"~ST|04|");
		}
		else if ( 2 == deviceArr[index]->deviceType ){
			WDJSendPrintStatus(deviceArr[index]->so,"SNO|PRS|0|\r");
			WDJSendPrintStatus(deviceArr[index]->so,"SST|4|\r");
		}
	}
	//---------------------------------		
	Guard<Mutex> mutex(m_Mutex);
	if(!workFlag) {
		if(0 == retFlag) tto->ListDevice.SetItemText(index,4,tto->deviceStats[0].c_str());
	}

	memset(sqlBuff,0,sizeof(sqlBuff));
	sprintf_s(sqlBuff,512,"update table_device SET finishFlag=0 where devicetag = '%s'",deviceArr[index]->DeviceId.c_str());
	deviceArr[index]->db.ExeSql(sqlBuff);

	if ( printFlag > 0 ){
		memset(sqlBuff,0,sizeof(sqlBuff));
		sprintf_s(sqlBuff,512,"insert into table_task_history(tabletstorid,taskFlag,tabletname,tabletnumweight,tabletspec,medaddress,tabletnumber,createtime,tabletboxnum,tabletitemnum,medPlaceSimple,tabletPrintedNum,tablet1code) values(%d,3,'%s','%s','%s','%s','%s','%s',%d,'%s','%s',%d,'%s')",
			atoi(deviceArr[index]->TaskId.c_str()),
			deviceArr[index]->medName.c_str(),
			deviceArr[index]->medWeight.c_str(),
			deviceArr[index]->medSpecial.c_str(),
			deviceArr[index]->medPlaceAll.c_str(),
			deviceArr[index]->medBatch.c_str(),
			deviceArr[index]->medDate.c_str(),
			deviceArr[index]->medPacketsNum,
			deviceArr[index]->meditemnum.c_str(),
			deviceArr[index]->medPlace.c_str(),
			deviceArr[index]->printedNum,
			deviceArr[index]->med1Code.c_str());
		deviceArr[index]->db.ExeSql(sqlBuff);
		memset(sqlBuff,0,sizeof(sqlBuff));
		sprintf_s(sqlBuff,512,"delete from table_task where tabletstorid  = %s",deviceArr[index]->TaskId.c_str());	
		deviceArr[index]->db.ExeSql(sqlBuff);
		memset(sqlBuff,0,sizeof(sqlBuff));
		sprintf_s(sqlBuff,512,"update table_device SET tabletstorid=0 where devicetag = '%s'",deviceArr[index]->DeviceId.c_str());
		EXESQL.put(sqlBuff);

		if (!workFlag){
			tto->ListDevice.SetItemText(index, 3, "未分配");
			tto->ListDevice.SetItemText(index, 2, "");
			tto->ListTask.DeleteAllItems();
			tto->GetTaskFromDB();
		}
	}
	return true;
}

BOOL CTtoCtrlPlatformDlg::GuanbiTaskAndInsertIntoHistroy(const char* taskId,int taskFlag,const char* name,const char* weight,const char* spec,const char* placeAll,const char* batch,const char* medDate,const string& packetNum,const char* place,int printedNum,const char* code1){
	char sqlBuff[1024]={0};
	if (-1 != packetNum.find("*"))
	{
		sprintf_s(sqlBuff,1024,"insert into table_task_history(tabletstorid,taskFlag,tabletname,tabletnumweight,tabletspec,medaddress,tabletnumber,createtime,tabletitemnum,medPlaceSimple,tabletPrintedNum,tablet1code) values(%s,%d,'%s',%s,'%s','%s','%s','%s','%s','%s',%d,'%s')",
			taskId,taskFlag,name,weight,spec,placeAll,batch,medDate,packetNum.c_str(),place,printedNum,code1);
		db.ExeSql(sqlBuff);

		memset(sqlBuff,0,sizeof(sqlBuff));
		sprintf_s(sqlBuff,1024,"delete from table_midcode where tabletstorid=%s",taskId);
	}
	else
	{
		sprintf_s(sqlBuff,1024,"insert into table_task_history(tabletstorid,taskFlag,tabletname,tabletnumweight,tabletspec,medaddress,tabletnumber,createtime,tabletboxnum,tabletitemnum,medPlaceSimple,tabletPrintedNum,tablet1code) values(%s,%d,'%s',%s,'%s','%s','%s','%s',%s,'%s','%s',%d,'%s')",
			taskId,taskFlag,name,weight,spec,placeAll,batch,medDate,packetNum.c_str(),"0",place,printedNum,code1);
	}

	return (-1 != (int)db.ExeSql(sqlBuff));
}


int PointNum(int num)
{
	if (num%10 != 0)
	{
		return 3;
	}
	else
	{
		if ((num/10)%10 !=0)
		{
			return 2;
		}
		else
		{
			if ((num/100)%10 != 0)
			{
				return 1;
			}
		}			
	}
	return 0;
}


unsigned int WINAPI CTtoCtrlPlatformDlg::PrintThread(void* lpParam){  
	CTtoCtrlPlatformDlg* tto = (CTtoCtrlPlatformDlg *)lpParam;
	int index = tto->gDevice;

	try{
		deviceArr[index]->db.ExeSql("SET NAMES GBK");
		deviceArr[index]->so.CInit(deviceArr[index]->ip,deviceArr[index]->port);
		deviceArr[index]->so.CCreateSocket();
		char sqlBuff[1024]={0};
		sprintf_s(sqlBuff,1024,"select tabletname,tabletnumweight,tabletspec,medaddress,tabletnumber,createtime,tabletboxnum,medPlaceSimple,tabletPrintedNum,tabletDef1Value,tabletDef2Value,tabletDef3Value,tabletitemnum,dateTo,piwenNum,standard from table_task where tabletstorid=%s",deviceArr[index]->TaskId.c_str());
		deviceArr[index]->iter = deviceArr[index]->db.ExeSql(sqlBuff);
		if (!deviceArr[index]->iter.HasMore()) throw exception("取任务基本信息失败!");

		deviceArr[index]->medName = (string)deviceArr[index]->iter.GetData(0);
		if (2 == tto->weightType)
		{
			deviceArr[index]->medWeight = (string)deviceArr[index]->iter.GetData(1);
		}
		else if (1 == tto->weightType)
		{
            int tmpWeight = (int)deviceArr[index]->iter.GetData(1);
			if (tmpWeight > 0)
			{
				char strWeight[32] = {0};
                int retNum = PointNum(tmpWeight);
				switch (retNum)
				{
				case 0:
					sprintf(strWeight,"%0.0fkg",(DOUBLE)tmpWeight/(DOUBLE)1000);
					break;
				case 1:
					sprintf(strWeight,"%0.1fkg",(DOUBLE)tmpWeight/(DOUBLE)1000);
					break;
				case 2:
					sprintf(strWeight,"%0.2fkg",(DOUBLE)tmpWeight/(DOUBLE)1000);
					break;
				case 3:
					sprintf(strWeight,"%0.3fkg",(DOUBLE)tmpWeight/(DOUBLE)1000);
					break;
				default:
					break;
				}
				//
				deviceArr[index]->medWeight = strWeight;
			}
		}
		else if (0 == tto->weightType)
		{
			string strwei =  (string)deviceArr[index]->iter.GetData(1);
			deviceArr[index]->medWeight = strwei + "g";
		}
		deviceArr[index]->medSpecial = (string)deviceArr[index]->iter.GetData(2);
		deviceArr[index]->medPlace = (string)deviceArr[index]->iter.GetData(7);
		deviceArr[index]->medPlaceAll = (string)deviceArr[index]->iter.GetData(3);
		deviceArr[index]->medBatch = (string)deviceArr[index]->iter.GetData(4);
		deviceArr[index]->medDate = (string)deviceArr[index]->iter.GetData(5);
		
		if (1 == tto->timeType)
		{   
			deviceArr[index]->medPrintDate = deviceArr[index]->medDate.substr(0,4)+"/"+deviceArr[index]->medDate.substr(5,2)+"/"+deviceArr[index]->medDate.substr(8,2);
		}
		else if (3 == tto->timeType)
		{   
			deviceArr[index]->medPrintDate = deviceArr[index]->medDate.substr(0,4)+"."+deviceArr[index]->medDate.substr(5,2)+"."+deviceArr[index]->medDate.substr(8,2);
		}
		else
			deviceArr[index]->medPrintDate = deviceArr[index]->medDate.substr(0,4)+deviceArr[index]->medDate.substr(5,2)+deviceArr[index]->medDate.substr(8,2);
		deviceArr[index]->medPacketsNum = (int)deviceArr[index]->iter.GetData(6);
		deviceArr[index]->printedNum = (int)deviceArr[index]->iter.GetData(8);
		deviceArr[index]->medDef1 = (string)deviceArr[index]->iter.GetData(9);
		deviceArr[index]->medDef2 = (string)deviceArr[index]->iter.GetData(10);
		deviceArr[index]->medDef3 = (string)deviceArr[index]->iter.GetData(11);
		deviceArr[index]->meditemnum = (string)deviceArr[index]->iter.GetData(12);
		string tmpDateTo = (string)deviceArr[index]->iter.GetData(13);
		if (1 == tto->timeType)
		{  
			deviceArr[index]->medDateTo = tmpDateTo.substr(0,4)+"/"+tmpDateTo.substr(5,2)+"/"+tmpDateTo.substr(8,2);
		}
		else if (3 == tto->timeType)
		{   
			deviceArr[index]->medDateTo = tmpDateTo.substr(0,4)+"."+tmpDateTo.substr(5,2)+"."+tmpDateTo.substr(8,2);
		}
		else
			deviceArr[index]->medDateTo = tmpDateTo.substr(0,4)+tmpDateTo.substr(5,2)+tmpDateTo.substr(8,2);
		deviceArr[index]->medPiwenNum = (string)deviceArr[index]->iter.GetData(14);
		deviceArr[index]->medStandard = (string)deviceArr[index]->iter.GetData(15);
		
		char strMsg[0x100]={0};
		if(!deviceArr[index]->so.CConnect()){
			sprintf_s(strMsg,0x100,"连接溯源打码机(%s:%d)失败！",deviceArr[index]->ip.c_str(),deviceArr[index]->port);

			throw exception(strMsg);
		}
		memset(sqlBuff,0,sizeof(sqlBuff));
        _itoa_s(deviceArr[index]->medPacketsNum,sqlBuff,1024,10);
		if(!workFlag) tto->m_printTotal.SetWindowText(sqlBuff);
		char tmpTaskId[16]={0};
		sprintf_s(tmpTaskId,16,"%s",deviceArr[index]->TaskId.c_str());
		if ( '-' == tmpTaskId[0])
		{
			GetCodeAndPrintFsy(index,tto);
		}
		else
			GetCodeAndPrint(index,tto);
	}catch(const exception& e){
		if(!workFlag) {
			tto->ListDevice.SetItemText(index,4,e.what());
		}
	}
	if (!workFlag){ 
		deviceArr[index]->so.CShutdownSocket();
		deviceArr[index]->so.CCloseSocket();
		CloseHandle(deviceArr[index]->hStatusThread);
		deviceArr[index]->hStatusThread = NULL;
		delete deviceArr[index];
		deviceArr.erase(index);
	}
	if(!workFlag){
		if( deviceArr.size() == 0 ) {
			tto->deviceBtnShowFlag = true;
			((CButton *)tto->GetDlgItem(IDC_BUTTON_UPDATE_DEVICE))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_DEVICE)));
		}
	}
	TRACE("One thread quit\n");
	return 1;  
}


void CTtoCtrlPlatformDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonDown(nFlags, point);
	PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x,point.y));
}


void CTtoCtrlPlatformDlg::OnQuxiaoTask()
{
	string sTask = ListDevice.GetItemText(gDevice,2);
	if("" == sTask)
	{
		ListDevice.SetItemText(gDevice,4,"该设备处于未分配任务状态！");
		return;
	}
	string sRunStatus = ListDevice.GetItemText(gDevice,4);
	if("正在打印" == sRunStatus)
	{
		MessageBox("该设备处于打印状态，请先停止打印后再取消任务！","设备列表操作提示：",MB_YESNO);
		return;
	}
	if (MessageBox("是否确定取消任务？","设备列表操作提示：",MB_YESNO) == IDNO){
		return;
	}
	DisMatchTaskFromDevice(sTask);

	ListDevice.DeleteAllItems();
	GetDeviceListFromDB();
}

void CTtoCtrlPlatformDlg::OnBnClickedButtonAddCommTask()
{
	CDlgFeiSuYuan fsyDlg(0,"");
	fsyDlg.DoModal();
	if (fsyDlg.writeDbFlag)
	{
		ListTask.DeleteAllItems();
		GetTaskFromDB();
	}
}

void CTtoCtrlPlatformDlg::OnEditTask()
{
	string tmpTaskId = ListTask.GetItemText(gTask,0);
	if ("" == tmpTaskId)
	{
		return;
	}
	int ret = tmpTaskId.find('-',0);
	if (0 == ret)
	{
		CDlgFeiSuYuan fsyDlg(1,tmpTaskId);
		fsyDlg.DoModal();
		if (fsyDlg.writeDbFlag)
		{
			ListTask.DeleteAllItems();
			GetTaskFromDB();
			ListDevice.DeleteAllItems();
			GetDeviceListFromDB();
		}
	}else{
		CWriteSimplePlaceDlg placeDlg(tmpTaskId);
		placeDlg.DoModal();

		string sPlace = "";
		string sCode1 = "";
		string sPec = "";
		GetSimplePlaceFromDB(tmpTaskId,sPlace,sCode1,sPec);
		ListTask.SetItemText(gTask, 8, sPlace.c_str());//设置数据
	    ListTask.SetItemText(gTask, 10, sCode1.c_str());//设置数据
		ListTask.SetItemText(gTask, 6, sPec.c_str());//设置数据
	}

}

void CTtoCtrlPlatformDlg::OnMidPack()
{
	string tmpTaskId = ListTask.GetItemText(gTask,0);
	if ("" == tmpTaskId)
	{
		return;
	}
	int ret = tmpTaskId.find('-',0);
	if (0 != ret)
	{
		string midname = ListTask.GetItemText(gTask,2);
		string midbatch = ListTask.GetItemText(gTask,3);
		string midweight = ListTask.GetItemText(gTask,5);
		string midspec = ListTask.GetItemText(gTask,6);
		string midsimplace = ListTask.GetItemText(gTask,8);
		string middate = ListTask.GetItemText(gTask,9);
		middate = middate.substr(0,10);

		CMidPacketInfo midPackInfodlg(tmpTaskId,midname,midbatch,midweight,midspec,gCpyNameSim,midsimplace,middate);
		midPackInfodlg.DoModal();
	}
	else
	{
		MessageBox("请选择溯源任务！","提示:",MB_SYSTEMMODAL);
	}
}

void CTtoCtrlPlatformDlg::OnOutSystem()
{
	if(MessageBox("是否确定退出？","溯源打码控制平台",MB_YESNO) == IDNO) return;
	workFlag = true;	
	for (map<int,DeviceArr*>::iterator iter = deviceArr.begin();iter != deviceArr.end();++iter){
		if( WAIT_OBJECT_0 == WaitForSingleObject(iter->second->hStatusThread, 60*1000)) {
			iter->second->so.CShutdownSocket();
			iter->second->so.CCloseSocket();
			CloseHandle(iter->second->hStatusThread);
			delete iter->second;
			iter->second = NULL; 	    
		}
	}
	deviceArr.erase(deviceArr.begin(), deviceArr.end());
	if( WAIT_OBJECT_0 == WaitForSingleObject(exeSqlH, 60*1000)) TRACE("exesql thread quit\n");
	CloseHandle(exeSqlH); 
	if( WAIT_OBJECT_0 == WaitForSingleObject(UiHandle, 60*1000)) TRACE("uihandle thread quit\n");
	CloseHandle(UiHandle);

	if (NULL != hInstMaster)
	{
		KeyClose();
		FreeLibrary(hInstMaster);
		hInstMaster = NULL;
	}
	DeleteTray();
	OnCancel();
}

void CTtoCtrlPlatformDlg::OnBackSystem()
{
	ShowWindow(SW_NORMAL);
}
