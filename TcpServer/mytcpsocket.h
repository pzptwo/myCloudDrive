#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include <QString>
#include "protocol.h"
#include <QDir>

class myTcpSocket : public QTcpSocket
{
    //注意如果支持信号槽机制的话Q_OBJECT加继承Object
    Q_OBJECT
public:
    explicit myTcpSocket(QObject *parent = nullptr);
    QString getstrName();
signals:
    void clientOffline(myTcpSocket *mysocket);    //信号要与槽函数参数及类型对应
//
public slots:
    //当有数据发送过来，会发送readyRead.
    void recvMsg();
    void solvediscoonnet();
private:
    QString strName_;
};

#endif // MYTCPSOCKET_H
