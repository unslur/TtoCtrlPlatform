// TaskListHistoryDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "TtoCtrlPlatform.h"
#include "TaskListHistoryDialog.h"
#include <mysql.h>

// CTaskListHistoryDialog 对话框

IMPLEMENT_DYNAMIC(CTaskListHistoryDialog, CDialog)

CTaskListHistoryDialog::CTaskListHistoryDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CTaskListHistoryDialog::IDD, pParent),db(new CMySql(DB_IP,DB_NAME,DB_USER,DB_PASS))
{
	curPage = 0;
	totalNum = 0;
	taskStats[0] = "未下载";
	taskStats[1] = "正在下载";
	taskStats[2] = "已经下载";
	taskStats[3] = "下载完成";
	taskStats[4] = "下载失败";
	taskStats[100] = "非溯源";

	db.ExeSql("SET NAMES GBK");
}

CTaskListHistoryDialog::~CTaskListHistoryDialog()
{
}

void CTaskListHistoryDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_TASK_HISTORY, ListTaskHistory);
	DDX_Control(pDX, IDC_COMBO_CUR_PER_TOTAL, cbCurPerTotal);
}

BEGIN_MESSAGE_MAP(CTaskListHistoryDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SHOUYE, &CTaskListHistoryDialog::OnBnClickedButtonShouye)
	ON_BN_CLICKED(IDC_BUTTON_SHANGYE, &CTaskListHistoryDialog::OnBnClickedButtonShangye)
	ON_BN_CLICKED(IDC_BUTTON_XIAYE, &CTaskListHistoryDialog::OnBnClickedButtonXiaye)
	ON_BN_CLICKED(IDC_BUTTON_MOYE, &CTaskListHistoryDialog::OnBnClickedButtonMoye)
	ON_CBN_SELCHANGE(IDC_COMBO_CUR_PER_TOTAL, &CTaskListHistoryDialog::OnCbnSelchangeComboCurPerTotal)
	ON_BN_CLICKED(IDC_BUTTON_HISTORY_QUIT, &CTaskListHistoryDialog::OnBnClickedButtonHistoryQuit)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

//curPage 从0开始
BOOL CTaskListHistoryDialog::GetCurPageFromDB(int curPage)  
{   
	string taskFlag = "";
	char sqlBuff[0x400] = {0};
	sprintf_s(sqlBuff,0x400,"select cast(tabletstorid as char),taskFlag,tabletname,cast(tabletnumweight as char),tabletspec,medaddress,tabletnumber,createtime,cast(tabletboxnum as char),cast(tabletPrintedNum as char),medPlaceSimple,tablet1code,tabletitemnum from table_task_history order by tabletstorid desc  LIMIT %d,%d",curPage*MAX_NUM_PER_PAGE,MAX_NUM_PER_PAGE);
	try{
		iter = db.ExeSql(sqlBuff); 
		int iTask=0;
		while (iter.HasMore()) {  
			int nrow = ListTaskHistory.InsertItem(iTask++, (const char *)iter.GetData(0));//插入行
			ListTaskHistory.SetItemText(nrow, 1, taskStats[(int)iter.GetData(1)].c_str());//设置数据//状态
			ListTaskHistory.SetItemText(nrow, 2, (const char *)iter.GetData(2));//品名
			ListTaskHistory.SetItemText(nrow, 3, (const char *)iter.GetData(6));//批次
			string itemnum = (string)iter.GetData(12);
			if ("" == itemnum || "0" == itemnum)//小包
			{
				ListTaskHistory.SetItemText(nrow, 4, (const char *)iter.GetData(8));//总包数
			}
			else//中包
			{
				ListTaskHistory.SetItemText(nrow, 4, itemnum.c_str());//总包数
			}
			
			ListTaskHistory.SetItemText(nrow, 5, (const char *)iter.GetData(9));//打印包数
			ListTaskHistory.SetItemText(nrow, 6, (const char *)iter.GetData(3));//重量
			ListTaskHistory.SetItemText(nrow, 7, (const char *)iter.GetData(4));//规格
			ListTaskHistory.SetItemText(nrow, 8, (const char *)iter.GetData(5));//产地
			ListTaskHistory.SetItemText(nrow, 9, (const char *)iter.GetData(10));//产地简称			
			ListTaskHistory.SetItemText(nrow, 10, (const char *)iter.GetData(7));//日期
			ListTaskHistory.SetItemText(nrow, 11, (const char *)iter.GetData(11));//条形码	
		}
	}
	catch(...){
		return FALSE;
	}
	return TRUE;
}

BOOL CTaskListHistoryDialog::GetTotalNumFromDB(int& totalNum)  
{  
	try{
		iter = db.ExeSql("select count(*) from table_task_history"); 
		if (iter.HasMore()){
			string sTotalNum=(string)iter.GetData(0);
			if (sTotalNum.length() != 0)
			{
				totalNum = atoi(sTotalNum.c_str());
			}
		}
	}
	catch(...){
		return FALSE;
	}
	return TRUE;
}


BOOL CTaskListHistoryDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	((CButton *)GetDlgItem(IDC_BUTTON_HISTORY_QUIT))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_MAIN_QUIT)));  
	LONG lStyle;
	lStyle = GetWindowLong(ListTaskHistory.m_hWnd, GWL_STYLE);//获取当前窗口style
	lStyle &= ~LVS_TYPEMASK; //清除显示方式位
	lStyle |= LVS_REPORT; //设置style
	SetWindowLong(ListTaskHistory.m_hWnd, GWL_STYLE, lStyle);//设置style

	DWORD dwStyle = ListTaskHistory.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
	dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl） 
	ListTaskHistory.SetExtendedStyle(dwStyle); //设置扩展风格

	ListTaskHistory.SetBkColor(RGB(247,247,255)); 
	ListTaskHistory.SetTextColor(RGB(0,0,255)); 
	ListTaskHistory.SetTextBkColor(RGB(247,247,255)); 

	ListTaskHistory.InsertColumn( 0, "任务编号", LVCFMT_RIGHT,0 );
	ListTaskHistory.InsertColumn( 1, "任务状态", LVCFMT_LEFT, 60 );
	ListTaskHistory.InsertColumn( 2, "品名", LVCFMT_LEFT, 60);
	ListTaskHistory.InsertColumn( 3, "批次", LVCFMT_LEFT, 80 );
	ListTaskHistory.InsertColumn( 4, "总包数", LVCFMT_LEFT, 50 );
	ListTaskHistory.InsertColumn( 5, "已打包数", LVCFMT_LEFT, 60 );
	ListTaskHistory.InsertColumn( 6, "重量(g)", LVCFMT_LEFT, 60 );
	ListTaskHistory.InsertColumn( 7, "规格", LVCFMT_LEFT, 40 );
	ListTaskHistory.InsertColumn( 8, "产地详情", LVCFMT_LEFT, 150 );
	ListTaskHistory.InsertColumn( 9, "产地简称", LVCFMT_LEFT, 60 );	
	ListTaskHistory.InsertColumn( 10, "日期", LVCFMT_LEFT, 137 );
	ListTaskHistory.InsertColumn( 11, "商品码", LVCFMT_LEFT, 100 );

	int num = 0;
	GetTotalNumFromDB(num);
	totalNum = num;
    
	if (0 == num)
	{
		cbCurPerTotal.InsertString(0,"0/0");
	}
	else
	{
		int maxCur = num/MAX_NUM_PER_PAGE+1;
		if (num%MAX_NUM_PER_PAGE == 0)
		{
			maxCur--;
		}
		//插入combox
		for (int i=0;i<maxCur;i++)
		{
			string showParam = "";
			char tmpCurPage[16]={0};
			sprintf_s(tmpCurPage,16,"%d",i+1);
			char tmpTotalPage[16]={0};
			sprintf_s(tmpTotalPage,16,"%d",maxCur);
			showParam += tmpCurPage;
			showParam += "/";
			showParam += tmpTotalPage;

			cbCurPerTotal.InsertString(i,showParam.c_str());
		}
	}
	
	cbCurPerTotal.SetCurSel(0);
	curPage = 0;
	GetCurPageFromDB(curPage);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CTaskListHistoryDialog::PreTranslateMessage(MSG* pMsg)
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

void CTaskListHistoryDialog::OnBnClickedButtonShouye()
{
	curPage = 0;
	ListTaskHistory.DeleteAllItems();
	GetCurPageFromDB(curPage);
	cbCurPerTotal.SetCurSel(curPage);
}

void CTaskListHistoryDialog::OnBnClickedButtonShangye()
{
	curPage--;
	if (curPage < 0)
	{
		curPage=0;
	}
	ListTaskHistory.DeleteAllItems();
	GetCurPageFromDB(curPage);
	cbCurPerTotal.SetCurSel(curPage);
}

void CTaskListHistoryDialog::OnBnClickedButtonXiaye()
{	
	if (totalNum%MAX_NUM_PER_PAGE != 0)
	{
		if (curPage >= totalNum/MAX_NUM_PER_PAGE)
		{
			curPage=totalNum/MAX_NUM_PER_PAGE;
		}else curPage++;
	}
	else{
		if(0 == totalNum/MAX_NUM_PER_PAGE)
		{
			curPage = 0;
		}else curPage++;
	}
	
	ListTaskHistory.DeleteAllItems();
	GetCurPageFromDB(curPage);
	cbCurPerTotal.SetCurSel(curPage);
}

void CTaskListHistoryDialog::OnBnClickedButtonMoye()
{
	curPage = totalNum/MAX_NUM_PER_PAGE;
	if (0 != curPage)
	{
		if (totalNum%MAX_NUM_PER_PAGE == 0)
		{
			curPage--;
		}
	}

	ListTaskHistory.DeleteAllItems();
	GetCurPageFromDB(curPage);
	cbCurPerTotal.SetCurSel(curPage);
}

void CTaskListHistoryDialog::OnCbnSelchangeComboCurPerTotal()
{
	int j=0;
	j=cbCurPerTotal.GetCurSel();//获取索引值
	if (-1 == j)
	{
		MessageBox("请选择要查看的页！","查询历史任务记录",MB_SYSTEMMODAL);
		return;
	}
	curPage = j;
	ListTaskHistory.DeleteAllItems();
	GetCurPageFromDB(curPage);
}

void CTaskListHistoryDialog::OnBnClickedButtonHistoryQuit()
{
	OnCancel();
}

void CTaskListHistoryDialog::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect   rect;   
	GetClientRect(&rect);   
	CDC   dcMem;   
	dcMem.CreateCompatibleDC(&dc);   
	CBitmap   bmpBackground;   
	bmpBackground.LoadBitmap(IDB_BITMAP_BJ_HIS);
	BITMAP   bitmap;   
	bmpBackground.GetBitmap(&bitmap);   
	CBitmap   *pbmpOld=dcMem.SelectObject(&bmpBackground);   
	dc.StretchBlt(0,0,rect.Width(),rect.Height(),&dcMem,0,0,bitmap.bmWidth,bitmap.bmHeight,SRCCOPY); 

	((CButton *)GetDlgItem(IDC_BUTTON_SHOUYE))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_FIRST_PAGE))); 
	((CButton *)GetDlgItem(IDC_BUTTON_SHANGYE))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_PREV_PAGE))); 
	((CButton *)GetDlgItem(IDC_BUTTON_XIAYE))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_NEXT_PAGE))); 
	((CButton *)GetDlgItem(IDC_BUTTON_MOYE))->SetBitmap(LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP_LAST_PAGE))); 
}

void CTaskListHistoryDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonDown(nFlags, point);
		PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x,point.y));	
}
