#include "EventAction.hh"

#include "RunAction.hh"

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4String.hh"
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"
#include "Randomize.hh"
#include <algorithm>
#include <cmath>

namespace NaI
{

	EventAction::EventAction(RunAction* runAction) : fRunAction(runAction) {}

	EventAction::TrackOrigin EventAction::GetTrackOrigin(const G4Track* track) const
	{
		const auto found = fTrackOrigins.find(track->GetTrackID());
		if (found != fTrackOrigins.end()) {
			return found->second;
		}
		return TrackOrigin::kUnknown;
	}

	void EventAction::CacheTrackOrigin(const G4Track* track)
	{
		const G4int trackID = track->GetTrackID();
		if (fTrackOrigins.find(trackID) != fTrackOrigins.end()) {
			return;
		}

		TrackOrigin origin = TrackOrigin::kUnknown;

		if (track->GetParentID() == 0) {
			const auto pName = track->GetParticleDefinition()->GetParticleName();
			if (pName == "gamma") {
				origin = TrackOrigin::kGamma;
			} else if (pName == "neutron") {
				origin = TrackOrigin::kNeutron;
			}
		} else {
			const auto parent = fTrackOrigins.find(track->GetParentID());
			if (parent != fTrackOrigins.end()) {
				origin = parent->second;
			}
		}

		fTrackOrigins[trackID] = origin;
	}

	void EventAction::BeginOfEventAction(const G4Event* event)
	{
		fEdep = 0.;
		fEdepGammaOrigin = 0.;
		fEdepNeutronOrigin = 0.;
		fTrackOrigins.clear();
		fHasPrimaryGamma = false;
		fHasPrimaryNeutron = false;

		const auto nVertices = event->GetNumberOfPrimaryVertex();
		for (G4int iv = 0; iv < nVertices; ++iv) {
			const auto* vertex = event->GetPrimaryVertex(iv);
			for (auto* particle = vertex->GetPrimary(); particle != nullptr;
					particle = particle->GetNext()) {
				const auto pName = particle->GetParticleDefinition()->GetParticleName();
				if (pName == "gamma") {
					fHasPrimaryGamma = true;
				} else if (pName == "neutron") {
					fHasPrimaryNeutron = true;
				}
			}
		}
	}

	void EventAction::EndOfEventAction(const G4Event*)
	{
		auto analysisManager = G4AnalysisManager::Instance();

		const auto totalEdepKeV = fEdep / keV;
		const auto gammaOriginEdepKeV = fEdepGammaOrigin / keV;
		const auto neutronOriginEdepKeV = fEdepNeutronOrigin / keV;

		analysisManager->FillH1(0, totalEdepKeV);
		analysisManager->FillH1(1, gammaOriginEdepKeV);
		analysisManager->FillH1(2, neutronOriginEdepKeV);

		if (fHasPrimaryGamma && !fHasPrimaryNeutron) {
			analysisManager->FillH1(3, totalEdepKeV);
		}
		if (!fHasPrimaryGamma && fHasPrimaryNeutron) {
			analysisManager->FillH1(4, totalEdepKeV);
		}
		if (fHasPrimaryGamma && fHasPrimaryNeutron) {
			analysisManager->FillH1(5, totalEdepKeV);
		}

		const auto center = fRunAction->GetPeakCenterKeV();
		const auto halfWidth = fRunAction->GetPeakHalfWidthKeV();
		if (totalEdepKeV >= (center - halfWidth) && totalEdepKeV <= (center + halfWidth)) {
			fRunAction->CountWindowEvent(fHasPrimaryGamma && !fHasPrimaryNeutron,
					!fHasPrimaryGamma && fHasPrimaryNeutron,
					fHasPrimaryGamma && fHasPrimaryNeutron);
		}
	}


}  // namespace NaI
