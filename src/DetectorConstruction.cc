//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file B1/src/DetectorConstruction.cc
/// \brief Implementation of the B1::DetectorConstruction class

#include "DetectorConstruction.hh"
#include "G4SubtractionSolid.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Trd.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

namespace B1
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();

  // Option to switch on/off checking of volumes overlaps
  //
  G4bool checkOverlaps = true;

  //
  // World
  //
    G4double world_size = 25*cm; 

    G4Material* worldMat = 
      nist ->FindOrBuildMaterial("G4_AIR");
  
  auto solidWorld = 
      new G4Box("World", world_size,world_size,world_size);
  auto logicWorld = 
      new G4LogicalVolume(solidWorld, worldMat, "World");
      
      logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());
  auto physWorld = 
      new G4PVPlacement( 
        nullptr, G4ThreeVector(),
        logicWorld, "world",
        nullptr, false,0,
        checkOverlaps);

  //
  // Timbal 
  //
    G4Material* leadMat = nist->FindOrBuildMaterial("G4_Pb");
    G4Material* detectorMat = nist->FindOrBuildMaterial("G4_Si");
  
  //
  // Lead Slab
  //
    auto solidLead = 
        new G4Box("Lead", 5*cm,5*cm,0.05*cm);
    auto logicLead = 
        new G4LogicalVolume(solidLead, leadMat, "Lead");
    new G4PVPlacement(
        nullptr, 
        G4ThreeVector(0,0,0),
        logicLead, "lead", logicWorld,
        false, 0, checkOverlaps);
  
  //
  // Detector A
  //
    auto solidDetA =
    new G4Box(
        "DetectorA",
        2*cm,
        2*cm,
        0.25*cm);

auto logicDetA =
    new G4LogicalVolume(
        solidDetA,
        detectorMat,
        "DetectorA");

new G4PVPlacement(
    nullptr,
    G4ThreeVector(0,0,2*cm),
    logicDetA,
    "DetectorA",
    logicWorld,
    false,
    0,
    checkOverlaps);
  
  //
  // Detector B
  //
    auto solidDetB =
    new G4Box(
        "DetectorB",
        5*cm,
        5*cm,
        0.5*cm);

    auto logicDetB =
        new G4LogicalVolume(
            solidDetB,
            detectorMat,
            "DetectorB");

        new G4PVPlacement(
            nullptr,
            G4ThreeVector(10*cm,0,0),
            logicDetB,
            "DetectorB",
            logicWorld,
            false,
            0,
            checkOverlaps);

            //
            // Sumber
            //

     auto solidSource =
        new G4Tubs(
            "Source",
            0,
            0.3*mm,
            1.75*mm,
            0,
            360*deg);

    auto logicSource =
        new G4LogicalVolume(
            solidSource,
            nist->FindOrBuildMaterial("G4_Ir"),
            "Source");

        new G4PVPlacement(
            nullptr,
            G4ThreeVector(0,0,-1*cm),
            logicSource,
            "Source",
            logicWorld,
            false,
            0,
            checkOverlaps);

fScoringVolume = logicDetA;
auto sourceVis = new G4VisAttributes(G4Colour::Yellow());
sourceVis->SetForceSolid(true);
logicSource->SetVisAttributes(sourceVis);

auto leadVis = new G4VisAttributes(G4Colour::Gray());
leadVis->SetForceSolid(true);
logicLead->SetVisAttributes(leadVis);

auto detAVis = new G4VisAttributes(G4Colour::Red());
detAVis->SetForceSolid(true);
logicDetA->SetVisAttributes(detAVis);

auto detBVis = new G4VisAttributes(G4Colour::Blue());
detBVis->SetForceSolid(true);
logicDetB->SetVisAttributes(detBVis);

  //
  // always return the physical World
  //
  return physWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}  // namespace B1
