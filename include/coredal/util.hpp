#ifndef _dal_util_H_
#define _dal_util_H_

#include <exception>

#include "oksdbinterfaces/Configuration.hpp"
#include "oksdbinterfaces/DalObject.hpp"
#include "nlohmann/json.hpp"


namespace dunedaq {

namespace coredal {
  class Session;

    /**
     *  \brief Get session object.
     *
     *  The algorithm %is searching the session object by given name.
     *  If the name %is empty, then the algorithm takes the name from
     *  the TDAQ_SESSION environment variable.<BR>
     *
     *  The last parameter of the algorithm can be used to optimise performance
     *  of the DAL in case if a database server config implementation %is used.
     *  The parameter defines how many layers of objects referenced by given 
     *  session object should be read into client's config cache together with
     *  session object during single network operation. For example:
     *  - if the parameter %is 0, then only session object %is read;
     *  - if the parameter %is 1, then session and first layer segment objects are read;
     *  - if the parameter %is 2, then session, segments of first and second layers, and application/resources of first layer segments objects are read;
     *  - if the parameter %is 10, then mostly probable all objects referenced by given session object are read.<BR>
     *
     *  The parameters of the algorithm are:
     *  \param conf      the configuration object with loaded database
     *  \param name      the name of the session to be loaded (if empty, TDAQ_SESSION variable %is used)
     *  \param rlevel    optional parameter to optimise performance ("the references level")
     *  \param rclasses  optional parameter to optimise performance ("names of classes which objects are cached")
     *
     *  \return Returns the pointer to the session object if found, or 0.
     */

  const dunedaq::coredal::Session * get_session(dunedaq::oksdbinterfaces::Configuration& conf, const std::string& name, unsigned long rlevel = 10, const std::vector<std::string> * rclasses = nullptr);

  template <typename T> void add_json_value(oksdbinterfaces::ConfigObject& obj,
                                            std::string& name,
                                            bool multi_value,
                                            nlohmann::json& attributes) {
    if (!multi_value) {
      T value;
      obj.get(name, value);
      attributes[name] = value;
    }
    else {
      std::vector<T> value_vector;
      obj.get(name, value_vector);
      attributes[name] = nlohmann::json(value_vector);
    }
  }
} // namespace coredal


  ERS_DECLARE_ISSUE(
    coredal,
    AlgorithmError,
    ,
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    BadVariableUsage,
    AlgorithmError,
    message,
    ,
    ((std::string)message)
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    BadApplicationInfo,
    AlgorithmError,
    "Failed to retrieve information for Application \'" << app_id << "\' from the database: " << message,
    ,
    ((std::string)app_id)
    ((std::string)message)
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    BadSessionID,
    AlgorithmError,
    "There is no session object with UID = \"" << name << '\"',
    ,
    ((std::string)name)
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    SegmentDisabled,
    AlgorithmError,
    "Cannot get information about applications because the segment is disabled",
    ,
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    BadProgramInfo,
    AlgorithmError,
    "Failed to retrieve information for Program \'" << prog_id << "\' from the database: " << message,
    ,
    ((std::string)prog_id)
    ((std::string)message)
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    BadHost,
    AlgorithmError,
    "Failed to retrieve application \'" << app_id << "\' from the database: " << message,
    ,
    ((std::string)app_id)
    ((std::string)message)
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    NoDefaultHost,
    AlgorithmError,
    "Failed to find default host for segment \'" << seg_id << "\' " << message,
    ,
    ((std::string)seg_id)
    ((std::string)message)
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    NoTemplateAppHost,
    AlgorithmError,
    "Both session default and segment default hosts are not defined for template application \'" << app_id << "\' from segment \'" << seg_id << "\' (will use localhost, that may cause problems presenting info in IGUI for distributed session).",
    ,
    ((std::string)app_id)
    ((std::string)seg_id)
  )


  ERS_DECLARE_ISSUE_BASE(
    coredal,
    BadTag,
    AlgorithmError,
    "Failed to use tag \'" << tag_id << "\' because: " << message,
    ,
    ((std::string)tag_id)
    ((std::string)message)
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    BadSegment,
    AlgorithmError,
    "Invalid Segment \'" << seg_id << "\' because: " << message,
    ,
    ((std::string)seg_id)
    ((std::string)message)
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    GetTemplateApplicationsOfSegmentError,
    AlgorithmError,
    "Failed to get template applications of \'" << name << "\' segment" << message,
    ,
    ((std::string)name)
    ((std::string)message)
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    BadTemplateSegmentDescription,
    AlgorithmError,
    "Bad configuration description of template segment \'" << name << "\': " << message,
    ,
    ((std::string)name)
    ((std::string)message)
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    CannotGetApplicationObject,
    AlgorithmError,
    "Failed to get application object from name: " << reason,
    ,
    ((std::string)reason)
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    CannotFindSegmentByName,
    AlgorithmError,
    "Failed to find segment object \'" << name << "\': " << reason,
    ,
    ((std::string)name)
    ((std::string)reason)
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    NotInitedObject,
    AlgorithmError,
    "The " << item << " object " << obj << " was not initialized",
    ,
    ((const char *)item)
    ((void *)obj)
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    NotInitedByDalAlgorithm,
    AlgorithmError,
    "The " << obj_id << '@' << obj_class << " object " << address << " was not initialized by DAL algorithm " << algo,
    ,
    ((std::string)obj_id)
    ((std::string)obj_class)
    ((void*)address)
    ((const char *)algo)
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    CannotCreateSegConfig,
    AlgorithmError,
    "Failed to create config for segment \'" << name << "\': " << reason,
    ,
    ((std::string)name)
    ((std::string)reason)
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    CannotGetParents,
    AlgorithmError,
    "Failed to get parents of \'" << object << '\'',
    ,
    ((std::string)object)
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    FoundCircularDependency,
    AlgorithmError,
    "Reach maximum allowed recursion (" << limit << ") during calculation of " << goal << "; possibly there is circular dependency between these objects: " << objects,
    ,
    ((unsigned int)limit)
    ((const char *)goal)
    ((std::string)objects)
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    NoJarFile,
    AlgorithmError,
    "Cannot find jar file \'" << file << "\' described by \'" << obj_id << '@' << obj_class << "\' that is part of \'" << rep_id << '@' << rep_class << '\'',
    ,
    ((std::string)file)
    ((std::string)obj_id)
    ((std::string)obj_class)
    ((std::string)rep_id)
    ((std::string)rep_class)
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    DuplicatedApplicationID,
    AlgorithmError,
    "Two applications have equal IDs:\n  1) " << first << "\n  2) " << second,
    ,
    ((std::string)first)
    ((std::string)second)
  )

  ERS_DECLARE_ISSUE_BASE(
    coredal,
    SegmentIncludedMultipleTimes,
    AlgorithmError,
    "The segment \"" << segment << "\" is included by:\n  1) " << first << "\n  2) " << second,
    ,
    ((std::string)segment)
    ((std::string)first)
    ((std::string)second)
  )

} // namespace dunedaq

#endif
