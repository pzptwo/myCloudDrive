#include "tcpclient.h"
#include "ui_tcpclient.h"
#include "protocol.h"
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
    //连接服务器的函数是connectToHost(),这里封装了成功发送connected信号,所以要验证是否成功。
    //绑定信号与槽函数
    connect(&mytcpSocket_,SIGNAL(connected()),this,SLOT(connectHost()));//QT4,容易丢括号。
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

void TcpClient::connectHost()
{
    QMessageBox::information(this,"连接服务器","连接服务器成功");
}

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

