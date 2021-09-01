#pragma once

#include <QDialog>
#include <QApplication>
#include<QMessageBox>
#include <QListWidget>
#include "ui_QUpdateDialog.h"
#include "UsbThread.h"
#include <QStandardItemModel>
#include <QDir>
#include <QModelIndex>


class QUpdateDialog : public QDialog, public Ui::QUpdateDialog
{
	Q_OBJECT


public:
	QUpdateDialog(QDialog* parent = Q_NULLPTR);
	~QUpdateDialog();

	QString get_firmware_version() { return selected_firmware; }

public slots:
	
	
void return_path(const QModelIndex mindex)
{
	selected_firmware = ui.tableView->model()->index(mindex.row(), 0).data().toString();
	qDebug() << selected_firmware;
	ui.QInstallButton->setEnabled(true);
}

signals:
	void selected_version(QString);

private:
	Ui::QUpdateDialog ui;
	void download_firmware();
	QStandardItemModel* model;
	QString selected_firmware;
	
	
	


	
	
};
