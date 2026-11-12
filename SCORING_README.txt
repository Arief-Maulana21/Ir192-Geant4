Pb-history dose scoring patch (Geant4 11.3.2)

Integrates the previous TrackInformation patch.
Existing DetectorA remains the sole scoring volume; this is not an OAR phantom.
ROOT ntuple 4 HistoryEventDose has one row per event: EdepPb_keV, EdepNoPb_keV, EdepTotal_keV.
The categories are particle-history based, not direct-vs-scattered or clinical dose.
Event-level sums (including zero-deposition events) are used for uncertainty.

Build on Ubuntu:
  mkdir -p build && cd build
  cmake ..
  make -j4
  ./Ir192_Pb run10k.mac

Validation in ROOT:
  root -l SecondaryRadiation.root
  HistoryEventDose->Print();
  HistoryEventDose->Draw("EdepPb_keV");
  HistoryEventDose->Draw("EdepNoPb_keV");

Caution: unverified by compilation in this environment.
Check that the Pb-history marking in SteppingAction matches the intended physics.
