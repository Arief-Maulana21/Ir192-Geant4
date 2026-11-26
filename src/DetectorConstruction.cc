
#include "DetectorConstruction.hh"

// ============================================================
// GEOMETRY LIBRARIES
// ============================================================

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Ellipsoid.hh"
#include "G4EllipticalTube.hh"
#include "G4SubtractionSolid.hh"
#include "G4VSolid.hh"

// ============================================================
// GEANT4 LIBRARIES
// ============================================================

#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4ios.hh"

namespace B1 {

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    // ========================================================
    // 1. MATERIAL DEFINITION
    // ========================================================

    auto* nist = G4NistManager::Instance();

    auto* air =
        nist->FindOrBuildMaterial("G4_AIR");

    auto* water =
        nist->FindOrBuildMaterial("G4_WATER");

    auto* soft =
        nist->FindOrBuildMaterial("G4_TISSUE_SOFT_ICRP");

    auto* bone =
        nist->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU");

    auto* catheterMaterial =
        nist->FindOrBuildMaterial("G4_NYLON-6-6");

    auto* steel =
        nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");

    auto* iridium =
        nist->FindOrBuildMaterial("G4_Ir");

    auto* lead =
        nist->FindOrBuildMaterial("G4_Pb");

    const G4bool checkOverlaps = true;

    const G4double clearance = 0.01*mm;

    // ========================================================
    // 2. WORLD VOLUME
    // ========================================================

    auto* worldSolid = new G4Box(
        "World",
        25*cm,
        25*cm,
        25*cm
    );

    auto* worldLV = new G4LogicalVolume(
        worldSolid,
        air,
        "World"
    );

    auto* worldPV = new G4PVPlacement(
        nullptr,
        G4ThreeVector(),
        worldLV,
        "World",
        nullptr,
        false,
        0,
        checkOverlaps
    );

    worldLV->SetVisAttributes(
        G4VisAttributes::GetInvisible()
    );

    // ========================================================
    // 3. HEAD AND NECK PHANTOM
    //
    // Water background retained for controlled comparison.
    //
    // X: lateral
    // Y: anterior (-) / posterior (+)
    // Z: inferior (-) / superior (+)
    // ========================================================

    auto* phantomSolid = new G4Box(
        "HeadNeckPhantom",
        10*cm,
        10*cm,
        12*cm
    );

    auto* phantomLV = new G4LogicalVolume(
        phantomSolid,
        water,
        "HeadNeckPhantom"
    );

    new G4PVPlacement(
        nullptr,
        G4ThreeVector(),
        phantomLV,
        "HeadNeckPhantom",
        worldLV,
        false,
        0,
        checkOverlaps
    );

    auto* phantomVis = new G4VisAttributes(
        G4Colour(0.4, 0.7, 0.9, 0.1)
    );

    phantomVis->SetForceWireframe(true);

    phantomLV->SetVisAttributes(phantomVis);

    // ========================================================
    // 4. APPLICATOR PARAMETERS
    //
    // Nominal 6F catheter surrogate:
    // Outer diameter = 2.0 mm
    // Inner diameter = 1.5 mm (assumed)
    // Segment length = 50 mm
    // ========================================================

    const G4double catheterInner = 0.75*mm;
    const G4double catheterOuter = 1.00*mm;
    const G4double catheterHalfLength = 25.0*mm;

    // ========================================================
    // 5. mHDR-v2 SOURCE PARAMETERS
    //
    // Main dimensions based on AAPM TG-253.
    //
    // Active core diameter = 0.65 mm
    // Active core length   = 3.50 mm
    // Capsule diameter     = 0.90 mm
    // Capsule length       = 4.50 mm
    // ========================================================

    const G4double capsuleRadius = 0.45*mm;
    const G4double capsuleHalfLength = 2.25*mm;

    const G4double distalTipOffset = 0.20*mm;

    const G4double coreOffsetZ =
        capsuleHalfLength
        - distalTipOffset
        - fSourceHalfLength;

    const G4ThreeVector capsulePosition =
        fSourcePosition
        - G4ThreeVector(0, 0, coreOffsetZ);

    // Simplified steel cable segment

    const G4double cableRadius = 0.35*mm;
    const G4double cableHalfLength = 2.50*mm;

    const G4ThreeVector cablePosition =
        capsulePosition
        - G4ThreeVector(
            0,
            0,
            capsuleHalfLength + cableHalfLength
        );

    // ========================================================
    // 6. FIXED LEAD SHIELDING
    //
    // Shield position is independent of source dwell position.
    // ========================================================

    const G4double pbThickness = 1.0*mm;
    const G4double pbHalfWidth = 5.0*mm;
    const G4double pbHalfLength = 5.0*mm;
    const G4double pbGap = 0.10*mm;

    const G4double pbX =
        catheterOuter
        + pbGap
        + pbThickness/2.;

    const G4ThreeVector pbPosition =
        fApplicatorPosition
        + G4ThreeVector(pbX, 0, 0);

    // ========================================================
    // 7. ORGAN CENTERS
    //
    // Preliminary stylized anatomical configuration.
    // Coordinates are not measured from ICRP voxel data.
    // ========================================================

    const G4ThreeVector tonguePosition =
        fApplicatorPosition;

    const G4ThreeVector oralPosition =
        tonguePosition + G4ThreeVector(
            0,
            -10*mm,
            0
        );

    const G4ThreeVector mandiblePosition =
        tonguePosition + G4ThreeVector(
            0,
            0,
            -10*mm
        );

    const G4ThreeVector spinalPosition =
        tonguePosition + G4ThreeVector(
            0,
            55*mm,
            -25*mm
        );

    const G4ThreeVector salivaryLeftPosition =
        tonguePosition + G4ThreeVector(
            -38*mm,
            18*mm,
            -25*mm
        );

    const G4ThreeVector salivaryRightPosition =
        tonguePosition + G4ThreeVector(
            38*mm,
            18*mm,
            -25*mm
        );

    // ========================================================
    // 8. ORGAN PLACEMENT HELPER
    //
    // Accepts any G4VSolid, not only G4Box.
    // Preserves organ ID and logical volume name.
    // ========================================================

    auto addOrgan = [&](
        G4int id,
        const G4String& name,
        G4VSolid* solid,
        const G4ThreeVector& center,
        G4Material* material,
        const G4Colour& colour
    ) -> G4LogicalVolume*
    {
        auto* lv = new G4LogicalVolume(
            solid,
            material,
            name
        );

        new G4PVPlacement(
            nullptr,
            center,
            lv,
            name,
            phantomLV,
            false,
            0,
            checkOverlaps
        );

        auto* vis = new G4VisAttributes(colour);

        vis->SetForceSolid(true);

        lv->SetVisAttributes(vis);

        fOrganVolumes[id] = lv;

        return lv;
    };

    // ========================================================
    // 9. TONGUE
    //
    // Ellipsoid:
    // X diameter = 50 mm
    // Y diameter = 84 mm
    // Z diameter = 32 mm
    //
    // Approximate adult tongue mass: 72.5 g before cuts.
    // ========================================================

    const G4double tongueX = 25*mm;
    const G4double tongueY = 42*mm;
    const G4double tongueZ = 16*mm;

    auto* tongueBase = new G4Ellipsoid(
        "TongueBase",
        tongueX,
        tongueY,
        tongueZ
    );

    // Clearance for the entire catheter, including lumen.

    auto* catheterCut = new G4Tubs(
        "CatheterClearance",
        0,
        catheterOuter + clearance,
        catheterHalfLength + clearance,
        0*deg,
        360*deg
    );

    auto* tongueWithoutCatheter =
        new G4SubtractionSolid(
            "TongueWithoutCatheter",
            tongueBase,
            catheterCut,
            nullptr,
            fApplicatorPosition - tonguePosition
        );

    // Clearance for the fixed Pb plate.

    auto* shieldCut = new G4Box(
        "ShieldClearance",
        pbThickness/2. + clearance,
        pbHalfWidth + clearance,
        pbHalfLength + clearance
    );

    auto* tongueSolid = new G4SubtractionSolid(
        "Tongue",
        tongueWithoutCatheter,
        shieldCut,
        nullptr,
        pbPosition - tonguePosition
    );

    auto* tongueLV = addOrgan(
        0,
        "Tongue",
        tongueSolid,
        tonguePosition,
        soft,
        G4Colour(0.95, 0.35, 0.45)
    );

    // Legacy scoring volume retained.

    fScoringVolume = tongueLV;

    // ========================================================
    // 10. MANDIBLE
    //
    // Simplified U-shaped mandibular body.
    //
    // Inner radius = 45 mm
    // Outer radius = 55 mm
    // Height       = 25 mm
    //
    // Angular range 180-360 deg:
    // anterior half of XY plane (negative Y).
    //
    // Ascending rami and teeth are not modeled.
    // ========================================================

    const G4double mandibleInnerRadius = 45*mm;
    const G4double mandibleOuterRadius = 55*mm;
    const G4double mandibleHalfHeight = 12.5*mm;

    auto* mandibleSolid = new G4Tubs(
        "MandibleBody",
        mandibleInnerRadius,
        mandibleOuterRadius,
        mandibleHalfHeight,
        180*deg,
        180*deg
    );

    addOrgan(
        2,
        "Mandible",
        mandibleSolid,
        mandiblePosition,
        bone,
        G4Colour(0.95, 0.95, 0.85)
    );

    // ========================================================
    // 11. CERVICAL SPINAL CORD
    //
    // Elliptical cylinder:
    // Transverse diameter = 12 mm
    // AP diameter         = 8 mm
    // Segment length      = 100 mm
    // ========================================================

    auto* spinalSolid = new G4EllipticalTube(
        "SpinalCordSolid",
        6*mm,
        4*mm,
        50*mm
    );

    addOrgan(
        3,
        "SpinalCord",
        spinalSolid,
        spinalPosition,
        soft,
        G4Colour(0.55, 0.55, 0.95)
    );

    // ========================================================
    // 12. SUBMANDIBULAR SALIVARY GLANDS
    //
    // Ellipsoid dimensions:
    // X = 20 mm
    // Y = 40 mm
    // Z = 30 mm
    //
    // Existing scoring names retained.
    // ========================================================

    auto* salivarySolid = new G4Ellipsoid(
        "SubmandibularSolid",
        10*mm,
        20*mm,
        15*mm
    );

    addOrgan(
        4,
        "SalivaryLeft",
        salivarySolid,
        salivaryLeftPosition,
        soft,
        G4Colour(0.35, 0.8, 0.45)
    );

    addOrgan(
        5,
        "SalivaryRight",
        salivarySolid,
        salivaryRightPosition,
        soft,
        G4Colour(0.35, 0.8, 0.45)
    );

    // ========================================================
    // 13. ORAL CAVITY MUCOSA SURROGATE
    //
    // Ellipsoidal shell:
    // Outer semi-axes = (32, 34, 22) mm
    // Inner semi-axes = (30, 32, 20) mm
    //
    // Nominal axial wall thickness = 2 mm.
    //
    // The scored volume represents soft tissue,
    // not the air inside the oral cavity.
    // ========================================================

    auto* oralOuter = new G4Ellipsoid(
        "OralOuter",
        32*mm,
        34*mm,
        22*mm
    );

    auto* oralInner = new G4Ellipsoid(
        "OralInner",
        30*mm,
        32*mm,
        20*mm
    );

    auto* oralShell = new G4SubtractionSolid(
        "OralShell",
        oralOuter,
        oralInner
    );

    // Remove tongue volume from oral mucosa.
    // Use the uncut tongue envelope to avoid intersections.

    auto* tongueClearance = new G4Ellipsoid(
        "TongueClearance",
        tongueX + clearance,
        tongueY + clearance,
        tongueZ + clearance
    );

    auto* oralWithoutTongue =
        new G4SubtractionSolid(
            "OralWithoutTongue",
            oralShell,
            tongueClearance,
            nullptr,
            tonguePosition - oralPosition
        );

    // Remove catheter and shield clearances.

    auto* oralWithoutCatheter =
        new G4SubtractionSolid(
            "OralWithoutCatheter",
            oralWithoutTongue,
            catheterCut,
            nullptr,
            fApplicatorPosition - oralPosition
        );

    auto* oralWithoutShield =
        new G4SubtractionSolid(
            "OralWithoutShield",
            oralWithoutCatheter,
            shieldCut,
            nullptr,
            pbPosition - oralPosition
        );

    // Additional gland clearance prevents intersections
    // between the oral mucosa and salivary gland volumes.

    auto* glandClearance = new G4Ellipsoid(
        "GlandClearance",
        10*mm + clearance,
        20*mm + clearance,
        15*mm + clearance
    );

    auto* oralWithoutLeftGland =
        new G4SubtractionSolid(
            "OralWithoutLeftGland",
            oralWithoutShield,
            glandClearance,
            nullptr,
            salivaryLeftPosition - oralPosition
        );

    auto* oralSolid = new G4SubtractionSolid(
        "OralCavitySolid",
        oralWithoutLeftGland,
        glandClearance,
        nullptr,
        salivaryRightPosition - oralPosition
    );

    addOrgan(
        1,
        "OralCavity",
        oralSolid,
        oralPosition,
        soft,
        G4Colour(0.95, 0.75, 0.35, 0.5)
    );

    // ========================================================
    // 14. INTERSTITIAL APPLICATOR
    //
    // Catheter wall only.
    // The lumen remains phantom water.
    // ========================================================

    auto* catheterSolid = new G4Tubs(
        "Applicator",
        catheterInner,
        catheterOuter,
        catheterHalfLength,
        0*deg,
        360*deg
    );

    auto* catheterLV = new G4LogicalVolume(
        catheterSolid,
        catheterMaterial,
        "Applicator"
    );

    new G4PVPlacement(
        nullptr,
        fApplicatorPosition,
        catheterLV,
        "Applicator",
        phantomLV,
        false,
        0,
        checkOverlaps
    );

    catheterLV->SetVisAttributes(
        new G4VisAttributes(
            G4Colour(0, 1, 1)
        )
    );

    // ========================================================
    // 15. STAINLESS STEEL SOURCE CAPSULE
    // ========================================================

    auto* capsuleSolid = new G4Tubs(
        "SourceCapsule",
        0,
        capsuleRadius,
        capsuleHalfLength,
        0*deg,
        360*deg
    );

    auto* capsuleLV = new G4LogicalVolume(
        capsuleSolid,
        steel,
        "SourceCapsule"
    );

    new G4PVPlacement(
        nullptr,
        capsulePosition,
        capsuleLV,
        "SourceCapsule",
        phantomLV,
        false,
        0,
        checkOverlaps
    );

    capsuleLV->SetVisAttributes(
        new G4VisAttributes(
            G4Colour(1, 0.5, 0)
        )
    );

    // ========================================================
    // 16. ACTIVE IRIDIUM-192 CORE
    // ========================================================

    auto* coreSolid = new G4Tubs(
        "Ir192Source",
        0,
        fSourceRadius,
        fSourceHalfLength,
        0*deg,
        360*deg
    );

    auto* coreLV = new G4LogicalVolume(
        coreSolid,
        iridium,
        "Ir192Source"
    );

    new G4PVPlacement(
        nullptr,
        G4ThreeVector(0, 0, coreOffsetZ),
        coreLV,
        "Ir192Source",
        capsuleLV,
        false,
        0,
        checkOverlaps
    );

    coreLV->SetVisAttributes(
        new G4VisAttributes(
            G4Colour(1, 0, 0)
        )
    );

    // ========================================================
    // 17. SOURCE CABLE
    // ========================================================

    auto* cableSolid = new G4Tubs(
        "SourceCable",
        0,
        cableRadius,
        cableHalfLength,
        0*deg,
        360*deg
    );

    auto* cableLV = new G4LogicalVolume(
        cableSolid,
        steel,
        "SourceCable"
    );

    new G4PVPlacement(
        nullptr,
        cablePosition,
        cableLV,
        "SourceCable",
        phantomLV,
        false,
        0,
        checkOverlaps
    );

    cableLV->SetVisAttributes(
        new G4VisAttributes(
            G4Colour(0.6, 0.6, 0.6)
        )
    );

    // ========================================================
    // 18. FIXED Pb SHIELD
    // ========================================================

    auto* pbSolid = new G4Box(
        "PbShield",
        pbThickness/2.,
        pbHalfWidth,
        pbHalfLength
    );

    auto* pbLV = new G4LogicalVolume(
        pbSolid,
        lead,
        "PbShield"
    );

    new G4PVPlacement(
        nullptr,
        pbPosition,
        pbLV,
        "PbShield",
        phantomLV,
        false,
        0,
        checkOverlaps
    );

    pbLV->SetVisAttributes(
        new G4VisAttributes(
            G4Colour(0.45, 0.45, 0.45)
        )
    );

    // ========================================================
    // 19. GEOMETRY INFORMATION
    // ========================================================

    G4cout
        << "\n"
        << "=============================================\n"
        << " STYLIZED HEAD AND NECK PHANTOM\n"
        << " HDR Ir-192 INTERSTITIAL BRACHYTHERAPY\n"
        << "=============================================\n"

        << "Active core position: "
        << fSourcePosition/mm << " mm\n"

        << "Capsule position: "
        << capsulePosition/mm << " mm\n"

        << "Cable position: "
        << cablePosition/mm << " mm\n"

        << "Applicator position: "
        << fApplicatorPosition/mm << " mm\n"

        << "Pb shield position: "
        << pbPosition/mm << " mm\n"

        << "Ir density (g/cm3): "
        << iridium->GetDensity()/(g/cm3) << "\n"

        << "Fixed Pb thickness: "
        << pbThickness/mm << " mm\n"

        << "Scoring organs: "
        << kNumOrgans << "\n"

        << "=============================================\n";

    // ========================================================
    // 20. ORGAN MASS AND DISTANCE INFORMATION
    // ========================================================

    const G4ThreeVector organPositions[] = {
        tonguePosition,
        oralPosition,
        mandiblePosition,
        spinalPosition,
        salivaryLeftPosition,
        salivaryRightPosition
    };

    G4cout << "\n=== ORGAN GEOMETRY INFORMATION ===\n";

    for (G4int i = 0; i < kNumOrgans; ++i)
    {
        auto* lv = fOrganVolumes[i];

        if (!lv) continue;

        const G4double volume =
            lv->GetSolid()->GetCubicVolume();

        const G4double density =
            lv->GetMaterial()->GetDensity();

        const G4double mass =
            volume*density;

        const G4double centerDistance =
            (organPositions[i] - fSourcePosition).mag();

        G4cout
            << "Organ ID: " << i << "\n"

            << "Name: "
            << lv->GetName() << "\n"

            << "Center (mm): "
            << organPositions[i]/mm << "\n"

            << "Center distance (mm): "
            << centerDistance/mm << "\n"

            << "Volume (cm3): "
            << volume/cm3 << "\n"

            << "Density (g/cm3): "
            << density/(g/cm3) << "\n"

            << lv->GetName()
            << " mass_g=" << mass/g << "\n"

            << "----------------------------------\n";
    }

    G4cout
        << "Mandible minimum radial distance "
        << "at central dwell: "
        << mandibleInnerRadius/mm << " mm\n"

        << "Spinal cord anterior surface distance "
        << "at central dwell: "
        << (55*mm - 4*mm)/mm << " mm\n"

        << "=============================================\n";

    return worldPV;
}

} // namespace B1
