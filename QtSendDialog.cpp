#include "QtSendDialog.h"

QtSendDialog::QtSendDialog(QDialog* parent)
	: QDialog(parent)
{
	ui_send.setupUi(this);
	
	QObject::connect(ui_send.pushButton_YES, SIGNAL(clicked()), this, SLOT(accept()));
	QObject::connect(ui_send.pushButton_NO, SIGNAL(clicked()), this, SLOT(reject()));

	setWindowTitle("alert");
}

QtSendDialog::~QtSendDialog()
{
}
