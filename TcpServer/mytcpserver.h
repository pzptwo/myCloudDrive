#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include "mytcpsocket.h"
#include <QTcpServer>
#include <QList>
class mytcpServer : public QTcpServer
{
    //注意如果支持信号槽机制的话Q_OBJECT加继承Object
    Q_OBJECT
public:
    mytcpServer();
    static mytcpServer &getInstance();  //运用单例的模式,其实主要是static
    void incomingConnection(qintptr socketDescriptor);  //重写
    //(myTcpSocket *mysocket);所以转发函数只能写在这里面
    void resend(const char* caAddUser,PDU *pdu);
private:
    QList<myTcpSocket *> tcpSocketList_;

    // [signal] void QAbstractSocket::disconnected()
    //     This signal is emitted when the socket has been disconnected.
    //     Warning: If you need to delete the sender() of this signal in a slot connected to it, use the deleteLater() function.
    //     See also connectToHost(), disconnectFromHost(), and abort().

    //定义socket取消链接！！！！而不是删除socket后面才能发出disconnected()后面采取数据库把数据更改。
    //但是取消socket，要等tcpsocket发出信号。（就是客户端已下线的信号）所以设置为槽函数
public slots:
    void deletesocket(myTcpSocket *mysocket);
};

#endif // MYTCPSERVER_H
