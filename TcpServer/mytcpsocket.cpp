#include "mytcpsocket.h"
#include <QDebug>

myTcpSocket::myTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    //要完成信号槽的功能，注意几点
    connect(this,&myTcpSocket::readyRead,this,&myTcpSocket::recvMsg);
}

//有信号调用。
void myTcpSocket::recvMsg()
{
    //tcpsocket套接字里面有东西了
    qDebug()<<this->bytesAvailable();//当前客户端已经发送过来、等待你读取的 字节数量;
    //分析现在的pdu格式,一定要先把uiPDULen先读出来，不先读的话，uiMsgLen得不到
    uint uiPDUlen=0;
    //qint64 read(char *data, qint64 maxlen);
    this->read((char*)&uiPDUlen,sizeof(uint));
    uint uiMsgLen=uiPDUlen-sizeof(PDU);
    //这里已经先读取了uint了,去取后面的
    PDU *pdu=mkPDU(uiMsgLen);
    this->read((char *)pdu+sizeof(uint),uiPDUlen-sizeof(uint));
    qDebug()<<pdu->uiMsgType_<<(char *)(pdu->caMsg);
}

