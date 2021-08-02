#include "QUpdateDialog.h"
#include <QString>


QUpdateDialog::QUpdateDialog(QDialog *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle("Firmware Versions:");
	QObject::connect(ui.QInstallButton, SIGNAL(clicked()), this, SLOT(accept()));
	QObject::connect(ui.QCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	

	
}

QUpdateDialog::~QUpdateDialog()
{
}

