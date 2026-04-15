#include "PrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"
#include <algorithm>
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
  // AmBe neutron spectrum provided as relative intensity vs. energy points.
  // Energy points are in MeV and intensity points are arbitrary relative units.
  const std::vector<G4double> energiesMeV = {
      0.257, 0.368, 0.515, 0.897, 1.059, 1.191, 1.353, 1.485, 1.544, 1.588, 1.618,
      1.735, 1.794, 1.824, 2.000, 2.088, 2.147, 2.265, 2.309, 2.368, 2.412, 2.515,
      2.603, 2.647, 2.721, 2.838, 2.956, 3.103, 3.147, 3.235, 3.471, 3.544, 3.691,
      3.735, 3.956, 4.147, 4.235, 4.368, 4.456, 4.529, 4.662, 4.882, 5.029, 5.221,
      5.309, 5.412, 5.500, 5.632, 5.706, 5.794, 5.824, 5.882, 5.941, 6.088, 6.294,
      6.485, 6.691, 6.794, 6.838, 7.088, 7.294, 7.647, 7.794, 8.147, 8.279, 8.397,
      8.559, 8.706, 8.985, 9.059, 9.235, 9.338, 9.544, 9.662, 9.750, 9.860};
  const std::vector<G4double> intensities = {
      199.072, 185.151, 169.838, 151.740, 151.740, 142.691, 134.339, 119.722, 108.585,
      95.360, 89.095, 74.478, 67.517, 60.557, 66.125, 70.998, 75.870, 87.007, 98.144,
      110.673, 125.986, 135.731, 153.132, 161.485, 184.455, 194.200, 206.032, 233.875,
      246.404, 255.452, 244.316, 237.355, 230.394, 220.650, 209.513, 194.896, 190.023,
      186.543, 194.896, 201.160, 208.817, 211.601, 210.905, 203.944, 198.376, 194.200,
      190.719, 187.935, 187.239, 173.318, 164.269, 144.780, 135.035, 128.074, 127.378,
      125.290, 122.506, 114.849, 107.889, 99.536, 98.144, 104.408, 98.144, 88.399, 84.919,
      64.037, 45.244, 42.459, 45.244, 52.900, 67.517, 72.390, 79.350, 68.213, 48.724,
      38.283};

  fNeutronEnergyRates.clear();
  if (energiesMeV.size() != intensities.size() || energiesMeV.empty()) {
    fNeutronEnergyRates = {{2.5 * MeV, 1.0}};
  } else {
    fNeutronEnergyRates.reserve(energiesMeV.size());
    for (std::size_t i = 0; i < energiesMeV.size(); ++i) {
      G4double dE = 0.;
      if (i == 0) {
        dE = energiesMeV[i + 1] - energiesMeV[i];
      } else if (i + 1 == energiesMeV.size()) {
        dE = energiesMeV[i] - energiesMeV[i - 1];
      } else {
        dE = 0.5 * (energiesMeV[i + 1] - energiesMeV[i - 1]);
      }
      const G4double weight = std::max(0., intensities[i]) * std::max(0., dE);
      fNeutronEnergyRates.emplace_back(energiesMeV[i] * MeV, weight);
    }
  }

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
      const G4double chosenPhotonEnergy = fGammaEnergy;
      const G4double gammaresolution = 0.02;  // 2% (sigma/E)
      const G4double sigma = gammaresolution * chosenPhotonEnergy;
      G4double realE = G4RandGauss::shoot(chosenPhotonEnergy, sigma);
      if (realE < 0.) {
        realE = chosenPhotonEnergy;
      }

      fGunGamma->SetParticlePosition(sourcePos);
      fGunGamma->SetParticleMomentumDirection(SampleIsotropicDirection());
      fGunGamma->SetParticleEnergy(realE);
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
