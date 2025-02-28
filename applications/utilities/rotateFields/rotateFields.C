/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2018 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
10-29-2020 Jeff Heylmun     | Specialized mapFields to rotate axisymmetric cases
-------------------------------------------------------------------------------
License
    This file is a derivative work of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Application
    rotateFields

Description
    Rotate mesh and fields from 1-D to 2-D or 2-D to 3-D. Only for
    axisymmetric cases. Mapping from parallel cases is not currently supported

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "timeSelector.H"
#include "labelVector.H"
#include "wedgeFvPatch.H"
#include "IOobjectList.H"
#include "HashSet.H"
#include "UautoPtr.H"
#include "genericFvPatchField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type>
wordList createBoundaryTypes
(
    const GeometricField<Type, fvPatchField, volMesh>& src,
    const fvMesh& targetMesh
)
{
    const typename GeometricField<Type, fvPatchField, volMesh>::Boundary&
        srcBoundary = src.boundaryField();

    HashTable<word> srcBoundaryTypes;
    forAll(srcBoundary, patchi)
    {
        if (!isA<genericFvPatchField<Type>>(srcBoundary[patchi]))
        {
            srcBoundaryTypes.insert
            (
                srcBoundary[patchi].patch().name(),
                srcBoundary[patchi].type()
            );
        }
        else
        {
            FatalErrorInFunction
                << "Unknown patch type for patch "
                << srcBoundary[patchi].patch().name()
                << " for " << src.name() << nl
                << "Perhaps the library is missing?" << nl
                << "include the necessary library with"
                << " \'libs (\"lib*.so\")\'" << nl
                << " in the controlDict" << endl
                << abort(FatalError);
        }
    }

    wordList targetBoundaryTypes
    (
        targetMesh.boundary().size(),
        calculatedFvPatchField<Type>::typeName
    );
    forAll(targetMesh.boundary(), patchi)
    {
        if (srcBoundaryTypes.found(targetMesh.boundaryMesh()[patchi].name()))
        {
            targetBoundaryTypes[patchi] =
                srcBoundaryTypes[targetMesh.boundaryMesh()[patchi].name()];
        }
    }
    return targetBoundaryTypes;
}


template<class Type>
void mapVolFields
(
    const fvMesh& sourceMesh,
    const fvMesh& targetMesh,
    const labelList& cellMap,
    const labelList& extendedCellMap,
    const IOobjectList& objects,
    const tensorField& R,
    const HashSet<word>& mapFields
)
{
    typedef GeometricField<Type, fvPatchField, volMesh> fieldType;
    IOobjectList fields = objects.lookupClass(fieldType::typeName);
    forAllIter(IOobjectList, fields, fieldIter)
    {
        IOobject fieldTargetIOobject
        (
            fieldIter()->name(),
            targetMesh.time().timeName(),
            targetMesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        );

        bool mapField = false;
        fieldType fieldSource(*fieldIter(), sourceMesh);
        autoPtr<fieldType> fieldTargetPtr;
        UautoPtr<const labelList> mapPtr;
        if (fieldTargetIOobject.typeHeaderOk<fieldType>(true))
        {
            mapField = true;
            fieldTargetPtr.set
            (
                new fieldType
                (
                    fieldTargetIOobject,
                    targetMesh
                )
            );
            mapPtr.set(&cellMap);
        }
        else if (mapFields.found(fieldTargetIOobject.name()))
        {
            mapField = true;
            fieldTargetIOobject.readOpt() = IOobject::NO_READ;
            fieldTargetPtr.set
            (
                new fieldType
                (
                    fieldTargetIOobject,
                    targetMesh,
                    dimensioned<Type>
                    (
                        "0",
                        fieldSource.dimensions(),
                        pTraits<Type>::zero
                    ),
                    createBoundaryTypes<Type>(fieldSource, targetMesh)
                )
            );
            mapPtr.set(&extendedCellMap);
        }

        if (mapField)
        {
            Info<< "    mapping " << fieldIter()->name() << endl;

            // Read fieldTarget
            fieldType& fieldTarget = fieldTargetPtr();

            const labelList& map = mapPtr();

            forAll(map, celli)
            {
                label cellj = map[celli];
                if (cellj != -1)
                {
                    Type v = fieldSource[cellj];
                    fieldTarget[celli] = transform(R[celli], v);
                }
            }
            forAll(fieldTarget.boundaryField(), patchi)
            {
                fieldTarget.boundaryFieldRef()[patchi] =
                    fieldTarget.boundaryField()[patchi].patchInternalField();
            }
            fieldTarget.write();
        }
#ifdef FULLDEBUG
        else
        {
            Info<< "    Not mapping " << fieldIter()->name() << nl
                << "         Add to \"additionalFields\" if you would "
                << "like to include it" << endl;
        }
#endif
    }
}


void mapFields
(
    const fvMesh& sourceMesh,
    const fvMesh& targetMesh,
    const labelList& cellMap,
    const labelList& extendedCellMap,
    const tensorField& R,
    const HashSet<word>& additionalFields
)
{
    IOobjectList objects(sourceMesh, sourceMesh.time().timeName());

    mapVolFields<scalar>
    (
        sourceMesh,
        targetMesh,
        cellMap,
        extendedCellMap,
        objects,
        R,
        additionalFields
    );
    mapVolFields<vector>
    (
        sourceMesh,
        targetMesh,
        cellMap,
        extendedCellMap,
        objects,
        R,
        additionalFields
    );
    mapVolFields<sphericalTensor>
    (
        sourceMesh,
        targetMesh,
        cellMap,
        extendedCellMap,
        objects,
        R,
        additionalFields
    );
    mapVolFields<symmTensor>
    (
        sourceMesh,
        targetMesh,
        cellMap,
        extendedCellMap,
        objects,
        R,
        additionalFields
    );
    mapVolFields<tensor>
    (
        sourceMesh,
        targetMesh,
        cellMap,
        extendedCellMap,
        objects,
        R,
        additionalFields
    );
}


Foam::Pair<Foam::vector> calculateAxis(const fvMesh& mesh)
{
    Pair<vector> axis(vector::one, Zero);
    vector& rAxis = axis[0];
    vector& yAxis = axis[1];

    List<vector> foundAxis;
    forAll(mesh.boundaryMesh(), patchi)
    {
        if (isA<wedgePolyPatch>(mesh.boundaryMesh()[patchi]))
        {
            const wedgePolyPatch& wp = dynamicCast<const wedgePolyPatch>
            (
                mesh.boundaryMesh()[patchi]
            );
            bool found = false;
            vector a = cmptMag(wp.axis());
            forAll(foundAxis, ai)
            {
                if (mag(a - foundAxis[ai]) < 1e-6)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                foundAxis.append(a);
                rAxis -= cmptMag(wp.centreNormal()) + a;
                yAxis += a;
            }
        }
    }
    forAll(yAxis, cmpti)
    {
        rAxis[cmpti] = min(rAxis[cmpti]*pos(rAxis[cmpti] - 1e-6), 1.0);
        yAxis[cmpti] = min(yAxis[cmpti]*pos(yAxis[cmpti] - 1e-6), 1.0);
    }
    return axis;
}


Foam::vector calculateCentre(const fvMesh& mesh)
{
    return
        gSum
        (
            cmptMultiply
            (
                mesh.C().primitiveField()*mesh.V().field(),
                calculateAxis(mesh)[1]
            )
        )/gSum(mesh.V().field());
}


void calcMapAndR
(
    const fvMesh& sourceMesh,
    const fvMesh& targetMesh,
    const scalar& maxR,
    const vector& sourceCentre,
    const vector& targetCentre,
    const vector& rotationAxis,
    const vector& rAxis,
    labelList& cellMap,
    labelList& extendedCellMap,
    tensorField& R
)
{
    label nSourceD = sourceMesh.nGeometricD();
    vector targetD(targetMesh.geometricD());

    forAll(cellMap, celli)
    {
        // Get the position on the target mesh
        vector ptTarget =
            cmptMultiply
            (
                targetMesh.cellCentres()[celli],
                targetD
            );

        // Offset from the center of the source mesh
        vector nTarget = (ptTarget - targetCentre);

        // Radius
        scalar r = mag(nTarget);

        // Offset from the source mesh center (only solved directions)
        vector nSource(Zero);
        if (nSourceD == 1)
        {
            nSource = r*rAxis;
        }
        else
        {
            scalar y = nTarget & rotationAxis;
            scalar x = mag((nTarget - y*rotationAxis));
            nSource = y*rotationAxis + x*rAxis;
        }


        // Actual point on the source mesh
        vector ptSource = nSource + sourceCentre;

        // Map from the source mesh to the target mesh
        cellMap[celli] = sourceMesh.findCell(ptSource);
        extendedCellMap[celli] = cellMap[celli];

        // Extend radius is the target point is outside of the source mesh
        if (cellMap[celli] < 0)
        {
            extendedCellMap[celli] = sourceMesh.findNearestCell(ptSource);
            if (r < maxR)
            {
                cellMap[celli] = extendedCellMap[celli];
            }
        }

        //- If mapping from a 2D case then there is only 1 rotation axis
        //  so to remove problems of rotating about the wrong axis we
        //  explicitly remove the axis direction from the directional
        //  vectors
        if (nSourceD == 2)
        {
            nTarget -= (nTarget & rotationAxis)*rotationAxis;
            nSource -= (nSource & rotationAxis)*rotationAxis;
        }

        // Normalise directions
        nTarget /= max(mag(nTarget), small);
        nSource /= max(mag(nSource), small);
        R[celli] = rotationTensor(nSource, nTarget);
    }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    //- Add options
    timeSelector::addOptions(true, false);

    argList::addNote
    (
        "map volume fields from one mesh to another"
    );
    argList::validArgs.append("sourceCase");

    argList::addOption
    (
        "sourceTime",
        "scalar|'latestTime'",
        "specify the source time"
    );

    argList::addOption
    (
        "sourceRegion",
        "word",
        "specify the source region"
    );
    argList::addOption
    (
        "targetRegion",
        "word",
        "specify the target region"
    );
//     argList::addBoolOption
//     (
//         "parallelSource",
//         "the source is decomposed"
//     );

    argList::addBoolOption
    (
        "extend",
        "Use the closest cell value if a given cell is outside of the mesh"
    );
    argList::addOption
    (
        "maxR",
        "scalar|'maximum radius'",
        "Cut off radius to map"
    );
    argList::addOption
    (
        "centre",
        "vector|'(0 0 0)'",
        "Location of center in the target mesh"
    );
    argList::addOption
    (
        "additionalFields",
        "wordList|'(rho U)'",
        "List of additional fields to map"
    );
    argList::addBoolOption
    (
        "uniform",
        "Copy uniform objects (not time)"
    );

    #include "addRegionOption.H"

    #include "setRootCase.H"
    #include "createTime.H"

    //- Select time
    runTime.functionObjects().off();
    instantList timeDirs = timeSelector::selectIfPresent(runTime, args);


    #include "createNamedMesh.H"

    fileName rootDirTarget(args.rootPath());
    fileName caseDirTarget(args.globalCaseName());

    fileName casePath = args[1];
    const fileName rootDirSource = casePath.path().toAbsolute();
    const fileName caseDirSource = casePath.name();

    Info<< "Source: " << rootDirSource << " " << caseDirSource << endl;
    word sourceRegion = fvMesh::defaultRegion;
    if (args.optionFound("sourceRegion"))
    {
        sourceRegion = args["sourceRegion"];
        Info<< "Source region: " << sourceRegion << endl;
    }

    Info<< "Target: " << rootDirTarget << " " << caseDirTarget << endl;
    word targetRegion = fvMesh::defaultRegion;
    if (args.optionFound("targetRegion"))
    {
        targetRegion = args["targetRegion"];
        Info<< "Target region: " << targetRegion << endl;
    }

//     const bool parallelSource = args.optionFound("parallelSource");

    scalar maxR(-1);
    if (args.optionFound("maxR"))
    {
        maxR = args.optionRead<scalar>("maxR");
        Info<< "Maximum distance from target centre is " << maxR << endl;
    }
    else if (args.optionFound("extend"))
    {
        maxR = great;
        Info<< "Extending mapping to the edge of the domain" << endl;
    }


    wordList additionalFieldsNames;
    if (args.optionFound("additionalFields"))
    {
        additionalFieldsNames = args.optionRead<wordList>("additionalFields");
    }
    HashSet<word> additionalFields(additionalFieldsNames);

    bool copyUniform = args.optionFound("uniform");

    fvMesh& targetMesh = mesh;
    Time& runTimeTarget = runTime;

    const string caseDirOrig = getEnv("FOAM_CASE");
    const string caseNameOrig = getEnv("FOAM_CASENAME");
    setEnv("FOAM_CASE", rootDirSource/caseDirSource, true);
    setEnv("FOAM_CASENAME", caseDirSource, true);
    Time runTimeSource
    (
        Time::controlDictName,
        rootDirSource,
        caseDirSource
    );
    setEnv("FOAM_CASE", caseDirOrig, true);
    setEnv("FOAM_CASENAME", caseNameOrig, true);

//     if (parallelSource)
//     {
//         IOdictionary decompositionDict
//         (
//             IOobject
//             (
//                 "decomposeParDict",
//                 runTimeSource.system(),
//                 runTimeSource,
//                 IOobject::MUST_READ_IF_MODIFIED,
//                 IOobject::NO_WRITE
//             )
//         );
//         label nProcs = readInt(decompositionDict.lookup("numberOfSubdomains"));
//
//         Info<< "Create target mesh\n" << endl;
//         Info<< "Target mesh size: " << meshTarget.nCells() << endl;
//
//         for (int proci=0; proci<nProcs; proci++)
//         {
//             Info<< nl << "Source processor " << proci << endl;
//             fileName parCaseDirSource
//             (
//                 caseDirSource/fileName(word("processor") + name(proci))
//             );
//             Info<<rootDirSource<<endl;
//
//             Time runTimeSource
//             (
//                 Time::controlDictName,
//                 rootDirSource,
//                 caseDirSource/fileName(word("processor") + name(proci))
//             );
//             Pout<<"time"<<endl;
//
//             #include "setTimeIndex.H"
//
//             fvMesh meshSource
//             (
//                 IOobject
//                 (
//                     sourceRegion,
//                     runTimeSource.timeName(),
//                     runTimeSource
//                 )
//             );
//             Info<< "Source mesh size: " << meshSource.nCells() << tab
//                 << "Target mesh size: " << meshTarget.nCells() << nl << endl;
//
//             Vector<label> sourceGeoD(meshSource.geometricD());
//             Vector<label> sourceSolD(meshSource.solutionD());
//             Vector<label> sourceD(0, 0, 0);
//             forAll(sourceGeoD, i)
//             {
//                 if (sourceGeoD[i] == sourceSolD[i])
//                 {
//                     sourceD[i] = 1;
//                 }
//             }
//
//             vector rotDir(0.0, 0.0, 0.0);
//             forAll(sourceD, i)
//             {
//                 if (sourceD[i] != targetD[i])
//                 {
//                     rotDir[i] = 1.0;
//                 }
//             }
//             vector solDir(0.0, 0.0, 0.0);
//             forAll(sourceD, i)
//             {
//                 if (sourceD[i] == 1 && targetD[i] == 1)
//                 {
//                     solDir[i] = 1.0;
//                 }
//             }
//
//             vector centre(sum(cmptMultiply(meshSource.cellCentres(), rotDir)));
//             centre = cmptMultiply(centre, rotDir);
//
//             // Create map
//             labelList cellMap(meshTarget.nCells(), -1);
//             vectorField n(meshTarget.nCells(), Zero);
//             forAll(cellMap, celli)
//             {
//                 vector ptTarget =
//                     cmptMultiply(meshTarget.C()[celli], solDir)
//                   + cmptMultiply(meshTarget.C()[celli], rotDir);
//                 scalar r = mag(ptTarget);
//                 vector ptSource(r*solDir + centre);
//                 cellMap[celli] = meshSource.findCell(ptSource);
//                 n[celli] =
//                     ((ptSource - centre) & (solDir + rotDir))
//                    *(solDir + rotDir);
//             }
//
//
//             mapFields(meshSource, meshTarget, cellMap, n);
//         }
//     }
//     else
    {
        Time runTimeSource
        (
            Time::controlDictName,
            rootDirSource,
            caseDirSource
        );
        #include "setTimeIndex.H"

        Info<< "Create meshes\n" << endl;

        fvMesh sourceMesh
        (
            IOobject
            (
                sourceRegion,
                runTimeSource.timeName(),
                runTimeSource
            )
        );

        vector sourceCentre(calculateCentre(sourceMesh));
        vector targetCentre(sourceCentre);
        if (args.optionFound("centre"))
        {
            targetCentre = args.optionRead<vector>("centre");
        }
        Info<< "Source centre: " << sourceCentre << nl
            << "Target centre: " << targetCentre << endl;

        Pair<vector> sourceAxis(calculateAxis(sourceMesh));
        Pair<vector> targetAxis(calculateAxis(targetMesh));
        vector rotationAxis = sourceAxis[1] - targetAxis[1];
        vector rAxis = sourceAxis[0];

        Info<< "Source mesh size: " << sourceMesh.nCells() << tab
            << "Target mesh size: " << targetMesh.nCells() << nl << endl;

        labelList cellMap(targetMesh.nCells(), -1);
        labelList extendedCellMap(targetMesh.nCells(), -1);
        tensorField R(targetMesh.nCells(), tensor::I);

        Info<< "Calulating map and rotation tensors" << endl;
        calcMapAndR
        (
            sourceMesh,
            targetMesh,
            maxR,
            sourceCentre,
            targetCentre,
            rotationAxis,
            rAxis,
            cellMap,
            extendedCellMap,
            R
        );

        // Map fields from the source mesh to the target mesh
        Info<< "Mapping field" << endl;
        mapFields
        (
            sourceMesh,
            targetMesh,
            cellMap,
            extendedCellMap,
            R,
            additionalFieldsNames
        );

        if (copyUniform)
        {
            fileName local = "uniform";
            fileName path = targetMesh.time().timePath();

            IOobjectList uniformObjects
            (
                sourceMesh,
                sourceMesh.time().timeName()/local
            );
            forAllConstIter
            (
                IOobjectList,
                uniformObjects,
                iter
            )
            {
                fileName name = iter()->name();
                if (name != "time")
                {
                    fileName srcPath = iter()->objectPath();
                    cp
                    (
                        iter()->objectPath(),
                        path/local/name
                    );
                }
            }
        }
    }

    Info<< "\nEnd\n" << endl;

    return 0;
}


// ************************************************************************* //
