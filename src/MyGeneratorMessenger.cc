#include "MyGeneratorMessenger.hh"
#include "generator.hh"

MyGeneratorMessenger::MyGeneratorMessenger(MyPrimaryGenerator* gen)
: fGenerator(gen)
{
    // /gun/ directory already created by G4ParticleGunMessenger — do NOT recreate it.

    fPositionCmd = new G4UIcmdWithADoubleAndUnit("/gun/positionY", this);
    fPositionCmd->SetGuidance("Move the X-ray source along Y (source plane z = -15 cm).\n"
                              "At each position the tube tilts to point at the focal point.\n"
                              "  0 cm   = frontal (standard PA)\n"
                              " +20 cm = source up (+Y), beam angled down\n"
                              " -20 cm = source down (-Y), beam angled up");
    fPositionCmd->SetParameterName("y", false);
    fPositionCmd->SetDefaultUnit("cm");
    fPositionCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

MyGeneratorMessenger::~MyGeneratorMessenger()
{
    delete fPositionCmd;
}

void MyGeneratorMessenger::SetNewValue(G4UIcommand* cmd, G4String val)
{
    if (cmd == fPositionCmd)
        fGenerator->SetGunPositionY(fPositionCmd->GetNewDoubleValue(val));
}
