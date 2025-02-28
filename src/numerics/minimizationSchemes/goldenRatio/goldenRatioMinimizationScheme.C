/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2021
     \\/     M anipulation  | Synthetik Applied Technologies
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

\*---------------------------------------------------------------------------*/

#include "goldenRatioMinimizationScheme.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(goldenRatioMinimizationScheme, 0);
    addToRunTimeSelectionTable
    (
        minimizationScheme,
        goldenRatioMinimizationScheme,
        dictionaryZero
    );
    addToRunTimeSelectionTable
    (
        minimizationScheme,
        goldenRatioMinimizationScheme,
        dictionaryOne
    );
    addToRunTimeSelectionTable
    (
        minimizationScheme,
        goldenRatioMinimizationScheme,
        dictionaryTwo
    );
}

const Foam::scalar Foam::goldenRatioMinimizationScheme::goldenRatio =
    (sqrt(5.0) + 1.0)/2.0;

const Foam::scalar Foam::goldenRatioMinimizationScheme::invPhi =
    (sqrt(5.0) - 1.0)/2.0;

const Foam::scalar Foam::goldenRatioMinimizationScheme::invPhi2 =
    (3.0 - sqrt(5.0))/2.0;

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::goldenRatioMinimizationScheme::goldenRatioMinimizationScheme
(
    const scalarEquation& eqn,
    const dictionary& dict
)
:
    minimizationScheme(eqn, dict)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::scalar Foam::goldenRatioMinimizationScheme::solve
(
    const scalar x,
    const scalar x1,
    const scalar x2,
    const label li
) const
{
    scalar a = min(x1, x2);
    scalar b = max(x1, x2);
    scalar h = b - a;
    if (h < tolerance_)
    {
        return x1;
    }
    label n = ceil(log(tolerance_/h)/log(invPhi));

    scalar c = a + invPhi2*h;
    scalar d = a + invPhi*h;

    scalar yc = eqn_.f(c, li);
    scalar yd = eqn_.f(d, li);

    for (stepi_ = 0; stepi_ < n; stepi_++)
    {
        if (yc < yd)
        {
            b = d;
            d = c;
            yd = yc;
            h = invPhi*h;
            c = a + invPhi2*h;
            yc = eqn_.f(c, li);
        }
        else
        {
            a = c;
            c = d;
            yc = yd;
            h = invPhi*h;
            d = a + invPhi*h;
            yd = eqn_.f(d, li);
        }
        converged(b - a);
    }
    return 0.5*(a + b);
}

// ************************************************************************* //
