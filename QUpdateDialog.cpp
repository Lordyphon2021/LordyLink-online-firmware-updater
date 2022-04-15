#include "QUpdateDialog.h"
#include <QString>


QUpdateDialog::QUpdateDialog(QDialog *parent)
	: QDialog(parent)
{
	
	
	
	ui.setupUi(this);
	setWindowTitle("Firmware Versions:");

	
	QObject::connect(ui.QInstallButton, SIGNAL(clicked()), this, SLOT(accept()));
	QObject::connect(ui.QCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	
	ui.QInstallButton->setDisabled(true);
	downloaded_firmware_versions();
	
}

QUpdateDialog::~QUpdateDialog()
{
}


void QUpdateDialog::downloaded_firmware_versions()
{
	
	QString home = QDir::homePath() + "/LordyLink/Firmware";
	QDir directory(home);
	QStringList txtfiles = directory.entryList(QStringList() << "*.*", QDir::Files);
	
	model = new QStandardItemModel();
    
	//read textfiles from directory
	
	foreach(QString filename, txtfiles) {
        qDebug() << filename;
        QStandardItem* itemname = new QStandardItem(filename);
        itemname->setFlags(itemname->flags() | Qt::ItemIsEditable);
        model->appendRow(QList<QStandardItem*>() << itemname);
    }
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("versions: "));

    //connect qtableview with model
    ui.tableView->setModel(model);
	// set column size
	for (int col = 0; col < model->rowCount(); col++)
		ui.tableView->setColumnWidth(col, 320);
	
    //single click to select item
    QObject::connect(ui.tableView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(send_path_to_lordylinkcpp(const QModelIndex)));
   
	

	

}