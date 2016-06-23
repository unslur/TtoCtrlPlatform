#include "stdafx.h"
#include "Function.h"

fOpenPort OpenPort = NULL;
fPTK_ClearBuffer PTK_ClearBuffer = NULL;
fPTK_SetLabelHeight PTK_SetLabelHeight = NULL;
fPTK_SetLabelWidth PTK_SetLabelWidth = NULL;
fPTK_SetDarkness PTK_SetDarkness = NULL;
fPTK_SetPrintSpeed PTK_SetPrintSpeed = NULL;
fPTK_DrawBar2D_QR PTK_DrawBar2D_QR = NULL;
fPTK_DrawText PTK_DrawText = NULL;
fPTK_DrawTextTrueTypeW PTK_DrawTextTrueTypeW = NULL;
fClosePort ClosePort = NULL;
fPTK_PrintLabel PTK_PrintLabel = NULL;

HINSTANCE gt1;


void  UTF8ToGB2132(string  strSrc,string &outstr)  
{  
	WCHAR *wstrSrc = NULL;  
	char *szRes = NULL;  
	int i;  

	// UTF8转换成Unicode   
	i = MultiByteToWideChar(CP_UTF8, 0, strSrc.c_str(), -1, NULL, 0);  
	wstrSrc = new WCHAR[i+1];  
	MultiByteToWideChar(CP_UTF8, 0, strSrc.c_str(), -1, wstrSrc, i);  

	// Unicode转换成GB2312   
	i = WideCharToMultiByte(CP_ACP, 0, wstrSrc, -1, NULL, 0, NULL, NULL);  
	szRes = new char[i+1];  
	WideCharToMultiByte(CP_ACP, 0, wstrSrc, -1, szRes, i, NULL, NULL);  

	outstr = string (szRes);
	if (wstrSrc != NULL)  
	{  
		delete []wstrSrc;  
		wstrSrc = NULL;  
	}  
	if (szRes != NULL)  
	{  
		delete []szRes;  
		szRes = NULL;  
	}  

}

BOOL Mult2Wide(char *inbuf,size_t inLen,wchar_t* outBuf,size_t *outLen)
{
	*outLen = mbstowcs(outBuf, inbuf, inLen);

	return TRUE;
}

string UrlProcess(LPCSTR url)   
{ 
	if (NULL == url) return "";
	string outBuffer = "";
	HINTERNET hSession = InternetOpen(_T("httpDownload"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);   
	if(hSession != NULL){ 
		HINTERNET hHttp = InternetOpenUrlA(hSession, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);   
		if (hHttp != NULL){ 
			char Temp[1024];   
			ULONG Number = 1;
			while (Number > 0){   
				BOOL ret = InternetReadFile(hHttp, Temp, 1024 - 1, &Number); 
				if (!ret) break;
				Temp[Number] = '\0';   
				outBuffer += Temp;
			}   
			InternetCloseHandle(hHttp);   
			hHttp = NULL; 
		}
		InternetCloseHandle(hSession);   
		hSession = NULL;   
	} 
	return outBuffer;
}

// UTF8编码转换到GBK编码
int UTF8ToGBK(unsigned char * lpUTF8Str,unsigned char * lpGBKStr,int nGBKStrLen)
{
	wchar_t * lpUnicodeStr = NULL;
	int nRetLen = 0;
		
	if(!lpUTF8Str)  //如果UTF8字符串为NULL则出错退出
		return 0;

	nRetLen = ::MultiByteToWideChar(CP_UTF8,0,(char *)lpUTF8Str,-1,NULL,NULL);  //获取转换到Unicode编码后所需要的字符空间长度
	lpUnicodeStr = new WCHAR[nRetLen + 1];  //为Unicode字符串空间
	nRetLen = ::MultiByteToWideChar(CP_UTF8,0,(char *)lpUTF8Str,-1,lpUnicodeStr,nRetLen);  //转换到Unicode编码
	if(!nRetLen)  //转换失败则出错退出
		return 0;

	nRetLen = ::WideCharToMultiByte(CP_ACP,0,lpUnicodeStr,-1,NULL,NULL,NULL,NULL);  //获取转换到GBK编码后所需要的字符空间长度

	if(!lpGBKStr)  //输出缓冲区为空则返回转换后需要的空间大小
	{
		if(lpUnicodeStr)
			delete []lpUnicodeStr;
		return nRetLen;
	}

	if(nGBKStrLen < nRetLen)  //如果输出缓冲区长度不够则退出
	{
		if(lpUnicodeStr)
			delete []lpUnicodeStr;
		return 0;
	}

	nRetLen = ::WideCharToMultiByte(CP_ACP,0,lpUnicodeStr,-1,(char *)lpGBKStr,nRetLen,NULL,NULL);  //转换到GBK编码

	if(lpUnicodeStr)
		delete []lpUnicodeStr;

	return nRetLen;
}

eRet WriteBinaryFile(const char* fileName, char* buffer, uint32 bufferLen)
{
	eRet ret;
	FILE* fs;
	uint32 res;

	fs = fopen(fileName,"wb");
	if(fs == NULL)
	{
		return E_RET_FAILED;
	}

	ret = E_RET_SUCCESS;
	if (bufferLen > 0)//luoyuan add_919 
	{
		res = fwrite(buffer,bufferLen,1,fs);
		if(res != 1)
		{
			ret = E_RET_FAILED;
		}
	}

	fclose(fs);
	return ret;
}

eRet ReadBinaryFile(const char* fileName, char** outBuffer, int* outBufferLen)
{
	FILE* fs;
	int32 res;
	uint32 len;

	len = 0;
	*outBufferLen = 0;
	fs = fopen(fileName,"rb");
	if(fs == NULL)
	{
		return E_RET_FAILED;
	}
	res = fseek(fs,0,SEEK_END);
	if(res != 0)
	{
		fclose(fs);
		return E_RET_FAILED;
	}
	len = ftell(fs);
	if(len == -1)
	{
		fclose(fs);
		return E_RET_FAILED;
	}
	res = fseek(fs,0,SEEK_SET);
	if(res != 0)
	{
		fclose(fs);
		return E_RET_FAILED;
	}
	*outBuffer = (char *)malloc(len);
	if(*outBuffer == NULL)
	{
		fclose(fs);
		return E_RET_FAILED;
	}
	res = fread(*outBuffer,len,1,fs);
	if(res != 1)
	{
		free(*outBuffer);
		*outBuffer = NULL;
		fclose(fs);
		return E_RET_FAILED;
	}
	*outBufferLen = len;
	fclose(fs);
	return E_RET_SUCCESS;
}


string MakeFileName()
{
	SYSTEMTIME lpsystime;
	GetLocalTime(&lpsystime);

	char pStr[1024]={0};
	_snprintf_s(pStr,sizeof(pStr), "%u-%u-%u_%u-%u-%u-%u.txt",lpsystime.wYear,lpsystime.wMonth,lpsystime.wDay,lpsystime.wHour,lpsystime.wMinute,lpsystime.wSecond,lpsystime.wMilliseconds); 
	string filename = pStr;
	return filename;
}

void splitStrThree( string printStr, string &Str1, string &Str2, string&Str3 )
{
	string strTemp;
	int b = printStr.find("|");
	if( b != -1 ){
		Str1 = printStr.substr(0,b);
		strTemp = printStr.substr(b+1,printStr.length());
		b = 0;
		b = strTemp.find("|");

		if (b != -1) {
			Str2 = strTemp.substr(0, b);
			strTemp = strTemp.substr(b+1,strTemp.length());
			b = 0;
			b = strTemp.find("|");

			if (b != -1) {
				Str3 = strTemp.substr(0,b);
			}
		}
		else {
			if(printStr.length()!=0){
				Str2 = strTemp;
			}
		}
	}
	else{
		if(printStr.length()!=0){
			Str1 = printStr;
		}
	}

	return;
}

list<string> splitPrintStrN( string printStr ){
	list<string> vstr;
	while (true){
		int b = printStr.find("\n");
		if( b != -1 ){
			vstr.push_back(printStr.substr(0,b));
			printStr = printStr.substr(b+1,printStr.length());
		}
		else{
			if(printStr.length()!=0){
				vstr.push_back(printStr);
			}
			break;
		}
	}
	return vstr;
}

list<string> splitPrintStrRN( string printStr ){
	list<string> vstr;
	while (true){
		int b = printStr.find("\r\n");
		if( b != -1 ){
			vstr.push_back(printStr.substr(0,b));
			printStr = printStr.substr(b+2,printStr.length());
		}
		else{
			if(printStr.length()!=0){
				vstr.push_back(printStr);
			}
			break;
		}
	}
	return vstr;
}

list<string> splitPrintv( string& tstr, int DYJC ){
	list<string> vstr;
	while(true){
		if(tstr.length()<=DYJC){
			vstr.push_back(tstr);
			break;
		}
		bool f = false;
		for( int i = 0; i < DYJC;){
			char m = tstr.at(i);
			if( unsigned(m) < 0x80 ){
				f = true;
				i++;
			}
			else{
				i += 2;
				if(i%2==0){
					f = true;
				}else{
					f = false;
				}
			}
		}
		if(f){
			vstr.push_back(tstr.substr(0,DYJC));
			tstr = tstr.substr(DYJC,tstr.length());
		}
		else{
			vstr.push_back(tstr.substr(0,DYJC-1));
			tstr = tstr.substr(DYJC-1,tstr.length());
		}
	}
	return vstr;
}

bool printNew3_(const string& printStr,int X, int Y,int& id_name,fPTK_DrawBar2D_QR PTK_DrawBar2D_QR,fPTK_DrawTextTrueTypeW PTK_DrawTextTrueTypeW){
	char buff[256];
	list<string> printv = splitPrintStrN(printStr);
	int y = 0,z = 0,ret = 0;

	for ( list<string>::const_iterator iter = printv.begin();iter != printv.end(); ++iter ){
		if( 0 == z ){ //打二维码
			//string test1 = iter->c_str();
			ret = PTK_DrawBar2D_QR(X+104, Y, 0, 0, 0, 3, 1, 1, 3, (char*)iter->c_str());
			//string test = iter->c_str();
			if ( ret != 0 ){
				//sprintf(buff,"PTK_DrawBar2D_QR:%d",ret);
				//throw(buff);
			}
			y = Y + 95;
		}else if( 1 == z ){//打数字码
			list<string> printCodes = splitPrintv(string(*iter),12);
			for ( list<string>::const_iterator citer = printCodes.begin(); citer != printCodes.end(); ++citer ){
				//string test1 = citer->c_str();
				ret = PTK_DrawTextTrueTypeW(X+104, y, 20,0,"微软雅黑", 1, 600, 0, 0, 0, itoa(id_name++,buff,10), (char*)citer->c_str()); //x-100
				//string test = citer->c_str();
				if ( ret != 0 ){
					//sprintf(buff,"PTK_DrawTextTrueTypeW:%d",ret);
					//throw(buff);
				}
				y += 15;
			}
			y = Y + 95;
		}
		else{//其它文字
			//string test1 = iter->c_str();
			ret = PTK_DrawTextTrueTypeW(X, y, 20,0,"微软雅黑", 1, 600, 0, 0, 0, itoa(id_name++,buff,10), (char*)iter->c_str()); //x-100
			//string test = iter->c_str();
			if ( ret != 0 ){
				//sprintf(buff,"PTK_DrawTextTrueTypeW:%d",ret);
				//throw(buff);
			}
			y += 15;
		}
		/**/
		z++;

	}
	return true;
}

char* Gb2312ToUtf8(const char *pcGb2312) 
{
	int nUnicodeLen = MultiByteToWideChar(CP_ACP, 0, pcGb2312, -1, NULL, 0);

	wchar_t * pcUnicode = new wchar_t[nUnicodeLen+1]; 
	memset(pcUnicode, 0, nUnicodeLen * 2 + 2); 

	MultiByteToWideChar(CP_ACP, 0, pcGb2312, -1, pcUnicode, nUnicodeLen);

	int nUtf8Len = WideCharToMultiByte(CP_UTF8, 0, pcUnicode, -1, NULL, 0, NULL, NULL);

	char *pcUtf8=new char[nUtf8Len + 1]; 
	memset(pcUtf8, 0, nUtf8Len + 1);   

	WideCharToMultiByte(CP_UTF8, 0, pcUnicode, -1, pcUtf8, nUtf8Len, NULL, NULL);

	delete[] pcUnicode; 
	return pcUtf8;
}

bool InitMyPrinter(const char* printerNamet)
{
	char buff[128];
	//判断
	try{
		gt1 = LoadLibrary("WINPSK.dll");
		if (NULL == gt1)
		{
			throw("LoadLibrary WINPSK.dll failed!");
		}
		OpenPort = (fOpenPort)GetProcAddress(gt1,"OpenPort");
		ClosePort = (fClosePort)GetProcAddress(gt1,"ClosePort");
		PTK_DrawBar2D_QR = (fPTK_DrawBar2D_QR)GetProcAddress(gt1,"PTK_DrawBar2D_QR");
		PTK_SetLabelHeight = (fPTK_SetLabelHeight)GetProcAddress(gt1,"PTK_SetLabelHeight");
		PTK_PrintLabel = (fPTK_PrintLabel)GetProcAddress(gt1,"PTK_PrintLabel");
		PTK_SetDarkness = (fPTK_SetDarkness)GetProcAddress(gt1,"PTK_SetDarkness");
		PTK_SetPrintSpeed = (fPTK_SetPrintSpeed)GetProcAddress(gt1,"PTK_SetPrintSpeed");
		PTK_SetLabelWidth = (fPTK_SetLabelWidth)GetProcAddress(gt1,"PTK_SetLabelWidth");
		PTK_ClearBuffer = (fPTK_ClearBuffer)GetProcAddress(gt1,"PTK_ClearBuffer");
		PTK_DrawText = (fPTK_DrawText)GetProcAddress(gt1,"PTK_DrawText");
		PTK_DrawTextTrueTypeW = (fPTK_DrawTextTrueTypeW)GetProcAddress(gt1,"PTK_DrawTextTrueTypeW");
	
		int ret = OpenPort((char *)printerNamet);		
		if ( ret != 0 ){
			memset(buff,0,sizeof(buff));
			sprintf_s(buff,128,"OpenPort:%d",ret);
			throw(buff);
		}

		ret = PTK_ClearBuffer();
		if ( ret != 0 ){
			memset(buff,0,sizeof(buff));
			sprintf_s(buff,128,"PTK_ClearBuffer:%d",ret);
			throw(buff);
		}
		ret = PTK_SetLabelHeight(200, 24);
		if ( ret != 0 ){
			memset(buff,0,sizeof(buff));
			sprintf_s(buff,128,"PTK_SetLabelHeight:%d",ret);
			throw(buff);
		}
		ret = PTK_SetLabelWidth(800);
		if ( ret != 0 ){
			memset(buff,0,sizeof(buff));
			sprintf_s(buff,128,"PTK_SetLabelWidth:%d",ret);
			throw(buff);
		}
		ret = PTK_SetDarkness(10);
		if ( ret != 0 ){
			memset(buff,0,sizeof(buff));
			sprintf_s(buff,128,"PTK_SetDarkness:%d",ret);
			throw(buff);
		}
		ret = PTK_SetPrintSpeed(5);
		if ( ret != 0 ){
			memset(buff,0,sizeof(buff));
			sprintf_s(buff,128,"PTK_SetPrintSpeed:%d",ret);
			throw(buff);
		}
	}
	catch (const exception& e) {
		AfxMessageBox(e.what());
		return false;
	}
	catch (...) {
		memset(buff,0,sizeof(buff));
		sprintf_s(buff,128,"打印机错误:%d",GetLastError());
		AfxMessageBox(buff);
		return false;
	}
	return true;
}

void ClearPrinter()
{
	try
	{
		int ret = PTK_ClearBuffer();
	}
	catch (...)
	{
		
	}

}

void UnInitMyPrinter()
{
	ClosePort();
	if (NULL != gt1)
	{
		FreeLibrary(gt1);
		gt1 = NULL;
	}
	
}

LONG printContent(const string pp,/* const string printerNamet, */int nPrintType)
{
	int rtn=0;
	string printStr1 = "", printStr2 = "", printStr3 = "";
	char buff[128];
	bool bIsPrint = false;
	int id_name = 0/*,rtn = 0*/;
	try{
		if ("" == pp || nPrintType <= 0)
		{
			throw("传入参数错误！");
		}
		string printStr = pp;
		splitStrThree(printStr, printStr1, printStr2, printStr3);
	
		int X = 25,Y = 10;

		//打印左边
		if (printStr1.length() > 0 && (nPrintType == 1 || nPrintType == 4)){
			bIsPrint |= printNew3_(printStr1,X,Y,id_name,PTK_DrawBar2D_QR,PTK_DrawTextTrueTypeW);
		}
		//打印中间
		if (printStr2.length() > 0 && (nPrintType == 2 || nPrintType == 4)){
			bIsPrint |= printNew3_(printStr2,X+=269,Y,id_name,PTK_DrawBar2D_QR,PTK_DrawTextTrueTypeW);
		}
		//打印右边
		if (printStr3.length() > 0 && (nPrintType == 3 || nPrintType == 4)){
			bIsPrint |= printNew3_(printStr3,X+=269,Y,id_name,PTK_DrawBar2D_QR,PTK_DrawTextTrueTypeW);
		}
		//==================================
		if (bIsPrint){
			int ret = PTK_PrintLabel(1, 1);	
			if ( ret != 0 ){
				memset(buff,0,sizeof(buff));
				sprintf_s(buff,128,"PTK_PrintLabel:%d",ret);
				throw(buff);
			}
		}
		rtn = 1;
	}
	catch (const exception& e) {
		AfxMessageBox(e.what());
		rtn = 0;
	}
	catch (...) {
		memset(buff,0,sizeof(buff));
		sprintf_s(buff,128,"打印机错误:%d",GetLastError());
		AfxMessageBox(buff);
		rtn = 0;
	}
	
	return rtn;
}//*/