#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include "protocol.h"

class myTcpSocket : public QTcpSocket
{
    //注意如果支持信号槽机制的话Q_OBJECT加继承Object
    Q_OBJECT
public:
    explicit myTcpSocket(QObject *parent = nullptr);

    //当有数据发送过来，会发送readyRead.
    void recvMsg();
};

#endif // MYTCPSOCKET_H
