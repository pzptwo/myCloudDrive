#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
#include <QString>
#include <QSqlDatabase> //模块问题
#include <QSqlQuery>
class opedb : public QObject
{
    Q_OBJECT
public:
    explicit opedb(QObject *parent = nullptr);
    static opedb &getInstance();
    void init();
    //操作数据库
    bool handleregister(const char *caName,const char* caPwd);
    ~opedb();
signals:
private:
    QSqlDatabase db_;

};

#endif // OPEDB_H
