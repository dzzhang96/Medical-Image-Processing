#include "MedicalImageProcessing.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MedicalImageProcessing w;
	w.show();
	return a.exec();
}
