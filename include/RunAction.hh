#ifndef B1RunAction_h
#define B1RunAction_h 1

#include "G4UserRunAction.hh"

#include "globals.hh"

class G4Run;

namespace NaI
{

class RunAction : public G4UserRunAction
{
  public:
    RunAction();
    ~RunAction() override = default;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

    void ScorePeakWindow(G4double gammaEdepKeV, G4double neutronEdepKeV);

  private:
    G4double fPeakCenterKeV = 4438.0;
    G4double fPeakHalfWidthKeV = 120.0;

    G4long fEventsGammaInWindow = 0;
    G4long fEventsNeutronInWindow = 0;
};

}  // namespace NaI

#endif
