# Home-brewed pulse oximeter 

An pulse oximeter is a relatively simple device. It uses the property of hemoglobin, which changes its absorption spectrum around 660nm (red) and to some lesser degree around 940nm (infrared) when bound to oxygen. The change in the absorption spectrum can therefore be measured by measuring the amount (intensity, brightness) of red and infrared light being transmitted through or back-scattered from e.g. a finger or more precisely from the the hemoglobin in it. The amount of light that gets detected, however, depends on many more factors including the sensitivity of the detector (photo diode) itself, the light source (LEDs), how the detector and source is fixated on the skin and, of course, on physiognomic properties of the finger. These dependencies turn the measurement with a single source virtually impossible. Many of these influences, however, affect light of all wavelengths. Hence it is possible to reduce these influences on the measurement by measuring the brightness of light of more than one wavelength. The oxidation of hemoglobin changes its spectrum at different wavelengths differently, hence using two light sources with different wavelengths allows to chancel out the influences which are wavelength insensitive. This "trick" then allows to measure the relative (or change in) oxidation of the hemoglobin by measuring the change in the difference of the intensities at these two wavelengths. 


## The hardware

For the [circuit](https://github.com/hmatuschek/pulseOxi/blob/master/hardware/pulse_scm.pdf), two LEDs are used as light sources. One red led (around 645nm) and one infra red (IR, 900nm) led. The intensity of the transmitted light is then measured using a photo diode. The current through this diode is then converted to a voltage using a operational amplifier (opamp). The voltage signal is then sampled using one of the analog/digital converters (ADCs) of the ATtiny45. By switching the LEDs on and off (also controlled by the ATtiny), the intensity difference can be computed, representing the relative oxidation level of the hemoglobin. For more details on how to construct a pulse-oximeter, consider the [AN4327 application node](http://www.nxp.com/files/32bit/doc/app_note/AN4327.pdf) by NXP. 

Please note that the LEDs are driven directly by the digital outputs of the ATtiny. They are able to deliver (only) about 20-30mA. Hence choose the LEDs and their series resistors accordingly. The circuit above does not include the series resistors. They are connected directly to the LEDs at the clip.


## The client software

The client software provides a convenient [Qt5](https://qt.io) GUI application using [QCustomPlot](http://www.qcustomplot.com/) for potting and [libusb](http://libusb.info/) to interface the pulse oximeter hardware.
 
 
## Features

 * Small circuit.
 * USB interface using [V-USB](https://www.obdev.at/products/vusb/index-de.html).
 * USB powered.
 

## License
Pulse - A simple pulse oximeter. (c) 2015 Hannes Matuschek <hmatuschek at gmail dot com>

The hardware is licensed under [Createtive Commons BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) and the software under [GPL 2.0+](https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt) (see below).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.</p>

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
