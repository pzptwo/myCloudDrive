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
    connect(pTcpSocket,&myTcpSocket::clientOffline,this,&mytcpServer::deletesocket);    //记得一定要是对象

    //这里进行信号的连接
}

void mytcpServer::resend(const char *caAddUser, PDU *pdu)
{
    if(caAddUser==NULL||pdu==NULL)
    {
        return;
    }
    //
    for(int i=0;i<tcpSocketList_.size();i++)
    {
        if(caAddUser==tcpSocketList_.at(i)->getstrName())
        {
            tcpSocketList_.at(i)->write((char *)pdu,pdu->uiPDULen_);
        }
        //qDebug()<<tcpSocketList_.at(i)->getstrName();
    }
}


//记得参数要删掉哪一个
void mytcpServer::deletesocket(myTcpSocket *mysocket)
{
    //由于是链表，使用迭代器
    QList<myTcpSocket *>::iterator iter=tcpSocketList_.begin();
    for(;iter!=tcpSocketList_.end();iter++)
    {
        //这里要删除两个，一个是我定义的iter,一个是链表里面的socket
        if(mysocket==*iter)
        {
            //我感觉逻辑有点问题,可能没有分清指针与对象
            mysocket->deleteLater();     // 延迟删除，等事件循环处理
            tcpSocketList_.erase(iter);
            break;
        }
    }
    //打印日志，验证
    for(int i=0;i<tcpSocketList_.size();i++)
    {
        qDebug()<<tcpSocketList_.at(i)->getstrName();
    }
}
