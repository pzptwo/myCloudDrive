#ifndef PROTOCOL_H
#define PROTOCOL_H

//设计自定义的通讯协议
//由于长度都是大于0的
//#define unsigned int uint
typedef unsigned int uint;  //两个的阶段不同（编译与预处理）
#define REGISTER_OK "register_ok"
#define REGISTER_FALSE  "register_false:name existed"

#define LOGIN_OK "login_ok"
#define LOGIN_FALSE  "login_false:name error or pwd error or relogin"

#define SEARCH_ONLINE "existd and online"
#define SEARCH_OFFLINE "existed but offline"
#define SEARCH_NOPERSON "not exist"

#define UNKNOWN_ERROR "unknown error"
#define ADD_FRIEND_OFFLINE "user offline"
#define EXITED_FRIEND "user had been your friend"
#define NOT_EXISTED "user not existed"
//需要分通信协议的类型了
enum ENUM_MSG_TYPE
{
    ENUM_MSG_TYPE_MIN=0,
    ENUM_MSG_TYPE_REGISTER_RESPEST, //注册逻辑
    ENUM_MSG_TYPE_REGISTER_RESPONSE,

    ENUM_MSG_TYPE_LOGIN_RESPEST, //登录逻辑
    ENUM_MSG_TYPE_LOGIN_RESPONSE,

    ENUM_MSG_TYPE_ALL_ONLINE_RESPEST, //所有在线
    ENUM_MSG_TYPE_ALL_ONLINE_RESPONSE,

    ENUM_MSG_TYPE_SEARCH_USER_RESPEST, //搜索用户
    ENUM_MSG_TYPE_SEARCH_USER_RESPONSE,

    ENUM_MSG_TYPE_ADD_USER_RESPEST, //加用户，（pdu里面先传addUser,后是登录的名字）
    ENUM_MSG_TYPE_ADD_USER_RESPONSE,

    ENUM_MSG_TYPE_ADD_USER_AGREED,
    ENUM_MSG_TYPE_ADD_USER_REFUSE,
    ENUM_MSG_TYPE_MAX=0x00ffffff
};
//struct 的作用域只要包含头文件即可；！！
typedef struct PDU
{
    uint uiPDULen_;
    uint uiMsgLen_;
    uint uiMsgType_;     //消息类型，不同的处理不同
    char caData[64];    //后面使用保存文件名
    int caMsg[];       //弹性数组，不占用内存，指向实际消息,当时为啥是int、？？？
}PDU;

PDU *mkPDU(uint uiMsgLen);   //声明，创建一个，并返回（类型初始化吧），我要的是实际的消息uiMsgLen+sizeof(PDU);
#endif // PROTOCOL_H
