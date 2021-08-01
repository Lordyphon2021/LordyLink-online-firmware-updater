#pragma once

#include <QDialog>

class UpdateDialog : public QDialog
{
	Q_OBJECT

public:
	UpdateDialog(QObject *parent);
	~UpdateDialog();
};
