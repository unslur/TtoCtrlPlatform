// TtoCtrlPlatform.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CTtoCtrlPlatformApp:
// �йش����ʵ�֣������ TtoCtrlPlatform.cpp
//

class CTtoCtrlPlatformApp : public CWinApp
{
public:
	CTtoCtrlPlatformApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CTtoCtrlPlatformApp theApp;