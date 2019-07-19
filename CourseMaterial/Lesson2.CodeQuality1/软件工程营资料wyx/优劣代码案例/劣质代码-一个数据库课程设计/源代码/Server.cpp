#include <windows.h>
#include <fstream>
#include <iomanip>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <process.h>
#include <winsock.h>
#include <tchar.h>          
#include <sql.h>     
#include <sqlext.h>     
#include <sqltypes.h>     
#include <odbcss.h> 
#pragma comment(lib,"ws2_32.lib")
using namespace std;

/*-------------------------------------------------- 宏定义 -------------------------------------------------- */
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
char stage = 0;					//阶段 0 - 4
char unit = 0;					//单位 0 - 4
char matchOrNot = 0;			// 0 为取消匹配， 1 为开启匹配
struct person listPerson[1000];	//当前窗口中的人物数据
char listPersonCount = 0;

char nowPersonNumber;			//当前选中的编号 0 - 71
struct person nowPerson;

char unitCount = 0;		//当前有几个单位
char unitName[5][100];		//当前单位名称

int port = 60001;

char username[100];

/*-------------------------------------------------- 建立连接 -------------------------------------------------- */
WSAData WSAData;
struct sockaddr_in sockAddr;
int socketServer;
int sockRecpet;

int Connection()
{	
	if(WSAStartup(MAKEWORD(1,1),&WSAData))
	{
		WSACleanup();
		return 1;
	}

	//设置通信地址
	memset(&sockAddr,0,sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;		
	sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sockAddr.sin_port = htons(port);

	socketServer = socket(AF_INET,SOCK_STREAM,0);

	bind(socketServer,(sockaddr *)&sockAddr,sizeof(sockAddr));
	listen(socketServer,10);

	printf("等待客户端上线...\n");
	sockRecpet = accept(socketServer,NULL,NULL);

	return 0;
}

/*-------------------------------------------------- main -------------------------------------------------- */
SQLHENV henv = SQL_NULL_HENV;     
SQLHDBC hdbc = SQL_NULL_HDBC;     
SQLHSTMT hstmt = SQL_NULL_HSTMT; 
RETCODE retcode;
SQLHENV henv1 = SQL_NULL_HENV;     
SQLHDBC hdbc1 = SQL_NULL_HDBC;     
SQLHSTMT hstmt1 = SQL_NULL_HSTMT; 


int main()
{
	//---------------------------------------- 连接数据库 ----------------------------------------
	//SQL语句  
	//1.连接数据源  
	//1.环境句柄  
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);     
	retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);     
	//2.连接句柄    
	retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);    
	retcode = SQLConnect(hdbc,   (SQLCHAR *)"wyx", SQL_NTS, (SQLCHAR *)"sa", SQL_NTS, (SQLCHAR *)"123456", SQL_NTS);  

	//判断连接是否成功  
	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO)) 
	{       
		printf("连接失败!\n");  
		return 0;
	}   
	printf("已连接数据库 data !\n");
	//2.创建并执行一条或多条SQL语句  
	/* 
	1.分配一个语句句柄(statement handle) 
	2.创建SQL语句 
	3.执行语句 
	4.销毁语句 
	*/  
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);    
	
	//---------------------------------------- 连接数据库完毕 ----------------------------------------

		
	int i, j, k;
	int x, y;
	//--------------------------------------------------
	
	again:;
	if(Connection())
	{
		printf("网络初始化失败\n");
		getch();
		return 0;
	}
	printf("客户端已上线\n");
	
	char recvBuff[100];
	char recvBuff1[100];
	char recvBuff2[100];
	char sendBuff[100];
	char sqlBuff[300];
	char buff[100];
	char buff1[100];
	char buff2[100];
	char buff3[100];
	char buff4[100];
	
	int stage = 6;	//阶段 0 - 4
	int unit = 6;	//单位 0 - 9
	
	while(1)
	{
		memset(recvBuff, 0, 100);
		memset(recvBuff1, 0, 100);
		memset(recvBuff2, 0, 100);
		memset(sendBuff, 0, 100);
		
		recv(sockRecpet, recvBuff, 1, 0);
		switch(recvBuff[0])
		{
			//------------------------------ 创建用户
			case '0':
			{
				printf("-----用户注册\n");
				recv(sockRecpet, recvBuff1, 80, 0);
				recv(sockRecpet, recvBuff2, 80, 0);
				printf("收到用户名 %s 与登录密码 %s\n", recvBuff1, recvBuff2);
				
				//---------- 判断接收数据是否正确
				memset(sqlBuff, 0, 300);
				strcpy(sqlBuff, "insert into UsernamePassword values('");
				strcat(sqlBuff, recvBuff1);
				strcat(sqlBuff, "','");
				strcat(sqlBuff, recvBuff2);
				strcat(sqlBuff, "')");
				printf("sqlBuff:\n%s\n", sqlBuff);
				i = SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
				retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);  
				printf("添加用户 i = SQLExecDirect = %d\n", i);
				if(i != SQL_SUCCESS)
				{
					send(sockRecpet, "0", 1, 0);
					printf("重复注册\n");
					break;
				}
				send(sockRecpet, "1", 1, 0);
				printf("用户已注册并登录\n");
				memset(username, 0, 100);
				strcpy(username, recvBuff1);
				printf("user:%s\n",username);
				break;
			}
			//------------------------------ 用户登录
			case '1':
			{
				printf("-----用户请求登录\n");
				recv(sockRecpet, recvBuff1, 80, 0);
				recv(sockRecpet, recvBuff2, 80, 0);
				printf("收到用户名 %s 与登录密码 %s\n", recvBuff1,recvBuff2);
				
				//---------- 判断接收数据是否正确
				memset(sqlBuff, 0, 300);
				strcpy(sqlBuff, "select * from UsernamePassword");
				printf("sqlBuff:\n%s\n", sqlBuff);
				
				i = SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
				printf("用户登录全部搜索 i = SQLExecDirect = %d\n", i);
				
				SQLBindCol(hstmt, 1, SQL_C_CHAR, buff1, 50, 0);	
				SQLBindCol(hstmt, 2, SQL_C_CHAR, buff2, 50, 0);	
				i = 0;
				do
				{
					memset(buff1, 0, 100);
					memset(buff2, 0, 100);
					retcode = SQLFetch(hstmt);
					
					if(retcode == SQL_NO_DATA)
					{
						printf("DATA OVER!\n");
						break;
					}
					printf("Username %s, Password %s\n", buff1, buff2);
					
					if(!strcmp(recvBuff1, buff1) && !strcmp(recvBuff2, buff2))
					{
						send(sockRecpet, "1", 1, 0);
						printf("用户已经成功登录\n");
						memset(username, 0, 100);
						strcpy(username, recvBuff1);
						i = 1;
						continue;
					}
				}while(1);
				if(i == 0)
				{
					send(sockRecpet, "0", 1, 0);
					printf("登录错误\n");
				}

				retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt); 
				break;
			}
			//------------------------------ 发送主界面数据
			case 'a':
			{
				printf("-----客户端请求列表数据\n");
				recv(sockRecpet, recvBuff1, 1, 0);
				matchOrNot = recvBuff1[0];				//获取是否匹配
				recv(sockRecpet, recvBuff1, 1, 0);
				stage = recvBuff1[0];					//获取当前阶段
				recv(sockRecpet, recvBuff1, 1, 0);
				unit = recvBuff1[0];					//获取当前单位
				printf("matchOrNot = %d, stage = %d, unit = %d\n", matchOrNot, stage, unit);
				
				
				//-------------------- 获取该阶段分组信息
				memset(sqlBuff, 0, 300);
				strcpy(sqlBuff, "select * from Units where Username like '");
				strcat(sqlBuff, username);
				strcat(sqlBuff, "' and stage =");
				memset(buff, 0, 100);
				sprintf(buff, "%d", stage);
				strcat(sqlBuff, buff);
				
				printf("sqlBuff:\n%s\n", sqlBuff);
				SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
				printf("获取全部分组信息 i = SQLExecDirect = %d\n", i);
				
				SQLBindCol(hstmt, 3, SQL_INTEGER, buff, 50, 0);	
				SQLBindCol(hstmt, 4, SQL_C_CHAR, buff1, 50, 0);	
				i = 0;
				memset(unitName, 0, 500);
				do
				{
					memset(buff1, 0, 100);
					retcode = SQLFetch(hstmt);
					if(retcode == SQL_NO_DATA)
					{
						printf("DATA OVER!\n");
						break;
					}
					j = buff[0];
					printf("unit %d, name %s\n", j, buff1);
					i++;
					strcpy(unitName[j], buff1);
					
				}while(1); 
				unitCount = i;
				retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt); 
				
				sendBuff[0] = unitCount;
				send(sockRecpet, sendBuff, 1, 0);		//发送当前阶段有几个单位
				//发送单位名称
				send(sockRecpet, unitName[0], 100, 0);
				send(sockRecpet, unitName[1], 100, 0);
				send(sockRecpet, unitName[2], 100, 0);
				send(sockRecpet, unitName[3], 100, 0);
				send(sockRecpet, unitName[4], 100, 0);
				
				//-------------------- 获取该分组所有成员信息
				memset(sqlBuff, 0, 300);
				strcpy(sqlBuff, "select * from Persons where Username like '");
				strcat(sqlBuff, username);
				strcat(sqlBuff, "'");
	
				printf("sqlBuff:\n%s\n", sqlBuff);
				i = SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
				printf("获取分组中所有人的信息 i = SQLExecDirect = %d\n", i);
				
				SQLBindCol(hstmt, 2, SQL_C_CHAR, nowPerson.name, 50, 0);	
				SQLBindCol(hstmt, 3, SQL_C_CHAR, nowPerson.nick, 50, 0);	
				SQLBindCol(hstmt, 4, SQL_C_CHAR, nowPerson.sex, 50, 0);	
				SQLBindCol(hstmt, 5, SQL_C_CHAR, nowPerson.birth, 50, 0);	
				SQLBindCol(hstmt, 6, SQL_C_CHAR, nowPerson.unit, 26, 0);	
				SQLBindCol(hstmt, 7, SQL_C_CHAR, nowPerson.habit, 100, 0);	
				SQLBindCol(hstmt, 8, SQL_C_CHAR, nowPerson.qq, 50, 0);	
				SQLBindCol(hstmt, 9, SQL_C_CHAR, nowPerson.phone, 50, 0);	
				SQLBindCol(hstmt, 10, SQL_C_CHAR, nowPerson.tips, 200, 0);	
				SQLBindCol(hstmt, 11, SQL_INTEGER, buff, 50, 0);	
				
				i = 0;
				do
				{
					memset(&nowPerson, 0, sizeof(struct person));
					retcode = SQLFetch(hstmt);
					if(retcode == SQL_NO_DATA)
					{
						printf("DATA OVER!\n");
						break;
					}
					if(nowPerson.unit[stage*5+unit] == '1')
					{
						nowPerson.relation = buff[0];
						listPerson[i++] = nowPerson;
						printf("nowPerson.unit:%s\n", nowPerson.unit);
						printf("name %s\n", nowPerson.name);
					}

				}while(1);
				listPersonCount = i;
				retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt); 
				
				sendBuff[0] = listPersonCount;
				send(sockRecpet, sendBuff, 1, 0);		//发送当前单位有几个人员
				
				//发送当前单位人员数据
				for(i = 0; i < listPersonCount; i++)
				{
					send(sockRecpet, listPerson[i].name, 100, 0);	//姓名					
					send(sockRecpet, listPerson[i].nick, 100, 0);	//外号					
					send(sockRecpet, listPerson[i].sex, 100, 0);	//性别					
					send(sockRecpet, listPerson[i].birth, 100, 0);	//生日			
					send(sockRecpet, listPerson[i].unit, 26, 0);	//分组			
					send(sockRecpet, listPerson[i].habit, 100, 0);	//爱好				
					memset(sendBuff, 0 ,100);
					sprintf(sendBuff, "%d", listPerson[i].relation);
					send(sockRecpet, sendBuff, 4, 0);				//关系					
					send(sockRecpet, listPerson[i].qq, 100, 0);		//QQ
					send(sockRecpet, listPerson[i].phone, 100, 0);	//电话
					send(sockRecpet, listPerson[i].tips, 200, 0);	//备注
					
					if(matchOrNot == 1)
					{
						//-------------------- 查找匹配
						memset(sqlBuff, 0, 300);
						strcpy(sqlBuff, "select * from Persons where Username like '");
						strcat(sqlBuff, listPerson[i].name);
						strcat(sqlBuff, "' and name like '");
						strcat(sqlBuff, username);
						strcat(sqlBuff, "'");

						printf("sqlBuff:\n%s\n", sqlBuff);
						k = SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
						printf("获取匹配信息 i = SQLExecDirect = %d\n", k);
						
						memset(buff, 0, 100);
						SQLBindCol(hstmt, 11, SQL_INTEGER, buff, 50, 0);
						retcode = SQLFetch(hstmt);
						retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt); 
						
						j = buff[0];
						if(j > 5 && listPerson[i].relation > 5) listPerson[i].match = 1;
						else listPerson[i].match = 0;
						
						sendBuff[0] = listPerson[i].match;
						send(sockRecpet, sendBuff, 1, 0);				//是否匹配上
					}
				}
				
				break;
			}
			
			//------------------------------ 修改人物属性
			case 'b':
			{
				recv(sockRecpet, nowPerson.name, 50, 0);
				recv(sockRecpet, nowPerson.nick, 50, 0);
				recv(sockRecpet, nowPerson.sex, 50, 0);
				recv(sockRecpet, nowPerson.birth, 50, 0);
				//recv(sockRecpet, nowPerson.unit, 25, 0);
				recv(sockRecpet, nowPerson.habit, 100, 0);
				recv(sockRecpet, nowPerson.qq, 50, 0);
				recv(sockRecpet, nowPerson.phone, 50, 0);
				recv(sockRecpet, nowPerson.tips, 200, 0);
				memset(recvBuff1, 0, 100);
				recv(sockRecpet, recvBuff1, 1, 0);
				nowPerson.relation = recvBuff1[0]-10;
				
				printf("修改该人属性: %s\n",nowPerson.name);
				
				memset(sqlBuff, 0, 300);
				strcpy(sqlBuff, "update Persons set nick ='");
				strcat(sqlBuff, nowPerson.nick);
				strcat(sqlBuff, "', sex ='");
				strcat(sqlBuff, nowPerson.sex);
				strcat(sqlBuff, "', birth ='");
				strcat(sqlBuff, nowPerson.birth);
				strcat(sqlBuff, "', habit ='");
				strcat(sqlBuff, nowPerson.habit);
				strcat(sqlBuff, "', qq ='");
				strcat(sqlBuff, nowPerson.qq);
				strcat(sqlBuff, "', phone ='");
				strcat(sqlBuff, nowPerson.phone);
				strcat(sqlBuff, "', tips ='");
				strcat(sqlBuff, nowPerson.tips);
				strcat(sqlBuff, "', relation =");
				memset(buff, 0, 100);
				sprintf(buff, "%d", nowPerson.relation);
				strcat(sqlBuff, buff);
				strcat(sqlBuff, " where Username ='");
				strcat(sqlBuff, username);
				strcat(sqlBuff, "' and name ='");
				strcat(sqlBuff, nowPerson.name);
				strcat(sqlBuff, "'");

				printf("sqlBuff:\n%s\n", sqlBuff);
				i = SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
				retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);  
				
				
				break;
			}
			
			//------------------------------ 删除某人在某单位的数据
			case 'c':
			{
				recv(sockRecpet, nowPerson.name, 50, 0);
				memset(recvBuff, 0, 100);
				recv(sockRecpet, recvBuff, 1, 0);
				stage = recvBuff[0];
				memset(recvBuff, 0, 100);
				recv(sockRecpet, recvBuff, 1, 0);
				unit = recvBuff[0];
				memset(nowPerson.unit, 0, 26);
				recv(sockRecpet, nowPerson.unit, 26, 0);
				nowPerson.unit[stage*5+unit] = '0';
				printf("-----删除 %s 后unit[26]: %s\n", nowPerson.name, nowPerson.unit);
				
				//如果没有删完
				if(strcmp(nowPerson.unit, "0000000000000000000000000"))
				{						  
					memset(sqlBuff, 0, 300);
					strcpy(sqlBuff, "update Persons set unit ='");
					strcat(sqlBuff, nowPerson.unit);
					strcat(sqlBuff, "' where Username ='");
					strcat(sqlBuff, username);
					strcat(sqlBuff, "' and name ='");
					strcat(sqlBuff, nowPerson.name);
					strcat(sqlBuff, "'");
				
					printf("sqlBuff:\n%s\n", sqlBuff);
					SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
					printf("不彻底删除个人 i = SQLExecDirect = %d\n", i);
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt); 
				}
				//彻底删除
				else
				{
					memset(sqlBuff, 0, 300);
					strcpy(sqlBuff, "delete from Persons where Username ='");
					strcat(sqlBuff, username);
					strcat(sqlBuff, "' and name ='");
					strcat(sqlBuff, nowPerson.name);
					strcat(sqlBuff, "'");

					printf("sqlBuff:\n%s\n", sqlBuff);
					SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
					printf("彻底删除个人 i = SQLExecDirect = %d\n", i);
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt); 
				}
				
				break;
			}
			//------------------------------ 添加单位中的某人
			case 'd':
			{
				recv(sockRecpet, nowPerson.name, 50, 0);
				recv(sockRecpet, nowPerson.nick, 50, 0);
				recv(sockRecpet, nowPerson.sex, 50, 0);
				recv(sockRecpet, nowPerson.birth, 50, 0);
				//recv(sockRecpet, nowPerson.unit, 25, 0);
				recv(sockRecpet, nowPerson.habit, 100, 0);
				recv(sockRecpet, nowPerson.qq, 50, 0);
				recv(sockRecpet, nowPerson.phone, 50, 0);
				recv(sockRecpet, nowPerson.tips, 200, 0);
				memset(recvBuff1, 0, 100);
				recv(sockRecpet, recvBuff1, 1, 0);
				nowPerson.relation = recvBuff1[0]-10;
				
				printf("-----添加联系人 %s\n",nowPerson.name);
				
				//---------- 判断联系人是否已经在其它分组存在
				memset(sqlBuff, 0, 300);
				strcpy(sqlBuff, "select * from Persons where Username like '");
				strcat(sqlBuff, username);
				strcat(sqlBuff, "'");
				
				printf("sqlBuff:\n%s\n", sqlBuff);
				SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
				printf("用户登录全部搜索 i = SQLExecDirect = %d\n", i);
				
				SQLBindCol(hstmt, 2, SQL_C_CHAR, buff1, 50, 0);	
				SQLBindCol(hstmt, 6, SQL_C_CHAR, buff2, 50, 0);	
				i = 0;
				do
				{
					memset(buff1, 0, 100);
					memset(buff2, 0, 100);
					retcode = SQLFetch(hstmt);
					if(retcode == SQL_NO_DATA)
					{
						printf("DATA OVER!\n");
						break;
					}
					printf("name %s\n", buff1);
					
					if(!strcmp(nowPerson.name, buff1))	//发现已经有这个人了
					{
						i = 1;
						strcpy(buff, buff2);
					}
					
				}while(1);
				retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt); 
				
				//将已经存在的人多给出一个分组
				if(i == 1)
				{
					printf("发现重复人员\n");
					buff[stage*5+unit] = '1';
					send(sockRecpet, "1", 1, 0);
					
					memset(sqlBuff, 0, 300);
					strcpy(sqlBuff, "update Persons set unit = '");
					strcat(sqlBuff, buff);
					strcat(sqlBuff, "' where Username like '");
					strcat(sqlBuff, username);
					strcat(sqlBuff, "' and name like '");
					strcat(sqlBuff, nowPerson.name);
					strcat(sqlBuff, "'");
					
					printf("sqlBuff:\n%s\n", sqlBuff);
					SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
					printf("更新人物的unit列 i = SQLExecDirect = %d\n", i);
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt); 
				}
				//没有重名的人，插入新的一行
				else		
				{
					send(sockRecpet, "0", 1, 0);
					printf("插入新的人物\n");
					memset(sqlBuff, 0, 300);
					strcpy(sqlBuff, "insert into Persons values('");
					strcat(sqlBuff, username);
					strcat(sqlBuff, "','");
					strcat(sqlBuff, nowPerson.name);
					strcat(sqlBuff, "','");
					strcat(sqlBuff, nowPerson.nick);
					strcat(sqlBuff, "','");
					strcat(sqlBuff, nowPerson.sex);
					strcat(sqlBuff, "','");
					strcat(sqlBuff, nowPerson.birth);
					strcat(sqlBuff, "','");
					memset(buff, 0, 100);
					strcpy(buff, "0000000000000000000000000");
					buff[stage*5+unit] = '1';
					strcat(sqlBuff, buff);
					strcat(sqlBuff, "','");
					strcat(sqlBuff, nowPerson.habit);
					strcat(sqlBuff, "','");
					strcat(sqlBuff, nowPerson.qq);
					strcat(sqlBuff, "','");
					strcat(sqlBuff, nowPerson.phone);
					strcat(sqlBuff, "','");
					strcat(sqlBuff, nowPerson.tips);
					strcat(sqlBuff, "',");
					memset(buff, 0, 100);
					sprintf(buff, "%d", nowPerson.relation);
					strcat(sqlBuff, buff);
					strcat(sqlBuff, ")");

					printf("sqlBuff:\n%s\n", sqlBuff);
					i = SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);  
					
					printf("插入新的人物 i = SQLExecDirect = %d\n", i);
				}
					
				break;
			}
			//------------------------------ 添加分组
			case 'e':
			{
				
				recv(sockRecpet, recvBuff1, 80, 0);
				printf("-----添加分组 %s\n", recvBuff1);

				memset(sqlBuff, 0, 300);
				strcpy(sqlBuff, "insert into Units values('");
				strcat(sqlBuff, username);
				strcat(sqlBuff, "',");
				memset(buff, 0 ,100);
				buff[0] = stage+'0';
				strcat(sqlBuff, buff);
				strcat(sqlBuff, ",");
				memset(buff, 0 ,100);
				buff[0] = unitCount+'0';
				unitCount++;
				strcat(sqlBuff, buff);
				strcat(sqlBuff, ",'");
				strcat(sqlBuff, recvBuff1);
				strcat(sqlBuff, "')");

				printf("sqlBuff:\n%s\n", sqlBuff);
				SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
				retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt); 
				printf("插入分组 i = SQLExecDirect = %d\n", i);

				break;
			}
			
			//------------------------------ 删除分组前的确认
			case 'f':
			{
				i = 0;
				printf("-----准备删除分组 stage = %d, unit = %d\n", stage, unit);
				
				memset(sqlBuff, 0, 300);
				strcpy(sqlBuff, "select * from Persons where Username = '");
				strcat(sqlBuff, username);
				strcat(sqlBuff, "'");
				
				printf("sqlBuff:\n%s\n", sqlBuff);
				SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
				printf("删除分组前的搜索 i = SQLExecDirect = %d\n", i);

				SQLBindCol(hstmt, 6, SQL_C_CHAR, nowPerson.unit, 26, 0);	
				i = 0;
				do
				{
					memset(nowPerson.unit, 0, 26);
					retcode = SQLFetch(hstmt);
					if(retcode == SQL_NO_DATA)
					{
						printf("DATA OVER!\n");
						break;
					}
					printf("nowPerson.unit:%s\n", nowPerson.unit);
					nowPerson.unit[stage*5+unit] = '0';	
					if(!strcmp(nowPerson.unit, "0000000000000000000000000"))	//删完后一个都不剩
					i++;
					
				}while(1);
				retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt); 
				
				memset(buff, 0, 100);
				buff[0] = i;
				send(sockRecpet, buff, 1, 0);
				break;
			}
			
			//------------------------------ 删除分组
			case 'g':
			{
				printf("-----删除分组 stage = %d, unit = %d\n", stage, unit);
				
				//删除Units表中的一行
				memset(sqlBuff, 0, 300);
				strcpy(sqlBuff, "delete from Units where Username = '");
				strcat(sqlBuff, username);
				strcat(sqlBuff, "' and stage = ");
				memset(buff, 0, 100);
				buff[0] = stage+'0';
				strcat(sqlBuff, buff);
				strcat(sqlBuff, " and unit = ");
				memset(buff, 0, 100);
				buff[0] = unit+'0';
				strcat(sqlBuff, buff);
				
				printf("sqlBuff:\n%s\n", sqlBuff);
				i = SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
				printf("删除分组 i = SQLExecDirect = %d\n", i);
				retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
				
				//调整Units表
				memset(sqlBuff, 0, 300);
				strcpy(sqlBuff, "update Units set unit = unit-1 where Username = '");
				strcat(sqlBuff, username);
				strcat(sqlBuff, "' and stage = ");
				memset(buff, 0, 100);
				buff[0] = stage+'0';
				strcat(sqlBuff, buff);
				strcat(sqlBuff, " and unit > ");
				memset(buff, 0, 100);
				buff[0] = unit+'0';
				strcat(sqlBuff, buff);

				printf("sqlBuff:\n%s\n", sqlBuff);
				SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
				printf("调整分组 i = SQLExecDirect = %d\n", i);
				retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
				
				//获取所有Username的name以及unit[26]
				memset(sqlBuff, 0, 300);
				strcpy(sqlBuff, "select * from Persons where Username ='");
				strcat(sqlBuff, username);
				strcat(sqlBuff, "'");
				
				printf("sqlBuff:\n%s\n", sqlBuff);
				SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
				printf("调整Persons前的查找 i = SQLExecDirect = %d\n", i);

				SQLBindCol(hstmt, 2, SQL_C_CHAR, nowPerson.name, 50, 0);
				SQLBindCol(hstmt, 6, SQL_C_CHAR, nowPerson.unit, 26, 0);
				
				i = 0;
				do
				{
					memset(nowPerson.name, 0, 50);
					retcode = SQLFetch(hstmt);
					if(retcode == SQL_NO_DATA)
					{
						printf("DATA OVER!\n");
						break;
					}
					printf("name: %s unit: %s\n", nowPerson.name, nowPerson.unit);
					listPerson[i++] = nowPerson;

				}while(1);
				k = i;
				retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
				
				//调整所有的unit
				printf("调整后\n");
				for(i = 0; i < k; i++)
				{
					j = stage*5;
					if(unit == 0)
					{
						listPerson[i].unit[j+0] = listPerson[i].unit[j+1];
						listPerson[i].unit[j+1] = listPerson[i].unit[j+2];
						listPerson[i].unit[j+2] = listPerson[i].unit[j+3];
						listPerson[i].unit[j+3] = listPerson[i].unit[j+4];
						listPerson[i].unit[j+4] = '0';
					}
					else if(unit == 1)
					{
						listPerson[i].unit[j+1] = listPerson[i].unit[j+2];
						listPerson[i].unit[j+2] = listPerson[i].unit[j+3];
						listPerson[i].unit[j+3] = listPerson[i].unit[j+4];
						listPerson[i].unit[j+4] = '0';
					}
					else if(unit == 2)
					{
						listPerson[i].unit[j+2] = listPerson[i].unit[j+3];
						listPerson[i].unit[j+3] = listPerson[i].unit[j+4];
						listPerson[i].unit[j+4] = '0';
					}
					else if(unit == 3)
					{
						listPerson[i].unit[j+3] = listPerson[i].unit[j+4];
						listPerson[i].unit[j+4] = '0';
					}
					else if(unit == 4)
					{
						listPerson[i].unit[j+4] = '0';
					}
					printf("name: %s unit: %s\n", listPerson[i].name, listPerson[i].unit);
				}
				
				//重新插入unit
				for(i = 0; i < k; i++)
				{
					memset(sqlBuff, 0, 300);
					strcpy(sqlBuff, "update Persons set unit ='");
					strcat(sqlBuff, listPerson[i].unit);
					strcat(sqlBuff, "' where Username ='");
					strcat(sqlBuff, username);
					strcat(sqlBuff, "' and name ='");
					strcat(sqlBuff, listPerson[i].name);
					strcat(sqlBuff, "'");

					printf("sqlBuff:\n%s\n", sqlBuff);
					SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
					printf("调整Persons前的查找 i = SQLExecDirect = %d\n", i);

					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
				}
				
				//删除多余的Persons
				memset(sqlBuff, 0, 300);
				strcpy(sqlBuff, "delete from Persons where Username ='");
				strcat(sqlBuff, username);
				strcat(sqlBuff, "' and unit ='0000000000000000000000000'");

				printf("sqlBuff:\n%s\n", sqlBuff);
				SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
				printf("彻底删除个人 i = SQLExecDirect = %d\n", i);
				retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt); 
				
				break;
			}
			
			//------------------------------ 查找
			case 'h':
			case 'i':
			case 'j':
			{
				memset(buff, 0, 100);
				recv(sockRecpet, buff, 50, 0);
				
				//-------------------- 获取查询中所有的信息
				memset(sqlBuff, 0, 300);
				strcpy(sqlBuff, "select * from Persons where Username like '");
				strcat(sqlBuff, username);
				
				if(recvBuff[0] == 'h')			//找名字
				{
					printf("-----查询名字: %s\n", buff);
					strcat(sqlBuff, "' and name like '");
					strcat(sqlBuff, buff);
					strcat(sqlBuff, "'");
				}
				else if(recvBuff[0] == 'i')		//找爱好
				{
					printf("-----查询爱好: %s\n", buff);
					strcat(sqlBuff, "' and habit like '%");
					strcat(sqlBuff, buff);
					strcat(sqlBuff, "%'");
				}
				else if(recvBuff[0] == 'j')		//找密友
				{	
					printf("-----查询关系: %s\n", buff);
					strcat(sqlBuff, "' and relation = ");
					if(buff[1] != 0) strcpy(buff, "10");
					strcat(sqlBuff, buff);
				}

				printf("sqlBuff:\n%s\n", sqlBuff);
				i = SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
				printf("获取查询信息 i = SQLExecDirect = %d\n", i);

				SQLBindCol(hstmt, 2, SQL_C_CHAR, nowPerson.name, 50, 0);	
				SQLBindCol(hstmt, 3, SQL_C_CHAR, nowPerson.nick, 50, 0);	
				SQLBindCol(hstmt, 4, SQL_C_CHAR, nowPerson.sex, 50, 0);	
				SQLBindCol(hstmt, 5, SQL_C_CHAR, nowPerson.birth, 50, 0);	
				SQLBindCol(hstmt, 7, SQL_C_CHAR, nowPerson.habit, 100, 0);	
				SQLBindCol(hstmt, 8, SQL_C_CHAR, nowPerson.qq, 50, 0);	
				SQLBindCol(hstmt, 9, SQL_C_CHAR, nowPerson.phone, 50, 0);	
				SQLBindCol(hstmt, 10, SQL_C_CHAR, nowPerson.tips, 200, 0);	
				SQLBindCol(hstmt, 11, SQL_INTEGER, buff, 50, 0);	

				i = 0;
				do
				{
					memset(&nowPerson, 0, sizeof(struct person));
					retcode = SQLFetch(hstmt);
					if(retcode == SQL_NO_DATA)
					{
						printf("DATA OVER!\n");
						break;
					}
					nowPerson.relation = buff[0];
					listPerson[i++] = nowPerson;
					printf("name %s\n", nowPerson.name);

				}while(1);
				listPersonCount = i;
				retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt); 

				sendBuff[0] = listPersonCount;
				send(sockRecpet, sendBuff, 1, 0);		//发送当前单位有几个人员

				//发送人员数据
				for(i = 0; i < listPersonCount; i++)
				{
					send(sockRecpet, listPerson[i].name, 100, 0);	//姓名					
					send(sockRecpet, listPerson[i].nick, 100, 0);	//外号					
					send(sockRecpet, listPerson[i].sex, 100, 0);	//性别					
					send(sockRecpet, listPerson[i].birth, 100, 0);	//生日						
					send(sockRecpet, listPerson[i].habit, 100, 0);	//爱好				
					memset(sendBuff, 0 ,100);
					sprintf(sendBuff, "%d", listPerson[i].relation);
					send(sockRecpet, sendBuff, 4, 0);				//关系					
					send(sockRecpet, listPerson[i].qq, 100, 0);		//QQ
					send(sockRecpet, listPerson[i].phone, 100, 0);	//电话
					send(sockRecpet, listPerson[i].tips, 200, 0);	//备注
				}
				
				break;
			}
			
			//------------------------------ 管理员——查找所有用户
			case 'o':
			{
				printf("/////查找所有用户\n");
				memset(sqlBuff, 0, 300);
				strcpy(sqlBuff, "select * from UsernamePassword");

				printf("sqlBuff:\n%s\n", sqlBuff);
				i = SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
				printf("获取查询信息 i = SQLExecDirect = %d\n", i);

				SQLBindCol(hstmt, 1, SQL_C_CHAR, buff, 50, 0);	

				i = 0;
				do
				{
					memset(buff, 0, 100);
					retcode = SQLFetch(hstmt);
					if(retcode == SQL_NO_DATA)
					{
						printf("DATA OVER!\n");
						break;
					}
					strcpy(listPerson[i++].name, buff);
					printf("Username %s\n", buff);

				}while(1);
				listPersonCount = i;
				retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt); 

				sendBuff[0] = listPersonCount;
				send(sockRecpet, sendBuff, 1, 0);		//发送当前单位有几个人员

				//发送人员数据
				for(i = 0; i < listPersonCount; i++)
				{
					send(sockRecpet, listPerson[i].name, 100, 0);	//用户名					
				}
				
				break;
			}
			
			//------------------------------ 管理员——删除用户
			case 'p':
			{	
				recv(sockRecpet, username, 100, 0);
				printf("/////删除用户: %s\n", username);
			
				memset(sqlBuff, 0, 300);
				strcpy(sqlBuff, "delete from UsernamePassword where Username = '");
				strcat(sqlBuff, username);
				strcat(sqlBuff, "'");
				printf("sqlBuff:\n%s\n", sqlBuff);
				i = SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
				printf("删除UsernamePassword表中用户 i = SQLExecDirect = %d\n", i);
				
				memset(sqlBuff, 0, 300);
				strcpy(sqlBuff, "delete from Persons where Username = '");
				strcat(sqlBuff, username);
				strcat(sqlBuff, "'");
				printf("sqlBuff:\n%s\n", sqlBuff);
				i = SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
				printf("删除Persons表中用户 i = SQLExecDirect = %d\n", i);
				
				memset(sqlBuff, 0, 300);
				strcpy(sqlBuff, "delete from Units where Username = '");
				strcat(sqlBuff, username);
				strcat(sqlBuff, "'");
				printf("sqlBuff:\n%s\n", sqlBuff);
				i = SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
				printf("删除Units表中用户 i = SQLExecDirect = %d\n", i);

				break;
			}
			
			//------------------------------ 管理员——更改用户密码
			case 'q':
			{	
				recv(sockRecpet, username, 100, 0);
				recv(sockRecpet, buff, 100, 0);
				printf("/////用户: %s 更改密码为: %s\n", username, buff);
				
				memset(sqlBuff, 0, 300);
				strcpy(sqlBuff, "update UsernamePassword set Password = '");
				strcat(sqlBuff, buff);
				strcat(sqlBuff, "' where Username = '");
				strcat(sqlBuff, username);
				strcat(sqlBuff, "'");
				printf("sqlBuff:\n%s\n", sqlBuff);
				i = SQLExecDirect(hstmt, (SQLCHAR *)sqlBuff, strlen(sqlBuff));
				printf("更改密码 i = SQLExecDirect = %d\n", i);
				
				break;
			}
			
			//------------------------------ 断开连接
			case 'z':
			{
				printf("断开连接\n");
				closesocket(socketServer);
				WSACleanup();
				goto again;
				break;
			}
		}
	}
	
	
	//释放语句句柄  
	SQLCloseCursor(hstmt);  
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);  
	//3.断开数据源  
	/* 
	1.断开与数据源的连接. 
	2.释放连接句柄. 
	3.释放环境句柄 (如果不再需要在这个环境中作更多连接) 
	*/  
	SQLDisconnect(hdbc);      
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);     
	SQLFreeHandle(SQL_HANDLE_ENV, henv);   
	
	getch();
	return 0;
}