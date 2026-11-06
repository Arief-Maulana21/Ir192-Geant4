
#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
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

    auto worldMat = nist->FindOrBuildMaterial("G4_AIR");
    auto leadMat  = nist->FindOrBuildMaterial("G4_Pb");
    auto siliconMat = nist->FindOrBuildMaterial("G4_Si");

    auto applicatorMat = nist->FindOrBuildMaterial("G4_POLYETHYLENE");
    auto capsuleMat = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");

    auto iridiumMat = nist->FindOrBuildMaterial("G4_Ir");


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
    // 3. EXISTING LEAD SLAB
    // =====================================================

    auto solidLead = new G4Box(
        "LeadSlab",
        5.0 * cm,
        5.0 * cm,
        0.05 * cm
    );

    auto logicLead = new G4LogicalVolume(
        solidLead,
        leadMat,
        "LeadSlab"
    );

    new G4PVPlacement(
        nullptr,
        G4ThreeVector(0., 0., 0.),
        logicLead,
        "LeadSlab",
        logicWorld,
        false,
        0,
        true
    );


    // =====================================================
    // 4. DETECTOR A
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
    // 5. DETECTOR B
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
    // 6. APPLICATOR GEOMETRY
    // =====================================================

    // Applicator dimensions
    G4double applicatorInnerRadius = 0.60 * mm;
    G4double applicatorOuterRadius = 0.80 * mm;
    G4double applicatorHalfLength  = 25.0 * mm;

    // Applicator position in World
    G4ThreeVector applicatorPosition(
        0.,
        0.,
        5.0 * cm
    );

    // Hollow cylindrical applicator
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
        applicatorPosition,
        logicApplicator,
        "Applicator",
        logicWorld,
        false,
        0,
        true
    );


    // =====================================================
    // 7. SOURCE CAPSULE
    // =====================================================

    G4double capsuleRadius = 0.45 * mm;
    G4double capsuleHalfLength = 1.75 * mm;

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

    // Capsule is placed in the hollow channel of applicator.
    // It is a daughter of World, not a daughter of applicator.
    new G4PVPlacement(
        nullptr,
        applicatorPosition,
        logicCapsule,
        "SourceCapsule",
        logicWorld,
        false,
        0,
        true
    );


    // =====================================================
    // 8. ACTIVE IR-192 SOURCE
    // =====================================================

    G4double sourceRadius = 0.30 * mm;
    G4double sourceHalfLength = 1.50 * mm;

    auto solidSource = new G4Tubs(
        "Ir192Source",
        0.,
        sourceRadius,
        sourceHalfLength,
        0. * deg,
        360. * deg
    );

    auto logicSource = new G4LogicalVolume(
        solidSource,
        iridiumMat,
        "Ir192Source"
    );

    // Active Ir-192 is inside the source capsule.
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
    // 9. FIXED LOCALIZED LEAD SHIELDING PLATE
    // =====================================================

    // Shield dimensions
    G4double shieldThickness = 1.0 * mm;
    G4double shieldHalfWidth = 5.0 * mm;
    G4double shieldHalfLength = 5.0 * mm;

    // Small gap between applicator and Pb plate
    G4double shieldGap = 0.10 * mm;

    // Position of shield relative to applicator axis
    G4double shieldX =
        applicatorOuterRadius
        + shieldGap
        + shieldThickness / 2.0;

    // Shield is located around the central dwell position.
    G4double shieldDwellZ = 0.0 * mm;

    G4ThreeVector shieldPosition(
        shieldX,
        0.,
        shieldDwellZ
    );

    // Convert local shield position to World coordinates
    G4ThreeVector shieldWorldPosition =
        applicatorPosition + shieldPosition;

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

    // Fixed plate: placed once in World.
    // It does not move when the source changes dwell position.
    new G4PVPlacement(
        nullptr,
        shieldWorldPosition,
        logicShield,
        "PbShield",
        logicWorld,
        false,
        0,
        true
    );


    // =====================================================
    // 10. VISUALIZATION
    // =====================================================

    auto worldVis = new G4VisAttributes();
    worldVis->SetVisibility(false);
    logicWorld->SetVisAttributes(worldVis);

    auto leadVis = new G4VisAttributes(
        G4Colour(0.35, 0.35, 0.35)
    );
    leadVis->SetVisibility(true);
    logicLead->SetVisAttributes(leadVis);

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
    logicApplicator->SetVisAttributes(applicatorVis);

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
        G4Colour(0.3, 0.3, 0.3)
    );
    shieldVis->SetVisibility(true);
    logicShield->SetVisAttributes(shieldVis);


    // =====================================================
    // 11. SCORING VOLUME
    // =====================================================

    fScoringVolume = logicDetA;


    // =====================================================
    // 12. GEOMETRY INFORMATION
    // =====================================================

    G4cout << G4endl;
    G4cout << "========================================" << G4endl;
    G4cout << "       IR-192 HDR GEOMETRY" << G4endl;
    G4cout << "========================================" << G4endl;

    G4cout << "Applicator position : "
           << applicatorPosition / mm << " mm" << G4endl;

    G4cout << "Applicator inner radius : "
           << applicatorInnerRadius / mm << " mm" << G4endl;

    G4cout << "Applicator outer radius : "
           << applicatorOuterRadius / mm << " mm" << G4endl;

    G4cout << "Pb shield position : "
           << shieldWorldPosition / mm << " mm" << G4endl;

    G4cout << "Pb shield thickness : "
           << shieldThickness / mm << " mm" << G4endl;

    G4cout << "Scoring volume : DetectorA" << G4endl;

    G4cout << "========================================" << G4endl;
    G4cout << G4endl;


    return physWorld;

}

}