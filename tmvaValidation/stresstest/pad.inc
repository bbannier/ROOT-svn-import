<?php

function pad($day) {

   $dow[0]='Sun';
   $dow[1]='Mon';
   $dow[2]='Tue';
   $dow[3]='Wed';
   $dow[4]='Thu';
   $dow[5]='Fri';
   $dow[6]='Sat';

   echo '    <div class="Page">';
   echo '      <div class="Pad">';
   echo '        <table border="0" cellpadding="0" cellspacing="0" height="70" width="100%">';
   echo '            <tr>';
   echo '              <td colspan="3" align="center" bgcolor="#D0D0D0"><a href="log_'.$dow[$day].'.html">Log File</a></td>';
   echo '            </tr>';
   if(file_exists("rt".$dow[$day].".html") and file_exists("lt".$dow[$day].".html")) {
      echo '            <tr>';
      echo '              <td width="200" valign="top">';
      echo '                <table bgcolor="\#5974BB" width="100%">';
      echo '                  <!--left table-->';
      
      if(file_exists("lt".$dow[$day].".html")) {
         require_once("lt".$dow[$day].".html");
      }
      echo '                </table>';
      echo '              </td>';
      echo '              <td width="20"></td>';
      echo '              <td>';
      echo '                <!--right table-->';
      echo '                <table width="100%">';
      
      if(file_exists("rt".$dow[$day].".html")) {
         require_once("rt".$dow[$day].".html");
      } else {
         echo '                  <td><a href="log_'.$dow[$day].'">Log File</a></td>';
      }
      echo '                </table>';
      echo '              </td>';
      echo '              <td width="20"></td>';
      echo '            </tr>';      
   }
   echo '        </table>';
   echo '      </div>';
   echo '    </div> ';

}


?>