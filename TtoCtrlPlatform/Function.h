#ifndef _FUNCTION_H_
#define _FUNCTION_H_
#include "Definition.h"
#include <afxinet.h>
#include "TtoJson.h"
#include <list>


typedef int (__stdcall *fOpenPort)(char* PortFlag);
typedef int (__stdcall *fPTK_ClearBuffer)();
typedef int (__stdcall *fPTK_SetLabelHeight)(unsigned int lheight, unsigned int gapH);
typedef int (__stdcall *fPTK_SetLabelWidth)(unsigned int lwidth);
typedef int (__stdcall *fPTK_SetDarkness)(unsigned  int id);
typedef int (__stdcall *fPTK_SetPrintSpeed)(unsigned int px);
typedef int (__stdcall *fPTK_DrawBar2D_QR)(unsigned int x, unsigned int y,
								 unsigned int w, unsigned int v,
								 unsigned int o, unsigned int r,
								 unsigned int m, unsigned int g,
								 unsigned int s, char* pstr);
typedef int (__stdcall *fPTK_DrawText)(unsigned int px, unsigned int py,
							 unsigned int pdirec, unsigned int pFont,
							 unsigned int pHorizontal, unsigned int pVertical,
							 char ptext, char* pstr);

typedef int (__stdcall *fPTK_DrawTextTrueTypeW)(int x,int y,int FHeight,int FWidth,LPCTSTR FType,int Fspin,int FWeight,BOOL FItalic,BOOL FUnline,BOOL FStrikeOut,LPCTSTR id_name,LPCTSTR data);

typedef int (__stdcall *fClosePort)(void);

typedef int (__stdcall *fPTK_PrintLabel)(unsigned int number,
							   unsigned int cpnumber);

#pragma comment(lib, "Wininet.lib")  

int UTF8ToGBK(unsigned char * lpUTF8Str,unsigned char * lpGBKStr,int nGBKStrLen);
void  UTF8ToGB2132(string  strSrc,string &outstr);
BOOL Mult2Wide(char *inbuf,size_t inLen,wchar_t* outBuf,size_t *outLen);
string UrlProcess(LPCSTR url);
eRet WriteBinaryFile(const char* fileName, char* buffer, uint32 bufferLen);
eRet ReadBinaryFile(const char* fileName, char** outBuffer, uint32* outBufferLen);
string MakeFileName();
list<string> splitPrintStrN( string printStr );
list<string> splitPrintStrRN( string printStr );
list<string> splitPrintv( string& tstr, int DYJC );
void splitStrThree( string printStr, string &Str1, string &Str2, string&Str3 );
bool printNew3_(int X, int Y,int& id_name,fPTK_DrawBar2D_QR PTK_DrawBar2D_QR,fPTK_DrawTextTrueTypeW PTK_DrawTextTrueTypeW);
char* Gb2312ToUtf8(const char *pcGb2312) ;
void splitStrThree( string printStr, string &Str1, string &Str2, string&Str3 );
LONG printContent(const string pp, /*const string printerNamet,*/ int nPrintType);

bool InitMyPrinter(const char* printerNamet);
void ClearPrinter();
void UnInitMyPrinter();

#endif