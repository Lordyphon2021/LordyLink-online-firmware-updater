#pragma once

#include <QDialog>
#include <QApplication>
#include "ui_DeleteDialog.h"

class DeleteDialog : public QDialog, public Ui::DeleteDialog
{
	Q_OBJECT

public:
	DeleteDialog(QDialog *parent = Q_NULLPTR);
	~DeleteDialog();

private:
	Ui::DeleteDialog ui_delete;
};


