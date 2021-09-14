

#include "LordyLink.h"



#include <QtWidgets/QApplication>






int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LordyLink w; 
   
    w.setWindowTitle("LordyLink 1.0");
   
    w.show();
    
    return a.exec(); 
}

