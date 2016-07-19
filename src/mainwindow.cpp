#include "mainwindow.h"
#include <QVBoxLayout>

MainWindow::MainWindow(Pulse &pulse, QWidget *parent)
  : QMainWindow(parent), _pulse(pulse), _plot(0), _graph(0)
{
  setMinimumSize(800, 320);
  setWindowTitle(tr("Pulse"));

  _timer.setInterval(250);
  _timer.setSingleShot(false);

  _plot = new QCustomPlot(this);
  _plot->setInteraction(QCP::iRangeZoom, true);
  _plot->setInteraction(QCP::iRangeDrag, true);
  _plot->yAxis->setRange(-1,1);
  _plot->xAxis->setRange(0,1);
  _graph = _plot->addGraph();

  connect(&_timer, SIGNAL(timeout()), this, SLOT(_onUpdate()));

  _startTime = QDateTime::currentDateTime();
  _timer.start();

  setCentralWidget(_plot);
}

void
MainWindow::_onUpdate() {
  double value = 1;
  if (_pulse.readMeasurement(value)) {
    double t = _startTime.msecsTo(QDateTime::currentDateTime());
    t /= 60e3;
    double tMax = std::ceil(t);
    _graph->addData(t, value);
    _plot->xAxis->setRange(0, tMax);
    _plot->replot();
  }
  if (! _pulse.startMeasurement()) {
    qDebug() << "Cannot restart measurement.";
  }
}
