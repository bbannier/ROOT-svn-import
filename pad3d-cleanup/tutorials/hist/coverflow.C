{
   // display the various 2-d drawing options
   //Author: Rene Brun
   
   gROOT->Reset();
   gStyle->SetCanvasPreferGL(1);
   gStyle->SetOptStat(0);
   gStyle->SetPalette(1);
   gStyle->SetCanvasColor(33);
   gStyle->SetFrameFillColor(18);
   TF2 *f2 = new TF2("f2","xygaus + xygaus(5) + xylandau(10)",-4,4,-4,4);
   Double_t params[] = {130,-1.4,1.8,1.5,1, 150,2,0.5,-2,0.5, 3600,-2,0.7,-3,0.3};
   f2.SetParameters(params);
   TH2F h2("h2","xygaus + xygaus(5) + xylandau(10)",20,-4,4,20,-4,4);
   h2.SetFillColor(46);
   h2.FillRandom("f2",40000);
   TPaveLabel pl;

   Int_t ci = 0;

   //basic 2-d options
   Float_t x1=0.67, y1=0.875, x2=0.85, y2=0.95;
   Int_t cancolor = 17;
   TCanvas cccc("c2h","2-d options",10,10,800,600);
   cccc.Divide(4,4);
   cccc.SetFillColor(cancolor);
   cccc.cd(++ci);
   h2.Draw();       pl.DrawPaveLabel(x1,y1,x2,y2,"SCAT","brNDC");
   cccc.cd(++ci);
   h2.Draw("box");  pl.DrawPaveLabel(x1,y1,x2,y2,"BOX","brNDC");
   cccc.cd(++ci);
   h2.Draw("arr");  pl.DrawPaveLabel(x1,y1,x2,y2,"ARR","brNDC");
   cccc.cd(++ci);
   h2.Draw("colz"); pl.DrawPaveLabel(x1,y1,x2,y2,"COLZ","brNDC");
   
   //contour options
   cccc.cd(++ci);
   h2.Draw("contz"); pl.DrawPaveLabel(x1,y1,x2,y2,"CONTZ","brNDC");
   cccc.cd(++ci);
   gPad->SetGrid();
   h2.Draw("cont1"); pl.DrawPaveLabel(x1,y1,x2,y2,"CONT1","brNDC");
   cccc.cd(++ci);
   gPad->SetGrid();
   h2.Draw("cont2"); pl.DrawPaveLabel(x1,y1,x2,y2,"CONT2","brNDC");
   cccc.cd(++ci);
   gPad->SetGrid();
   h2.Draw("cont3"); pl.DrawPaveLabel(x1,y1,x2,y2,"CONT3","brNDC");
   
   //lego options
   cccc.cd(++ci);
   h2.Draw("lego");     pl.DrawPaveLabel(x1,y1,x2,y2,"LEGO","brNDC");
   cccc.cd(++ci);
   h2.Draw("lego1");    pl.DrawPaveLabel(x1,y1,x2,y2,"LEGO1","brNDC");
   cccc.cd(++ci);
   gPad->SetTheta(61); gPad->SetPhi(-82);
   h2.Draw("surf1pol"); pl.DrawPaveLabel(x1,y1,x2+0.05,y2,"SURF1POL","brNDC");
   cccc.cd(++ci);
   gPad->SetTheta(21); gPad->SetPhi(-90);
   h2.Draw("surf1cyl"); pl.DrawPaveLabel(x1,y1,x2+0.05,y2,"SURF1CYL","brNDC");
   
   //surface options
   cccc.cd(++ci);
   h2.Draw("surf1");   pl.DrawPaveLabel(x1,y1,x2,y2,"SURF1","brNDC");
   cccc.cd(++ci);
   h2.Draw("surf2z");  pl.DrawPaveLabel(x1,y1,x2,y2,"SURF2Z","brNDC");
   cccc.cd(++ci);
   h2.Draw("surf3");   pl.DrawPaveLabel(x1,y1,x2,y2,"SURF3","brNDC");
   cccc.cd(++ci);
   h2.Draw("surf4");   pl.DrawPaveLabel(x1,y1,x2,y2,"SURF4","brNDC");

   cccc.Update();
}
