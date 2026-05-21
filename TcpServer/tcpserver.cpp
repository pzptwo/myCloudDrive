#include "tcpserver.h"
#include "ui_tcpserver.h"
#include "mytcpserver.h"
#include <QMessageBox>
#include <QDebug>

TcpServer::TcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpServer)
{
    ui->setupUi(this);
    loadconfig();
    //bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
    //[virtual protected] void QTcpServer::incomingConnection(qintptr socketDescriptor)（QTcpServer）
    //监听成功自动调用，所以继承的要重写,注意类啊，自动调用是在mytcpServer::getInstance()
    mytcpServer::getInstance().listen(QHostAddress(strIp_),port_);
    qDebug()<<"ip:"<<strIp_<<"port:"<<port_;
}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadconfig()
{
    QFile config(":/server.config");
    //打开文件
    if(config.open(QIODevice::ReadOnly))
    {
        QByteArray config_data =config.readAll();   //把配置文件的内容读取。并且转换为QString
        //qDebug()<<config_data;
        QString strData=config_data.toStdString().c_str();
        //qDebug()<<strData;
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


