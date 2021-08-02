#pragma once

#include <QDialog>
#include <QApplication>
#include<QMessageBox>
#include "ui_QUpdateDialog.h"

class QUpdateDialog : public QDialog, public Ui::QUpdateDialog
{
	Q_OBJECT

public slots:
	


private:
	Ui::QUpdateDialog ui;

public:
	QUpdateDialog(QDialog *parent = Q_NULLPTR);
	~QUpdateDialog();
};
