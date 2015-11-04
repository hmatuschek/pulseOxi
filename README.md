# Homebrew pulse oximeter 

An oximeter is a relatively simple device. It uses the property of hemoglobin, changing its absorption spectrum around 700nm (dark red) when bound to oxigen. For other parts of the spectrum of visible light and near infrared, the absorption spectrum remains almost unchanged. 

[TODO: Insert figure]

The change in the absorption spectrum can therefore be measured by measuring the amount (intensity, brightness) of red light (around 700nm) being transmitted through or back-scattered from e.g. a finger or more precisely from the the hemoglobin in it. The amount of light that gets detected, however, depends on many more factors including the sensitivity of the detector (photo diode) itself, the light source (LEDs), how the detector and source is fixated on the skin and, of course, on physiologic properties of the finger. These dependencies turn the measurement with a single 700nm source virtually impossible. Many of these influences, however, affect light of all wavelengths. Hence it is possible to reduce these influences on the measurement by measuring the brightness of light of more than one wavelength. The oxidation of hemoglobin changes its spectrum from 600nm-800nm, hence using an additional light source with a wavelength outside this interval allows to chancel out all influences which are wavelength insensitive. This "trick" then allows to measure the relative (or change in) oxidation of the hemoglobin by measuring the change in the difference of the intensities at these two wavelengths. 


## The hardware

[Insert circuit here]

For this circuit, two LEDs are used as light sources. One red led (around 645nm) and one infra red (IR, 900nm) led. The intensity of the back-scattered light is then measured using a photo diode. The current through this diode is then converted to a voltage using a operational amplifier (opamp) and filtered with a second one. The filtered voltage signal is then sampled using one of the analog/digital converters (ADCs) of the ATMega 8. By switching the LEDs on and off (also controlled by the ATMega), the intensity difference can be computed, representing the relative oxidation level of the hemoglobin.

 
## Features

 * Arduino compatible hardware for easy hacking.
 * Runs on battery or by power supply
 * trickle-charger for NiMH rechargeable.
 * Self calibrating (maybe)
 * RS-232 interface
 

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
