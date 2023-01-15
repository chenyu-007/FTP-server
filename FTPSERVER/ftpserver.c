#include<stdio.h>
#include"ftpserver.h"
char g_recvbuf[1024];
int g_filesize;//文件大小
char* g_filebuf;//存储文件内容
int main()
{
    Socketinit();

    listenToclient();

    Socketclose();
 
	return 0;
}
//初始化socket api库（windows独有操作）
bool Socketinit()
{
    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
    {
        printf("WSAStartup failed :%d\n", WSAGetLastError());
        return false;
    }
    return true;
}
//关闭socket API库
bool Socketclose()
{
    if (WSACleanup() != 0)
    {
        printf("WSACleanup failed :%d\n", WSAGetLastError());
        return false;
    }
    return true;
}
//监听客户端的TCP链接
void listenToclient()
{
    //监听socket套接字创建（四元组）
    SOCKET serfd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serfd == INVALID_SOCKET)
    {
        printf("socket creation failed:%d\n",WSAGetLastError());
        return;
    }
    struct sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(SEVERPORT);//本地字节序转网络字节序
    serAddr.sin_addr.S_un.S_addr = ADDR_ANY;//监听本机所有网卡 一个网卡=一个mac地址
    if (bind(serfd, (struct sockaddr*)&serAddr, sizeof(serAddr)) != 0)//绑定监听ocket的本地二元组
    {
        printf("socket bind failed:%d\n", WSAGetLastError());
        return;
    }
    if (listen(serfd, 5)!=0)//把监听队列设置为5个
    {
        printf("socket listen failed:%d\n", WSAGetLastError());
        return;
    }
    struct sockaddr_in cliAddr;
    int len = sizeof(cliAddr);
    SOCKET clifd=accept(serfd, (struct sockaddr*)&cliAddr,&len);//accept取握完手的TCP链接,返回建立好连接的新socket clifd
    if(clifd == INVALID_SOCKET)
    {
        printf("socket accpet failed:%d\n", WSAGetLastError());
        return;
    }
    
    while (processMessage(clifd))
    {
    }
}
//消息处理
bool processMessage(SOCKET clifd)
{

    int res=recv(clifd,g_recvbuf,1024,0);
    if (res <= 0)
    {
        printf("客户端下线：%d", WSAGetLastError());
        return false;
    }
    
    struct  MsgHeader* msg = (struct MsgHeader*)g_recvbuf;//字节流强转结构体
    switch (msg->msgID)
    {
        case MSG_FILENAME:
            readfile(clifd, msg);
        break;
        case MSG_SENDFILE:
            sendfile(clifd, msg);
        break;
        case MSG_SUCCESSED:
            printf("MSG_SUCCESSED\n");
        break;
    }
    //printf("%s\n", g_recvbuf);
    return true;
}
/*
* 1.客户端请求下载文件=》将想要下载文件名发送给服务器
* 2.服务器接收客户端发送过来的文件名-》根据文件名找到文件，将文件大小发送给客户端
* 3.客户端接收到文件大小-》准备开始接收，开辟内存 准备完成并告诉服务器可以接收
* 4.服务器接收并开始发送-》发送文件数据
* 5.客户端开始接 收数据并存储-》接收完成，告诉服务器接收完成
* 6.关闭连接
*/
bool readfile(SOCKET clifd, struct MsgHeader* msg)
{
    FILE* pread = fopen(msg->fileinfo.filename, "rb");
    if (pread == NULL)
    {
        printf("找不到【%s】文件", msg->fileinfo.filename);
        struct MsgHeader pmsg;pmsg.msgID = MSG_OPEN_FAILED;
        if (send(clifd, (char*)&msg, sizeof(struct MsgHeader), 0) == SOCKET_ERROR)
        {
            printf("send failed:%d\n", WSAGetLastError());
        }
        return false;
    }
    //获取文件大小
    fseek(pread, 0, SEEK_END);
    g_filesize = ftell(pread);
    fseek(pread, 0, SEEK_SET);
    struct MsgHeader pmsg; pmsg.msgID = MSG_FILESIZE; pmsg.fileinfo.filesize = g_filesize;
    //c:\users\team\desktop\sese.jpg tfname=see text=.jpg
    char tfname[200] = { 0 }, text[100];
    _splitpath(msg->fileinfo.filename,NULL,NULL,tfname,text);//c语言路径分割
    strcat(tfname,text);
    strcpy(pmsg.fileinfo.filename,tfname);
    printf("readfile filename:%s filesize:%d\n",pmsg.fileinfo.filename,pmsg.fileinfo.filesize);
    send(clifd, (char*)&pmsg, sizeof(struct MsgHeader),0);//报文发送

    //读取磁盘文件内容到内存缓冲区g_filebuf
    g_filebuf = calloc(g_filesize + 1, sizeof(char));
    if (g_filebuf == NULL)
    {
        printf("存储文件的内存不足\n");
        return false;
    }
    fread(g_filebuf,sizeof(char),g_filesize,pread);
    g_filebuf[g_filesize] = '\0';//此举脱裤子放屁
    fclose(pread);
}
bool sendfile(SOCKET s, struct MsgHeader* msg)
{
    struct MsgHeader pmsg;
    pmsg.msgID = MSG_READY_READ;
    //对于一次传不完的分包传
    for (size_t i=0;i<g_filesize;i = i+PACKET_SIZE)
    {
        pmsg.packet.nstart = i;
        //判断是否到文件最后一个包
        if (i + PACKET_SIZE+1>g_filesize)
        {
            pmsg.packet.nsize = g_filesize - i;
        }
        else
        {
            pmsg.packet.nsize = PACKET_SIZE;
        }
        memcpy(pmsg.packet.buf, g_filebuf+msg->packet.nstart,msg->packet.nsize);

        if (SOCKET_ERROR == send(s, (char*)&pmsg, sizeof(struct MsgHeader), 0))
        {
            printf("文件发送失败:%d\n", WSAGetLastError());
        }
    }
    return true;
}