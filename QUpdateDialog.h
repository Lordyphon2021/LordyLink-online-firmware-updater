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

		//QString get_firmware_version() { return selected_firmware; }

	public slots:
	
	
	void send_path_to_lordylinkcpp(const QModelIndex mindex){
	
		selected_firmware = ui.tableView->model()->index(mindex.row(), 0).data().toString();
		qDebug() << "emitting path string from dialog to lordylink.cpp" << selected_firmware;
		ui.QInstallButton->setEnabled(true);
	
		emit selected_version(selected_firmware);
	}

	signals:
		void selected_version(QString);

	private:
		Ui::QUpdateDialog ui;
	
		void downloaded_firmware_versions();
	
		QStandardItemModel* model;
		QString selected_firmware;
};
