// Assume we have x, y, and z as RooRealVar* parameters of our model

// Create clones to serve as mean variables
RooRealVar* mu_x = (RooRealVar*)x->clone("mu_x");
RooRealVar* mu_y = (RooRealVar*)y->clone("mu_y");
RooRealVar* mu_z = (RooRealVar*)z->clone("mu_z");

// Fit model to data to get a covariance matrix
// (you can also just construct your own custom covariance matrix)
TMatrixDSym covFit = model->fitTo(*data)->covarianceMatrix();

// Make a PDF to be our proposal density function
RooMultiVarGaussian mvg("mvg", "mvg", RooArgList(*x, *y, *z),   // Careful!
                        RooArgList(*mu_x, *mu_y, *mu_z), covFit);
PdfProposal pf(mvg);

// Optional mappings to center the proposal function around the current point
pf.AddMapping(*mu_x, *x);
pf.AddMapping(*mu_y, *y);
pf.AddMapping(*mu_z, *z);

pf.SetCacheSize(100); // when we must generate proposal points, generate 100
