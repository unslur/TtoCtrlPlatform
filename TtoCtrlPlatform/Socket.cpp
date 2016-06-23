

//#if 0
#include "stdafx.h"
//#endif

#include "Socket.h"
#pragma comment(lib,"ws2_32.lib") 
int Socket::Count = 0;

Socket::Socket()
{
	if ( 0 == Count++ )	Init();
}

Socket::~Socket()
{
	if ( 0 == --Count )	CleanUp();
}
//========================================================

/*
* 函数介绍：初始化socket的com
* 输入参数：无
* 输出参数：无
* 返回值  ：成功为true
*/
bool Socket::Init()
{
	WSADATA wsaData;
	return WSAStartup(MAKEWORD(2, 2), &wsaData) != 0 ? false : true;
}

bool Socket::CleanUp()
{
	return WSACleanup() == SOCKET_ERROR ? false : true;
}

bool Socket::CInit(const string& ip, int port)
{
	CIP = ip;
	CPort = port;
	return true;
}

bool Socket::SInit(const string& ip, int port)
{
	SIP = ip;
	SPort = port;
	return true;
}

bool Socket::CCreateSocket()
{
	return (csock=socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET ? false : true;
}

bool Socket::CConnect()
{
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(u_short(CPort));
	addr.sin_addr.s_addr = inet_addr(CIP.c_str());
	if ( connect(csock, (SOCKADDR*)(&addr), sizeof(SOCKADDR)) == SOCKET_ERROR )
	{
		closesocket(csock);
		return false;
	}
	return true;
}

bool Socket::Send( const string& str ) const throw()
{
	try
	{
		unsigned int n = 0, tn = 0;
		do 
		{
			tn = send( csock, str.substr(n,str.length()-n).c_str(), str.length()-n, 0 );
			if ( SOCKET_ERROR == tn || 0 == tn ) return false;
			n += tn;
		} while( n < str.length() );
		return true;
	}
	catch ( ... )
	{
		return false;
	}
}

bool Socket::Send( const char* str, int len ) const throw()
{
	try
	{
		int n = 0, tn = 0;
		do 
		{
			tn = send( csock, (str+n), len-n, 0 );
			if ( SOCKET_ERROR == tn || 0 == tn ) return false;
			n += tn;
		} while( n < len );
		return true;
	}
	catch ( ... )
	{
		return false;
	}
}

bool Socket::Recv( char* str, char flag, int& relen ) const throw()
{
	try
	{	
		int n = 0, tn = 0;
		do 
		{
			tn = recv(csock, str+n, Recv_MAXLEN-n, 0);
			if ( tn == SOCKET_ERROR || tn == 0 ) return false;
			n += tn;		
		} while( str[n-1] != flag );
		relen = n;
		return true;
	}
	catch ( ... )
	{
		return false;
	}
}

bool Socket::Recv( char* str, char flag ) const throw()
{
	int n = 0;
	return	Recv(str,flag,n);
}

bool Socket::Recv( char* str, int len, int& relen  ) const throw()
{
	try
	{	
		int tn = 0;
		tn = recv(csock, str, len<Recv_MAXLEN?len:Recv_MAXLEN, 0);
		if ( tn == SOCKET_ERROR || tn == 0 ) return false;
		relen = tn;
		return true;
	}
	catch ( ... )
	{
		return false;
	}
}

bool Socket::Recv( OUT char* str, IN int len ) const throw()
{
	int n = 0;
	return	Recv(str,len,n);	
}

int Socket::Recv2( char* str, int len, int& relen  ) const throw()
{
	try
	{	
		int tn = 0;
		tn = recv(csock, str, len<Recv_MAXLEN?len:Recv_MAXLEN, 0);
		if ( tn == SOCKET_ERROR ) 
		{
			return -1;
		}
		else if (tn == 0)
		{
			return 0;
		}
		else 
		{
			relen = tn;
			return 1;
		}
	}
	catch ( ... )
	{
		return -1;
	}
}

int Socket::Recv2( OUT char* str, IN int len ) const throw()
{
	int n = 0;
	return	Recv2(str,len,n);	
}

int Socket::CanRecv()
{
	TIMEVAL tv01 = {TIME_SECOND, TIME_MINSECOND}; //秒，微秒
	fd_set fdr;
	FD_ZERO(&fdr);
	FD_SET(csock, &fdr);
	return select( 0, &fdr, NULL, NULL, &tv01 ); //检查可读状态 
}

bool Socket::CReleaseSocket()
{
	if ( shutdown(csock, 2) == SOCKET_ERROR ) return false;
	if ( closesocket(csock) == SOCKET_ERROR ) return false;
	return true;
}

bool Socket::CShutdownSocket()
{
	if ( shutdown(csock, 2) == SOCKET_ERROR ) return false;
	return true;
}

bool Socket::CCloseSocket()
{
	if ( closesocket(csock) == SOCKET_ERROR ) return false;
	return true;
}

bool Socket::SCreateSocket()
{
	return (ssock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET ? false : true;
}

bool Socket::SSetBind()
{
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(u_short(SPort));
	addr.sin_addr.s_addr = inet_addr(SIP.c_str());
	return bind(ssock,(SOCKADDR*)&addr,sizeof(addr)) == SOCKET_ERROR ? false : true;
}

bool Socket::SAccept( Socket& tsocket )
{
	SOCKADDR_IN addr;
	int k = sizeof(SOCKADDR_IN);
	return (tsocket.csock = accept(ssock,(SOCKADDR*)&addr, &k)) == SOCKET_ERROR ? false : true;
}


bool Socket::SSetListen( int num )
{
	return listen(ssock,num) == SOCKET_ERROR ? false : true;
}


bool Socket::SReleaseSocket()
{
	if ( shutdown(ssock, 2) == SOCKET_ERROR ) return false;
	if ( closesocket(ssock) == SOCKET_ERROR ) return false;
	return true;
}

bool Socket::SShutdownSocket()
{
	if ( shutdown(ssock, 2) == SOCKET_ERROR ) return false;
	return true;
}

bool Socket::SCloseSocket()
{
	if ( closesocket(ssock) == SOCKET_ERROR ) return false;
	return true;
}

int Socket::GetLastError()
{
	return WSAGetLastError();
}

bool Socket::CGetIPAddr( std::string& IPAddr, int& Port )
{
	sockaddr name;
	int nameLen = sizeof(name);
	if ( getpeername(csock,&name,&nameLen) == -1 )	return false;	
	if (name.sa_family == AF_INET)
	{
		IPAddr = inet_ntoa(((struct sockaddr_in *)&name)->sin_addr);
		Port = ((struct sockaddr_in *)&name)->sin_port;
	}		
	else
		return false;
	return true;
}