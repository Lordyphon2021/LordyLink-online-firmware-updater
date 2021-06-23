#include "LordyLink.h"


#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    
   
    
    
    QApplication a(argc, argv);
    
    FileHandler f_handle;  //Filehandler instanzieren
    CParser pars;
    LordyLink_derived w(&f_handle, &pars); //übergebe filehandler- und parserinstanzen by reference
    
    
        
    
    w.show();
    return a.exec();
}

