/** @file CbmStsClusterFinder_test
 ** @brief Unit test of CbmStsAddress
 ** This macro tests the functionality of the CbmStsClusterFinder
 ** and CbmStsClusterAnalysis
 **
 ** @author V. Friese <v.friese@gsi.de>
 */


#include <iostream>

using namespace std;



Int_t CbmStsClusterFinder_test() {

   // ----- Timer
   TStopwatch timer;
   timer.Start();


   // --- Create a module
   Int_t    nChannels    = 2048;
   Double_t dynRange     = 40960.;
   Double_t threshold    = 0.;
   Int_t    nAdc         = 4096;
   Double_t tResol       = 5.;
   CbmStsModule module("TestModule", "Module");
   module.SetParameters(nChannels, dynRange, threshold, nAdc, tResol);


   // --- Create some input digis and their array
   TClonesArray digiArray("CbmStsDigi", 10);
   Int_t    channel = 0;
   Double_t charge  = 0.;
   ULong64_t time   = 0;
   UInt_t address   = 0;
   UShort_t adc     = 0;
   Int_t  nDigis    = 0;
   CbmStsDigi* digi = NULL;
   CbmMatch* match  = NULL;

   // --- 1-strip cluster
   channel = 5;
   charge  = 24005.;
   address = CbmStsAddress::SetElementId(module.GetAddress(), kStsChannel, channel);
   adc = module.ChargeToAdc(charge);
   nDigis = digiArray.GetEntriesFast();
   digi = new (digiArray[nDigis]) CbmStsDigi(address, time, adc);
   match = new CbmMatch();
   match->AddLink(charge, 1);   // weight, index of MC
   digi->SetMatch(match);


   // --- 2-strip cluster
   channel = 100;
   charge  = 8005.;
   address = CbmStsAddress::SetElementId(module.GetAddress(), kStsChannel, channel);
   CbmStsAddress::SetElementId(address, kStsChannel, channel);
   adc = module.ChargeToAdc(charge);
   nDigis = digiArray.GetEntriesFast();
   digi = new (digiArray[nDigis]) CbmStsDigi(address, time, adc);
   match = new CbmMatch();
   match->AddLink(charge, 2);   // weight, index of MC
   digi->SetMatch(match);

   channel = 101;
   charge  = 16005.;
   address = module.GetAddress();
   address = CbmStsAddress::SetElementId(module.GetAddress(), kStsChannel, channel);
   adc = module.ChargeToAdc(charge);
   nDigis = digiArray.GetEntriesFast();
   digi = new (digiArray[nDigis]) CbmStsDigi(address, time, adc);
   match = new CbmMatch();
   match->AddLink(charge, 2);   // weight, index of MC
   digi->SetMatch(match);


   // --- 4-strip cluster
   channel = 1400;
   charge = 6005.;
   address = CbmStsAddress::SetElementId(module.GetAddress(), kStsChannel, channel);
   adc = module.ChargeToAdc(charge);
   nDigis = digiArray.GetEntriesFast();
   digi = new (digiArray[nDigis]) CbmStsDigi(address, time, adc);
   match = new CbmMatch();
   match->AddLink(charge, 3);   // weight, index of MC
   digi->SetMatch(match);

   channel = 1401;
   charge = 8005.;
   address = CbmStsAddress::SetElementId(module.GetAddress(), kStsChannel, channel);
   adc = module.ChargeToAdc(charge);
   nDigis = digiArray.GetEntriesFast();
   digi = new (digiArray[nDigis]) CbmStsDigi(address, time, adc);
   match = new CbmMatch();
   match->AddLink(charge, 3);   // weight, index of MC
   digi->SetMatch(match);

   channel = 1402;
   charge = 8005.;
   address = CbmStsAddress::SetElementId(module.GetAddress(), kStsChannel, channel);
   adc = module.ChargeToAdc(charge);
   nDigis = digiArray.GetEntriesFast();
   digi = new (digiArray[nDigis]) CbmStsDigi(address, time, adc);
   match = new CbmMatch();
   match->AddLink(charge, 3);   // weight, index of MC
   digi->SetMatch(match);

   channel = 1403;
   charge = 2005.;
   address = CbmStsAddress::SetElementId(module.GetAddress(), kStsChannel, channel);
   adc = module.ChargeToAdc(charge);
   nDigis = digiArray.GetEntriesFast();
   digi = new (digiArray[nDigis]) CbmStsDigi(address, time, adc);
   match = new CbmMatch();
   match->AddLink(charge, 3);   // weight, index of MC
   digi->SetMatch(match);


   // --- Add the digis to the module
   nDigis = digiArray.GetEntriesFast();
   cout << "Digis in array: " << nDigis << endl;
   for (Int_t iDigi = 0; iDigi < nDigis; iDigi++) {
  	 CbmStsDigi* digi = (CbmStsDigi*) digiArray.At(iDigi);
  	 module.AddDigi(digi, iDigi);
   }

   // --- Instantiate cluster finder and find clusters
   TClonesArray clusterArray("CbmStsCluster", 10);
   CbmStsClusterFinderGap finder(&clusterArray);
   finder.FindClusters(&module);
   cout << "Found " << clusterArray.GetEntriesFast() << " clusters." << endl;

   // --- Cluster analysis
   CbmStsClusterAnalysis ana;
   for (Int_t iCluster = 0; iCluster < clusterArray.GetEntriesFast(); iCluster++) {
  	 CbmStsCluster* cluster = (CbmStsCluster*) clusterArray.At(iCluster);
  	 ana.Analyze(cluster, &module, &digiArray);
  	 cout << cluster->ToString() << endl;
   }


   /* The results should be:
    * Cluster 1: 1 digi, charge 24005, position 5, error 0.204124
    * Cluster 2: 2 digi, charge 24010, position 100.667, error 0.0076336
    * Cluster 3: 4 digi, charge 24020, position 1401.25, error 0.
    */

};



