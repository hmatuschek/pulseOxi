#include <QApplication>
#include "portdialog.h"
#include "pulse.h"
#include "mainwindow.h"


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QString portname;

  // Get portname
  PortDialog dialog;
  if (QDialog::Accepted != dialog.exec()) {
    return 0;
  }
  portname = dialog.systemLocation();

  Pulse pulse(portname);
  MainWindow mainwin(pulse);
  mainwin.show();

  app.exec();

  return 0;
}
