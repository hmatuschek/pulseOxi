set(pulse_SOURCES main.cpp
    pulse.cpp mainwindow.cpp qcustomplot.cc)
set(pulse_MOC_HEADERS
    pulse.h mainwindow.h qcustomplot.hh)
qt5_wrap_cpp(pulse_MOC_SOURCES ${pulse_MOC_HEADERS})
set(pulse_HEADERS ${pulse_MOC_HEADERS})

qt5_add_resources(pulse_RCC_SOURCES ../shared/resources.qrc)
add_executable(pulse ${pulse_SOURCES} ${pulse_MOC_SOURCES} ${pulse_RCC_SOURCES})
target_link_libraries(pulse ${LIBS})
