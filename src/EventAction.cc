#include "EventAction.hh"

#include "RunAction.hh"

#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"

namespace NaI
{

EventAction::EventAction(RunAction* runAction) : fRunAction(runAction) {}

void EventAction::BeginOfEventAction(const G4Event*)
{
  fEdepTotal = 0.;
  fEdepGammaLineage = 0.;
  fEdepNeutronLineage = 0.;
  fTrackLineage.clear();
}

void EventAction::ClassifyTrack(G4int trackId, G4int parentId, const G4String& particleName)
{
  if (fTrackLineage.find(trackId) != fTrackLineage.end()) {
    return;
  }

  LineageTag tag = LineageTag::kUnknown;

  if (parentId == 0) {
    if (particleName == "gamma") {
      tag = LineageTag::kPrimaryGamma;
    }
    else if (particleName == "neutron") {
      tag = LineageTag::kPrimaryNeutron;
    }
  }
  else {
    const auto it = fTrackLineage.find(parentId);
    if (it != fTrackLineage.end()) {
      tag = it->second;
    }
  }

  fTrackLineage.emplace(trackId, tag);
}

void EventAction::AddEdepByTrack(G4int trackId, G4double edep)
{
  fEdepTotal += edep;

  const auto it = fTrackLineage.find(trackId);
  if (it == fTrackLineage.end()) {
    return;
  }

  if (it->second == LineageTag::kPrimaryGamma) {
    fEdepGammaLineage += edep;
  }
  else if (it->second == LineageTag::kPrimaryNeutron) {
    fEdepNeutronLineage += edep;
  }
}

void EventAction::EndOfEventAction(const G4Event*)
{
  auto* analysisManager = G4AnalysisManager::Instance();

  const auto total_keV = fEdepTotal / keV;
  const auto gamma_keV = fEdepGammaLineage / keV;
  const auto neutron_keV = fEdepNeutronLineage / keV;

  analysisManager->FillH1(0, total_keV);
  analysisManager->FillH1(1, gamma_keV);
  analysisManager->FillH1(2, neutron_keV);

  fRunAction->ScorePeakWindow(gamma_keV, neutron_keV);
}

}  // namespace NaI
