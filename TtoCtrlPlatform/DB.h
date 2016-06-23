#ifndef _DB_H_
#define _DB_H_

#define IN
#define CMYSQL 1
#define DB_MAX_CONNECT_COUNT 1

#include <string>
#include <vector>
#include <assert.h>
using namespace std;

template<typename T>  
class xconst_iterator;

template<typename T>   
class FormateSqlStr
{
public:
	FormateSqlStr()
	{
		buff = NULL;
	}

	~FormateSqlStr()
	{
		if ( buff != NULL )
		{
			free(buff);
			buff = NULL;
		}
	}

	string operator() ( const string& str )
	{
		const char* p = str.c_str();
		if ( buff != NULL ) free(buff);
		buff = (char*)malloc(str.size()*2+1);
		char* q = buff;

		while ( *p != '\0' )
		{
			if ( *p == '\'' )
			{
				*q++ = '\'';
				*q++ = '\'';
			} 
			else
				*q++ = *p;
			p++;
		}
		*q = '\0';
		return buff;
	}
private:
	char* buff;
};

//=========================================
#if CMYSQL 

#undef CLASS_TYPE
#define CLASS_TYPE CMySql

#pragma warning(disable:4786)
#ifdef CUSE_MFC
	#include <WINSOCK2.H>
#else
	#include <windows.h>
#endif

#include <mysql.h>

class CMySql
{
public:
	typedef MYSQL_RES* RecordSet;

	RecordSet ExeSql( const char* sqlStr, int len = -1 );
	unsigned long escape_string( char*& to, const char* from, unsigned long from_length ); //二进制流转换
	CMySql( const char* host, const char* dbName, const char* user, const char* pass, unsigned long port = 3306, const char* unix_socket = NULL, unsigned long clientflag = 0 );	
	~CMySql();	
private:
	MYSQL* Connect;
	MYSQL_RES* Set;   

	const string Host, DBName, User, Pass, UnixSocket;
	unsigned long Port, ClientFlag;

	char* binaryBuff;

	const CMySql& operator = ( const CMySql& );
	CMySql( const CMySql& );
};

template<>  
class FormateSqlStr<CMySql>
{
public:
	FormateSqlStr()
	{
		buff = NULL;
	}

	~FormateSqlStr()
	{
		if ( buff != NULL )
		{
			free(buff);
			buff = NULL;
		}
	}

	string operator() ( const string& str )
	{
		const char* p = str.c_str();
		if ( buff != NULL ) free(buff);
		buff = (char*)malloc(str.size()*2+1);
		char* q = buff;

		while ( *p != '\0' )
		{
			if ( *p == '\'' )
			{
				*q++ = '\'';
				*q++ = '\'';
			} else if ( *p == '\\' )
			{
				*q++ = '\\';
				*q++ = '\\';
			}
			else
				*q++ = *p;
			p++;
		}
		*q = '\0';
		return buff;
	}
private:
	char* buff;
};

template<>
class xconst_iterator<CMySql>
{
public:
	typedef MYSQL_RES* Set;
	MYSQL_ROW  Row;

	xconst_iterator()
	{
		_set = NULL;
	}

	xconst_iterator( Set set )
	{
		if ( set == (Set)-1 ) 
			_set = NULL;
		else
			_set = set;		
	}

	xconst_iterator& operator = ( Set set )
	{
		if ( set == (Set)-1 )  
			_set = NULL;
		else
			_set = set;	
		return *this;
	}

	~xconst_iterator()
	{
		_set = NULL;
	}

	operator int ()
	{		
		return atoi(Row[Index]);	
	}

	operator string ()  
	{
		unsigned long* len = mysql_fetch_lengths(_set);
		string str;
		str.assign(Row[Index],len[Index]);
		return str;
	}

	operator const char* ()
	{
		return Row[Index];
	}

	operator double ()
	{
		double value = 0;
		sscanf( Row[Index], "%lf", &value );
		return value;
	}

	xconst_iterator& GetData( const int ind )
	{
		Index = ind;
		return *this;
	}

	bool HasMore()
	{
		if ( _set == NULL ) return false;
		return (Row = mysql_fetch_row(_set))==0?false:true;
	}

	string FormatSqlStr( IN const string& inStr )
	{
		return formateSqlStr(inStr);
	}
private:
	const xconst_iterator& operator = ( const xconst_iterator& );
	xconst_iterator( const xconst_iterator& );
	Set _set;
	int Index;

	FormateSqlStr<CMySql> formateSqlStr;
};

#endif


template<typename T>
class DB
{
public:
	typedef typename T::RecordSet RecordSet;
	typedef xconst_iterator<T> const_iterator;

	RecordSet ExeSql( const char* sqlStr, int len = -1 )
	{
		return dbp->ExeSql(sqlStr, len);
	}

	unsigned long EscapeString( char*& to, const char* from, unsigned long from_length )
	{
		return dbp->escape_string(to,from,from_length);
	}

	DB( T* t )
	{
		dbp = t;
	}

	~DB()
	{
		delete dbp;
		dbp = NULL;
	}
	DB() {}

private:	
	T* dbp;

	const DB& operator = ( const DB& );
	DB( const DB& );
	
};
//=========================================
#endif