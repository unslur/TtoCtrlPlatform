

#include "stdafx.h"
#include "TtoCtrlPlatform.h"
#include "DlgFeiSuYuan.h"


// CDlgFeiSuYuan 对话框

IMPLEMENT_DYNAMIC(CDlgFeiSuYuan, CDialog)

CDlgFeiSuYuan::CDlgFeiSuYuan(int editType,string taskId,CWnd* pParent)
	: CDialog(CDlgFeiSuYuan::IDD, pParent),db(new CMySql(DB_IP,DB_NAME,DB_USER,DB_PASS))
{
	fName="";
	fNum="";
	fWeight="";
	fSpec="";
	fPlace="";
	fBatch="";
	fDate="";
	f1Code="";
	fDefine1Value="";
	fDefine2Value="";
	fDefine3Value="";

	writeDbFlag=false;
	gEditType = editType;
	gTaskId = taskId;
	db.ExeSql("SET NAMES 'GBK'");
}

CDlgFeiSuYuan::~CDlgFeiSuYuan()
{
}

void CDlgFeiSuYuan::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DATETIMEPICKER_FSY, m_DtPicker);
	DDX_Control(pDX, IDC_DATETIMEPICKER_FSY_TIME, m_DtPickerTime);
	DDX_Control(pDX, IDC_DATETIMEPICKER_FSY_DATE_TO, mDateToCtrl);
}


BEGIN_MESSAGE_MAP(CDlgFeiSuYuan, CDialog)
	ON_BN_CLICKED(IDOK, &CDlgFeiSuYuan::OnBnClickedOk)
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_FSY, &CDlgFeiSuYuan::OnDtnDatetimechangeDatetimepickerFsy)
ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_FSY_TIME, &CDlgFeiSuYuan::OnDtnDatetimechangeDatetimepickerFsyTime)
ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_FSY_DATE_TO, &CDlgFeiSuYuan::OnDtnDatetimechangeDatetimepickerFsyDateTo)
END_MESSAGE_MAP()

void CDlgFeiSuYuan::OnBnClickedOk()
{
	try
	{
		writeDbFlag = false;
		int fsyNum=0;
		char sqlBuff[0x200] = {0};
		CString tmpContent;
		GetDlgItemText(IDC_EDIT_F_NAME,tmpContent);
		fName = tmpContent;
		GetDlgItemText(IDC_EDIT_F_NUM,tmpContent);
		fNum = tmpContent;
		if (fNum == "" || fNum == "0" || (0 == fNum.find('-',0)))
		{
			MessageBox("请输入要打印的饮片包数！","温馨提示：",MB_SYSTEMMODAL); 
			return;
		}

		GetDlgItemText(IDC_EDIT_F_WEIGHT,tmpContent);
		fWeight = tmpContent;
		if (fWeight == "" || fWeight == "0" || (0 == fWeight.find('-',0)))
		{
			MessageBox("请检查饮片重量的输入！","温馨提示：",MB_SYSTEMMODAL); 
			return;
		}
		if (strstr(fWeight.c_str(),"g") != NULL || strstr(fWeight.c_str(),"G") != NULL || strstr(fWeight.c_str(),"克") != NULL)
		{
			MessageBox("要打印的饮片重量不需要带单位！","温馨提示：",MB_SYSTEMMODAL); 
			return;
		}
		GetDlgItemText(IDC_EDIT_F_SPEC,tmpContent);
		fSpec = tmpContent;
		GetDlgItemText(IDC_EDIT_F_PLACE,tmpContent);
		fPlace = tmpContent;
		GetDlgItemText(IDC_EDIT_F_BATCH,tmpContent);
		fBatch = tmpContent;
		GetDlgItemText(IDC_EDIT_F_DATE,tmpContent);
		fDate = tmpContent;
		GetDlgItemText(IDC_EDIT_F_1CODE,tmpContent);
		f1Code = tmpContent;
		GetDlgItemText(IDC_EDIT_F_DATE_TO,tmpContent);
		fDateTo = tmpContent;
		GetDlgItemText(IDC_EDIT_F_OFFDOC,tmpContent);
		fOffdoc = tmpContent;
		GetDlgItemText(IDC_EDIT_F_STANDARD,tmpContent);
		fStandard = tmpContent;
		GetDlgItemText(IDC_EDIT_F_DEFINE1_VALUE,tmpContent);
		fDefine1Value = tmpContent;
		GetDlgItemText(IDC_EDIT_F_DEFINE2_VALUE,tmpContent);
		fDefine2Value = tmpContent;
		GetDlgItemText(IDC_EDIT_F_DEFINE3_VALUE,tmpContent);
		fDefine3Value = tmpContent;

		int timeType1 = ((CButton *)GetDlgItem(IDC_RADIO_FSY_TIME1))->GetCheck();
		int timeType2 = ((CButton *)GetDlgItem(IDC_RADIO_FSY_TIME2))->GetCheck();
		int timeType3 = ((CButton *)GetDlgItem(IDC_RADIO_FSY_TIME3))->GetCheck();
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
		memset(sqlBuff,0,sizeof(sqlBuff));
		sprintf_s(sqlBuff,0x200,"update table_login SET timeType = %d",timeType);
		iter = db.ExeSql(sqlBuff);

		char strNum[16]={0};
		//某一项为空  添加失败 考虑
		memset(sqlBuff,0,sizeof(sqlBuff));
		if (1 == gEditType)//修改
		{
			sprintf_s(strNum,16,"%s",gTaskId.c_str());
			sprintf_s(sqlBuff,0x200,"update table_task set taskFlag=%d,tabletname='%s',tabletboxnum=%s,tabletnumweight=%s,tabletspec='%s',medaddress='%s',medPlaceSimple='%s',tabletnumber='%s',createtime='%s',tablet1code='%s',tabletDef1Value='%s',tabletDef2Value='%s',tabletDef3Value='%s',dateTo='%s',piwenNum='%s',standard='%s'where tabletstorid=%s",\
				100,fName.c_str(),fNum.c_str(),fWeight.c_str(),fSpec.c_str(),fPlace.c_str(),fPlace.c_str(),fBatch.c_str(),fDate.c_str(),f1Code.c_str(),\
				fDefine1Value.c_str(),fDefine2Value.c_str(),fDefine3Value.c_str(),fDateTo.c_str(),fOffdoc.c_str(),fStandard.c_str(),gTaskId.c_str());
		}
		else//添加
		{
			iter = db.ExeSql("select fsyNum from table_login");
			if(iter.HasMore()){
				fsyNum = (int)iter.GetData(0);
			}
			fsyNum++;
			sprintf_s(strNum,16,"-%d",fsyNum);
			sprintf_s(sqlBuff,0x200,"insert into table_task(tabletstorid,taskFlag,tabletname,tabletboxnum,tabletnumweight,tabletspec,medaddress,medPlaceSimple,tabletnumber,createtime,tablet1code,tabletDef1Value,tabletDef2Value,tabletDef3Value,dateTo,piwenNum,standard) values(%s,%d,'%s',%s,%s,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",\
				strNum,100,fName.c_str(),fNum.c_str(),fWeight.c_str(),fSpec.c_str(),fPlace.c_str(),fPlace.c_str(),fBatch.c_str(),fDate.c_str(),f1Code.c_str(),\
				fDefine1Value.c_str(),fDefine2Value.c_str(),fDefine3Value.c_str(),fDateTo.c_str(),fOffdoc.c_str(),fStandard.c_str());
		}

		if (-1 == (int)db.ExeSql(sqlBuff)){
			if (1 == gEditType)
				MessageBox("修改非溯源打码任务失败，请检查各项输入是否正确！","温馨提示：",MB_SYSTEMMODAL);
			else
				MessageBox("添加非溯源打码任务失败，请检查各项输入是否正确！","温馨提示：",MB_SYSTEMMODAL); 
			return;
		}else{
			writeDbFlag = true;
			if (gEditType != 1)//不是修改  是添加才做如下事情
			{
				memset(sqlBuff,0,sizeof(sqlBuff));
				sprintf_s(sqlBuff,0x200,"update table_login set fsyNum=%d",fsyNum);
				iter = db.ExeSql(sqlBuff);
			}
			int iDefine3=0;
			if ("" != fDefine3Value) iDefine3 = 1;
			int iDefine2=0;
			if ("" != fDefine2Value) iDefine2 = 1;
			int iDefine1=0;
			if ("" != fDefine1Value) iDefine1 = 1;
			int iStandard=0;
			if ("" != fStandard) iStandard = 1;
			int iOffDoc=0;
			if ("" != fOffdoc) iOffDoc = 1;
			int iDateTo=0;
			if ("" != fDateTo) iDateTo = 1;

			int i1Code=0;
			if ("" != f1Code) i1Code = 1;
			int iName=0;
			if ("" != fName) iName = 1;
			int iWeight=0;
			if ("" != fWeight) iWeight = 1;
			int iSpec=0;
			if ("" != fSpec) iSpec = 1;
			int iPlace=0;
			if ("" != fPlace) iPlace = 1;
			int iBatch=0;
			if ("" != fBatch) iBatch = 1;
			int iDate=0;
			if ("" != fDate) iDate = 1;

			int pf = 0;
			pf |= iDefine3<<14;
			pf |= iDefine2<<13;
			pf |= iDefine1<<12;
			pf |= iStandard<<10;
			pf |= iOffDoc<<9;
			pf |= iDateTo<<8;
			pf |= i1Code<<7;
			pf |= iName<<6;
			pf |= iWeight<<5;
			pf |= iSpec<<4;
			pf |= iPlace<<3;
			pf |= iBatch<<2;
			pf |= iDate<<1;
			char buff[0x200]={0};
			sprintf_s(buff,0x200,"update table_task set fsyPrintFlag=%d where tabletstorid=%s",pf,strNum);	
			if (-1 == (int)db.ExeSql(buff))
			{		
				MessageBox("非溯源任务对应打印信息配置失败！","温馨提示：",MB_SYSTEMMODAL);
			}			
		}
	}
	catch (...)
	{
	}

	OnOK();
}

BOOL CDlgFeiSuYuan::OnInitDialog()
{
	CDialog::OnInitDialog();
	if (1 == gEditType)
	{
		try
		{
			char sqlBuff[0x200] = {0};
			sprintf_s(sqlBuff,0x200,"select tabletname,CAST(tabletboxnum as char),CAST(tabletnumweight as char),tabletspec,medPlaceSimple,tabletnumber,createtime,tablet1code,dateTo,piwenNum,standard,tabletDef1Value,tabletDef2Value,tabletDef3Value from table_task where tabletstorid = %s",gTaskId.c_str());
			iter = db.ExeSql(sqlBuff);
			if (iter.HasMore())
			{
				//显示从数据库里面读取出来的信息
				SetDlgItemText(IDC_EDIT_F_NAME,iter.GetData(0));
				SetDlgItemText(IDC_EDIT_F_NUM,iter.GetData(1));
				SetDlgItemText(IDC_EDIT_F_WEIGHT,iter.GetData(2));
				SetDlgItemText(IDC_EDIT_F_SPEC,iter.GetData(3));
				SetDlgItemText(IDC_EDIT_F_PLACE,iter.GetData(4));
				SetDlgItemText(IDC_EDIT_F_BATCH,iter.GetData(5));
				string tmpdata = iter.GetData(6);
				tmpdata = tmpdata.substr(0,4)+"/"+tmpdata.substr(5,2)+"/"+tmpdata.substr(8,2);
				SetDlgItemText(IDC_EDIT_F_DATE,tmpdata.c_str());
				SetDlgItemText(IDC_EDIT_F_1CODE,iter.GetData(7));
				string tmpdateTo = iter.GetData(8);
				tmpdateTo = tmpdateTo.substr(0,4)+"/"+tmpdateTo.substr(5,2)+"/"+tmpdateTo.substr(8,2);
				if (tmpdateTo != "0000/00/00")
					SetDlgItemText(IDC_EDIT_F_DATE_TO,tmpdateTo.c_str());
				SetDlgItemText(IDC_EDIT_F_OFFDOC,iter.GetData(9));
				SetDlgItemText(IDC_EDIT_F_STANDARD,iter.GetData(10));
				SetDlgItemText(IDC_EDIT_F_DEFINE1_VALUE,iter.GetData(11));
				SetDlgItemText(IDC_EDIT_F_DEFINE2_VALUE,iter.GetData(12));
				SetDlgItemText(IDC_EDIT_F_DEFINE3_VALUE,iter.GetData(13));
			}
		}
		catch (...)
		{
		}
	}
	iter = db.ExeSql("select timeType from table_login");
	int timeType = 1;
	if(iter.HasMore())
	{
		timeType = (int)iter.GetData(0);
		if (1 == timeType)
		{
			((CButton *)GetDlgItem(IDC_RADIO_FSY_TIME1))->SetCheck(TRUE);
		}
		else if (2 == timeType)
		{
			((CButton *)GetDlgItem(IDC_RADIO_FSY_TIME2))->SetCheck(TRUE);
		}
		else 
            ((CButton *)GetDlgItem(IDC_RADIO_FSY_TIME3))->SetCheck(TRUE);	
	}
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CDlgFeiSuYuan::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonDown(nFlags, point);
	PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x,point.y));
}

void CDlgFeiSuYuan::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect   rect;   
	GetClientRect(&rect);   
	CDC   dcMem;   
	dcMem.CreateCompatibleDC(&dc);   
	CBitmap   bmpBackground;   
	bmpBackground.LoadBitmap(IDB_BITMAP_FSY_BJ);
	BITMAP   bitmap;   
	bmpBackground.GetBitmap(&bitmap);   
	CBitmap   *pbmpOld=dcMem.SelectObject(&bmpBackground);   
	dc.StretchBlt(0,0,rect.Width(),rect.Height(),&dcMem,0,0,bitmap.bmWidth,bitmap.bmHeight,SRCCOPY); 
	((CButton *)GetDlgItem(IDOK))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_FSY_TIANJIA)));  
	((CButton *)GetDlgItem(IDCANCEL))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_MAIN_QUIT)));

}

void CDlgFeiSuYuan::OnDtnDatetimechangeDatetimepickerFsy(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	string showStr="";
	char tmpStr[32]={0};
	m_DtPicker.GetWindowText(tmpStr,sizeof(tmpStr));	
	showStr+=tmpStr;


	SetDlgItemText(IDC_EDIT_F_DATE,showStr.c_str());
	*pResult = 0;
}

void CDlgFeiSuYuan::OnDtnDatetimechangeDatetimepickerFsyTime(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	string showStr="";
	char tmpStr[32]={0};
	m_DtPicker.GetWindowText(tmpStr,sizeof(tmpStr));	
	showStr+=tmpStr;


	SetDlgItemText(IDC_EDIT_F_DATE,showStr.c_str());
	*pResult = 0;
}

void CDlgFeiSuYuan::OnDtnDatetimechangeDatetimepickerFsyDateTo(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	string showStr="";
	char tmpStr[32]={0};
	mDateToCtrl.GetWindowText(tmpStr,sizeof(tmpStr));	
	showStr+=tmpStr;

	SetDlgItemText(IDC_EDIT_F_DATE_TO,showStr.c_str());
	*pResult = 0;
}
