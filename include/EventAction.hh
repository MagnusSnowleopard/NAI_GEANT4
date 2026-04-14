#ifndef B1EventAction_h
#define B1EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

#include <unordered_map>

class G4Event;

namespace NaI
{

class RunAction;

class EventAction : public G4UserEventAction
{
  public:
    explicit EventAction(RunAction* runAction);
    ~EventAction() override = default;

    void BeginOfEventAction(const G4Event* event) override;
    void EndOfEventAction(const G4Event* event) override;

    enum class LineageTag
    {
      kUnknown,
      kPrimaryGamma,
      kPrimaryNeutron
    };

    void AddEdepByTrack(G4int trackId, G4double edep);
    void ClassifyTrack(G4int trackId, G4int parentId, const G4String& particleName);

  private:
    RunAction* fRunAction = nullptr;

    G4double fEdepTotal = 0.;
    G4double fEdepGammaLineage = 0.;
    G4double fEdepNeutronLineage = 0.;

    std::unordered_map<G4int, LineageTag> fTrackLineage;
};

}  // namespace NaI

#endif
