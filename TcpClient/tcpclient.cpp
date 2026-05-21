#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QFile>
#include <QDebug>
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
