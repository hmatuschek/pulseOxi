#include "mainwindow.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QToolButton>
#include <QIcon>

#define DURATION 5.0


MainWindow::MainWindow(Pulse &pulse, QWidget *parent)
  : QMainWindow(parent), _pulse(pulse), _plot(0), _beep()
{
  //setWindowFlags(windowFlags() | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
  setMinimumSize(800, 480);
  setWindowTitle(tr("Pulse"));

  QToolBar *toolbar = addToolBar(tr("Toolbar"));
  _start = new QToolButton();
  _start->setText(tr("Start"));
  _start->setIcon(QIcon("://icons/play.png"));
  _start->setCheckable(true);
  _start->setChecked(false);
  _start->setToolTip(tr("Start/stop measurment."));
  toolbar->addWidget(_start);

  _log = new QToolButton();
  _log->setText(tr("Log"));
  _log->setIcon(QIcon("://icons/log.png"));
  _log->setCheckable(true);
  _log->setChecked(false);
  _log->setToolTip(tr("Start/stop logging measurements to a file."));
  toolbar->addWidget(_log);
  toolbar->addSeparator();

  _soundButton = new QToolButton();
  _soundButton->setText(tr("Enable pulse beep"));
  _soundButton->setIcon(QIcon("://icons/soundon.png"));
  _soundButton->setCheckable(true);
  _soundButton->setChecked(false);
  _soundButton->setToolTip(tr("Enable/disable pulse beep"));
  toolbar->addWidget(_soundButton);

  QAction *settings = toolbar->addAction(
        QIcon("://icons/config.png"), tr("Settings"), this, SLOT(_onSettings()));
  settings->setToolTip(tr("Settings"));

  QWidget* empty = new QWidget();
  empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  toolbar->addWidget(empty);
  QAction *quit = toolbar->addAction(
        QIcon("://icons/quit.png"), tr("Quit"), this, SLOT(close()));
  quit->setToolTip(tr("Quit the application."));

  QColor color;
  _plot = new QCustomPlot(this);
  _plot->setInteraction(QCP::iRangeZoom, true);
  _plot->setInteraction(QCP::iRangeDrag, true);
  _plot->yAxis->setRange(60,100);
  _plot->yAxis2->setRange(40,150);
  _plot->xAxis->setRange(1-DURATION,1);
  _plot->yAxis->setTickLabelColor(Qt::blue);
  _plot->yAxis->setLabel(tr("SpO2 [%]"));
  _plot->yAxis2->setVisible(true);
  _plot->yAxis2->setTickLabelColor(Qt::red);
  _plot->yAxis2->setLabel(tr("Pulse [BPM]"));

  _spo2Graph = _plot->addGraph();
  _spo2Graph->setPen(QColor(Qt::blue));

  _pulseGraph = _plot->addGraph(0, _plot->yAxis2);
  _pulseGraph->setPen(QColor(Qt::red));

  _pulsePlot = new QCustomPlot(this);
  _pulsePlot->yAxis->setLabel(tr("Pulse signal"));
  _pulsePlot->yAxis->setRange(-1, 1);
  _pulsePlot->xAxis->setRange(0, 1);
  _irPulseGraph = _pulsePlot->addGraph();
  _irStdGraph   = _pulsePlot->addGraph();
  color = Qt::blue;
  _irPulseGraph->setPen(color);
  color.setAlpha(64);
  _irStdGraph->setPen(color);
  _redPulseGraph = _pulsePlot->addGraph();
  _redStdGraph   = _pulsePlot->addGraph();
  color = Qt::red;
  _redPulseGraph->setPen(color);
  color.setAlpha(64);
  _redStdGraph->setPen(color);

  QWidget *panel = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(_plot, 2);
  layout->addWidget(_pulsePlot, 1);
  panel->setLayout(layout);
  setCentralWidget(panel);

  _beep.setSource(QUrl::fromLocalFile("://sounds/beep1.wav"));
  _beep.setMuted(true);

  connect(&_pulse, SIGNAL(connectionLost()), this, SLOT(_onConnectionLoss()));
  connect(&_pulse, SIGNAL(measurement()), this, SLOT(_onUpdate()));
  connect(_start, SIGNAL(toggled(bool)), this, SLOT(_onStart(bool)));
  connect(_log, SIGNAL(toggled(bool)), this, SLOT(_onLog(bool)));
  connect(_soundButton, SIGNAL(toggled(bool)), this, SLOT(_onSoundToggled(bool)));
  connect(&_pulse, SIGNAL(pulseEvent()), &_beep, SLOT(play()));
}

void
MainWindow::_onUpdate() {
  double t  = _pulse.t();
  double tMax = std::ceil(t);

  _spo2Graph->addData(t, _pulse.SpO2());
  _spo2Graph->removeDataBefore(t-DURATION);
  _pulseGraph->addData(t, _pulse.pulse());
  _pulseGraph->removeDataBefore(t-DURATION);

  _irPulseGraph->addData(t, _pulse.irPulse());
  _irPulseGraph->removeDataBefore(t-1);
  _irPulseGraph->rescaleValueAxis(false,false);
  _irStdGraph->addData(t, _pulse.irStd());
  _irStdGraph->removeDataBefore(t-1);
  _irStdGraph->rescaleValueAxis(true,false);
  _redPulseGraph->addData(t, _pulse.redPulse());
  _redPulseGraph->removeDataBefore(t-1);
  _redPulseGraph->rescaleValueAxis(true,false);
  _redStdGraph->addData(t, _pulse.redStd());
  _redStdGraph->removeDataBefore(t-1);
  _redStdGraph->rescaleValueAxis(true,false);

  _plot->xAxis->setRange(tMax-DURATION, tMax);
  _pulsePlot->xAxis->setRange(tMax-1, tMax);
  _plot->replot();
  _pulsePlot->replot();
}

void
MainWindow::_onConnectionLoss() {
  if (_start->isChecked())
    _start->toggle();
  // Ask user for reconnect
  while (QMessageBox::Retry == QMessageBox::question(
           this, tr("Connection lost"), tr("Connection to the device lost. Try reconnect?"),
           QMessageBox::Retry, QMessageBox::Abort) ) {
    // Try to reconnect
    if (_pulse.reconnect()) {
      return;
    }
  }
}

void
MainWindow::_onStart(bool start) {
  if (start) {
    _pulse.start();
    _start->setText(tr("Stop"));
    _start->setIcon(QIcon("://icons/stop.png"));
  } else {
    _pulse.stop();
    _start->setText(tr("Start"));
    _start->setIcon(QIcon("://icons/play.png"));
  }
}

void
MainWindow::_onLog(bool log) {
  if (log) {
    QString filename = QFileDialog::getSaveFileName(
          this, tr("Log to"), "", tr("*.csv *.txt (Comma separated values)"));
    if (filename.isEmpty()) {
      _log->setChecked(false);
      return;
    }
    _pulse.logTo(filename);
  } else {
    _pulse.closeLog();
  }
}

void
MainWindow::_onSoundToggled(bool on) {
  _beep.setMuted(! on);
  if (on)
    _beep.play();
}

void
MainWindow::_onSettings() {
  // pass...
}
