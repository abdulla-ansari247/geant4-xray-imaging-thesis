#include "MyGeneratorMessenger.hh"
#include "generator.hh"

MyGeneratorMessenger::MyGeneratorMessenger(MyPrimaryGenerator* gen)
: fGenerator(gen)
{
    // /gun/ directory already created by G4ParticleGunMessenger — do NOT recreate it.

    fAngleCmd = new G4UIcmdWithADoubleAndUnit("/gun/angleY", this);
    fAngleCmd->SetGuidance("Rotate the X-ray source in the Y-Z plane.\n"
                           "  0 deg  = frontal (standard PA)\n"
                           " +30 deg = source moves up (+Y), beam angled down\n"
                           " -30 deg = source moves down (-Y), beam angled up");
    fAngleCmd->SetParameterName("angle", false);
    fAngleCmd->SetDefaultUnit("deg");
    fAngleCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

MyGeneratorMessenger::~MyGeneratorMessenger()
{
    delete fAngleCmd;
}

void MyGeneratorMessenger::SetNewValue(G4UIcommand* cmd, G4String val)
{
    if (cmd == fAngleCmd)
        fGenerator->SetGunAngleY(fAngleCmd->GetNewDoubleValue(val));
}
