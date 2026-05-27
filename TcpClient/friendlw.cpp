#include "friendlw.h"

#include "tcpclient.h"
FriendLW::FriendLW(QWidget *parent)
    : QWidget{parent}
{
    //都是继承QWidget
    pShowMsgTE_=new QTextEdit;
    pFriendListWidget_=new QListWidget;
    pInputMsgLE_=new QLineEdit;

    pDelFriendPB_=new QPushButton("删除好友");
    pFlushFriendPB_=new QPushButton("刷新好友");
    pShowOnlineUsrPB_=new QPushButton("显示在线用户");
    pSearchUsrPB_=new QPushButton("查找用户");
    pMsgSendPB_=new QPushButton("信息发送");
    pPrivateChatPB_=new QPushButton("私聊");

    QVBoxLayout *pRightPBVBL=new QVBoxLayout;
    pRightPBVBL->addWidget(pDelFriendPB_);
    pRightPBVBL->addWidget(pFlushFriendPB_);
    pRightPBVBL->addWidget(pShowOnlineUsrPB_);
    pRightPBVBL->addWidget(pSearchUsrPB_);
    pRightPBVBL->addWidget(pPrivateChatPB_);

    QHBoxLayout *pTopHBL=new QHBoxLayout;
    pTopHBL->addWidget(pShowMsgTE_);
    pTopHBL->addWidget(pFriendListWidget_);
    pTopHBL->addLayout(pRightPBVBL);

    QHBoxLayout *pMsgHBL=new QHBoxLayout;
    pMsgHBL->addWidget(pInputMsgLE_);
    pMsgHBL->addWidget(pMsgSendPB_);

    pOnline_=new online;

    QVBoxLayout *pMain=new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addWidget(pOnline_);
    pOnline_->hide();

    setLayout(pMain);

    connect(pShowOnlineUsrPB_,&QPushButton::clicked,this,&FriendLW::showOnline);
    connect(pSearchUsrPB_,&QPushButton::clicked,this,&FriendLW::serachUser);
}

void FriendLW::showAllOnline(PDU *pdu)
{
    if(pdu==NULL)
    {
        return ;
    }
    if(pOnline_->isHidden())   // 防止响应问题
        return;
    //这里调用online的方法
    pOnline_->showOnlineUser(pdu);
}

void FriendLW::showOnline()
{
    if(pOnline_->isHidden())
    {
        pOnline_->show();
        //这里要显示发送数据，需要tcpsocket
        PDU *pdu=mkPDU(0);  //没有实际要发的消息,就是发送一个请求。
        pdu->uiMsgType_=ENUM_MSG_TYPE_ALL_ONLINE_RESPEST;
        TcpClient::getinstance().getTcpSocket().write((char *)pdu,pdu->uiPDULen_);
        free(pdu);
        pdu=NULL;
    }
    else
    {
        pOnline_->hide();
    }
}

void FriendLW::serachUser()
{
    // static QString getText(QWidget *parent, const QString &title, const QString &label,
    //                        QLineEdit::EchoMode echo = QLineEdit::Normal,
    //                        const QString &text = QString(), bool *ok = nullptr,
    //                        Qt::WindowFlags flags = Qt::WindowFlags(),
    //                        Qt::InputMethodHints inputMethodHints = Qt::ImhNone);
    strName_=QInputDialog::getText(this,"搜索","搜索用户");
    //打印日志验证
    qDebug()<<strName_;
    if(!strName_.isEmpty())
    {
        //就要打包pdu协议,把相关的值赋值
        PDU *pdu=mkPDU(0);  //实际长度为零是因为，可以放到结构体里面的cadata
        pdu->uiMsgType_=ENUM_MSG_TYPE_SEARCH_USER_RESPEST;
        memcpy(pdu->caData,strName_.toStdString().c_str(),strName_.size());

        //发送pdu
        TcpClient::getinstance().getTcpSocket().write((char *)pdu,pdu->uiPDULen_);
        free(pdu);
        pdu=NULL;
    }
    else
    {
        return;
    }
}
