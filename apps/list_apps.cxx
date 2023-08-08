#include "logging/Logging.hpp"

#include "oksdbinterfaces/Configuration.hpp"

#include "coredal/Component.hpp"
#include "coredal/DaqApplication.hpp"
#include "coredal/DaqModule.hpp"
#include "coredal/ResourceSet.hpp"
#include "coredal/Segment.hpp"
#include "coredal/Session.hpp"

#include <iostream>
#include <string>

using namespace dunedaq;

int main(int argc, char* argv[]) {
  dunedaq::logging::Logging::setup();

  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " session database-file\n";
    return 0;
  }
  std::string confimpl = "oksconfig:" + std::string(argv[2]);
  auto confdb = new oksdbinterfaces::Configuration(confimpl);

  std::string sessionName(argv[1]);
  auto session = confdb->get<coredal::Session>(sessionName);
  if (session==nullptr) {
    std::cerr << "Session " << sessionName << " not found in database\n";
    return -1;
  }
  for (auto app : session->get_all_applications()) {
    std::cout << "Application: " << app->UID();
    auto res = app->cast<coredal::ResourceSet>();
    if (res) {
      if (res->disabled(*session)) {
        std::cout << "<disabled>";
      }
      else {
        for (auto mod : res->get_contains()) {
          std::cout << " " << mod->UID();
          if (mod->disabled(*session)) {
            std::cout << "<disabled>";
          }
        }
      }
    }
    auto daqApp = app->cast<coredal::DaqApplication>();
    if (daqApp) {
      std::cout << " Modules:";
      for (auto mod : daqApp->get_modules()) {
        std::cout << " " << mod->UID();
      }
    }
    std::cout << std::endl;
  }
}
