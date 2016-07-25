#ifndef FIT_HH
#define FIT_HH

#include <cmath>
#include <cinttypes>
#include <iostream>


template<uint16_t _size>
class LowPassKernel
{
public:
  const static uint16_t size = _size;

public:
  LowPassKernel(float Fc)
    : _Fc(Fc)
  {
    // pass...
  }

  float eval(float t) const {
    if (0 == t) {
      return 2*_Fc;
    }
    return 2*_Fc*std::sin(2*M_PI*_Fc*t)/(2*M_PI*_Fc*t);
  }

protected:
  float _Fc;
};


template<uint16_t _size>
class HighPassKernel
{
public:
  const static uint16_t size = _size;

public:
  HighPassKernel(float Fc)
    : _Fc(Fc)
  {
    // pass...
  }

  float eval(float t) const {
    if (0 == t) {
      return 1./_Fc - 2*_Fc;
    }
    return -2*_Fc*std::sin(2*M_PI*_Fc*t)/(2*M_PI*_Fc*t);
  }

protected:
  float _Fc;
};


template<uint16_t _size>
class BandPassKernel
{
public:
  const static uint16_t size = _size;

public:
  BandPassKernel(float Fl, float Fu)
    : _Fl(Fl), _Fu(Fu)
  {
    // pass...
  }

  float eval(float t) const {
    if (0 == t) {
      return 2*_Fu-2*_Fl;
    }
    return 2*_Fu*std::sin(2*M_PI*_Fu*t)/(2*M_PI*_Fu*t) -
        2*_Fl*std::sin(2*M_PI*_Fl*t)/(2*M_PI*_Fl*t);
  }

protected:
  float _Fl;
  float _Fu;
};


template <uint16_t size>
class WelchWindow
{
public:
  static float eval(size_t i) {
    float x  = (i-float(size-1)/2)/(float(size-1)/2);
    return 1. - x*x;
  }
};


template<class Kernel, class Window=WelchWindow<Kernel::size> >
class FIR
{
public:
  const static uint16_t size = Kernel::size;
  const static uint16_t mask = size-1;

public:
  FIR(const Kernel &kernel)
    : _idx(0)
  {
    for (int i=0; i<size; i++) {
      _kernel[i] = kernel.eval(i-size/2)*Window::eval(i);
      _buffer[i] = 0;
    }
  }

  float apply(float value) {
    _buffer[_idx] = value;
    value = 0;
    for (size_t i=0; i<size; i++) {
      value += _kernel[i]*_buffer[(_idx+i)&mask];
    }
    _idx = (_idx+1)&mask;
    return value;
  }

protected:
  float _kernel[size];
  float _buffer[size];
  uint16_t _idx;
};



#endif // FIT_HH
