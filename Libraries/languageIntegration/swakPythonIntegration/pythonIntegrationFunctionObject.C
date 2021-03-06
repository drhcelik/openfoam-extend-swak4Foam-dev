/*---------------------------------------------------------------------------*\
|                       _    _  _     ___                       | The         |
|     _____      ____ _| | _| || |   / __\__   __ _ _ __ ___    | Swiss       |
|    / __\ \ /\ / / _` | |/ / || |_ / _\/ _ \ / _` | '_ ` _ \   | Army        |
|    \__ \\ V  V / (_| |   <|__   _/ / | (_) | (_| | | | | | |  | Knife       |
|    |___/ \_/\_/ \__,_|_|\_\  |_| \/   \___/ \__,_|_| |_| |_|  | For         |
|                                                               | OpenFOAM    |
-------------------------------------------------------------------------------
License
    This file is part of swak4Foam.

    swak4Foam is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    swak4Foam is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with swak4Foam; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

Contributors/Copyright:
    2011-2013, 2015-2018 Bernhard F.W. Gschaider <bgschaid@hfd-research.com>

 SWAK Revision: $Id$
\*---------------------------------------------------------------------------*/

#include "pythonIntegrationFunctionObject.H"
#include "addToRunTimeSelectionTable.H"

#include "polyMesh.H"
#include "IOmanip.H"
#include "swakTime.H"
#include "IFstream.H"

#include "DebugOStream.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(pythonIntegrationFunctionObject, 0);

    addToRunTimeSelectionTable
    (
        functionObject,
        pythonIntegrationFunctionObject,
        dictionary
    );

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

pythonIntegrationFunctionObject::pythonIntegrationFunctionObject
(
    const word& name,
    const Time& t,
    const dictionary& dict
)
:
    simpleFunctionObject(
        name,
        t,
        dict
    ),
    pythonInterpreterWrapper(
        t.db(),
        dict
    ),
    time_(t)
{
    Pbug << "Constructor" << endl;

    if(!parallelNoRun()) {
        initEnvironment(t);

        setInterpreter();
        PyObject *m = PyImport_AddModule("__main__");
        PyObject_SetAttrString(
            m,
            "functionObjectName",
            PyString_FromString(this->name().c_str())
        );
        releaseInterpreter();

        setRunTime();
    }

    read(dict);
}

pythonIntegrationFunctionObject::~pythonIntegrationFunctionObject()
{
}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void pythonIntegrationFunctionObject::setRunTime()
{
    Pbug << "setRunTime" << endl;

    pythonInterpreterWrapper::setRunTime(time_);
}

bool pythonIntegrationFunctionObject::start()
{
    Pbug << "start" << endl;

    simpleFunctionObject::start();

    if(!parallelNoRun()) {
        setRunTime();
    }

    return executeCode(startCode_,true);
}

void pythonIntegrationFunctionObject::writeSimple()
{
    Pbug << "writeSimple" << endl;

    if(!parallelNoRun()) {
        setRunTime();
    }

    executeCode(executeCode_,true);

    if(this->time_.outputTime()) {
        executeCode(writeCode_,true);
    }
}

bool pythonIntegrationFunctionObject::end()
{
    Pbug << "end" << endl;

    if(!parallelNoRun()) {
        setRunTime();
    }

    return executeCode(endCode_,true);
}

bool pythonIntegrationFunctionObject::read(const dictionary& dict)
{
    Pbug << "read" << endl;

    readCode(dict,"start",startCode_);
    readCode(dict,"end",endCode_);
    readCode(dict,"execute",executeCode_);
    readCode(dict,"write",writeCode_,false);

#ifdef FOAM_FUNCTIONOBJECT_HAS_SEPARATE_WRITE_METHOD_AND_NO_START
    return start();
#else
    return true;
#endif
}

} // namespace Foam

// ************************************************************************* //
