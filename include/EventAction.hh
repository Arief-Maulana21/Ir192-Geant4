#ifndef B1EventAction_h
#define B1EventAction_h
#include "G4UserEventAction.hh"
#include "ScoreSchema.hh"
namespace B1 {
class RunAction;
class EventAction : public G4UserEventAction {
public:
 explicit EventAction(RunAction* r): fRunAction(r) {}
 void BeginOfEventAction(const G4Event*) override;
 void EndOfEventAction(const G4Event*) override;
 void AddOrganEdep(G4int organ,G4double e,G4bool pb,G4int particle,G4int birth);
private:
 RunAction* fRunAction;
 std::array<Scores,NOrg> fScores{};
};
}
#endif
