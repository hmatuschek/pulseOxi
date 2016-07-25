#include "settingsdialog.hh"
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QCheckBox>
#include <QSlider>

#include <QDialogButtonBox>
#include <QDebug>
#include <QSoundEffect>


SettingsDialog::SettingsDialog(Settings &settings)
  : QDialog(), _settings(settings), _beep()
{
  setWindowTitle(tr("Settings..."));
  setModal(true);
  _beep.setSource(QUrl::fromLocalFile("://sounds/beep1.wav"));

  _plotDuration = new QLineEdit(QString::number(_settings.plotDuration()));
  QDoubleValidator *validator = new QDoubleValidator();
  validator->setBottom(0.1);
  _plotDuration->setValidator(validator);

  _minSpO2 = new QLineEdit(QString::number(_settings.minSpO2()));
  validator = new QDoubleValidator();
  validator->setBottom(0); validator->setTop(100);
  _minSpO2->setValidator(validator);

  _maxSpO2 = new QLineEdit(QString::number(_settings.maxSpO2()));
  validator = new QDoubleValidator();
  validator->setBottom(0); validator->setTop(100);
  _maxSpO2->setValidator(validator);

  _minPulse = new QLineEdit(QString::number(_settings.minPulse()));
  validator = new QDoubleValidator();
  validator->setBottom(0);
  _minPulse->setValidator(validator);

  _maxPulse = new QLineEdit(QString::number(_settings.maxPulse()));
  validator = new QDoubleValidator();
  validator->setBottom(0);
  _maxPulse->setValidator(validator);

  _pulsePlotVisible = new QCheckBox();
  _pulsePlotVisible->setChecked(_settings.pulsePlotVisible());

  _pulsePlotDuration = new QLineEdit(QString::number(_settings.pulsePlotDuration()));
  validator = new QDoubleValidator();
  validator->setBottom(0.1);
  _pulsePlotDuration->setValidator(validator);

  _pulseBeepEnabled = new QCheckBox();
  _pulseBeepEnabled->setChecked(_settings.pulseBeepEnabled());

  _pulseBeepVolume = new QSlider(Qt::Horizontal);
  _pulseBeepVolume->setMinimum(0);
  _pulseBeepVolume->setMaximum(100);
  _pulseBeepVolume->setValue(100*_settings.pulseBeepVolume());

  QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

  QFormLayout *form1 = new QFormLayout();
  form1->addRow(tr("Pulse/SpO2 time window"), _plotDuration);
  form1->addRow(tr("min. SpO2 value"), _minSpO2);
  form1->addRow(tr("max. SpO2 value"), _maxSpO2);
  form1->addRow(tr("min. Pulse value"), _minPulse);
  form1->addRow(tr("max. Pulse value"), _maxPulse);

  QFormLayout *form2 = new QFormLayout();
  form2->addRow(tr("Pulse signal plot visible"), _pulsePlotVisible);
  form2->addRow(tr("Pulse signal time window"), _pulsePlotDuration);

  QFormLayout *form3 = new QFormLayout();
  form2->addRow(tr("Pulse beep enabled"), _pulseBeepEnabled);
  form2->addRow(tr("Pulse beep volume"), _pulseBeepVolume);

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addLayout(form1);
  layout->addSpacing(5);
  layout->addLayout(form2);
  layout->addSpacing(5);
  layout->addLayout(form3);
  layout->addWidget(bb);

  setLayout(layout);

  connect(bb, SIGNAL(accepted()), this, SLOT(apply()));
  connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_pulseBeepVolume, SIGNAL(sliderReleased()), this, SLOT(_onBeep()));
}


void
SettingsDialog::apply() {
  qDebug() << "Apply settings...";
  _settings.setPlotDuration(_plotDuration->text().toDouble());
  _settings.setMinSpO2(_minSpO2->text().toDouble());
  _settings.setMaxSpO2(_maxSpO2->text().toDouble());
  _settings.setMinPulse(_minPulse->text().toDouble());
  _settings.setMaxPulse(_maxPulse->text().toDouble());
  _settings.setPulsePlotVisible(_pulsePlotVisible->isChecked());
  _settings.setPulsePlotDuration(_pulsePlotDuration->text().toDouble());
  _settings.setPulseBeepEnabled(_pulseBeepEnabled->isChecked());
  _settings.setPulseBeepVolume(double(_pulseBeepVolume->value())/100);
  accept();
}

void
SettingsDialog::_onBeep() {
  _beep.setVolume(double(_pulseBeepVolume->value())/100);
  _beep.play();
}
