/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2021
     \\/     M anipulation  | Synthetik Applied Technologies
-------------------------------------------------------------------------------
License
    This file is derivative work of OpenFOAM.

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

SourceFiles


\*---------------------------------------------------------------------------*/

#ifndef FieldSetTypesFwd_H
#define FieldSetTypesFwd_H

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "FieldSetType.H"
#include "volFields.H"
#include "surfaceFields.H"
#include "typeInfo.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

template<class Type>
using VolFieldSetType = FieldSetType<Type, fvPatchField, volMesh>;

template<class Type>
using SurfaceFieldSetType = FieldSetType<Type, fvsPatchField, surfaceMesh>;

typedef VolFieldSetType<scalar> scalarVolFieldSetType;
typedef VolFieldSetType<vector> vectorVolFieldSetType;
typedef VolFieldSetType<symmTensor> symmTensorVolFieldSetType;
typedef VolFieldSetType<sphericalTensor> sphericalTensorVolFieldSetType;
typedef VolFieldSetType<tensor> tensorVolFieldSetType;


// typedef SurfaceFieldSetType<scalar> scalarSurfaceFieldSetType;
// typedef SurfaceFieldSetType<vector> vectorSurfaceFieldSetType;
// typedef SurfaceFieldSetType<symmTensor> symmTensorSurfaceFieldSetType;
// typedef SurfaceFieldSetType<sphericalTensor> sphericalTensorSurfaceFieldSetType;
// typedef SurfaceFieldSetType<tensor> tensorSurfaceFieldSetType;

}

#endif
// ************************************************************************* //

