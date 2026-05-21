#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>

class mytcpServer : public QTcpServer
{
    //注意如果支持信号槽机制的话Q_OBJECT加继承Object
    Q_OBJECT
public:
    mytcpServer();
    static mytcpServer &getInstance();  //运用单例的模式,其实主要是static
    void incomingConnection(qintptr socketDescriptor);  //重写
};

#endif // MYTCPSERVER_H
