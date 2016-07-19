# - Find libusb
#
#  LIBUSB_INCLUDES    - where to find libusb.h
#  LIBUSB_LIBRARIES   - List of libraries when using libusb.
#  LIBUSB_FOUND       - True if libusb found.

if (LIBUSB_INCLUDES)
  # Already in cache, be silent
  set (LIBUSB_FIND_QUIETLY TRUE)
endif (LIBUSB_INCLUDES)

find_path (LIBUSB_INCLUDE_DIRS libusb.h
  PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    /sw/include
  PATH_SUFFIXES
    libusb-1.0)

find_library (LIBUSB_LIBRARIES
  NAMES
    usb-1.0
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib)

# handle the QUIETLY and REQUIRED arguments and set *_FOUND to TRUE if
# all listed variables are TRUE
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (LIBUSB DEFAULT_MSG LIBUSB_LIBRARIES LIBUSB_INCLUDE_DIRS)

mark_as_advanced (LIBUSB_LIBRARIES LIBUSB_INCLUDE_DIRS)
