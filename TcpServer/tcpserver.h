#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QWidget>
#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QStringList>
#include <QHostAddress>


QT_BEGIN_NAMESPACE
namespace Ui {
class TcpServer;
}
QT_END_NAMESPACE

class TcpServer : public QWidget
{
    Q_OBJECT

public:
    TcpServer(QWidget *parent = nullptr);
    ~TcpServer();
    void loadconfig();  //配置文件

private:
    Ui::TcpServer *ui;
    QString strIp_;
    quint16 port_;
};
#endif // TCPSERVER_H
