#include "mainwindow.h"
#include <QApplication>

MainWindow *w;

int main(int argc, char *argv[])
{
    QApplication prog(argc, argv);
    MainWindow *w = new MainWindow;
    w->show();
    return prog.exec();
}
