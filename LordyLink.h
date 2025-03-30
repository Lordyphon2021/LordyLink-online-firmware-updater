#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_LordyLink.h"
#include <QPushButton>
#include <qlineedit.h>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QListWidget>
#include <QString>
#include <QUrl>
#include <QDateTime>
#include <QFile>
#include <String>
#include <QDebug>
#include <QSerialPort.h>
#include <QIODevice.h>
#include <QProgressBar>
#include "Parser.h"
#include "Serial.h"
#include "QUpdateDialog.h"
#include "Downloader.h"
#include "QtSendDialog.h"
#include "UsbThread.h"
#include "QNoHardwareDialog.h"
#include <QStandardItemModel>
#include <QDir>
#include <QModelIndex>
#include <QDesktopServices>
#include <QTextBrowser> 




class LordyLink : public QMainWindow
{
    
private:
    Q_OBJECT
    
public:
    LordyLink(QWidget *parent = Q_NULLPTR);
    
    ~LordyLink() {
        
        file_cleanup();  //delete downloads, firmware versions and empty set files
        usb_port->quit_message();
        usb_port->close_usb_port();
    }
    
    Ui::LordyLinkClass ui;
    
signals:

public slots:
    //MAIN FEATURES
    void OnUpdateButton();
    void OnGetSetButton();
    void OnSendSetButton();
    //Set QTableView
    void addNewSet(QString filename);
    void renameStart(const QModelIndex);
    void renameEnd(QStandardItem*);
    void selectItemToSend(const QModelIndex mindex);
    void selectItemToDelete(const QModelIndex mindex);
    void deleteSet();
    //GUI Feedback
    void ProgressBar_OnValueChanged(int val) { ui.QInstallProgressBar->setValue(val); }
    void ProgressBar_OnsetMax(int val) { ui.QInstallProgressBar->setMaximum(val); }
    void OnsetLabel(QString message){ ui.QInstallLabel->setText(message); }
    void OnActivateButtons();
    void OnDeactivateButtons();
    void OnHideAbortButton() { ui.abort_pushButton->hide(); }
    //void activate_install_button(){ ui.Q_UpdateLordyphonButton->setEnabled(true); }
    //HELPER
    void set_firmware_path_from_dialog(QString path){ firmware_path = path; }
    void OnRemoteMessageBox(QString message);
    void try_download();
    void check_for_lordyphon();
    void check_manufacturer_ID();
    void hotplugtimer_on();
    void hotplugtimer_off();
    void on_download_status(bool download_status) { download_done = download_status; }
    void on_download_status_message(QString msg) { downloader_message = msg; }
    void onAboutTriggered();
    void checkConnection();

private:
    
    SerialHandler* usb_port = nullptr;
    QUpdateDialog* update_dialog = nullptr;
    QNoHardwareDialog* dialog_no_hardware_found = nullptr;
    Downloader* filehandler = nullptr;
    QStandardItemModel* model = nullptr;
    QString home;
    QString oldName;
    QString selected_set;
    QTimer* download_timer = nullptr;
    QTimer* hot_plug_timer = nullptr;
    QString firmware_path;
    QString to_delete = "";
    QString downloader_message;

    bool lordyphon_connected = false;
    bool update_mode = false;
    bool download_done= false;
    const int firmware_size = 188459;

    inline void show_messagebox(QString message, QString button_text = "Ok", bool quit_button = false)
    {
            QMessageBox messagebox;
            QFont font("Lucida Typewriter", 8, QFont::Bold);
            messagebox.setFont(font);
            messagebox.setButtonText(1, button_text);
            messagebox.setText(message);

            if (quit_button == true)
            {
                QPushButton* button = messagebox.addButton("Quit", QMessageBox::ActionRole);
                QObject::connect(button, &QPushButton::clicked, []() {exit(1);});
            }

            messagebox.exec();
    }

    class AboutDialog : public QDialog {
        public:
            AboutDialog(QWidget* parent = nullptr) : QDialog(parent) {
                setWindowTitle("About");

                // Layout für den Dialog
                QVBoxLayout* layout = new QVBoxLayout(this);

                // QTextBrowser für HTML-Inhalt
                QTextBrowser* textBrowser = new QTextBrowser(this);
                textBrowser->setOpenExternalLinks(true); // Links werden automatisch geöffnet
                textBrowser->setText("LordyLink (c)2025 by Stefan Deisenberger<br>"
                    "Check it out on GitHub: <a href=\"https://github.com/Lordyphon2021/LordyLink-online-firmware-updater\">GitHub Link</a><br>"
                    "This application uses Qt, licensed under the <a href=\"http://www.gnu.org/licenses/gpl-2.0.html\">GPL 2.0 License</a>");

                layout->addWidget(textBrowser);

                // Ok-Button
                QPushButton* okButton = new QPushButton("OK", this);
                layout->addWidget(okButton);
                connect(okButton, &QPushButton::clicked, this, &AboutDialog::accept);
            }
    };

    inline void delay(int millisecondsWait){
        QEventLoop loop;
        QTimer t;
        t.connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
        t.start(millisecondsWait);
        loop.exec();
    }
    void file_cleanup() {
        //remove downloads
        QDir firmware(QDir::homePath() + "/LordyLink/Firmware");
        firmware.setNameFilters(QStringList() << "*.*");
        //firmware.setFilter(QDir::Files);

        foreach(QString firmwareFile, firmware.entryList())
            firmware.remove(firmwareFile);

        //cleanup empty files from crashes

        QFile firmware_versions = QDir::homePath() + "/LordyLink/downloads/firmware_versions.txt";
        if (firmware_versions.exists())
            firmware_versions.remove();

        QDir set_path(QDir::homePath() + "/LordyLink/Sets");
        set_path.setNameFilters(QStringList() << "*.*");


        foreach(QString setfile, set_path.entryList()) {
            QFile tempfile(set_path.absoluteFilePath(setfile));
            tempfile.open((QIODevice::ReadWrite | QIODevice::Text));

            if (tempfile.size() == 0) {
                tempfile.close();
                tempfile.remove();

            }
            else
                tempfile.close();
        }

    }
};

