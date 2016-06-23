// TtoParamConfig.cpp : 实现文件
//

#include "stdafx.h"
#include "TtoCtrlPlatform.h"
#include "TtoParamConfig.h"
#include "Definition.h"


// CTtoParamConfig 对话框

IMPLEMENT_DYNAMIC(CTtoParamConfig, CDialog)

CTtoParamConfig::CTtoParamConfig(int index,string deciceId,string taskId,CWnd* pParent/*=NULL*/)
	: CDialog(CTtoParamConfig::IDD, pParent),db(new CMySql(DB_IP,DB_NAME,DB_USER,DB_PASS))
{
	iDeviceId = index;//考虑删掉
    gDeviceId = deciceId;
	gTaskId = taskId;
}

CTtoParamConfig::~CTtoParamConfig()
{
}

void CTtoParamConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_NAME, m_ComBoxMedName);
	DDX_Control(pDX, IDC_CHECK_WEIGHT, m_ComBoxMedWeight);
	DDX_Control(pDX, IDC_CHECK_SPEC, m_ComBoxMedSpecial);
	DDX_Control(pDX, IDC_CHECK_PLACE, m_ComBoxMedPlace);
	DDX_Control(pDX, IDC_CHECK_BATCH, m_ComBoxMedBatch);
	DDX_Control(pDX, IDC_CHECK_DATE, m_ComBoxMedDate);
	DDX_Control(pDX, IDC_CHECK_QR_CODE, m_ComBoxMedQRCode);
	DDX_Control(pDX, IDC_CHECK_1_CODE, m_ComBox1Code);
	DDX_Control(pDX, IDC_CHECK_DEFINE1, m_ComBoxMedDefine1);
	DDX_Control(pDX, IDC_CHECK_DEFINE2, m_ComBoxMedDefine2);
	DDX_Control(pDX, IDC_CHECK_DEFINE3, m_ComBoxMedDefine3);
	DDX_Control(pDX, IDC_CHECK_DATE_TO, m_ComBoxMedDataTo);
	DDX_Control(pDX, IDC_CHECK_OFFDOC, m_ComBoxMedOffdoc);
	DDX_Control(pDX, IDC_CHECK_STANDARD, m_ComBoxMedStandard);
	DDX_Control(pDX, IDC_CHECK_CMY_SIM_NAME, m_ComBoxCpyName);
}

BEGIN_MESSAGE_MAP(CTtoParamConfig, CDialog)
	ON_BN_CLICKED(IDOK, &CTtoParamConfig::SetDeviceConfigParamToDB)
	//ON_BN_CLICKED(IDC_BUTTON_CFG_QUIT, &CTtoParamConfig::OnBnClickedButtonCfgQuit)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_PZ_QUIT, &CTtoParamConfig::OnBnClickedButtonPzQuit)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CTtoParamConfig 消息处理程序
void CTtoParamConfig::SetDeviceConfigParamToDB(){
	//unsigned char pf = 0;
	int pf = 0;
	pf |= m_ComBoxMedDefine3.GetCheck()<<14;
	pf |= m_ComBoxMedDefine2.GetCheck()<<13;
	pf |= m_ComBoxMedDefine1.GetCheck()<<12;
	pf |= m_ComBoxCpyName.GetCheck()<<11;
	pf |= m_ComBoxMedStandard.GetCheck()<<10;
	pf |= m_ComBoxMedOffdoc.GetCheck()<<9;
	pf |= m_ComBoxMedDataTo.GetCheck()<<8;
	pf |= m_ComBox1Code.GetCheck()<<7;
    pf |= m_ComBoxMedName.GetCheck()<<6;
	pf |= m_ComBoxMedWeight.GetCheck()<<5;
	pf |= m_ComBoxMedSpecial.GetCheck()<<4;
	pf |= m_ComBoxMedPlace.GetCheck()<<3;
	pf |= m_ComBoxMedBatch.GetCheck()<<2;
	pf |= m_ComBoxMedDate.GetCheck()<<1;
	pf |= m_ComBoxMedQRCode.GetCheck();
	char tmpTaskId[16]={0};
	sprintf_s(tmpTaskId,16,"%s",gTaskId.c_str());
	char buff[0x200]={0};
	if ('-' == tmpTaskId[0])
	{
		sprintf_s(buff,0x200,"update table_task set fsyPrintFlag=%d where tabletstorid=%s",pf,gTaskId.c_str());	
		if (-1 != (int)db.ExeSql(buff))
		{		
			if(pf > 0){
				MessageBox("设备信息配置成功！","温馨提示：",MB_SYSTEMMODAL);
			}else{
				MessageBox("设备未配置！","温馨提示：",MB_SYSTEMMODAL);
			}
		}
	}
	else
	{
		sprintf_s(buff,0x200,"update table_device set printFlag=%d where devicetag='%s'",pf,gDeviceId.c_str());	
		if (-1 != (int)db.ExeSql(buff))
		{		
			if(pf > 0){
				MessageBox("设备信息配置成功！","温馨提示：",MB_SYSTEMMODAL);
			}else{
				MessageBox("设备未配置！","温馨提示：",MB_SYSTEMMODAL);
			}
		}
	}

 	OnOK();
}

//什么时候创建表  建表的时候是否考虑全部初始值为0
BOOL CTtoParamConfig::OnInitDialog()
{
	CDialog::OnInitDialog();
	char sqlBuff[0x200] = {0};
	char tmpTaskId[16]={0};
	sprintf_s(tmpTaskId,16,"%s",gTaskId.c_str());
	int pf=0;
	if ('-' == tmpTaskId[0])
	{
		sprintf_s(sqlBuff,0x200,"select fsyPrintFlag from table_task where tabletstorid = %s",tmpTaskId);
		iter = db.ExeSql(sqlBuff); 
		if(iter.HasMore()){ //读取结果集中的数据，返回的是下一行。因为保存结果集时，当前的游标在第一行【之前】   
			pf = (int)iter.GetData(0);
			memset(sqlBuff,0,sizeof(sqlBuff));
			sprintf_s(sqlBuff,0x200,"update table_device set printFlag = %d where devicetag = '%s'",pf,gDeviceId.c_str());
			iter = db.ExeSql(sqlBuff); 
		} 
	}
	else 
	{
		//从数据库读取出来展示给用户看 哪些已经选择了
		sprintf_s(sqlBuff,0x200,"select printFlag from table_device where devicetag = '%s'",gDeviceId.c_str());
		iter = db.ExeSql(sqlBuff); 
		if(iter.HasMore()){ //读取结果集中的数据，返回的是下一行。因为保存结果集时，当前的游标在第一行【之前】   
			pf = (int)iter.GetData(0);
		} 
	}
	m_ComBoxMedDefine3.SetCheck(pf&0x4000);
	m_ComBoxMedDefine2.SetCheck(pf&0x2000);
	m_ComBoxMedDefine1.SetCheck(pf&0x1000);
	m_ComBoxCpyName.SetCheck(pf&0x800);
	m_ComBoxMedStandard.SetCheck(pf&0x400);
	m_ComBoxMedOffdoc.SetCheck(pf&0x200);
	m_ComBoxMedDataTo.SetCheck(pf&0x100);
	m_ComBox1Code.SetCheck(pf&0x80);
	m_ComBoxMedName.SetCheck(pf&0x40);
	m_ComBoxMedWeight.SetCheck(pf&0x20);
	m_ComBoxMedSpecial.SetCheck(pf&0x10);
	m_ComBoxMedPlace.SetCheck(pf&0x8);
	m_ComBoxMedBatch.SetCheck(pf&0x4);
	m_ComBoxMedDate.SetCheck(pf&0x2);
	m_ComBoxMedQRCode.SetCheck(pf&0x1);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


BOOL CTtoParamConfig::PreTranslateMessage(MSG* pMsg)
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


void CTtoParamConfig::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect   rect;   
	GetClientRect(&rect);   
	CDC   dcMem;   
	dcMem.CreateCompatibleDC(&dc);   
	CBitmap   bmpBackground;   
	bmpBackground.LoadBitmap(IDB_BITMAP_PZ_BJ);
	BITMAP   bitmap;   
	bmpBackground.GetBitmap(&bitmap);   
	CBitmap   *pbmpOld=dcMem.SelectObject(&bmpBackground);   
	dc.StretchBlt(0,0,rect.Width(),rect.Height(),&dcMem,0,0,bitmap.bmWidth,bitmap.bmHeight,SRCCOPY); 

	((CButton *)GetDlgItem(IDOK))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_PZ_OK))); 
	((CButton *)GetDlgItem(IDC_BUTTON_PZ_QUIT))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_MAIN_QUIT))); 
}

void CTtoParamConfig::OnBnClickedButtonPzQuit()
{
	OnCancel();
}

void CTtoParamConfig::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnLButtonDown(nFlags, point);
		PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x,point.y));
}
