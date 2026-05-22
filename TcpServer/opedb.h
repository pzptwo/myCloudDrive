#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
#include <QSqlDatabase> //模块问题
#include <QSqlQuery>
class opedb : public QObject
{
    Q_OBJECT
public:
    explicit opedb(QObject *parent = nullptr);
    static opedb &getInstance();
    void init();
signals:
private:
    QSqlDatabase db_;

};

#endif // OPEDB_H
