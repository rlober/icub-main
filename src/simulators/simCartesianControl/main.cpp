/* 
 * Copyright (C) 2010 RobotCub Consortium, European Commission FP6 Project IST-004370
 * Author: Ugo Pattacini
 * email:  ugo.pattacini@iit.it
 * website: www.robotcub.org
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

/** 
\defgroup simCartesianControl simCartesianControl
 
@ingroup icub_module  
 
A simple module that makes the Cartesian Interface available 
with the robot simulator. 
 
Copyright (C) 2010 RobotCub Consortium
 
Author: Ugo Pattacini 

CopyPolicy: Released under the terms of the GNU GPL v2.0.

\section intro_sec Description
 
This module allows to integrate the Cartesian Interface with the
the robot simulator. 
 
\section lib_sec Dependencies
- YARP libraries. 
- The \ref iKin library. 
- The \ref servercartesiancontroller module. 
 
\section usage_sec Usage 
Follow this steps: 
-# Launch the \ref icub_Simulation "iCub Simulator".
-# Launch the \ref simCartesianControl module.
-# Launch the \ref iKinCartesianSolver "Cartesian Solvers" for 
   the required limbs: have a look to the template located in
   the directory <i> $ICUB_ROOT/app/simCartesianControl/scripts
   </i>.

\section parameters_sec Parameters
--robot \e name 
- specifies the simulated robot name to connect to. 
 
Other options are available but their default values should be 
fine for normal use. If you are really curious then get into the
short code :) 

\section tested_os_sec Tested OS
Windows, Linux

\author Ugo Pattacini
*/ 

#include <yarp/os/Network.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/ResourceFinder.h>
#include <yarp/dev/Wrapper.h>
#include <yarp/dev/Drivers.h>
#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/CartesianControl.h>

#include <iostream>
#include <iomanip>
#include <string>

YARP_DECLARE_DEVICES(icubmod)

using namespace std;
using namespace yarp::os;
using namespace yarp::dev;


class SimCartCtrlModule: public RFModule
{
protected:
    PolyDriver *torso;
    PolyDriver *armR,    *armL;
    PolyDriver *serverR, *serverL;

public:
    virtual bool configure(ResourceFinder &rf)
    {   
        torso=NULL;
        armR=NULL;
        armL=NULL;
        serverR=NULL;
        serverL=NULL;

        Property optTorso("(device remote_controlboard)");
        Property optArmR("(device remote_controlboard)"); 
        Property optArmL("(device remote_controlboard)"); 

        string robot=rf.find("robot").asString().c_str();
        optTorso.put("remote",("/"+robot+"/torso").c_str());
        optArmR.put("remote",("/"+robot+"/right_arm").c_str());
        optArmL.put("remote",("/"+robot+"/left_arm").c_str());

        string local=rf.find("local").asString().c_str();
        optTorso.put("local",("/"+local+"/torso").c_str());
        optArmR.put("local",("/"+local+"/right_arm").c_str());
        optArmL.put("local",("/"+local+"/left_arm").c_str());

        torso=new PolyDriver(optTorso);
        armR =new PolyDriver(optArmR);
        armL =new PolyDriver(optArmL);

        if (!torso->isValid() || !armR->isValid() || !armL->isValid())
        {
            cout<<"Device drivers not available!"<<endl;
            close();

            return false;
        }

        PolyDriverList listR, listL;
        listR.push(torso,"torso");
        listR.push(armR,"right_arm");
        listL.push(torso,"torso");
        listL.push(armL,"left_arm");

        Property optServerR("(device cartesiancontrollerserver)");
        Property optServerL("(device cartesiancontrollerserver)");
        optServerR.fromConfigFile(rf.findFile("right_arm_file"),false);
        optServerL.fromConfigFile(rf.findFile("left_arm_file"),false);

        serverR=new PolyDriver;
        serverL=new PolyDriver;
        if (!serverR->open(optServerR) || !serverL->open(optServerL))
        {
            close();    
            return false;
        }

        IMultipleWrapper *wrapperR, *wrapperL;
        serverR->view(wrapperR);
        serverL->view(wrapperL);
        if (!wrapperR->attachAll(listR) || !wrapperL->attachAll(listL))
        {
            close();    
            return false;
        }

        return true;
    }

    virtual bool close()
    {
        if (serverR)
            delete serverR;

        if (serverL)
            delete serverL;

        if (torso)
            delete torso;

        if (armR)
            delete armR;

        if (armL)
            delete armL;

        return true;
    }

    virtual double getPeriod()    { return 1.0;  }
    virtual bool   updateModule() { return true; }
};


int main(int argc, char *argv[])
{
    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultContext("simCartesianControl/conf");
    rf.setDefault("robot","icubSim");
    rf.setDefault("local","simCartesianControl");
    rf.setDefault("right_arm_file","cartesianRightArm.ini");
    rf.setDefault("left_arm_file","cartesianLeftArm.ini");
    rf.configure("ICUB_ROOT",argc,argv);

    Network yarp;

    if (!yarp.checkNetwork())
        return -1;

    YARP_REGISTER_DEVICES(icubmod)

    SimCartCtrlModule mod;

    return mod.runModule(rf);
}



