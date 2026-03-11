#ifndef GENERATOR_HH
#define GENERATOR_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"
#include "G4ParticleTable.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"

class MyPrimaryGenerator : public G4VUserPrimaryGeneratorAction
{
public:
    MyPrimaryGenerator();
    ~MyPrimaryGenerator();

    virtual void GeneratePrimaries(G4Event*);

private:
    G4ParticleGun* fParticleGun;

    // Geometry/config of the "tube" and detector
    const G4double fZsource = -0.15 * m;
    const G4double fZdet    =  0.240 * m;   // detector plane 
    const G4double fHalfX   =  0.245 * m;   // half-size of active field in X (≈ 49 cm width)
    const G4double fHalfY   =  0.245 * m;   // half-size of active field in Y
    const G4double fSpotR   =  0.5   * mm;  // focal spot radius
    // Separation for the extra guns
    const G4double fYOffset = 2.0*cm;   // up/down separation (edit as needed)
    const G4double fGunAngle = 10.0 * deg;   // choose your angle


    // Subject (isocenter) position
    G4ThreeVector fIsocenter = G4ThreeVector(0,0,0);

    G4ThreeVector AimAtIsocenter(const G4ThreeVector& src) const;


    const G4double fEmin = 15.0 * keV;     // low-energy cutoff
    const G4double fEmax = 80.0 * keV;     // tube kVp

    G4double SampleKramersEnergy() const;  // I(E) ∝ E*(Emax - E)
    G4ThreeVector SampleFocalSpot() const; // uniform disk at z=fZsource
    G4ThreeVector DirectionToRandomDetectorPoint(const G4ThreeVector& src) const; // aim to (x,y,z_det)
};

#endif
