// 
//   Simulation model for RPL (Routing Protocol for Low-Power and Lossy Networks)
//  
//   Copyright (C) 2021  Institute of Communication Networks (ComNets),
//                       Hamburg University of Technology (TUHH)
//             (C) 2021  Yevhenii Shudrenko
//  
//   This program is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//  
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//  
//   You should have received a copy of the GNU General Public License
//   along with this program.  If not, see <https://www.gnu.org/licenses/>.
//  

package AMI;

import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;

import inet.node.inet.AdhocHost;
import inet.routing.rpl.Rpl;
import inet.routing.rpl.TrickleTimer;
import inet.routing.rpl.ObjectiveFunction; //CL 2022-01-27

module RplRouter extends AdhocHost
{   
    parameters:
        double x @unit(m);  //CL
        double y @unit(m);  //CL
        
    submodules:
        rpl: Rpl {
            @display("p=825,226");
        }
        trickleTimer: TrickleTimer {
            @display("p=946.57495,225.22499");
        }
        objectiveFunction: ObjectiveFunction {  //CL 2022-01-27
            @display("p=946.57495,125.22499");
        }

    connections:
        rpl.ipOut --> tn.in++;
        rpl.ipIn <-- tn.out++;
        rpl.ttModule <--> trickleTimer.rpModule;
        rpl.ofModule <--> objectiveFunction.rplModule; //CL 2022-01-27
}

