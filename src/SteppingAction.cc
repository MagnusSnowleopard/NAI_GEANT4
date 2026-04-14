#include "SteppingAction.hh"

#include "EventAction.hh"

#include "G4Step.hh"
#include "G4Track.hh"

namespace NaI
{

SteppingAction::SteppingAction(EventAction* eventAction) : fEventAction(eventAction) {}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  auto* volume = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume();
  if (!volume || volume->GetName() != "NaI_crystal") {
    return;
  }

  auto* track = step->GetTrack();
  fEventAction->ClassifyTrack(track->GetTrackID(), track->GetParentID(),
                              track->GetDefinition()->GetParticleName());

  const auto edep = step->GetTotalEnergyDeposit();
  if (edep > 0.) {
    fEventAction->AddEdepByTrack(track->GetTrackID(), edep);
  }
}

}  // namespace NaI
