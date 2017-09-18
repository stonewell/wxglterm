#include <pybind11/pybind11.h>

char const* yay()
{
  return "Yay!";
}

namespace py = pybind11;

PYBIND11_MODULE(wxglterm_interface, m)
{
  m.def("yay", &yay);
}
