#ifndef SETTINGSDIALOG_HH
#define SETTINGSDIALOG_HH

#include <QDialog>
#include "settings.hh"
#include <QSoundEffect>

class QLineEdit;
class QCheckBox;
class QSlider;

class SettingsDialog: public QDialog
{
  Q_OBJECT

public:
  SettingsDialog(Settings &settings);

public slots:
  void apply();

protected slots:
  void _onBeep();

protected:
  Settings &_settings;
  QLineEdit *_plotDuration;
  QLineEdit *_minSpO2;
  QLineEdit *_maxSpO2;
  QLineEdit *_minPulse;
  QLineEdit *_maxPulse;
  QCheckBox *_pulsePlotVisible;
  QLineEdit *_pulsePlotDuration;
  QCheckBox *_pulseBeepEnabled;
  QSlider   *_pulseBeepVolume;
  QSoundEffect _beep;
};

#endif // SETTINGSDIALOG_HH
