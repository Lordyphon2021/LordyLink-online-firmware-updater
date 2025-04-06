

#include "LordyLink.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LordyLink w; 
   
    QFont font("Lucida Typewriter", 8, QFont::Bold);
    w.setFont(font);
    w.setWindowTitle("LordyLink V1.00");
   
    w.show();
    
    return a.exec(); 
}

