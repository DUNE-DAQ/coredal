/**
 * @file dalMethods.cxx
 *
 * Implementations of Methods defined in coredal schema classes
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "coredal/Application.hpp"
#include "coredal/Component.hpp"
#include "coredal/DaqApplication.hpp"
#include "coredal/DaqModule.hpp"
#include "coredal/Resource.hpp"
#include "coredal/ResourceSetAND.hpp"
#include "coredal/ResourceSetOR.hpp"
#include "coredal/Segment.hpp"
#include "coredal/Session.hpp"

#include "test_circular_dependency.hpp"

#include "oksdbinterfaces/ConfigObject.hpp"

#include <list>
#include <set>
#include <iostream>

// Stolen from ATLAS dal package
using namespace dunedaq::oksdbinterfaces;

namespace dunedaq::coredal {
  /**
   *  Static function to calculate list of components
   *  from the root segment to the lowest component which
   *  the child object (a segment or a resource) belongs.
   */

static void
make_parents_list(
    const ConfigObjectImpl * child,
    const dunedaq::coredal::ResourceSet * resource_set,
    std::vector<const dunedaq::coredal::Component *> & p_list,
    std::list< std::vector<const dunedaq::coredal::Component *> >& out,
    dunedaq::coredal::TestCircularDependency& cd_fuse)
{
  dunedaq::coredal::AddTestOnCircularDependency add_fuse_test(cd_fuse, resource_set);

  // add the resource set to the path
  p_list.push_back(resource_set);

  // check if the application is in the resource relationship, i.e. is a resource or belongs to resource set(s)
  for (const auto& i : resource_set->get_contains()) {
    if (i->config_object().implementation() == child) {
      out.push_back(p_list);
    }
    else if (const dunedaq::coredal::ResourceSet * rs = i->cast<dunedaq::coredal::ResourceSet>()) {
      make_parents_list(child, rs, p_list, out, cd_fuse);
    }
  }

  // remove the resource set from the path
  p_list.pop_back();
}

static void
make_parents_list(
    const ConfigObjectImpl * child,
    const dunedaq::coredal::Segment * segment,
    std::vector<const dunedaq::coredal::Component *> & p_list,
    std::list<std::vector<const dunedaq::coredal::Component *> >& out,
    bool is_segment,
    dunedaq::coredal::TestCircularDependency& cd_fuse)
{
  dunedaq::coredal::AddTestOnCircularDependency add_fuse_test(cd_fuse, segment);

  // add the segment to the path
  p_list.push_back(segment);

  // check if the application is in the nested segment
  for (const auto& seg : segment->get_segments()) {
    if (seg->config_object().implementation() == child)
      out.push_back(p_list);
    else
      make_parents_list(child, seg, p_list, out, is_segment, cd_fuse);
  }
  if (!is_segment) {
    for (const auto& app : segment->get_applications()) {
      if (app->config_object().implementation() == child)
        out.push_back(p_list);
      else if (const auto resource_set = app->cast<dunedaq::coredal::ResourceSet>())
        make_parents_list(child, resource_set, p_list, out, cd_fuse);
    }
  }

  // remove the segment from the path

  p_list.pop_back();
}


static void
check_segment(
    std::list< std::vector<const dunedaq::coredal::Component *> >& out,
    const dunedaq::coredal::Segment * segment,
    const ConfigObjectImpl * child,
    bool is_segment,
    dunedaq::coredal::TestCircularDependency& cd_fuse)
{
  dunedaq::coredal::AddTestOnCircularDependency add_fuse_test(cd_fuse, segment);

  std::vector<const dunedaq::coredal::Component *> compList;

  if (segment->config_object().implementation() == child) {
    out.push_back(compList);
  }
  make_parents_list(child, segment, compList, out, is_segment, cd_fuse);
}

void
dunedaq::coredal::Component::get_parents(
  const dunedaq::coredal::Session& session,
  std::list<std::vector<const dunedaq::coredal::Component *>>& parents) const
{
  const ConfigObjectImpl * obj_impl = config_object().implementation();

  const bool is_segment = castable(dunedaq::coredal::Segment::s_class_name);

  try {
    dunedaq::coredal::TestCircularDependency cd_fuse("component parents", &session);

    // check session's segment
    check_segment(parents, session.get_segment(), obj_impl, is_segment,
                  cd_fuse);


    if (parents.empty()) {
      TLOG_DEBUG(1) <<  "cannot find segment/resource path(s) between Component " << this << " and session " << &session << " objects (check this object is linked with the session as a segment or a resource)" ;
    }
  }
  catch (ers::Issue & ex) {
    ers::error(CannotGetParents(ERS_HERE, full_name(), ex));
  }
}

// ========================================================================

static std::vector<const Application*> getSegmentApps(const Segment* segment) {
  auto apps = segment->get_applications();
  for (auto seg : segment->get_segments()) {
    auto segapps = getSegmentApps(seg);
    apps.insert(apps.end(), segapps.begin(),segapps.end());
  }
  return apps;
}

std::vector<const Application*>
Session::get_all_applications() const {
  std::vector<const Application*> apps;
  auto segapps = getSegmentApps(m_segment);
  apps.insert(apps.end(), segapps.begin(),segapps.end());
  return apps;
}

// ========================================================================

std::set<const HostComponent*>
DaqApplication::get_used_hostresources() const {
  std::set<const HostComponent*> res;
  for (auto module :  get_modules()) {
    for (auto hostresource : module->get_used_resources()) {
      res.insert(hostresource);
    }
  }
  return res;
}

}
