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

#include "G4Event.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4VPhysicalVolume.hh"
#include "G4RotationMatrix.hh"

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
        // ------------------------------------------------------------
        // One NEW shared vertex per event, bounded by the circular face
        // of the W disk.
        //
        // Keep BOTH guns.
        // Keep the momentum cone centered on world +z.
        //
        // Geometry reminder:
        // - W disk is centered at local z = +3.5 mm in ChamberLV
        // - thickness = 6 mm  -> front face at local z = 0.5 mm
        // - ChamberPV is rotated by +90 deg about x
        //
        // Under rotateX(+90 deg):
        //   world x = local x
        //   world y = -local z
        //   world z = local y
        //
        // So the W face is a circular disk in world (x,z) at fixed y.
        // ------------------------------------------------------------

        // ---- Direction cone: same idea as before, around world +z ----
        G4double angularSigma = 0.4 * deg;
        G4double dirx = G4RandGauss::shoot(0.0, angularSigma);
        G4double diry = G4RandGauss::shoot(0.0, angularSigma);
        G4ThreeVector dir(dirx, diry, 1.0);
        dir = dir.unit();

        // ---- W disk geometry in chamber-local coordinates ----
        G4double puckRadius        = 0.5 * 25.4 * mm;
        G4double puckCenterZ       = -3 * mm;
        G4double puckHalfThickness = 0.5 * 6.0 * mm;

        // Beam-facing/front face of the tungsten disk in local coords.
        // Move 1 um INSIDE the W to avoid placing exactly on a boundary.
        G4double zLocal = (puckCenterZ - puckHalfThickness) + 1.0 * um;

        // ---- New random vertex EACH event on the circular W face ----
        // Gaussian beam spot, truncated by the disk radius.
        G4double beamSigma = 5.0 * mm;

        G4double xLocal = 0.0;
        G4double yLocal = 0.0;
        do {
                xLocal = G4RandGauss::shoot(0.0, beamSigma);
                yLocal = G4RandGauss::shoot(0.0, beamSigma);
        } while (xLocal*xLocal + yLocal*yLocal > puckRadius*puckRadius);

        // Convert from chamber-local to world coordinates.
        // rotateX(+90 deg): (x, y, z) -> (x, -z, y)
        G4double sourceX = xLocal;
        G4double sourceY = yLocal;
        G4double sourceZ = zLocal;

        G4ThreeVector sourcePos(sourceX, sourceY, sourceZ);

        // ---------------- Photon energy sampling ----------------
        G4double chosenPhotonEnergy = 0.0;
        if (fPhotonRateSum > 0.0 && !fPhotonEnergyRates.empty()) {
                G4double randomPhoton = G4UniformRand() * fPhotonRateSum;
                G4double cumulative = 0.0;
                for (const auto& pr : fPhotonEnergyRates) {
                        cumulative += pr.second;
                        if (randomPhoton <= cumulative) {
                                chosenPhotonEnergy = pr.first;
                                break;
                        }
                }
                if (chosenPhotonEnergy <= 0.0) {
                        chosenPhotonEnergy = fPhotonEnergyRates.back().first;
                }
        }

        G4double realE = chosenPhotonEnergy;
        if (chosenPhotonEnergy > 0.0) {
                G4double gammaresolution = 0.018;
                G4double sigma = gammaresolution * chosenPhotonEnergy;
                realE = G4RandGauss::shoot(chosenPhotonEnergy, sigma);
                if (realE <= 0.0) realE = chosenPhotonEnergy;
        }
        fGunGamma->SetParticleEnergy(realE);

        // ---------------- Neutron energy sampling ----------------
        G4double chosenNeutronEnergy = 0.0;
        if (fNeutronRateSum > 0.0 && !fNeutronEnergyRates.empty()) {
                G4double randomNeutron = G4UniformRand() * fNeutronRateSum;
                G4double cumulative = 0.0;
                for (const auto& nr : fNeutronEnergyRates) {
                        cumulative += nr.second;
                        if (randomNeutron <= cumulative) {
                                chosenNeutronEnergy = nr.first;
                                break;
                        }
                }
                if (chosenNeutronEnergy <= 0.0) {
                        chosenNeutronEnergy = fNeutronEnergyRates.back().first;
                }
        }

        G4double realEn = chosenNeutronEnergy;
        if (chosenNeutronEnergy > 0.0) {
                G4double neutronresolution = 0.5;
                G4double nsigma = neutronresolution * chosenNeutronEnergy;
                realEn = G4RandGauss::shoot(chosenNeutronEnergy, nsigma);
                if (realEn <= 0.0) realEn = chosenNeutronEnergy;
        }
        fGunNeutron->SetParticleEnergy(realEn);

        // ---- Same new event vertex for both guns ----
        fGunGamma->SetParticlePosition(sourcePos);
        fGunNeutron->SetParticlePosition(sourcePos);

        fGunGamma->SetParticleMomentumDirection(dir);
        fGunNeutron->SetParticleMomentumDirection(dir);

        // Optional debug: uncomment for a quick check
        
/*        if (event->GetEventID() < 20) {
                G4cout << "Event " << event->GetEventID()
                       << "  local(x,y,z)=("
                       << xLocal/mm << ", "
                       << yLocal/mm << ", "
                       << zLocal/mm << ") mm   worldPos="
                       << sourcePos/mm << " mm"
                       << G4endl;
        }
        
*/
        fGunGamma->GeneratePrimaryVertex(event);
        fGunNeutron->GeneratePrimaryVertex(event);
}

}  // namespace NaI
