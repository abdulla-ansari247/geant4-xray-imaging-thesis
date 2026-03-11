// #include <iostream>
 
// #include "G4RunManager.hh"
// #include "G4UImanager.hh"
// #include "G4VisManager.hh"
// #include "G4VisExecutive.hh"
// #include "G4UIExecutive.hh"

// #include "construction.hh"
// #include "physics.hh"
// #include "action.hh"

// int main(int argc, char** argv)
// {
//     bool visualization = true;
//     G4RunManager *runManager = new G4RunManager();

//     runManager->SetUserInitialization(new MyDetectorConstruction());
//     runManager->SetUserInitialization(new MyPhysicsList());
//     runManager->SetUserInitialization(new MyActionInitialization());

//     runManager->Initialize();

//     G4UIExecutive *ui = new G4UIExecutive(argc, argv);

//     G4VisManager *visManager = new G4VisExecutive();
//     visManager->Initialize();
//     G4UImanager *UImanager = G4UImanager::GetUIpointer();

//     // if (visualization == false)
//     // {   // Batch mode
//     //     runManager->Initialize();
//     // }
//     // else
//     // {
//     //     // Interactive mode
//     //     G4UIExecutive* ui = 0;
//     //     ui = new G4UIExecutive(argc, argv);
//     //     UImanager->ApplyCommand("/control/macroPath ../macros"); //set for your environment
//     //     UImanager->ApplyCommand("/control/execute gui.mac");
//     //     ui->SessionStart();
//     //     delete ui;
//     // }

//     // // analysis.Close();
//     // delete visManager;
//     // delete runManager;

// // }

//     UImanager->ApplyCommand("/vis/open OGLI");
//     UImanager->ApplyCommand("/vis/drawVolume");
//     UImanager->ApplyCommand("/vis/scene/add/trajectories");
//     UImanager->ApplyCommand("/tracking/storeTrajectory 1");          // ensure trajectories are kept
//     UImanager->ApplyCommand("/vis/scene/endOfEventAction accumulate");
//     UImanager->ApplyCommand("/vis/viewer/set/viewpointVector 1 1 1");
//     UImanager->ApplyCommand("/vis/viewer/set/autorefresh true");
//     // UImanager->ApplyCommand("/run/beamOn 100000");                      
//     ui->SessionStart();

//     delete visManager;
//     delete runManager;
//     delete ui;


//     return 0;
// }


#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"

#include "construction.hh"
#include "physics.hh"
#include "action.hh"

int main(int argc, char** argv)
{
    // --- Run manager
    auto* runManager = new G4RunManager();

    runManager->SetUserInitialization(new MyDetectorConstruction());
    runManager->SetUserInitialization(new MyPhysicsList());
    runManager->SetUserInitialization(new MyActionInitialization());

    auto* UImanager = G4UImanager::GetUIpointer();

    // --- Make macros discoverable whether you run from build/ or project root
    UImanager->ApplyCommand("/control/macroPath .");
    UImanager->ApplyCommand("/control/macroPath ./macro");
    UImanager->ApplyCommand("/control/macroPath ../macro");

    // --- Interactive if no macro argument was provided
    const bool interactive = (argc == 1);

    if (interactive)
    {
        auto* ui = new G4UIExecutive(argc, argv);

        auto* visManager = new G4VisExecutive();
        visManager->Initialize();

        // Execute by filename (macroPath will resolve it)
        UImanager->ApplyCommand("/control/execute gui.mac");

        ui->SessionStart();

        delete visManager;
        delete ui;
    }
    else
    {
        // Batch mode: execute macro passed as argv[1]
        // Accept "run.mac" or "macro/run.mac" etc.
        G4String macroFile = argv[1];
        UImanager->ApplyCommand(G4String("/control/execute ") + macroFile);
    }

    delete runManager;
    return 0;
}
