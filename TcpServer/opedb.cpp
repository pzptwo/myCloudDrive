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

bool opedb::handlelogin(const char *caName, const char *caPwd)
{
    //登录的时候注意online字段
    if(caName!=NULL&&caPwd!=NULL)
    {
        //进行拼接
        QString data=QString("select * from usrInfo where name='%1' and pwd='%2' and online=0").arg(caName).arg(caPwd);
        QSqlQuery query;
        query.exec(data);
        //query现在拥有数据段,bool next();
        if(query.next())
        {
            //如果有数据证明查到了,把相关的数据更改
            data=QString("update usrInfo set online=1 where name='%1' and pwd='%2'").arg(caName).arg(caPwd);
            qDebug()<<data;
            //还是分为？？？？
            query.exec(data);
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

void opedb::handleoffline(const char *caName)
{
    if(caName!=NULL)
    {
        //进行拼接
        QString data=QString("update usrInfo set online=0 where name='%1'").arg(caName);
        qDebug()<<data;
        QSqlQuery query;
        query.exec(data);
    }
    else
    {
        qDebug()<<"error";
        return;
    }
}

QStringList opedb::handleallonline()
{
    QString data=QString("select name from usrInfo where online=1");
    QSqlQuery query;
    query.exec(data);
    //query现在拥有数据段,bool next();
    QStringList ret;
    ret.clear();
    while(query.next())
    {
        //这里定义一个链表或者啥容器接受就好了
        ret.append(query.value(0).toString());
    }
    return ret;
}

int opedb::handleSearchUser(const char *caName)
{
    QString data=QString("select online from usrInfo where name='%1'").arg(caName);
    QSqlQuery query;
    query.exec(data);


    if(query.next())
    {
        int ret=query.value(0).toInt();
        if(ret==1)
        {
            return 1;
        }
        else if(ret==0)
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }

    //可以替换为return ret；
}

int opedb::handleAddUserCheak(const char *caLoginName, const char *caAddUserName)
{
    if(caLoginName==NULL||caAddUserName==NULL)
    {
        return -1;
    }
    //这里区分单向好友，还是双向好友
    QString data=QString(
                       "SELECT * FROM friend "
                       "WHERE (id = (SELECT id FROM usrInfo WHERE name = '%1') AND friendid = (SELECT id FROM usrInfo WHERE name = '%2')) "
                       "OR (id = (SELECT id FROM usrInfo WHERE name = '%3') AND friendid = (SELECT id FROM usrInfo WHERE name = '%4'))"
                       ).arg(caLoginName)
                       .arg(caAddUserName)
                       .arg(caAddUserName)
                       .arg(caLoginName);
    QSqlQuery query;
    query.exec(data);
    //这种做法很冗杂，otherways-->调用上面的查询，就是注意返回值，第二因为列表是更新后的，肯定都是在线的
    if(query.next())
    {
        //说明查询出来了
        return 0;   //双方是好友
    }
    else
    {
        QString data=QString("select online from usrInfo where name='%1'").arg(caAddUserName);
        QSqlQuery query1;
        query1.exec(data);

        //调用上面的查询，就是注意返回值优化方向，
        if(query1.next())
        {
            int ret=query1.value(0).toInt();
            if(ret==1)
            {
                return 1;   //对方在线
            }
            else if(ret==0)
            {
                return 2;   //对方不在线
            }
        }
        else
        {
            return 3;  //对方不存在。设计的逻辑很好但是与一开始不符合？？？。
        }
    }
}

void opedb::handleAddUser(const char *caLoginName, const char *caAddUserName)
{
    //这里才是收到agreed后执行的加好友的操作
    if(caLoginName==NULL||caAddUserName==NULL)
    {
        return ;
    }
    //不好直接完成（id,friendid）,直接分段使用
    //获得发出加好友的Id
    QString data=QString("select id from usrInfo where name ='%1'").arg(caLoginName);
    QSqlQuery query;
    query.exec(data);
    //记得把值取出来
    int selfid=query.value(0).toInt();

    //获得好友的id
    data=QString("select id from usrInfo where name ='%1'").arg(caAddUserName);
    query.exec(data);
    //记得把值取出来
    int friendid=query.value(0).toInt();

    //都有了id，双向好友
    data=QString("insert into friend (id,friendid) values (%1,%2), (%2,%1").arg(selfid).arg((friendid));
    query.exec(data);
}

QStringList opedb::handleFlushFriend(const char *caLoginName)
{
    //分几步获得
    QString data=QString("select name from usrInfo where id in(select id from friend where friendid=(select id from usrInfo where name='%1'))and online=1").arg(caLoginName);
    QSqlQuery query;
    query.exec(data);

    QStringList strFriendList;
    strFriendList.clear();
    while (query.next())
    {
        strFriendList.append(query.value(0).toString());
        //打印日志验证一下
        qDebug()<<query.value(0).toString();
    }
    data=QString("select name from usrInfo where id in(select friendid from friend where id=(select id from usrInfo where name='%1'))and online=1").arg(caLoginName);
    //QSqlQuery query;
    query.exec(data);
    while (query.next())
    {
        strFriendList.append(query.value(0).toString());
        //打印日志验证一下
        qDebug()<<query.value(0).toString();
    }
    return strFriendList;

}

bool opedb::handleDelFriend(const char *caLoginName, const char *caAddUserName)
{
    if(caLoginName==NULL||caAddUserName==NULL)
    {
        return false;
    }

    //双向的，分两次，清晰一点
    QString data=QString("delete from friend where id=(select id from usrInfo where name='%1') and friendid=(select id from usrInfo where name='%2')").arg(caLoginName).arg(caAddUserName);
    QSqlQuery query;
    query.exec(data);
    //如果不存在的数据库无影响？？
    data=QString("delete from friend where id=(select id from usrInfo where name='%1') and friendid=(select id from usrInfo where name='%2')").arg(caAddUserName).arg(caLoginName);
    query.exec(data);

    return true;
}

QStringList opedb::handleGroupChat(const char *caLoginName)
{
    //调用handleFlushFriend
    QStringList onlineStrList=handleFlushFriend(caLoginName);
    return onlineStrList;
}

opedb::~opedb()
{
    db_.close();
}
