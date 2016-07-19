#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "pulse.h"
#include "qcustomplot.hh"

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(Pulse &pulse, QWidget *parent = 0);

protected slots:
  void _onUpdate();

protected:
  Pulse &_pulse;
  QTimer _timer;
  QDateTime _startTime;
  QCustomPlot *_plot;
  QCPGraph *_graph;

};

#endif // MAINWINDOW_H
