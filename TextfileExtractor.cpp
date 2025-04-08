#include "TextfileExtractor.h"

//extracts individual intel hex firmware files from combined .txt file ( firmware_versions.txt )
//and copies each firmware version into an individual text file ( compatible with parser class )

bool TextfileExtractor::unwrap(QString path_to_file) {

	QFile firmware_versions(path_to_file);

	if (!firmware_versions.exists()) 
	{
		return false;
	}

	if (!firmware_versions.open(QIODevice::ReadOnly | QIODevice::Text)) 
	{
		return false;
	}

	do 
	{	
		// go through firmware versions file, read identifier string
		separator_string = firmware_versions.readLine();
		
		//if valid, extract version number...
		if (separator_string.contains("Lord")) 
		{
			version_number = separator_string.mid(19, 4).toDouble();
		}

		//create text file for individual firmware version
		QFile single_firmware(QDir::homePath() + "/LordyLink/Firmware/LordyphonFirmwareV" + QString::number(version_number) + ".hex");
		
		if (!single_firmware.open(QIODevice::WriteOnly | QIODevice::Text)) 
		{
			return false;
		}
		
		QTextStream out(&single_firmware);

		do 
		{
			templine = firmware_versions.readLine();
			out << templine; //copy hex data to new file
			out.flush();
		
		} while (!templine.contains(":00000001FF")); // end of firmware data
		
		single_firmware.close();

	}while (!firmware_versions.atEnd()); // END: do { // go through firmware versions file

	firmware_versions.close();
	
	return true;
}