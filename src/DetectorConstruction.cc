#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4SubtractionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4ios.hh"

namespace B1 {

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    auto* nist = G4NistManager::Instance();
    auto* air = nist->FindOrBuildMaterial("G4_AIR");
    auto* water = nist->FindOrBuildMaterial("G4_WATER");
    auto* soft = nist->FindOrBuildMaterial("G4_TISSUE_SOFT_ICRP");
    auto* bone = nist->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU");
    auto* polyethylene = nist->FindOrBuildMaterial("G4_POLYETHYLENE");
    auto* steel = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
    auto* iridium = nist->FindOrBuildMaterial("G4_Ir");
    auto* lead = nist->FindOrBuildMaterial("G4_Pb");

    auto* worldSolid = new G4Box("World", 25*cm, 25*cm, 25*cm);
    auto* worldLV = new G4LogicalVolume(worldSolid, air, "World");
    auto* worldPV = new G4PVPlacement(nullptr, {}, worldLV, "World", nullptr,
                                      false, 0, true);
    worldLV->SetVisAttributes(G4VisAttributes::GetInvisible());

    auto* phantomSolid = new G4Box("HeadNeckPhantom", 10*cm, 10*cm, 12*cm);
    auto* phantomLV = new G4LogicalVolume(phantomSolid, water, "HeadNeckPhantom");
    new G4PVPlacement(nullptr, {}, phantomLV, "HeadNeckPhantom", worldLV,
                      false, 0, true);
    auto* phantomVis = new G4VisAttributes(G4Colour(0.4, 0.7, 0.9, 0.1));
    phantomVis->SetForceWireframe(true);
    phantomLV->SetVisAttributes(phantomVis);

    // Simplified surrogate geometry; all positions are in the phantom/world frame.
    // Tongue: x[-25,25], y[-20,20], z[32,68] mm.
    // Catheter axis: x=y=0, z[25,75] mm, inside tongue over z[32,68].
    const G4double catheterInner = 0.60*mm;
    const G4double catheterOuter = 0.80*mm;
    const G4double catheterHalfLength = 25.0*mm;
    const G4double capsuleRadius = 0.45*mm;
    const G4double capsuleHalfLength = 1.75*mm;

    // Fixed Pb plate on +X side of catheter, independent of dwell position.
    const G4double pbThickness = 1.0*mm;
    const G4double pbHalfWidth = 5.0*mm;
    const G4double pbHalfLength = 5.0*mm;
    const G4double pbGap = 0.10*mm;
    const G4double pbX = catheterOuter + pbGap + pbThickness/2.;
    const G4ThreeVector pbPosition = fApplicatorPosition + G4ThreeVector(pbX, 0, 0);

    // Subtract catheter clearance and Pb plate from the tongue tissue.
    // This avoids overlapping sibling physical volumes while keeping tissue
    // around the interstitial source. Clearance is a geometry tolerance only.
    const G4double clearance = 0.01*mm;
    auto* tongueBox = new G4Box("TongueBase", 25*mm, 20*mm, 18*mm);
    auto* catheterCut = new G4Tubs("TongueCatheterCut", 0,
                                    catheterOuter + clearance,
                                    catheterHalfLength + clearance,
                                    0*deg, 360*deg);
    auto* tongueWithoutCatheter = new G4SubtractionSolid(
        "TongueWithoutCatheter", tongueBox, catheterCut, nullptr,
        G4ThreeVector(0,0,0));
    auto* shieldCut = new G4Box("TongueShieldCut",
                                pbThickness/2. + clearance,
                                pbHalfWidth + clearance,
                                pbHalfLength + clearance);
    auto* tongueSolid = new G4SubtractionSolid(
        "Tongue", tongueWithoutCatheter, shieldCut, nullptr,
        G4ThreeVector(pbX,0,0));
    auto* tongueLV = new G4LogicalVolume(tongueSolid, soft, "Tongue");
    new G4PVPlacement(nullptr, fApplicatorPosition, tongueLV, "Tongue",
                      phantomLV, false, 0, true);
    fOrganVolumes[0] = tongueLV;
    fScoringVolume = tongueLV;

    auto addOrgan = [&](G4int id, const G4String& name,
                        G4double hx, G4double hy, G4double hz,
                        const G4ThreeVector& center, G4Material* material,
                        const G4Colour& colour) {
        auto* solid = new G4Box(name, hx, hy, hz);
        auto* lv = new G4LogicalVolume(solid, material, name);
        new G4PVPlacement(nullptr, center, lv, name, phantomLV,
                          false, 0, true);
        auto* vis = new G4VisAttributes(colour);
        vis->SetForceSolid(true);
        lv->SetVisAttributes(vis);
        fOrganVolumes[id] = lv;
    };

    addOrgan(1, "OralCavity", 25*mm, 6*mm, 18*mm,
             {0,-34*mm,50*mm}, soft, {0.95,0.75,0.35});
    addOrgan(2, "Mandible", 35*mm, 8*mm, 20*mm,
             {0,-55*mm,50*mm}, bone, {0.95,0.95,0.85});
    addOrgan(3, "SpinalCord", 7*mm, 8*mm, 25*mm,
             {0,50*mm,50*mm}, soft, {0.55,0.55,0.95});
    addOrgan(4, "SalivaryLeft", 12*mm, 12*mm, 15*mm,
             {-55*mm,12*mm,50*mm}, soft, {0.35,0.8,0.45});
    addOrgan(5, "SalivaryRight", 12*mm, 12*mm, 15*mm,
             {55*mm,12*mm,50*mm}, soft, {0.35,0.8,0.45});
    auto* tongueVis = new G4VisAttributes(G4Colour(0.95,0.35,0.45));
    tongueVis->SetForceSolid(true);
    tongueLV->SetVisAttributes(tongueVis);

    // Polyethylene catheter WALL only. Its lumen remains phantom water.
    auto* catheterSolid = new G4Tubs("Applicator", catheterInner,
                                     catheterOuter, catheterHalfLength,
                                     0*deg,360*deg);
    auto* catheterLV = new G4LogicalVolume(catheterSolid, polyethylene, "Applicator");
    new G4PVPlacement(nullptr, fApplicatorPosition, catheterLV, "Applicator",
                      phantomLV, false, 0, true);
    catheterLV->SetVisAttributes(new G4VisAttributes(G4Colour(0,1,1)));

    // Capsule is in catheter lumen and contains the active iridium core.
    auto* capsuleSolid = new G4Tubs("SourceCapsule", 0, capsuleRadius,
                                    capsuleHalfLength, 0*deg,360*deg);
    auto* capsuleLV = new G4LogicalVolume(capsuleSolid, steel, "SourceCapsule");
    new G4PVPlacement(nullptr, fSourcePosition, capsuleLV, "SourceCapsule",
                      phantomLV, false, 0, true);
    capsuleLV->SetVisAttributes(new G4VisAttributes(G4Colour(1,0.5,0)));

    auto* coreSolid = new G4Tubs("Ir192Source", 0, fSourceRadius,
                                 fSourceHalfLength, 0*deg,360*deg);
    auto* coreLV = new G4LogicalVolume(coreSolid, iridium, "Ir192Source");
    new G4PVPlacement(nullptr, {}, coreLV, "Ir192Source",
                      capsuleLV, false, 0, true);
    coreLV->SetVisAttributes(new G4VisAttributes(G4Colour(1,0,0)));

    auto* pbSolid = new G4Box("PbShield", pbThickness/2.,
                              pbHalfWidth, pbHalfLength);
    auto* pbLV = new G4LogicalVolume(pbSolid, lead, "PbShield");
    new G4PVPlacement(nullptr, pbPosition, pbLV, "PbShield",
                      phantomLV, false, 0, true);
    pbLV->SetVisAttributes(new G4VisAttributes(G4Colour(0.45,0.45,0.45)));

    G4cout << "\n=== IR-192 INTERSTITIAL TONGUE PHANTOM (SURROGATE) ===\n"
           << "Source position: " << fSourcePosition/mm << " mm\n"
           << "Pb shield position: " << pbPosition/mm << " mm\n"
           << "Scoring organs: " << kNumOrgans << "\n"
           << "Fixed Pb plate: " << pbThickness/mm << " mm thick\n"
           << "=====================================================\n";
    return worldPV;
}

} // namespace B1
