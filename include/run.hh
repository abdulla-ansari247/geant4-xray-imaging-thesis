#ifndef RUN_HH
#define RUN_HH

#include "G4UserRunAction.hh"
#include "G4SDManager.hh"
#include "detector.hh"

class MyRunAction : public G4UserRunAction
{
public:
    MyRunAction() {}
    ~MyRunAction() {}

    virtual void EndOfRunAction(const G4Run*) override {
        auto* sdManager = G4SDManager::GetSDMpointer();
        auto* sd = sdManager->FindSensitiveDetector("SensitiveDetector");
        auto* mySD = dynamic_cast<MySensitiveDetector*>(sd);
        if (mySD) {
            mySD->EndOfRun(nullptr);  // flush file
        }
    }
};

#endif
