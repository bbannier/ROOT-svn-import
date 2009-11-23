<!-- -*- mode: HTML -*-  -->
<html>
<head>

<LINK href="tmva_nightly.css" rel="stylesheet" type="text/css"/>

<script type="text/javascript" src="tabView.js"></script>

</head>

  <body style="background-color: #597485;">
    <?php
    require_once("pad.inc");
    ?>

    <center>
      <table border="0" cellpadding="0" cellspacing="0" height="70" width="100%"> 
        <tr height="10"></tr>
        <tr valign=middle align="center">
          <td valign="middle"><img align="left" src="tmva_logo.gif" width="300"/></td>
          <td align="left"><font size="+3" color="orange">Nightly Functionality and Performance Tests</font></td>
        </tr>
        <tr height="10"></tr> <tr><td colspan="2">For all classifiers the
        Kolmogorov-Smirnov distance between the MVA outputs for
        training and testing is printed. This is done separately for
        signal (blue) and background (red). </td></tr>
        <tr>
          <td colspan="2">
            <div class="TabView" id="TabView">
              <div class="Tabs" style="width: 100%;">
                <a>&nbsp; Sunday &nbsp;&nbsp;</a>
                <a>&nbsp; Monday &nbsp;&nbsp;</a>
                <a>&nbsp; Tuesday &nbsp;&nbsp;</a>
                <a>&nbsp; Wednesday &nbsp;&nbsp;</a>
                <a>&nbsp; Thursday  &nbsp;&nbsp;</a>
                <a>&nbsp; Friday &nbsp;&nbsp;</a>
                <a>&nbsp; Saturday &nbsp;&nbsp;</a>
              </div>

              <div class="Pages" style="width: 100%; height: 650; text-align: left;">
                <?php
                for ($var=0; $var<7; $var++) {
                pad($var);
                }
                ?>
              </div>
          </td>
        </tr>
      </table>


    </center>
    <script type="text/javascript">
      var now = new Date();
      tabview_switch('TabView',now.getDay()+1);
    </script>

  </body>
</html>
