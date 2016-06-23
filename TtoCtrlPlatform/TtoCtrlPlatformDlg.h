#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include <windows.h>
#include <process.h>
#include <string.h>
#include <list>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include "DB.h"
#include "TtoJson.h"
#include "Function.h"
#include "png.h"
#include "Socket.h"
#include "Definition.h"
#include "Queue.h"
using namespace std;

#pragma comment( lib, "Wsock32.lib" ) 
#define DEFAULT_BUFFER 256

#define UTF8_ONE_START      (0xOOO1)
#define UTF8_ONE_END        (0x007F)
#define UTF8_TWO_START      (0x0080)
#define UTF8_TWO_END        (0x07FF)
#define UTF8_THREE_START    (0x0800)
#define UTF8_THREE_END      (0xFFFF)

typedef unsigned short  UTF16;  /* at least 16 bits */ 
typedef unsigned char   UTF8; 

typedef int (USBKeyOpen_T)(int port, int baudrate);
typedef int (USBKeyClose_T)();
typedef int (USBKeyID_T)(char* id);
typedef int (USBKeyAuth_T)(char* dst, const char* src);

struct DeviceArr{
	Socket so;
	string TaskId;
	string DeviceId;
	WSAEVENT clientEvent;
	int indexParam;
	string medName;
	string medWeight;
	string medSpecial;
	string medPlace;
	string medBatch;
	string medDate;
	string medPrintDate;
	string med1Code;
	string medDateTo;
	string medPiwenNum;
	string medStandard;
	string medDef1;
	string medDef2;
	string medDef3;
	DWORD medPacketsNum;
	string medPlaceAll;
	string meditemnum;

	string InmedCode2d;
	string InmedName;
	string InmedWeight;
	string InmedSpecial;
	string InmedPlace;
	string InmedBatch;
	string InmedDate;
	string Inmed1Code;
	string InmedDateTo;
	string InmedPiwenNum;
	string InmedStandard;
	string InCmpNameSim;
	string InmedDef1;
	string InmedDef2;
	string InmedDef3;

	HANDLE hStatusThread;
	char gArrRetCode[64][32];
	int printedNum;
	string ip;
	int port;
	int deviceType;
	//int oneSendCount;
	bool oneStopFlag;
	DB<CMySql> db;
	DB<CMySql>::const_iterator iter;
	DeviceArr():db(new CMySql(DB_IP,DB_NAME,DB_USER,DB_PASS)){}
private:
	const DeviceArr& operator = ( const DeviceArr& );
	DeviceArr( const DeviceArr& );
};

 typedef struct MytagNMITEMACTIVATE
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
 } MyNMITEMACTIVATE, *LMyPNMITEMACTIVATE;


class CTtoCtrlPlatformDlg : public CDialog
{
public:
	CTtoCtrlPlatformDlg(CWnd* pParent = NULL);	
	enum { IDD = IDD_TTOCTRLPLATFORM_DIALOG };
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	string testsend(string& taskIP,int taskPort,char* usbId);

protected:
	HICON m_hIcon;
	CBrush m_brush;
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl ListTask;
	CListCtrl ListDevice;
	int gTask;
	int gDevice;
	CEdit m_Edit2dCode;
	CEdit m_EditCode2;
	CEdit m_printTotal;
	CEdit m_printFinishedNum;
	CFont m_Fontbs;
	BOOL hideFlag;
	string gCpyNameSim;
public:
	bool Device_Task(const string& deviceid,int& taskid);
	bool Device_Task(const int& taskid,string& deviceid);
	bool Device_Task(const string& taskid,string& deviceid);
	BOOL GetSimplePlaceFromDB(string inTaskId,string &sPlace,string &sCode1);
	BOOL GetSimplePlaceFromDB(string inTaskId,string &sPlace,string &sCode1,string &sPec);
	BOOL DisMatchTaskFromDevice(string taskId);
	BOOL GetTotalNumAndFinishNum(const string& taskId,string& totalNum,string& finishNum);
	BOOL GetTaskFromDB();
	BOOL GetDeviceListFromDB();
	bool ParseDeviceJson(HWND hwind,const char* jsonBuf,int jsonLen);
	BOOL CheckTaskIdInUseOrNot(const char* taskId);
	BOOL SetTaskListFlag(const char* taskId,int taskFinishFlag);
	BOOL DelOneFromTaskList( const char* taskId );
	BOOL Get2dcodeTotalAndPrintNum(const char* taskId,int& totalNum,int& printNum);
	BOOL GuanbiTaskAndInsertIntoHistroy(const char* taskId,int taskFlag,const char* name,const char* weight,const char* spec,const char* placeAll,
		                                const char* batch,const char* medDate,const string& packetNum,const char* place,int printedNum,const char* code1);
	string GetDownData(IN const char* sendCommand, IN const char* ip, IN int port, OUT unsigned char** buffer, OUT size_t& bufferLen) const;
	string SentUsbIdToServer(string& taskIP,int taskPort,string& taskId,char* usbId);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonUpdateDevice();
	afx_msg void OnBnClickedButtonTaskHistory();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnBnClickedButtonUpdateTask();
	afx_msg void OnBnClickedButtonMin();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg LRESULT OnSystemTray(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonChangePass();
	afx_msg void OnQuxiaoTask();
	afx_msg void OnBnClickedButtonAddCommTask();
	afx_msg void OnEditTask();
	afx_msg void OnTaskFenPei();
	afx_msg void OnTaskDownload();
	afx_msg void OnTaskGuanBi();
	afx_msg void OnTaskQuXiao();
	afx_msg void OnModelSet();
	afx_msg void OnMidPack();
	afx_msg void OnDevicePrintStop();
	afx_msg void OnDevicePrintStart();
	void DeleteTray();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnNMClickListDevice(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickListTask(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickListTask(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickListDevice(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	static DWORD UTF8ToUTF16(UTF8* pUTF8Start, UTF8* pUTF8End, UTF16* pUTF16Start, UTF16* pUTF16End);
	static int UTF16ToUTF8(UTF16* pUTF16Start, UTF16* pUTF16End, UTF8* pUTF8Start, UTF8* pUTF8End);
	static BOOL Gb2312ToUtf8(const char *pcGb2312,char* pcUtf8,int &utf8Len);
	static BOOL UTF8ToGB2312(const void * pIn,int inLen,char *pOut,int *outLen );
	static bool MarkemSendPrintInfo( const Socket& so,const string& sendStr,int &recvStrLen,char* recvStr);
	static bool MarkemGetPrintInfo(const char* GetStr,const int len,string& conferStr);
	static bool MarkemSendPrintStatus( const Socket& so,const string& sendStr);
	static bool WDJSendPrintStatus(const Socket& so,const string& sendStr);
	static BOOL MarkemPrint(const char *code2d,int index,bool firstFlag);
	static BOOL WDJPrint(const char *code2d,int index,bool firstFlag,int& retFlag);
	static BOOL GetCodeAndPrintFsy(int index,CTtoCtrlPlatformDlg* tto/*,int totalNum*/);
	static BOOL GetCodeAndPrint(int index,CTtoCtrlPlatformDlg* tto);
private:
	DB<CMySql> db;
	DB<CMySql>::const_iterator iter;
	char sqlBuff[512];
	map<int,string> taskStats;
	map<int,string> deviceStats;
	string deviceIP,taskIP,orgid;
	int devicePort,taskPort,comPort,timeType,weightType;
	char usbId[64],usbAuth[64];
	static bool workFlag;	
	static Queue<string> EXESQL;
	static Queue<string> UIQUEUE;
	static unsigned int __stdcall exeSql( void* );
	static unsigned int __stdcall uiQueue( void* );
	static HANDLE exeSqlH;
	static HANDLE UiHandle;
	BOOL initUsb();
	static map<int,DeviceArr*> deviceArr;
	static unsigned int WINAPI PrintThread(void* lpParam);
	static Mutex m_Mutex;
	bool deviceBtnShowFlag;
	USBKeyOpen_T* KeyOpen;
	USBKeyClose_T* KeyClose;
	USBKeyID_T* KeyID;
	USBKeyAuth_T* KeyAuth;
	HINSTANCE hInstMaster;
public:
	afx_msg void OnOutSystem();
	afx_msg void OnBackSystem();
};
	