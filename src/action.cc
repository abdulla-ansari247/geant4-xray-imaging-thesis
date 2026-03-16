#include "action.hh"
#include "generator.hh"
#include "run.hh"

MyActionInitialization::MyActionInitialization()
{}

MyActionInitialization::~MyActionInitialization()
{}

void MyActionInitialization::BuildForMaster() const
{
    SetUserAction(new MyRunAction());
}

void MyActionInitialization::Build() const
{
    MyPrimaryGenerator* generator = new MyPrimaryGenerator();
    SetUserAction(generator);

    SetUserAction(new MyRunAction());
}
