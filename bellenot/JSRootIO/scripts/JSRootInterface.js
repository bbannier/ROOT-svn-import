// JSRootInterface.js
//
// interface methods for Javascript ROOT Web Page.
//

var gFile;
var obj_list = new Array();
var obj_index;

function loadScript(url, callback) {
   // dynamic script loader using callback
   // (as loading scripts may be asynchronous)
   var script = document.createElement("script")
   script.type = "text/javascript";
   if (script.readyState) { // Internet Explorer specific
      script.onreadystatechange = function() {
         if (script.readyState == "loaded" ||
             script.readyState == "complete") {
            script.onreadystatechange = null;
            callback();
         }
      };
   } else { // Other browsers
      script.onload = function(){
         callback();
      };
   }
   var rnd = Math.floor(Math.random()*80000); 
   script.src = url;//+ "?r=" + rnd;
   document.getElementsByTagName("head")[0].appendChild(script);
};

function displayRootStatus(msg) {
   $("#status").append(msg);
};

function displayListOfKeys(keys) {
   JSROOTPainter.displayListOfKeys(keys, '#status');
};

function displayInflatedBuffer(buffer) {
   var id = buffer['key']['className'] + " : " +
            buffer['key']['name'] + " (" +
            buffer['key']['title'] + ")";
   var entryInfo = "<h5><a>" + id + "</a>&nbsp; </h5>\n<div>";
   // contrive an id for the entry, make it unique
   var randomId = "buffer-id-" + buffer['key']['name'];
   var findElement = $('#report').find('#'+randomId);
   if (findElement.length) {
      return;
   }
   entryInfo += "<h6>Inflated Buffer:</h6><span id='" + randomId + "'></span></div>\n";
   // insert the info for one entry as the last child within the report div
   $("#report").append(entryInfo);
   var content = "<pre style='font-size:x-small'>" +
                 JSIO.htmlEscape(JSIO.formatByteArray(buffer['buffer'])) + "</pre>";
   $("#"+randomId).html(content);
};

function displayInflatedBuffers(buffers) {
   for (var i=0; i<buffers.length; ++i) {
      if (buffers[i]['key']['className'] != "" && buffers[i]['key']['name'] != "")
         displayInflatedBuffer(buffers[i])
   }
};

function displayStreamerInfos(streamerInfo) {
   var findElement = $('#report').find('#treeview');
   if (findElement.length) {
      var ndx = findElement.index() * 0.5;
      ndx = Math.floor(ndx);
//      $('#report').accordion("activate", ndx);
   }
   else {
      var entryInfo = "<h5><a> Streamer Infos </a>&nbsp; </h5><div>\n";
      entryInfo += "<h6>Streamer Infos</h6><span id='treeview' class='dtree'></span></div>\n";
      $("#report").append(entryInfo);
      JSROOTPainter.displayStreamerInfos(streamerInfo, '#treeview');
      $('#report').accordion('destroy');
      $('#report').accordion({collapsible:true, active:false, autoHeight:false});
   }
};

function findObject(obj_name) {
   for (var i in obj_list) {
      if (obj_list[i] == obj_name) {
         var findElement = $('#report').find('#histogram'+i);
         if (findElement.length) {
            var ndx = findElement.index() * 0.5;
            ndx = Math.floor(ndx);
            $('#report').accordion("activate", ndx);
         }
         return true;
      }
   }
   return false;
};

function showObject(obj_name, cycle) {
   gFile.ReadObject(obj_name, cycle);
};

function displayDirectory(directory, cycle, dir_id) {
   $("#status").html("<br/>JSROOTIO.RootFile.js version: " + JSROOTIO.version + "<br/>");
   var url = $("#urlToLoad").val();
   $("#status").append("load: " + url + "<br/>");
   JSROOTPainter.addDirectoryKeys(directory.fKeys, '#status', dir_id);
};

function showDirectory(dir_name, cycle, dir_id) {
   gFile.ReadDirectory(dir_name, cycle, dir_id);
};

function displayObject(obj, cycle, idx) {
   if (!obj['_typename'].match(/\bTH1/) &&
       !obj['_typename'].match(/\bTH2/) &&
       obj['_typename'] != 'JSROOTIO.TGraph') {
      return;
   }
   var entryInfo = "<h5><a> " + obj['fName'] + ";" + cycle + "</a>&nbsp; </h5>\n";
   entryInfo += "<div id='histogram" + idx + "'>\n";
   $("#report").append(entryInfo);
   JSROOTPainter.displayObject(obj, idx);
   $('#report').accordion('destroy');
   $('#report').accordion({collapsible:true, active:false, autoHeight:false});
   var findElement = $('#report').find('#histogram'+idx);
   if (findElement.length) {
      var ndx = findElement.index() * 0.5;
      ndx = Math.floor(ndx);
      $('#report').accordion("activate", ndx);
   }
};

function AssertPrerequisites(andThen) {
   if (typeof JSROOTIO == "undefined") {
      // if JSROOTIO is not defined, then dynamically load the required scripts and open the file
      loadScript('http://ajax.googleapis.com/ajax/libs/jquery/1.7.1/jquery.min.js', function() {
      loadScript('http://ajax.googleapis.com/ajax/libs/jqueryui/1.8.18/jquery-ui.min.js', function() {
      loadScript('scripts/dtree.js', function() {
      loadScript('scripts/rawinflate.js', function() {
      loadScript('scripts/JSIO.core.js', function() {
      loadScript('http://code.highcharts.com/highcharts.js', function() {
      loadScript('http://code.highcharts.com/modules/exporting.js', function() {
      loadScript('scripts/JSRootPainter.js', function() {
      loadScript('scripts/JSRootIOEvolution.js', function() {
         if (andThen) { andThen(); }
         $("#status").html("<br/>JSROOTIO.RootFile.js version: " + JSROOTIO.version + "<br/>");
         var url = $("#urlToLoad").val();
         if (url == "" || url == " ") return;
         $("#status").append("load: " + url + "<br/>");
         $('#report').accordion('destroy');
         $("#report").get(0).innerHTML = '';
         obj_list = [];
         obj_index = 0;
         delete gFile;
         gFile = new JSROOTIO.RootFile(url);
         $('#report').append("</body></html>");
         $('#report').accordion('destroy');
         $('#report').accordion({collapsible:true, active:false, autoHeight:false});
      }) }) }) }) }) }) }) }) });
      return;
   }
}
function ReadFile() {
   AssertPrerequisites();
   // else simply open the file
   $("#status").html("<br/>JSROOTIO.RootFile.js version: " + JSROOTIO.version + "<br/>");
   var url = $("#urlToLoad").val();
   if (url == "" || url == " ") return;
   $("#status").append("load: " + url + "<br/>");
   $('#report').accordion('destroy');
   $("#report").get(0).innerHTML = '';
   obj_list = [];
   obj_index = 0;
   delete gFile;
   gFile = new JSROOTIO.RootFile(url);
   $('#report').append("</body></html>");
   $('#report').accordion('destroy');
   $('#report').accordion({collapsible:true, active:false, autoHeight:false});
};

function ResetUI() {
   $("#status").html("<br/>JSROOTIO.RootFile.js version: " + JSROOTIO.version + "<br/>");
   $('#report').accordion('destroy');
   $('#report').get(0).innerHTML = '';
};

function BuildSimpleGUI() {
  AssertPrerequisites(function DisplayGUI() {
  var myDiv = $('#simpleGUI');
  if (!myDiv) {
    alert("You have to define a div with id='simpleGUI'!");
    return;
  }
  var files = myDiv.attr("files");
  if (!files) {
    alert("div id='simpleGUI' must have a files attribute!");
    return;
  }
  var arrFiles = files.split(';');
  var guiCode = "<div id='main' class='column'>\n"
        +"<h1><font face='Verdana' size='4'>Read a ROOT file with Javascript</font></h1>\n"
        +"<p><b>Select a ROOT file to read, or enter a url (*): </b><br/>\n"
        +'<small><sub>*: Other URLs might not work because of cross site scripting protection, see e.g. <a href="https://developer.mozilla.org/en/http_access_control">http://developer.mozilla.org/en/http_access_control</a> on how to avoid it.</sub></small></p>'
      +'<form name="ex">'
      +'<div style="margin-left:10px;">'
      +'<input type="text" name="state" value="" size="40" id="urlToLoad"/><br/>'
      +'<select name="s" size="1" '
      +'onchange="document.ex.state.value = document.ex.s.options[document.ex.s.selectedIndex].value;document.ex.s.selectedIndex=0;document.ex.s.value=\'\'">'
      +'<option value = " " selected = "selected">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>';
      for (var i=0; i<arrFiles.length; i++) {
        guiCode += '<option value = "' + arrFiles[i] + '">' + arrFiles[i] + '</option>';
      }
      guiCode += '</select>'
         +'</div>'
         +'<input style="padding:2px; margin-left:10px; margin-top:5px;"'
         +' onclick="ReadFile()" type="button" title="Read the Selected File" value="Load"/>'
         +'<input style="padding:2px; margin-left:10px;"'
         +'onclick="ResetUI()" type="button" title="Clear All" value="Reset"/>'
      +'</form>'

      +'<br/>'
      +'<div id="status"></div>'
      +'</div>'

      +'<div id="reportHolder" class="column">'
      +'<div id="report"> </div>'
      +'</div>';
  $('#simpleGUI').append(guiCode);
});
}
