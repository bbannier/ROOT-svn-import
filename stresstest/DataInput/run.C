{
   gInterpreter->LoadMacro( "createComparisonData.C");
   CreateDataForInputTests(500,1000,"TMVAInputDataChain1.root");
   CreateDataForInputTests(5000,150000,"TMVAInputDataChain2.root");
   CreateDataForInputTests();
}
