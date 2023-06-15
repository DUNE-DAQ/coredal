/**
 * @file genReadoutModules.cpp
 *
 * Quick test/demonstration of ReadoutApplication's dal method
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2023.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "logging/Logging.hpp"

#include "oksdbinterfaces/Configuration.hpp"

#include "coredal/Connection.hpp"
#include "coredal/DataReader.hpp"
#include "coredal/DataStreamDesccriptor.hpp"
#include "coredal/DLH.hpp"
#include "coredal/LinkHandlerConf.hpp"
#include "coredal/QueueConnectionRule.hpp"
#include "coredal/QueueDescriptor.hpp"
#include "coredal/ReadoutApplication.hpp"

#include <string>

using namespace dunedaq;

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " <readout-app> <database-file>\n";
    return 0;
  }
  logging::Logging::setup();

  std::string dbfile(argv[2]);
  auto confdb = new oksdbinterfaces::Configuration("oksconfig:" + dbfile);
  std::string appName(argv[1]);

  std::vector<const coredal::DataReader*> dataReaders;
  std::vector<const coredal::DLH*> dataHandlers;

  auto daqapp = confdb->get<coredal::ReadoutApplication>(appName);
  if (daqapp) {
    for (auto module: daqapp->generate_modules(confdb, dbfile)) {
      std::cout << "module " << module->UID() << std::endl;
      module->config_object().print_ref(std::cout, *confdb, "  ");
      std::cout  << " input objects "  << std::endl;
      for (auto input : module->get_inputs()) {
        auto iObj = input->config_object();
        iObj.print_ref(std::cout, *confdb, "    ");
      }
      std::cout  << " output objects "  << std::endl;
      for (auto output : module->get_outputs()) {
        auto oObj = output->config_object();
        oObj.print_ref(std::cout, *confdb, "    ");
      }
      std::cout << std::endl;
    }
  }
  else {
    std::cout << "Failed to get ReadoutApplication " << appName
              << " from database\n";
    return 0;
  }
}
