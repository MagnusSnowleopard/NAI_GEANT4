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
/// \file NaI/src/PrimaryGeneratorAction.cc
/// \brief Implementation of the NaI::PrimaryGeneratorAction class

#include "PrimaryGeneratorAction.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include <numeric>



namespace NaI
{

	//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

	PrimaryGeneratorAction::PrimaryGeneratorAction()
	{

		fGunNeutron = new G4ParticleGun(1);

		// default particle kinematic
		G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
		G4String particleName;
		G4ParticleDefinition* neutronDef = particleTable->FindParticle(particleName = "neutron");
		fGunNeutron->SetParticleDefinition(neutronDef);
		fGunNeutron->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));
		fGunNeutron->SetParticlePosition(G4ThreeVector(0.,0.,0.));

		// fGunNeutron->SetParticleEnergy(2.0 * MeV);

		fGunGamma = new G4ParticleGun(1);
		auto gammaDef = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
		fGunGamma->SetParticleDefinition(gammaDef);

		fGunGamma->SetParticlePosition(G4ThreeVector(0.,0.,0.));
		fGunGamma->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));
		fGunGamma->SetParticlePosition(G4ThreeVector(0.,0.,0.));

		// fGunGamma->SetParticleEnergy(1.0 * MeV);

		fPhotonEnergyRates = {
//			{121.*keV, 100},
	//		{244.*keV, 100}
//			{344.*keV, 100},
//			{662.*keV, 100},
		//	{778.*keV, 100},
//			{1172.*keV, 100},
//			{1332.*keV, 100},
//			{1408.*keV,100},
//			{1700.*keV,100},
//			{2000.*keV,100},
	//		{2400.*keV,100},
		//	{2800.*keV,100},
//			{3200.*keV,100},
//			{3600.*keV,100},
	//		{3800.*keV,100},
			{4400.*keV,100},
	//		{4800.*keV,100},
//			{5200.*keV,100},
//			{5800.*keV,100},
	//		{6200.*keV,100},
	//		{11400.*keV,100}

		};

//		fPhotonEnergyRates = {
//			{2000.*keV, 100}
			//{11662.*keV, 100},

//		};
		fNeutronEnergyRates= {
			{5000 *keV,0.000001},//2100 11B

		};
		fPhotonRateSum = 0.;
		for(const auto& pr: fPhotonEnergyRates){
			fPhotonRateSum +=pr.second;
		}
		fNeutronRateSum = 0.;
		for(const auto& nr : fNeutronEnergyRates){
			fNeutronRateSum += nr.second;
		}

	}

	//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

	PrimaryGeneratorAction::~PrimaryGeneratorAction()
	{
		delete fGunNeutron;
		delete fGunGamma;
	}

	//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

	void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
	{
		//Make a momentum cone
		G4double thetaMax = 1.*deg;
		G4double cosThetaMin = std::cos(thetaMax);
		G4double cosTheta = 1. - G4UniformRand()*(1. - cosThetaMin);
		G4double sinTheta = std::sqrt(1. - cosTheta*cosTheta);

		G4double phi = 2*CLHEP::pi * G4UniformRand();

		G4double dirx = sinTheta*std::cos(phi);
		G4double diry = sinTheta*std::sin(phi);
		G4double dirz = cosTheta;

		G4ThreeVector dir(dirx,diry,dirz);


		//sample photon energy 
		G4double randomPhoton = G4UniformRand() * fPhotonRateSum;
		G4double cumulative = 0.;
		G4double chosenPhotonEnergy = 0.0;
		for(const auto& pr : fPhotonEnergyRates){
			cumulative += pr.second;
			if(randomPhoton <= cumulative){
			chosenPhotonEnergy = pr.first;
			break;
			}
		}
		
	        G4double gammaresolution = 0.018; // 0.015~~ 4.1% 
		G4double sigma = gammaresolution*chosenPhotonEnergy;
		G4double realE = G4RandGauss::shoot(chosenPhotonEnergy,sigma);

	//	if(realE < 0.) realE = chosenPhotonEnergy;

		fGunGamma->SetParticleEnergy(realE);

		G4double randomNeutron = G4UniformRand() * fNeutronRateSum;
		G4double ncumulative = 0.;
		G4double chosenNeutronEnergy = 0.0;
		for(const auto& pr : fNeutronEnergyRates){
			ncumulative += pr.second;
			if(randomNeutron <= ncumulative){
			chosenNeutronEnergy = pr.first;
			break;
			}
		}

		G4double neutronresolution = 0.5; //10% 
		G4double nsigma = neutronresolution*chosenNeutronEnergy;
		G4double realEn = G4RandGauss::shoot(chosenNeutronEnergy,nsigma);

		if(realEn < 0.) realEn = chosenNeutronEnergy;
		fGunNeutron->SetParticleEnergy(chosenNeutronEnergy);


		fGunNeutron->SetParticleMomentumDirection(dir);
		fGunGamma->SetParticleMomentumDirection(dir);


		fGunNeutron->GeneratePrimaryVertex(event);
		fGunGamma->GeneratePrimaryVertex(event);
	}

	//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace NaI
