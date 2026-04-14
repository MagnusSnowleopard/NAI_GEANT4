#include "SteppingAction.hh"

#include "EventAction.hh"

#include "G4Step.hh"
#include "G4String.hh"
#include "G4Track.hh"

namespace NaI
{

SteppingAction::SteppingAction(EventAction* eventAction) : fEventAction(eventAction) {}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  const auto volume = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume();
  if (!volume || volume->GetName() != "NaI_crystal") {
    return;
  }

  const G4double edep = step->GetTotalEnergyDeposit();
  if (edep <= 0.) {
    return;
  }

  const auto* track = step->GetTrack();
  fEventAction->CacheTrackOrigin(track);

  fEventAction->AddEdep(edep);

  const auto origin = fEventAction->GetTrackOrigin(track);
  if (origin == EventAction::TrackOrigin::kGamma) {
    fEventAction->AddGammaOriginEdep(edep);
  } else if (origin == EventAction::TrackOrigin::kNeutron) {
    fEventAction->AddNeutronOriginEdep(edep);
  }
}

}  // namespace NaI
