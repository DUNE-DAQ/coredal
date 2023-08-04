#include "logging/Logging.hpp"

#include "oksdbinterfaces/Configuration.hpp"

#include "coredal/Component.hpp"
#include "coredal/DaqApplication.hpp"
#include "coredal/DaqModule.hpp"
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
    auto daqApp = app->cast<coredal::DaqApplication>();
    if (daqApp) {
      if (daqApp->disabled(*session)) {
        std::cout << "<disabled>";
      }
      else {
        std::cout << " Modules:";
        for (auto mod : daqApp->get_contains()) {
          std::cout << " " << mod->UID();
          if (mod->disabled(*session)) {
            std::cout << "<disabled>";
          }
        }
      }
    }
    std::cout << std::endl;
  }
}
