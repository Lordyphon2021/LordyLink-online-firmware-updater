#include "TextfileExtractor.h"

//extracts individual intel hex firmware files from combined .txt file ( firmware_versions.txt )
//and copies each firmware version into an individual text file ( compatible with parser class )

bool TextfileExtractor::unwrap(QString path_to_file) {

	QFile firmware_versions(path_to_file);

	if (!firmware_versions.exists()) {
		qDebug() << "firmwareversions not found";
		return false;
	}

	if (!firmware_versions.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "firmwareversions not open";
		return false;
	}

	do { // go through firmware versions file
		//read identifier string
		separator_string = firmware_versions.readLine();
		qDebug() << "sep_string: " << separator_string;
		//if valid, extract version number...
		if (separator_string.contains("LORDYPHON")) {
			version_number = separator_string.mid(23, 2).toInt();
			qDebug() << "version_number" << version_number;
		}
		else
			qDebug() << "separator not found";
		
		//create text file for individual firmware version
		QFile single_firmware(QDir::homePath() + "/LordyLink/Firmware/Version" + QString::number(version_number) + ".txt");
		qDebug() << QDir::homePath() + "/LordyLink/Firmware/Version" + QString::number(version_number) + ".txt";
		
		if (!single_firmware.open(QIODevice::WriteOnly | QIODevice::Text)) {
			qDebug() << "file not found";
			return false;
		}
		
		QTextStream out(&single_firmware);

		do {
			templine = firmware_versions.readLine();
			out << templine; //copy hex data to new file
		} while (!templine.contains(":00000001FF")); // end of firmware data
		
		single_firmware.close();

	} while (!firmware_versions.atEnd()); // END: do { // go through firmware versions file

	firmware_versions.close();
	//firmware_versions.remove();
	
	return true;

}