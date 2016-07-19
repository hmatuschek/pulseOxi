#include <QApplication>
#include "portdialog.h"
#include "pulse.h"
#include "mainwindow.h"


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  Pulse pulse(0);
  MainWindow mainwin(pulse);
  mainwin.show();

  app.exec();

  return 0;
}
