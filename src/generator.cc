#include "generator.hh"
#include "MyGeneratorMessenger.hh"

MyPrimaryGenerator::MyPrimaryGenerator()
{
    fParticleGun = new G4ParticleGun(1);
    auto* particleTable = G4ParticleTable::GetParticleTable();
    auto* gamma = particleTable->FindParticle("gamma");
    fParticleGun->SetParticleDefinition(gamma);

    fMessenger = new MyGeneratorMessenger(this);
}

MyPrimaryGenerator::~MyPrimaryGenerator()
{
    delete fMessenger;
    delete fParticleGun;
}

void MyPrimaryGenerator::GeneratePrimaries(G4Event* event)
{
    // Sample focal spot position on the tilted disk at the current gun angle.
    const G4ThreeVector src = SampleFocalSpot();

    G4double energy = SampleKramersEnergy();
    G4ThreeVector dir = DirectionToRandomDetectorPoint(src).unit();

    fParticleGun->SetParticlePosition(src);
    fParticleGun->SetParticleMomentumDirection(dir);
    fParticleGun->SetParticleEnergy(energy);
    fParticleGun->GeneratePrimaryVertex(event);
}


G4double MyPrimaryGenerator::SampleKramersEnergy() const
{
    const G4double Emin = fEmin;
    const G4double Emax = fEmax;

    G4double Epeak = 0.5 * Emax;
    if (Epeak < Emin) Epeak = Emin;
    const G4double fmax = Epeak * (Emax - Epeak);

    while (true)
    {
        const G4double E  = Emin + G4UniformRand() * (Emax - Emin);
        const G4double f  = E * (Emax - E);
        const G4double u  = G4UniformRand() * fmax;
        if (u <= f) return E;
    }
}

G4ThreeVector MyPrimaryGenerator::SampleFocalSpot() const
{
    // Source centre on an arc of radius fSID, rotated by fGunAngleY around x-axis.
    //   +angle -> source moves to +Y, beam aimed downward toward isocenter.
    //   -angle -> source moves to -Y, beam aimed upward.
    const G4double sinA = std::sin(fGunAngleY);
    const G4double cosA = std::cos(fGunAngleY);

    const G4ThreeVector srcCenter(0.0, fSID * sinA, -fSID * cosA);

    // Focal spot: uniform disk perpendicular to the beam axis.
    // Beam axis (toward isocenter): n = (0, -sinA, cosA)
    // In-plane basis:  u = (1, 0, 0),  v = n x u = (0, cosA, sinA)
    const G4double r = fSpotR * std::sqrt(G4UniformRand());
    const G4double phi = 2.0 * CLHEP::pi * G4UniformRand();

    const G4double du = r * std::cos(phi);   // along u = (1,0,0)
    const G4double dv = r * std::sin(phi);   // along v = (0, cosA, sinA)

    return G4ThreeVector(
        srcCenter.x() + du,
        srcCenter.y() + dv * cosA,
        srcCenter.z() + dv * sinA
    );
}

G4ThreeVector MyPrimaryGenerator::DirectionToRandomDetectorPoint(const G4ThreeVector& src) const
{
    // The central ray passes through the isocenter (0,0,0).
    // From focal spot src, the ray through the isocenter hits the detector at (xc, yc).
    // The beam then fans out ±fHalfX/fHalfY around that central point.
    const G4double xc = src.x() * fZdet / src.z();
    const G4double yc = src.y() * fZdet / src.z();

    const G4double x = xc + (2.0 * G4UniformRand() - 1.0) * fHalfX;
    const G4double y = yc + (2.0 * G4UniformRand() - 1.0) * fHalfY;
    const G4double z = fZdet;
    return G4ThreeVector(x - src.x(), y - src.y(), z - src.z());
}

