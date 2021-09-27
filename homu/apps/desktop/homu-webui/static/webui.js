
var firstNotify = true;
var UserResources = [];
var Applications  = [];



function webui_logout (){

  $.ajax({
           url:        "logout",
           type:       "GET",
           beforeSend: function(){

             var hi  = "";
             var now = new Date();

             if((hi == "") && (now.getHours() > 21))
               hi = "Good night.";
             if((hi == "") && (now.getHours() < 12))
               hi = "Have a good day.";
             if((hi == "") && (now.getDay() == 5))
               hi = "Have a nice weekend.";
             if((hi == "") && (now.getHours() > 22))
               hi = "Thank you for your time.";
             if((hi == "") && (now.getMinutes() == 46))
               hi = "Homuhomu!";
             if(hi == "")
               hi = "Bye.";

             $("body").html(
                             "<p style='text-align:center;'>" + hi + "</p>"
                           );
           }
  });
}


function webui_getAppInfoByPath (path){

  for(var i=0; i < Applications.length; i++){

    if(Applications[i]["path"] == path)
      return Applications[i];
  }

  /* not found */
  return {
           path: path,
           name: (path == "///defaultapprule///")? lang["isNotExApp"] : path,
           desc: ""
         };
}




function webui_getResNameById (resid){

 reslist = {
             1: "NET_CONNECT",
             2: "NET_SERVER",
             3: "FS_READ",
             4: "FS_WRITE",
             5: "SYS_CHANGE",
             6: "HW_FULLCTRL",
           };

  return (resid in reslist)? reslist[resid] : "";
}




function webui_getDescByResId (resid){

  /* well-known resoruces (resid is String) */
  if("res_" + resid in lang){

    return lang["res_" + resid];
  }

  key = webui_getResNameById(resid);

  if(key != ""){

    /* well-known resoruces (resid is Integer) */
    return lang["res_" + key];

  }else{

    /* user resources */
    for(var i=0; i < UserResources.length; i++){

      if(UserResources[i]["id"] == resid){
        return ures["desc"];
      }
    }
  }

  return "resid(" + resid + ")";
}




function webui_changeMode (mode){

 $.ajax({
          url:      "set/mode",
          data:     {mode: mode},
          type:     "POST",
          complete: function(){ location.reload() }
  });
}




function webui_applyRules (){

  $.ajax({
           url:      "apply_rules",
           type:     "POST",
           complete: function(){ location.reload() }
  });
}




function webui_deleteUserRes (id){

  $.ajax({
           url:      "delete/user_res",
           type:     "POST",
           data:     { id: id },
           complete: function(){ location.reload() }
  });
}




function webui_deleteExApp (path){

 $.ajax({
          url:      "delete/exApp",
          type:     "POST",
          data:     { path: path },
          complete: function(){ webui_updateExApps() }
  });
}




function webui_addUserRes (){

  drawerbox_closeWindow();

   $.ajax({
            url:   "set/user_res",
            type:  "POST",
            data:  {
                     desc: $("#win_addUserRes_desc").val(),
                     type: $("#win_addUserRes_type input[type=radio]:checked").val(),
                     str:  $("#win_addUserRes_path").val()
                   },

            complete: function(){ location.reload() }
   });
}




function webui_addExApp (){

  drawerbox_closeWindow();
 

  selectedRes = "";

  $(".win_addException_resCheckbox:checked").each(function(){

    selectedRes += $(this).val() + ",";
  });


  $.ajax({
           url:   "set/exApp",
           type:  "POST",
           data:  { path: $("#selected_app").val(), res: selectedRes.slice(0, -1) },
           complete: function(){ webui_updateExApps() }
  });
}




function webui_searchApplication (query){

  if(query == "")
    return;

  webui_do_searchApplication(query);
}




function webui_do_searchApplication(query){

  $("#win_addException > #body > #applist").fadeOut("fast", function(){

    $("#win_addException > #body > #applist").html("");

    found = false;
    apps  = Applications;

    for(var i=0; i < apps.length; i++){

      apptext = apps[i]["name"] + apps[i]["author"] + apps[i]["url"] +
                apps[i]["url"]  + apps[i]["description"];

      if(apptext.toLowerCase().search(query.toLowerCase()) != -1){

        found = true;

        $("#win_addException > #body > #applist").append(
                                                          "<p class='app' onClick=\""+
                                                            "$('#win_addException > #body > #applist > .app').css('background', '#fafafa');" +
                                                            "$(this).css('background', '#ffffcc');" +
                                                            "$('#selected_app').val('" + apps[i]["path"] + "')" +
                                                          "\">" +
                                                          "  <img src='" + apps[i]["icon"] + "'>"  +
                                                          "  <span>"     + apps[i]["name"] + "</span>" +
                                                          "</p>"
                                                          );
      }
    }


    /*  not found  */
    if(!found){

      $("#win_addException > #body > #applist").html("<p>" + lang["search_not_found"] + "</p>");
    }


    /* show the result */
    $("#win_addException > #body > #applist").fadeIn("fast");

  });
}


function webui_updateUserRes (){

  $("#userres > #body > ul").fadeOut("fast", function(){

    $("#userres > #body > ul").html("");

    $.ajax({
             url:     "/userres/all",
             type:     "GET",
             dataType: "json",

             success: function(userres, type){

               UserResources = userres;
               webui_update();

               for(var i=0; i < userres.length; i++){

                 ures = userres[i];

                 drawerbox_prependLi(
                                      $("#userres"),

                                      "/static/icons/blank.png",
                                      ures["desc"],
                                      [
                                        {key: lang["type"],         val: ures["type"]},
                                        {key: lang["user_res_str"], val: ures["str"]}
                                      ],
                                      "<a href='#' class='drawerbox_button1'  onClick='webui_deleteUserRes(\"" + ures["id"] + "\")'>" + lang["delete"] + "</a>"
                                    );
               }

               $("#userres > #body > ul").fadeIn("fast");
             }
    });
  });
}




function webui_updateExApps (){

  $("#applications > #body > ul").fadeOut("fast", function(){

    $("#applications > #body > ul").html("");

    $.ajax({
             url:      "/applications/ex",
             type:     "GET",
             dataType: "json",

             success: function(apps, type){

               if(apps == [])
                 return;

               for(var i=0; i < apps.length; i++){

                 app  = apps[i];

                 div = "<b>" + lang["allowed_res"] + "</b> <ul>";

                 for(var j=0; j < app.res.length; j++){

                   div += "<li>" + webui_getDescByResId(app["res"][j]) + "</li>";
                 }

                 div += "</ul><p style='text-align:right;'><a href='#' class='drawerbox_button1'  onClick='webui_deleteExApp(\"" + app["path"] + "\")'>" + lang["delete"] + "</a></p>";

                 drawerbox_prependLi(
                                      $("#applications"),
                                      app["icon"],
                                      app["name"],
                                      [
                                        {key: lang["description"], val: app["description"]},
                                      ],
                                      div
                                    );
               }

               $("#applications > #body > ul").fadeIn("fast");
             }
    });
  });
}





var logId="";

function webui_update (){

  /*
  *  Status
  */
  $.ajax({
           url:      "/status",
           type:     "GET",
           dataType: "json",

           success: function(data, type){

             $("#current_mode").text(lang["current_mode"].replace("%s", lang["mode_" + data["mode"].toUpperCase()]));
           }
  });



  /*
  *  Notifications
  */

  $.ajax({
           url:      "/notifications/" + logId,
           type:     "GET",
           dataType: "json",

           success: function(data, type){

             if(data["logid"] == logId)
               return;

             log   = data["log"];
             logId = data["logid"];

             /* for not displaying same notifications back to back */
             last  = {type: "", args: []};

             for(var i=0; i < log.length; i++){

               arg = log[i]["args"];

               if(last["type"] == log[i]["type"]){

                 isSame = true;
                 for(var j=0; log[i]["args"][j] != undefined; j++){

                   if(last["args"][j] != log[i]["args"][j]){
                     isSame = false;
                     break;
                   }
                 }

                 if(isSame){
                   continue;
                 }
               }

               last["type"] = log[i]["type"];
               last["args"] = log[i]["args"];

               switch(log[i]["type"]){
                 case "ERROR":
                   attrs = [
                             {key: lang["tech_info"], val: arg[0]}
                           ];
                 break;

                 case "WARNING":
                   attrs = [
                             {key: lang["tech_info"], val: arg[0]}
                           ];
                 break;

                 case "RULE_VIOLATION":
                   appinfo = webui_getAppInfoByPath(arg[0]);
                   attrs = [
                             {key: lang["app"], val: appinfo["name"]},
                             {key: lang["res"],  val: webui_getDescByResId(arg[1])}
                           ];
                 break;

                 case "MODE_CHANGED":
                   attrs = [
                             {key: lang["mode"], val: lang["mode_" + arg[0].toUpperCase()]}
                           ];
                 break;
               }

               drawerbox_prependLi(
                                    $("#notifications"),
                                    "/static/icons/blank.png",
                                    lang["log_" + log[i]["type"]],
                                    attrs,
                                    "<a href='#' onClick='$(this).parent().parent().fadeOut(\"fast\");' class='drawerbox_button1'>"+ lang["hide"] + "</a>",
                                    (!firstNotify)? lang["new_notification"] : undefined
                                  );
             }

             firstNotify = false;
           }
  });
}




function webui_init (){

  webui_updateUserRes();
  webui_updateExApps();

  $.ajax({
           url:      "/applications/all",
           type:     "GET",
           dataType: "json",
           success:  function(apps, type){ Applications = apps; }
  });

  /* get new notifications every 5 secounds */
  setInterval(webui_update, 1000 * 5 /* 5 sec. */);
}
