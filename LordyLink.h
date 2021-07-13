#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_LordyLink.h"
#include <QPushButton>
#include <qlineedit.h>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDateTime>
#include <QFile>
#include <QDebug>
#include "Filehandler.h"


class LordyLink : public QMainWindow
{
    
private:
    Q_OBJECT
    

public:
    LordyLink(QWidget *parent = Q_NULLPTR);
    Ui::LordyLinkClass ui;
    

   

signals:

public slots:
   
    void download_wrapper();
   

private slots:
    

private:
    
   
    Filehandler* filehandler;
    QNetworkAccessManager* manager;
   
    void download(Filehandler* filehandler);
   
};

