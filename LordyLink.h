#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_LordyLink.h"
#include <QPushButton>
#include <qlineedit.h>
#include "Filehandler.h"
#include "Parser.h"


class LordyLink : public QMainWindow
{
    Q_OBJECT

public:
    LordyLink(QWidget *parent = Q_NULLPTR);
   
protected:
    Ui::LordyLinkClass ui;
    
    
protected slots:
    
    virtual bool downloadFile() = 0;
    virtual void DisplayContainerData() = 0;
   
};

class LordyLink_derived : public LordyLink
{
  
private:
    FileHandler* p_filehandler;
    bool downloadFile() override;
    void DisplayContainerData() override;
    CParser* parser;

public:  
    LordyLink_derived(FileHandler* fhandle, CParser* pars) 
        : p_filehandler(fhandle), parser(pars)
    {}

};
