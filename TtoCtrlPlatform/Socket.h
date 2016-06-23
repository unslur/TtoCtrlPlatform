/*
* Copyright (c) 2005,�ɶ�Ӣ����Ӧ�ó��򿪷���
* All rights reserved.
* 
* �ļ����ƣ�Socket.h
* �ļ���ʶ���������ĵ�
* ժ    Ҫ��socket��װ
* 
* ��ǰ�汾��1.1
* ��    �ߣ�������
* ������ڣ�2005��11��03��
*
* ȡ���汾��1.0
* ԭ�� �� ��������
* ������ڣ�2005��09��05��
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
//===========��ʼ�趨ֵ==================================
#define Recv_MAXLEN 0x800   //һ�ζ������󳤶�		|
#define TIME_SECOND 0		//�ȴ�ʱ����				|
//#define TIME_MINSECOND 50000	//΢��1/1000000��
#define TIME_MINSECOND 5000	//΢��1/1000000��   //2015-6-18
//#define TIME_MINSECOND 100000	//΢��1/1000000��|
//=======================================================
/***********************************
------------------------------------
            Socket ��װ
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
