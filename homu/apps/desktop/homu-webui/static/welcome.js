
function leaveTour (){

  config = {
             "locale": langName
           };

  $.ajax({
           url:      "/set/config",
           type:     "POST",
           data:     config,
           complete: function(){

             $(document.body).fadeOut("slow", function(){
               location.href = "/";
             });
           }
  });
}
