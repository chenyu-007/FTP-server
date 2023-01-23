# FTP-server
c语言简易FTP协议实现，包含客户端与服务端<br>
## 项目实现<br>
   项目在windows下实现，采用的idea为VS2019。主力语言为C语言，使用了windows下的winsock2库。<br>
   代码逻辑:采用状态机对报文进行解析并执行相应动作
## 项目功能<br>
   实现FTP协议的基本逻辑，实现了对大文件的分包传输.
## 项目演示<br>
> * 服务器运行

<div align=center><img src="https://raw.githubusercontent.com/Eren-cc/FTP-server/main/image/001.png" height="400"/> </div>

> * 客户端运行

<div align=center><img src="https://raw.githubusercontent.com/Eren-cc/FTP-server/main/image/002.png" height="400"/> </div>

> * 传输结果

<div align=center><img src="https://raw.githubusercontent.com/Eren-cc/FTP-server/main/image/003.png" height="400"/> </div>

## 项目待改进<br>
    未实现gui,服务器ip在客户端的代码逻辑中（注释很详细）
    上传大文件速度过慢，可以考虑多线程加快文件传输
    文本文件的utf-8传输后变ansi编码，待解决
    采用明文传输，安全性低。
