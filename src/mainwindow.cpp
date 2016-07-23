#include "mainwindow.h"
#include <QVBoxLayout>
#include <QMessageBox>

#define DURATION 2.0


MainWindow::MainWindow(Pulse &pulse, QWidget *parent)
  : QMainWindow(parent), _pulse(pulse), _plot(0)
{
  setMinimumSize(800, 320);
  setWindowTitle(tr("Pulse"));

  QColor color;
  _plot = new QCustomPlot(this);
  _plot->setInteraction(QCP::iRangeZoom, true);
  _plot->setInteraction(QCP::iRangeDrag, true);
  _plot->yAxis->setRange(0,100);
  _plot->xAxis->setRange(0,1);
  _plot->yAxis->setTickLabelColor(Qt::blue);
  _plot->yAxis->setLabel("SpO2 [%]");
  _plot->yAxis2->setVisible(true);
  _plot->yAxis2->setTickLabelColor(Qt::red);
  _plot->yAxis2->setLabel("Pulse [BPM]");

  _relGraph = _plot->addGraph();
  _relMeanGraph = _plot->addGraph();
  color = Qt::blue; color.setAlpha(64);
  _relGraph->setPen(color);
  color.setAlpha(255);
  _relMeanGraph->setPen(color);

  _pulseGraph = _plot->addGraph(0, _plot->yAxis2);
  color = Qt::red; color.setAlpha(64);
  _pulseGraph->setPen(color);
  _pulseMeanGraph = _plot->addGraph(0, _plot->yAxis2);
  color = Qt::red; color.setAlpha(255);
  _pulseMeanGraph->setPen(color);

  setCentralWidget(_plot);

  connect(&_pulse, SIGNAL(connectionLost()), this, SLOT(_onConnectionLoss()));
  connect(&_pulse, SIGNAL(measurement()), this, SLOT(_onUpdate()));

  _pulse.start();
}

void
MainWindow::_onUpdate() {
  double t  = _pulse.t();
  double pulse = _pulse.pulse();
  double pulseMean = _pulse.pulseMean();
  double tMax = std::ceil(t);
  double rel = _pulse.SpO2();
  double relMean = _pulse.SpO2Mean();

  _relGraph->addData(t, rel);
  _relGraph->removeDataBefore(t-DURATION);
  _relMeanGraph->addData(t, relMean);
  _relMeanGraph->removeDataBefore(t-DURATION);
  _relGraph->rescaleValueAxis(false, false);
  _relMeanGraph->rescaleValueAxis(true, false);

  _pulseGraph->addData(t, pulse);
  _pulseGraph->removeDataBefore(t-DURATION);
  _pulseMeanGraph->addData(t, pulseMean);
  _pulseMeanGraph->removeDataBefore(t-DURATION);
  _pulseGraph->rescaleValueAxis(false, false);
  _pulseMeanGraph->rescaleValueAxis(true, false);

  _plot->xAxis->setRange(tMax-DURATION, tMax);
  _plot->replot();
}

void
MainWindow::_onConnectionLoss() {
  // Ask user for reconnect
  QMessageBox::StandardButton res = QMessageBox::Retry;
  while (QMessageBox::Retry == QMessageBox::question(
           this, tr("Connection lost"), tr("Connection to the device lost. Try reconnect?"),
           QMessageBox::Retry, QMessageBox::Abort) ) {
    // Try to reconnect
    if (_pulse.reconnect()) {
      _pulse.start();
      return;
    }
  }
  // Quit
  this->close();
}
