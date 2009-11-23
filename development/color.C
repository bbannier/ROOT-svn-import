color( int col )
{
   gROOT->GetColor(col)->Print();
   int red   = gROOT->GetColor(col)->GetRed()*256;
   int green = gROOT->GetColor(col)->GetGreen()*256;
   int blue  = gROOT->GetColor(col)->GetBlue()*256;   
   float lum   = gROOT->GetColor(col)->GetLight();

   cout << red << " " << green << " " << blue << endl;
   cout << Form( "#%02x%02x%02x %g", red, green, blue, lum ) << endl;
   
}
