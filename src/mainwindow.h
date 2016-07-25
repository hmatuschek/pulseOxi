#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "pulse.h"
#include "qcustomplot.hh"
#include <QToolButton>
#include <QSoundEffect>


class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(Pulse &pulse, QWidget *parent = 0);

protected slots:
  void _onConnectionLoss();
  void _onUpdate();
  void _onStart(bool start);
  void _onLog(bool log);
  void _onSoundToggled(bool on);
  void _onSettings();

protected:
  Pulse &_pulse;
  QToolButton *_start;
  QToolButton *_log;
  QToolButton *_soundButton;

  QCustomPlot *_plot;
  QCPGraph *_spo2Graph;
  QCPGraph *_pulseGraph;

  QCustomPlot *_pulsePlot;
  QCPGraph *_irPulseGraph;
  QCPGraph *_irStdGraph;
  QCPGraph *_redPulseGraph;
  QCPGraph *_redStdGraph;

  QSoundEffect _beep;
};

#endif // MAINWINDOW_H
