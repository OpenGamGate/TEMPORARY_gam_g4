/* --------------------------------------------------
   Copyright (C): OpenGATE Collaboration
   This software is distributed under the terms
   of the GNU Lesser General  Public Licence (LGPL)
   See LICENSE.md for further details
   -------------------------------------------------- */
#include <pybind11/pybind11.h>

namespace py = pybind11;

#include "G4StepPoint.hh"

void init_G4StepPoint(py::module &m) {

    py::class_<G4StepPoint>(m, "G4StepPoint")
      .def(py::init())
      .def("GetPosition", &G4StepPoint::GetPosition)
      .def("GetPhysicalVolume", &G4StepPoint::GetPhysicalVolume, py::return_value_policy::copy)
      ;
}

