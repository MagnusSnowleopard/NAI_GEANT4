//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file NaI/include/PrimaryGeneratorAction.hh
/// \brief Definition of the NaI::PrimaryGeneratorAction class

#ifndef B1PrimaryGeneratorAction_h
#define B1PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"
#include "globals.hh"
#include <vector>
#include <utility>
class G4ParticleGun;
class G4Event;
class G4GenericMessenger;

namespace NaI
{

/// The primary generator action class with particle gun.
///
/// The default kinematic is a 6 MeV gamma, randomly distribued
/// in front of the phantom across 80% of the (X,Y) phantom size.

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    enum class SourceMode
    {
      kGammaOnly,
      kNeutronOnly,
      kFullAmBe
    };

    PrimaryGeneratorAction();
    ~PrimaryGeneratorAction() override;

    // method from the base class
    void GeneratePrimaries(G4Event*) override;

    // method to access particle gun
    const G4ParticleGun* GetNeutronGun() const { return fGunNeutron; }
    const G4ParticleGun* GetGammaGun() const { return fGunGamma; }

  private:
    void BuildDefaultAmBeSpectrum();
    void ConfigureMessenger();
    G4double SampleNeutronEnergy() const;
    G4double SampleIsotropicCostheta() const;
    G4ThreeVector SampleIsotropicDirection() const;

    G4ParticleGun* fGunNeutron = nullptr;  // pointer a to G4 gun class
    G4ParticleGun* fGunGamma = nullptr;  // pointer a to G4 gun class

    G4GenericMessenger* fMessenger = nullptr;
    G4String fModeName = "full";
    SourceMode fMode = SourceMode::kFullAmBe;
    G4double fGammaEnergy = 4438. * keV;
    G4double fGammaPerNeutron = 5.75e-3;
    G4double fSourceRadius = 0.0 * mm;

    std::vector<std::pair<double,double>> fPhotonEnergyRates;
    std::vector<std::pair<double,double>> fNeutronEnergyRates;

    double fPhotonRateSum;
    double fNeutronRateSum;
};

}  // namespace NaI

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
