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
// * Thi:s  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file NaI/src/RunAction.cc
/// \brief Implementation of the NaI::RunAction class

#include "RunAction.hh"

#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"

#include "G4AccumulableManager.hh"
#include "G4LogicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleGun.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4AnalysisManager.hh"

namespace NaI
{

	//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

	RunAction::RunAction()
	{
		auto analysisManager = G4AnalysisManager::Instance();
		G4cout << "Using" << analysisManager->GetType()
			<< "analysis manager " << G4endl;

		analysisManager->SetVerboseLevel(1);
		analysisManager->SetFileName("NaI.root");

//#ifdef G4MULTITHREADED
//		analysisManager->SetNtupleMerging(true);
//#endif

		//create hist 

		G4int nbins = 8192;
		G4double xmin = 0.;
		G4double xmax = 16384.;

		analysisManager->CreateH1("gammaE",
				"Gamma response energy deposited in crystal",
				nbins,xmin,xmax);

		analysisManager->CreateH1("ProtonE",
				"Proton Energy deposited in crystal from n-reaction",
				nbins,xmin,xmax);

//		analysisManager->CreateH1("GammaE",
//				"Gamma Energy deposited in crystal",
//				nbins,xmin,xmax);


		analysisManager->CreateH1("AlphaE",
				"Alpha Energy deposited in crystal from n-reaction",
				nbins,xmin,xmax);


		analysisManager->CreateH1("TritonE",
				"Triton Energy deposited in crystal from n-reaction",
				nbins,xmin,xmax);
	}

	//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

	void RunAction::BeginOfRunAction(const G4Run*)
	{
		fEdep = 0.;

		auto analysisManager = G4AnalysisManager::Instance();

//		if(IsMaster()){

			analysisManager->OpenFile();

//		}
	}

	//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

	void RunAction::EndOfRunAction(const G4Run* run)
	{
		auto analysisManager = G4AnalysisManager::Instance();

//		if(IsMaster()){
			analysisManager->Write();
			analysisManager->CloseFile();
//		}
	}

	//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
	//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace NaI
