#include "DeleteDialog.h"

DeleteDialog::DeleteDialog(QDialog *parent)
	: QDialog(parent)
{
	ui_delete.setupUi(this);
	
	connect(ui_delete.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui_delete.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

DeleteDialog::~DeleteDialog()
{
}


