#pragma once

#include <QDialog>
#include <QApplication>
#include "ui_QtSendDialog.h"

class QtSendDialog : public QDialog, public Ui::QtSendDialog
{
	Q_OBJECT


public:
	QtSendDialog(QDialog *parent = Q_NULLPTR);
	~QtSendDialog();

private:
	Ui::QtSendDialog ui_send;
};
