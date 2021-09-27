
function drawerbox_init (){


  /*
  *  when a <li> is clicked
  */
  $(".drawerbox > #body > ul > li").click(function(){

    $("> div", this).slideToggle("fast");

    if($("> div", this).hasClass("drawerbox_hidden")){

      $(this).css("background-color", "#eeeeee");
      $("> div", this).removeClass("drawerbox_hidden");

    }else{

      $(this).css("background-color", "#dfdfdf");
      $("> div", this).addClass("drawerbox_hidden");
    }
  });


  /*
  *  show a searchbox
  */
  $(".drawerbox_searchbox").each(function(){

    $("#title", this).after(
                              "<div style='text-align:center; margin-bottom: 1.2em;'>"   +
                              "  <input type='search' class='drawerbox_emd_searchbox'>"  +
                              "</div>"
                           );
  });


  /*
  *  search
  */
  $(".drawerbox_emd_searchbox").each(function(){

    $(this).keydown(function(){

      query = $(this).val().toLowerCase();

      $("ul > li", $(this).parent().parent()).each(function(){

        if(query == ""){

          $(this).show();

        }else{

          if($("b, dl", $(this)).text().toLowerCase().search(query) == -1){
            $(this).hide();
          }else{
            $(this).show();
          }
        }
      });
    });
  });
}




/*
*
*  close all windows
*
*/

function drawerbox_closeWindow (){

  $("#drawerbox_blackCurtain").fadeOut("fast");
  $(".drawerbox_window").fadeOut("fast");
}




/*
*
*  open a window
*
*/

function drawerbox_openWindow (win){


  if($("#drawerbox_blackCurtain").length == 0){

    $(document.body).prepend("<div id='drawerbox_blackCurtain'></div>");

    $("#drawerbox_blackCurtain").css("display",    "none");
    $("#drawerbox_blackCurtain").css("position",   "absolute");
    $("#drawerbox_blackCurtain").css("width",      "100%");
    $("#drawerbox_blackCurtain").css("height",     $(window).attr("innerHeight"));
    $("#drawerbox_blackCurtain").css("top",        "0px");
    $("#drawerbox_blackCurtain").css("left",       "0px");
    $("#drawerbox_blackCurtain").css("background", "#000000");
    $("#drawerbox_blackCurtain").css("opacity",    "0.5");
    $("#drawerbox_blackCurtain").css("z-index",    "128");
  }


  if($("> #closeButton", win).length == 0){

    win.prepend(
                 "<div style='text-align:right;' id='closeButton'>"                                          + 
                 "  <a href='#' class='drawerbox_button1' onClick='drawerbox_closeWindow()'>" + lang["close"] + "</a></div>" +
                 "</div>"
               );
  }

  $("#drawerbox_blackCurtain").fadeIn();
  win.fadeIn();
}




/*
*
*  prepend a new <li>
*
*/

function drawerbox_prependLi (dbox, icon, name, attrs, div, msg){

  liHTML =
           "<li style='display: none'>" +
           "  <b>" + name + "</b>"      +
           "  <img src='"+ icon  +"'>"  +
           "  <table border='0'>"
           ;

  for(var i=0; i < attrs.length; i++){

    attr = attrs[i];

    liHTML +=
              "<tr>"                            +
              "  <td>" + attr["key"] + "</td>"  +
              "  <td>" + attr["val"] + "</td>"  +
              "</tr>"
              ;
  }


  liHTML +=
            "</table>"         +
            "<div>"            +
              div              +
            "</div>"           +
            "</li>"
            ;


  $("#body > ul", dbox).prepend(liHTML);


  // show a notification message
  if(msg != undefined){

    $("#notification", dbox).html(msg);
    $("#notification", dbox).slideDown("normal");


    $("#notification", dbox).click(function(){
      $("#notification",   dbox).hide();
      $("#body > ul > li", dbox).fadeIn();
    });

  // show the <li> immediately
  }else{

    $("#body > ul > li:first", dbox).fadeIn("fast");
  }


  $("#body > ul > li:first", dbox).click(function(){

    $("div", this).slideToggle("fast");

    if($("div", this).hasClass("drawerbox_hidden")){

      $(this).css("background-color", "#eeeeee");
      $("div", this).removeClass("drawerbox_hidden");

    }else{

      $(this).css("background-color", "#dfdfdf");
      $("div", this).addClass("drawerbox_hidden");
    }
  });
}

