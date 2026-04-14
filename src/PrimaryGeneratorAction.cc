#include "PrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"
#include <cmath>

namespace NaI
{

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
  fGunNeutron = new G4ParticleGun(1);
  auto particleTable = G4ParticleTable::GetParticleTable();
  fGunNeutron->SetParticleDefinition(particleTable->FindParticle("neutron"));
  fGunNeutron->SetParticlePosition(G4ThreeVector(0., 0., 0.));

  fGunGamma = new G4ParticleGun(1);
  fGunGamma->SetParticleDefinition(particleTable->FindParticle("gamma"));
  fGunGamma->SetParticlePosition(G4ThreeVector(0., 0., 0.));

  BuildDefaultAmBeSpectrum();
  ConfigureMessenger();
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fGunNeutron;
  delete fGunGamma;
  delete fMessenger;
}

void PrimaryGeneratorAction::BuildDefaultAmBeSpectrum()
{
  // Approximate AmBe neutron spectrum (piecewise points, weights are relative).
  fNeutronEnergyRates = {
    {0.1 * MeV, 0.06}, {0.3 * MeV, 0.08}, {0.5 * MeV, 0.10}, {0.8 * MeV, 0.14},
    {1.2 * MeV, 0.17}, {1.8 * MeV, 0.19}, {2.5 * MeV, 0.16}, {3.2 * MeV, 0.12},
    {4.0 * MeV, 0.08}, {5.0 * MeV, 0.05}, {6.5 * MeV, 0.03}, {8.0 * MeV, 0.015},
    {10.0 * MeV, 0.01}, {11.0 * MeV, 0.005}};

  fNeutronRateSum = 0.;
  for (const auto& nr : fNeutronEnergyRates) {
    fNeutronRateSum += nr.second;
  }

  fPhotonEnergyRates = {{4438. * keV, 1.0}};
  fPhotonRateSum = 1.0;
}

void PrimaryGeneratorAction::ConfigureMessenger()
{
  fMessenger = new G4GenericMessenger(this, "/NaI/source/", "Primary generator control");

  auto& modeCmd = fMessenger->DeclareProperty("mode", fModeName,
                                               "Source mode: gamma, neutron, full");
  modeCmd.SetStates(G4State_PreInit, G4State_Idle);

  auto& gammaECmd = fMessenger->DeclarePropertyWithUnit(
      "gammaEnergy", "keV", fGammaEnergy, "Prompt gamma energy (default 4438 keV)");
  gammaECmd.SetStates(G4State_PreInit, G4State_Idle);

  auto& ratioCmd = fMessenger->DeclareProperty(
      "gammaPerNeutron", fGammaPerNeutron,
      "Average number of prompt 4.438 MeV gammas per neutron (default 5.75e-3)");
  ratioCmd.SetStates(G4State_PreInit, G4State_Idle);

  auto& radiusCmd = fMessenger->DeclarePropertyWithUnit(
      "sourceRadius", "mm", fSourceRadius,
      "Active source radius; 0 mm gives point source.");
  radiusCmd.SetStates(G4State_PreInit, G4State_Idle);
}

G4double PrimaryGeneratorAction::SampleNeutronEnergy() const
{
  if (fNeutronEnergyRates.empty() || fNeutronRateSum <= 0.) {
    return 2.5 * MeV;
  }

  const G4double r = G4UniformRand() * fNeutronRateSum;
  G4double cumulative = 0.;
  for (const auto& eAndW : fNeutronEnergyRates) {
    cumulative += eAndW.second;
    if (r <= cumulative) {
      return eAndW.first;
    }
  }
  return fNeutronEnergyRates.back().first;
}

G4double PrimaryGeneratorAction::SampleIsotropicCostheta() const
{
  return 2.0 * G4UniformRand() - 1.0;
}

G4ThreeVector PrimaryGeneratorAction::SampleIsotropicDirection() const
{
  const G4double cosTheta = SampleIsotropicCostheta();
  const G4double sinTheta = std::sqrt(1. - cosTheta * cosTheta);
  const G4double phi = CLHEP::twopi * G4UniformRand();
  return G4ThreeVector(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
  if (fModeName == "gamma") {
    fMode = SourceMode::kGammaOnly;
  } else if (fModeName == "neutron") {
    fMode = SourceMode::kNeutronOnly;
  } else {
    fMode = SourceMode::kFullAmBe;
  }

  G4ThreeVector sourcePos(0., 0., 0.);
  if (fSourceRadius > 0.) {
    const G4double rho = fSourceRadius * std::sqrt(G4UniformRand());
    const G4double phi = CLHEP::twopi * G4UniformRand();
    sourcePos = G4ThreeVector(rho * std::cos(phi), rho * std::sin(phi), 0.);
  }

  if (fMode == SourceMode::kGammaOnly || fMode == SourceMode::kFullAmBe) {
    const G4bool emitGamma =
        (fMode == SourceMode::kGammaOnly) ? true : (G4UniformRand() < fGammaPerNeutron);

    if (emitGamma) {
      fGunGamma->SetParticlePosition(sourcePos);
      fGunGamma->SetParticleMomentumDirection(SampleIsotropicDirection());
      fGunGamma->SetParticleEnergy(fGammaEnergy);
      fGunGamma->GeneratePrimaryVertex(event);
    }
  }

  if (fMode == SourceMode::kNeutronOnly || fMode == SourceMode::kFullAmBe) {
    fGunNeutron->SetParticlePosition(sourcePos);
    fGunNeutron->SetParticleMomentumDirection(SampleIsotropicDirection());
    fGunNeutron->SetParticleEnergy(SampleNeutronEnergy());
    fGunNeutron->GeneratePrimaryVertex(event);
  }
}

}  // namespace NaI
