/** @file CbmStsAddress_test
 ** @brief Unit test of CbmStsAddress
 ** This macro tests the functionality of the CbmStsAddress class.
 ** i.e. the encoding and decoding of the STS address bit field.
 **
 ** @author V. Friese <v.friese@gsi.de>
 */


#include <iostream>

using namespace std;



Int_t CbmStsAddress_test(Int_t nTests = 10000) {

   // =====   Init   ========================================================
   // ----- Timer
   TStopwatch timer;
   timer.Start();

   cout << "==========================" << endl;
   cout << "Unit test of CbmStsAddress" << endl;
   cout << "==========================" << endl;

   // ---- Get and check number of STS levels
   Int_t nLevels = CbmStsAddress::GetNofLevels();
   if ( nLevels > 10 ) {
     cout << "Number of STS levels ( " << nLevels
          << " ) is larger than 10." << endl;
     cout << "Please adjust size of static arrays in this macro." << endl;
     return 1;
   }

   // -----  Some variables
   Bool_t testStatus = kTRUE;
   Int_t bitField[10];
   TStopwatch watch1;
   TStopwatch watch2;
   Int_t pass = 0;
   Int_t fail = 0;
   Int_t elementId[10], newElementId[10];
   UInt_t address, newAddress;


   // -----   Get address bit field
   for (Int_t level = 0; level < nLevels; level++)
     bitField[level] = CbmStsAddress::GetNofBits(level);
   cout << endl;
   CbmStsAddress::Print();
   // =======================================================================



   // =======================================================================
   // Test 1:  elementId -> address, compare GetAddress methods
   // =======================================================================
   cout << endl << endl;
   cout << "Test 1: elementId -> address, number of tests "
        << nTests << endl;
   watch1.Reset();
   watch2.Reset();
   pass = 0;
   fail = 0;
   watch1.Start();
   for (Int_t iTest = 0; iTest < nTests; iTest++) {

     // ----- Generate random element Ids
     for (Int_t level = 1; level < nLevels; level++) {
       Int_t range = ( 1 << bitField[level] ) - 1;
       elementId[level] = gRandom->Integer(range + 1);
     }

     watch2.Start(kFALSE);
     // ----- Get address
     address = CbmStsAddress::GetAddress(elementId[1],
                                         elementId[2],
                                         elementId[3],
                                         elementId[4],
                                         elementId[5],
                                         elementId[6],
                                         elementId[7]);

     // ----- Get address with id array
     newAddress = CbmStsAddress::GetAddress(elementId);
     watch2.Stop();

     // ---- Compare both addresses
     cout << "\rTest " << setw(6) << iTest+1
               << ", address 1 " << setw(10) << address
               << ", address 2 " << setw(10) << newAddress << std::flush;
     if ( newAddress == address ) pass++;
     else                         fail++;

   }  // test loop

   watch1.Stop();
   cout << endl;
   cout << "Tests passed: " << pass << ", failed " << fail << endl;
   cout << "Total time per test: CPU "
        << 1.e6 * watch1.CpuTime()/Double_t(nTests) << " mus, Real "
        << 1.e6 * watch1.RealTime()/Double_t(nTests) << " mus" << endl;
   cout << "Core  time per test: CPU "
        << 1.e6 * watch2.CpuTime()/Double_t(nTests) << " mus, Real "
        << 1.e6 * watch2.RealTime()/Double_t(nTests) << " mus" << endl;
   if ( fail ) testStatus = kFALSE;
   // =======================================================================



   // =======================================================================
   // Test 2:  elementId -> address -> elementId
   // =======================================================================
   cout << endl << endl;
   cout << "Test 2: elementId -> address -> elementId, number of tests "
        << nTests << endl;
   watch1.Reset();
   watch2.Reset();
   pass = 0;
   fail = 0;
   watch1.Start();
   for (Int_t iTest = 0; iTest < nTests; iTest++) {

     // ----- Generate random element Ids
     for (Int_t level = 1; level < nLevels; level++) {
       Int_t range = ( 1 << bitField[level] ) - 1;
       elementId[level] = gRandom->Integer(range + 1);
     }

     // ----- Get address
     watch2.Start(kFALSE);
     address = CbmStsAddress::GetAddress(elementId[1],
                                         elementId[2],
                                         elementId[3],
                                         elementId[4],
                                         elementId[5],
                                         elementId[6],
                                         elementId[7]);

     // ----- Get element Ids from address
     for (Int_t level = 0; level < nLevels; level++)
       newElementId[level] = CbmStsAddress::GetElementId(address, level);
     watch2.Stop();

     // ----- Compare element Ids
     Bool_t good = kTRUE;
     for (Int_t level = 1; level < nLevels; level++)
       if ( newElementId[level] != elementId[level] ) {
         cout << "Failure: level " << level << ", generated Id "
              << elementId[level] << ", converted Id "
              << newElementId[level] << endl;
         good = kFALSE;
       }
     if ( good ) pass++;
     else        fail++;
     cout << "\rTest " << setw(6) << iTest + 1 << ", old Ids: ";
     for (Int_t level = 1; level < nLevels; level++)
       cout << elementId[level] << " ";
     cout << ", new Ids: ";
     for (Int_t level = 1; level < nLevels; level++)
       cout << newElementId[level] << " ";
     cout << "                  " << flush;

   } // test loop
   cout << endl;
   watch1.Stop();

   cout << "Tests passed: " << pass << ", failed " << fail << endl;
   cout << "Total time per test: CPU "
        << 1.e6 * watch1.CpuTime()/Double_t(nTests) << " mus, Real "
        << 1.e6 * watch1.RealTime()/Double_t(nTests) << " mus" << endl;
   cout << "Core  time per test: CPU "
        << 1.e6 * watch2.CpuTime()/Double_t(nTests) << " mus, Real "
        << 1.e6 * watch2.RealTime()/Double_t(nTests) << " mus" << endl;
   if ( fail ) testStatus = kFALSE;
   // =======================================================================




   // =======================================================================
   // Test 3:  address-> elementId -> address
   // =======================================================================
   cout << endl << endl;
   cout << "Test 3: address-> elementId -> address, number of tests "
        << nTests << endl;
   watch1.Reset();
   watch2.Reset();
   pass = 0;
   fail = 0;
   watch1.Start();
   for (Int_t iTest = 0; iTest < nTests; iTest++) {

     // ---- Generate a random STS address
     Int_t nBits = 32 - bitField[0];
     address = ( gRandom->Integer( 1 << nBits ) << bitField[0] ) | 2;

     // ---- Get element Ids from address
     watch2.Start(kFALSE);
     for (Int_t level = 0; level < nLevels; level++)
       elementId[level] = CbmStsAddress::GetElementId(address, level);

     // ---- Generate new address from element Ids
     newAddress = CbmStsAddress::GetAddress(elementId[1],
                                            elementId[2],
                                            elementId[3],
                                            elementId[4],
                                            elementId[5],
                                            elementId[6],
                                            elementId[7]);
     watch2.Stop();

     // ---- Compare old and new addresses
     cout << "\rTest " << setw(6) << iTest+1
               << ", old address " << setw(10) << address
               << ", new address " << setw(10) << newAddress << std::flush;
     if ( newAddress == address ) pass++;
     else                         fail++;

   }  // test loop

   watch1.Stop();
   cout << endl;
   cout << "Tests passed: " << pass << ", failed " << fail << endl;
   cout << "Total time per test: CPU "
        << 1.e6 * watch1.CpuTime()/Double_t(nTests) << " mus, Real "
        << 1.e6 * watch1.RealTime()/Double_t(nTests) << " mus" << endl;
   cout << "Core  time per test: CPU "
        << 1.e6 * watch2.CpuTime()/Double_t(nTests) << " mus, Real "
        << 1.e6 * watch2.RealTime()/Double_t(nTests) << " mus" << endl;
   if ( fail ) testStatus = kFALSE;
   // =======================================================================



   // =======================================================================
   // Test 4:  elementId -> address -> SetElementId
   // =======================================================================
   cout << endl << endl;
   cout << "Test 4: elementId -> address -> SetElementId, number of tests "
        << nTests << endl;
   watch1.Reset();
   watch2.Reset();
   pass = 0;
   fail = 0;
   watch1.Start();
   for (Int_t iTest = 0; iTest < nTests; iTest++) {

     // ----- Generate random element Ids
     for (Int_t level = 1; level < nLevels; level++) {
       Int_t range = ( 1 << bitField[level] ) - 1;
       elementId[level] = gRandom->Integer(range + 1);
     }

     // -----  Generate random new Id
     Int_t changeLevel = gRandom->Integer(nLevels-1) + 1;
     Int_t range = ( 1 << bitField[changeLevel] ) -1;
     Int_t newId = gRandom->Integer(range + 1);


     // ----- Get address
     watch2.Start(kFALSE);
     address = CbmStsAddress::GetAddress(elementId[1],
                                         elementId[2],
                                         elementId[3],
                                         elementId[4],
                                         elementId[5],
                                         elementId[6],
                                         elementId[7]);

     // ----- Change id of given level
     newAddress = CbmStsAddress::SetElementId(address, changeLevel, newId);

     // ----- Get element Ids from address
     for (Int_t level = 0; level < nLevels; level++)
       newElementId[level] = CbmStsAddress::GetElementId(newAddress, level);
     watch2.Stop();

     // ----- Compare element Ids
     Bool_t good = kTRUE;
     for (Int_t level = 1; level < nLevels; level++) {
       Int_t testId = 0;
       if ( level == changeLevel ) testId = newId;
       else testId = elementId[level];
       if ( newElementId[level] != testId ) {
         cout << "Failure: level " << level << ", generated Id "
              << testId << ", converted Id "
              << newElementId[level] << endl;
         good = kFALSE;
       }
     }
     if ( good ) pass++;
     else        fail++;
     cout << "\rTest " << setw(6) << iTest + 1 << ", old Ids: ";
     for (Int_t level = 1; level < nLevels; level++)
       cout << elementId[level] << " ";
     cout << ", change level " << changeLevel << " to id " << newId;
     cout << ", new Ids: ";
     for (Int_t level = 1; level < nLevels; level++)
       cout << newElementId[level] << " ";
     cout << "                  " << flush;

   } // test loop
   cout << endl;
   watch1.Stop();

   cout << "Tests passed: " << pass << ", failed " << fail << endl;
   cout << "Total time per test: CPU "
        << 1.e6 * watch1.CpuTime()/Double_t(nTests) << " mus, Real "
        << 1.e6 * watch1.RealTime()/Double_t(nTests) << " mus" << endl;
   cout << "Core  time per test: CPU "
        << 1.e6 * watch2.CpuTime()/Double_t(nTests) << " mus, Real "
        << 1.e6 * watch2.RealTime()/Double_t(nTests) << " mus" << endl;
   if ( fail ) testStatus = kFALSE;
   // =======================================================================




   // =======================================================================
   // Test 5:  Overflow in GetAddress
   // =======================================================================
   cout << endl << endl;
   cout << "Test 5: overflow in GetAddress" << endl;

   // ----- Generate random element Ids
   for (Int_t level = 1; level < nLevels; level++) {
     Int_t range = ( 1 << bitField[level] ) - 1;
     elementId[level] = gRandom->Integer(range + 1);
   }

   // ----- Illegal value for a random level
   Int_t checkLevel = gRandom->Integer(nLevels-1) + 1;
   Int_t range = ( 1 << bitField[checkLevel] ) - 1;
   elementId[checkLevel] = range + 1;

   // ----- Get address  (should give an error)
   address = CbmStsAddress::GetAddress(elementId[1],
                                       elementId[2],
                                       elementId[3],
                                       elementId[4],
                                       elementId[5],
                                       elementId[6],
                                       elementId[7]);

   // ----- Get address with array (should give an error)
   newAddress = CbmStsAddress::GetAddress(elementId);

   // ----- Check result
   cout << "Addresses are " << address << " " << newAddress;
   if ( address + newAddress ) {
     testStatus = kFALSE;
     cout << "  : FAILED" << endl;
   }
   else cout << "  : OK" << endl;
   // =======================================================================



   // =======================================================================
   // Test 6:  Overflow in SetElementId
   // =======================================================================
   cout << endl << endl;
   cout << "Test 6: overflow in SetElementId" << endl;

   // ---- Generate a random STS address
   Int_t nBits = 32 - bitField[0];
   address = ( gRandom->Integer( 1 << nBits ) << bitField[0] ) | 2;

   // ----- Illegal value for a random level
   Int_t checkLevel = gRandom->Integer(nLevels-1) + 1;
   Int_t range = ( 1 << bitField[checkLevel] ) - 1;
   Int_t newId = range + 1;

   // ----- Set illegal element id for chosen level (should return error)
   newAddress = CbmStsAddress::SetElementId(address, checkLevel, newId);

   // ----- Check result
   cout << "New address is " << newAddress;
   if ( newAddress ) {
     testStatus = kFALSE;
     cout << "  : FAILED" << endl;
   }
   else cout << "  : OK" << endl;
   // =======================================================================



   // =====   Test result     ===============================================
   timer.Stop();
   cout << endl << endl;
   cout << "Time consumed: CPU " << timer.CpuTime() << " s, real "
        << timer.RealTime() << " s" << endl;
   cout << "Test status: ";
   if ( testStatus ) {
     cout << " PASSED" << endl << endl;
     return 0;
   }
   cout << " FAILED" << endl << endl;
   return 1;
   // =======================================================================

};
