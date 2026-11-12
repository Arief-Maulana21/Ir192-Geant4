#ifndef B1TrackInformation_h
#define B1TrackInformation_h 1

#include "G4VUserTrackInformation.hh"
#include "globals.hh"

namespace B1
{
// Metadata follows a track and is copied to its descendants.
// PbHistory is true after an electromagnetic interaction in PbShield,
// or when the particle descends from a track with that history.
class TrackInformation : public G4VUserTrackInformation
{
public:
    TrackInformation() = default;
    explicit TrackInformation(G4bool pbHistory)
        : fPbHistory(pbHistory) {}
    ~TrackInformation() override = default;

    G4bool HasPbHistory() const { return fPbHistory; }
    void MarkPbHistory() { fPbHistory = true; }

    void Print() const override;

private:
    G4bool fPbHistory = false;
};
}
#endif
