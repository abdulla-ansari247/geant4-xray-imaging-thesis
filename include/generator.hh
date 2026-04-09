#ifndef GENERATOR_HH
#define GENERATOR_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"
#include "G4ParticleTable.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"

class MyGeneratorMessenger;

class MyPrimaryGenerator : public G4VUserPrimaryGeneratorAction
{
public:
    MyPrimaryGenerator();
    ~MyPrimaryGenerator();

    virtual void GeneratePrimaries(G4Event*);

    // Called by MyGeneratorMessenger: angle in Geant4 internal units (radians)
    void SetGunAngleY(G4double angleRad) { fGunAngleY = angleRad; }

private:
    G4ParticleGun*          fParticleGun;
    MyGeneratorMessenger*   fMessenger;

    // Geometry/config of the "tube" and detector
    const G4double fSID   =  0.15  * m;    // source-isocenter distance
    const G4double fZdet  =  0.240 * m;    // detector plane z
    const G4double fHalfX =  0.245 * m;    // half-size of active field in X (≈ 49 cm width)
    const G4double fHalfY =  0.245 * m;    // half-size of active field in Y
    const G4double fSpotR =  0.5   * mm;   // focal spot radius

    // Gun rotation angle (Y-Z plane, radians). Set via /gun/angleY.
    // +theta -> source moves up (+Y), beam tilted down; -theta -> source down, beam up.
    G4double fGunAngleY = 0.0;

    // Subject (isocenter) position
    G4ThreeVector fIsocenter = G4ThreeVector(0,0,0);

    const G4double fEmin = 15.0 * keV;     // low-energy cutoff
    const G4double fEmax = 80.0 * keV;     // tube kVp

    G4double      SampleKramersEnergy() const;
    G4ThreeVector SampleFocalSpot()     const; // tilted disk at rotated source position
    G4ThreeVector DirectionToRandomDetectorPoint(const G4ThreeVector& src) const;
};

#endif
