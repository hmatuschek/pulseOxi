#ifndef SETTINGS_HH
#define SETTINGS_HH

#include <QSettings>


class Settings: public QSettings
{
  Q_OBJECT

public:
  Settings(QObject *parent=0);

  double plotDuration() const;
  void setPlotDuration(double dur);

  double minSpO2() const;
  void setMinSpO2(double value);

  double maxSpO2() const;
  void setMaxSpO2(double value);

  double minPulse() const;
  void setMinPulse(double value);

  double maxPulse() const;
  void setMaxPulse(double value);

  bool pulsePlotVisible() const;
  void setPulsePlotVisible(bool visible);

  double pulsePlotDuration() const;
  void setPulsePlotDuration(double dur);

  bool pulseBeepEnabled() const;
  void setPulseBeepEnabled(bool enabled);
  double pulseBeepVolume() const;
  void setPulseBeepVolume(double value);

protected:
  double _plotDuration;

  double _minSpO2;
  double _maxSpO2;

  double _minPulse;
  double _maxPulse;

  bool _pulsePlotVisible;
  double _pulsePlotDuration;

  bool _pulseBeepEnabled;
  double _pulseBeepVolume;
};

#endif // SETTINGS_HH
