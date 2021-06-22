#include "LordyLink.h"
#include "Parser.h"
#include <boost/asio.hpp>
#include <qstring.h>
#include<qlistwidget.h>
using namespace std;
using namespace boost::asio;




LordyLink::LordyLink(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    QObject::connect(ui.Q_UpdateLordyphonButton, SIGNAL(clicked()), SLOT(downloadFile()));
   
   
}
bool LordyLink::downloadFile()
{
    ui.Q_UpdateFeedback->clear();
    ui.Q_UpdateFeedback->setText("connecting to lordyphon.com ");
    
    
    if (p->GetPageContent()) {
        ui.Q_UpdateFeedback->clear();
        ui.Q_UpdateFeedback->insert("download complete");
       
        p->writeFileToContainer();
        DisplayContainerData();
        
        return true;
    }
    
    ui.Q_UpdateFeedback->insert("connection error");
    
    return false;
}


void LordyLink::DisplayContainerData()
{
    

    for (size_t i = 0; i < (p->return_record_coll_size()); ++i )   {
        QString qtemp_str = QString::fromStdString(p->record_collection_at(i));
        ui.textEdit->insertPlainText(qtemp_str);
    }
   
}

void LordyLink::get_Parser_ptr(CParser* parser)
{

    p = parser;

}