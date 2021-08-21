#include "QUpdateDialog.h"
#include <QString>


QUpdateDialog::QUpdateDialog(QDialog *parent)
	: QDialog(parent)
{
	
	
	
	ui.setupUi(this);
	setWindowTitle("Firmware Versions:");
	download();
	//QObject::connect(ui.QInstallButton, SIGNAL(clicked()), SLOT(download()));
	QObject::connect(ui.QInstallButton, SIGNAL(clicked()), this, SLOT(accept()));
	QObject::connect(ui.QCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	

	
}
void QUpdateDialog::download()
{

	filehandler = new Filehandler(ui.QInternetStatus);

	QString file_location = "ftp://stefandeisenberger86881@ftp.lordyphon.com/firmware_versions/lordyphon_proto.txt";
	QString file_path = "C:/Users/trope/OneDrive/Desktop/Neuer Ordner/lordyphon_proto.txt";
	QString folder_location = "ftp://stefandeisenberger86881@ftp.lordyphon.com";
	QString folder_path = "C:/Users/trope/OneDrive/Desktop/Neuer Ordner/version_list.txt";

	
	
	
	
filehandler->download(file_location, file_path);
	//filehandler->download(folder_location, folder_path);


	


}
QUpdateDialog::~QUpdateDialog()
{
}

