#include "mytcpsocket.h"
#include <QDebug>
#include "opedb.h"
#include "mytcpserver.h"
#include <QDir>
#include <QFileInfoList>

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
                //当注册成功，区分不同的用户,已该用户的名字（主键）作为文件地址,但是注意是该文件./下面
                QDir dir;
                qDebug()<<"create dir :"<<dir.mkdir(QString("./%1").arg(caName));
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
        case ENUM_MSG_TYPE_DEL_FRIEND_RESPEST:
        {
            //这里caData顺序是loginname frinedname
            char caFrinedName[32]={'\0'};
            char caLoginName[32]={'\0'};
            memcpy(caLoginName,pdu->caData,32);
            memcpy(caFrinedName,pdu->caData+32,32);
            //相关数据库的操作,而且删除好友肯定是一定能成功的，不用if也行
            if(opedb::getInstance().handleDelFriend(caLoginName,caFrinedName))
            {
                PDU *respdu=mkPDU(0);
                respdu->uiMsgType_=ENUM_MSG_TYPE_DEL_FRIEND_RESPONSE;
                memcpy(respdu->caData,DEL_FRIEND_OK,32);

                write((char *)respdu,respdu->uiPDULen_);
                free(respdu);
                respdu=NULL;
            }

            //这里还需要进行转发，被删除的人也要知道，被删了。(就是不同的socket)
            mytcpServer::getInstance().resend(caFrinedName,pdu);
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_RESPEST:
        {
            //这里其实就是要获得接收消息用户的名字，进行转发
            char caRecvName[32]={'\0'};
            memcpy(caRecvName,pdu->caData+32,32);
            //打印日志验证
            qDebug()<<caRecvName;

            mytcpServer::getInstance().resend(caRecvName,pdu);
            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_RESPEST:
        {
            //还是数据库
            QStringList strOnlineList=opedb::getInstance().handleGroupChat(pdu->caData);
            //根据大小进行转发
            for(int i=0;i<strOnlineList.size();i++)
            {
                //要获得接收方的名字
                QString strRecvName=strOnlineList.at(i);
                mytcpServer::getInstance().resend(strRecvName.toStdString().c_str(),pdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_RESPEST:
        {
            QDir dir;
            //由于类型有问题，所以要转换
            QString strCurpath=QString("%1").arg((char *)pdu->caMsg);
            qDebug()<<strCurpath;
            bool ret =dir.exists(strCurpath);
            PDU *respdu=mkPDU(0);
            respdu->uiMsgType_=ENUM_MSG_TYPE_CREATE_DIR_RESPONSE;
            //首先根文件夹必须存在
            if(ret)
            {
                char strName[32]={'\0'};
                memcpy(strName,pdu->caData+32,32);
                QString strNewPath=strCurpath+"/"+strName;
                ret=dir.exists(strNewPath);
                qDebug()<<strNewPath;
                qDebug()<<"---->"<<ret;
                if(ret)
                {
                    memcpy(respdu->caData,FILE_EXIST,64);
                    //存在，重复了，不能创建
                }
                else
                {
                    memcpy(respdu->caData,FILE_CREATE_OK,64);
                    //可以创建
                    dir.mkdir(strNewPath);
                }
            }
            //根文件夹不存在
            else
            {
                memcpy(respdu->caData,DIR_NOT_EXISTED,64);
            }

            write((char *)respdu,respdu->uiPDULen_);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_DIR_RESPEST:
        {
            //使用新的写法，利用uiMsgLen
            //C++ 中，对象创建后，不能用 对象(参数) 这种语法重新赋值 / 初始化，必须调用成员函数（如 setPath）。
            char *caPath=new char[pdu->uiMsgLen_];
            strcpy(caPath,(char *)(pdu->caMsg));
            QDir dir(caPath);
            //可以查询
            // QFileInfoList entryInfoList(Filters filters = NoFilter, SortFlags sort = NoSort) const;
            // QFileInfoList entryInfoList(const QStringList &nameFilters, Filters filters = NoFilter,
            //                             SortFlags sort = NoSort) const;
            QFileInfoList fileInfoLIst=dir.entryInfoList();
            FileInfo *pFileInfo=nullptr;//文件指针
            uint uiMsgLen=(uint)(sizeof(FileInfo)*fileInfoLIst.size());
            PDU *respdu=mkPDU(uiMsgLen);
            respdu->uiMsgType_=ENUM_MSG_TYPE_FLUSH_DIR_RESPONSE;
            QString strFileName;
            for(auto i=0;i<fileInfoLIst.size();i++)
            {
                //给结构体的属性赋值FileInfo *与FileInfo??
                pFileInfo=(FileInfo *)(respdu->caMsg)+i;
                strFileName=fileInfoLIst[i].fileName();
                memcpy(pFileInfo->caFileName,strFileName.toStdString().c_str(),strFileName.size());
                if(fileInfoLIst[i].isDir())
                {
                    pFileInfo->iFileType=0;
                }
                else if(fileInfoLIst[i].isFile())
                {
                    pFileInfo->iFileType=1;
                }
            }
            //PDU *respdu=mkPDU();
            write((char *)respdu,respdu->uiPDULen_);
            free(respdu);
            respdu=nullptr;
            break;
        }
        case ENUM_MSG_TYPE_DEL_DIR_RESPEST:
        {
            //获得路径及其名字啦
            char strName[32]={'\0'};
            char *pPath=new char[pdu->uiMsgLen_];

            memcpy(strName,pdu->caData,32);
            memcpy(pPath,(char *)(pdu->caMsg),uiMsgLen);

            //拼接新路径
            QString strNewPath=QString("%1/%2").arg(pPath).arg(strName);
            //打印验证日志
            qDebug()<<strNewPath;

            //这里是删除文件夹，！！！
            QFileInfo fileInfo(strNewPath);
            bool ret=false;
            if(fileInfo.isDir())
            {
                //可以删除
                QDir dir(strNewPath);
                ret=dir.removeRecursively();
            }
            else if(fileInfo.isFile())
            {
                ret=false;
            }

            PDU *respdu=mkPDU(0);
            respdu->uiMsgType_=ENUM_MSG_TYPE_DEL_DIR_RESPONSE;
            //根据ret的不同传值
            if(ret)
            {
                memcpy(respdu->caData,DEL_DIR_OK,32);
            }
            else
            {
                memcpy(respdu->caData,DEL_DIR_FLASE,32);
            }
            write((char *)respdu,respdu->uiPDULen_);
            free(respdu);
            respdu=nullptr;
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_RESPEST:
        {
            char strOldName[32]={'\0'};
            char strNewName[32]={'\0'};
            char *pPath=new char[pdu->uiMsgLen_];
            memcpy(strOldName,pdu->caData,32);
            memcpy(strNewName,pdu->caData+32,32);
            memcpy(pPath,(char *)(pdu->caMsg),uiMsgLen);

            QString strOldPath=QString("%1/%2").arg(pPath).arg(strOldName);
            QString strNewPath=QString("%1/%2").arg(pPath).arg(strNewName);

            QDir dir;
            //bool rename(const QString &oldName, const QString &newName);
            bool ret=dir.rename(strOldPath,strNewPath);

            PDU *respdu=mkPDU(0);
            respdu->uiMsgType_=ENUM_MSG_TYPE_RENAME_FILE_RESPONSE;
            if(ret==true)
            {
                memcpy(respdu->caData,RENAME_OK,32);
            }
            else
            {
                memcpy(respdu->caData,RENAME_FLASE,32);
            }
            write((char *)respdu,respdu->uiPDULen_);
            free(respdu);
            respdu=nullptr;
            break;
        }
        case ENUM_MSG_TYPE_ENTRY_DIR_RESPEST:
        {
            char strName[32]={'\0'};
            char *pPath=new char[pdu->uiMsgLen_];
            strcpy(strName,pdu->caData);
            memcpy(pPath,(char *)(pdu->caMsg),uiMsgLen);
            pPath[uiMsgLen] = '\0';

            QString strPath=QString("%1/%2").arg(pPath).arg(strName);
            qDebug()<<strPath;

            PDU *respdu=nullptr;
            //首先先判断当前的文件类型
            QFileInfo fileInfo(strPath);
            if(fileInfo.isFile())
            {
                respdu=mkPDU(0);
                respdu->uiMsgType_=ENUM_MSG_TYPE_ENTRY_DIR_RESPONSE;
                memcpy(respdu->caData,ENTRY_DIR_FLASE,sizeof(ENTRY_DIR_FLASE));
            }
            else if(fileInfo.isDir())
            {
                //这里的优化可以设定为函数，封装更好，可以复用
                //这里功能与上面的刷新类似
                QDir dir(strPath);
                //可以查询
                // QFileInfoList entryInfoList(Filters filters = NoFilter, SortFlags sort = NoSort) const;
                // QFileInfoList entryInfoList(const QStringList &nameFilters, Filters filters = NoFilter,
                //                             SortFlags sort = NoSort) const;
                QFileInfoList fileInfoLIst=dir.entryInfoList();
                FileInfo *pFileInfo=nullptr;//文件指针，注意噶
                uint uiMsgLen=(uint)(sizeof(FileInfo)*fileInfoLIst.size());
                respdu=mkPDU(uiMsgLen+1);
                respdu->uiMsgType_=ENUM_MSG_TYPE_FLUSH_DIR_RESPONSE;
                QString strFileName;
                for(auto i=0;i<fileInfoLIst.size();i++)
                {
                    //给结构体的属性赋值FileInfo *与FileInfo??
                    pFileInfo=(FileInfo *)(respdu->caMsg)+i;
                    strFileName=fileInfoLIst[i].fileName();
                    memcpy(pFileInfo->caFileName,strFileName.toStdString().c_str(),strFileName.size());
                    if(fileInfoLIst[i].isDir())
                    {
                        pFileInfo->iFileType=0;
                    }
                    else if(fileInfoLIst[i].isFile())
                    {
                        pFileInfo->iFileType=1;
                    }
                }
            }
            write((char *)respdu,respdu->uiPDULen_);
            free(respdu);
            respdu=nullptr;
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

