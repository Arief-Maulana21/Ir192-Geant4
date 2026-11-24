#include "EventAction.hh"
#include "RunAction.hh"
#include "G4Event.hh"
namespace B1 {
void EventAction::BeginOfEventAction(const G4Event*) { for(auto& s:fScores) s.fill(0.); }
void EventAction::AddOrganEdep(G4int i,G4double e,G4bool pb,G4int particle,G4int birth) {
 if(i<0||i>=NOrg||e<=0.) return;
 auto& s=fScores[i]; s[0]+=e; s[pb?1:2]+=e; s[particle]+=e; s[birth]+=e;
}
void EventAction::EndOfEventAction(const G4Event* event) {
 if(!event||!fRunAction) return;
 for(G4int i=0;i<NOrg;++i) fRunAction->RecordOrgan(event->GetEventID(),i,fScores[i]);
 const auto& s=fScores[0];
 fRunAction->AddEdep(s[0]); fRunAction->AddHistoryEdep(s[1],s[2]);
 fRunAction->RecordEventDose(event->GetEventID(),s[0]);
 fRunAction->RecordHistoryEventDose(event->GetEventID(),s[1],s[2]);
}
}
