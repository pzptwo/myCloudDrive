#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "tcpclient.h"
#include <QMessageBox>
#include <QInputDialog>
#include "protocol.h"

class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void updateFileList(PDU *pdu);
    void ClearEntryName();
    QString getEntryName();
private:
    QListWidget *pBookListW_;
    QPushButton *pReturnPB_;
    QPushButton *pCreateDirPB_;
    QPushButton *pDelDirPB_;
    QPushButton *pRanamePB_;
    QPushButton *pFlushFilePB_;
    QPushButton *pUploadPB_;
    QPushButton *pDownLoadPB_;
    QPushButton *pDelFilePB_;
    QPushButton *pShareFilePB_;

    QString strEntryName_;
signals:
public slots:
    void createDir();
    void flushDir();
    void delDir();
    void reName();
    //需要双击，进行相关的参数
    void entryDir(const QModelIndex &index);
};

#endif // BOOK_H
