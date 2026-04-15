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
/// \file NaI/include/RunAction.hh
/// \brief Definition of the NaI::RunAction class

#ifndef B1RunAction_h
#define B1RunAction_h 1

#include "G4UserRunAction.hh"
#include "G4SystemOfUnits.hh"
#include "globals.hh"

class G4Run;
class G4GenericMessenger;

namespace NaI
{

/// Run action class
///
/// In EndOfRunAction(), it calculates the dose in the selected volume
/// from the energy deposit accumulated via stepping and event actions.
/// The computed dose is then printed on the screen.

class RunAction : public G4UserRunAction
{
  public:
    RunAction();
    ~RunAction() override;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

    void CountWindowEvent(G4bool gammaOnlyPrimary, G4bool neutronOnlyPrimary, G4bool fullAmBePrimary);
    G4double GetPeakCenterKeV() const { return fPeakCenter / keV; }
    G4double GetPeakHalfWidthKeV() const { return fPeakHalfWidth / keV; }
    G4double GetResolutionRefEnergy() const { return fResolutionRefEnergy; }
    G4double GetResolutionRefFwhmFraction() const { return fResolutionRefFwhmFraction; }
    G4bool GetApplyResolutionSmearing() const { return fApplyResolutionSmearing; }

    G4double fEdep = 0.;

  private:
    void ConfigureMessenger();

    G4GenericMessenger* fMessenger = nullptr;
    G4double fPeakCenter = 4438. * keV;
    G4double fPeakHalfWidth = 120. * keV;
    G4double fResolutionRefEnergy = 4438. * keV;
    G4double fResolutionRefFwhmFraction = 0.055;
    G4bool fApplyResolutionSmearing = true;
    G4int fGammaOnlyWindowCounts = 0;
    G4int fNeutronOnlyWindowCounts = 0;
    G4int fFullAmBeWindowCounts = 0;
    
};

}  // namespace NaI

#endif
