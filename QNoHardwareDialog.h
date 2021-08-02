#pragma once

#include <QDialog>
#include "ui_QNoHardwareDialog.h"

class QNoHardwareDialog : public QDialog, public Ui::QNoHardwareDialog
{
	Q_OBJECT

		


private:

	Ui::QNoHardwareDialog ui;

public:
	QNoHardwareDialog(QDialog *parent = Q_NULLPTR);
	~QNoHardwareDialog();
};
