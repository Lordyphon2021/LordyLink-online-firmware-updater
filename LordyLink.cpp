#include "LordyLink.h"


#include <qstring.h>
#include<qlistwidget.h>
#include <QApplication>


using namespace std;





LordyLink::LordyLink(QWidget *parent)
    : QMainWindow(parent)
{
    
    
    ui.setupUi(this);
    QObject::connect(ui.Q_UpdateLordyphonButton, SIGNAL(clicked()), SLOT(download_wrapper()));
    filehandler = new Filehandler(ui.Q_UpdateFeedback);
    
}

LordyLink::~LordyLink()
{

    delete filehandler;

}






void LordyLink::download(Filehandler* filehandler)
{
    QString location = "ftp://stefandeisenberger86881@ftp.lordyphon.com/lordyphon_proto.txt";
    QString path = "C:/Users/trope/OneDrive/Desktop/Neuer Ordner/lordyphon_proto.txt"; //Replace this with your file
    filehandler->download(location, path);
    
}


void LordyLink::download_wrapper()
{
    download(filehandler);
}

