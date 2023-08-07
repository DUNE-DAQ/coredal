#include "coredal/Application.hpp"
#include "coredal/ResourceSet.hpp"
#include "coredal/ResourceSetAND.hpp"
#include "coredal/ResourceSetOR.hpp"
#include "coredal/Segment.hpp"
#include "coredal/Session.hpp"
#include "coredal/util.hpp"
#include "coredal/disabled-components.hpp"

#include "logging/Logging.hpp"

#include "test_circular_dependency.hpp"

using namespace dunedaq::oksdbinterfaces;

dunedaq::coredal::DisabledComponents::DisabledComponents(Configuration& db,
 Session* session) :
  m_db(db),
  m_session(session),
  m_num_of_slr_enabled_resources(0),
  m_num_of_slr_disabled_resources(0)
{
  TLOG_DEBUG(2) <<  "construct the object " << (void *)this  ;
  m_db.add_action(this);
}

dunedaq::coredal::DisabledComponents::~DisabledComponents()
{
  TLOG_DEBUG(2) <<  "destroy the object " << (void *)this ;
  m_db.remove_action(this);
}

void
dunedaq::coredal::DisabledComponents::notify(std::vector<ConfigurationChange *>& /*changes*/) noexcept
{
  TLOG_DEBUG(2) <<  "reset session components because of notification callback on object " << (void *)this ;
  __clear();
}

void
dunedaq::coredal::DisabledComponents::load() noexcept
{
  TLOG_DEBUG(2) <<  "reset session components because of configuration load on object " << (void *)this ;
  __clear();
}

void
dunedaq::coredal::DisabledComponents::unload() noexcept
{
  TLOG_DEBUG(2) <<  "reset session components because of configuration unload on object " << (void *)this ;
  __clear();
}

void
dunedaq::coredal::DisabledComponents::update(const ConfigObject& obj, const std::string& name) noexcept
{
  TLOG_DEBUG(2) <<  "reset session components because of configuration update (obj = " << obj << ", name = \'" << name << "\') on object " << (void *)this ;
  __clear();
}

void
dunedaq::coredal::DisabledComponents::reset() noexcept
{
  TLOG_DEBUG(2) <<  "reset disabled by explicit user call" ;
  m_disabled.clear(); // do not clear s_user_disabled && s_user_enabled !!!
}

bool
dunedaq::coredal::DisabledComponents::is_enabled(const dunedaq::coredal::Component * c)
{
  if (const dunedaq::coredal::Segment * seg = c->cast<dunedaq::coredal::Segment>()) {
    return !seg->disabled(*m_session);
  }


  return is_enabled_short(c);
}



void
dunedaq::coredal::DisabledComponents::disable_children(const dunedaq::coredal::ResourceSet& rs)
{
  for (auto & res : rs.get_contains()) {
    disable(*res);
    if (const auto * rs2 = res->cast<dunedaq::coredal::ResourceSet>()) {
      disable_children(*rs2);
    }
  }
}

void
dunedaq::coredal::DisabledComponents::disable_children(const dunedaq::coredal::Segment& segment)
{
  for (auto & res : segment.get_resources()) {
    if (const auto * rs = res->cast<dunedaq::coredal::ResourceSet>()) {
      disable_children(*rs);
    }
  }
  for (auto & app : segment.get_applications()) {
    auto res = app->cast<dunedaq::coredal::Component>();
    if (res) {
      disable(*res);
    }
  }
  for (auto & seg : segment.get_segments()) {
    TLOG_DEBUG(6) <<  "disable segment " << seg << " because it's parent segment " << &segment << " is disabled" ;
    disable(*seg);
    disable_children(*seg);
  }
}

void
dunedaq::coredal::Session::set_disabled(const std::set<const dunedaq::coredal::Component *>& objs) const
{
  m_disabled_components.m_user_disabled.clear();

  for (const auto& comp : objs) {
    m_disabled_components.m_user_disabled.insert(comp);
  }
  m_disabled_components.m_num_of_slr_disabled_resources = m_disabled_components.m_user_disabled.size();

  m_disabled_components.reset();
}

void
dunedaq::coredal::Session::set_enabled(const std::set<const dunedaq::coredal::Component *>& objs) const
{
  m_disabled_components.m_user_enabled.clear();

  for (const auto& i : objs) {
    m_disabled_components.m_user_enabled.insert(i);
  }
  m_disabled_components.m_num_of_slr_enabled_resources = m_disabled_components.m_user_enabled.size();

  m_disabled_components.reset();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace dunedaq {
  ERS_DECLARE_ISSUE_BASE(
    coredal,
    ReadMaxAllowedIterations,
    AlgorithmError,
    "Has exceeded the maximum of iterations allowed (" << limit << ") during calculation of disabled objects",
    ,
    ((unsigned int)limit)
  )
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // fill data from resource sets

static void fill(
  const dunedaq::coredal::ResourceSet& rs,
  std::vector<const dunedaq::coredal::ResourceSetOR *>& rs_or,
  std::vector<const dunedaq::coredal::ResourceSetAND *>& rs_and,
  dunedaq::coredal::TestCircularDependency& cd_fuse
)
{
  if (const dunedaq::coredal::ResourceSetAND * r1 = rs.cast<dunedaq::coredal::ResourceSetAND>())
    {
      rs_and.push_back(r1);
    }
  else if (const dunedaq::coredal::ResourceSetOR * r2 = rs.cast<dunedaq::coredal::ResourceSetOR>())
    {
      rs_or.push_back(r2);
    }

  for (auto & i : rs.get_contains())
    {
      dunedaq::coredal::AddTestOnCircularDependency add_fuse_test(cd_fuse, i);
      if (const dunedaq::coredal::ResourceSet * rs2 = i->cast<dunedaq::coredal::ResourceSet>())
        {
          fill(*rs2, rs_or, rs_and, cd_fuse);
        }
    }
}


  // fill data from segments

static void fill(
  const dunedaq::coredal::Segment& s,
  std::vector<const dunedaq::coredal::ResourceSetOR *>& rs_or,
  std::vector<const dunedaq::coredal::ResourceSetAND *>& rs_and,
  dunedaq::coredal::TestCircularDependency& cd_fuse
)
{
  for (auto & app : s.get_applications()) {
    dunedaq::coredal::AddTestOnCircularDependency add_fuse_test(cd_fuse, app);
    if (const dunedaq::coredal::ResourceSet * rs = app->cast<dunedaq::coredal::ResourceSet>()) {
      fill(*rs, rs_or, rs_and, cd_fuse);
    }
  }
  for (auto & res : s.get_resources()) {
    dunedaq::coredal::AddTestOnCircularDependency add_fuse_test(cd_fuse, res);
    if (const dunedaq::coredal::ResourceSet * rs = res->cast<dunedaq::coredal::ResourceSet>()) {
      fill(*rs, rs_or, rs_and, cd_fuse);
    }
  }

  for (auto & seg : s.get_segments()) {
    TLOG_DEBUG(6) << "Filling segment " << seg->UID();
    dunedaq::coredal::AddTestOnCircularDependency add_fuse_test(cd_fuse, seg);
    fill(*seg, rs_or, rs_and, cd_fuse);
  }
}


  // fill data from session

static void fill(
  const dunedaq::coredal::Session& session,
  std::vector<const dunedaq::coredal::ResourceSetOR *>& rs_or,
  std::vector<const dunedaq::coredal::ResourceSetAND *>& rs_and,
  dunedaq::coredal::TestCircularDependency& cd_fuse
)
{
#if 0
  if (const dunedaq::coredal::OnlineSegment * onlseg = p.get_OnlineInfrastructure())
    {
      dunedaq::coredal::AddTestOnCircularDependency add_fuse_test(cd_fuse, onlseg);
      fill(*onlseg, rs_or, rs_and, cd_fuse);

      // NOTE: normally application may not be ResourceSet, but for some "exotic" cases put this code
      for (auto &a : p.get_OnlineInfrastructureApplications())
        {
          if (const dunedaq::coredal::ResourceSet * rs = a->cast<dunedaq::coredal::ResourceSet>())
            {
              fill(*rs, rs_or, rs_and, cd_fuse);
            }
        }
    }
#endif

  auto seg = session.get_segment();
  dunedaq::coredal::AddTestOnCircularDependency add_fuse_test(cd_fuse, seg);
  fill(*seg, rs_or, rs_and, cd_fuse);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool
dunedaq::coredal::Component::disabled(const dunedaq::coredal::Session& session, bool skip_check) const
{
  TLOG_DEBUG( 6) << "Session UID: " << session.UID() << " this->UID()=" << UID();
  // fill disabled (e.g. after session changes)

  if (session.m_disabled_components.size() == 0) {
    if (session.get_disabled().empty() && 
        session.m_disabled_components.m_user_disabled.empty()) {
      TLOG_DEBUG( 6) << "Session has no disabled components";
      return false;  // the session has no disabled components
    }
    else {
      // get two lists of all session's resource-set-or and resource-set-and
      // also test any circular dependencies between segments and resource sets
      dunedaq::coredal::TestCircularDependency cd_fuse("component \'is-disabled\' status", &session);
      std::vector<const dunedaq::coredal::ResourceSetOR *> rs_or;
      std::vector<const dunedaq::coredal::ResourceSetAND *> rs_and;
      fill(session, rs_or, rs_and, cd_fuse);

      // calculate explicitly and implicitly (nested) disabled components
      {
        std::vector<const dunedaq::coredal::Component *> vector_of_disabled;
        vector_of_disabled.reserve(session.get_disabled().size() + session.m_disabled_components.m_user_disabled.size());

        // add user disabled components, if any
        for (auto & i : session.m_disabled_components.m_user_disabled) {
          vector_of_disabled.push_back(i);
          TLOG_DEBUG(6) <<  "disable component " << i->UID() << " because it is explicitly disabled by user" ;
        }

        // add session-disabled components ignoring explicitly enabled by user
        for (auto & i : session.get_disabled()) {
          TLOG_DEBUG(6) <<  "check component " << i->UID() << " explicitly disabled in session" ;

          if (session.m_disabled_components.m_user_enabled.find(i) == session.m_disabled_components.m_user_enabled.end()) {
            vector_of_disabled.push_back(i);
            TLOG_DEBUG(6) <<  "disable component " << i->UID() << " because it is not explicitly enabled in session" ;
          }
          else {
            TLOG_DEBUG(6) <<  "skip component " << i->UID() << " because it is enabled by user" ;
          }
        }

        // fill set of explicitly and implicitly (segment/resource-set containers) disabled components
        for (auto & i : vector_of_disabled) {
          session.m_disabled_components.disable(*i);

          if (const dunedaq::coredal::ResourceSet * rs = i->cast<dunedaq::coredal::ResourceSet>()) {
            session.m_disabled_components.disable_children(*rs);
          }
          else if (const dunedaq::coredal::Segment * seg = i->cast<dunedaq::coredal::Segment>()) {
            TLOG_DEBUG(6) << "Disabling children of segment " << seg->UID();
            session.m_disabled_components.disable_children(*seg);
          }
        }
      }

      for (unsigned long count = 1; true; ++count) {
        const unsigned long num(session.m_disabled_components.size());

        TLOG_DEBUG(6) <<  "before auto-disabling iteration " << count << " the number of disabled components is " << num ;

        TLOG_DEBUG(6) <<  "Session has " << rs_or.size() << " resourceSetORs";
        for (const auto& i : rs_or) {
          if (session.m_disabled_components.is_enabled_short(i)) {
            // check ANY child is disabled
            TLOG_DEBUG(6) << "ResourceSetOR " << i->UID() << " contains " << i->get_contains().size() << " resources";
            for (auto & i2 : i->get_contains()) {
              if (!session.m_disabled_components.is_enabled_short(i2)) {
                TLOG_DEBUG(6) <<  "disable resource-set-OR " << i->UID() << " because it's child " << i2 << " is disabled" ;
                session.m_disabled_components.disable(*i);
                session.m_disabled_components.disable_children(*i);
                break;
              }
            }
          }
        }

        TLOG_DEBUG(6) <<  "Session has " << rs_and.size() << " resourceSetANDs";
        for (const auto& j : rs_and) {
          if (session.m_disabled_components.is_enabled_short(j)) {
            const std::vector<const dunedaq::coredal::ResourceBase*> &resources = j->get_contains();
            TLOG_DEBUG(6) << "Checking " << resources.size() << " ResourceSetAND resources";
            if (!resources.empty()) {
              // check ANY child is enabled
              bool found_enabled = false;
              for (auto & j2 : resources) {
                if (session.m_disabled_components.is_enabled_short(j2)) {
                  found_enabled = true;
                  TLOG_DEBUG(6) << "Found enabled resource " << j2->UID();
                  break;
                }
              }
              if (found_enabled == false) {
                TLOG_DEBUG(6) <<  "disable resource-set-AND " << j->UID() << " because all it's children are disabled" ;
                session.m_disabled_components.disable(*j);
                session.m_disabled_components.disable_children(*j);
              }
            }
          }
        }

        if (session.m_disabled_components.size() == num) {
          TLOG_DEBUG(6) <<  "after " << count << " iteration(s) auto-disabling algorithm found no newly disabled sets, exiting loop ..." ;
          break;
        }

        unsigned int iLimit(1000);
        if (count > iLimit) {
          ers::error(dunedaq::coredal::ReadMaxAllowedIterations(ERS_HERE, iLimit));
          break;
        }
      }
    }
  }

  bool result(skip_check ? !session.m_disabled_components.is_enabled_short(this) : !session.m_disabled_components.is_enabled(this));
  TLOG_DEBUG( 6) <<  "disabled(" << this << ")  (UID=" << UID() << ") returns " << std::boolalpha << result  ;
  return result;
}

unsigned long
dunedaq::coredal::DisabledComponents::get_num_of_slr_resources(const dunedaq::coredal::Session& session)
{
  return (session.m_disabled_components.m_num_of_slr_enabled_resources + session.m_disabled_components.m_num_of_slr_disabled_resources);
}
