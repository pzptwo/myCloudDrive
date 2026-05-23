#include "mytcpsocket.h"
#include <QDebug>
#include "opedb.h"

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

    //作为回复（交互）
    //作为回复的PDU respdu
    PDU *respdu=mkPDU(0);
    switch (pdu->uiMsgType_)
    {
        case ENUM_MSG_TYPE_REGISTER_RESPEST:
    {
            //这里其实才开始时注册的部分
            char caName[32]={'\0'};
            char caPwd[32]={'\0'};
            memcpy(caName,pdu->caData,32);
            memcpy(caPwd,pdu->caData+32,32);
            bool ret=opedb::getInstance().handleregister(caName,caPwd);
            //不管成功还是失败都要给回复
            respdu->uiMsgType_=ENUM_MSG_TYPE_REGISTER_RESPONSE;
            if(ret)
            {
                //为了省略一个大小的获取用strcpy,要用双引号！！！！
                strcpy(respdu->caData,REGISTER_OK);
            }
            else
            {
                strcpy(respdu->caData,REGISTER_FALSE);
            }
            write((char *)respdu,respdu->uiPDULen_);
            free(respdu);
            respdu=NULL;
            break;
    }
        default:
            break;
    }
        free(pdu);
        pdu=NULL;
    //qDebug()<<pdu->uiMsgType_<<(char *)(pdu->caMsg);
}

