#pragma once

#include <QDialog>
#include <QApplication>
#include<QMessageBox>
#include <QListWidget>
#include "ui_QUpdateDialog.h"
#include "Filehandler.h"

class QUpdateDialog : public QDialog, public Ui::QUpdateDialog
{
	Q_OBJECT

public slots:
	


private:
	Ui::QUpdateDialog ui;
	Filehandler* filehandler = nullptr;

public:
	QUpdateDialog(QDialog *parent = Q_NULLPTR);
	~QUpdateDialog();
	void download();
	
};
