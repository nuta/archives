#!/usr/bin/env python
#
#  homu-cmdui.py - the CUI-based homu manager
#

import os
import sys
import argparse
import pickle
from homulib import *


versionStr = "1.00"

#
#
#  sub commands
#
#


## delete an app rule
def action_deleteAppRule (args):

  path = os.path.abspath(args["path"])

  homu = homulib()
  homu.deleteAppRule(path)



## delete an user resource
def action_deleteUserRes (id):

  id  = args["id"]

  homu = homulib()
  homu.deleteUserResource(id)



## add a new user resource
def action_addUserRes (args):

  desc = args["desc"]
  type = args["type"]
  str  = args["str"]

  homu = homulib()
  homu.addUserResource(desc, type, str)



## set an app rule
def action_setAppRule (args):

  path = os.path.abspath(args["path"])
  res  = tuple(args["res"].split(","))

  homu = homulib()
  homu.setAppRule(path, res)



## set the default app rule
def action_setDefaultAppRule (args):

  res = tuple(args["res"].split(","))

  homu = homulib()
  homu.setDefaultAppRule(res)



## show all user resources
def action_showUserRes (args):

  homu = homulib()

  for resource in homu.getUserResourceAll():
    print "id: %s [%s] %s - %s" % (resource["id"], resource["type"], resource["str"], resource["desc"])



## show all app rules
def action_showAppRules (args):

  homu = homulib()

  for app in homu.getAppRuleAll():

    resources = ""

    for res in app["res"]:
      resources += " %s" % (res,)

    print "%s -%s" % (app["path"], resources)



## show the default app rule
def action_showDefaultAppRule (args):

  homu = homulib()

  resources = ""

  for resource in homu.getDefaultAppRule():
    resources += "%s " % (resource,)

  print "%s" % (resources,)



## show log
def action_showLog (args):

  homu = homulib()

  for log in homu.getLog()["unread"]:

    args = ""

    for arg in log["args"]:
      args += "%s " % (arg,)

    print "%s: %s" % (log["type"], args)



## show status
def action_showStatus (args):

  homu = homulib()
  info = homu.getInfo()

  print "version: %s (Homu/Main)"        % (info["version"],)
  print "         %s (Homu/App Homulib)" % (versionStr,)
  print "mode:    %s"                    % (info["mode"],)



## apply-rules
def action_applyRules (args):

  homu = homulib()
  homu.applyRules()



## set-mode
def action_setMode (args):

  mode = args["mode"]

  homu = homulib()
  homu.setMode(mode)



#
#
#  the entry point
#
#

if __name__ == "__main__":

  #
  #  parse command-line arguments
  #
  arg = argparse.ArgumentParser(description="the CUI-based homu manager.")
  arg.add_argument("--version", action="version", version=versionStr)
  subcmds = arg.add_subparsers(title="action")


  # del-userres
  subcmd = subcmds.add_parser("del-userres", help="delete an user resource")
  subcmd.add_argument("id")
  subcmd.set_defaults(func=action_deleteUserRes)

  # del-apprule
  subcmd = subcmds.add_parser("del-apprule", help="set an app rule")
  subcmd.add_argument("path")
  subcmd.set_defaults(func=action_deleteAppRule)

  # add-userres
  subcmd = subcmds.add_parser("add-userres", help="add a new user resource")
  subcmd.add_argument("desc")
  subcmd.add_argument("type")
  subcmd.add_argument("str")
  subcmd.set_defaults(func=action_addUserRes)

  # set-apprule
  subcmd = subcmds.add_parser("set-apprule", help="set an app rule")
  subcmd.add_argument("path")
  subcmd.add_argument("res")
  subcmd.set_defaults(func=action_setAppRule)

  # set-default-apprule
  subcmd = subcmds.add_parser("set-default-apprule", help="set the default app rule")
  subcmd.add_argument("res")
  subcmd.set_defaults(func=action_setDefaultAppRule)

  # show-userres
  subcmd = subcmds.add_parser("show-userres", help="show all user resources")
  subcmd.set_defaults(func=action_showUserRes)

  # show-apprules
  subcmd = subcmds.add_parser("show-apprules", help="show all app rules")
  subcmd.set_defaults(func=action_showAppRules)

  # show-default-apprule
  subcmd = subcmds.add_parser("show-default-apprule", help="show the default app rule")
  subcmd.set_defaults(func=action_showDefaultAppRule)

  # set-mode
  subcmd = subcmds.add_parser("set-mode", help="set mode")
  subcmd.set_defaults(func=action_setMode)

  # apply-rules
  subcmd = subcmds.add_parser("apply-rules", help="apply rules")
  subcmd.set_defaults(func=action_applyRules)

  # show-log
  subcmd = subcmds.add_parser("show-log", help="show log")
  subcmd.set_defaults(func=action_showLog)

  # show-status
  subcmd = subcmds.add_parser("show-status", help="show status")
  subcmd.set_defaults(func=action_showStatus)



  #
  #  execute the sub command
  #

  arg.parse_args().func(vars(arg.parse_args()))


