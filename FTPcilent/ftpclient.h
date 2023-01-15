#pragma once
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#include<stdbool.h>//c布尔类型库
#define SEVERPORT 1234 //服务器端口号
#define PACKET_SIZE (1024 - sizeof(int)*3)//包的大小
enum MSGTAG
{
	MSG_FILENAME = 1,//文件名
	MSG_FILESIZE = 2,//文件大小，
	MSG_READY_READ = 3,//准备接收
	MSG_SENDFILE = 4,//发送
	MSG_SUCCESSED = 5,//传输完成
	MSG_OPEN_FAILED = 6//文件打开失败
};
#pragma pack(1)
struct MsgHeader //封装消息头
{
	enum MSGTAG msgID;//当前消息标记 4byte
	union MyUnion
	{
		struct
		{
			char filename[256];//文件名 256 byte
			int filesize;//文件大小 4byte
		}fileinfo;//文件信息 
		struct
		{
			int nsize;//包的大小
			int nstart;//包的起始
			char buf[PACKET_SIZE];

		}packet;
	};
};
#pragma pack()
//socket初始化api
bool Socketinit();
//socket关闭
bool Socketclose();
//监听客户端的TCP链接
void connectToserver();
//消息处理
bool processMessage(SOCKET s);
void downloadFilename(SOCKET s);
void readyread(SOCKET s,struct MsgHeader* pmsg);
//写文件
bool writeFile(SOCKET s, struct MsgHeader* pmsg);