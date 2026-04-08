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
/// \file NaI/src/DetectorConstruction.cc
/// \brief Implementation of the NaI::DetectorConstruction class

#include "DetectorConstruction.hh"
#include "G4Tubs.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Trd.hh"
#include "G4Isotope.hh"
#include "G4Element.hh"
#include "G4Material.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"


namespace NaI
{

	//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

	G4VPhysicalVolume* DetectorConstruction::Construct()
	{
		// Get nist material manager
		G4NistManager* nist = G4NistManager::Instance();
		// 95% enriched Li6, 5% Li7.
		G4Isotope* isoLi6 = new G4Isotope("Li6",3,6,6.015*g/mole);
		G4Isotope* isoLi7 = new G4Isotope("Li7",3,7,7.016*g/mole);
		G4Element* elLi = new G4Element("Lithium","Li",2);
		elLi->AddIsotope(isoLi6,0.95);
		elLi->AddIsotope(isoLi7,0.05);

		//Cl35and Cl37. 

		G4Isotope* isoCl35 = new G4Isotope("Cl35",17,35, 34.96885 * g/mole);
		G4Isotope* isoCl37 = new G4Isotope("Cl37",17,37, 36.96590 * g/mole);
		G4Element* elCl = new G4Element("Chlorine", "Cl",2);

		elCl->AddIsotope(isoCl35,.755);
		elCl->AddIsotope(isoCl37,.245);

		//Cesium, Yttrium, and cerium from nist at natural composition

		G4Element* elCs = nist->FindOrBuildElement("Cs");
		G4Element* elY = nist->FindOrBuildElement("Y");
		G4Element* elCe = nist->FindOrBuildElement("Ce");

		G4double density = 3.31*g/cm3;
		G4int nComponents  = 4; //Cs, Li, Y, Cl

		G4Material* mat_Cs2LiYCl6 = new G4Material("Cs2LiYCl6",density, nComponents);
		mat_Cs2LiYCl6->AddElement(elCs,2);
		mat_Cs2LiYCl6->AddElement(elLi,1);
		mat_Cs2LiYCl6->AddElement(elY,1);
		mat_Cs2LiYCl6->AddElement(elCl,6);

		// NaI detector Crystal construction. 
		
		G4Element* elNa = nist->FindOrBuildElement("Na");
		G4Element* elI = nist->FindOrBuildElement("I");
		G4Element* elTl = nist->FindOrBuildElement("Tl");

		G4double Ndensity = 3.67*g/cm3;
		G4int NnComponents  = 2; //Na + I 

		G4Material* mat_NaI = new G4Material("NaI",Ndensity, NnComponents);
		mat_NaI->AddElement(elNa,1);
		mat_NaI->AddElement(elI,1);

		//adding a 1% doping fraction of Thalium. 

		G4double dopingfraction = 0.01;

		G4Material* mat_NaI_Tl = new G4Material("NaI",Ndensity,2);
		mat_NaI_Tl->AddMaterial(mat_NaI,(1.-dopingfraction));
		mat_NaI_Tl->AddElement(elTl, dopingfraction);

		//adding a 1% doping fraction of Cerium. 


		G4Material* mat_CLYC_Ce = new G4Material("CLYC_Ce",density,2);
		mat_CLYC_Ce->AddMaterial(mat_Cs2LiYCl6,(1.-dopingfraction));
		mat_CLYC_Ce->AddElement(elCe, dopingfraction);

		G4bool checkOverlaps = true;

		//World
		//
		G4double world_size = 1000*cm;

		G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");

		G4Box* solidWorld = new G4Box("World",
				0.5*world_size,
				0.5*world_size,
				0.5*world_size);

		G4LogicalVolume* logicWorld = 
			new G4LogicalVolume(solidWorld, world_mat, "World");

		G4VPhysicalVolume* physWorld =
			new G4PVPlacement(nullptr,                //no rotation
					G4ThreeVector(),          // at (0,0,0)
					logicWorld,               // its logic vol
					"World",                  // its name
					nullptr,                  //no mother volume
					false,                    //no boolean operator
					0,                        //copy number
					checkOverlaps);
		
		// define target chamber (aluminum).
		G4Material* chamberMat = nist->FindOrBuildMaterial("G4_Al");

	
		G4double chamberRadius = 0.5*7.6*cm;
		G4double chamberHeight = 10.*cm;
		G4double chamberInnerRadius = chamberRadius - 0.5*cm;

		auto vaccumMat = nist->FindOrBuildMaterial("G4_Galactic");

		auto alChamber = new G4Tubs("alChamber",
				chamberInnerRadius,
				chamberRadius,
				chamberHeight,
				0.*deg,
				360.*deg);

		auto logicChamber = 
			new G4LogicalVolume(alChamber, chamberMat, "ChamberLV");

		G4RotationMatrix* rot = new G4RotationMatrix();
		rot->rotateX(90.*deg);

		new G4PVPlacement(rot,
				G4ThreeVector(0.,0.,0.),
				logicChamber,
				"ChamberPV",
				logicWorld,
				false,
				0,
				true);

		G4Tubs* solidVac =
			new G4Tubs("ChamberVac",
					0.,
					chamberInnerRadius,
					chamberHeight,
					0.*deg,
					360.*deg);
		G4LogicalVolume* logicVac = 
			new G4LogicalVolume(solidVac, vaccumMat,"ChamberVacLV");

		// place the vacuum as the daughter of the chamber.
		
	
		new G4PVPlacement(nullptr,
				G4ThreeVector(0.,0.,0.),
				logicVac,
				"chamberVacPV",
				logicChamber,
				false,
				0,
				true);

		// Tungsten attenuation insert (25.4 mm diameter, 6 mm thick) centered at z = +3.5 mm.
		G4Material* tungstenMat = nist->FindOrBuildMaterial("G4_W");
		G4double attenuatorRadius = 0.5 * 25.4 * mm;
		G4double attenuatorHalfThickness = 0.5 * 6.0 * mm;
		G4double attenuatorCenterZ = 3.5 * mm;

		G4Tubs* solidAttenuator = new G4Tubs("WAttenuator",
				0.,
				attenuatorRadius,
				attenuatorHalfThickness,
				0.*deg,
				360.*deg);
		G4LogicalVolume* logicAttenuator =
			new G4LogicalVolume(solidAttenuator, tungstenMat, "WAttenuatorLV");

		new G4PVPlacement(nullptr,
				G4ThreeVector(0., 0., attenuatorCenterZ),
				logicAttenuator,
				"WAttenuatorPV",
				logicVac,
				false,
				0,
				checkOverlaps);

		//Place CLYC detector

		G4double crystal_rad = 5.1*cm;
		G4double crystal_h = 5.1*cm;

		G4double NaI_x = 46.7*cm; 
		G4double NaI_y = 12.7*cm;
		G4double NaI_z = 7.6*cm;

		G4double NaIzpos = chamberRadius + 8* cm + NaI_z*0.5;

		G4double zPos = 1.*cm + 0.5*crystal_h +chamberRadius;
//CLYC
/*		G4Tubs* solidCrystal = new G4Tubs("solidCrystal",
				0,
				crystal_rad/2.,
				crystal_h/2,
				0.0*rad,
				360.0*rad);
*///NAI
		G4Box* solidCrystal = new G4Box("solidCrystal",
				NaI_x*0.5,
				NaI_y*0.5,
				NaI_z*0.5);

/*		G4LogicalVolume* logicCrystal = 
			new G4LogicalVolume(solidCrystal,
					mat_CLYC_Ce,
					"CLYC_crystal");

*///
		G4LogicalVolume* logicCrystal = 
			new G4LogicalVolume(solidCrystal,
					mat_NaI_Tl,
					"NaI_crystal");
		new G4PVPlacement(nullptr,
				G4ThreeVector(0.,0.,NaIzpos),
				logicCrystal,
				"NaI_crystal",
				logicWorld,
				false,
				0,
				checkOverlaps);

// NaI crystal is purple

		auto visCLYC = new G4VisAttributes(G4Colour(1.0,0.0,1.0));
		visCLYC->SetVisibility(true);
		visCLYC->SetForceSolid(true);
		logicCrystal->SetVisAttributes(visCLYC);
		
		auto visChamber = new G4VisAttributes(G4Colour(0.,1.,0.));
		visChamber->SetVisibility(true);
		visChamber->SetForceWireframe(true);
		logicChamber->SetVisAttributes(visChamber);

		auto visVac = new G4VisAttributes();
		visVac->SetVisibility(false);
		logicVac->SetVisAttributes(visVac);

		auto visAttenuator = new G4VisAttributes(G4Colour(0.4,0.4,0.6));
		visAttenuator->SetVisibility(true);
		visAttenuator->SetForceSolid(true);
		visAttenuator->SetForceAuxEdgeVisible(true);
		logicAttenuator->SetVisAttributes(visAttenuator);

		return physWorld;
	}

	//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace NaI
