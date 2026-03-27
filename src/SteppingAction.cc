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
/// \file NaI/src/SteppingAction.cc
/// \brief Implementation of the NaI::SteppingAction class

#include "SteppingAction.hh"

#include "DetectorConstruction.hh"
#include "EventAction.hh"

#include "G4Event.hh"
#include "G4LogicalVolume.hh"
#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4String.hh"
#include "G4Proton.hh"
#include "G4Alpha.hh"
#include "G4Deuteron.hh"
#include "G4Triton.hh"
#include "G4SystemOfUnits.hh"

namespace NaI
{


	SteppingAction::SteppingAction(EventAction* eventAction) : fEventAction(eventAction) {}


	void SteppingAction::UserSteppingAction(const G4Step* step)
	{
		G4double edep = step->GetTotalEnergyDeposit();
		if (edep > 0.){
			//check if we are in the crystal
			auto volume = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume();
			if(!volume) return;

			G4String volumeName = volume->GetName();

			if(volumeName == "CLYC_crystal"){

				fEventAction->AddEdep(edep);


			}
		}

		// Now detect newly create seconaries that are protons or alphas. 

		auto secondaries = step->GetSecondaryInCurrentStep();

		if(!secondaries) return;

		auto analysisManager = G4AnalysisManager::Instance();

		//for each seconardy, if its p or a, fill the hist. 

		for(const auto& secTrack : *secondaries) {
			auto pd = secTrack->GetDefinition();
			if(!pd) continue;



			// Check if this secondary is create INSIDE the crystal
			auto preVol = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume();
			if(preVol->GetName() != "CLYC_crystal") continue;

			// We only want the initial kinetic energy when it is created 

			G4double kinEnergy = secTrack->GetKineticEnergy(); //in MeV
			G4double kinEnergy_keV = kinEnergy / keV; 

			if(pd == G4Proton::Definition()){
				analysisManager->FillH1(1, kinEnergy_keV); // Hist ID=1 --> "ProtonE"
			}else if(pd == G4Alpha::Definition()){
				analysisManager->FillH1(2, kinEnergy_keV); //Hist ID=3 --> "AlphaE"

			}else if(pd == G4Triton::Definition()){
				analysisManager->FillH1(3, kinEnergy_keV); //Hist ID=4 --> "Triton"

			}

		}
	}
}// namespace NaI
