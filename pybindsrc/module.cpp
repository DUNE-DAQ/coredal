/**
 * @file module.cpp
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#include "coredal/DaqApplication.hpp"
#include "coredal/HostComponent.hpp"

namespace py = pybind11;

namespace dunedaq::coredal::python {

extern void
register_dal_methods(py::module&);

PYBIND11_MODULE(_daq_coredal_py, m)
{

  m.doc() = "C++ implementation of the coredal modules";
#if 0
  py::class_<dunedaq::coredal::DaqApplication>(m,"DaqApplication")
    .def(py::init<oksdbinterfaces::Configuration& , const oksdbinterfaces::ConfigObject&>())
    .def("get_used_hostresources", &dunedaq::coredal::DaqApplication::get_used_hostresources);
  py::class_<dunedaq::coredal::HostComponent>(m,"HostComponent");
#endif
  register_dal_methods(m);
}

} // namespace dunedaq::coredal::python
