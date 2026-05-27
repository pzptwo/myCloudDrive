#include "mytcpsocket.h"
#include <QDebug>
#include "opedb.h"
#include "mytcpserver.h"


myTcpSocket::myTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    //要完成信号槽的功能，注意几点
    connect(this,&myTcpSocket::readyRead,this,&myTcpSocket::recvMsg);
    connect(this,&QAbstractSocket::disconnected,this,&myTcpSocket::solvediscoonnet);
}

QString myTcpSocket::getstrName()
{
    return strName_;
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
        case ENUM_MSG_TYPE_LOGIN_RESPEST:
        {
            char caName[32]={'\0'};
            char caPwd[32]={'\0'};
            memcpy(caName,pdu->caData,32);
            memcpy(caPwd,pdu->caData+32,32);
            bool ret=opedb::getInstance().handlelogin(caName,caPwd);
            //不管成功还是失败都要给回复
            respdu->uiMsgType_=ENUM_MSG_TYPE_LOGIN_RESPONSE;
            if(ret)
            {
                //为了省略一个大小的获取用strcpy,要用双引号！！！！
                strcpy(respdu->caData,LOGIN_OK);
                strName_=caName;    //strName_保存的名字,就是一个名字对应一个socket吗？？？,这里好像就开始埋雷了，后面多个客户端,

            }
            else
            {
                strcpy(respdu->caData,LOGIN_FALSE);
            }
            write((char *)respdu,respdu->uiPDULen_);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_RESPEST:
        {
            //这里获得相关数据库的数据
            QStringList ret=opedb::getInstance().handleallonline();
            //准备发送的pdu,一个名字占32位，设定的
            uint uiMsgLen=ret.size()*32;
            PDU *respdu=mkPDU(uiMsgLen);
            respdu->uiMsgType_=ENUM_MSG_TYPE_ALL_ONLINE_RESPONSE;
            //注意pdu里面的不同职责,还没在caMsg里面赋值，那就读不到。
            for(int i=0;i<ret.size();i++)
            {
                //记得需要偏移
                memcpy((char *)respdu->caMsg+i*32,ret.at(i).toStdString().c_str(),ret.at(i).size());
            }
            //发送pdu
            write((char *)respdu,respdu->uiPDULen_);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USER_RESPEST:
        {
            char caName[32]={'\0'};
            memcpy(caName,pdu->caData,32);
            //相关数据库操作
            int ret=opedb::getInstance().handleSearchUser(caName);
            PDU *respdu=mkPDU(0);   //用data即可
            respdu->uiMsgType_=ENUM_MSG_TYPE_SEARCH_USER_RESPONSE;
            if(ret==1)
            {
                strcpy(respdu->caData,SEARCH_ONLINE);
            }
            else if(ret==0)
            {
                strcpy(respdu->caData,SEARCH_OFFLINE);
            }
            else if(ret==-1)
            {
                strcpy(respdu->caData,SEARCH_NOPERSON);
            }
            write((char *)respdu,respdu->uiPDULen_);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_USER_RESPEST:
        {
            char caAddUserName[32]={'\0'};
            char caLoginName[32]={'\0'};
            memcpy(caAddUserName,pdu->caData,32);
            memcpy(caLoginName,pdu->caData+32,32);
            //依旧数据库
            int ret=opedb::getInstance().handleAddUserCheak(caLoginName,caAddUserName);
            //有四种类似于回复出现问题，在线的才可以加好友，但是加好友的话，要进行转发，好友的socket就是请求，就是一个过程
            PDU *respdu=mkPDU(0);
            respdu->uiMsgType_=ENUM_MSG_TYPE_ADD_USER_RESPONSE;
            if(ret==-1)
            {
                strcpy(respdu->caData,UNKNOWN_ERROR);
            }
            else if(ret==0)
            {
                strcpy(respdu->caData,EXITED_FRIEND);
            }
            else if(ret==1)
            {
                //这里就是在线了，可以进行流程，转发函数
                mytcpServer::getInstance().resend(caAddUserName,pdu);
            }
            else if(ret==2)
            {
                strcpy(respdu->caData,ADD_FRIEND_OFFLINE);
            }
            else if(ret==3)
            {
                strcpy(respdu->caData,NOT_EXISTED);
            }
            write((char *)respdu,respdu->uiPDULen_);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_USER_AGREED:
        {
            char caAddUserName[32]={'\0'};
            char caLoginName[32]={'\0'};
            memcpy(caAddUserName,pdu->caData,32);
            memcpy(caLoginName,pdu->caData+32,32);
            qDebug()<<caAddUserName<<caLoginName;
            //这里全利用回转函数就好了，数据库操作
            opedb::getInstance().handleAddUser(caLoginName,caAddUserName);
            //这里传的name->!!!我要self要收到的pdu
            mytcpServer::getInstance().resend(caLoginName,pdu);
            break;
        }
        case ENUM_MSG_TYPE_ADD_USER_REFUSE:
        {
            char caLoginName[32]={'\0'};
            memcpy(caLoginName,pdu->caData+32,32);
            mytcpServer::getInstance().resend(caLoginName,pdu);
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPEST:
        {
            //数据库相关操作
            QStringList strFlushFriend=opedb::getInstance().handleFlushFriend(pdu->caData);
            uint uiMsgLen=strFlushFriend.size()*32;
            PDU *respdu=mkPDU(uiMsgLen);
            respdu->uiMsgType_=ENUM_MSG_TYPE_FLUSH_FRIEND_RESPONSE;
            for(int i=0;i<strFlushFriend.size();i++)
            {
                //注意是caMsg
                memcpy((char *)(respdu->caMsg)+i*32,strFlushFriend.at(i).toStdString().c_str(),strFlushFriend.at(i).size());
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

void myTcpSocket::solvediscoonnet()
{
    //这里已经没有socket了
    //这里就要对数据库进行处理
    // opedb::getInstance().handleoffline(strName_.toStdString().c_str());
    // emit clientOffline(this);
    std::string name = strName_.toStdString();
    opedb::getInstance().handleoffline(name.c_str());
    emit clientOffline(this);
}

