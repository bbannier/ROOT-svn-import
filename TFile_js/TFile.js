// See https://developer.mozilla.org/En/Using_XMLHttpRequest

function load_binary_resource_a(url) {  
   var req = new XMLHttpRequest();  
   req.open('GET', url, true);
   //req.setRequestHeader('Range', 'bytes=0-3')
   //XHR binary charset opt by Marcus Granado 2006 [http://mgran.blogspot.com]
   req.overrideMimeType('text/plain; charset=x-user-defined');
   req.setRequestHeader("content-type", "text/plain; charset=x-user-defined");
   req.onreadystatechange = function (aEvt) {  
      if (req.readyState == 4) {  
         document.getElementById('log').value = this.status;
         if(this.status == 200)
            showfile(req.responseText);  
         else
            alert("Error loading page\n");  
      }
   };
   req.send(null);
}  

function load_binary_resource(url) {  
   var req = new XMLHttpRequest();  
   req.open('GET', url, false);
   //req.setRequestHeader('Range', 'bytes=0-3')
   //XHR binary charset opt by Marcus Granado 2006 [http://mgran.blogspot.com]
   req.overrideMimeType('text/plain; charset=x-user-defined');
   req.setRequestHeader("content-type", "text/plain; charset=x-user-defined");
   req.send(null);
   if (req.status != 200) return;

   showfile(req.responseText);
}  

function log(s, i) {
   var l = document.getElementById('log');
   if (!i) i = '';
   for (var e in s) {
      if (s[e] != null && typeof(s[e]) == 'object') {
         l.value += i + e + ':\n';
         log(s[e], i + '  ');
      } else {
         l.value += i + e + ' = ' + s[e] + '\n';
      }
   }
}

function ntoh2(b, o) {
   var n = (b.charCodeAt(o) & 0xff) << 8;
   n += (b.charCodeAt(o+1) & 0xff);
   return n;
}

function ntoh4(b, o) {
   var n = (b.charCodeAt(o) & 0xff) << 24;
   n += (b.charCodeAt(o+1) & 0xff) << 16;
   n += (b.charCodeAt(o+2) & 0xff) << 8;
   n += (b.charCodeAt(o+3) & 0xff);
   return n;
}

function ntoh8(b, o) {
   var n = (b.charCodeAt(o) & 0xff) << 56;
   n += (b.charCodeAt(o+1) & 0xff) << 48;
   n += (b.charCodeAt(o+2) & 0xff) << 40;
   n += (b.charCodeAt(o+3) & 0xff) << 32;
   n += (b.charCodeAt(o+4) & 0xff) << 24;
   n += (b.charCodeAt(o+5) & 0xff) << 16;
   n += (b.charCodeAt(o+6) & 0xff) << 8;
   n += (b.charCodeAt(o+7) & 0xff);
   return n;
}

function readheader(str) {
   if (str.substring(0, 4) != "root") {
      alert("NOT A ROOT FILE!");
      return null;
   }
   var header = {};
   header['version'] = ntoh4(str, 4);
   var largeFile = header['version'] >= 1000000;
   var ntohoff = largeFile ? ntoh8 : ntoh4;
   header['begin'] = ntoh4(str, 8);
   header['end'] = ntohoff(str, 12);
   header['units'] = str.charCodeAt(largeFile ? 40 : 32) & 0xff;
   header['seekInfo'] = ntohoff(str, largeFile ? 45 : 37);
   header['nbytesInfo'] = ntohoff(str, largeFile ? 53 : 41);
   if (!header['seekInfo'] && !header['nbytesInfo']) {
      // empty file
      return null;
   }
   if (header['nbytesInfo'] != 3062) {
      alert("File too new: this version of TStreamerInfo still needs to be implemented");
      return null;
   }
   log(header);
   return header;
}

function readTString(str, off) {
   var len = str.charCodeAt(off) & 0xff;
   off++;
   if (len == 255) {
      // large string
      len = ntoh4(str, off);
      off += 4;
   }
   return {
      'off' : off + len,
      'str' : str.substring(off, off + len)
   };
}

function readkey(str, o) {
   var key = {};
   key['offset'] = o;
   var nbytes = ntoh4(str, o);
   key['nbytes'] = nbytes;
   var largeKey = o + nbytes > 2 * 1024 * 1024 * 1024 /*2G*/;
   var ntohoff = largeKey ? ntoh8 : ntoh4;
   key['objLen'] = ntoh4(str, o + 6);
   var datime = ntoh4(str, o + 10);
   key['datime'] = {
      year : (datime >>> 26) + 1995,
      month : (datime << 6) >>> 28,
      day : (datime << 10) >>> 27,
      hour : (datime << 15) >>> 27,
      min : (datime << 20) >>> 26,
      sec : (datime << 26) >>> 26
   };
   key['keyLen'] = ntoh2(str, o + 14);
   key['cycle'] = ntoh2(str, o + 16);
   // skip seekKey, seekPdir
   o += largeKey ? 34 : 26;
   var so = readTString(str, o);
   key['className'] = so['str'];
   o = so['off'];
   so = readTString(str, o);
   key['name'] = so['str'];
   o = so['off'];
   so = readTString(str, o);
   key['title'] = so['str'];
   o = so['off'];
   key['dataoffset'] = o;
   return key;
}

function interpretStreamerInfo(str, key) {
   // TList of TStreamerInfo
   var o = key['dataoffset'];
   var data = str.substring(o, key['offset'] + key['nbytes']);
   //var unzipdata = (new JXG.Util.Unzip(data)).unzip();
   var unzipdata = RawDeflate.deflate(data);
   log({ 'lendata' : unzipdata.length}, '  ');
   /*
   var version = ntoh2(str, o); o += 2;
   if (version > 3) {
      o += 8; // TObject's uniqueID, bits
      var so = readTString(str, o); // name
      o = so['off'];
      var so = readTString(str, o); // title
      o = so['off'];
      var nobjects = ntoh4(str, o); o += 4;
      for (var i = 0; i < nobjects; ++i) {
         var bcnt = ntoh4(str, o); o += 4;
         so = readTString(str, o);
         var readOption = so['str'];
         o = so['off']
         var tag = 0;
         var clVersion = 0;
         if (!(bcnt & 0x40000000 /*kByteCountMask* /)
             || bcnt == 0xFFFFFFFF /*kNewClassTag* /) {
            tag = bcnt;
            bcnt = 0;
         } else {
            clVersion = 1;
            tag = ntoh4(str, o); o+= 4;
         }
         if (!(tag & /*kClassMask* /)) {
            
         }
      }
   }
   */
}

function showfile(str) {
   var header = readheader(str);
   if (header == null) return;
   var key = null;
   var offset = header['begin'];
   var end = header['end'];
   var streamerInfo = {};
   do {
      key = readkey(str, offset);
      log({'KEY':''});
      log(key, '  ');
      if (offset == header['seekInfo']) {
         streamerInfo = interpretStreamerInfo(str, key);
      }
      offset += key['nbytes'];
   } while (offset < end);
}

function loadfile() {
   document.getElementById('log').value = '';
   load_binary_resource('http://127.0.0.1/TFile_JS/f.root');
}
