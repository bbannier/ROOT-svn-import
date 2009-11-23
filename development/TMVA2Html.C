
void TMVA2Html() {
   THtml html;
   html.SetSourceDir("../src");
   html.SetIncludePath("../include");
   html.SetOutputDir("../html");

   html.MakeAll();

}
