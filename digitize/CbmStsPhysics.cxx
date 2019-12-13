/** @file CbmStsPhysics.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 02.12.2014
 ** @date 13.03.2019
 **/


#include "CbmStsPhysics.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include "TDatabasePDG.h"
#include "TMath.h"
#include "TRandom.h"
#include "TSystem.h"
#include "FairLogger.h"


using std::ifstream;
using std::right;
using std::setw;
using std::map;
using std::stringstream;


// -----   Initialisation of static variables   ----------------------------
CbmStsPhysics* CbmStsPhysics::fgInstance    = nullptr;
const Double_t CbmStsPhysics::fgkSiCharge   = 14.;
const Double_t CbmStsPhysics::fgkSiDensity  = 2.336;        // g/cm^3
const Double_t CbmStsPhysics::fgkProtonMass = 0.938272081;  // GeV
// -------------------------------------------------------------------------



// -----   Constructor   ---------------------------------------------------
CbmStsPhysics::CbmStsPhysics() :
  fELossModel(kELossUrban),
  fUseLorentzShift(kTRUE),
  fUseDiffusion(kTRUE),
  fUseCrossTalk(kTRUE),
  fGenerateNoise(kTRUE),
  fUrbanI(0.),
  fUrbanE1(0.),
  fUrbanE2(0.),
  fUrbanF1(0.),
  fUrbanF2(0.),
  fUrbanEmax(0.),
  fUrbanR(0.),
  fStoppingElectron(),
  fStoppingProton(),
  fLandauWidth()
{
  // --- Read the energy loss data tables
  LOG(info) << "Instantiating STS Physics... ";
  ReadDataTablesStoppingPower();
  ReadDataTablesLandauWidth();

  // --- Initialise the constants for the Urban model
  SetUrbanParameters(fgkSiCharge);
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsPhysics::~CbmStsPhysics() {
}
// -------------------------------------------------------------------------



// -----   Diffusion width   -----------------------------------------------
Double_t CbmStsPhysics::DiffusionWidth(Double_t z, Double_t d,
                                       Double_t vBias, Double_t vFd,
                                       Double_t temperature,
                                       Int_t chargeType) {

  // --- Check parameters. A tolerance of 0.1 micrometer on the sensor borders
  // --- is used to avoid crashes due to rounding errors.
  if ( z < 0. && z > -0.00001 ) z = 0.;
  if ( z > d  && z < d + 0.00001 ) z = d;
  if ( z < 0. || z > d ) {
    LOG(error) << "StsPhysics: z coordinate " << z
        << " not inside sensor (d = " << d << ")";
    return -1.;
  }
  if ( temperature < 0. ) {
    LOG(error) << "StsPhysics: illegal temperature value " << temperature;
    return -1.;
  }

  // --- Diffusion constant over mobility [J/C]
  // --- The numerical factor is k_B/e in units of J/(KC).
  Double_t diffConst = 8.61733e-5 * temperature;

  // --- Drift time times mobility [cm**2 * C / J]
  // For the formula, see the STS digitiser note.
  Double_t tau = 0.;
  if ( chargeType == 0 ) {   // electrons, drift to n (front) side
    tau = 0.5 * d * d / vFd
        * log( ( vBias + ( 1. - 2. * z / d ) * vFd ) / ( vBias - vFd ) );
  }
  else if ( chargeType == 1 ) { // holes, drift to the p (back) side
    tau = -0.5 * d * d / vFd
        * log( 1. - 2. * vFd * z / d / ( vBias + vFd ) );
  }
  else {
    LOG(error) << "StsPhysics: Illegal charge type " << chargeType;
    return -1.;
  }

  return sqrt(2. * diffConst * tau);
}
// -------------------------------------------------------------------------



// -----   Electric field   ------------------------------------------------
Double_t CbmStsPhysics::ElectricField(Double_t vBias, Double_t vFd,
                                      Double_t dZ, Double_t z) {
  return ( vBias + vFd * (2. * z / dZ - 1.) ) / dZ;
}
// -------------------------------------------------------------------------



// -----   Energy loss from fluctuation model   ----------------------------
Double_t CbmStsPhysics::EnergyLoss(Double_t dz, Double_t mass, Double_t eKin,
                                   Double_t dedx) const {

  // Gamma and beta
  Double_t gamma = (eKin + mass) / mass;
  Double_t beta2 = 1. - 1. / ( gamma * gamma );

  // Auxiliary
  Double_t xAux = 2. * mass * beta2 * gamma * gamma;

  // Mean energy losses (PHYS333 2.4 eqs. (2) and (3))
  Double_t sigma1 = dedx * fUrbanF1 / fUrbanE1
      * ( TMath::Log(xAux / fUrbanE1) - beta2 )
      / ( TMath::Log(xAux / fUrbanI) - beta2 )
      * (1. - fUrbanR);
  Double_t sigma2 = dedx * fUrbanF2 / fUrbanE2
      * ( TMath::Log(xAux / fUrbanE2) - beta2 )
      / ( TMath::Log(xAux / fUrbanI) - beta2 )
      * (1. - fUrbanR);
  Double_t sigma3 = dedx * fUrbanEmax * fUrbanR
      / ( fUrbanI * ( fUrbanEmax + fUrbanI ) )
      / TMath::Log( (fUrbanEmax + fUrbanI) / fUrbanI );

  // Sample number of processes Poissonian energy loss distribution
  // (PHYS333 2.4 eq. (6))
  Int_t n1 = gRandom->Poisson( sigma1 * dz );
  Int_t n2 = gRandom->Poisson( sigma2 * dz );
  Int_t n3 = gRandom->Poisson( sigma3 * dz );

  // Ion energy loss (PHYS333 2.4 eq. (12))
  Double_t eLossIon = 0.;
  for (Int_t j = 1; j <= n3; j++) {
    Double_t uni = gRandom->Uniform(1.);
    eLossIon += fUrbanI
        / ( 1. - uni * fUrbanEmax / ( fUrbanEmax + fUrbanI ) );
  }

  // Total energy loss
  return (n1 * fUrbanE1 + n2 * fUrbanE2 + eLossIon);

}
// -------------------------------------------------------------------------



// ----- Get static instance   ---------------------------------------------
CbmStsPhysics* CbmStsPhysics::Instance() {
  if ( ! fgInstance ) fgInstance = new CbmStsPhysics();
  return fgInstance;
}
// -------------------------------------------------------------------------



// -----   Interpolate a value from a data table   -------------------------
Double_t CbmStsPhysics::InterpolateDataTable(Double_t eEquiv,
                                             map<Double_t, Double_t>& table) {

  std::map<Double_t, Double_t>::iterator it = table.lower_bound(eEquiv);

  // Input value smaller than or equal to first table entry:
  // return first value
  if ( it == table.begin() ) return it->second;

  // Input value larger than last table entry: return last value
  if ( it == table.end() ) return (--it)->second;

  // Else: interpolate from table values
  Double_t e2 = it->first;
  Double_t v2 = it->second;
  it--;
  Double_t e1 = it->first;
  Double_t v1 = it->second;
  return ( v1 + ( eEquiv - e1 ) * ( v2 - v1 ) / ( e2 - e1 ) );

}
// -------------------------------------------------------------------------



// -----   Landau Width   ------------------------------------------------
Double_t CbmStsPhysics::LandauWidth(Double_t mostProbableCharge) {

  // --- Get interpolated value from the data table
  return InterpolateDataTable(mostProbableCharge, fLandauWidth);
}
// -------------------------------------------------------------------------



// -----    Particle charge for PDG PID   ----------------------------------
Double_t CbmStsPhysics::ParticleCharge(Int_t pid) {

  Double_t charge = 0.;

  // --- For particles in the TDatabasePDG. Note that TParticlePDG
  // --- gives the charge in units of |e|/3, God knows why.
  TParticlePDG* particle = TDatabasePDG::Instance()->GetParticle(pid);
  if ( particle ) charge = particle->Charge() / 3.;

  // --- For ions
  else if ( pid > 1000000000  && pid < 1010000000 ) {
    Int_t myPid = pid / 10000;
    charge = Double_t( myPid - ( myPid / 1000 ) * 1000 );
  }

  return charge;
}
// -------------------------------------------------------------------------



// -----    Particle mass for PDG PID   ------------------------------------
Double_t CbmStsPhysics::ParticleMass(Int_t pid) {

  Double_t mass = -1.;

  // --- For particles in the TDatabasePDG
  TParticlePDG* particle = TDatabasePDG::Instance()->GetParticle(pid);
  if ( particle ) mass = particle->Mass();

  // --- For ions
  else if ( pid > 1000000000  && pid < 1010000000 ) {
    Int_t myPid = pid - 1e9;
    myPid -= (myPid / 10000 ) * 10000;
    mass = Double_t( myPid / 10 );
  }

  return mass;
}
// -------------------------------------------------------------------------



// -----   Print processes to screen   -------------------------------------
void CbmStsPhysics::ShowProcesses() const {

  LOG(info) << GetName() << ": Process settings";
  stringstream ss;
  switch (fELossModel) {
    case kELossIdeal:   ss << "\t Energy loss model: Ideal"; break;
    case kELossUniform: ss << "\t Energy loss model: Uniform"; break;
    case kELossUrban:   ss << "\t Energy loss model: Urban"; break;
    default: ss << "\t Energy loss model: !!! UNKNOWN !!!"; break;
  }
  LOG(info) << ss.str();
  LOG(info) << "\t Lorentz shift      " << (fUseLorentzShift ? "ON" : "OFF");
  LOG(info) << "\t Diffusion          " << (fUseDiffusion ? "ON" : "OFF");
  LOG(info) << "\t Cross-talk         " << (fUseCrossTalk ? "ON" : "OFF");
  LOG(info) << "\t Noise              " << (fGenerateNoise ? "ON" : "OFF");

}
// -------------------------------------------------------------------------



// -----   Read data tables for stopping power   ---------------------------
void CbmStsPhysics::ReadDataTablesLandauWidth() {

  // The table with errors for Landau distribution:
  // MP charge (e) --> half width of charge distribution (e)

  TString dir = gSystem->Getenv("VMCWORKDIR");
  TString errFileName = dir + "/parameters/sts/LandauWidthTable.txt";

  ifstream inFile;
  Double_t q, err;

  // --- Read electron stopping power
  inFile.open(errFileName);
  if ( inFile.is_open() ) {
    while (true) {
      inFile >> q;
      inFile >> err;
      if ( inFile.eof() ) break;
      fLandauWidth[q] = err;
    }
    inFile.close();
    LOG(info) << "StsPhysics: " << setw(5) << right
        << fLandauWidth.size() << " values read from " << errFileName;
  }
  else
    LOG(fatal) << "StsPhysics: Could not read from " << errFileName;

}
// -------------------------------------------------------------------------



// -----   Read data tables for stopping power   ---------------------------
void CbmStsPhysics::ReadDataTablesStoppingPower() {

  // The data tables are obtained from the NIST ESTAR and PSTAR databases:
  // http://www.nist.gov/pml/data/star/index.cfm
  // The first column in the tables is the kinetic energy in MeV, the second
  // one is the specific stopping power in MeV*cm^2/g for Silicon.
  // Internally, the values are stored in GeV and GeV*cm^2/g, respectively.
  // Conversion MeV->GeV is done when reading from file.

  TString dir = gSystem->Getenv("VMCWORKDIR");
  TString eFileName = dir + "/parameters/sts/dEdx_Si_e.txt";
  TString pFileName = dir + "/parameters/sts/dEdx_Si_p.txt";

  ifstream inFile;
  Double_t e, dedx;

  // --- Read electron stopping power
  inFile.open(eFileName);
  if ( inFile.is_open() ) {
    while (true) {
      inFile >> e;
      inFile >> dedx;
      if ( inFile.eof() ) break;
      e    *= 1.e-3;  // MeV -> GeV
      dedx *= 1.e-3;  // MeV -> GeV
      fStoppingElectron[e] = dedx;
    }
    inFile.close();
    LOG(info) << "StsPhysics: " << setw(5) << right
        << fStoppingElectron.size() << " values read from " << eFileName;
  }
  else
    LOG(fatal) << "StsPhysics: Could not read from " << eFileName;

  // --- Read proton stopping power
  inFile.open(pFileName);
  if ( inFile.is_open() ) {
    while (true) {
      inFile >> e;
      inFile >> dedx;
      if ( inFile.eof() ) break;
      e    *= 1.e-3;  // MeV -> GeV
      dedx *= 1.e-3;  // MeV -> GeV
      fStoppingProton[e] = dedx;
    }
    inFile.close();
    LOG(info) << "StsPhysics: " << setw(5) << right
        << fStoppingProton.size() << " values read from " << pFileName;
  }
  else
    LOG(fatal) << "StsPhysics: Could not read from " << pFileName;

}
// -------------------------------------------------------------------------



// -----   Set the parameters for the Urban model   ------------------------
void CbmStsPhysics::SetUrbanParameters(Double_t z) {

  // --- Mean ionisation potential according to PHYS333 2.1
  fUrbanI = 1.6e-8 * TMath::Power(z, 0.9);  // in GeV

  // --- Maximal energy loss (delta electron threshold) [GeV]
  // TODO: 1 MeV is the default setting in our transport simulation.
  // It would be desirable to obtain the actually used value automatically.
  fUrbanEmax = 1.e-3;

  // --- The following parameters were defined according the GEANT3 choice
  // --- described in PHYS332 2.4 (p.264)

  // --- Oscillator strengths of energy levels
  fUrbanF1 = 1. - 2. / z;
  fUrbanF2 = 2. / z;

  // --- Energy levels [GeV]
  fUrbanE2= 1.e-8 * z * z;
  fUrbanE1
  = TMath::Power(fUrbanI / TMath::Power(fUrbanE2, fUrbanF2), 1./fUrbanF1);

  // --- Relative weight excitation / ionisation
  fUrbanR = 0.4;

  // --- Screen output
  LOG(info) << "StsPhysics: Urban parameters for z = " << z << " :";
  LOG(info) << "I = " << fUrbanI*1.e9 << " eV, Emax = " << fUrbanEmax*1.e9
      << " eV, E1 = " << fUrbanE1*1.e9 << " eV, E2 = "
      << fUrbanE2*1.e9 << " eV, f1 = " << fUrbanF1 << ", f2 = "
      << fUrbanF2 << ", r = " << fUrbanR;

}
// -------------------------------------------------------------------------



// -----   Stopping power   ------------------------------------------------
Double_t CbmStsPhysics::StoppingPower(Double_t eKin, Int_t pid) {

  Double_t mass = ParticleMass(pid);
  if ( mass < 0. ) return 0.;
  Double_t charge = ParticleCharge(pid);
  Bool_t isElectron = ( pid == 11 || pid == -11 );

  return StoppingPower(eKin, mass, charge, isElectron);
}
// -------------------------------------------------------------------------



// -----   Stopping power   ------------------------------------------------
Double_t CbmStsPhysics::StoppingPower(Double_t energy,
                                      Double_t mass,
                                      Double_t charge,
                                      Bool_t isElectron) {

  // --- Get interpolated value from data table
  Double_t stopPower = -1.;
  if ( isElectron )
    stopPower = InterpolateDataTable(energy, fStoppingElectron);
  else {
    Double_t eEquiv = energy * fgkProtonMass / mass; // equiv. proton energy
    stopPower = InterpolateDataTable(eEquiv, fStoppingProton);
  }

  // --- Calculate stopping power (from specific SP and density of silicon)
  stopPower *= fgkSiDensity;    // density of silicon

  // --- For non-electrons: scale with squared charge
  if ( ! isElectron ) stopPower *= ( charge * charge );

  return stopPower;
}
// -------------------------------------------------------------------------


ClassImp(CbmStsPhysics)


