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
#include <cmath>



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
		// Sample a Gaussian angular spread around the local inward normal (-Z),
		// representing a beam spot hitting the attenuator target.
		G4double angularSigma = 0.4 * deg;
		G4double dirx = G4RandGauss::shoot(0., angularSigma);
		G4double diry = G4RandGauss::shoot(0., angularSigma);
		G4ThreeVector dir(dirx, diry, -1.0);
		dir = dir.unit();

		// Sample source position from a circular 2D Gaussian across the tungsten puck face.
		// The Gaussian is centered on axis and truncated at the puck radius.
		// Puck geometry: diameter = 25.4 mm, thickness = 6 mm, centered at z = +3.5 mm.
		// Emit from the +Z face and point into the puck (-Z direction).
		G4double puckRadius = 0.5 * 25.4 * mm;
		G4double puckCenterZ = 3.5 * mm;
		G4double puckHalfThickness = 0.5 * 6.0 * mm;
		G4double sourceZ = puckCenterZ + puckHalfThickness;
		G4double sigmaXY = puckRadius / 3.0;

		G4double sourceX = 0.;
		G4double sourceY = 0.;
		G4bool accepted = false;
		while(!accepted){
			G4double u1 = G4UniformRand();
			G4double u2 = G4UniformRand();
			if(u1 <= 0.) continue;

			// Box-Muller transform for independent Gaussian x/y samples.
			G4double rho = std::sqrt(-2.0 * std::log(u1));
			G4double theta = 2.0 * CLHEP::pi * u2;
			G4double gx = rho * std::cos(theta);
			G4double gy = rho * std::sin(theta);

			sourceX = sigmaXY * gx;
			sourceY = sigmaXY * gy;

			if((sourceX*sourceX + sourceY*sourceY) <= (puckRadius*puckRadius)){
				accepted = true;
			}
		}
		G4ThreeVector sourcePos(sourceX, sourceY, sourceZ);

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
		fGunNeutron->SetParticleEnergy(realEn);


		fGunNeutron->SetParticleMomentumDirection(dir);
		fGunGamma->SetParticleMomentumDirection(dir);
		fGunNeutron->SetParticlePosition(sourcePos);
		fGunGamma->SetParticlePosition(sourcePos);


		fGunNeutron->GeneratePrimaryVertex(event);
		fGunGamma->GeneratePrimaryVertex(event);
	}

	//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace NaI
