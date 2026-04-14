#include "PrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <string>

namespace
{
G4ThreeVector SampleIsotropicDirection()
{
  const auto cosTheta = 2. * G4UniformRand() - 1.;
  const auto sinTheta = std::sqrt(std::max(0., 1. - cosTheta * cosTheta));
  const auto phi = CLHEP::twopi * G4UniformRand();
  return G4ThreeVector(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);
}

NaI::PrimaryGeneratorAction::SourceMode ParseModeFromEnv(const char* modeEnv)
{
  if (!modeEnv) {
    return NaI::PrimaryGeneratorAction::SourceMode::kAmBeMixed;
  }

  const std::string mode(modeEnv);
  if (mode == "gamma" || mode == "gamma_only") {
    return NaI::PrimaryGeneratorAction::SourceMode::kGammaOnly;
  }
  if (mode == "neutron" || mode == "neutron_only") {
    return NaI::PrimaryGeneratorAction::SourceMode::kNeutronOnly;
  }
  return NaI::PrimaryGeneratorAction::SourceMode::kAmBeMixed;
}
}  // namespace

namespace NaI
{

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
  fGunNeutron = new G4ParticleGun(1);
  fGunGamma = new G4ParticleGun(1);

  auto* particleTable = G4ParticleTable::GetParticleTable();
  fGunNeutron->SetParticleDefinition(particleTable->FindParticle("neutron"));
  fGunGamma->SetParticleDefinition(particleTable->FindParticle("gamma"));

  fSourceMode = ParseModeFromEnv(std::getenv("NAI_SOURCE_MODE"));
  if (const char* ratioEnv = std::getenv("NAI_AMBE_GAMMA_PER_NEUTRON")) {
    const auto ratio = std::atof(ratioEnv);
    if (ratio > 0.) {
      fGammaPerNeutron = ratio;
    }
  }

  // Approximate AmBe neutron spectrum (Geiger/Van der Zwan-like shape), MeV.
  static constexpr std::array<double, 11> kEnergiesMeV = {
    0.2, 0.5, 1.0, 1.5, 2.0, 2.7, 3.5, 4.5, 6.0, 8.0, 10.5};
  static constexpr std::array<double, 11> kRelativeWeights = {
    0.03, 0.06, 0.10, 0.13, 0.15, 0.16, 0.14, 0.11, 0.07, 0.04, 0.01};

  double sum = 0.;
  for (const auto w : kRelativeWeights) {
    sum += w;
  }

  double cumulative = 0.;
  for (std::size_t i = 0; i < kEnergiesMeV.size(); ++i) {
    fNeutronEnergies.push_back(kEnergiesMeV[i] * MeV);
    cumulative += kRelativeWeights[i] / sum;
    fNeutronCdf.push_back(cumulative);
  }
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fGunNeutron;
  delete fGunGamma;
}

double PrimaryGeneratorAction::SampleNeutronEnergy() const
{
  const auto u = G4UniformRand();
  for (std::size_t i = 0; i < fNeutronCdf.size(); ++i) {
    if (u <= fNeutronCdf[i]) {
      return fNeutronEnergies[i];
    }
  }
  return fNeutronEnergies.back();
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
  const G4ThreeVector vertex(0., 0., 0.);

  if (fSourceMode != SourceMode::kNeutronOnly) {
    fGunGamma->SetParticlePosition(vertex);
    fGunGamma->SetParticleMomentumDirection(SampleIsotropicDirection());
    fGunGamma->SetParticleEnergy(4.438 * MeV);

    if (fSourceMode == SourceMode::kGammaOnly || G4UniformRand() < fGammaPerNeutron) {
      fGunGamma->GeneratePrimaryVertex(event);
    }
  }

  if (fSourceMode != SourceMode::kGammaOnly) {
    fGunNeutron->SetParticlePosition(vertex);
    fGunNeutron->SetParticleMomentumDirection(SampleIsotropicDirection());
    fGunNeutron->SetParticleEnergy(SampleNeutronEnergy());
    fGunNeutron->GeneratePrimaryVertex(event);
  }
}

}  // namespace NaI
