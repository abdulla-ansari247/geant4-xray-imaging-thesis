#include "generator.hh"

MyPrimaryGenerator::MyPrimaryGenerator()
{
    fParticleGun = new G4ParticleGun(1);
    auto* particleTable = G4ParticleTable::GetParticleTable();
    auto* gamma = particleTable->FindParticle("gamma");
    fParticleGun->SetParticleDefinition(gamma);
}

MyPrimaryGenerator::~MyPrimaryGenerator()
{
    delete fParticleGun;
}

void MyPrimaryGenerator::GeneratePrimaries(G4Event* event)
{
    // Base focal spot position (disk at z=fZsource)
    G4ThreeVector baseSrc = SampleFocalSpot();

    // Three "guns": center, up, down (shift in y)
    G4ThreeVector srcCenter = baseSrc;
    G4ThreeVector srcUp     = baseSrc + G4ThreeVector(0, +fYOffset, 0);
    G4ThreeVector srcDown   = baseSrc + G4ThreeVector(0, -fYOffset, 0);

    // const G4ThreeVector sources[3] = { srcCenter, srcUp, srcDown };
    const G4ThreeVector sources[1] = { srcCenter };


    for (const auto& src : sources)
    {
        // Energy: sample Kramers spectrum
        G4double energy = SampleKramersEnergy(); // Monoenergetic

        // Direction: aim at the subject (isocenter)
        G4ThreeVector dir = DirectionToRandomDetectorPoint(src).unit();

        // Configure gun
        fParticleGun->SetParticlePosition(src);
        fParticleGun->SetParticleMomentumDirection(dir);
        fParticleGun->SetParticleEnergy(energy);

        // Shoot (adds a primary vertex to the same event)
        fParticleGun->GeneratePrimaryVertex(event);
    }
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
    // polar method for uniform disk
    const G4double r = fSpotR * std::sqrt(G4UniformRand());
    const G4double a = 2.0 * CLHEP::pi * G4UniformRand();
    const G4double x = r * std::cos(a);
    const G4double y = r * std::sin(a);
    return G4ThreeVector(x, y, fZsource);
}

G4ThreeVector MyPrimaryGenerator::DirectionToRandomDetectorPoint(const G4ThreeVector& src) const
{
    const G4double x = (2.0 * G4UniformRand() - 1.0) * fHalfX;
    const G4double y = (2.0 * G4UniformRand() - 1.0) * fHalfY;
    const G4double z = fZdet;
    return G4ThreeVector(x - src.x(), y - src.y(), z - src.z());
}

G4ThreeVector MyPrimaryGenerator::AimAtIsocenter(const G4ThreeVector& src) const
{
    return (fIsocenter - src).unit();
}