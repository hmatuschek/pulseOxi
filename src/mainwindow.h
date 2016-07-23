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
  void _onConnectionLoss();
  void _onUpdate();

protected:
  Pulse &_pulse;
  QCustomPlot *_plot;
  QCPGraph *_relGraph;
  QCPGraph *_relMeanGraph;
  QCPGraph *_pulseGraph;
  QCPGraph *_pulseMeanGraph;

};

#endif // MAINWINDOW_H
