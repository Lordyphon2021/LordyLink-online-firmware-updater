#include "QNoHardwareDialog.h"

QNoHardwareDialog::QNoHardwareDialog(QDialog *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	QObject::connect(ui.TryAgainPushButton, SIGNAL(clicked()), this, SLOT(accept()));
	QObject::connect(ui.QuitPushButton, SIGNAL(clicked()), this, SLOT(reject()));
}

QNoHardwareDialog::~QNoHardwareDialog()
{
}
