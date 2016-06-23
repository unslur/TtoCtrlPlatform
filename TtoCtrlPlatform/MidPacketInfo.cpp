// MidPacketInfo.cpp : 实现文件
//

#include "stdafx.h"
#include "TtoCtrlPlatform.h"
#include "MidPacketInfo.h"
#include "DB.h"
#include "Function.h"
#include <winspool.h>
#include <list>
#include <vector>


// CMidPacketInfo 对话框
////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CMidPacketInfo, CDialog)

CMidPacketInfo::CMidPacketInfo(string task,string tname,string tbatch,string tweight,string tspec,string tplace,string tsimplace,string tdate,CWnd* pParent /*=NULL*/)
	: CDialog(CMidPacketInfo::IDD, pParent)
{
    inTaskId = task;
	midname = tname;
	midweight = tweight;
	midspec = tspec;
	midbatch = tbatch;
	middate = tdate;
	midplace = tplace;
	midsimplace = tsimplace;
	printerName = "";
}

CMidPacketInfo::~CMidPacketInfo()
{
	UnInitMyPrinter();
}

void CMidPacketInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMidPacketInfo, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_PRINT, &CMidPacketInfo::OnBnClickedButtonPrint)
	ON_BN_CLICKED(IDC_BUTTON_MID_CANCEL, &CMidPacketInfo::OnBnClickedButtonMidCancel)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CMidPacketInfo 消息处理程序
BOOL CMidPacketInfo::OnInitDialog()
{
	CDialog::OnInitDialog();

	CPrintDialog   printDlg(FALSE); 
	printDlg.GetDefaults();
	CString test = printDlg.GetDeviceName();
	printerName = test.GetBuffer(0);
	test.ReleaseBuffer();	
    InitMyPrinter(printerName.c_str());

	DB<CMySql> db(new CMySql(DB_IP,DB_NAME,DB_USER,DB_PASS));
	DB<CMySql>::const_iterator iter;
    
	char sqlBuff[256]={0};
	sprintf_s(sqlBuff,256,"select midcode from table_midcode where tabletstorid =%d",atoi(inTaskId.c_str()));
	iter = db.ExeSql(sqlBuff);
	int i=0;
	int nrow = -1;
	char tmpId[16]={0};
	string tmpMidcode="";
	while(iter.HasMore()){
		tmpMidcode += (string)iter.GetData(0);
		if("" != tmpMidcode)
		{
			tmpMidcode+="\r\n";
		}
	}
	SetDlgItemText(IDC_EDIT_MID_CODE,tmpMidcode.c_str());
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CMidPacketInfo::OnBnClickedButtonFile()
{
	/*string allMidCode = "";
	string tmpCode = "";
	int i=0;
	while (1)
	{
		tmpCode = mListMidPack.GetItemText(i++,1);
		if ("" == tmpCode)
		{
			break;
		}
		allMidCode += tmpCode;
		allMidCode += "\r\n";
	}

	TCHAR pszPath[MAX_PATH];  
	BROWSEINFO bi;   
	bi.hwndOwner      = this->GetSafeHwnd();  
	bi.pidlRoot       = NULL;  
	bi.pszDisplayName = NULL;   
	bi.lpszTitle      = TEXT("请选择文件夹");   
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;  
	bi.lpfn           = NULL;   
	bi.lParam         = 0;  
	bi.iImage         = 0;   

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);  
	if (pidl == NULL)  
	{  
		return;  
	}  
    SHGetPathFromIDList(pidl, pszPath);

	string tmpfilename="";
	tmpfilename = MakeFileName();
	string filename = filename + pszPath;
	if ("" == filename)
	{
		MessageBox("未正确选择路径！","提示：",MB_SYSTEMMODAL);
		return;
	}
	if (filename[filename.length()-1] == '\\')
	{
		filename+=tmpfilename;
	}
	else
	filename = filename + "\\"+ tmpfilename;
	eRet ret = E_RET_FAILED;
	ret = WriteBinaryFile(filename.c_str(),(char *)allMidCode.c_str(),allMidCode.length());
	if (ret != E_RET_FAILED)
	{
		MessageBox("导入文件成功！","提示：",MB_SYSTEMMODAL);
	}*/
}

void CMidPacketInfo::OnBnClickedButtonPrint()
{
	if (MessageBox("是否确定打印当前任务？","打印提示：",MB_YESNO) == IDNO)	return;
	string allStr="";
	string midcode="";
	int i=0;
	if ("" == midsimplace)
	{
		MessageBox("请输入产地简称！","提示：",MB_SYSTEMMODAL);
		return;
	}

	CString ctmpCode;
	GetDlgItemText(IDC_EDIT_MID_CODE,ctmpCode);
	string sMidCode = ctmpCode;

	list<string> printv = splitPrintStrRN(sMidCode);
	int y = 0,z = 0,ret = 0;
    int num = printv.size();
	int index = 0;
	char showBuf[64];
	for (list<string>::const_iterator iter = printv.begin();iter != printv.end(); ++iter )
	{
		allStr = allStr+"*CPC*ZYCZS"+(char *)iter->c_str()+"\n";
		allStr = allStr + (char *)iter->c_str()+"\n";
		allStr += midname+""+midweight+"g\n";
		allStr += midsimplace+"\n";
		allStr += middate+"\n";
		allStr += "批号："+midbatch+"\n";
		allStr += "厂家："+midplace+"\n|";

        //3gechuanlian
        if (index%3 == 2 || num == index+1)
        {
			ClearPrinter();
			printContent(allStr/*,printerName*/,4);
			allStr="";
            memset(showBuf,0,sizeof(showBuf));
			if (num != index+1)
			{
				sprintf_s(showBuf,64,"打印中，当前打印第：%d个",index+1);
			}
			else
			{
				sprintf_s(showBuf,64,"打印完成，共打印：%d个",index+1);
			}
			SetDlgItemText(IDC_STATIC_STATUS,showBuf);
        }
		index++;
	}
}

void CMidPacketInfo::OnBnClickedButtonMidCancel()
{
	OnCancel();
}

void CMidPacketInfo::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect   rect;   
	GetClientRect(&rect);   
	CDC   dcMem;   
	dcMem.CreateCompatibleDC(&dc);   
	CBitmap   bmpBackground;   
	bmpBackground.LoadBitmap(IDB_BITMAP_ZB_INFO);
	BITMAP   bitmap;   
	bmpBackground.GetBitmap(&bitmap);   
	CBitmap   *pbmpOld=dcMem.SelectObject(&bmpBackground);   
	dc.StretchBlt(0,0,rect.Width(),rect.Height(),&dcMem,0,0,bitmap.bmWidth,bitmap.bmHeight,SRCCOPY); 
	((CButton *)GetDlgItem(IDC_BUTTON_PRINT))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_ZB_PRINT)));  
	((CButton *)GetDlgItem(IDC_BUTTON_MID_CANCEL))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_MAIN_QUIT)));
}

void CMidPacketInfo::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonDown(nFlags, point);
	PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x,point.y));
}
