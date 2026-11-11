
#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4ios.hh"

namespace B1
{

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    // =====================================================
    // 1. MATERIALS
    // =====================================================

    auto nist = G4NistManager::Instance();

    auto worldMat =
        nist->FindOrBuildMaterial("G4_AIR");

    auto leadMat =
        nist->FindOrBuildMaterial("G4_Pb");

    auto siliconMat =
        nist->FindOrBuildMaterial("G4_Si");

    auto applicatorMat =
        nist->FindOrBuildMaterial("G4_POLYETHYLENE");

    auto capsuleMat =
        nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");

    auto iridiumMat =
        nist->FindOrBuildMaterial("G4_Ir");


    // =====================================================
    // 2. WORLD
    // =====================================================

    auto solidWorld = new G4Box(
        "World",
        25.0 * cm,
        25.0 * cm,
        25.0 * cm
    );

    auto logicWorld = new G4LogicalVolume(
        solidWorld,
        worldMat,
        "World"
    );

    auto physWorld = new G4PVPlacement(
        nullptr,
        G4ThreeVector(),
        logicWorld,
        "World",
        nullptr,
        false,
        0,
        true
    );


    // =====================================================
    // 3. DETECTOR A (TEMPORARY SCORING VOLUME)
    // =====================================================

    auto solidDetA = new G4Box(
        "DetectorA",
        2.0 * cm,
        2.0 * cm,
        0.25 * cm
    );

    auto logicDetA = new G4LogicalVolume(
        solidDetA,
        siliconMat,
        "DetectorA"
    );

    new G4PVPlacement(
        nullptr,
        G4ThreeVector(0., 0., 2.0 * cm),
        logicDetA,
        "DetectorA",
        logicWorld,
        false,
        0,
        true
    );


    // =====================================================
    // 4. DETECTOR B (TEMPORARY TEST DETECTOR)
    // =====================================================

    auto solidDetB = new G4Box(
        "DetectorB",
        5.0 * cm,
        5.0 * cm,
        0.5 * cm
    );

    auto logicDetB = new G4LogicalVolume(
        solidDetB,
        siliconMat,
        "DetectorB"
    );

    new G4PVPlacement(
        nullptr,
        G4ThreeVector(10.0 * cm, 0., 0.),
        logicDetB,
        "DetectorB",
        logicWorld,
        false,
        0,
        true
    );


    // =====================================================
    // 5. INTERSTITIAL APPLICATOR
    // =====================================================

    const G4double applicatorInnerRadius =
        0.60 * mm;

    const G4double applicatorOuterRadius =
        0.80 * mm;

    const G4double applicatorHalfLength =
        25.0 * mm;

    auto solidApplicator = new G4Tubs(
        "Applicator",
        applicatorInnerRadius,
        applicatorOuterRadius,
        applicatorHalfLength,
        0. * deg,
        360. * deg
    );

    auto logicApplicator = new G4LogicalVolume(
        solidApplicator,
        applicatorMat,
        "Applicator"
    );

    new G4PVPlacement(
        nullptr,
        fApplicatorPosition,
        logicApplicator,
        "Applicator",
        logicWorld,
        false,
        0,
        true
    );


    // =====================================================
    // 6. STAINLESS STEEL SOURCE CAPSULE
    // =====================================================

    const G4double capsuleRadius =
        0.45 * mm;

    const G4double capsuleHalfLength =
        1.75 * mm;

    auto solidCapsule = new G4Tubs(
        "SourceCapsule",
        0.,
        capsuleRadius,
        capsuleHalfLength,
        0. * deg,
        360. * deg
    );

    auto logicCapsule = new G4LogicalVolume(
        solidCapsule,
        capsuleMat,
        "SourceCapsule"
    );

    // The capsule occupies the hollow applicator
    // channel. Both are daughters of World.
    new G4PVPlacement(
        nullptr,
        fSourcePosition,
        logicCapsule,
        "SourceCapsule",
        logicWorld,
        false,
        0,
        true
    );


    // =====================================================
    // 7. ACTIVE IR-192 SOURCE
    // =====================================================

    auto solidSource = new G4Tubs(
        "Ir192Source",
        0.,
        fSourceRadius,
        fSourceHalfLength,
        0. * deg,
        360. * deg
    );

    auto logicSource = new G4LogicalVolume(
        solidSource,
        iridiumMat,
        "Ir192Source"
    );

    // Daughter of SourceCapsule:
    // its position is LOCAL to the capsule.
    new G4PVPlacement(
        nullptr,
        G4ThreeVector(0., 0., 0.),
        logicSource,
        "Ir192Source",
        logicCapsule,
        false,
        0,
        true
    );


    // =====================================================
    // 8. FIXED LOCALIZED Pb SHIELD
    // =====================================================

    const G4double shieldThickness =
        1.0 * mm;

    const G4double shieldHalfWidth =
        5.0 * mm;

    const G4double shieldHalfLength =
        5.0 * mm;

    const G4double shieldGap =
        0.10 * mm;

    const G4double shieldX =
        applicatorOuterRadius
        + shieldGap
        + shieldThickness / 2.0;

    // Pb shield remains centered at z = 50 mm.
    const G4ThreeVector shieldPosition =
        fApplicatorPosition
        + G4ThreeVector(shieldX, 0., 0.);

    auto solidShield = new G4Box(
        "PbShield",
        shieldThickness / 2.0,
        shieldHalfWidth,
        shieldHalfLength
    );

    auto logicShield = new G4LogicalVolume(
        solidShield,
        leadMat,
        "PbShield"
    );

    new G4PVPlacement(
        nullptr,
        shieldPosition,
        logicShield,
        "PbShield",
        logicWorld,
        false,
        0,
        true
    );


    // =====================================================
    // 9. VISUALIZATION
    // =====================================================

    auto worldVis = new G4VisAttributes();
    worldVis->SetVisibility(false);
    logicWorld->SetVisAttributes(worldVis);

    auto detAVis = new G4VisAttributes(
        G4Colour(0.0, 0.0, 1.0)
    );
    detAVis->SetVisibility(true);
    logicDetA->SetVisAttributes(detAVis);

    auto detBVis = new G4VisAttributes(
        G4Colour(0.0, 1.0, 0.0)
    );
    detBVis->SetVisibility(true);
    logicDetB->SetVisAttributes(detBVis);

    auto applicatorVis = new G4VisAttributes(
        G4Colour(0.0, 1.0, 1.0)
    );
    applicatorVis->SetVisibility(true);
    logicApplicator->SetVisAttributes(
        applicatorVis
    );

    auto capsuleVis = new G4VisAttributes(
        G4Colour(1.0, 0.5, 0.0)
    );
    capsuleVis->SetVisibility(true);
    logicCapsule->SetVisAttributes(capsuleVis);

    auto sourceVis = new G4VisAttributes(
        G4Colour(1.0, 0.0, 0.0)
    );
    sourceVis->SetVisibility(true);
    logicSource->SetVisAttributes(sourceVis);

    auto shieldVis = new G4VisAttributes(
        G4Colour(0.45, 0.45, 0.45)
    );
    shieldVis->SetVisibility(true);
    logicShield->SetVisAttributes(shieldVis);


    // =====================================================
    // 10. SCORING VOLUME
    // =====================================================

    fScoringVolume = logicDetA;


    // =====================================================
    // 11. GEOMETRY INFORMATION
    // =====================================================

    G4cout << G4endl;
    G4cout << "========================================"
           << G4endl;
    G4cout << "     IR-192 HDR SINGLE DWELL MODEL"
           << G4endl;
    G4cout << "========================================"
           << G4endl;

    G4cout << "Applicator center (mm): "
           << fApplicatorPosition / mm
           << G4endl;

    G4cout << "Source center (mm): "
           << fSourcePosition / mm
           << G4endl;

    G4cout << "Source radius (mm): "
           << fSourceRadius / mm
           << G4endl;

    G4cout << "Source half-length (mm): "
           << fSourceHalfLength / mm
           << G4endl;

    G4cout << "Pb shield center (mm): "
           << shieldPosition / mm
           << G4endl;

    G4cout << "Pb thickness (mm): "
           << shieldThickness / mm
           << G4endl;

    G4cout << "Scoring volume: DetectorA"
           << G4endl;

    G4cout << "========================================"
           << G4endl;

    return physWorld;
}

} // namespace B1
