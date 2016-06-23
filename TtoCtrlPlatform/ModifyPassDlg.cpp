// ModifyPassDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TtoCtrlPlatform.h"
#include "ModifyPassDlg.h"
#include "DB.h"


IMPLEMENT_DYNAMIC(CModifyPassDlg, CDialog)

CModifyPassDlg::CModifyPassDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModifyPassDlg::IDD, pParent){
}

CModifyPassDlg::~CModifyPassDlg()
{
}

void CModifyPassDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CModifyPassDlg, CDialog)
	ON_BN_CLICKED(ID_BUTTON_MODIFY_OK, &CModifyPassDlg::OnBnClickedButtonModifyOk)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// CModifyPassDlg 消息处理程序
void CModifyPassDlg::OnBnClickedButtonModifyOk()
{
	CString tmpNewPass,tmpNewPassAgain,tmpPass;
	GetDlgItemText(IDC_EDIT_NEW_PASS,tmpNewPass);
	GetDlgItemText(IDC_EDIT_NEW_PASS_AGAIN,tmpNewPassAgain);
	GetDlgItemText(IDC_EDIT_OLD_PASS,tmpPass);
	string stmpNewPass((const char*)tmpNewPass),stmpNewPassAgain((const char*)tmpNewPassAgain),stmpPass((const char*)tmpPass);

	if ( stmpNewPass.length() < 5 || stmpNewPass.length() > 20 ){
		MessageBox("密码长度5~20","温馨提示：",MB_SYSTEMMODAL);
		return;
	}
	if ( stmpNewPassAgain.length() < 5 || stmpNewPassAgain.length() > 20 ){
		MessageBox("密码长度5~20","温馨提示：",MB_SYSTEMMODAL);
		return;
	}
	if ( stmpPass.length() < 5 || stmpPass.length() > 20 ){
		MessageBox("密码长度5~20","温馨提示：",MB_SYSTEMMODAL);
		return;
	}
	if( stmpNewPass != stmpNewPassAgain ){
		MessageBox("两次输入密码不一致","温馨提示：",MB_SYSTEMMODAL);
		return;
	}

	DB<CMySql> db(new CMySql(DB_IP,DB_NAME,DB_USER,DB_PASS));
	DB<CMySql>::const_iterator iter;
	iter = db.ExeSql("SELECT DECODE(loginPass,'key') FROM table_login");
	if(iter.HasMore()){
		string dbPass=iter.GetData(0);
		if( dbPass != stmpPass ){
			MessageBox("旧密码输入错误！","修改密码提示：",0);
			return;
		}
	}else{return;}

	char sqlBuff[0x200] = {0};
	sprintf_s(sqlBuff,0x200,"UPDATE table_login SET loginPass = ENCODE('%s','key')",iter.FormatSqlStr(stmpNewPass).c_str());
	if( -1 == (int)db.ExeSql(sqlBuff) ){
		MessageBox("修改密码失败！","修改密码提示：",MB_SYSTEMMODAL);
		return;
	}
	MessageBox("密码修改成功！","修改密码提示：",MB_SYSTEMMODAL);
    OnCancel();
}



void CModifyPassDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect   rect;   
	GetClientRect(&rect);   
	CDC   dcMem;   
	dcMem.CreateCompatibleDC(&dc);   
	CBitmap   bmpBackground;   
	bmpBackground.LoadBitmap(IDB_BITMAP_BJ_PASS);
	BITMAP   bitmap;   
	bmpBackground.GetBitmap(&bitmap);   
	CBitmap   *pbmpOld=dcMem.SelectObject(&bmpBackground);   
	dc.StretchBlt(0,0,rect.Width(),rect.Height(),&dcMem,0,0,bitmap.bmWidth,bitmap.bmHeight,SRCCOPY); 

	((CButton *)GetDlgItem(ID_BUTTON_MODIFY_OK))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_M_QUEDING)));  
	((CButton *)GetDlgItem(IDCANCEL))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_M_QUEXIAO)));  
}

void CModifyPassDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CDialog::OnLButtonDown(nFlags, point);
	PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x,point.y));
}
