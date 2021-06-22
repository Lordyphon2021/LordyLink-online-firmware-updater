#include "LordyLink.h"
#include "Parser.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    
    CParser pars;  //Parser instanzieren
    
    
    QApplication a(argc, argv);
    LordyLink w;
    
    w.get_Parser_ptr(&pars); //Pointer auf pars-objekt übergeben
    //pars.get_gui_ptr(&w);     
    
    w.show();
    return a.exec();
}

