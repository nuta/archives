(function($){

  $.fn.slideshow = function (vars){

    var prevText = "&laquo;";
    var nextText = "&raquo;";

    var canvas   = (vars["canvas"]  != undefined) ? $(vars["canvas"]) :
                                                    this;
    var slides   = (vars["slides"]  != undefined) ? vars["slides"]    :
                                                    canvas.data("slides");
    var current  = (vars["current"] != undefined) ? vars["current"]   :
                                                    1;
    var slideNum   = slides.length;
    var title      = slides[current  - 1]["title"];
    var body       = slides[current  - 1]["body"];
    var bodyStyle  = (slides[current - 1]["bodyStyle"] != undefined) ? slides[current - 1]["bodyStyle"] :
                                                                       "";

    canvas.data("slides", slides);

    canvas.hide();

    canvas.html(

      "<div class='slideshow'>"                           +
      "  <p   id='title'></p>"                            + 
      "  <div id='body' style='" + bodyStyle + "'></div>" +
      "</div>"

    );

    $(".slideshow #title", canvas).html(title);
    $(".slideshow #body",  canvas).html(body);


    // prev / next
    $(".slideshow", canvas).append(

      "<div id='ctrl'>" +
      "  <p><span id='current'></span><span id='slideNum'></span></p>" +
      "  <p>" +
      "    <span id='prev' onClick='$(this).parent().parent().parent().parent().slideshow({current: " + (current - 1) + "})'></span>" +
      "    <span id='next' onClick='$(this).parent().parent().parent().parent().slideshow({current: " + (current + 1) + "})'></span>" +
      "  </p>" +
      "</div>"

    );

    $(".slideshow #current",  canvas).text(current);
    $(".slideshow #slideNum", canvas).text(slideNum);

    if(current-1 <= 0)
      $(".slideshow #prev", canvas).html("");
    else
      $(".slideshow #prev", canvas).html(prevText);

    if(current+1 > slideNum)
      $(".slideshow #next", canvas).html("");
    else
      $(".slideshow #next", canvas).html(nextText);

    canvas.fadeIn("fast");
  }

})(jQuery);
