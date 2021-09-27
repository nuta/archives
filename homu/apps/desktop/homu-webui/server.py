#!/usr/bin/env python
# coding: utf-8

Debug = False

import os
import json
import glob
import yaml
import socket
import oslib
import hashlib
from   uuid    import *
from   homulib import *
from   flask   import *

app = Flask(__name__)


onetimePasswd = hashlib.sha1(os.urandom(256)).hexdigest()
onetimeId     = ""
port          = 0 if not Debug else 12345


# get an unused port
if port == 0:
  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  s.bind(("localhost", 0))
  addr, port = s.getsockname()
  s.close()




def authMe ():

  global onetimeId

  if Debug:
    return

  if session.get("onetimeId") != onetimeId:
    abort(403)

  return


def loadConfig ():

  conf = oslib.config()

  # show welcome pages if the config file is not exist
  if not os.path.exists(conf.configFilePath):
    return {}

  return conf.load()



#
#
#  /auth
#
#

@app.route("/auth/<challenge>")
def auth (challenge=""):

  global onetimeId

  if challenge == onetimePasswd:

    onetimeId = str(uuid4()) + hashlib.sha1(os.urandom(128)).hexdigest()
    session["onetimeId"] = onetimeId
    return redirect(url_for("index"))

  abort(401)



#
#
#  /
#
#

@app.route("/")
def index ():

  authMe()

  if loadConfig() == {}:
    return render_template("welcome.html")

  return render_template("index.html")


#
#
#  /set
#
#

@app.route("/set/<type>", methods=["POST"])
def set (type):

  authMe()

  homu   = homulib()
  config = loadConfig()

  # Config
  if type == "config":

    osconfig = oslib.config()

    newconf = {
                "locale": request.form["locale"]
              }

    f = open(osconfig.configFilePath, "w")
    yaml.dump(newconf, f, encoding="utf8", allow_unicode=True)

  # Exceptional Application
  if type == "exApp":
    homu.setAppRule(request.form["path"], request.form["res"].strip().split(","))

  # User Resource
  elif type == "user_res":
    homu.addUserResource(request.form["desc"], request.form["type"], request.form["str"])

  # Mode
  elif type == "mode":
    homu.setMode(request.form["mode"])

  # error: unknown type
  else:
    return json.dumps({"error": "invalid_request"}), 400

  return json.dumps({})


#
#
#  /delete
#
#

@app.route("/delete/<type>", methods=["POST"])
def delete (type):

  authMe()

  config = loadConfig()
  homu   = homulib()

  #
  #  Exceptional Application
  #
  if type == "exApp":

    homu.deleteAppRule(request.form["path"])

  #
  #  User Resource
  #
  elif type == "user_res":

    homu.deleteUserResource(request.form["id"])

  #
  #  error: unknown type
  #
  else:
    return json.dumps({"error": "invalid_request"}), 400


  return json.dumps({})


#
#
#  /notifications
#
#

@app.route("/notifications/")
@app.route("/notifications/<int:readP>")
def notifications (readP=0):

  config = loadConfig()

  homu = homulib()
  log  = homu.getLog(readP)
  return json.dumps({"logid": log["readP"], "log": log["unread"]})


#
#
#  /applications
#
#

@app.route("/applications/<type>")
def applications (type):

  authMe()

  config = loadConfig()
  homu    = homulib()
  appinfo = oslib.appinfo(config["locale"])
  apps    = []

  #
  #  All Applications
  #
  if   type == "all":

    ## TODO read *.desktop
    for app in appinfo.getAllApps():
      apps.append(app)

  #
  #  Exceptional Applications
  #
  elif type == "ex":

    for app in homu.getAppRuleAll():

      info = appinfo.getAppInfoByPath(app["path"])

      if info == {}:
        continue

      info["res"] = app["res"]
      apps.append(info)


  #
  #  error: unknown type
  #
  else:
    return json.dumps({"error": "invalid_request"}), 400

  return json.dumps(apps)


#
#
#  /userres
#
#

@app.route("/userres/<type>")
def userres (type):

  authMe()

  config = loadConfig()
  homu   = homulib()
  ures   = []

  if type == "all":

    for res in homu.getUserResourceAll():
      ures.append(res)

  #
  #  error: unknown type
  #
  else:
    return json.dumps({"error": "invalid_request"}), 400

  return json.dumps(ures)


#
#
#  /apply_rules
#
#

@app.route("/apply_rules", methods=["POST"])
def apply_rules ():

  authMe()

  homu = homulib()
  homu.applyRules()

  return json.dumps({})


#
#
#  /status
#
#

@app.route("/status")
def status ():

  authMe()

  homu = homulib()
  return json.dumps(homu.getInfo())


#
#  /logout
#

@app.route("/logout")
def logout ():

  authMe()
  stopServer()


#
#  /filelist
#
@app.route("/filelist")
def filelist():

  if request.args["path"] == "~":
    path = os.environ["HOME"]
  else:
    path = request.args["path"]

  list = [{
            "type":    "dir",
            "name":    "Parent Directory" ,
            "abspath": os.path.abspath(path) + "/.."
         }]

  for file in glob.glob(path + "/*"):

    list.append({
                  "type":    "dir" if os.path.isdir(file) else "file",
                  "name":    os.path.basename(file),
                  "abspath": os.path.abspath(file)
               })

  return json.dumps(list)


def startServer ():
  app.secret_key = os.urandom(256)
  app.run(host="127.0.0.1", port=port, debug=Debug, use_reloader=Debug)


def stopServer ():
  os._exit(1)



if __name__ == "__main__":

  print "Welcome to Homu WebUI Server!%s" % (" (Debug Mode)" if Debug else "",)
  print "http://localhost:%s/auth/%s" % (port, onetimePasswd)
  sys.stdout.flush()

  startServer()

