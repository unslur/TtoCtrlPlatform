#include "stdafx.h"
#include "TtoCtrlPlatform.h"
#include "WriteSimplePlaceDlg.h"

IMPLEMENT_DYNAMIC(CWriteSimplePlaceDlg, CDialog)

CWriteSimplePlaceDlg::CWriteSimplePlaceDlg(string task,CWnd* pParent /*=NULL*/)
	: CDialog(CWriteSimplePlaceDlg::IDD, pParent),db(new CMySql(DB_IP,DB_NAME,DB_USER,DB_PASS))
{
	inTaskId = task;
	db.ExeSql("SET NAMES GBK");
}

CWriteSimplePlaceDlg::~CWriteSimplePlaceDlg()
{

}

void CWriteSimplePlaceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_SIMPLE_PLACE, m_EditSimplePlace);
	DDX_Control(pDX, IDC_EDIT_SY_CODE1, m_Ecode1);
	DDX_Control(pDX, IDC_EDIT_SY_DATE_TO, m_EdateTo);
	DDX_Control(pDX, IDC_EDIT_SY_PIWEN_NUM, m_EoffDocNum);
	DDX_Control(pDX, IDC_EDIT_SY_STANDARD, m_Estandard);
	DDX_Control(pDX, IDC_EDIT_SY_DEF1, m_Edef1);
	DDX_Control(pDX, IDC_EDIT_SY_DEF2, m_Edef2);
	DDX_Control(pDX, IDC_EDIT_SY_DEF3, m_Edef3);
	DDX_Control(pDX, IDC_DATETIMEPICKER_SY_DATE, m_SyDate);
}


BEGIN_MESSAGE_MAP(CWriteSimplePlaceDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CWriteSimplePlaceDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_SP_QUIT, &CWriteSimplePlaceDlg::OnBnClickedButtonSpQuit)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_SY_DATE, &CWriteSimplePlaceDlg::OnDtnDatetimechangeDatetimepickerSyDate)
END_MESSAGE_MAP()


void CWriteSimplePlaceDlg::OnBnClickedOk()
{	
	CString tmpBuf;
	CString tmpPlace;
	((CEdit*)GetDlgItem(IDC_EDIT_SIMPLE_PLACE))->GetWindowText(tmpPlace);
	string sPlace = tmpPlace;
	if (sPlace == "")
	{
		MessageBox("请输入产地简介!","修改打印产地简介：",MB_SYSTEMMODAL);
		return;
	}

	((CEdit*)GetDlgItem(IDC_EDIT_SPEC))->GetWindowText(tmpBuf);
	string sPec = tmpBuf;
	if (sPec == "")
	{
		MessageBox("打印规格为空!","温馨提示：",MB_SYSTEMMODAL);
		return;
	}

	CString tmpCode1;
	((CEdit*)GetDlgItem(IDC_EDIT_SY_CODE1))->GetWindowText(tmpCode1);
	string sCode1 = tmpCode1;
	if (sCode1 != ""){
		if (12 != sCode1.length()){//
			MessageBox("商品码长度需12位!","温馨提示：",MB_SYSTEMMODAL);
			return;
		}
	}

	((CEdit*)GetDlgItem(IDC_EDIT_SY_DATE_TO))->GetWindowText(tmpBuf);
	string sDateTo = tmpBuf;

	((CEdit*)GetDlgItem(IDC_EDIT_SY_PIWEN_NUM))->GetWindowText(tmpBuf);
	string sOffdoc = tmpBuf;

	((CEdit*)GetDlgItem(IDC_EDIT_SY_STANDARD))->GetWindowText(tmpBuf);
	string sStandard = tmpBuf;

	((CEdit*)GetDlgItem(IDC_EDIT_SY_DEF1))->GetWindowText(tmpBuf);
	string sDef1 = tmpBuf;

	((CEdit*)GetDlgItem(IDC_EDIT_SY_DEF2))->GetWindowText(tmpBuf);
	string sDef2 = tmpBuf;

	((CEdit*)GetDlgItem(IDC_EDIT_SY_DEF3))->GetWindowText(tmpBuf);
	string sDef3 = tmpBuf;

	//时间格式
	int timeType1 = ((CButton *)GetDlgItem(IDC_RADIO_TIME1))->GetCheck();
	int timeType2 = ((CButton *)GetDlgItem(IDC_RADIO_TIME2))->GetCheck();
	int timeType3 = ((CButton *)GetDlgItem(IDC_RADIO_TIME3))->GetCheck();
    int timeType = 1;
	if (1 == timeType1)
	{
		timeType = 1;
	}
	else if (1 == timeType2)
	{
		timeType = 2;
	}
	else if (1 == timeType3)
	{
		timeType = 3;
	}

	int weightType1 = ((CButton *)GetDlgItem(IDC_RADIO_WEIGHT_G))->GetCheck();
	int weightType2 = ((CButton *)GetDlgItem(IDC_RADIO_WEIGHT_KG))->GetCheck();
	int weightType3 = ((CButton *)GetDlgItem(IDC_RADIO_WEIGHT_NO))->GetCheck();
	int weightType = 0;
	if (1 == weightType1)
	{
		weightType = 0;
	}
	else if (1 == weightType2)
	{
		weightType = 1;
	}
	else if (1 == weightType3)
	{
		weightType = 2;
	}
	char sqlBuff[512]={0};
	sprintf_s(sqlBuff,512,"update table_login SET timeType = %d,weightType = %d",timeType,weightType);
	iter = db.ExeSql(sqlBuff);
    memset(sqlBuff,0,sizeof(sqlBuff));
	sprintf_s(sqlBuff,512,"update table_task SET medPlaceSimple = '%s',tablet1code = '%s',dateTo='%s',piwenNum='%s',standard='%s',tabletDef1Value='%s',tabletDef2Value='%s',tabletDef3Value='%s',tabletSpec='%s' where tabletstorid=%s",sPlace.c_str(),sCode1.c_str(),sDateTo.c_str(),sOffdoc.c_str(),sStandard.c_str(),sDef1.c_str(),sDef2.c_str(),sDef3.c_str(),sPec.c_str(),inTaskId.c_str());
	if (-1 == (int)db.ExeSql(sqlBuff)){
		MessageBox(sqlBuff,"温馨提示：",MB_SYSTEMMODAL); 
		return;
	}
    
	OnOK();
}

BOOL CWriteSimplePlaceDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	((CButton *)GetDlgItem(IDC_BUTTON_SP_QUIT))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_MAIN_QUIT)));  
	((CButton *)GetDlgItem(IDOK))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_MOD_PASS_QD)));  
	char sqlBuff[512]={0};
	sprintf_s(sqlBuff,512,"select medPlaceSimple,tablet1code,dateTo,piwenNum,standard,tabletDef1Value,tabletDef2Value,tabletDef3Value,tabletspec from table_task where tabletstorid =%d",atoi(inTaskId.c_str()));
	iter = db.ExeSql(sqlBuff);
	if(iter.HasMore()){
		string spec = (string)iter.GetData(8);
		if("" != spec)
		{
			SetDlgItemText(IDC_EDIT_SPEC,spec.c_str());
			//m_EditSimplePlace.SetWindowText(spec.c_str());
		}
		string simplePlace = (string)iter.GetData(0);
		if("" != simplePlace)
		{
			m_EditSimplePlace.SetWindowText(simplePlace.c_str());
		}
		string code1 = (string)iter.GetData(1);
		if("" != code1)
		{
			m_Ecode1.SetWindowText(code1.c_str());
		}
		string dateTo = (string)iter.GetData(2);
		if("" != dateTo)
		{
			dateTo = dateTo.substr(0,4)+"/"+dateTo.substr(5,2)+"/"+dateTo.substr(8,2);
			if (dateTo != "0000/00/00")
				m_EdateTo.SetWindowText(dateTo.c_str());
		}
		string offdocNum = (string)iter.GetData(3);
		if("" != offdocNum)
		{
			m_EoffDocNum.SetWindowText(offdocNum.c_str());
		}
		string standard = (string)iter.GetData(4);
		if("" != standard)
		{
			m_Estandard.SetWindowText(standard.c_str());
		}
		string def1 = (string)iter.GetData(5);
		if("" != def1)
		{
			m_Edef1.SetWindowText(def1.c_str());
		}
		string def2 = (string)iter.GetData(6);
		if("" != def2)
		{
			m_Edef2.SetWindowText(def2.c_str());
		}
		string def3 = (string)iter.GetData(7);
		if("" != def3)
		{
			m_Edef3.SetWindowText(def3.c_str());
		}
	}
	iter = db.ExeSql("select timeType,weightType from table_login");
	int timeType = 1;
	int weightType = 2;
	if(iter.HasMore())
	{
	     timeType = (int)iter.GetData(0);
		 if (1 == timeType)
		 {
			((CButton *)GetDlgItem(IDC_RADIO_TIME1))->SetCheck(TRUE);
		 }
		 else if (2 == timeType)
		 {
			 ((CButton *)GetDlgItem(IDC_RADIO_TIME2))->SetCheck(TRUE);
		 }
		 else 
			 ((CButton *)GetDlgItem(IDC_RADIO_TIME3))->SetCheck(TRUE);


		 weightType = (int)iter.GetData(1);
		 if (0 == weightType)
		 {
			 ((CButton *)GetDlgItem(IDC_RADIO_WEIGHT_G))->SetCheck(TRUE);
		 }
		 else if(1 == weightType)
			 ((CButton *)GetDlgItem(IDC_RADIO_WEIGHT_KG))->SetCheck(TRUE);
	     else
		     ((CButton *)GetDlgItem(IDC_RADIO_WEIGHT_NO))->SetCheck(TRUE);
	}


	return TRUE;  // return TRUE unless you set the focus to a control
}

void CWriteSimplePlaceDlg::OnBnClickedButtonSpQuit()
{
	OnCancel();
}

void CWriteSimplePlaceDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect   rect;   
	GetClientRect(&rect);   
	CDC   dcMem;   
	dcMem.CreateCompatibleDC(&dc);   
	CBitmap   bmpBackground;   
	bmpBackground.LoadBitmap(IDB_BITMAP_BJ_SIMPLE_PLACE);
	BITMAP   bitmap;   
	bmpBackground.GetBitmap(&bitmap);   
	CBitmap   *pbmpOld=dcMem.SelectObject(&bmpBackground);   
	dc.StretchBlt(0,0,rect.Width(),rect.Height(),&dcMem,0,0,bitmap.bmWidth,bitmap.bmHeight,SRCCOPY); 
}

void CWriteSimplePlaceDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnLButtonDown(nFlags, point);
		PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x,point.y));
}

void CWriteSimplePlaceDlg::OnDtnDatetimechangeDatetimepickerSyDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	string showStr="";
	char tmpStr[32]={0};
	m_SyDate.GetWindowText(tmpStr,sizeof(tmpStr));	
	showStr+=tmpStr;
	//showStr+=" ";
	//memset(tmpStr,0,sizeof(tmpStr));
	//m_SyTime.GetWindowText(tmpStr,sizeof(tmpStr));
	//showStr+=tmpStr;

	SetDlgItemText(IDC_EDIT_SY_DATE_TO,showStr.c_str());
	*pResult = 0;
}
