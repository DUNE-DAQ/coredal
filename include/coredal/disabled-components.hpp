#ifndef DUNEDAQDAL_DISABLED_COMPONENTS_H
#define DUNEDAQDAL_DISABLED_COMPONENTS_H

#include <string>
#include <vector>

#include "oksdbinterfaces/Configuration.hpp"
#include "oksdbinterfaces/ConfigAction.hpp"

#include "coredal/Component.hpp"

namespace dunedaq::coredal {

    class Session;
    class ResourceSet;
    // class Segment;

    class DisabledComponents : public dunedaq::oksdbinterfaces::ConfigAction
    {

      friend class Session;
      friend class Component;

    private:

      struct SortStringPtr
      {
        bool
        operator()(const std::string * s1, const std::string * s2) const
        {
          return (*s1 < *s2);
        }
      };

      dunedaq::oksdbinterfaces::Configuration& m_db;
      Session* m_session;

      unsigned long m_num_of_slr_enabled_resources;
      unsigned long m_num_of_slr_disabled_resources;

      std::set<const std::string *, SortStringPtr> m_disabled;
      std::set<const dunedaq::coredal::Component *> m_user_disabled;
      std::set<const dunedaq::coredal::Component *> m_user_enabled;

      void
      __clear() noexcept
      {
        m_disabled.clear();
        m_user_disabled.clear();
        m_user_enabled.clear();
        m_num_of_slr_enabled_resources = 0;
        m_num_of_slr_disabled_resources = 0;
      }

    public:

      DisabledComponents(dunedaq::oksdbinterfaces::Configuration& db, Session* session);

      virtual
      ~DisabledComponents();

      void
      notify(std::vector<dunedaq::oksdbinterfaces::ConfigurationChange *>& /*changes*/) noexcept;

      void
      load() noexcept;

      void
      unload() noexcept;

      void
      update(const dunedaq::oksdbinterfaces::ConfigObject& obj, const std::string& name) noexcept;

      void
      reset() noexcept;

      size_t
      size() noexcept
      {
        return m_disabled.size();
      }

      void
      disable(const dunedaq::coredal::Component& c)
      {
        m_disabled.insert(&c.UID());
      }

      bool
      is_enabled(const dunedaq::coredal::Component* c);

      bool
      is_enabled_short(const dunedaq::coredal::Component* c)
      {
        return (m_disabled.find(&c->UID()) == m_disabled.end());
      }

      void
      disable_children(const dunedaq::coredal::ResourceSet&);

      void
      disable_children(const dunedaq::coredal::Segment&);

      static unsigned long
      get_num_of_slr_resources(const dunedaq::coredal::Session& p);

    };
} // namespace dunedaq::coredal

#endif // DUNEDAQDAL_DISABLED_COMPONENTS_H
