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

void listApps(const coredal::Session* session) {
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

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " session database-file\n";
    return 0;
  }

  dunedaq::logging::Logging::setup();

  std::string confimpl = "oksconfig:" + std::string(argv[2]);
  auto confdb = new oksdbinterfaces::Configuration(confimpl);

  std::string sessionName(argv[1]);
  auto session = confdb->get<coredal::Session>(sessionName);
  if (session == nullptr) {
    std::cerr << "Session " << sessionName << " not found in database\n";
    return -1;
  }


  std::cout << "Checking segments disabled state\n";
  auto rseg = session->get_segment();
  if (!rseg->disabled(*session)) {
    std::cout << "Root segment " << rseg->UID()
              << " is not disabled, looping over contained segments\n";
    for (auto seg : rseg->get_segments()) {
      std::cout << "Segment " << seg->UID()
                << std::string(seg->disabled(*session)? " is ":" is not ")
                << "disabled\n";
    }
  }

  auto disabled = session->get_disabled();
  std::cout << "======\nCurrently " << disabled.size() << " items disabled: ";
  for (auto item : disabled) {
    std::cout << " " << item->UID();
  }
  std::cout << std::endl;
  listApps(session);

  std::cout << "======\nNow trying to set enabled  \n";
  std::set<const coredal::Component*> enable;
  for (auto item : disabled) {
    enable.insert(item);
  }
  session->set_enabled(enable);
  listApps(session);

  std::cout << "======\nNow trying to set enabled to an empty list\n";
  enable.clear();
  session->set_enabled(enable);
  listApps(session);

  std::cout << "======\nNow trying to set disabled to an empty list \n";
  session->set_disabled({});
  listApps(session);

}
