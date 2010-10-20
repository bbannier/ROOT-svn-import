function showTOC() {
   var par=location.href.split("?")[1];
   var session=par.split(".")[0];
   var part=par.split(".")[1];
   for (var i=0; i<3; i++) {
      var tocId='toc' + (i + 1);
      var tocLi=document.getElementById(tocId);
      if (session == i + 1) {
         if (part) {
            var children=tocLi.childNodes;
            var lc=children.length;
            var cli = 1;
            for (var c = 0; c < lc; c++) {
               var child=children[c];
               if (child.tagName && child.tagName.toLowerCase() == 'li') {
                  if (part == cli) {
                     child.className="stocCurrent";
                  }
                  cli++;
               }
            }
         }
      } else {
         while (tocLi.childNodes.length) {
            tocLi.removeChild(tocLi.childNodes[0]);
         }
      }
   }
}

var lvl=0;
function help() {
   lvl++;
   var h = document.getElementById('help' + lvl);
   var btnhelp = document.getElementById('btnhelp');
   if (!h) {
      h = document.getElementById('helps');
      btnhelp.className = 'disabled';
      /*btnhelp.href = '';*/
   } else {
      if (document.getElementById('help' + (lvl + 1))) {
         btnhelp.innerHTML = '<span><img src="../img/help.png" alt="" />More Help</span>';
      } else {
         btnhelp.innerHTML = '<span><img src="../img/help.png" alt="" />Solution</span>';
      }
   }
   h.className='helpVisible';
   return false;
}
