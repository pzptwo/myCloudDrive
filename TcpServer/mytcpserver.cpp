#include "mytcpserver.h"

mytcpServer::mytcpServer() {}

mytcpServer &mytcpServer::getInstance()
{
    static mytcpServer instance;    //一定要有static
    return instance;
}
void mytcpServer::incomingConnection(qintptr socketDescriptor)
{
    //这里只有新客户端连接才调用这个，后面的信息交互在别的地方，所以这里区分是哪个socket！！！
    //把自定义的加入进来，用链表接受

    qDebug()<<"new client connected";
    myTcpSocket *pTcpSocket=new myTcpSocket;
//     socketDescriptor 是操作系统给的 客户端唯一 ID
//         你 new 出来的 myTcpSocket 一开始是空的、不知道管谁
//             调用 setSocketDescriptor
// → 把 socket 和 客户端 绑定在一起
// → 从此这个 socket 只和这个客户端通信
// → 收消息、发消息、断开连接，都是它管
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    tcpSocketList_.append(pTcpSocket);
}
