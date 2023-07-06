//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.routing.rpl;

import inet.routing.contract.IManetRouting;

//
// TODO auto-generated type
//
simple ObjectiveFunction
{
    parameters:
        // properties
        //@class("inet::TrickleTimer");  //CL 2021-12-10
        //@class("TrickleTimer");  //CL 2021-12-10
        @class("ObjectiveFunction");  //CL 2022-01-27


    gates:
        inout rplModule; // connection to routing protocol module
}
