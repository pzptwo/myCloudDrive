#include "opedb.h"
#include <QMessageBox>
#include <QDebug>

opedb::opedb(QObject *parent)
    : QObject{parent}
{
    //[static] QSqlDatabase QSqlDatabase::addDatabase(const QString &type, const QString &connectionName = QLatin1String(defaultConnection))
    db_=QSqlDatabase::addDatabase("QSQLITE");
    //对于数据库需要初始化
}

opedb &opedb::getInstance()
{
    static opedb instance;
    return instance;
}

void opedb::init()
{
    db_.setHostName("localhost");   //设置域名
    //void setDatabaseName(const QString& name);ctrl
    //添加的方法不行
    db_.setDatabaseName("C:\\Users\\bob\\Desktop\\myCloudDrive\\review_cloudDrive\\TcpServer\\cloudDrive.db"); //这样的写法为什么.

    if(db_.open())
    {
        //查询数据库
        QSqlQuery query;
        query.exec("select * from usrInfo");

        while(query.next())
        {
            QString data=QString("%1,%2,%3").arg(query.value(0).toString()).arg(query.value(1).toString()).arg(query.value(2).toString());
            qDebug()<<"data:"<<data;
        }
    }
    else
    {
        QMessageBox::critical(NULL,"打开数据库","打开数据库失败");
    }
}

bool opedb::handleregister(const char *caName, const char *caPwd)
{
    if(caName!=NULL&&caPwd!=NULL)
    {
    //进行拼接
        QString data=QString("insert into usrInfo (name,pwd) values ('%1','%2')").arg(caName).arg(caPwd);
        QSqlQuery query;
        return query.exec(data);;
    }
    else
    {
        return false;
    }
}

opedb::~opedb()
{
    db_.close();
}
