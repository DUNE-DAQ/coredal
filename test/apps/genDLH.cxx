#include "logging/Logging.hpp"

#include "oksdbinterfaces/Configuration.hpp"
#include "oksdbinterfaces/Schema.hpp"

#include "coredal/ReadoutApplication.hpp"
#include "coredal/DataReader.hpp"
#include "coredal/DLH.hpp"
//#include "coredal/.hpp"
#include "coredal/QueueConnectionRule.hpp"
#include "coredal/QueueDescriptor.hpp"
#include "coredal/LinkHandlerConf.hpp"

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
  auto daqapp = confdb->get<coredal::ReadoutApplication>(appName);
  if (daqapp) {
    auto linkHandler = daqapp->get_link_handler();
    std::cout << "Application " << daqapp->UID()
              << " link handler read from database: "
              << linkHandler << std::endl;
    auto lhObj = linkHandler->config_object();
    const coredal::QueueDescriptor* inputQDesc = nullptr;
    const coredal::QueueDescriptor* outputQDesc = nullptr;
    for (auto rule : daqapp->get_queue_rules()) {
      if (rule->get_destination_class() == "DLH") {
        inputQDesc = rule->get_descriptor();
      }
      else if (rule->get_destination_class() == "TPHandler") {
        outputQDesc = rule->get_descriptor();
      }
    }

    std::string tpQueueUid("inputToTPH-"+daqapp->UID());
    oksdbinterfaces::ConfigObject tpQueueObj;
    if (outputQDesc != nullptr) {
      confdb->create(dbfile, "Queue", tpQueueUid, tpQueueObj);
    }

    for (auto reader : daqapp->get_data_readers()) {
      for (auto id : reader->get_source_ids()) {
        std::string uid("DLH-"+std::to_string(id));
        oksdbinterfaces::ConfigObject dlhObj;
        confdb->create(dbfile, "DLH", uid, dlhObj);
        dlhObj.set_by_val<uint32_t>("source_id", id);
        dlhObj.set_obj("handler_configuration", &lhObj);

        std::string queueUid("inputToDLH-"+std::to_string(id));
        oksdbinterfaces::ConfigObject queueObj;
        confdb->create(dbfile, "Queue", queueUid, queueObj);
        dlhObj.set_obj("raw_data_input", &queueObj);

        if (outputQDesc != nullptr) {
          dlhObj.set_obj("processing_output", &tpQueueObj);
        }

//        confdb->commit();
        auto dlhDal = const_cast<coredal::DLH*>(
          confdb->get<coredal::DLH>(uid));
        std::cout << dlhDal->UID() 
                  << " source_id " << dlhDal->get_source_id()
                  << " handler " << dlhDal->get_handler_configuration()
                  << std::endl;
        dlhObj.print_ref(std::cout, *confdb, "  ");
        std::cout << std::endl;
      }
    }
  }
  else {
    std::cout << "Failed to get ReadoutApplication " << appName
              << " from database\n";
  }
}
