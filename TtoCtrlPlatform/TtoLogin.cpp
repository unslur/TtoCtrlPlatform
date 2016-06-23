#include "stdafx.h"
#include "TtoCtrlPlatform.h"
#include "TtoLogin.h"
#include "Definition.h"
#include "DB.h"
#include "Socket.h"
IMPLEMENT_DYNAMIC(CTtoLogin, CDialog)

CTtoLogin::CTtoLogin(CWnd* pParent /*=NULL*/)
	: CDialog(CTtoLogin::IDD, pParent){
	passFlag = false;
}

CTtoLogin::~CTtoLogin(){}

void CTtoLogin::DoDataExchange(CDataExchange* pDX){
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_LOGIN_NAME, m_EditLoginName);
	DDX_Control(pDX, IDC_EDIT_LOGIN_PASS, m_EditLoginPass);
}

BEGIN_MESSAGE_MAP(CTtoLogin, CDialog)
	ON_BN_CLICKED(ID_LOGIN, &CTtoLogin::OnBnClickedLogin)
	ON_BN_CLICKED(IDCANCEL, &CTtoLogin::OnBnClickedCancel)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


void CTtoLogin::OnBnClickedLogin(){
	////////取消Ukey

#if 0
	DB<CMySql> db(new CMySql(DB_IP,DB_NAME,DB_USER,DB_PASS));
	DB<CMySql>::const_iterator iter;

	CString tmpName;
	GetDlgItemText(IDC_EDIT_LOGIN_NAME,tmpName);
	
	char sqlBuff[0x200] = {0};
	sprintf_s(sqlBuff,0x200,"SELECT DECODE(loginPass,'key') FROM table_login where loginName='%s'",iter.FormatSqlStr((string)tmpName).c_str());
	iter = db.ExeSql(sqlBuff);
	if(iter.HasMore()){	
		CString tmpPass;
		GetDlgItemText(IDC_EDIT_LOGIN_PASS,tmpPass);
		string sPass = iter.GetData(0);
		if( sPass == (string)tmpPass ){
			passFlag = true;
			OnCancel();			
		}
		else{
			MessageBox("密码错误！","登陆提示：",MB_SYSTEMMODAL);
		}
	}else{
		MessageBox("该用户不存在！","登陆提示：",MB_SYSTEMMODAL);   
	}
#endif

	///////////////////////////////////////////
#if 1
	DB<CMySql> db(new CMySql(DB_IP,DB_NAME,DB_USER,DB_PASS));
	DB<CMySql>::const_iterator iter;
	CString tmpNameC;
	CString tmpPassC;

	GetDlgItemText(IDC_EDIT_LOGIN_NAME,tmpNameC);
	GetDlgItemText(IDC_EDIT_LOGIN_PASS,tmpPassC);
	string tmpName(tmpNameC.GetBuffer(tmpNameC.GetLength()));
	string tmpPass(tmpPassC.GetBuffer(tmpPassC.GetLength()));
	CString m_strCurrentIP;
	CString m_strCurrenthttp;
	CString m_strCurrentPort;
	CString m_strCurrentHttpPort;
	CString m_strCurrentSimName;
	
	::GetPrivateProfileString(_T("Server"),"IP",_T("没找到IP信息"),m_strCurrentIP.GetBuffer(MAX_PATH),MAX_PATH,_T("./config.ini")); 
	::GetPrivateProfileString(_T("Server"),"Port",_T("没找到IP信息"),m_strCurrentPort.GetBuffer(MAX_PATH),MAX_PATH,_T("./config.ini")); 
	::GetPrivateProfileString(_T("WebServer"),"Http",_T("没找到IP信息"),m_strCurrenthttp.GetBuffer(MAX_PATH),MAX_PATH,_T("./config.ini")); 
	::GetPrivateProfileString(_T("WebServer"),"HttpPort",_T("没找到IP信息"),m_strCurrentHttpPort.GetBuffer(MAX_PATH),MAX_PATH,_T("./config.ini")); 
	::GetPrivateProfileString(_T("Info"),"SimpleName",_T("没找到IP信息"),m_strCurrentSimName.GetBuffer(MAX_PATH),MAX_PATH,_T("./config.ini")); 
	string ip(m_strCurrentIP.GetBuffer(m_strCurrentIP.GetLength()));
	string port(m_strCurrentPort.GetBuffer(m_strCurrentPort.GetLength()));
	string http(m_strCurrenthttp.GetBuffer(m_strCurrenthttp.GetLength()));
	string httpport(m_strCurrentHttpPort.GetBuffer(m_strCurrentHttpPort.GetLength()));
	string simname(m_strCurrentSimName.GetBuffer(m_strCurrentSimName.GetLength()));
	Socket so;
	
	so.CInit(ip,atoi(port.c_str()));
	so.CCreateSocket();
	string sendcommand="LOGIN "+tmpName+" "+tmpPass;
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
	if (strstr(recvBuff,"OK"))
	{
		char sqlBuff[0x200] = {0};
		string recvString=recvBuff;
		recvString=recvString.substr(recvString.find('|')+1);
		string orgid=recvString.substr(0,recvString.find('|'));
		/*recvString=recvString.substr(recvString.find('|')+1);
		string =recvString.substr(0,recvString.find('|'));
		recvString=recvString.substr(recvString.find('|')+1);
		recvString=recvString.substr(recvString.find('|')+1);
		recvString=recvString.substr(recvString.find('|')+1);*/
		sprintf_s(sqlBuff,0x100,"delete from table_login where orgid='%s'",orgid.c_str());
		db.ExeSql(sqlBuff);
		
		sprintf_s(sqlBuff,0x100,"insert into table_login(loginName,orgid,taskServerIp,taskServerPort,httpServerIp,httpServerPort,cpyNameSim) values('%s','%s','%s',%d,'%s',%d,'%s')",tmpName.c_str(),orgid.c_str(),ip.c_str(),atoi(port.c_str()),http.c_str(),atoi(httpport.c_str()),simname.c_str());
		//sprintf_s(sqlBuff,0x200,"insert into table_login(orgId,taskServerIp,taskServerPort,httpServerIp,httpServerPort,comPort,cpyNameSim,timeType,weightType) values);
		db.ExeSql(sqlBuff);
		passFlag = true;
		OnCancel();			
	}else{
		MessageBox("密码或帐号错误！","登陆提示：",MB_SYSTEMMODAL);
	}
#endif
}


void CTtoLogin::OnBnClickedCancel(){
	OnCancel();
}


void CTtoLogin::OnPaint(){
	CPaintDC dc(this); // device context for painting
	CRect   rect;   
	GetClientRect(&rect);   
	CDC   dcMem;   
	dcMem.CreateCompatibleDC(&dc);   
	CBitmap   bmpBackground;   
	bmpBackground.LoadBitmap(IDB_BITMAP_LOGIN);
	BITMAP   bitmap;   
	bmpBackground.GetBitmap(&bitmap);   
	CBitmap   *pbmpOld=dcMem.SelectObject(&bmpBackground);   
	dc.StretchBlt(0,0,rect.Width(),rect.Height(),&dcMem,0,0,bitmap.bmWidth,bitmap.bmHeight,SRCCOPY); 
}

BOOL CTtoLogin::PreTranslateMessage(MSG* pMsg){
	if(pMsg -> message == WM_KEYDOWN)	{
		if(pMsg -> wParam == VK_ESCAPE)
			return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CTtoLogin::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonDown(nFlags, point);
	PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x,point.y));
}

BOOL CTtoLogin::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	((CButton *)GetDlgItem(ID_LOGIN))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_LG_LOGIN)));  
	((CButton *)GetDlgItem(IDCANCEL))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_LG_QUIT)));  

	SetWindowPos(&wndTopMost,0,0,0,0,SWP_NOSIZE | SWP_NOMOVE); 
	return TRUE;  // return TRUE unless you set the focus to a control
}
