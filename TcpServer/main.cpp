#include "tcpserver.h"
#include "opedb.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    opedb::getInstance().init();
    // TcpServer w;
    // w.show();
    return a.exec();
}
