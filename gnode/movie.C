// mencoder mf://pepe*.jpg -mf fps=15 -ovc lavc -lavcopts vcodec=mpeg4 -o xx.avi


void movie()
{
  Double_t  g_rotate_speed = 1;
  Double_t  g_rotate_theta = 0; 
  Int_t npic=0;
  for (Float_t i=0; i<40; i += 0.1)
  {
    file_list->how_much_space_circle(i);
    
   static Double_t hRotateStep = 0.005;
   static Double_t vRotateStep = 0.025;


   g_rotate_theta += hRotateStep * g_rotate_speed;
   if (g_rotate_theta >= 0.8 || g_rotate_theta <= -0.8) 
   {
     hRotateStep = -hRotateStep;
   }

   TGLViewer *v   = gEve->GetDefaultGLViewer();
   TGLCamera &cam = v->CurrentCamera();
   cam.RotateRad(hRotateStep * g_rotate_speed, vRotateStep * g_rotate_speed);

    gEve->Redraw3D();
    printf("Gooo %f\n", i);
    gSystem->ProcessEvents();
    gEve->GetDefaultGLViewer()->SavePicture(Form("pepe-%04d.jpg", npic++));
  }
}
void auto_rotate_camera()
{
   static Double_t hRotateStep = 0.005;
   static Double_t vRotateStep = 0.025;

   g_rotate_theta += hRotateStep * g_rotate_speed;
   if (g_rotate_theta >= 0.8 || g_rotate_theta <= -0.8) 
   {
     hRotateStep = -hRotateStep;
   }

   TGLViewer *v   = gEve->GetDefaultGLViewer();
   TGLCamera &cam = v->CurrentCamera();
   cam.RotateRad(hRotateStep * g_rotate_speed, vRotateStep * g_rotate_speed);
   v->RequestDraw(TGLRnrCtx::kLODHigh);
}