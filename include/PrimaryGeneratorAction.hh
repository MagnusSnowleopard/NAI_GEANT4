#ifndef B1PrimaryGeneratorAction_h
#define B1PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"

#include <vector>

class G4Event;
class G4ParticleGun;

namespace NaI
{

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    PrimaryGeneratorAction();
    ~PrimaryGeneratorAction() override;

    void GeneratePrimaries(G4Event*) override;

  public:
    enum class SourceMode
    {
      kGammaOnly,
      kNeutronOnly,
      kAmBeMixed
    };

  private:
    G4ParticleGun* fGunNeutron = nullptr;
    G4ParticleGun* fGunGamma = nullptr;

    std::vector<double> fNeutronEnergies;
    std::vector<double> fNeutronCdf;

    SourceMode fSourceMode = SourceMode::kAmBeMixed;
    double fGammaPerNeutron = 5.75e-3;

    double SampleNeutronEnergy() const;
};

}  // namespace NaI

#endif
