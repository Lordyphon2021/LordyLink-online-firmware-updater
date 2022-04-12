#include "TextfileExtractor.h"



bool TextfileExtractor::unzipper(QString path_to_file) {

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
		} while (!templine.contains(":00000001FF"));
		single_firmware.close();



	} while (!firmware_versions.atEnd());

	firmware_versions.close();
	firmware_versions.remove();
	return true;

}