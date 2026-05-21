#include "mytcpserver.h"

mytcpServer::mytcpServer() {}

mytcpServer &mytcpServer::getInstance()
{
    static mytcpServer instance;    //一定要有static
    return instance;
}
void mytcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug()<<"new client connected";
}
