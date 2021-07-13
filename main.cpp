// simples gui, lädt firmware(intel hex file) von server (www.lordyphon.com) und speichert es in vector, dieser wird zum debuggen im gui ausgwegeben
//TODO: parser implementieren (eigener prototyp)
//TODO: web-zugriff und serial ports mit QT umsetzen, damit kann man sich die boost-library völlig sparen.





#include "LordyLink.h"
#include "Filehandler.h"


#include <QtWidgets/QApplication>






int main(int argc, char *argv[])
{
    
    
    
    QApplication a(argc, argv);
    LordyLink w; 
    w.show();
    
    return a.exec();
}

