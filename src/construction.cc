#include "construction.hh"

MyDetectorConstruction::MyDetectorConstruction()
{}

MyDetectorConstruction::~MyDetectorConstruction()
{}

G4VPhysicalVolume *MyDetectorConstruction::Construct()
{
    G4NistManager *nist = G4NistManager::Instance();
    
    // World
    G4Material *worldMat = nist->FindOrBuildMaterial("G4_AIR");
    G4Box *solidWorld = new G4Box("solidWorld", 0.25*m, 0.25*m, 0.25*m);
    G4LogicalVolume *logicWorld = new G4LogicalVolume(solidWorld, worldMat, "logicWorld");
    G4VPhysicalVolume *physWorld = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "physWorld", 0, false, 0);                        // Delcares it as a physical volume

    // // Meat (soft tissue) slab
    // G4Material *softTissue = nist->FindOrBuildMaterial("G4_TISSUE_SOFT_ICRP");
    // G4Box *solidMeat = new G4Box("solidMeat", 0.1*m, 0.1*m, 0.025*m); // 20x20x5 cm
    // G4LogicalVolume *logicMeat = new G4LogicalVolume(solidMeat, softTissue, "logicMeat");
    // G4VPhysicalVolume *physMeat = new G4PVPlacement(0, G4ThreeVector(0,0,0.05*m), logicMeat, "physMeat", logicWorld, false, 0);

    //  Lung Inhale material
    // Materials
    G4Element *elH  = nist->FindOrBuildElement("H");
    G4Element *elC  = nist->FindOrBuildElement("C");
    G4Element *elN  = nist->FindOrBuildElement("N");
    G4Element *elO  = nist->FindOrBuildElement("O");
    G4Element *elNa = nist->FindOrBuildElement("Na");
    G4Element *elP  = nist->FindOrBuildElement("P");
    G4Element *elS  = nist->FindOrBuildElement("S");
    G4Element *elCl = nist->FindOrBuildElement("Cl");
    G4Element *elK  = nist->FindOrBuildElement("K");

    //Lung inhale construction
    G4Material *lunginhale = new G4Material("LungInhale", 0.217*g/cm3, 9);      // 0.217 density, 9 no. of elements
    lunginhale->AddElement(elH, 0.103);
    lunginhale->AddElement(elC, 0.105);
    lunginhale->AddElement(elN, 0.031);
    lunginhale->AddElement(elO, 0.749);
    lunginhale->AddElement(elNa, 0.002);
    lunginhale->AddElement(elP, 0.002);
    lunginhale->AddElement(elS, 0.003);
    lunginhale->AddElement(elCl, 0.002);
    lunginhale->AddElement(elK, 0.003);

    G4Box *solidMeat = new G4Box("solidMeat", 0.1*m, 0.1*m, 0.025*m); // 20x20x5 cm
    G4LogicalVolume *logicMeat = new G4LogicalVolume(solidMeat, lunginhale, "logicMeat");
    G4VPhysicalVolume *physMeat = new G4PVPlacement(0, G4ThreeVector(0,0,0.05*m), logicMeat, "physMeat", logicWorld, false, 0);

    // // Bone slab (behind meat) (ORIGINAL)
    // G4Material *bone = nist->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU");
    // G4Box *solidBone = new G4Box("solidBone", 0.03*m, 0.03*m, 0.003*m); // 6x6x0.6 cm
    // G4LogicalVolume *logicBone = new G4LogicalVolume(solidBone, bone, "logicBone");
    // G4VPhysicalVolume *physBone = new G4PVPlacement(0, G4ThreeVector(0,0,0.08*m), logicBone, "physBone", logicWorld, false, 0);


    // Three bones layout
    // ===== Bones (3 behind meat, side-by-side along x) =====
    G4Material* boneMat = nist->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU");

    // Common placement z (same as your current bone)
    const G4double zBone = 0.08*m;

    // Spacing: bone full width is 0.06 m, add 2 mm gap
    const G4double boneHalfX = 0.03*m;
    const G4double gap = 2.0*mm;
    const G4double dx = 2.0*boneHalfX + gap;   // 0.062 m

    const G4ThreeVector posBone1(-dx, 0, zBone);
    const G4ThreeVector posBone2( 0 , 0, zBone);
    const G4ThreeVector posBone3(+dx, 0, zBone);

    // ---------- Bone 1: normal rectangular ----------
    auto solidBone1 = new G4Box("solidBone1", 0.03*m, 0.03*m, 0.003*m); // 6x6x0.6 cm
    auto logicBone1 = new G4LogicalVolume(solidBone1, boneMat, "logicBone1");
    new G4PVPlacement(nullptr, posBone1, logicBone1, "physBone1", logicWorld, false, 0, true);

    // // ---------- Bone 2: hollow rectangular (shell via subtraction) ----------
    // auto solidBone2Outer = new G4Box("solidBone2Outer", 0.03*m, 0.03*m, 0.003*m);

    // // Inner cavity: leave wall thickness in x/y and keep a little z wall too
    // // (tweak these to taste; these values make a clear "hollow" but stable shell)
    // const G4double wallXY = 4.0*mm;     // wall thickness in x/y
    // const G4double wallZ  = 0.6*mm;     // wall thickness in z

    // auto solidBone2Inner = new G4Box("solidBone2Inner", 0.03*m - wallXY, 0.03*m - wallXY, 0.003*m - wallZ);

    // auto solidBone2 = new G4SubtractionSolid("solidBone2", solidBone2Outer, solidBone2Inner, nullptr, G4ThreeVector(0,0,0)); // centered cavity

    // auto logicBone2 = new G4LogicalVolume(solidBone2, boneMat, "logicBone2");
    // new G4PVPlacement(nullptr, posBone2, logicBone2, "physBone2", logicWorld, false, 0, true);

    // ---------- Bone 2: hollow rectangular with circular cavity ----------
    auto solidBone2Outer = new G4Box("solidBone2Outer", 0.03*m, 0.03*m, 0.003*m);

    // Wall thickness
    const G4double wallXY = 4.0*mm;
    const G4double wallZ  = 0.6*mm;

    // Radius of circular cavity
    const G4double cavityRadius = 0.03*m - wallXY;

    // Cylinder cavity (hole through the bone)
    auto solidBone2Inner = new G4Tubs("solidBone2Inner",0.0,cavityRadius,0.003*m - wallZ,0.0,360.0*deg);

    // Subtract cylinder from box
    auto solidBone2 = new G4SubtractionSolid("solidBone2",solidBone2Outer,solidBone2Inner,nullptr,G4ThreeVector(0,0,0));

    auto logicBone2 = new G4LogicalVolume(solidBone2, boneMat, "logicBone2");

    new G4PVPlacement(nullptr,posBone2,logicBone2,"physBone2",logicWorld,false,0,true);


    // ---------- Bone 3: Cartoon dog bone (4-lobed symmetric) ----------
    // Common thickness (6 mm total)
    const G4double hz = 3.0*mm;
    const G4double halfLenX = 23.0*mm;
    const G4double lobeRadius = 7.0*mm;

    // Central shaft
    auto shaft = new G4Box("dogShaft", halfLenX, 6.0*mm, hz);

    // Circular lobes (modeled as cylinders in z)
    auto lobe = new G4Tubs("dogLobe", 0, lobeRadius, hz, 0, 360*deg);

    auto dog1 = new G4UnionSolid("dog1", shaft, lobe, nullptr, G4ThreeVector(-halfLenX, +lobeRadius, 0));
    auto dog2 = new G4UnionSolid("dog2", dog1, lobe, nullptr, G4ThreeVector(-halfLenX, -lobeRadius, 0));
    auto dog3 = new G4UnionSolid("dog3", dog2, lobe, nullptr, G4ThreeVector(+halfLenX, +lobeRadius, 0));
    auto dogBoneSolid = new G4UnionSolid("dogBoneSolid", dog3, lobe, nullptr, G4ThreeVector(+halfLenX, -lobeRadius, 0));

    auto logicBone3 = new G4LogicalVolume(dogBoneSolid, boneMat, "logicBone3");

    new G4PVPlacement(nullptr, posBone3, logicBone3, "physBone3", logicWorld, false, 0, true);


    // Water drop behind each bone
    G4Material *tumorMat = nist->FindOrBuildMaterial("G4_WATER");

    const G4double xTumor = 4*mm;
    const G4double yTumor = 14*mm;        // was 6mm
    const G4double zTumor = 6*mm;
    const G4double tumorOffset = zTumor + 3.0*mm;

    const G4ThreeVector posTumor1 = posBone1 + G4ThreeVector(0,0,tumorOffset);
    const G4ThreeVector posTumor2 = posBone2 + G4ThreeVector(0,0,tumorOffset);
    const G4ThreeVector posTumor3 = posBone3 + G4ThreeVector(0,0,tumorOffset);


    auto solidTumor1 = new G4Ellipsoid("solidTumor1", xTumor, yTumor, zTumor);
    auto logicTumor1 = new G4LogicalVolume(solidTumor1, tumorMat, "logicTumor1");
    new G4PVPlacement(nullptr, posTumor1, logicTumor1, "physTumor1", logicWorld, false, 0, true);

    auto solidTumor2 = new G4Ellipsoid("solidTumor2", xTumor, yTumor, zTumor);
    auto logicTumor2 = new G4LogicalVolume(solidTumor2, tumorMat, "logicTumor2");
    new G4PVPlacement(nullptr, posTumor2, logicTumor2, "physTumor2", logicWorld, false, 0, true);

    auto solidTumor3 = new G4Ellipsoid("solidTumor3", xTumor, yTumor, zTumor);
    auto logicTumor3 = new G4LogicalVolume(solidTumor3, tumorMat, "logicTumor3");
    new G4PVPlacement(nullptr, posTumor3, logicTumor3, "physTumor3", logicWorld, false, 0, true);


    // ---------- Visualization ----------
    auto bone1Vis = new G4VisAttributes(G4Colour(1.0, 1.0, 0.8, 0.6));
    bone1Vis->SetForceSolid(true);
    logicBone1->SetVisAttributes(bone1Vis);

    auto bone2Vis = new G4VisAttributes(G4Colour(0.9, 0.95, 1.0, 0.6)); // slightly bluish tint
    bone2Vis->SetForceSolid(true);
    logicBone2->SetVisAttributes(bone2Vis);

    auto bone3Vis = new G4VisAttributes(G4Colour(1.0, 0.9, 0.7, 0.7)); // warmer tint
    bone3Vis->SetForceSolid(true);
    logicBone3->SetVisAttributes(bone3Vis);

    // Visualization: tumors blue
    G4VisAttributes* tumorVis = new G4VisAttributes(G4Colour(0.2, 0.4, 1.0));
    tumorVis->SetVisibility(true);
    tumorVis->SetForceSolid(true);

    logicTumor1->SetVisAttributes(tumorVis);
    logicTumor2->SetVisAttributes(tumorVis);
    logicTumor3->SetVisAttributes(tumorVis);


    // // Sensitive detector
    G4Material *detectorMat = nist->FindOrBuildMaterial("G4_Pb"); 
    // G4Box *solidDetector = new G4Box("solidDetector", 0.0025*m, 0.0025*m, 0.005*m);        // width was 0.00005*m
    // logicDetector = new G4LogicalVolume(solidDetector, detectorMat, "logicalDetector");

    // "Panel" material (simple approximation)
    // G4Material* detectorMat = nist->FindOrBuildMaterial("G4_Si");
    // Alternative if you want a scintillator-ish material:
    // G4Material* detectorMat = nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");

    // Pixel size (5 mm x 5 mm) and thickness (0.5 mm)
    const G4double pixelXY = 5.0*mm;
    const G4double pixelThick = 0.5*mm;

    G4Box* solidDetector = new G4Box("solidDetector", 0.5*pixelXY, 0.5*pixelXY, 0.5*pixelThick);
    logicDetector = new G4LogicalVolume(solidDetector, detectorMat, "logicalDetector");
    logicDetector->SetVisAttributes(G4VisAttributes::GetInvisible());


    // Collimator set up
    const G4double colPitch = pixelXY;
    const G4double colHoleXY = 4.0*mm;
    const G4double colLen = 5.0*mm;
    

    auto colMat = nist->FindOrBuildMaterial("G4_Pb");
    auto holeMat = nist->FindOrBuildMaterial("G4_AIR");

    auto solidColOuter = new G4Box("solidColOuter", 0.5*colPitch, 0.5*colPitch, 0.5*colLen);
    auto solidColInner = new G4Box("solidColInner", 0.5*colHoleXY, 0.5*colHoleXY, 0.5*colLen);
    auto solidColCell = new G4SubtractionSolid("solidColCell", solidColOuter, solidColInner, nullptr, G4ThreeVector(0,0,0));
    auto logicColCell = new G4LogicalVolume(solidColCell, colMat, "logicColCell");

    auto colVis = new G4VisAttributes(G4Colour(0.3, 0.3, 0.3, 0.5));
    colVis->SetForceSolid(true);
    logicColCell->SetVisAttributes(colVis);


    const G4int Nx = 80;
    const G4int Ny = 80;

    const G4double pitch = pixelXY;        // contiguous pixels
    const G4double zDet  = 0.240*m;        // your detector plane

    const G4double zCol = zDet - 0.5*pixelThick - 0.5*colLen;

    // Center the whole panel around (0,0)
    const G4double x0 = -0.5*Nx*pitch + 0.5*pitch;
    const G4double y0 = -0.5*Ny*pitch + 0.5*pitch;

    for (G4int iy = 0; iy < Ny; iy++)
    {
        for (G4int ix = 0; ix < Nx; ix++)
        {
            const G4double x = x0 + ix*pitch;
            const G4double y = y0 + iy*pitch;
            
            // Collimator cell
            // new G4PVPlacement(0, G4ThreeVector(x, y, zCol), logicColCell, "physColCell", logicWorld, false, ix + iy*Nx, true);

            // Detector pixel
            new G4PVPlacement(0, G4ThreeVector(x, y, zDet), logicDetector, "physDetector", logicWorld, false, ix + iy*Nx, true);
        }
    }


    // Adding colour to bone and meat
    G4VisAttributes* meatVis = new G4VisAttributes(G4Colour(1.0, 0.6, 0.6, 0.3)); // pinkish, transparent
    meatVis->SetForceSolid(true);
    logicMeat->SetVisAttributes(meatVis);

    G4VisAttributes* boneVis = new G4VisAttributes(G4Colour(1.0, 1.0, 0.8, 0.6)); // ivory, semi-transparent
    boneVis->SetForceSolid(true);
    // logicBone->SetVisAttributes(boneVis);

    return physWorld;
}

void MyDetectorConstruction::ConstructSDandField()
{
    auto* sdManager = G4SDManager::GetSDMpointer();

    auto* sensDet = new MySensitiveDetector("SensitiveDetector", 80, 80);

    sdManager->AddNewDetector(sensDet);            // register with SD manager
    logicDetector->SetSensitiveDetector(sensDet);  // attach to logical volume
}