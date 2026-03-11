#include "physics.hh"
#include "G4EmLivermorePhysics.hh"

MyPhysicsList::MyPhysicsList()
{
    RegisterPhysics(new G4EmLivermorePhysics());
    RegisterPhysics(new G4OpticalPhysics());

    SetDefaultCutValue(0.01*mm);   // tighter cuts for low-energy EM
}


MyPhysicsList::~MyPhysicsList()
{}
