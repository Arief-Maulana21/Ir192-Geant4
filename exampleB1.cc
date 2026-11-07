
#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"

// =====================================================
// PHYSICS LIST
// =====================================================

#include "QBBC.hh"
#include "G4EmLivermorePhysics.hh"
#include "G4EmParameters.hh"
#include "G4SystemOfUnits.hh"

// =====================================================
// GEANT4 CORE
// =====================================================

#include "G4RunManagerFactory.hh"
#include "G4SteppingVerbose.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4ios.hh"

using namespace B1;

int main(int argc, char** argv)
{
    // =================================================
    // 1. USER INTERFACE
    // =================================================

    G4UIExecutive* ui = nullptr;

    if (argc == 1)
    {
        ui = new G4UIExecutive(argc, argv);
    }


    // =================================================
    // 2. STEPPING VERBOSE
    // =================================================

    G4int precision = 4;

    G4SteppingVerbose::UseBestUnit(precision);


    // =================================================
    // 3. RUN MANAGER
    // =================================================

    auto runManager =
        G4RunManagerFactory::CreateRunManager(
            G4RunManagerType::Default
        );


    // =================================================
    // 4. DETECTOR CONSTRUCTION
    // =================================================

    runManager->SetUserInitialization(
        new DetectorConstruction()
    );


    // =================================================
    // 5. ELECTROMAGNETIC PHYSICS PARAMETERS
    // =================================================

    auto emParameters =
        G4EmParameters::Instance();

    // Enable atomic relaxation
    emParameters->SetFluo(true);

    // Enable Auger electron production
    emParameters->SetAuger(true);

    // Enable PIXE
    emParameters->SetPixe(true);


    // =================================================
    // 6. PHYSICS LIST
    // =================================================

    auto physicsList = new QBBC;

    // Replace default electromagnetic physics
    // with Livermore low-energy EM models.
    physicsList->ReplacePhysics(
        new G4EmLivermorePhysics()
    );

    // Initial production cut for sensitivity testing
    physicsList->SetDefaultCutValue(
        0.01 * mm
    );

    physicsList->SetVerboseLevel(1);

    runManager->SetUserInitialization(
        physicsList
    );


    // =================================================
    // 7. USER ACTION INITIALIZATION
    // =================================================

    runManager->SetUserInitialization(
        new ActionInitialization()
    );


    // =================================================
    // 8. VISUALIZATION
    // =================================================

    auto visManager =
        new G4VisExecutive(argc, argv);

    visManager->Initialize();


    // =================================================
    // 9. USER INTERFACE MANAGER
    // =================================================

    auto UImanager =
        G4UImanager::GetUIpointer();


    // =================================================
    // 10. EXECUTE MACRO OR INTERACTIVE SESSION
    // =================================================

    if (!ui)
    {
        // Batch mode
        G4String command =
            "/control/execute ";

        G4String fileName =
            argv[1];

        UImanager->ApplyCommand(
            command + fileName
        );
    }
    else
    {
        // Interactive mode
        UImanager->ApplyCommand(
            "/control/execute init_vis.mac"
        );

        ui->SessionStart();

        delete ui;
    }


    // =================================================
    // 11. CLEANUP
    // =================================================

    delete visManager;
    delete runManager;

    return 0;
}
