#include "settings.hh"
#include <cmath>


Settings::Settings(QObject *parent)
  : QSettings("io.github.hmatuschek", "pulseOxy", parent)
{
  // load settings
  _plotDuration = value("plotDuration", 5.).toDouble();
  _minSpO2      = value("minSpO2", 60.).toDouble();
  _maxSpO2      = value("maxSpO2", 100.).toDouble();
  _minPulse     = value("minPulse", 40.).toDouble();
  _maxPulse     = value("maxPulse", 150.).toDouble();
  _pulsePlotVisible = value("pulsePlotVisible", false).toBool();
  _pulsePlotDuration = value("pulsePlotDuration", 1.).toDouble();
  _pulseBeepEnabled = value("pulseBeepEnabled", false).toBool();
  _pulseBeepVolume = value("pulseBeepVolume", 1.0).toDouble();
  _swapChannels = value("swapChannels", false).toBool();
}


double
Settings::plotDuration() const {
  return _plotDuration;
}

void
Settings::setPlotDuration(double dur) {
  _plotDuration = std::max(.1, dur);
  setValue("plotDuration", _plotDuration);
}

double
Settings::minSpO2() const {
  return _minSpO2;
}

void
Settings::setMinSpO2(double value) {
  _minSpO2 = std::max(0.0, value);
  setValue("minSpO2", _minSpO2);
}

double
Settings::maxSpO2() const {
  return _maxSpO2;
}

void
Settings::setMaxSpO2(double value) {
  _maxSpO2 = std::min(100., value);
  setValue("maxSpO2", _maxSpO2);
}

double
Settings::minPulse() const {
  return _minPulse;
}

void
Settings::setMinPulse(double value) {
  _minPulse = std::max(0., value);
  setValue("minPulse", _minPulse);
}

double
Settings::maxPulse() const {
  return _maxPulse;
}

void
Settings::setMaxPulse(double value) {
  _maxPulse = std::min(300., value);
  setValue("maxPulse", _maxPulse);
}

bool
Settings::pulsePlotVisible() const {
  return _pulsePlotVisible;
}

void
Settings::setPulsePlotVisible(bool visible) {
  _pulsePlotVisible = visible;
  setValue("pulsePlotVisible", _pulsePlotVisible);
}

double
Settings::pulsePlotDuration() const {
  return _pulsePlotDuration;
}

void
Settings::setPulsePlotDuration(double dur) {
  _pulsePlotDuration = std::max(0.1, dur);
  setValue("pulsePlotDuration", _pulsePlotDuration);
}

bool
Settings::pulseBeepEnabled() const {
  return _pulseBeepEnabled;
}

void
Settings::setPulseBeepEnabled(bool enabled) {
  _pulseBeepEnabled = enabled;
  setValue("pulseBeepEnabled", _pulseBeepEnabled);
}

double
Settings::pulseBeepVolume() const {
  return _pulseBeepVolume;
}

void
Settings::setPulseBeepVolume(double value) {
  _pulseBeepVolume = std::min(1., std::max(0., value));
  setValue("pulseBeepVolume", _pulseBeepVolume);
}

bool
Settings::swapChannels() const {
  return _swapChannels;
}

void
Settings::setSwapChannels(bool swap) {
  _swapChannels = swap;
}
