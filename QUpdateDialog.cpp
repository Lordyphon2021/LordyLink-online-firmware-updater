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
	download_firmware();
	
}

QUpdateDialog::~QUpdateDialog()
{
}


void QUpdateDialog::download_firmware()
{
	
	QString home = QDir::homePath() + "/LordyLink/Firmware";
	QDir directory(home);
	QStringList txtfiles = directory.entryList(QStringList() << "*.txt", QDir::Files);
	
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
    QObject::connect(ui.tableView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(return_path(const QModelIndex)));
   
	

	

}