#pragma once
#include <QString>
#include <QFile>
#include <QDir>
#include <qtextstream.h>
#include <qDebug>



//Application "FirmwareWrapper" takes path to hexfile location, reads all files and copies
//them into a new textfile "firmware_collection" separated by identifier strings
//and uploads it onto the manufacturer's server.



//this class extracts all firmware files from text collection and stores them into individual text files in the firmware folder ( compatibel with the the parser class )

class TextfileExtractor
{
public:
	
	bool unzipper(QString path_to_file);
	

private:
	int version_number;
	QString templine;
	QString separator_string;
};

