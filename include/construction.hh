#ifndef CONSTRUCTION_HH
#define CONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4SDManager.hh"
#include "G4SubtractionSolid.hh"
#include "G4Ellipsoid.hh"
#include "G4UnionSolid.hh"
#include "G4Tubs.hh"
#include "G4Orb.hh"

#include "detector.hh"


class MyDetectorConstruction : public G4VUserDetectorConstruction
{
public:
    MyDetectorConstruction();
    ~MyDetectorConstruction();

    virtual G4VPhysicalVolume* Construct();

private:
    G4LogicalVolume *logicDetector;
    virtual void ConstructSDandField();
};

#endif