#include "run.hh"
#include "MyRunMessenger.hh"
#include "G4ios.hh"
#include <cstdlib>

MyRunAction::MyRunAction()
: fSendNotification(false), fMessenger(nullptr)
{
    fMessenger = new MyRunMessenger(this);
}

MyRunAction::~MyRunAction()
{
    delete fMessenger;
}

void MyRunAction::SetNotification(G4bool val)
{
    fSendNotification = val;
}

void MyRunAction::EndOfRunAction(const G4Run*)
{
    auto* sdManager = G4SDManager::GetSDMpointer();
    auto* sd = sdManager->FindSensitiveDetector("SensitiveDetector");
    auto* mySD = dynamic_cast<MySensitiveDetector*>(sd);

    if (mySD) {
        mySD->EndOfRun(nullptr);   // flush file
    }

    if (fSendNotification) {
        G4cout << "Sending notification..." << G4endl;

        int ret = system("python3 /home/abdulla-ansari/Desktop/University/Thesis/Xray_Sim/Optimize?/notifyMe.py");

        if (ret != 0) {
            G4cout << "Notification script failed." << G4endl;
        }
    }
}
