/*
* Copyright (c) 2005,成都英瑞奇应用程序开发部
* All rights reserved.
* 
* 文件名称：Socket.h
* 文件标识：见开发文档
* 摘    要：socket封装
* 
* 当前版本：1.1
* 作    者：李秀旭
* 完成日期：2005年11月03日
*
* 取代版本：1.0
* 原作 者 ：李秀旭
* 完成日期：2005年09月05日
*/

#ifndef _SOCKET_H_ 
#define _SOCKET_H_

#include <winsock2.h>
#include <string>

using namespace std;

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif
//===========初始设定值==================================
#define Recv_MAXLEN 0x800   //一次读入的最大长度		|
#define TIME_SECOND 0		//等待时的秒				|
//#define TIME_MINSECOND 50000	//微秒1/1000000秒
#define TIME_MINSECOND 5000	//微秒1/1000000秒   //2015-6-18
//#define TIME_MINSECOND 100000	//微秒1/1000000秒|
//=======================================================
/***********************************
------------------------------------
            Socket 封装
------------------------------------
***********************************/
class Socket
{
public:
	Socket();
	~Socket();
//==========init socket=========================
private:
	static bool Init();
	static bool CleanUp();
//==========Client==============================
public:
	bool CInit( IN const string& ip, IN int port );
	bool CCreateSocket();
	bool CConnect();
	bool CShutdownSocket();
	bool CCloseSocket();
	bool CReleaseSocket();
	bool CGetIPAddr( std::string& IPAddr, int& port );
//=========Server===============================
	bool SInit( IN const string& ip, IN int port );	
	bool SCreateSocket();
	bool SSetBind();
	bool SSetListen( int num );
	bool SAccept( OUT Socket& tsocket );
	bool SShutdownSocket();
	bool SCloseSocket();
	bool SReleaseSocket();
//==============================================
	bool Send( IN const string& str ) const throw();
	bool Send( IN const char* str, IN int len ) const throw();
	bool Recv( OUT char* str, IN char flag ) const throw();
	bool Recv( OUT char* str, IN char flag, OUT int& relen ) const throw();
	bool Recv( OUT char* str, IN int len, OUT int& relen ) const throw();
	bool Recv( OUT char* str, IN int len ) const throw();
	int Recv2( char* str, int len, int& relen  ) const throw();
	int Recv2( OUT char* str, IN int len ) const throw();
	int CanRecv();
//==============================================
	int GetLastError();
private: //luotest
	SOCKET csock;
	string CIP;
	unsigned int CPort;

	SOCKET ssock;
	string SIP;
	unsigned int SPort;

	static int Count;

private:
	const Socket & operator = ( const Socket& );
	Socket ( const Socket& );
};

#endif
