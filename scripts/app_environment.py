#!/usr/bin/env python3

import sys

import coredal
import oksdbinterfaces

dal = oksdbinterfaces.dal.module('x', 'schema/coredal/dunedaq.schema.xml')

# Process a dal::Variable object, placing key/value pairs in a dictionary
def process_variables(variables, envDict):
  for item in variables:
    if item.className() == 'VariableSet':
      process_variables(item.contains, envDict)
    else:
      if item.className() == 'Variable':
        envDict[item.name] = item.value
 
# Recursively process all Segments in given Segment extracting Applications
def process_segment(db, session, segment):
  controller = segment.controller.id

  # Get default environment from Session
  defenv = {}
  process_variables(session.environment, defenv)

  apps = []

  # Recurse over nested segments
  for seg in segment.segments:
    apps.append(process_segment(db, session, seg))

  # Get all the enabled applications of this segment
  for app in segment.applications:
    #print()
    if not coredal.component_disabled(db._obj, session.id, app.id):
      #print(f"Controller: {controller}, App: {app}")

      appenv = defenv
      # Override with any app specific environment from Application
      process_variables(app.applicationEnvironment, appenv)
      #print(f"Application environment={appenv}")

      host = app.runs_on.runs_on.id
      apps.append((app.id, host, appenv))
    else:
      print(f"Ignoring disabled app {app.id}")
  return apps

def process_services(session):
  services = []
  for srv in session.services:
    if isinstance(srv, dal.Application) and srv.enabled:
      services.append((srv.className(), srv.runs_on))
  return services

def main():
  if len(sys.argv) != 3:
    print(f"Usage: {sys.argv[0]} <path-to-oks-data-file> <session-name>")
    return
  db = oksdbinterfaces.Configuration("oksconfig:" + sys.argv[1])
  session_name = sys.argv[2]
  session = db.get_dal(class_name="Session", uid=session_name)
  #apps = coredal.session_get_all_applications(db._obj, session_name)
  #for apploc in apps:
  #  app = db.get_dal(apploc.class_name, apploc.id)

  environment = {}
  process_variables(session.environment, environment)
  #print(f"Session environment={environment}")

  services = process_services(session)
  apps = process_segment(db, session, session.segment)

  print(f"Services:")
  for s in services:
    print(f"  {s}")
  print("Apps:")
  for a in apps:
    print(f"  {a}")

if __name__ == '__main__':
    main()
