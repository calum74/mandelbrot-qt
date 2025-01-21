#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
  fclose(stderr); // Get rid of unacceptable Apple stderr noise
  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  return a.exec();
}
