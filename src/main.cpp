#include <iostream>
#include <QApplication>

//#include "mainwindow.h"
#include "SwatchMainWindow.h"

int main(int argc, char** argv)
{
	QApplication a(argc, argv);
	SwatchMainWindow smw;
	smw.show();
	return a.exec();
}