

# This is a skeleton CMakeLists.txt file, auto-generated on
# Tue Mar 21 09:59:14 GMT 2023.  The developer(s) of this package should delete
# this comment as well as adding dependent targets, packages,
# etc. specific to the package. For details on how to write a package,
# please see
# https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/

cmake_minimum_required(VERSION 3.12)
project(coredal VERSION 0.0.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(okssystem REQUIRED)
find_package(genconfig REQUIRED)
find_package(ers REQUIRED)
find_package(logging REQUIRED)
find_package(oksdbinterfaces REQUIRED)


#find_package(Boost COMPONENTS unit_test_framework REQUIRED)

##############################################################################

daq_generate_dal(dunedaq.schema.xml NAMESPACE dunedaq::coredal)

daq_add_library(dalMethods.cpp
  test_circular_dependency.cpp disabled-components.cpp
  LINK_LIBRARIES oksdbinterfaces::oksdbinterfaces okssystem::okssystem
  logging::logging coredal_oks)


daq_add_application(listApps list_apps.cxx
  LINK_LIBRARIES coredal oksdbinterfaces::oksdbinterfaces)

daq_add_application(disable_test disable_test.cxx TEST
  LINK_LIBRARIES coredal oksdbinterfaces::oksdbinterfaces logging::logging)
##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_python_bindings


#daq_add_python_bindings(*.cpp LINK_LIBRARIES ${PROJECT_NAME} ) # Any additional libraries to link in beyond the main library not yet determined
daq_add_python_bindings(*.cpp LINK_LIBRARIES coredal coredal_oks)


##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_unit_test

#daq_add_unit_test(Placeholder_test LINK_LIBRARIES)  # Placeholder_test should be replaced with real unit tests

##############################################################################

daq_install()

