#include "stdafx.h"
#include "DB.h"

#if CMYSQL

CMySql::RecordSet CMySql::ExeSql( const char* sqlStr, int len )
{
	if ( Set != NULL )
	{
		mysql_free_result(Set);
		Set = NULL;
	}

	if ( mysql_real_query( Connect, sqlStr, len==-1?strlen(sqlStr):len ) )
	{
		printf("Error making mysql_real_query: %s\n", mysql_error(Connect));
		printf(sqlStr);
		printf("\r\n");
		return reinterpret_cast<CMySql::RecordSet>(-1);
	}

	return Set=mysql_use_result(Connect);
}

CMySql::CMySql( const char* host, const char* dbName, const char* user, const char* pass, unsigned long port, const char* unix_socket, unsigned long clientflag ) \
: Host(host), DBName(dbName), User(user), Pass(pass), UnixSocket(NULL==unix_socket?"":unix_socket)
{
	ClientFlag = clientflag;
	Port = port;
	Connect = NULL;
	Set = NULL;
	binaryBuff = NULL;
		
	if ( NULL == (Connect = mysql_init((MYSQL*)0)) )
	{
		MessageBox(NULL,"初始化数据库失败，请检查Mysql服务是否安装！","打码机信息提示：",MB_SYSTEMMODAL);
		throw exception("Database mysql_init false!");	
	}	
	if (!mysql_real_connect(Connect,Host.c_str(),User.c_str(),Pass.c_str(),DBName.c_str(),Port,UnixSocket.c_str(),ClientFlag))
	{
		char errorStr[0x100];
		sprintf(errorStr,"Create database connect false! %s", mysql_error(Connect));
		MessageBox(NULL,"连接数据库失败，请检查Mysql服务是否安装并开启！","打码机信息提示：",MB_SYSTEMMODAL);
		throw exception(errorStr);
	}
}

CMySql::~CMySql()
{	
	if ( Set != NULL )
	{
		mysql_free_result(Set);
		Set = NULL;
	}
	mysql_close(Connect);
	Connect = NULL;
	if ( binaryBuff != NULL )
	{
		free(binaryBuff);
		binaryBuff = NULL;
	}
}

unsigned long CMySql::escape_string( char*& to, const char* from, unsigned long from_length )
{
	if ( binaryBuff != NULL )
	{
		free(binaryBuff);
		binaryBuff = NULL;
	}

	if ( (binaryBuff = (char*)malloc(sizeof(char)*from_length*2+1)) == NULL )
	{
		printf("Error malloc false\n");
		return -1;
	}
	
	to = binaryBuff;
	return mysql_escape_string(to,from,from_length);
}

#endif
