#include <windows.h>
#include <fstream>
#include <iomanip>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <process.h>
#include <winsock.h>
#include <tchar.h>
#pragma comment(lib,"ws2_32.lib")
using namespace std;

/*-------------------------------------------------- 宏定义 -------------------------------------------------- */
#define WindWidth 800
#define WindHighth 600

//程序状态设置
#define ProConnect 0
#define ProLoad 1
#define ProCreate 2
#define ProMain 3
#define ProSure 100
#define ProPersonInformation 4
#define ProAddPerson 5
#define ProAddUnit 6
#define ProDeleteSure 7
#define ProSelect 8
#define ProSelectPersonInformation 9

struct person
{
	char name[100];		//姓名
	char nick[100];		//外号
	char sex[100];		//性别
	char birth[100];	//生日
	char unit[26];		//单位
	char habit[100];	//爱好	
	char qq[100];		//QQ	
	char phone[100];	//电话
	char tips[200];		//备注
	int relation;		//关系
	int match;			//是否被匹配上
};

/*-------------------------------------------------- 全局变量 -------------------------------------------------- */
static HINSTANCE hIns;	//程序实例句柄
static HWND hWnd;		//窗口参数

static HDC hdc ;			//设备环境句柄
static PAINTSTRUCT ps ;		//绘制结构
static POINT pt ;			//鼠标位置

static HDC hh;					
static HBITMAP bitMap;			//用于加载图片

static LOGFONT logFont;
static HFONT hFont;				//字体

static HPEN hPen;			//画笔
static HBRUSH hBrush;		//画刷

int proState = ProConnect;
int lastState = ProConnect;
int nextState = ProConnect;

int hStateConnect = 0;	//是否可以开启连接线程
int mainShow = 0;		//主界面是否已经显示过了

char serverIP[100] = "127.0.0.1";		//服务器IP
int port = 60001;

TCHAR g_buffT[100];
TCHAR g_buffT1[100];
TCHAR g_buffT2[100];

//-------------------- 列表数据
char stage = 0;					//阶段 0 - 4
char unit = 0;					//单位 0 - 4
char unitCount = 0;		//当前有几个单位
char unitName[5][100];	//当前单位名称

char matchOrNot = 1;			// 0 为取消匹配， 1 为开启匹配
struct person listPerson[100];	//当前窗口中的人物数据
char listPersonCount = 0;

char nowPersonNumber;			//当前选中的编号 0 - 71
struct person nowPerson;
char nowUnit[100];					//当前分组名称
char unitDeletePersonCount = 0;		//删除当前分组的关联删除人数

char selectInformation[100];	//查询信息


/*-------------------------------------------------- 功能函数 -------------------------------------------------- */
//一字节字符串转二字节字符串
void charToTchar(char * buff, TCHAR * buffT)  
{
	int iLength;  
	iLength = MultiByteToWideChar(CP_ACP, 0, buff, strlen(buff)+1, NULL, 0);  
	MultiByteToWideChar(CP_ACP, 0, buff, strlen(buff)+1, buffT, iLength);  
}

//二字节字符串转一字节字符串
void tcharToChar(TCHAR * buffT, char * buff)
{   
	int iLength;
	iLength = WideCharToMultiByte(CP_ACP, 0, buffT, wcslen(buffT), NULL, 0, NULL, NULL);     
	WideCharToMultiByte(CP_ACP, 0, buffT, wcslen(buffT), buff, iLength, NULL, NULL);   
	buff[iLength] = '\0';   
}	

/*-------------------------------------------------- 绘图函数 -------------------------------------------------- */
//画当前阶段、单位和是否匹配
void drawNowStateUnitMatch()
{
	int i;
	int x, y;
	TCHAR buffT[100];
	
	//改变输出字体为宋体    
	ZeroMemory(&logFont, sizeof(LOGFONT));   
	logFont.lfCharSet = GB2312_CHARSET;   
	hFont = CreateFontIndirect(&logFont);   
	SelectObject(hdc, hFont); 

	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_CENTER);	
	
	//写单位名称
	x = 109;
	y = 41;
	for(i = 0; i < unitCount; i++)
	{
		memset(buffT, 0, 200);
		charToTchar(unitName[i], buffT);
		TextOut(hdc, x+i*98, y, buffT, wcslen(buffT));	
	}
	
	//画阶段
	if(stage < 5)
	{
		x = 672;
		y = 98;
		y += 48*stage;
		hPen = CreatePen(PS_SOLID, 1, RGB(0,0,0));
		SelectObject(hdc,hPen);
		hBrush = CreateSolidBrush(RGB(0, 0, 0));  
		SelectObject(hdc, hBrush);
		Ellipse(hdc, x-3, y-3, x+3, y+3);
	}
	
	//画单位
	if(unit < 5)
	{
		x = 65;
		y = 32;
		x += 98*unit;
		MoveToEx(hdc, x, y, NULL);
		LineTo(hdc, x+90, y);
		y++;
		MoveToEx(hdc, x-1, y, NULL);
		LineTo(hdc, x+91, y);
	}
	
	//画匹配
	if(matchOrNot == 1)
	{
		x = 695;
		y = 343;
		hPen = CreatePen(PS_SOLID, 1, RGB(154,236,165));
		SelectObject(hdc,hPen);
		hBrush = CreateSolidBrush(RGB(128,222,143));  
		SelectObject(hdc, hBrush);
		Ellipse(hdc, x-15, y-15, x+15, y+15);
		x = 710;
		y = 353;
		hPen = CreatePen(PS_SOLID, 1, RGB(255,178,178));
		SelectObject(hdc,hPen);
		hBrush = CreateSolidBrush(RGB(255,129,129));  
		SelectObject(hdc, hBrush);
		Ellipse(hdc, x-15, y-15, x+15, y+15);
	}
}

//显示主界面表中的人名
void drawPersons()
{
	int i, j, k;
	int x, y;

	//改变输出字体为宋体      
	ZeroMemory(&logFont, sizeof(LOGFONT));   
	logFont.lfCharSet = GB2312_CHARSET;   
	hFont = CreateFontIndirect(&logFont);   
	SelectObject(hdc, hFont); 
	
	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_LEFT);	
	TCHAR buffT[100];
	
	for(i = 0; i < listPersonCount; i++)
	{
		x = 95;
		y = 90;
		j = i/6;
		k = i%6;
		y += j*25;
		x += k*90;
		memset(buffT, 0, 200);
		charToTchar(listPerson[i].name, buffT);
		TextOut(hdc, x, y, buffT, wcslen(buffT));	
		if(listPerson[i].match == 1 && matchOrNot == 1)
		{
			if(strcmp(listPerson[i].sex, "男") == 0)
			{
				hPen = CreatePen(PS_SOLID, 1, RGB(128,222,143));
			}
			else 
			{
				hPen = CreatePen(PS_SOLID, 1, RGB(255,129,129));
			}
			SelectObject(hdc,hPen); 
			MoveToEx(hdc, x, y+18, NULL);
			LineTo(hdc, x+8*strlen(listPerson[i].name), y+18);
			
		}
	}
}

//显示查询界面表中的人名
void drawSelectPersons()
{
	int i, j, k;
	int x, y;

	//改变输出字体为宋体    
	ZeroMemory(&logFont, sizeof(LOGFONT));   
	logFont.lfCharSet = GB2312_CHARSET;   
	hFont = CreateFontIndirect(&logFont);   
	SelectObject(hdc, hFont); 

	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_LEFT);	
	TCHAR buffT[100];
	
	for(i = 0; i < listPersonCount && i < 45; i++)
	{
		x = 175;
		y = 80;
		j = i/5;
		k = i%5;
		y += j*25;
		x += k*90;
		memset(buffT, 0, 200);
		charToTchar(listPerson[i].name, buffT);
		TextOut(hdc, x, y, buffT, wcslen(buffT));	
	}
}

/*-------------------------------------------------- 网络函数 -------------------------------------------------- */
WSAData WSAData;
struct sockaddr_in sockAddr;
int socketClient;

//建立连接
unsigned int __stdcall Connection(void *)
{
	if(WSAStartup(MAKEWORD(1,1),&WSAData))
	{
		MessageBox(NULL, TEXT("网络初始化失败!"), TEXT("ERROR"), NULL);
		WSACleanup();
		return 0;
	}

	//设置通信地址
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;							
	sockAddr.sin_addr.s_addr = inet_addr(serverIP);		
	sockAddr.sin_port = htons(port);						//端口号
	

	//设置一个socket
	socketClient = socket(AF_INET, SOCK_STREAM, 0);
	int sockGet = -1;

	//循环连接
	while(sockGet < 0)
	{
		sockGet = connect(socketClient,(struct sockaddr *)&sockAddr,sizeof(sockAddr));
		Sleep(500);
	}
	//MessageBox(NULL, TEXT("服务器已经上线!"), TEXT("Welcome"), NULL);
	proState = ProLoad;
	InvalidateRect(hWnd, NULL, TRUE);
	return 0;
}	

/*------------------------------ // 传输约定 // ------------------------------
1、客户端首先传输一个字符
	0：创建用户名
	1：传输用户名和密码
	z：终止连接

2、之后进行此字符约定的传输
3、回到步骤1
------------------------------------------------------------*/

int sendAll(char s)
{
	int i;
	char recvBuff[100];
	char sendBuff[100];
	char sendBuff1[100];
	char sendBuff2[100];
	TCHAR buffT[100];
	memset(recvBuff, 0 ,100);
	memset(sendBuff, 0 ,100);
	memset(sendBuff1, 0 ,100);
	memset(sendBuff2, 0 ,100);
	
	sendBuff[0] = s;
	send(socketClient, sendBuff, 1, 0);
	switch(s)
	{
		//------------------------------ 创建用户
		case '0':
		{
			tcharToChar(g_buffT, sendBuff);
			tcharToChar(g_buffT1, sendBuff1);
			send(socketClient, sendBuff, 80, 0);
			send(socketClient, sendBuff1, 80, 0);
			recv(socketClient, recvBuff, 1, 0);
			if(recvBuff[0] == '1') return 1;
			else return 0;
			
			break;
		}
		//------------------------------ 用户登录
		case '1':
		{
			tcharToChar(g_buffT, sendBuff);
			tcharToChar(g_buffT1, sendBuff1);
			send(socketClient, sendBuff, 80, 0);
			send(socketClient, sendBuff1, 80, 0);
			recv(socketClient, recvBuff, 1, 0);
			if(recvBuff[0] == '1') return 1;
			else if(recvBuff[0] == '2') return 2;
			else return 0;
			break;
		}

		//------------------------------ 传输主界面数据
		case 'a':
		{
			sendBuff[0] = matchOrNot;
			send(socketClient, sendBuff, 1, 0);		//发送是否匹配
			sendBuff[0] = stage;
			send(socketClient, sendBuff, 1, 0);		//发送当前阶段
			sendBuff[0] = unit;
			send(socketClient, sendBuff, 1, 0);		//发送当前单位
			
			recv(socketClient, recvBuff, 1, 0);		//接受当前阶段有几个单位
			unitCount = recvBuff[0];
			
			//接收当前阶段单位名称
			recv(socketClient, unitName[0], 100, 0);
			recv(socketClient, unitName[1], 100, 0);
			recv(socketClient, unitName[2], 100, 0);
			recv(socketClient, unitName[3], 100, 0);
			recv(socketClient, unitName[4], 100, 0);
			
			recv(socketClient, recvBuff, 1, 0);		//接收当前单位有几个人员
			listPersonCount = recvBuff[0];
			
			//接收当前单位人员数据
			for(i = 0; i < listPersonCount; i++)
			{
				recv(socketClient, listPerson[i].name, 100, 0);		//姓名
				recv(socketClient, listPerson[i].nick, 100, 0);		//外号
				recv(socketClient, listPerson[i].sex, 100, 0);		//性别				
				recv(socketClient, listPerson[i].birth, 100, 0);	//生日	
				recv(socketClient, listPerson[i].unit, 26, 0);		//分组			
				recv(socketClient, listPerson[i].habit, 100, 0);	//爱好				
				memset(recvBuff, 0 ,100);
				recv(socketClient, recvBuff, 4, 0);					//关系
				sscanf(recvBuff, "%d", &listPerson[i].relation);				
				recv(socketClient, listPerson[i].qq, 100, 0);		//QQ				
				recv(socketClient, listPerson[i].phone, 100, 0);	//电话					
				recv(socketClient, listPerson[i].tips, 200, 0);		//备注
				
				if(matchOrNot == 1)
				{
					recv(socketClient, recvBuff, 1, 0);					//是否匹配上
					listPerson[i].match = recvBuff[0];
				}
			}
			break;
		}
		
		//------------------------------ 修改某人属性
		case 'b':
		{
			send(socketClient, nowPerson.name, 50, 0);
			send(socketClient, nowPerson.nick, 50, 0);
			send(socketClient, nowPerson.sex, 50, 0);
			send(socketClient, nowPerson.birth, 50, 0);
			send(socketClient, nowPerson.habit, 100, 0);	
			send(socketClient, nowPerson.qq, 50, 0);
			send(socketClient, nowPerson.phone, 50, 0);
			send(socketClient, nowPerson.tips, 200, 0);
			memset(sendBuff1, 0, 100);
			sendBuff1[0] = nowPerson.relation+10;
			send(socketClient, sendBuff1, 1, 0);
			
			
			break;
		}
		//------------------------------ 删除某人在某单位的数据
		case 'c':
		{
			send(socketClient, nowPerson.name, 50, 0);
			memset(sendBuff, 0, 100);	
			sendBuff[0] = stage;
			send(socketClient, sendBuff, 1, 0);	
			memset(sendBuff, 0, 100);	
			sendBuff[0] = unit;
			send(socketClient, sendBuff, 1, 0);	
			
			memset(buffT, 0, 100);
			charToTchar(nowPerson.unit, buffT);
			send(socketClient, nowPerson.unit, 26, 0);	
		
			break;
		}
		
		//------------------------------ 添加单位中的某人
		case 'd':
		{
			send(socketClient, nowPerson.name, 50, 0);
			send(socketClient, nowPerson.nick, 50, 0);
			send(socketClient, nowPerson.sex, 50, 0);
			send(socketClient, nowPerson.birth, 50, 0);
			send(socketClient, nowPerson.habit, 100, 0);	
			send(socketClient, nowPerson.qq, 50, 0);
			send(socketClient, nowPerson.phone, 50, 0);
			send(socketClient, nowPerson.tips, 200, 0);
			memset(sendBuff1, 0, 100);
			sendBuff1[0] = nowPerson.relation+10;
			send(socketClient, sendBuff1, 1, 0);
			
			memset(recvBuff, 0, 100);
			recv(socketClient, recvBuff, 1, 0);
			if(recvBuff[0] == '1') MessageBox(NULL, TEXT("检测到相同的姓名，此人信息已加入本分组"), TEXT("提示"), NULL);
			
			break;
		}
		//------------------------------ 添加分组
		case 'e':
		{
			send(socketClient, nowUnit, 80, 0);
			break;
		}
		//------------------------------ 删除分组前的确认
		case 'f':
		{
			memset(recvBuff, 0, 100);
			recv(socketClient, recvBuff, 1, 0);
			unitDeletePersonCount = recvBuff[0];
			
			break;
		}
		//------------------------------ 删除分组
		case 'g':
		{
			break;
		}
		
		//------------------------------ 查找
		case 'h':
		case 'i':
		case 'j':
		{
			send(socketClient, selectInformation, 50, 0);
			
			recv(socketClient, recvBuff, 1, 0);		//接收有几个人员
			listPersonCount = recvBuff[0];

			//接收人员数据
			for(i = 0; i < listPersonCount; i++)
			{
				recv(socketClient, listPerson[i].name, 100, 0);		//姓名
				recv(socketClient, listPerson[i].nick, 100, 0);		//外号
				recv(socketClient, listPerson[i].sex, 100, 0);		//性别				
				recv(socketClient, listPerson[i].birth, 100, 0);	//生日				
				recv(socketClient, listPerson[i].habit, 100, 0);	//爱好				
				memset(recvBuff, 0 ,100);
				recv(socketClient, recvBuff, 4, 0);					//关系
				sscanf(recvBuff, "%d", &listPerson[i].relation);				
				recv(socketClient, listPerson[i].qq, 100, 0);		//QQ				
				recv(socketClient, listPerson[i].phone, 100, 0);	//电话					
				recv(socketClient, listPerson[i].tips, 200, 0);		//备注
			}

			break;
		}
		
		//------------------------------ 断开连接
		case 'z':
		{
			hStateConnect = 0;
			proState = ProConnect;
			break;
		}
	}	
	return 0;
}

void DeleteAllGDI()
{
	/*
	static HINSTANCE hIns;	//程序实例句柄
	static HWND hWnd;		//窗口参数

	static HDC hdc ;			//设备环境句柄
	static PAINTSTRUCT ps ;		//绘制结构
	static POINT pt ;			//鼠标位置

	static HDC hh;					
	static HBITMAP bitMap;			//用于加载图片

	static LOGFONT logFont;
	static HFONT hFont;				//字体

	static HPEN hPen;			//画笔
	static HBRUSH hBrush;		//画刷
	*/
	
	DeleteDC(hdc);
	DeleteDC(hh);
	DeleteObject(bitMap);
	//DeleteObject(hFont);
	DeleteObject(hPen);
	DeleteObject(hBrush);
}

/*-------------------------------------------------- 主函数体 -------------------------------------------------- */
LRESULT CALLBACK WindProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i = 0, j = 0 ,k = 0, l = 0, m = 0, n = 0, x = 0, y = 0;
	char c;
	char buff[200];
	char buff1[200];
	char buff2[200];
	TCHAR buffT[100];
	TCHAR buffT1[100];
	TCHAR buffT2[100];
	
	switch(proState)
	{
		//---------------------------------------- 确认界面 ----------------------------------------
		case ProSure:
		{
			switch(message)
			{
				case WM_PAINT:
				{
					hdc = BeginPaint(hwnd, &ps);
					bitMap = (HBITMAP)LoadImage(NULL, L"Face/确认界面.bmp", IMAGE_BITMAP, WindWidth, WindHighth, LR_LOADFROMFILE);
					hh = CreateCompatibleDC(NULL);
					SelectObject(hh, bitMap);
					BitBlt(hdc, 0, 0, WindWidth, WindHighth, hh, 0, 0, SRCCOPY);
					
					DeleteAllGDI();
					EndPaint(hwnd, &ps);
				
					return 0;
				}
				case WM_LBUTTONUP:
				{	
					GetCursorPos(&pt);
					ScreenToClient(hwnd, &pt);   

					//---------------------------------------- 点击“是”
					if(pt.x > 266 && pt.x < 364 && pt.y > 267 && pt.y < 325)
					{
						if(nextState == ProConnect)
						{
							sendAll('z');
							closesocket(socketClient);
							WSACleanup();
						}
						else if(lastState == ProPersonInformation && nextState == ProMain)
						{
							sendAll('c');		//删除某人在某单位的数据
						}
						proState = nextState;
							
					}
					//---------------------------------------- 点击“否”
					else if(pt.x > 435 && pt.x < 533 && pt.y > 267 && pt.y < 325)
					{
						proState = lastState;
					}
					else return 0;
					
					InvalidateRect(hwnd, NULL, TRUE);
					return 0;
				}
				case WM_DESTROY:
				{
					sendAll('z');
					closesocket(socketClient);
					WSACleanup();
					PostQuitMessage(0);
					return 0;
				}
			}
			
			break;
		}
		
		//---------------------------------------- 分组删除确认界面 ----------------------------------------
		case ProDeleteSure:
		{
			static int state = 0;
			if(state == 0)
			{
				sendAll('f');
				state = 1;
			}
			switch(message)
			{
				case WM_PAINT:
				{
					hdc = BeginPaint(hwnd, &ps);
					bitMap = (HBITMAP)LoadImage(NULL, L"Face/删除单位界面.bmp", IMAGE_BITMAP, WindWidth, WindHighth, LR_LOADFROMFILE);
					hh = CreateCompatibleDC(NULL);
					SelectObject(hh, bitMap);
					BitBlt(hdc, 0, 0, WindWidth, WindHighth, hh, 0, 0, SRCCOPY);
					
					memset(buff, 0 ,200);
					memset(buffT, 0, 200);
					buff[0] = unitDeletePersonCount + '0';
					charToTchar(buff, buffT);
					TextOut(hdc, 422, 132, buffT, wcslen(buffT));	
					
					DeleteAllGDI();
					EndPaint(hwnd, &ps);
				
					return 0;
				}
				case WM_LBUTTONUP:
				{	
					GetCursorPos(&pt);
					ScreenToClient(hwnd, &pt);   

					//---------------------------------------- 点击“是”
					if(pt.x > 265 && pt.x < 364 && pt.y > 301 && pt.y < 359)
					{
						proState = ProMain;
						sendAll('g');			//删除分组
						if(unit > 0) unit--;
						state = 0;
					}
					//---------------------------------------- 点击“否”
					else if(pt.x > 435 && pt.x < 534 && pt.y > 301 && pt.y < 359)
					{
						proState = ProMain;
						state = 0;
					}
					else return 0;
					
					InvalidateRect(hwnd, NULL, TRUE);
					return 0;
				}
				case WM_DESTROY:
				{
					sendAll('z');
					closesocket(socketClient);
					WSACleanup();
					PostQuitMessage(0);
					return 0;
				}
			}
			
			break;
		}
		
		//---------------------------------------- 连接到服务器 ----------------------------------------
		case ProConnect:
		{
			if(hStateConnect == 0)
			{
				hStateConnect = 1;
				_beginthreadex(NULL, 0, Connection, NULL, 0, NULL);		//开启连接线程
				stage = 0;					
				unit = 0;				
				matchOrNot = 0;			
			}

			switch(message)
			{
				case WM_PAINT:
				{
					hdc = BeginPaint(hwnd, &ps);
					bitMap = (HBITMAP)LoadImage(NULL, L"Face/连接界面.bmp", IMAGE_BITMAP, WindWidth, WindHighth, LR_LOADFROMFILE);
					hh = CreateCompatibleDC(NULL);
					SelectObject(hh, bitMap);
					BitBlt(hdc, 0, 0, WindWidth, WindHighth, hh, 0, 0, SRCCOPY);
					
					DeleteAllGDI();
					EndPaint(hwnd, &ps);
				
					return 0;
				}
				case WM_DESTROY:
				{
					closesocket(socketClient);
					WSACleanup();
					PostQuitMessage(0);
					return 0;
				}
			}
			break;
		}	
		
		//---------------------------------------- 登录 ----------------------------------------
		case ProLoad:
		{
			static HWND hText;
			static HWND hText1;
			static int textState = 0;
			
			//仅第一次创建输入框
			if(textState == 0)
			{
				textState = 1;
				hText = CreateWindow		//用户名
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER,
					350, 178, 160, 20,
					hwnd, NULL, hIns, NULL 
				);
				hText1 = CreateWindow		//密码
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER | ES_PASSWORD,
					350, 225, 160, 20,
					hwnd, NULL, hIns, NULL 
				);		
				//改变字体
				SendMessage(hText, WM_SETFONT, (WPARAM)hFont, TRUE);
				SendMessage(hText1, WM_SETFONT, (WPARAM)hFont, TRUE);
			}
			
			switch(message)
			{
				case WM_PAINT:
				{
					hdc = BeginPaint(hwnd, &ps);
					bitMap = (HBITMAP)LoadImage(NULL, L"Face/登录界面.bmp", IMAGE_BITMAP, WindWidth, WindHighth, LR_LOADFROMFILE);
					hh = CreateCompatibleDC(NULL);
					SelectObject(hh, bitMap);
					BitBlt(hdc, 0, 0, WindWidth, WindHighth, hh, 0, 0, SRCCOPY);
							
					DeleteAllGDI();
					EndPaint(hwnd, &ps);
					
					return 0;
				}
				case WM_LBUTTONUP:	
				{
					GetCursorPos(&pt);
					ScreenToClient(hwnd, &pt);   
					
					//---------------------------------------- 点击“确认”
					if(pt.x > 359 && pt.x < 438 && pt.y > 348 && pt.y < 387)
					{
						GetWindowText(hText, g_buffT, 50);
						GetWindowText(hText1, g_buffT1, 50);
						i = sendAll('1');
						if(i == 0)			//登录失败
						{
							MessageBox(NULL, TEXT("输入错误! 请核对后重新输入"), TEXT("Error"), NULL);
							return 0;
						}
						else				//用户登录
						{
							textState = 0;
							proState = ProMain;
						}
					}
					//---------------------------------------- 点击“注册”
					else if(pt.x > 440 && pt.x < 478 && pt.y > 104 && pt.y < 120)
					{
						
						textState = 0;
						proState = ProCreate;
					}
					else return 0;

					DestroyWindow(hText);
					DestroyWindow(hText1);
					InvalidateRect(hwnd, NULL, TRUE);
					return 0;
				}
				case WM_DESTROY:
				{
					sendAll('z');
					closesocket(socketClient);
					WSACleanup();
					PostQuitMessage(0);
					return 0;
				}
			}
		
			break;
		}

		//---------------------------------------- 创建用户 ----------------------------------------
		case ProCreate:
		{
			static HWND hText;
			static HWND hText1;
			static HWND hText2;
			static int textState = 0;
			
			//仅第一次创建输入框
			if(textState == 0)
			{
				textState = 1;
				hText = CreateWindow		//用户名
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER,
					350, 178, 160, 20,
					hwnd, NULL, hIns, NULL 
				);
				hText1 = CreateWindow		//密码
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER | ES_PASSWORD,
					350, 225, 160, 20,
					hwnd, NULL, hIns, NULL 
				);
				hText2 = CreateWindow		//确认密码
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER | ES_PASSWORD,
					350, 273, 160, 20,
					hwnd, NULL, hIns, NULL 
				);
				SendMessage(hText, WM_SETFONT, (WPARAM)hFont, TRUE);
				SendMessage(hText1, WM_SETFONT, (WPARAM)hFont, TRUE);
				SendMessage(hText2, WM_SETFONT, (WPARAM)hFont, TRUE);
			}
			
			switch(message)
			{
				case WM_PAINT:
				{
					hdc = BeginPaint(hwnd, &ps);
					bitMap = (HBITMAP)LoadImage(NULL, L"Face/注册界面.bmp", IMAGE_BITMAP, WindWidth, WindHighth, LR_LOADFROMFILE);
					hh = CreateCompatibleDC(NULL);
					SelectObject(hh, bitMap);
					BitBlt(hdc, 0, 0, WindWidth, WindHighth, hh, 0, 0, SRCCOPY);
				
					DeleteAllGDI();
					EndPaint(hwnd, &ps);
				
					return 0;
				}
				case WM_LBUTTONUP:	
				{
					GetCursorPos(&pt);
					ScreenToClient(hwnd, &pt);   
					
					//---------------------------------------- 点击“确认”
					if(pt.x > 359 && pt.x < 438 && pt.y > 348 && pt.y < 387)
					{
						GetWindowText(hText, g_buffT, 50);
						GetWindowText(hText1, g_buffT1, 50);
						GetWindowText(hText2, g_buffT2, 50);
						
						tcharToChar(g_buffT, buff);
						tcharToChar(g_buffT1, buff1);
						tcharToChar(g_buffT2, buff2);	
						
						//客户端判断的错误
						if(strlen(buff) <= 0)
						{
							MessageBox(NULL, TEXT("用户名不能为空"), TEXT("提示"), NULL);
							return 0;
						}
						if(strcmp(buff1, buff2))			
						{
							MessageBox(NULL, TEXT("两次密码不相同"), TEXT("提示"), NULL);
							return 0;
						}
						if(strlen(buff1) <= 3)
						{
							MessageBox(NULL, TEXT("密码至少4个字符"), TEXT("提示"), NULL);
							return 0;
						}
						
						i = sendAll('0');
						if(i == 0)			//登录失败
						{
							MessageBox(NULL, TEXT("用户已存在!"), TEXT("Error"), NULL);
							return 0;
						}
						else if(i == 1)		//用户创建成功
						{
							
							textState = 0;
							proState = ProMain;
						}
					}
					//---------------------------------------- 点击“登录”
					else if(pt.x > 440 && pt.x < 478 && pt.y > 104 && pt.y < 120)
					{
						textState = 0;
						proState = ProLoad;
					}
					else return 0;

					DestroyWindow(hText);
					DestroyWindow(hText1);
					DestroyWindow(hText2);
					InvalidateRect(hwnd, NULL, TRUE);
					return 0;
				}
				case WM_DESTROY:
				{
					sendAll('z');
					closesocket(socketClient);
					WSACleanup();
					PostQuitMessage(0);
					return 0;
				}
			}
			
			break;
		}
		
		//---------------------------------------- 查询界面 ----------------------------------------
		case ProSelect:
		{
			static int state = 0;
			static HWND hText;
			if(state == 0 || state == 2)
			{
				if(state == 0) listPersonCount = 0;
				state = 1;
				
				hText = CreateWindow		//查询内容
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | ES_CENTER,
					358, 389, 85, 20,
					hwnd, NULL, hIns, NULL 
				);		
				SendMessage(hText, WM_SETFONT, (WPARAM)hFont, TRUE);
			}
			
			switch(message)
			{
				case WM_PAINT:
				{
					hdc = BeginPaint(hwnd, &ps);
					bitMap = (HBITMAP)LoadImage(NULL, L"Face/查询界面.bmp", IMAGE_BITMAP, WindWidth, WindHighth, LR_LOADFROMFILE);
					hh = CreateCompatibleDC(NULL);
					SelectObject(hh, bitMap);
					BitBlt(hdc, 0, 0, WindWidth, WindHighth, hh, 0, 0, SRCCOPY);
					
					drawSelectPersons();
					
					DeleteAllGDI();
					EndPaint(hwnd, &ps);
				
					return 0;
				}
				case WM_LBUTTONUP:
				{	
					
					GetCursorPos(&pt);
					ScreenToClient(hwnd, &pt);   

					//---------------------------------------- 点击“返回”
					if(pt.x > 590 && pt.x < 670 && pt.y > 325 && pt.y < 360)
					{
						proState = ProMain;
						state = 0;
						DestroyWindow(hText);
					}
					
					//---------------------------------------- 点击“找名字”
					else if(pt.x > 261 && pt.x < 336 && pt.y > 446 && pt.y < 521)
					{
						memset(buffT, 0, 100);
						GetWindowText(hText, buffT, 100);
						if(wcslen(buffT) == 0)
						{
							MessageBox(NULL, TEXT("查询内容不能为空"), TEXT("提示"), NULL);
							return 0;
						}
						memset(selectInformation, 0, 100);
						tcharToChar(buffT, selectInformation);
						
						sendAll('h');		
					}
					
					//---------------------------------------- 点击“找爱好”
					else if(pt.x > 362 && pt.x < 437 && pt.y > 446 && pt.y < 521)
					{
						memset(buffT, 0, 100);
						GetWindowText(hText, buffT, 100);
						if(wcslen(buffT) == 0)
						{
							MessageBox(NULL, TEXT("查询内容不能为空"), TEXT("提示"), NULL);
							return 0;
						}
						memset(selectInformation, 0, 100);
						tcharToChar(buffT, selectInformation);

						sendAll('i');		
					}
					
					//---------------------------------------- 点击“找密友”
					else if(pt.x > 465 && pt.x < 540 && pt.y > 446 && pt.y < 521)
					{
						memset(buffT, 0, 100);
						GetWindowText(hText, buffT, 100);
						if(wcslen(buffT) == 0)
						{
							MessageBox(NULL, TEXT("查询内容不能为空"), TEXT("提示"), NULL);
							return 0;
						}
						memset(selectInformation, 0, 100);
						tcharToChar(buffT, selectInformation);

						sendAll('j');		
					}
					
					//---------------------------------------- 点击“人员”
					else if(pt.x > 175 && pt.x < 625 && pt.y > 80 && pt .y < 305)
					{
						j = pt.y-80;
						k = pt.x-175;
						j = j/25;
						k = k/90;
						i = j*5+k;
						if(i >= listPersonCount) return 0;	//点击空白区域
						
						nowPerson = listPerson[i];
						nowPersonNumber = i;
						
						proState = ProSelectPersonInformation;
						state = 2;
						DestroyWindow(hText);
					}
					else return 0;
					
					InvalidateRect(hwnd, NULL, TRUE);
					return 0;
					
				}
				case WM_DESTROY:
				{
					sendAll('z');
					closesocket(socketClient);
					WSACleanup();
					PostQuitMessage(0);
					return 0;
				}
			}
			break;
		}
		
		//---------------------------------------- 主界面 ----------------------------------------
		case ProMain:
		{
			if(mainShow == 0)	
			{
				mainShow = 1;
				sendAll('a');
			}
			switch(message)
			{
				case WM_PAINT:
				{
					hdc = BeginPaint(hwnd, &ps);
					bitMap = (HBITMAP)LoadImage(NULL, L"Face/主界面.bmp", IMAGE_BITMAP, WindWidth, WindHighth, LR_LOADFROMFILE);
					hh = CreateCompatibleDC(NULL);
					SelectObject(hh, bitMap);
					BitBlt(hdc, 0, 0, WindWidth, WindHighth, hh, 0, 0, SRCCOPY);
					
					drawNowStateUnitMatch();	
					drawPersons();
					
					DeleteAllGDI();
					EndPaint(hwnd, &ps);
				
					return 0;
				}
				case WM_LBUTTONUP:
				{	
					
					GetCursorPos(&pt);
					ScreenToClient(hwnd, &pt);   

					//---------------------------------------- 点击“退出”
					if(pt.x > 645 && pt.x < 720 && pt.y > 453 && pt.y < 528)
					{
						lastState = ProMain;
						nextState = ProConnect;
						proState = ProSure;
						mainShow = 0;
					}
					//---------------------------------------- 点击“匹配”
					else if(pt.x > 387 && pt.x < 462 && pt.y > 453 && pt.y < 528)
					{
						matchOrNot = !matchOrNot;
						sendAll('a');				//获取列表中人物名单
					}
					//---------------------------------------- 点击“查询”
					else if(pt.x > 106 && pt.x < 367 && pt.y > 453 && pt.y < 528)
					{
						proState = ProSelect;
						mainShow = 0;
					}
					
					//---------------------------------------- 点击“阶段”
					else if(pt.x > 661 && pt.x < 739 && pt.y > 72 && pt.y < 312)
					{
						if(pt.y < 122) stage = 0;						
						else if(pt.y < 170) stage = 1;	
						else if(pt.y < 217) stage = 2;
						else if(pt.y < 265) stage = 3;
						else stage = 4;
						unit = 0;
						sendAll('a');				//获取列表中人物名单
					}
					//---------------------------------------- 点击“单位”
					else if(pt.x > 60 && pt.x < 550 && pt.y > 35 && pt.y < 63)
					{
						if(pt.x < 158 && unitCount >= 1) unit = 0;						
						else if(pt.x < 256 && unitCount >= 2) unit = 1;	
						else if(pt.x < 354 && unitCount >= 3) unit = 2;
						else if(pt.x < 452 && unitCount >= 4) unit = 3;
						else if(unitCount >= 5) unit = 4;
						else return 0;
						sendAll('a');				//获取列表中人物名单
					}
					//---------------------------------------- 点击“人员”
					else if(pt.x > 95 && pt.x < 635 && pt.y > 90 && pt .y < 390)
					{
						j = pt.y-90;
						k = pt.x-95;
						j = j/25;
						k = k/90;
						i = j*6+k;
						if(i >= listPersonCount) return 0;	//点击空白区域
						
						nowPerson = listPerson[i];
						nowPersonNumber = i;
						
						proState = ProPersonInformation;
						mainShow = 0;
					}
					//---------------------------------------- 点击“添加人员”
					else if(pt.x > 680 && pt.x < 732 && pt.y > 387 && pt .y < 407)
					{
						if(unitCount > 0 && listPersonCount < 72)
						{
							proState = ProAddPerson;
							mainShow = 0;
						}
						else MessageBox(NULL, TEXT("当前无法添加"), TEXT("提示"), NULL);
					}
					//---------------------------------------- 点击“添加分组”
					else if(pt.x > 550 && pt.x < 579 && pt.y > 35 && pt .y < 63)
					{
						if(unitCount < 5)
						{
							proState = ProAddUnit;
							mainShow = 0;
						}
						else MessageBox(NULL, TEXT("分组已满"), TEXT("提示"), NULL);
					}
					//---------------------------------------- 点击“删除分组”
					else if(pt.x > 579 && pt.x < 608 && pt.y > 35 && pt .y < 63)
					{
						if(unitCount > 0)
						{
							proState = ProDeleteSure;
							mainShow = 0;
						}
						else MessageBox(NULL, TEXT("当前阶段没有分组"), TEXT("提示"), NULL);
					}	
					else return 0;
					
					InvalidateRect(hwnd, NULL, TRUE);
					return 0;
					
				}
				case WM_DESTROY:
				{
					sendAll('z');
					closesocket(socketClient);
					WSACleanup();
					PostQuitMessage(0);
					return 0;
				}
			}
			break;
		}
		
		//---------------------------------------- 查询信息界面 ----------------------------------------
		case ProSelectPersonInformation:
		{
			static HWND hTextName, hTextNick, hTextSex, hTextBirth, hTextPhone, hTextQq, hTextHabit, hTextTips, hTextRelation;
			
			switch(message)
			{
				case WM_PAINT:
				{
					hdc = BeginPaint(hwnd, &ps);
					bitMap = (HBITMAP)LoadImage(NULL, L"Face/查询信息界面.bmp", IMAGE_BITMAP, WindWidth, WindHighth, LR_LOADFROMFILE);
					hh = CreateCompatibleDC(NULL);
					SelectObject(hh, bitMap);
					BitBlt(hdc, 0, 0, WindWidth, WindHighth, hh, 0, 0, SRCCOPY);
					
					//改变输出字体为宋体      
					ZeroMemory(&logFont, sizeof(LOGFONT));   
					logFont.lfCharSet = GB2312_CHARSET;   
					hFont = CreateFontIndirect(&logFont);   
					SelectObject(hdc, hFont); 
					
					memset(buffT, 0 ,200);					
					charToTchar(nowPerson.name, buffT);
					TextOut(hdc, 220, 113, buffT, wcslen(buffT));
					
					memset(buffT, 0 ,200);
					charToTchar(nowPerson.nick, buffT);
					TextOut(hdc, 220, 157, buffT, wcslen(buffT));
					
					memset(buffT, 0 ,200);
					charToTchar(nowPerson.sex, buffT);
					TextOut(hdc, 220, 199, buffT, wcslen(buffT));
					
					memset(buffT, 0 ,200);
					charToTchar(nowPerson.birth, buffT);
					TextOut(hdc, 392, 113, buffT, wcslen(buffT));
					
					memset(buffT, 0 ,200);
					charToTchar(nowPerson.phone, buffT);
					TextOut(hdc, 572, 113, buffT, wcslen(buffT));
				
					memset(buffT, 0 ,200);
					charToTchar(nowPerson.qq, buffT);
					TextOut(hdc, 572, 157, buffT, wcslen(buffT));
			
					memset(buffT, 0 ,200);
					charToTchar(nowPerson.habit, buffT);
					TextOut(hdc, 160, 274, buffT, wcslen(buffT));
					
					memset(buff, 0, 100);
					strncpy(buff, nowPerson.tips, 26);
					memset(buffT, 0 ,200);
					charToTchar(buff, buffT);
					TextOut(hdc, 420, 274, buffT, wcslen(buffT));
					memset(buff, 0, 100);
					char *p = nowPerson.tips;
					p += 26;
					strncpy(buff, p, 26);
					memset(buffT, 0 ,200);
					charToTchar(buff, buffT);
					TextOut(hdc, 420, 290, buffT, wcslen(buffT));
					
					memset(buff, 0 ,200);
					memset(buffT, 0 ,200);
					sprintf(buff, "%d", nowPerson.relation);
					charToTchar(buff, buffT);
					TextOut(hdc, 395, 382, buffT, wcslen(buffT));
				
					DeleteAllGDI();
					EndPaint(hwnd, &ps);
			
					return 0;
				}
				case WM_LBUTTONUP:
				{	
					
					GetCursorPos(&pt);
					ScreenToClient(hwnd, &pt);   

					//---------------------------------------- 点击“返回”
					if(pt.x > 497 && pt.x < 615 && pt.y > 361 && pt.y < 419)
					{
						proState = ProSelect;
					}
					else return 0;

					InvalidateRect(hwnd, NULL, TRUE);
					return 0;
					
				}
				case WM_DESTROY:
				{
					sendAll('z');
					closesocket(socketClient);
					WSACleanup();
					PostQuitMessage(0);
					return 0;
				}
			}
			
			break;
		}
			
		//---------------------------------------- 个人信息界面 ----------------------------------------
		case ProPersonInformation:
		{
			static HWND hTextName, hTextNick, hTextSex, hTextBirth, hTextPhone, hTextQq, hTextHabit, hTextTips, hTextRelation;
			static int textState = 0;
			
			//仅第一次创建输入框
			if(textState == 0)
			{
				textState = 1;
				//------------------------------ 绘制8个文本框
				{
				hTextNick = CreateWindow		//外号
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | ES_LEFT,
					220, 157, 85, 20,
					hwnd, NULL, hIns, NULL 
				);
				memset(buffT, 0 ,200);
				charToTchar(nowPerson.nick, buffT);
				SetWindowText(hTextNick, buffT);
				
				hTextSex = CreateWindow		//性别
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | ES_LEFT,
					220, 199, 20, 20,
					hwnd, NULL, hIns, NULL 
				);
				memset(buffT, 0 ,200);
				charToTchar(nowPerson.sex, buffT);
				SetWindowText(hTextSex, buffT);
				
				hTextBirth = CreateWindow		//生日
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER,
					392, 113, 70, 20,
					hwnd, NULL, hIns, NULL 
				);
				memset(buffT, 0 ,200);
				charToTchar(nowPerson.birth, buffT);
				SetWindowText(hTextBirth, buffT);
				
				hTextPhone = CreateWindow		//电话
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER,
					572, 113, 95, 20,
					hwnd, NULL, hIns, NULL 
				);
				memset(buffT, 0 ,200);
				charToTchar(nowPerson.phone, buffT);
				SetWindowText(hTextPhone, buffT);
				
				hTextQq = CreateWindow		//QQ
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER,
					572, 157, 95, 20,
					hwnd, NULL, hIns, NULL 
				);
				memset(buffT, 0 ,200);
				charToTchar(nowPerson.qq, buffT);
				SetWindowText(hTextQq, buffT);
				
				hTextHabit = CreateWindow		//爱好
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | ES_LEFT,
					160, 274, 220, 20,
					hwnd, NULL, hIns, NULL 
				);
				memset(buffT, 0 ,200);
				charToTchar(nowPerson.habit, buffT);
				SetWindowText(hTextHabit, buffT);
				
				hTextTips = CreateWindow		//备注
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE,
					420, 274, 220, 35,
					hwnd, NULL, hIns, NULL 
				);
				memset(buffT, 0 ,200);
				charToTchar(nowPerson.tips, buffT);
				SetWindowText(hTextTips, buffT);
				
				hTextRelation = CreateWindow		//亲密度
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | ES_CENTER | ES_NUMBER,
					392, 382, 20, 20,
					hwnd, NULL, hIns, NULL 
				);
				memset(buff, 0 ,200);
				memset(buffT, 0 ,200);
				sprintf(buff, "%d", nowPerson.relation);
				charToTchar(buff, buffT);
				SetWindowText(hTextRelation, buffT);
				
				//改变字体
				SendMessage(hTextNick, WM_SETFONT, (WPARAM)hFont, TRUE);
				SendMessage(hTextSex, WM_SETFONT, (WPARAM)hFont, TRUE);
				SendMessage(hTextBirth, WM_SETFONT, (WPARAM)hFont, TRUE);
				SendMessage(hTextPhone, WM_SETFONT, (WPARAM)hFont, TRUE);
				SendMessage(hTextQq, WM_SETFONT, (WPARAM)hFont, TRUE);
				SendMessage(hTextHabit, WM_SETFONT, (WPARAM)hFont, TRUE);
				SendMessage(hTextTips, WM_SETFONT, (WPARAM)hFont, TRUE);
				SendMessage(hTextRelation, WM_SETFONT, (WPARAM)hFont, TRUE);
				}
			}
			switch(message)
			{
				case WM_PAINT:
				{
					hdc = BeginPaint(hwnd, &ps);
					bitMap = (HBITMAP)LoadImage(NULL, L"Face/个人信息界面.bmp", IMAGE_BITMAP, WindWidth, WindHighth, LR_LOADFROMFILE);
					hh = CreateCompatibleDC(NULL);
					SelectObject(hh, bitMap);
					BitBlt(hdc, 0, 0, WindWidth, WindHighth, hh, 0, 0, SRCCOPY);
					
					//改变输出字体为宋体    
					ZeroMemory(&logFont, sizeof(LOGFONT));   
					logFont.lfCharSet = GB2312_CHARSET;   
					hFont = CreateFontIndirect(&logFont);   
					SelectObject(hdc, hFont); 
					memset(buffT, 0 ,200);
					charToTchar(nowPerson.name, buffT);
					SetWindowText(hTextName, buffT);
					TextOut(hdc, 220, 113, buffT, wcslen(buffT));
					
					DeleteAllGDI();
					EndPaint(hwnd, &ps);
			
					return 0;
				}
				case WM_LBUTTONUP:
				{	
					
					GetCursorPos(&pt);
					ScreenToClient(hwnd, &pt);   

					//---------------------------------------- 点击“返回”
					if(pt.x > 497 && pt.x < 615 && pt.y > 361 && pt.y < 419)
					{
						proState = ProMain;
						textState = 0;
					}
					//---------------------------------------- 点击“修改”
					else if(pt.x > 153 && pt.x < 222 && pt.y > 361 && pt.y < 418)
					{						
						memset(buffT, 0, 200);
						GetWindowText(hTextNick, buffT, 100);
						tcharToChar(buffT, nowPerson.nick);
						
						memset(buffT, 0, 200);
						GetWindowText(hTextBirth, buffT, 100);
						tcharToChar(buffT, nowPerson.birth);
						
						memset(buffT, 0, 200);
						GetWindowText(hTextSex, buffT, 100);
						tcharToChar(buffT, nowPerson.sex);
						if(strcmp(nowPerson.sex, "男") && strcmp(nowPerson.sex, "女"))
						{
							memset(nowPerson.sex, 0, 10);
							strcpy(nowPerson.sex, "男");
						}
						
						memset(buffT, 0, 200);
						GetWindowText(hTextQq, buffT, 100);
						tcharToChar(buffT, nowPerson.qq);
						
						memset(buffT, 0, 200);
						GetWindowText(hTextPhone, buffT, 100);
						tcharToChar(buffT, nowPerson.phone);
						
						memset(buffT, 0, 200);
						GetWindowText(hTextHabit, buffT, 100);
						tcharToChar(buffT, nowPerson.habit);
						
						memset(buffT, 0, 200);
						GetWindowText(hTextTips, buffT, 100);
						tcharToChar(buffT, nowPerson.tips);
						
						memset(buff, 0, 100);
						memset(buffT, 0, 200);
						GetWindowText(hTextRelation, buffT, 100);
						tcharToChar(buffT, buff);
						sscanf(buff, "%d", &nowPerson.relation);
						
						sendAll('b');		//修改某人属性
						proState = ProMain;
						textState = 0;
					}
					//---------------------------------------- 点击“删除”
					else if(pt.x > 247 && pt.x < 326 && pt.y > 361 && pt.y < 418)
					{
						nextState = ProMain;
						proState = ProSure;
						lastState = ProPersonInformation;
						textState = 0;
					}
					else return 0;
					
					DestroyWindow(hTextName);
					DestroyWindow(hTextNick);
					DestroyWindow(hTextSex);
					DestroyWindow(hTextBirth);
					DestroyWindow(hTextPhone);
					DestroyWindow(hTextQq);
					DestroyWindow(hTextHabit);
					DestroyWindow(hTextTips);
					DestroyWindow(hTextRelation);
					InvalidateRect(hwnd, NULL, TRUE);
					return 0;
					
				}
				case WM_DESTROY:
				{
					sendAll('z');
					closesocket(socketClient);
					WSACleanup();
					PostQuitMessage(0);
					return 0;
				}
			}
			
			break;
		}
		
		//---------------------------------------- 添加人物界面 ----------------------------------------
		case ProAddPerson:
		{
			static HWND hTextName, hTextNick, hTextSex, hTextBirth, hTextPhone, hTextQq, hTextHabit, hTextTips, hTextRelation;
			static int textState = 0;
			
			//仅第一次创建输入框
			if(textState == 0)
			{
				memset(&nowPerson, 0, sizeof(struct person));
				textState = 1;
				//------------------------------ 绘制9个文本框
				{
				hTextName = CreateWindow		//姓名
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | ES_LEFT,
					220, 113, 85, 20,
					hwnd, NULL, hIns, NULL 
				);
				
				hTextNick = CreateWindow		//外号
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | ES_LEFT,
					220, 157, 85, 20,
					hwnd, NULL, hIns, NULL 
				);

				hTextSex = CreateWindow		//性别
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | ES_LEFT,
					220, 199, 20, 20,
					hwnd, NULL, hIns, NULL 
				);
				
				hTextBirth = CreateWindow		//生日
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER,
					392, 113, 70, 20,
					hwnd, NULL, hIns, NULL 
				);

				hTextPhone = CreateWindow		//电话
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER,
					572, 113, 95, 20,
					hwnd, NULL, hIns, NULL 
				);
				
				hTextQq = CreateWindow		//QQ
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER,
					572, 157, 95, 20,
					hwnd, NULL, hIns, NULL 
				);

				hTextHabit = CreateWindow		//爱好
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | ES_LEFT,
					160, 274, 220, 20,
					hwnd, NULL, hIns, NULL 
				);
				
				hTextTips = CreateWindow		//备注
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE,
					420, 274, 220, 35,
					hwnd, NULL, hIns, NULL 
				);
				
				hTextRelation = CreateWindow		//亲密度
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | ES_CENTER | ES_MULTILINE | ES_NUMBER,
					392, 382, 20, 20,
					hwnd, NULL, hIns, NULL 
				);
				
				//改变字体
				SendMessage(hTextName, WM_SETFONT, (WPARAM)hFont, TRUE);
				SendMessage(hTextNick, WM_SETFONT, (WPARAM)hFont, TRUE);
				SendMessage(hTextSex, WM_SETFONT, (WPARAM)hFont, TRUE);
				SendMessage(hTextBirth, WM_SETFONT, (WPARAM)hFont, TRUE);
				SendMessage(hTextPhone, WM_SETFONT, (WPARAM)hFont, TRUE);
				SendMessage(hTextQq, WM_SETFONT, (WPARAM)hFont, TRUE);
				SendMessage(hTextHabit, WM_SETFONT, (WPARAM)hFont, TRUE);
				SendMessage(hTextTips, WM_SETFONT, (WPARAM)hFont, TRUE);
				SendMessage(hTextRelation, WM_SETFONT, (WPARAM)hFont, TRUE);
				}
			}
			switch(message)
			{
				case WM_PAINT:
				{
					hdc = BeginPaint(hwnd, &ps);
					bitMap = (HBITMAP)LoadImage(NULL, L"Face/修改与添加个人信息界面.bmp", IMAGE_BITMAP, WindWidth, WindHighth, LR_LOADFROMFILE);
					hh = CreateCompatibleDC(NULL);
					SelectObject(hh, bitMap);
					BitBlt(hdc, 0, 0, WindWidth, WindHighth, hh, 0, 0, SRCCOPY);
					
					DeleteAllGDI();
					EndPaint(hwnd, &ps);
				
					return 0;
				}
				case WM_LBUTTONUP:
				{	
					
					GetCursorPos(&pt);
					ScreenToClient(hwnd, &pt);   

					//---------------------------------------- 点击“取消”
					if(pt.x > 497 && pt.x < 615 && pt.y > 361 && pt.y < 419)
					{
						proState = ProMain;
						textState = 0;
					}
					//---------------------------------------- 点击“确定”
					else if(pt.x > 183 && pt.x < 301 && pt.y > 361 && pt.y < 418)
					{
						memset(buffT, 0, 200);
						GetWindowText(hTextName, buffT, 100);
						tcharToChar(buffT, nowPerson.name);
						
						memset(buffT, 0, 200);
						GetWindowText(hTextNick, buffT, 100);
						tcharToChar(buffT, nowPerson.nick);
						
						memset(buffT, 0, 200);
						GetWindowText(hTextBirth, buffT, 100);
						tcharToChar(buffT, nowPerson.birth);
						
						memset(buffT, 0, 200);
						GetWindowText(hTextSex, buffT, 100);
						tcharToChar(buffT, nowPerson.sex);
						if(strcmp(nowPerson.sex, "男") && strcmp(nowPerson.sex, "女"))
						{
							memset(nowPerson.sex, 0, 10);
							strcpy(nowPerson.sex, "男");
						}
						
						memset(buffT, 0, 200);
						GetWindowText(hTextQq, buffT, 100);
						tcharToChar(buffT, nowPerson.qq);
						
						memset(buffT, 0, 200);
						GetWindowText(hTextPhone, buffT, 100);
						tcharToChar(buffT, nowPerson.phone);
						
						memset(buffT, 0, 200);
						GetWindowText(hTextHabit, buffT, 100);
						tcharToChar(buffT, nowPerson.habit);
						
						memset(buffT, 0, 200);
						GetWindowText(hTextTips, buffT, 100);
						tcharToChar(buffT, nowPerson.tips);
						
						memset(buff, 0, 100);
						memset(buffT, 0, 200);
						GetWindowText(hTextRelation, buffT, 100);
						tcharToChar(buffT, buff);
						sscanf(buff, "%d", &nowPerson.relation);
						
						for(i = 0; i < listPersonCount; i++)	//判断人物是否重名
						{	
							if(!strcmp(nowPerson.name, listPerson[i].name))
							{
								MessageBox(NULL, TEXT("此分组已有该名字"), TEXT("提示"), NULL);
								return 0;
							}
						}
						if(nowPerson.name[0] == '\0')
						{
							MessageBox(NULL, TEXT("姓名不能为空"), TEXT("提示"), NULL);
							return 0;
						}
						
						sendAll('d');		//添加
						proState = ProMain;
						textState = 0;
					}
					else return 0;
					
					DestroyWindow(hTextName);
					DestroyWindow(hTextNick);
					DestroyWindow(hTextSex);
					DestroyWindow(hTextBirth);
					DestroyWindow(hTextPhone);
					DestroyWindow(hTextQq);
					DestroyWindow(hTextHabit);
					DestroyWindow(hTextTips);
					DestroyWindow(hTextRelation);
					InvalidateRect(hwnd, NULL, TRUE);
					return 0;
					
				}
				case WM_DESTROY:
				{
					sendAll('z');
					closesocket(socketClient);
					WSACleanup();
					PostQuitMessage(0);
					return 0;
				}
			}
			
			break;
		}
		
		//---------------------------------------- 添加分组界面 ----------------------------------------
		case ProAddUnit:
		{
			static HWND hText;
			static int textState = 0;
			
			//仅第一次创建输入框
			if(textState == 0)
			{
				textState = 1;
				hText = CreateWindow		
				( 
					TEXT("edit"), NULL,			
					WS_CHILD | WS_VISIBLE | ES_CENTER,
					355, 228, 90, 17,
					hwnd, NULL, hIns, NULL 
				);
				SendMessage(hText, WM_SETFONT, (WPARAM)hFont, TRUE);
			}
			switch(message)
			{
				case WM_PAINT:
				{
					hdc = BeginPaint(hwnd, &ps);
					bitMap = (HBITMAP)LoadImage(NULL, L"Face/添加分组界面.bmp", IMAGE_BITMAP, WindWidth, WindHighth, LR_LOADFROMFILE);
					hh = CreateCompatibleDC(NULL);
					SelectObject(hh, bitMap);
					BitBlt(hdc, 0, 0, WindWidth, WindHighth, hh, 0, 0, SRCCOPY);
					
					DeleteAllGDI();
					EndPaint(hwnd, &ps);
				
					return 0;
				}
				case WM_LBUTTONUP:
				{	
					
					GetCursorPos(&pt);
					ScreenToClient(hwnd, &pt);   

					//---------------------------------------- 点击“取消”
					if(pt.x > 497 && pt.x < 615 && pt.y > 361 && pt.y < 419)
					{
						proState = ProMain;
						textState = 0;
					}
					//---------------------------------------- 点击“确定”
					else if(pt.x > 183 && pt.x < 301 && pt.y > 361 && pt.y < 418)
					{
						memset(buffT, 0, 200);
						GetWindowText(hText, buffT, 100);
						tcharToChar(buffT, nowUnit);
						if(nowUnit[0] == '\0') 
						{
							MessageBox(NULL, TEXT("单位名不能为空"), TEXT("提示"), NULL);
							return 0;
						}
						for(i = 0; i < unitCount; i++)
						{
							if(!strcmp(nowUnit, unitName[i]))
							{
								MessageBox(NULL, TEXT("单位重名"), TEXT("提示"), NULL);
								return 0;
							}
						}
						
						sendAll('e');		//添加
						proState = ProMain;
						textState = 0;
					}
					else return 0;
					
					DestroyWindow(hText);
					InvalidateRect(hwnd, NULL, TRUE);
					return 0;
					
				}
				case WM_DESTROY:
				{
					sendAll('z');
					closesocket(socketClient);
					WSACleanup();
					PostQuitMessage(0);
					return 0;
				}
			}
			
			break;
		}
		
		return 0;
	}

	//DefWindowProc处理自定义的消息处理函数没有处理到的消息
	return DefWindowProc(hwnd, message, wParam, lParam);		
}





/*-------------------------------------------------- WinMain -------------------------------------------------- */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	hIns = hInstance;
	static TCHAR myName[] = TEXT("MyWindow");
	HWND hwnd;					
	MSG msg;
	WNDCLASS wndclass;			

	wndclass.style = CS_HREDRAW | CS_VREDRAW;							//窗口样式
	wndclass.lpszClassName = myName;									//窗口类名
	wndclass.lpszMenuName = NULL;										//窗口菜单:无
	wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);		//窗口背景颜色
	wndclass.lpfnWndProc = WindProc;									//窗口处理函数
	wndclass.cbWndExtra = 0;											//窗口实例扩展：无
	wndclass.cbClsExtra = 0;											//窗口类扩展：无
	wndclass.hInstance = hInstance;										//窗口实例句柄
	wndclass.hIcon = LoadIcon(NULL, IDI_SHIELD);						//窗口最小化图标：使用缺省图标
	wndclass.hCursor = LoadCursor(NULL,IDC_HAND);						//窗口光标

	RegisterClass( &wndclass );	//注册窗口类

	//创建窗口
	hwnd = CreateWindow			
	(                   
		myName,														//窗口类名
		TEXT("客户端"),												//窗口标题
		WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,		//窗口的风格
		CW_USEDEFAULT,												//窗口初始显示位置x：使用缺省值
		CW_USEDEFAULT,												//窗口初始显示位置y：使用缺省值
		WindWidth,													//窗口的宽度
		WindHighth,													//窗口的高度
		NULL,														//父窗口：无
		NULL,														//子菜单：无
		hInstance,													//该窗口应用程序的实例句柄 
		NULL                       
	);
	hWnd = hwnd;

	RECT rcWindow; 
	RECT rcClient;
	int borderWidth, borderHeight;

	//调整窗口客户端大小到精确像素尺寸
	GetWindowRect(hwnd, &rcWindow);
	GetClientRect(hwnd, &rcClient);
	borderWidth = (rcWindow.right-rcWindow.left)-(rcClient.right-rcClient.left);
	borderHeight = (rcWindow.bottom-rcWindow.top)-(rcClient.bottom-rcClient.top);
	SetWindowPos(hwnd, 0, 0, 0, borderWidth+WindWidth, borderHeight+WindHighth, SWP_NOMOVE | SWP_NOZORDER);

	ShowWindow(hwnd, iCmdShow);		//显示窗口
	UpdateWindow(hwnd);				//更新窗口
	
	//改变窗口字体
	hdc = GetDC(hwnd);
	logFont.lfHeight = MulDiv(12, GetDeviceCaps(hdc, LOGPIXELSY), 72);	//第一个参数是字号大小
	ReleaseDC(hwnd, hdc);
	logFont.lfWidth = 0;
	logFont.lfEscapement = 0;
	logFont.lfOrientation = 0;
	logFont.lfWeight = FW_REGULAR;
	logFont.lfItalic = 0;
	logFont.lfUnderline = 0;
	logFont.lfStrikeOut = 0;
	logFont.lfCharSet = GB2312_CHARSET;
	logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	logFont.lfQuality = PROOF_QUALITY;
	logFont.lfPitchAndFamily = VARIABLE_PITCH  | FF_ROMAN;
	wcscpy(logFont.lfFaceName, TEXT("宋体")); 
	hFont = CreateFontIndirect(&logFont);

	while(GetMessage(&msg, NULL, 0, 0))			//从消息队列中获取消息
	{
		TranslateMessage(&msg);                 //将虚拟键消息转换为字符消息
		DispatchMessage(&msg);                  //分发到回调函数(过程函数)
	}
	return msg.wParam;
}