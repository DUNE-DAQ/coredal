/**
 * @file generate_modules .cpp
 *
 * Implementation of ReadoutApplication's dal method
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2023.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "oksdbinterfaces/Configuration.hpp"

#include "coredal/Connection.hpp"
#include "coredal/DataReader.hpp"
#include "coredal/DataReaderConf.hpp"
#include "coredal/DataStreamDesccriptor.hpp"
#include "coredal/DLH.hpp"
#include "coredal/LinkHandlerConf.hpp"
#include "coredal/QueueConnectionRule.hpp"
#include "coredal/QueueDescriptor.hpp"
#include "coredal/ReadoutApplication.hpp"
#include "coredal/TPHandler.hpp"
#include "coredal/TPHandlerConf.hpp"

#include "ers/Issue.hpp"

#include <string>

namespace dunedaq {
  ERS_DECLARE_ISSUE(coredal, BadConf, what, ((std::string)what))
}

using namespace dunedaq;
using namespace dunedaq::coredal;

std::vector<const DaqModule*> 
ReadoutApplication::generate_modules(oksdbinterfaces::Configuration* confdb,
                                     std::string& dbfile) const {
  std::vector<const DaqModule*> modules;
  auto linkHandler = get_link_handler();
  auto lhObj = linkHandler->config_object();
  const QueueDescriptor* inputQDesc = nullptr;
  const QueueDescriptor* outputQDesc = nullptr;
  for (auto rule : get_queue_rules()) {
    if (rule->get_destination_class() == "DLH") {
      inputQDesc = rule->get_descriptor();
    }
    else if (rule->get_destination_class() == "TPHandler") {
      outputQDesc = rule->get_descriptor();
    }
  }

  oksdbinterfaces::ConfigObject tpQueueObj;
  auto tpHandlerConf = get_tp_handler();
  if (tpHandlerConf) {
    if (outputQDesc == nullptr) {
      throw (BadConf(ERS_HERE, "No output queue descriptor given"));
    }
    auto tpsrc = get_tp_src_id();
    if (tpsrc == 0) {
      throw (BadConf(ERS_HERE, "No TPHandler src_id given"));
    }
    std::string tpQueueUid("inputToTPH-"+std::to_string(tpsrc));
    confdb->create(dbfile, "Queue", tpQueueUid, tpQueueObj);
    tpQueueObj.set_by_val<std::string>("data_type", outputQDesc->get_data_type());
    tpQueueObj.set_by_val<std::string>("queue_type", outputQDesc->get_queue_type());
    tpQueueObj.set_by_val<uint32_t>("capacity", outputQDesc->get_capacity());

    auto tphConfObj = tpHandlerConf->config_object();
    oksdbinterfaces::ConfigObject tpObj;
    std::string tpUid("tphandler-"+std::to_string(tpsrc));
    confdb->create(dbfile, "TPHandler", tpUid, tpObj);
    tpObj.set_by_val<uint32_t>("source_id", tpsrc);
    tpObj.set_obj("handler_configuration", &tphConfObj);
    tpObj.set_objs("inputs", {&tpQueueObj});

    auto tphDal = const_cast<TPHandler*>(confdb->get<TPHandler>(tpUid));
    modules.push_back(tphDal);
  }

  auto rdrConf = get_data_reader();
  if (rdrConf == 0) {
    throw (BadConf(ERS_HERE, "No DataReader configuration given"));
  } 
  auto readerConfObj = rdrConf->config_object();
  int rnum = 0;
  for (auto stream : get_data_streams()) {
    std::vector<const Connection*> outputQueues;
    for (auto id : stream->get_src_ids()) {
      std::string uid("DLH-"+std::to_string(id));
      oksdbinterfaces::ConfigObject dlhObj;
      confdb->create(dbfile, "DLH", uid, dlhObj);
      dlhObj.set_by_val<uint32_t>("source_id", id);
      dlhObj.set_obj("handler_configuration", &lhObj);
      if (tpHandlerConf) {
        dlhObj.set_objs("outputs", {&tpQueueObj});
      }
      std::string queueUid("inputToDLH-"+std::to_string(id));
      oksdbinterfaces::ConfigObject queueObj;
      confdb->create(dbfile, "Queue", queueUid, queueObj);
      queueObj.set_by_val<std::string>("data_type", inputQDesc->get_data_type());
      queueObj.set_by_val<std::string>("queue_type", inputQDesc->get_queue_type());
      queueObj.set_by_val<uint32_t>("capacity", inputQDesc->get_capacity());
      dlhObj.set_objs("inputs", {&queueObj});
      // Add the input queue dal pointer to the outputs of the DataReader
      outputQueues.push_back(confdb->get<Connection>(queueUid));

      auto dlhDal = const_cast<DLH*>(confdb->get<DLH>(uid));
      modules.push_back(dlhDal);
    }

    std::string readerUid("datareader-"+UID()+"-"+std::to_string(rnum++));
    oksdbinterfaces::ConfigObject readerObj;
    if (readerConfObj.class_name() == "NicReaderConf") {
      confdb->create(dbfile, "NicReader", readerUid, readerObj);
    }
    else if (readerConfObj.class_name() == "FlxReaderConf") {
      confdb->create(dbfile, "FlxReader", readerUid, readerObj);
    }
    else {
      throw (BadConf(ERS_HERE, "DataReaderConf has unexpected type "+readerConfObj.class_name()));
    }
    std::vector<const oksdbinterfaces::ConfigObject*> qObjs;
    for (auto q : outputQueues) {
      qObjs.push_back(&q->config_object());
    }
    readerObj.set_objs("outputs", qObjs);
    readerObj.set_obj("configuration", &readerConfObj);
    auto reader = const_cast<DataReader*>(
      confdb->get<DataReader>(readerUid));
    modules.push_back(reader);
  }
  return modules;
}
