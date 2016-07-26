#ifndef SETTINGS_HH
#define SETTINGS_HH

#include <QSettings>


/** Implements a simple persistent settings class. */
class Settings: public QSettings
{
  Q_OBJECT

public:
  /** Constructor. */
  Settings(QObject *parent=0);

  /** Returns the time range for the SpO2/Pulse plots. */
  double plotDuration() const;
  /** Sets the time range for the SpO2/Pulse plots. */
  void setPlotDuration(double dur);

  /** Returns the lower-bound of the SpO2 plot range.*/
  double minSpO2() const;
  /** Sets the lower-bound of the SpO2 plot range.*/
  void setMinSpO2(double value);

  /** Returns the upper-bound of the SpO2 plot range.*/
  double maxSpO2() const;
  /** Sets the upper-bound of the SpO2 plot range.*/
  void setMaxSpO2(double value);

  /** Returns the lower-bound of the pulse plot range.*/
  double minPulse() const;
  /** Sets the lower-bound of the pulse plot range.*/
  void setMinPulse(double value);

  /** Returns the upper-bound of the pulse plot range.*/
  double maxPulse() const;
  /** Sets the upper-bound of the pulse plot range.*/
  void setMaxPulse(double value);

  /** Returns @c true if the pulse signal plot is visible. */
  bool pulsePlotVisible() const;
  /** Enables/disables the pulse signal plot. */
  void setPulsePlotVisible(bool visible);

  /** Returns the pulse signal time range. */
  double pulsePlotDuration() const;
  /** Sets the pulse signal time range. */
  void setPulsePlotDuration(double dur);

  /** Returns @c true if the pulse beep is enabled. */
  bool pulseBeepEnabled() const;
  /** Enables/disables the pulse beep. */
  void setPulseBeepEnabled(bool enabled);
  /** Returns the pulse beep volume in range [0,1]. */
  double pulseBeepVolume() const;
  /** Sets the pulse beep volume in range [0,1]. */
  void setPulseBeepVolume(double value);

  /** Returns @c true if the IR and RED channels are swaped. */
  bool swapChannels() const;
  /** Swaps the IR and RED channels. */
  void setSwapChannels(bool swap);

protected:
  /** The time range for the SpO2/pulse plot. */
  double _plotDuration;
  /** The lower-bound of the SpO2 plot range. */
  double _minSpO2;
  /** The upper-bound of the SpO2 plot range. */
  double _maxSpO2;
  /** The lower-bound of the pulse plot range. */
  double _minPulse;
  /** The upper-bound of the pulse plot range. */
  double _maxPulse;
  /** Visibility of the pulse signal plot. */
  bool _pulsePlotVisible;
  /** Time range of the pulse signal plot. */
  double _pulsePlotDuration;
  /** @c true if pulse beep is enabled. */
  bool _pulseBeepEnabled;
  /** The pulse beep volume. */
  double _pulseBeepVolume;
  /** @c true if IR and RED channels are swaped. */
  bool _swapChannels;
};

#endif // SETTINGS_HH
