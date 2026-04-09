#ifndef MYGENERATORMESSENGER_HH
#define MYGENERATORMESSENGER_HH

#include "G4UImessenger.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"

class MyPrimaryGenerator;

class MyGeneratorMessenger : public G4UImessenger
{
public:
    MyGeneratorMessenger(MyPrimaryGenerator* gen);
    ~MyGeneratorMessenger();

    void SetNewValue(G4UIcommand* cmd, G4String val) override;

private:
    MyPrimaryGenerator*        fGenerator;
    G4UIcmdWithADoubleAndUnit* fAngleCmd;
};

#endif
