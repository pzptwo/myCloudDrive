#include "tcpclient.h"
#include "ui_tcpclient.h"
#include "protocol.h"
#include "opewidget.h"
#include <QFile>
#include <QDebug>
#include <QString>
#include <QMessageBox>
#include <QStringList>
#include <QHostAddress>

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    this->loadconfig();
    resize(700,300);
    //连接服务器的函数是connectToHost(),这里封装了成功发送connected信号,所以要验证是否成功。
    //绑定信号与槽函数
    connect(&mytcpSocket_,SIGNAL(connected()),this,SLOT(connectHost()));//QT4,容易丢括号。
    connect(&mytcpSocket_,&QTcpSocket::readyRead,this,&TcpClient::recvMsg);
    //QT5connect( &对象名, &类名::信号名, 接收者对象, &类名::槽函数名 );
    //connect(&mytcpSocket_,&QTcpSocket::connected,this,&TcpClient::connectHost);
    //virtual void connectToHost(const QHostAddress &address, quint16 port, OpenMode mode = ReadWrite);
    mytcpSocket_.connectToHost(QHostAddress(strIp_),port_);
}

TcpClient::~TcpClient()
{
    delete ui;
}

void TcpClient::loadconfig()
{
    QFile config(":/client.config");
    //打开文件
    if(config.open(QIODevice::ReadOnly))
    {
        QByteArray config_data =config.readAll();   //把配置文件的内容读取。并且转换为QString
        qDebug()<<config_data;
        QString strData=config_data.toStdString().c_str();
        qDebug()<<strData;
        config.close();
        //分隔"127.0.0.1\r\n8888"
        strData.replace("\r\n"," ");
        // QStringList split(const QRegularExpression &sep,
        //                   Qt::SplitBehavior behavior = Qt::KeepEmptyParts) const;

        QStringList spStr=strData.split(" ");
        strIp_=spStr.at(0);
        port_=spStr.at(1).toUShort();
        qDebug()<<"ip:"<<strIp_<<"port:"<<port_;

    }
    else
    {
        QMessageBox::critical(this,"error","open error");
        //error
    }
}

//一定要把connect用上，否则信号槽没有用
void TcpClient::recvMsg()
{
    //tcpsocket套接字里面有东西了
    qDebug()<<mytcpSocket_.bytesAvailable();//当前客户端已经发送过来、等待你读取的 字节数量;
    //分析现在的pdu格式,一定要先把uiPDULen先读出来，不先读的话，uiMsgLen得不到
    uint uiPDUlen=0;
    //qint64 read(char *data, qint64 maxlen);
    mytcpSocket_.read((char*)&uiPDUlen,sizeof(uint));
    uint uiMsgLen=uiPDUlen-sizeof(PDU);
    //这里已经先读取了uint了,去取后面的
    PDU *pdu=mkPDU(uiMsgLen);
    mytcpSocket_.read((char *)pdu+sizeof(uint),uiPDUlen-sizeof(uint));
    switch (pdu->uiMsgType_)
    {
        case ENUM_MSG_TYPE_REGISTER_RESPONSE:
        {
            if(0==strcmp(pdu->caData, REGISTER_OK))
            {
                QMessageBox::information(this,"注册","注册成功");
            }

            else if(0==strcmp(pdu->caData, REGISTER_FALSE))
            {
                QMessageBox::warning(this,"注册","注册失败");
            }
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_RESPONSE:
        {
            if(0==strcmp(pdu->caData, LOGIN_OK))
            {
                QMessageBox::information(this,"登录","登录成功");
                opeWidget::getInstance().show();
                this->hide();
            }

            else if(0==strcmp(pdu->caData, LOGIN_FALSE))
            {
                QMessageBox::warning(this,"登录","登录失败");
            }
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_RESPONSE:
        {
            //要把数据传到online页面上，online在friendlw上，
            opeWidget::getInstance().getFriend().showAllOnline(pdu);
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USER_RESPONSE:
        {
            //根据不同cadata来判断
            if(0==strcmp(pdu->caData,SEARCH_ONLINE))
            {
                QMessageBox::information(this,"搜索",SEARCH_ONLINE);
            }
            else if(0==strcmp(pdu->caData,SEARCH_OFFLINE))
            {
                QMessageBox::information(this,"搜索",SEARCH_OFFLINE);
            }
            else if(0==strcmp(pdu->caData,SEARCH_NOPERSON))
            {
                QMessageBox::information(this,"搜索",SEARCH_NOPERSON);
            }
            break;
        }
        //分为两种类型
        case ENUM_MSG_TYPE_ADD_USER_RESPONSE:
        {

            QMessageBox::information(this,"添加用户",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_ADD_USER_RESPEST:
        {
            //注意现在在的socket,与上面的不一样，是想加好友的，被加好友的在这里回复
            char caLoginName[32]={'\0'};
            memcpy(caLoginName,pdu->caData+32,32);
            //枚举int
            int ret=QMessageBox::information(this,"添加用户",QString("%1 want to add you as friend.").arg(caLoginName),QMessageBox::Yes,QMessageBox::No);
            PDU *respdu=mkPDU(0);
            memcpy(respdu->caData,pdu->caData,32);  //拷贝
            memcpy(respdu->caData+32,pdu->caData+32,32);
            if(ret==QMessageBox::Yes)
            {
                respdu->uiMsgType_=ENUM_MSG_TYPE_ADD_USER_AGREED;
                //要发一个回复的respdu，后面进行转发
            }
            else
            {
                respdu->uiMsgType_=ENUM_MSG_TYPE_ADD_USER_REFUSE;
            }
            mytcpSocket_.write((char *)respdu,respdu->uiPDULen_);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_USER_AGREED:
        {
            //打印消息
            char caAddUser[32]={'\0'};
            memcpy(caAddUser,pdu->caData,32);
            QMessageBox::information(this,"添加好友",QString("'%1'同意添加好友").arg(caAddUser));

            //这里主动刷新好友列表，变化了
            break;
        }

        case ENUM_MSG_TYPE_ADD_USER_REFUSE:
        {
            char caAddUser[32]={'\0'};
            memcpy(caAddUser,pdu->caData,32);
            QMessageBox::information(this,"添加好友",QString("'%1'拒绝添加好友").arg(caAddUser));
            break;
        }

    default:
        break;
    }
    free(pdu);
    pdu=NULL;
}

TcpClient &TcpClient::getinstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return mytcpSocket_;
}

QString TcpClient::getstrLoginName()
{
    return strLoginName_;
}

void TcpClient::connectHost()
{
    QMessageBox::information(this,"连接服务器","连接服务器成功");
}
/*
void TcpClient::on_send_pb_clicked()
{
    //获得文本框的数据
    QString strMsg=ui->lineEdit->text();    //得到实际数据了，要发送数据，
    if(strMsg.isEmpty())
    {
        QMessageBox::warning(this,"信息发送","信息发送不能为空");
    }
    else
    {
        //要发送了,通过myTcpSocket进行通信，要得到自定义的协议
        PDU *pdu=mkPDU(strMsg.size());
        //注意发送的实际的数据caMsg_,所以通过memcpy或者strcpy进行赋值
        //void * __cdecl memcpy(void * __restrict__ _Dst,const void * __restrict__ _Src,size_t _Size) __MINGW_ATTRIB_DEPRECATED_SEC_WARN;
        memcpy(pdu->caMsg,strMsg.toStdString().c_str(),strMsg.size());
        qDebug()<<(char *)pdu->caMsg;
        //现在随便定义一个类型
        pdu->uiMsgType_=6666;
        mytcpSocket_.write((char *)pdu,pdu->uiPDULen_);
        free(pdu);
        pdu=NULL;
    }

}
*/



void TcpClient::on_login_pb_clicked()
{
    //这里进行登录的应用
    QString strName=ui->name_le->text();
    QString strPwd=ui->pwd_le->text();

    //这里先验证一遍（我想优化的方向可能是封装在数据库里面？？？？）
    if(!strName.isEmpty()&&!strPwd.isEmpty())
    {
        strLoginName_=strName;
        //这里为啥要用上pdu(数据库在服务器那边),这里没有实际发送的消息
        PDU *pdu=mkPDU(0);
        pdu->uiMsgType_=ENUM_MSG_TYPE_LOGIN_RESPEST;
        memcpy(pdu->caData,strName.toStdString().c_str(),32);
        memcpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
        //发送
        mytcpSocket_.write((char *)pdu,pdu->uiPDULen_);
        free(pdu);
        pdu=NULL;
    }
    else
    {
        QMessageBox::warning(this,"登录","登录用户名和名字不能为空");
    }

}


void TcpClient::on_register_pb_clicked()
{
    QString strName=ui->name_le->text();
    QString strPwd=ui->pwd_le->text();

    //这里先验证一遍（我想优化的方向可能是封装在数据库里面？？？？）
    if(!strName.isEmpty()&&!strPwd.isEmpty())
    {
        //这里为啥要用上pdu(数据库在服务器那边),这里没有实际发送的消息
        PDU *pdu=mkPDU(0);
        pdu->uiMsgType_=ENUM_MSG_TYPE_REGISTER_RESPEST;
        memcpy(pdu->caData,strName.toStdString().c_str(),32);
        memcpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
        //发送
        mytcpSocket_.write((char *)pdu,pdu->uiPDULen_);
        free(pdu);
        pdu=NULL;
    }
    else
    {
        QMessageBox::warning(this,"注册","注册时用户名和名字不能为空");
    }
}


void TcpClient::on_layout_pb_clicked()
{

}

