// Code1Dialog.cpp : 实现文件
//商品码

#include "stdafx.h"
#include "TtoCtrlPlatform.h"
#include "Code1Dialog.h"

// CCode1Dialog 对话框

IMPLEMENT_DYNAMIC(CCode1Dialog, CDialog)

CCode1Dialog::CCode1Dialog(string task,CWnd* pParent /*=NULL*/)
	: CDialog(CCode1Dialog::IDD, pParent),db(new CMySql(DB_IP,DB_NAME,DB_USER,DB_PASS))
{
	inTaskId = task;
}

CCode1Dialog::~CCode1Dialog()
{
}

void CCode1Dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_1CODE, m_EditCode1);
}


BEGIN_MESSAGE_MAP(CCode1Dialog, CDialog)
	ON_BN_CLICKED(ID_1CODE_OK, &CCode1Dialog::OnBnClicked1codeOk)
	ON_BN_CLICKED(IDC_BTN_1CODE_QUIT, &CCode1Dialog::OnBnClickedBtn1codeQuit)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CCode1Dialog 消息处理程序

void CCode1Dialog::OnBnClicked1codeOk()
{
	CString tmpCode1;
	CEdit* pCode1 = (CEdit*)GetDlgItem(IDC_EDIT_1CODE);
	pCode1->GetWindowText(tmpCode1);
	string sCode1 = tmpCode1.GetBuffer(0);
	tmpCode1.ReleaseBuffer();
	if (sCode1.length() == 0){
		MessageBox("请输入商品码!","温馨提示：",MB_SYSTEMMODAL);
		return;
	}
	if (12 != sCode1.length()){//
		MessageBox("商品码长度需12位!","温馨提示：",MB_SYSTEMMODAL);
		return;
	}
	
	char sqlBuff[0x200]={0};
	sprintf(sqlBuff,"update table_task SET tablet1code = '%s' where tabletstorid=%s",sCode1.c_str(),inTaskId.c_str());
	if (-1 == (int)db.ExeSql(sqlBuff)){
		MessageBox("编辑商品码失败！","温馨提示：",MB_SYSTEMMODAL); 
	}
	else MessageBox("商品码编辑成功！","温馨提示：",MB_SYSTEMMODAL); 
	OnCancel();
}

BOOL CCode1Dialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	char sqlBuff[0x200]={0};
	sprintf(sqlBuff,"select tablet1code from table_task where tabletstorid =%d",atoi(inTaskId.c_str()));
	iter = db.ExeSql(sqlBuff);
	if(iter.HasMore()){
		string code1 = (string)iter.GetData(0);
		if(0 != code1.length())
		{
			m_EditCode1.SetWindowText(code1.c_str());
		}
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CCode1Dialog::OnBnClickedBtn1codeQuit()
{
	OnCancel();
}

void CCode1Dialog::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect   rect;   
	GetClientRect(&rect);   
	CDC   dcMem;   
	dcMem.CreateCompatibleDC(&dc);   
	CBitmap   bmpBackground;   
	bmpBackground.LoadBitmap(IDB_BITMAP_1CODE_BJ);
	BITMAP   bitmap;   
	bmpBackground.GetBitmap(&bitmap);   
	CBitmap   *pbmpOld=dcMem.SelectObject(&bmpBackground);   
	dc.StretchBlt(0,0,rect.Width(),rect.Height(),&dcMem,0,0,bitmap.bmWidth,bitmap.bmHeight,SRCCOPY); 

	((CButton *)GetDlgItem(ID_1CODE_OK))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_1CODE_OK)));  
	((CButton *)GetDlgItem(IDC_BTN_1CODE_QUIT))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_MAIN_QUIT))); 
}

void CCode1Dialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonDown(nFlags, point);
	PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x,point.y));
}
