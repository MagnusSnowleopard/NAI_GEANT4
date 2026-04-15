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
/// \file NaI/include/EventAction.hh
/// \brief Definition of the NaI::EventAction class

#ifndef B1EventAction_h
#define B1EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"
#include <map>

class G4Event;
class G4Track;

namespace NaI
{

class RunAction;

/// Event action class

class EventAction : public G4UserEventAction
{
  public:
    EventAction(RunAction* runAction);
    ~EventAction() override = default;

    void BeginOfEventAction(const G4Event* event) override;
    void EndOfEventAction(const G4Event* event) override;

    enum class TrackOrigin
    {
      kUnknown = 0,
      kGamma,
      kNeutron
    };

    void AddEdep(G4double edep) { fEdep += edep; }
    void AddGammaOriginEdep(G4double edep) { fEdepGammaOrigin += edep; }
    void AddNeutronOriginEdep(G4double edep) { fEdepNeutronOrigin += edep; }

    TrackOrigin GetTrackOrigin(const G4Track* track) const;
    void CacheTrackOrigin(const G4Track* track);

  private:
    G4double SmearDetectedEnergy(G4double energy) const;

    RunAction* fRunAction = nullptr;
    G4double fEdep = 0.;
    G4double fEdepGammaOrigin = 0.;
    G4double fEdepNeutronOrigin = 0.;
    G4bool fHasPrimaryGamma = false;
    G4bool fHasPrimaryNeutron = false;
    std::map<G4int, TrackOrigin> fTrackOrigins;
};

}  // namespace NaI

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
