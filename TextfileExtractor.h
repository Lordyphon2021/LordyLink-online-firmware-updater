#pragma once
#include <QString>
#include <QFile>
#include <QDir>
#include <qtextstream.h>
#include <qDebug>



//Application "FirmwareWrapper" takes path to hexfile location, reads all files and copies
//them into a new textfile "firmware_collection" separated by an identifier (line format: begin: XXXBEGINXXX, XXXVERS01XXX to XXXVERS99XXX, XXXEOFXXX)
//and uploads it onto the manufaturer's server.



//this class extracts all firmware files from text collection and stores them into individual text files in the firmware folder compatibel with the the parser class.

class TextfileExtractor
{
public:
	
	bool unzipper(QString path_to_file) {
		
		QFile firmware_versions(path_to_file);
		
		if (!firmware_versions.exists()) {
			qDebug() << "firmwareversions not found";
			return false;

		}
		
		
		if (!firmware_versions.open(QIODevice::ReadOnly | QIODevice::Text)) {
			qDebug() << "firmwareversions not open";
			return false;
		}
		
		do {
			
			separator_string = firmware_versions.readLine();
			qDebug() << "sep_string: " << separator_string;
			if (separator_string.contains("LORDYPHON")) {
				version_number = separator_string.mid(23, 2).toInt();
				qDebug() << "version_number" << version_number;

			}
			else
				qDebug() << "separator not found";

			QFile single_firmware(QDir::homePath() + "/LordyLink/Firmware/Version" + QString::number(version_number) + ".txt");
			qDebug() << QDir::homePath() + "/LordyLink/Firmware/Version" + QString::number(version_number) + ".txt";
			if (!single_firmware.open(QIODevice::WriteOnly | QIODevice::Text)) {
				qDebug() << "file not found";
				return false;
			}
			QTextStream out(&single_firmware);
			
			do {
				templine = firmware_versions.readLine();
				out << templine;
			}while (!templine.contains(":00000001FF"));
			single_firmware.close();
		
		
		
		} while (!firmware_versions.atEnd());
		
		firmware_versions.close();
		firmware_versions.remove();
		return true;

	}


private:
	int version_number;
	QString templine;
	QString separator_string;
};

