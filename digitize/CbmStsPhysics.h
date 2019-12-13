/** @file CbmStsPhysics.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 02.12.2014
 ** @date 13.03.2019
 **/

#ifndef CBMSTSPHYSICS_H
#define CBMSTSPHYSICS_H 1


#include <iostream>
#include <map>
#include "Rtypes.h"
#include "TObject.h"


/** @enum ECbmELossModel
 ** @brief Switch for energy loss model in STS response simulation
 **
 ** kELossIdeal:   Energy loss is concentrated in sensor mid-plane
 ** kELossUniform: Uniform energy loss over the trajectory in the sensor
 ** kELossUrban:   Energy loss fluctuations following the Urban model
 **
 ** In the detector response simulation, charge packets are created along the
 ** trajectory of a charged particle in the silicon sensors. The sum of the
 ** charges corresponds to the deposited energy. The energy loss model
 ** controls how these charge packets are generated.
 **/
enum ECbmELossModel{
  kELossIdeal, kELossUniform, kELossUrban
};


/** @class CbmStsPhysics
 ** @brief Auxiliary class for physics processes in Silicon
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 02.12.2014
 **
 ** This singleton class is auxiliary for the detector response simulation
 ** of the CBM-STS, but can also be used from reconstruction ar analysis
 ** (e.g., Lorentz shift).
 **/
class CbmStsPhysics : public TObject {

  public:

    /** Destructor **/
    virtual ~CbmStsPhysics();


    /** Diffusion width as function of z
     ** @param z           Distance from p side [cm]
     ** @param d           Thickness of sensor [cm]
     ** @param vBias       Bias voltage [V]
     ** @param vFd         Full depletion voltage [V]
     ** @param temperature Temperature [K]
     ** @param chargeType  0 = electron, 1 = hole
     ** @return Drift time [s]
     **
     ** Calculates the diffusion width (sigma) for a charge drifting
     ** from z to the readout (z = 0 for hole, z = d for electrons).
     **
     ** For the reference to the formulae, see the STS digitiser note.
     **/
    static Double_t DiffusionWidth(Double_t z, Double_t d, Double_t vBias,
                                   Double_t vFd, Double_t temperature,
                                   Int_t chargeType);


    /** @brief Electric field magnitude in a silicon sensor as function of z
     ** @param vBias  Bias voltage [V]
     ** @param vFd    Full depletion voltage [V]
     ** @param dZ     Thickness of sensor [cm]
     ** @param z      z coordinate, measured from the p side [cm]
     ** @return       z component of electric field [V/cm]
     **/
    static Double_t ElectricField(Double_t vBias, Double_t vFd,
                                  Double_t dZ, Double_t z);


    /** @brief Energy loss in a Silicon layer
     ** @param dz    Layer thickness [cm]
     ** @param mass  Particle mass [GeV]
     ** @param eKin  Kinetic energy [GeV]
     ** @param dedx  Average specific energy loss [GeV/cm]
     ** @return Energy loss in the layer [GeV]
     **
     ** The energy loss is sampled from the Urban fluctuation model
     ** described in the GEANT3 manual (PHYS333 2.4, pp. 262-264).
     */
    Double_t EnergyLoss(Double_t dz, Double_t mass,
                        Double_t eKin, Double_t dedx) const;


    /** @brief Flag for generation of inter-event noise
     ** @return if kTRUE, noise will be generated
     **/
    Bool_t GenerateNoise() const { return fGenerateNoise; }


    /** Atomic charge of Silicon
     ** @return Atomic charge of Silicon [e]
     **/
    static Double_t GetSiCharge() { return fgkSiCharge; }


    /** @brief Accessor to singleton instance
     ** @return  Pointer to singleton instance
     **
     ** Will instantiate a singleton object if not yet existing.
     **/
    static CbmStsPhysics* Instance();


    /** @brief Half width at half max of Landau distribution
     ** in ultra-relativistic case
     ** @param mostProbableCharge [e]
     ** @return half width [e]
     **/
    Double_t LandauWidth(Double_t mostProbableCharge);


    /** @brief Energy for electron-hole pair creation in silicon
     ** @return Pair creation energy [GeV]
     **/
    static Double_t PairCreationEnergy() { return 3.57142e-9; }


    /** @brief Particle charge from PDG particle ID
     ** @param pid   PID (PDG code)
     ** @return Particle charge [e]
     **
     ** For particles in the TDataBasePDG, the charge is taken from there.
     ** For ions, it is calculated following the PDG code convention.
     ** If not found, zero is returned.
     **/
    static Double_t ParticleCharge(Int_t pid);


    /** @brief Particle mass from PDG particle ID
     ** @param pid   PID (PDG code)
     ** @return Particle mass [GeV]
     **
     ** For particles in the TDataBasePDG, the mass is taken from there.
     ** For ions, it is calculated following the PDG code convention.
     ** If not found, zero is returned.
     **/
    static Double_t ParticleMass(Int_t pid);


    /** @brief Set process flags
     ** @param eLossModel  Switch for energy loss model
     ** @param useLorentShift  Switch for usage of Lorentz shift
     ** @param useDiffusion  Switch for usage of thermal diffusion
     ** @param useCrossTalk Switch for usage of cross-talk
     ** @param generateNoise  Switch for the generation of electronic noise
     **
     ** The energy loss model determines where along the trajectory of the
     ** particle in the sensor charges are created. Lorentz shift and
     ** diffusion are relevant for the propagation of the charges to the
     ** read-out surface. Cross talk regulates capacitance-induced charge
     ** sharing between neighbouring channels. Noise is thermal noise
     ** in the read-out ASIC, independent of activation of the sensors by
     ** traversing charged particles.
     **/
    void SetProcesses(ECbmELossModel eLossModel, Bool_t useLorentzShift,
                      Bool_t useDiffusion, Bool_t useCrossTalk,
                      Bool_t generateNoise) {
      fELossModel = eLossModel;
      fUseLorentzShift = useLorentzShift;
      fUseDiffusion = useDiffusion;
      fUseCrossTalk = useCrossTalk;
      fGenerateNoise = generateNoise;
    }


    /** @brief Print processes to screen **/
    void ShowProcesses() const;


    /** @brief Stopping power (average specific energy loss) in Silicon
     ** @param eKin  Kinetic energy pf the particle [GeV]
     ** @param pid   Particle ID (PDG code)
     ** @return Stopping power [GeV/cm]
     **
     ** This function calculates the stopping power
     ** (average specific energy loss) in Silicon of a particle specified
     ** by its PDG code. For an unknown pid, null is returned.
     **/
    Double_t StoppingPower(Double_t eKin, Int_t pid);


    /** Stopping power in Silicon
     ** @param energy      Energy of particle [GeV]
     ** @param mass        Particle mass [GeV]
     ** @param charge      Electric charge [e]
     ** @param isElectron  kTRUE if electron, kFALSE else
     ** @return            Stopping power [GeV/cm]
     **
     ** This function calculates the stopping power
     ** (average specific energy loss) in Silicon of a particle
     ** with given mass and charge.
     **/
    Double_t StoppingPower(Double_t energy, Double_t mass,
                           Double_t charge, Bool_t isElectron);


    /** @brief Flag for cross-talk
     ** @return if kTRUE, cross-talk will be used
     **/
    Bool_t UseCrossTalk() const { return fUseCrossTalk; }


    /** @brief Flag for diffusion
     ** @return if kTRUE, diffusion will be used
     **/
    Bool_t UseDiffusion() const { return fUseDiffusion; }


    /** @brief Flag for Lorentz shift
     ** @return if kTRUE, Lorentz shift will be used
     **/
    Bool_t UseLorentzShift() const { return fUseLorentzShift; }



  private:

    static CbmStsPhysics* fgInstance;      ///< Pointer to signleton instance

    // --- Physical constants
    static const Double_t fgkSiCharge;     ///< Silicon atomic charge number
    static const Double_t fgkSiDensity;    ///< Silicon density [g/cm^3]
    static const Double_t fgkProtonMass;   ///< proton mass [GeV]

    // --- Process flags
    ECbmELossModel  fELossModel;
    Bool_t fUseLorentzShift;
    Bool_t fUseDiffusion;
    Bool_t fUseCrossTalk;
    Bool_t fGenerateNoise;

    // --- Parameters for the Urban model
    Double_t fUrbanI;     ///< Urban model: mean ionisation potential of Silicon
    Double_t fUrbanE1;    ///< Urban model: first atomic energy level
    Double_t fUrbanE2;    ///< Urban model: second atomic energy level
    Double_t fUrbanF1;    ///< Urban model: oscillator strength first level
    Double_t fUrbanF2;    ///< Urban model: oscillator strength second level
    Double_t fUrbanEmax;  ///< Urban model: cut-off energy (delta-e threshold)
    Double_t fUrbanR;     ///< Urban model: weight parameter excitation/ionisation

    // --- Data tables for stopping power
    std::map<Double_t, Double_t> fStoppingElectron;  ///< E [GeV] -> <-dE/dx> [GeV*g/cm^2]
    std::map<Double_t, Double_t> fStoppingProton  ;  ///< E [GeV] -> <-dE/dx> [GeV*g/cm^2]

    // --- Data tables for width of Landau distribution
    std::map<Double_t, Double_t> fLandauWidth; ///< q [e] -> width [e]



    // =====   Private member functions   ========================================

    /** @brief Constructor **/
    CbmStsPhysics();


    /** @brief Copy constructor forbidden **/
    CbmStsPhysics(const CbmStsPhysics&) = delete;;


    /** @brief Assignment operator forbidden **/
    CbmStsPhysics operator=(const CbmStsPhysics&) = delete;


    /** @brief Interpolate a value from the data tables
     ** @param eKin   Equivalent kinetic energy [GeV]
     ** @param table  Reference to data map (fStoppingElectron or fStoppingProton)
     ** @return Interpolated value from data table
     **
     ** The eEquiv is below the tabulated range, the first table value is
     ** returned; if it is above the range, the last value is returned.
     **/
    Double_t InterpolateDataTable(Double_t eKin, std::map<Double_t, Double_t>& table);


    /** @brief Read stopping power data table from file **/
    void ReadDataTablesStoppingPower();


    /** @brief Read Landau width data table from file **/
    void ReadDataTablesLandauWidth();


    /** @brief Calculate the parameters for the Urban model
     ** @param z  Atomic charge of material element
     **
     ** The parameters are set according to the GEANT3 manual (PHYS332 and PHYS333)
     **/
    void SetUrbanParameters(Double_t z);


    ClassDef(CbmStsPhysics,1);

};

#endif /* CBMSTSPHYSICS_H_ */
