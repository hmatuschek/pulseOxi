#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "pulse.h"


class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(Pulse &pulse, QWidget *parent = 0);

protected:
  Pulse &_pulse;
};

#endif // MAINWINDOW_H
