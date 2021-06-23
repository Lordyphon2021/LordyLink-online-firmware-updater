#include "LordyLink.h"
#include "Filehandler.h"
#include <boost/asio.hpp>
#include <qstring.h>
#include<qlistwidget.h>
#include <QApplication>

using namespace std;
using namespace boost::asio;




LordyLink::LordyLink(QWidget *parent)
    : QMainWindow(parent)
{
    
    
    ui.setupUi(this);
    QObject::connect(ui.Q_UpdateLordyphonButton, SIGNAL(clicked()), SLOT(downloadFile()));
    
    
}


bool LordyLink_derived::downloadFile()
{
    ui.Q_UpdateFeedback->clear();
    ui.Q_UpdateFeedback->setText("connecting to server... ");
    qApp->processEvents();
    
    if (p_filehandler->GetPageContent()) {
        ui.Q_UpdateFeedback->clear();
        ui.Q_UpdateFeedback->insert("download complete");
       
        p_filehandler->writeFileToContainer();
        DisplayContainerData();
        
        return true;
    }
    
    ui.Q_UpdateFeedback->insert("connection error");
    
    return false;
}


void LordyLink_derived::DisplayContainerData()
{
    for (size_t i = 0; i < (p_filehandler->return_record_coll_size()); ++i )   {
        QString qtemp_str = QString::fromStdString(p_filehandler->record_collection_at(i));
        ui.textEdit->insertPlainText(qtemp_str);
    }
}

