#ifndef B1ScoreSchema_h
#define B1ScoreSchema_h
#include "globals.hh"
#include <array>
namespace B1 {
constexpr G4int NOrg=6, NCat=17;
using Scores=std::array<G4double, NCat>;
inline const char* Category(G4int i) {
 static const char* names[]={"Total","PbHistory","NoPbHistory","PrimaryGammaUnscattered","PrimaryGammaScattered","SecondaryGamma","SecondaryElectron","SecondaryPositron","OtherParticle","PrimaryBirth","PbGenerated","CapsuleGenerated","ApplicatorGenerated","SourceGenerated","TissueGenerated","PhantomGenerated","OtherGenerated"};
 return names[i];
}
inline G4int BirthCategory(const G4String& v) {
 if(v=="PbShield") return 10;
 if(v=="SourceCapsule") return 11;
 if(v=="Applicator") return 12;
 if(v=="Ir192Source") return 13;
 if(v=="Tongue"||v=="OralCavity"||v=="Mandible"||v=="SpinalCord"||v=="SalivaryLeft"||v=="SalivaryRight") return 14;
 if(v=="HeadNeckPhantom") return 15;
 return 16;
}
}
#endif
