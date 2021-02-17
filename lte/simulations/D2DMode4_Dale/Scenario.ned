// 
//                           SimuLTE
// 
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself, 
// and cannot be removed from it.
//
package lte.simulations.D2DMode4_Dale;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.ethernet.Eth10G;

import inet.node.xmipv6.HomeAgent6;
import lte.world.radio.LteChannelControl;
import lte.epc.PgwStandardSimplified;
import lte.corenetwork.binder.LteBinder;
import lte.corenetwork.deployer.LteDeployer;
import lte.corenetwork.nodes.eNodeB;
import lte.corenetwork.nodes.Ue;
import lte.corenetwork.nodes.cars.CarNonIp;

//import org.car2x.veins.nodes.Car;
//import org.car2x.veins.subprojects.veins_inet.VeinsInetManager;

network Scenario
{
    parameters:
        int numUav = default(1);
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)

        @display("bgb=113,306");

    submodules:

        //# LTE modules
        channelControl: LteChannelControl {
            @display("p=50,25;is=s");
        }
        binder: LteBinder {
            @display("p=51,74;is=s");
        }
        deployer: LteDeployer {
            @display("p=51,128;is=s");
        }
        uav[numUav]: CarNonIp {
        //car0: CarNonIp{

        //}
        //car1: CarNonIp{

        //}
        //car2: CarNonIp{

        //}

            @display("p=51,257");
        }
}
