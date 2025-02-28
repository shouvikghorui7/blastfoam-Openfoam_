/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2014-2020 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

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

\*---------------------------------------------------------------------------*/

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

inline const Foam::fvMesh& Foam::phaseSystem::mesh() const
{
    return mesh_;
}


inline const Foam::phaseSystem::phaseModelList&
Foam::phaseSystem::phases() const
{
    return phaseModels_;
}


inline Foam::phaseSystem::phaseModelList&
Foam::phaseSystem::phases()
{
    return phaseModels_;
}


inline const Foam::phaseSystem::phaseModelList&
Foam::phaseSystem::movingPhases() const
{
    return phaseModels_;
}


inline const Foam::phaseSystem::phasePairTable&
Foam::phaseSystem::phasePairs() const
{
    return phasePairs_;
}


inline const Foam::phaseModel&
Foam::phaseSystem::otherPhase(const phaseModel& phase) const
{
    if (phaseModels_.size() != 2)
    {
        FatalErrorInFunction
            << "Call from a two-phase model in a multi-phase system."
            << exit(FatalError);
    }

    if (&phase == &phaseModels_[0])
    {
        return phaseModels_[1];
    }
    else
    {
        return phaseModels_[0];
    }
}


inline const Foam::fvModels& Foam::phaseSystem::models() const
{
    return fvModelsPtr_();
}

inline const Foam::fvConstraints& Foam::phaseSystem::constraints() const
{
    return fvConstraintsPtr_();
}


// ************************************************************************* //
