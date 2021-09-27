
function jsfiler_open (it, path){

  $(".jsfiler_dir").css("background",  "#e7e8e8");
  $(".jsfiler_file").css("background", "#e7e8e8");
  $(it).css("background", "#ccffff");

  $("#win_addUserRes_path").val(path);
}




function jsfiler_ls(server, div_list, path){

  $.getJSON(server, {path: path}, function(data, status){

    html = "<ul>";

    for(i=0; i < data.length; i++){

      if(data[i]["type"] == "dir"){

        if(data[i]["name"] == "Parent Directory"){

        html += "<li class=\"jsfiler_dir\" onClick=\"jsfiler_ls(" + "'" + server + "','" + div_list + "','" + data[i]["abspath"] + "')\" style=\"text-align:center;color:#660\">"   +
                data[i]["name"]                                                                                                                        +
                "</li>";
        }else{

        html += "<li class=\"jsfiler_dir\" onClick=\"jsfiler_ls(" + "'" + server + "','" + div_list + "','" + data[i]["abspath"] + "')\">"   +
                data[i]["name"] + "/"                                                                                                        +
                "</li>";
        }

      }else{

        html += "<li class=\"jsfiler_file\" onClick=\"jsfiler_open(this, '" + data[i]["abspath"] + "')\">"   +
                data[i]["name"]                                                                              +
                "</li>";
      }
    }

    html += "</ul>";

    $(div_list).html(html);
    $("#jsfiler_current").text(path);
  });
}
