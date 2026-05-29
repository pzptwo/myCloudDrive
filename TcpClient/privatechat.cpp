#include "privatechat.h"
#include "ui_privatechat.h"

PrivateChat::PrivateChat(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

PrivateChat &PrivateChat::getInstance()
{
    static PrivateChat instance;
    return instance;
}

QString &PrivateChat::getChatName(QString strname)
{
    strChatName_=strname;
    strSelfName_=TcpClient::getinstance().getstrLoginName();
}

void PrivateChat::updateMsg(PDU *pdu)
{
    if(pdu==nullptr)
    {
        return;
    }
    char caSendName[32]={'\0'};
    memcpy(caSendName,pdu->caData,32);

    QString strMsg=QString("%1 says %2").arg(caSendName).arg((char *)pdu->caMsg);
    ui->strMsg_te->append(strMsg);
}

void PrivateChat::on_sendMsg_Pd_clicked()
{
    //这里获得相关的
    QString strMsg=ui->sendMsg_le->text();
    ui->sendMsg_le->clear();
    if(!strMsg.isEmpty())
    {
        //类型不同，只传大小
        PDU *pdu=mkPDU(strMsg.size()+1);
        //对自定义的pdu进行设计
        pdu->uiMsgType_=ENUM_MSG_TYPE_PRIVATE_CHAT_RESPEST;
        memcpy((char *)(pdu->caMsg),strMsg.toStdString().c_str(),strMsg.size());

        //我这里设定了，先用，再去tcpclient里面传参
        memcpy(pdu->caData,strSelfName_.toStdString().c_str(),strSelfName_.size());
        memcpy(pdu->caData+32,strChatName_.toStdString().c_str(),strChatName_.size());

        TcpClient::getinstance().getTcpSocket().write((char *)pdu,pdu->uiPDULen_);
        free(pdu);
        pdu=nullptr;
    }
    else
    {
        QMessageBox::warning(this,"发送消息","发送消息不能为空");
    }
}

