#include "mainwindow.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QToolButton>
#include <QIcon>
#include "settingsdialog.hh"
#include "aboutdialog.hh"


MainWindow::MainWindow(Pulse &pulse, Settings &settings, QWidget *parent)
  : QMainWindow(parent), _pulse(pulse), _settings(settings), _plot(0), _beep()
{
  //setWindowFlags(windowFlags() | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
  setMinimumSize(800, 480);
  setWindowTitle(tr("Pulse"));

  QToolBar *toolbar = new QToolBar(tr("Pulse"));
  addToolBar(Qt::RightToolBarArea, toolbar);

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
  _soundButton->setChecked(_settings.pulseBeepEnabled());
  _soundButton->setToolTip(tr("Enable/disable pulse beep"));
  toolbar->addWidget(_soundButton);

  QAction *settingsAct = toolbar->addAction(
        QIcon("://icons/config.png"), tr("Settings"), this, SLOT(_onSettings()));
  settingsAct->setToolTip(tr("Settings"));

  QWidget* empty = new QWidget();
  empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  toolbar->addWidget(empty);

  QAction *about = toolbar->addAction(
        QIcon("://icons/about.png"), tr("About Pulse"), this, SLOT(_onAbout()));
  about->setToolTip(tr("Shows about dialog."));

  toolbar->addSeparator();
  QAction *quit = toolbar->addAction(
        QIcon("://icons/quit.png"), tr("Quit"), this, SLOT(close()));
  quit->setToolTip(tr("Quit the application."));

  QColor color;
  _plot = new QCustomPlot(this);
  //_plot->setInteraction(QCP::iRangeZoom, true);
  //_plot->setInteraction(QCP::iRangeDrag, true);
  _plot->yAxis->setRange(_settings.minSpO2(), _settings.maxSpO2());
  _plot->yAxis2->setRange(_settings.minPulse(), _settings.maxPulse());
  _plot->xAxis->setRange(1-_settings.plotDuration(),1);
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
  _pulsePlot->setVisible(_settings.pulsePlotVisible());
  _pulsePlot->yAxis->setLabel(tr("Pulse signal"));
  _pulsePlot->yAxis->setRange(-1, 1);
  _pulsePlot->xAxis->setRange(0, _settings.pulsePlotDuration());
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
  layout->setSpacing(0);
  layout->setContentsMargins(0,0,0,0);
  panel->setLayout(layout);
  setCentralWidget(panel);

  _beep.setSource(QUrl::fromLocalFile("://sounds/beep1.wav"));
  _beep.setMuted(! _settings.pulseBeepEnabled());
  _beep.setVolume(_settings.pulseBeepVolume());

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
  _spo2Graph->addData(t, _pulse.SpO2());
  _pulseGraph->addData(t, _pulse.pulse());

  _irPulseGraph->addData(t, _pulse.irPulse());
  _irPulseGraph->rescaleValueAxis(false,false);
  _irStdGraph->addData(t, _pulse.irStd());
  _irStdGraph->rescaleValueAxis(true,false);
  _redPulseGraph->addData(t, _pulse.redPulse());
  _redPulseGraph->rescaleValueAxis(true,false);
  _redStdGraph->addData(t, _pulse.redStd());
  _redStdGraph->rescaleValueAxis(true,false);

  _applySettings();
}

void
MainWindow::_applySettings() {
  double tMax = std::ceil(_pulse.t());
  _spo2Graph->removeDataBefore(_pulse.t()-_settings.plotDuration());
  _pulseGraph->removeDataBefore(_pulse.t()-_settings.plotDuration());
  _irPulseGraph->removeDataBefore(_pulse.t()-_settings.pulsePlotDuration());
  _irStdGraph->removeDataBefore(_pulse.t()-_settings.pulsePlotDuration());
  _redPulseGraph->removeDataBefore(_pulse.t()-_settings.pulsePlotDuration());
  _redStdGraph->removeDataBefore(_pulse.t()-_settings.pulsePlotDuration());

  _plot->xAxis->setRange(tMax-_settings.plotDuration(), tMax);
  _plot->yAxis->setRange(_settings.minSpO2(), _settings.maxSpO2());
  _plot->yAxis2->setRange(_settings.minPulse(), _settings.maxPulse());
  _pulsePlot->xAxis->setRange(tMax-_settings.pulsePlotDuration(), tMax);

  _pulsePlot->setVisible(_settings.pulsePlotVisible());
  _plot->replot();
  if (_settings.pulsePlotVisible()) {
    _pulsePlot->replot();
  }

  if (_settings.pulseBeepEnabled()) {
    _beep.setMuted(false);
    _soundButton->setChecked(true);
  } else {
    _beep.setMuted(true);
    _soundButton->setChecked(false);
  }
  _beep.setVolume(_settings.pulseBeepVolume());
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
  _settings.setPulseBeepEnabled(on);
  _beep.setMuted(! _settings.pulseBeepEnabled());
  if (_settings.pulseBeepEnabled())
    _beep.play();
}

void
MainWindow::_onSettings() {
  SettingsDialog dialog(_settings);
  if (QDialog::Accepted == dialog.exec())
    _applySettings();
}

void
MainWindow::_onAbout() {
  AboutDialog dialog;
  dialog.exec();
}
