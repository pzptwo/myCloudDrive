#include "book.h"
#include <QListWidgetItem>

Book::Book(QWidget *parent)
    : QWidget{parent}
{
    pBookListW_=new QListWidget;
    pReturnPB_=new QPushButton("返回");
    pCreateDirPB_=new QPushButton("创建文件夹");
    pDelDirPB_=new QPushButton("删除文件夹");
    pRanamePB_=new QPushButton("重命名文件");
    pFlushFilePB_=new QPushButton("刷新文件");
    QVBoxLayout *pDirVBL=new QVBoxLayout;
    pDirVBL->addWidget(pReturnPB_);
    pDirVBL->addWidget(pCreateDirPB_);
    pDirVBL->addWidget(pDelDirPB_);
    pDirVBL->addWidget(pRanamePB_);
    pDirVBL->addWidget(pFlushFilePB_);

    pUploadPB_=new QPushButton("上传文件");
    pDownLoadPB_=new QPushButton("下载文件");
    pDelFilePB_=new QPushButton("删除文件");
    pShareFilePB_=new QPushButton("共享文件");
    QVBoxLayout *pFileVBL=new QVBoxLayout;
    pFileVBL->addWidget(pUploadPB_);
    pFileVBL->addWidget(pDownLoadPB_);
    pFileVBL->addWidget(pDelFilePB_);
    pFileVBL->addWidget(pShareFilePB_);

    QHBoxLayout *pMain=new QHBoxLayout;
    pMain->addWidget(pBookListW_);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);

    setLayout(pMain);
    connect(pCreateDirPB_,&QPushButton::clicked,this,&Book::createDir);
    connect(pFlushFilePB_,&QPushButton::clicked,this,&Book::flushDir);
    connect(pDelDirPB_,&QPushButton::clicked,this,&Book::delDir);
    connect(pRanamePB_,&QPushButton::clicked,this,&Book::reName);
    //void doubleClicked(const QModelIndex &index);
    connect(pBookListW_,&QListWidget::doubleClicked,this,&Book::entryDir);
}

void Book::updateFileList(PDU *pdu)
{
    if(pdu==nullptr)
    {
        return;
    }
    QListWidgetItem *pItemTmp=nullptr;
    int row=pBookListW_->count()-1;
    for(row;row>=0;row--)
    {
        //inline void QListWidget::removeItemWidget(QListWidgetItem *aItem)
        //QListWidgetItem *item(int row) const;
        pItemTmp=pBookListW_->item(row);
        pBookListW_->removeItemWidget(pItemTmp);
        delete pItemTmp;
    }
    FileInfo *fileInfo=nullptr;
    int iCount=pdu->uiMsgLen_/sizeof(FileInfo);
    for(int i=0;i<iCount;i++)
    {
        fileInfo=(FileInfo *)pdu->caMsg+i;
        //打印日志验证
        qDebug()<<fileInfo->caFileName<<fileInfo->iFileType;
        QListWidgetItem *pItem=new QListWidgetItem;
        if(fileInfo->iFileType==0)
        {
            pItem->setIcon(QIcon(QPixmap(":/map/dir.jpg")));
        }
        else if(fileInfo->iFileType==1)
        {
            pItem->setIcon(QIcon(QPixmap(":/map/reg.png")));
        }
        pItem->setText(fileInfo->caFileName);
        pBookListW_->addItem(pItem);
    }
}

void Book::ClearEntryName()
{
    strEntryName_.clear();
}

QString Book::getEntryName()
{
    return strEntryName_;
}

void Book::createDir()
{
    //需要登录名，目录信息，新建文件名字

    //class Q_WIDGETS_EXPORT QInputDialog : public QDialog
    // static QString getText(QWidget *parent, const QString &title, const QString &label,
    //                        QLineEdit::EchoMode echo = QLineEdit::Normal,
    //                        const QString &text = QString(), bool *ok = nullptr,
    //                        Qt::WindowFlags flags = Qt::WindowFlags(),
    //                        Qt::InputMethodHints inputMethodHints = Qt::ImhNone);
    QString strNewDir=QInputDialog::getText(this,"新文件夹","新文件夹名字");
    if(strNewDir.size()<32)
    {
        if(!strNewDir.isEmpty())
        {
            QString strLoginName=TcpClient::getinstance().getstrLoginName();
            QString strCurPath=TcpClient::getinstance().getCurPath();
            PDU *pdu=mkPDU(strCurPath.size()+1);
            pdu->uiMsgType_=ENUM_MSG_TYPE_CREATE_DIR_RESPEST;
            memcpy(pdu->caData,strLoginName.toStdString().c_str(),strLoginName.size());
            memcpy(pdu->caData+32,strNewDir.toStdString().c_str(),strNewDir.size());

            memcpy((char *)(pdu->caMsg),strCurPath.toStdString().c_str(),strCurPath.size());
            TcpClient::getinstance().getTcpSocket().write((char *)pdu,pdu->uiPDULen_);
            free(pdu);
            pdu=nullptr;
        }
        else
        {
            QMessageBox::warning(this,"新文件夹","新文件夹名字不能为空");
        }
    }
    else
    {
        QMessageBox::warning(this,"新文件夹","新文件夹名字不能超过32");
    }
}

void Book::flushDir()
{
    //获得路径传给服务器
    QString strPath=TcpClient::getinstance().getCurPath();
    PDU *pdu=mkPDU(strPath.size()+1);
    pdu->uiMsgType_=ENUM_MSG_TYPE_FLUSH_DIR_RESPEST;
    memcpy((char*)(pdu->caMsg),strPath.toStdString().c_str(),strPath.size());
    TcpClient::getinstance().getTcpSocket().write((char *)pdu,pdu->uiPDULen_);
    free(pdu);
    pdu=nullptr;

}

void Book::delDir()
{
    //这里要获得路径及选择的item,因为这里的是widgetList
    QString strPath=TcpClient::getinstance().getCurPath();
    //QListWidgetItem *currentItem() const;
    QListWidgetItem* pItem=pBookListW_->currentItem();
    if(pItem==nullptr)
    {
        QMessageBox::warning(this,"选择的文件","选择的文件不能为空");
        return ;
    }
    else
    {
        QString getName=pItem->text();
        PDU *pdu=mkPDU(strPath.size()+1);
        pdu->uiMsgType_=ENUM_MSG_TYPE_DEL_DIR_RESPEST;
        memcpy((char *)pdu->caMsg,strPath.toStdString().c_str(),strPath.size());
        memcpy(pdu->caData,getName.toStdString().c_str(),32); //这里默认显示32

        TcpClient::getinstance().getTcpSocket().write((char *)pdu,pdu->uiPDULen_);
        free(pdu);
        pdu=nullptr;

    }
}

void Book::reName()
{
    QString strPath=TcpClient::getinstance().getCurPath();
    //QListWidgetItem *currentItem() const;
    QListWidgetItem* pItem=pBookListW_->currentItem();
    if(pItem==nullptr)
    {
        QMessageBox::warning(this,"选择的重名的文件","选择的重名的文件不能为空");
        return ;
    }
    else
    {
        QString strOldName=pItem->text();
        QString strNewName=QInputDialog::getText(this,"重命名文件","请输入新的文件名字");

        PDU *pdu=mkPDU(strPath.size()+1);
        pdu->uiMsgType_=ENUM_MSG_TYPE_RENAME_FILE_RESPEST;
        memcpy((char *)pdu->caMsg,strPath.toStdString().c_str(),strPath.size());
        //这里要判断32
        memcpy(pdu->caData,strOldName.toStdString().c_str(),32);
        memcpy(pdu->caData+32,strNewName.toStdString().c_str(),32);

        TcpClient::getinstance().getTcpSocket().write((char *)pdu,pdu->uiPDULen_);
        free(pdu);
        pdu=nullptr;

        //flushDir();
    }
}

void Book::entryDir(const QModelIndex &index)
{
    //注意strCurPath_=QString("./%1").arg(strLoginName_);，所以要更新strCurPath
    //inline QVariant QModelIndex::data(int arole) const

        //首先双击获得文件名
        QString strName=index.data().toString();
        qDebug()<<strName;
        QString strPath=TcpClient::getinstance().getCurPath();

        PDU *pdu=mkPDU(strPath.size()+1);
        pdu->uiMsgType_=ENUM_MSG_TYPE_ENTRY_DIR_RESPEST;
        memcpy((char *)pdu->caMsg,strPath.toStdString().c_str(),strPath.size());
        memcpy(pdu->caData,strName.toStdString().c_str(),strName.size());

        TcpClient::getinstance().getTcpSocket().write((char *)pdu,pdu->uiPDULen_);
        free(pdu);
        pdu=nullptr;
}
