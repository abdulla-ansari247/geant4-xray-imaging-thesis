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

void MyPrimaryGenerator::SetGunPositionY(G4double y)
{
    fGunPosY = y;

    // Warn if the beam aimed through the focal point no longer covers the whole detector.
    const G4double t  = (fZdet + fSID) / (fFocalPoint.z() + fSID);
    const G4double yc = y + t * (fFocalPoint.y() - y);
    if (std::abs(yc) + fDetHalf > fHalfY)
    {
        G4ExceptionDescription msg;
        msg << "Source at y = " << y / cm << " cm: beam centre hits the detector at y = "
            << yc / cm << " cm, so part of the detector gets no photons. Increase fHalfY.";
        G4Exception("MyPrimaryGenerator::SetGunPositionY", "Gun001", JustWarning, msg);
    }
}

G4ThreeVector MyPrimaryGenerator::SampleFocalSpot() const
{
    // Source moves along Y in the plane z = -fSID; at each position the tube is
    // tilted so its axis points at the imaginary focal point.
    const G4ThreeVector srcCenter(0.0, fGunPosY, -fSID);

    // Focal spot: uniform disk perpendicular to the tube axis.
    // Tube axis n lies in the Y-Z plane, so u = (1,0,0) is perpendicular to it; v = n x u.
    const G4ThreeVector n = (fFocalPoint - srcCenter).unit();
    const G4ThreeVector u(1.0, 0.0, 0.0);
    const G4ThreeVector v = n.cross(u);

    const G4double r = fSpotR * std::sqrt(G4UniformRand());
    const G4double phi = 2.0 * CLHEP::pi * G4UniformRand();

    return srcCenter + r * std::cos(phi) * u + r * std::sin(phi) * v;
}

G4ThreeVector MyPrimaryGenerator::DirectionToRandomDetectorPoint(const G4ThreeVector& src) const
{
    // The central ray passes through the imaginary focal point.
    // From focal spot src, the ray through the focal point hits the detector at (xc, yc).
    // The beam then fans out ±fHalfX/fHalfY around that central point.
    const G4double t  = (fZdet - src.z()) / (fFocalPoint.z() - src.z());
    const G4double xc = src.x() + t * (fFocalPoint.x() - src.x());
    const G4double yc = src.y() + t * (fFocalPoint.y() - src.y());

    const G4double x = xc + (2.0 * G4UniformRand() - 1.0) * fHalfX;
    const G4double y = yc + (2.0 * G4UniformRand() - 1.0) * fHalfY;
    const G4double z = fZdet;
    return G4ThreeVector(x - src.x(), y - src.y(), z - src.z());
}

