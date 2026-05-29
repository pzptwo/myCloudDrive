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
    bool handleregister(const char *caName,const char* caPwd);  //对于注册的时候
    bool handlelogin(const char *caName,const char* caPwd);  //对于登录的时候的变更
    void handleoffline(const char *caName); //对于下线的时候，数据库的更改
    QStringList handleallonline(); //获得在线的数据，只要名字即可
    //因为有三种类型（已online为检索，1,0，-1：不存在）
    int handleSearchUser(const char *caName);

    //也是好几种情况,这里的与pdu传的顺序不一样，注意哇，这里和建表的一样（(Login)id,friendid）
    int handleAddUserCheak(const char *caLoginName,const char* caAddUserName);

    void handleAddUser(const char *caLoginName,const char* caAddUserName);

    //由于多个数据，用链表接收
    QStringList handleFlushFriend(const char *caLoginName);

    bool handleDelFriend(const char *caLoginName,const char* caAddUserName);
    ~opedb();
signals:
private:
    QSqlDatabase db_;

};

#endif // OPEDB_H
