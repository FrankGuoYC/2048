#include <QApplication>
#include "sound.h"

class MainWindow;
#include "mainwindow.h"
MainWindow *w;  //Declared globally because lots of classes would use it

int main(int argc, char *argv[])
{
    QApplication prog(argc, argv);
    w = new MainWindow;
    w->show();
    return prog.exec();
}
