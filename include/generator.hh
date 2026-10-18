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

    // Called by MyGeneratorMessenger: source Y position in Geant4 internal units (mm)
    void SetGunPositionY(G4double y);

private:
    G4ParticleGun*          fParticleGun;
    MyGeneratorMessenger*   fMessenger;

    // Geometry/config of the "tube" and detector
    const G4double fSID   =  0.15  * m;    // source plane sits at z = -fSID
    const G4double fZdet  =  0.240 * m;    // detector plane z
    const G4double fHalfX =  0.245 * m;    // half-size of active field in X (≈ 49 cm width)
    const G4double fHalfY =  0.245  * m;    // half-size of active field in Y (wider: the aimed beam
                                           // must still cover the whole detector at y = +-20 cm)
    const G4double fDetHalf = 0.20 * m;    // detector half-size (160 px * 2.5 mm / 2)
    const G4double fSpotR =  0.5   * mm;   // focal spot radius

    // Imaginary focal point the beam is always aimed through. It sits one Bone 3
    // thickness (15 mm) behind the back face of Bone 3: z = 80 + 7.5 + 15 = 102.5 mm.
    // Bone 3 (dog bone) is centred at x = 0, y = 0. Keep in sync with construction.cc.
    const G4ThreeVector fFocalPoint = G4ThreeVector(0.0, 0.0, 102.5 * mm);

    // Source Y position (plane z = -fSID). Set via /gun/positionY.
    // The tube tilts at each position so its axis points at fFocalPoint.
    G4double fGunPosY = 0.0;

    // Subject (isocenter) position
    G4ThreeVector fIsocenter = G4ThreeVector(0,0,0);

    const G4double fEmin = 15.0 * keV;     // low-energy cutoff
    const G4double fEmax = 80.0 * keV;     // tube kVp

    G4double      SampleKramersEnergy() const;
    G4ThreeVector SampleFocalSpot()     const; // flat disk at translated source position
    G4ThreeVector DirectionToRandomDetectorPoint(const G4ThreeVector& src) const;
};

#endif
