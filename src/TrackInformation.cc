#include "TrackInformation.hh"
#include "G4ios.hh"

namespace B1
{
void TrackInformation::Print() const
{
    G4cout << "Track PbHistory: " << (fPbHistory ? "true" : "false")
           << G4endl;
}
}
