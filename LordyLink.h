#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_LordyLink.h"
#include <QPushButton>
#include <qlineedit.h>
#include "Parser.h"

class LordyLink : public QMainWindow
{
    Q_OBJECT

public:
    LordyLink(QWidget *parent = Q_NULLPTR);
    void get_Parser_ptr(CParser* parser);

private:
    Ui::LordyLinkClass ui;
    CParser* p;

private slots:
    
    bool downloadFile();
    void DisplayContainerData();
   
};
