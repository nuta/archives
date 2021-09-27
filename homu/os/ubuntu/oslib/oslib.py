#
#  oslib.py for Ubuntu
#

import gtk
import re
import os
import yaml
from   glob import glob


def parseDesktopFile (path, locale="en"):

  f = open(path, "r")
  res = {}
  isDesktopEntries = False

  for line in f.readlines():

    line = line.strip()

    if line == "":
      continue

    if line[0] == "[":
      if line == "[Desktop Entry]":
        isDesktopEntries = True
      else:
        isDesktopEntries = False

    elif isDesktopEntries:

      try:
        key, val = line.split("=")

      except ValueError:
        pass

      # is a localized value?
      r = re.search("(.*)\[([a-zA-Z0-9\-\_\@]*)\]", key)
      if r != None:
        if r.group(1) == locale:
          res[r.group(0)] = val

      else:
        res[key] = val


  f.close()
  return res




class appinfo:

  def __init__ (self, locale="en"):

    self.appInfoDir = "/usr/share/applications/"
    self.locale     = locale
    self.applist    = []

    # load all applications information (.desktop file)
    for appfile in glob(self.appInfoDir + "*.desktop"):

      info = parseDesktopFile(appfile, self.locale)

      if info.has_key("Exec") and info.has_key("Name"):
        self.applist.append(info)



  def getExecPathByCmd (self, cmd):

    found     = False
    execfile  = cmd.split(" ")[0]

    for path in os.environ["PATH"].split(":"):
      if os.path.exists(path + "/" + execfile):
        found   = True
        dirpath = path
        break

    if found:
      return dirpath + "/" + execfile
    else:
      return execfile



  def getAppInfoByPath (self, path):

    for app in self.applist:

      if self.getExecPathByCmd(app["Exec"]) == path:

        name   = app["Name"]
        icon   = app["Icon"]    if app.has_key("Icon")    else ""
        desc   = app["Comment"] if app.has_key("Comment") else ""

        # look for the app's icon file
        if icon != "":
          theme = gtk.icon_theme_get_default()
          info  = theme.lookup_icon(app["Icon"], 48, 0)

          if info != None:
            icon  = info.get_filename()
          else:
            icon = "static/icons/blank.png"
        else:
          icon = "static/icons/blank.png"


        return {
                 "name":         name,
                 "path":         path,
                 "icon":         "data:image/gif;base64," + open(icon).read().encode("base64"),
                 "description":  desc
               }

    return {}



  def getAllApps (self):

    apps = []

    for app in self.applist:

      icon   = app["Icon"]    if app.has_key("Icon")    else ""
      desc   = app["Comment"] if app.has_key("Comment") else ""


      # look for the app's icon file
      if icon != "":
        theme = gtk.icon_theme_get_default()
        info  = theme.lookup_icon(app["Icon"], 48, 0)

        if info != None:
          icon  = info.get_filename()
        else:
          icon = "static/icons/blank.png"

      else:
        icon = "static/icons/blank.png"

      apps.append({
                    "name":         app["Name"],
                    "path":         self.getExecPathByCmd(app["Exec"]),
                    "icon":         "data:image/gif;base64," + open(icon).read().encode("base64"),
                    "description":  desc
                 })

    return apps




class config:

  def __init__ (self):

    self.configFilePath = os.environ['HOME'] + "/.homu-webui.yaml"


  def load (self):

    f      = open(self.configFilePath)
    config = yaml.load(f.read())
    f.close()
    return config


  def set (self, vars):

    f = open(self.configFilePath)
    yaml.dump(vars, f, encoding="utf-8", allow_unicode=True)
    f.close()
