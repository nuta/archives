#
#  homulib-ubuntu.py - the library for homu apps on Ubuntu
#

import os
import pwd
import sqlite3
import pickle
from   uuid     import uuid4



resourceId ={

  "NET_CONNECT":       1,
  "NET_SERVER":        2,
  "FS_READ":           3,
  "FS_WRITE":          4,
  "SYS_CHANGE":        5,
  "HW_FULLCTRL":       6,
  "USERRES_BEGINNING": 10

}



## get well-known resource id by the name
#
#  @return the resource id (Integer)
#

def getResIdByName (name):

  if not resourceId.has_key(name):
    return 0

  return resourceId[name]




## check is the @c type is a valid resource type
#
#  @return True if @c type is valid resource type or False if @c type is not valid one
#

def isValidResType (type):

  if type == "FILE":
    return True

  return False



## the library for Homu apps
class homulib:

  ## the constructor
  def __init__ (self):

    ## the path for the user's rule database file
    self.dbFilePath           = os.environ['HOME'] + "/.homu.sqlite3"

    ## the operating system's name
    self.os                   = "Linux"

    ## the path for homu_query
    self.queryPath            = "/proc/homu_query"

    ## the path for homu_query
    self.statusPath           = "/proc/homu_status"

    ## the path for homu_log
    self.logPath              = "/proc/homu_log"

    ## default app rule path
    # @warning  This value should be invalid file path.
    self.defaultAppRulePath   = "///defaultAppRule///"

    ## apprules' table name
    self.appruleTableName     = "apps"

    ## user resources' table name
    self.userresTableName     = "userres"

    ## the line number where I have read
    self.logReadP             = 0


    #
    #  create a database file if it does not exist
    #

    if not os.path.exists(self.dbFilePath):

      db = sqlite3.connect(self.dbFilePath, isolation_level=None)

      db.execute("""CREATE TABLE %s (
                                      path   TEXT    NOT NULL PRIMARY KEY,
                                      res    BLOB
                                    )
                 """ % (self.appruleTableName,))

      db.execute("""CREATE TABLE %s (
                                      id    TEXT     NOT NULL PRIMARY KEY,
                                      data  BLOB     NOT NULL,
                                      desc  TEXT
                                    )
                 """ % (self.userresTableName,))

      db.close()
      os.chmod(self.dbFilePath, 0600)

      ## set the default app rule
      self.setDefaultAppRule((1, 2, 3, 4, 5, 6));


  ## the deconstructor
  def __del__ (self):

    return


  ## delete an app rule
  #
  #  This method deletes an app rule.
  #
  #  @param path  the path for the application file (executable file or script file)
  #  @param res   a tuple which has resource ids
  #
  #  @return  none
  #

  def deleteAppRule (self, path):

    db = sqlite3.connect(self.dbFilePath, isolation_level=None)
    db.execute(u"DELETE FROM %s WHERE path=?" % (self.appruleTableName,), (path,))
    db.close()




  ## delete an user resource
  #
  #  This method deletes an user resource.
  #
  #  @param type  the resource id
  #
  #  @return none
  #

  def deleteUserResource (self, id):

    # delete from apps' resource lists
    for app in self.getAppRuleAll():

      delete  = False
      reslist = []

      for res in app["res"]:

        if id == res:
          delete = True
          continue

        reslist.append(res)

      self.setAppRule(app["path"], reslist)

    # delete from the userres table
    db = sqlite3.connect(self.dbFilePath, isolation_level=None)
    db.execute(u"DELETE FROM %s WHERE id=?" % (self.userresTableName,), (id,))
    db.close()




  ## set an app rule
  #
  #  This method set an app rule for the @c path.
  #  The rule for @c path will be overwritten if it already exists.
  #
  #
  #  @param path  the path for the application file (executable file or script file)
  #  @param res   a tuple which has resource ids
  #
  #  @return  none
  #

  def setAppRule (self, path, res):

    db = sqlite3.connect(self.dbFilePath, isolation_level=None)
    db.execute(u"INSERT OR REPLACE INTO %s VALUES (?, ?)" % (self.appruleTableName,), (path, pickle.dumps(res)))
    db.close()




  ## set the default app rule
  #
  #  The default app rule should have all permission to well-known resources.
  #
  #  @param res  a tuple which has resource ids
  #
  #  @return  none
  #

  def setDefaultAppRule (self, res):

    db = sqlite3.connect(self.dbFilePath, isolation_level=None)
    db.execute(u"INSERT OR REPLACE INTO %s VALUES (?, ?)" % (self.appruleTableName,), (self.defaultAppRulePath, pickle.dumps(res)))
    db.close()




  ## add an user resource
  #
  #  This method adds a new user resource.
  #  The @c id 's resource will be overwritten if it already exists.
  #
  #  @note  This method now supports "file" type.
  #
  #  @param desc   the description
  #  @param type   the type of the user resource
  #  @param string the string which specifies the resource (ex. filepath)
  #
  #  @return none
  #

  def addUserResource (self, desc, type, string):

    if not isValidResType(type):
      return

    db = sqlite3.connect(self.dbFilePath, isolation_level=None)
    db.execute(u"INSERT OR IGNORE INTO %s VALUES (?,?,?)" % (self.userresTableName,), (str(uuid4()), pickle.dumps((type, string)), desc))
    db.close()




  ## get an app rule
  #
  #  @param path  the path for the application file
  #
  #  @return  the app rule (a tuple which has resource id's)
  #

  def getAppRule (self, path):

    db = sqlite3.connect(self.dbFilePath, isolation_level=None)
    c  = db.execute(u"SELECT res FROM %s WHERE id=?" % (self.appruleTableName,), (path,))
    r  = c.fetchone()
    db.close()
    return r




  ## get all app rules
  #
  #  @return  all app rules
  #

  def getAppRuleAll (self):

    db = sqlite3.connect(self.dbFilePath, isolation_level=None)
    c  = db.execute(u"SELECT * FROM %s" % (self.appruleTableName,))
    r  = c.fetchall()
    db.close()

    apps = []

    for app in r:

      # hide the default app rule
      if app[0] == self.defaultAppRulePath:
        continue

      resources = []

      for resource in pickle.loads(app[1].encode("utf-8", "ignore")):
        resources.append(resource)

      apps.append({"path": app[0], "res": resources})

    return apps




  ## get the default app rule
  #
  #  @return  the default app rule (a tuple which has resource id's)
  #

  def getDefaultAppRule (self):

    db = sqlite3.connect(self.dbFilePath, isolation_level=None)
    c  = db.execute(u"SELECT res FROM %s WHERE path=?" %  (self.appruleTableName,), (self.defaultAppRulePath,))
    r  = c.fetchone()
    db.close()

    resources = []

    for resource in pickle.loads(r[0].encode("utf-8", "ignore")):
      resources.append(resource)

    return resources




  ## get all user resources
  #
  #  @return  all user resources
  #

  def getUserResourceAll (self):

    db = sqlite3.connect(self.dbFilePath, isolation_level=None)
    c  = db.execute(u"SELECT * FROM %s" % (self.userresTableName,))
    r  = c.fetchall()
    db.close()

    resources = []
    i         = resourceId["USERRES_BEGINNING"] + 1

    for resource in r:
      ures = pickle.loads(resource[1].encode("utf-8", "ignore"))
      resources.append({"id": resource[0], "type": ures[0], "str": ures[1], "desc": resource[2]})
      i+=1

    return resources




  ## sendQuery
  #
  #  This method sends query to Homu Main through /proc .
  #

  def sendQuery (self, query):

    f = open(self.queryPath, "a")
    f.write(query + "\n")
    print f
    f.close()




  ## applyRules
  #
  #  This method sends user's rules to Homu Main.
  #

  def applyRules (self):


    # erase rules in Homu Main
    self.sendQuery("reset_rule")

    # user resources
    i       = resourceId["USERRES_BEGINNING"] + 1
    userres = resourceId

    for res in self.getUserResourceAll():

      self.sendQuery("set_userres %d %s %s" % (i, res["type"], res["str"]))
      userres[res["id"]]  = i
      i+=1


    reslist = ""

    # set the default application rule
    for res in self.getDefaultAppRule():

      if not res in userres:
        continue

      reslist += "%s," % (userres[res],)

    reslist = reslist[:-1]
    self.sendQuery("set_defaultapprule %s" % (reslist,))


    # set applications rules
    for app in self.getAppRuleAll():

      reslist = ""

      for res in app["res"]:

        if not res in userres:
          continue

        reslist += "%s," % (userres[res],)

      reslist = reslist[:-1]
      self.sendQuery("set_apprule %s %s" % (app["path"], reslist))




  ## getInfo
  #
  #  This method gets and returns information about Homu Port/Main.
  #

  def getInfo (self):

    f    = open(self.statusPath, "r")
    info = {}

    for line in f.readlines():
      key, val  = line.split(": ")
      info[key] = val.strip()

    f.close()
    return info



  ## setMode
  #
  #  This method changes the user's Homu Main mode.
  #

  def setMode (self, mode):
    self.sendQuery("set_mode %s" % (mode,))




  ## getLog
  #
  #  This method reads and returns unread log messages.
  #

  def getLog (self, readP):

    f = open(self.logPath, "r")
    i = 1
    unread = []

    for line in f.readlines():

      if readP < i:
        words = line.split(" ")

        first = True
        args  = []

        for word in words:
          if first:
            first = False
            continue
          args.append(word.strip())

        unread.append({"type": words[0], "args": args})

      i+=1

    f.close()
    return {"unread": unread, "readP": i-1}


