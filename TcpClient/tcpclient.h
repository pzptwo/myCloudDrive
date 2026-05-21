#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QString>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui {
class TcpClient;
}
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void loadconfig();  //配置文件

public slots:
    void connectHost();

private:
    Ui::TcpClient *ui;
    QString strIp_;
    quint16 port_;
    //添加TcpSocket进行连接
    QTcpSocket mytcpSocket_;    //与服务器连接，2.与服务器进行交互。
};
#endif // TCPCLIENT_H
