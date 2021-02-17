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

/**
 * Mode4App is a new application developed to be used with Mode 4 based simulation
 * Author: Brian McCarthy
 * Email: b.mccarthy@cs.ucc.ie
 */
#include <iostream>
#include <sstream>
#include <fstream>
#include "apps/mode4App/Mode4App.h"
#include "common/LteControlInfo.h"
#include "stack/phy/packet/cbr_m.h"


Define_Module(Mode4App);

void Mode4App::initialize(int stage)
{

    Mode4BaseApp::initialize(stage);
    if (stage==inet::INITSTAGE_LOCAL){
        // Register the node with the binder
        // Issue primarily is how do we set the link layer address

        // Get the binder
        binder_ = getBinder();

        // Get our UE
        cModule *ue = getParentModule();

        //Register with the binder
        nodeId_ = binder_->registerNode(ue, UE, 0);

        // Register the nodeId_ with the binder.
        binder_->setMacNodeId(nodeId_, nodeId_);
    } else if (stage==inet::INITSTAGE_APPLICATION_LAYER) {

        EV << "INIZIO" << endl;
        selfSender_ = NULL;
        nextSno_ = 1; //parto da 1 piuttosto che 0
        last_forwarded_ = 0;
        selfSender_ = new cMessage("selfSender");

        size_ = par("packetSize");
        period_ = par("period");
        priority_ = par("priority");
        duration_ = par("duration");

        cbr_ = registerSignal("cbr");
        sentMsg_ = registerSignal("sentMsg");
        delay_ = registerSignal("delay");
        rcvdMsg_ = registerSignal("rcvdMsg");
        tput_ = registerSignal("tput");

        double delay = 0.001 * intuniform(0, 1000, 0);
        if(period_>0){
            scheduleAt((simTime() + delay).trunc(SIMTIME_MS), selfSender_);
        }
    }
}

void Mode4App::handleLowerMessage(cMessage* msg)
{
    if (msg->isName("CBR")) {
        Cbr* cbrPkt = check_and_cast<Cbr*>(msg);
        double channel_load = cbrPkt->getCbr();
        emit(cbr_, channel_load);
        delete cbrPkt;
    } else {
        AlertPacket* pkt = check_and_cast<AlertPacket*>(msg);
        if (pkt == 0)
            throw cRuntimeError("Mode4App::handleMessage - FATAL! Error when casting to AlertPacket");
        EV << "getDest: " << pkt->getDest() << " currentNode: " << this->getParentModule()->getIndex()<< endl;
        FlowControlInfoNonIp *source = check_and_cast<FlowControlInfoNonIp*>(msg->getControlInfo());

        if(pkt->getDest()==this->getParentModule()->getIndex()){

            simtime_t delay = simTime() - pkt->getTimestamp();

            if(simTime()>10){
                //pacchetto per me prendo il tempo e faccio le statistiche
                // emit statistics
                emit(delay_, delay);
                emit(rcvdMsg_, (long)1);
                emit(tput_, (long)size_); //sommo tutti i byte ricevuti
            }


            EV << "PACCHETTO ARRIVATO" << endl;
            EV << "Mode4App::handleMessage - Packet received: SeqNo[" << pkt->getSno() << "] Delay[" << delay << "]" << endl;

            delete msg;
        }else{
            if(pkt->getDest()>this->getParentModule()->getIndex()){

                EV << "getSno: " << pkt->getSno() << " lastForwarded: " << last_forwarded_ << endl;

                if(pkt->getSno()>last_forwarded_){
                    //EV << "getSno: " << pkt->getSno() << " lastForwarded: " << last_forwarded_ << endl;
                    last_forwarded_=pkt->getSno(); //salvo l' ultimo pacchetto che sto inoltrando

                    //pacchetto non per me inoltro
                    EV << "INOLTRO" << endl;
                    pkt->removeControlInfo(); //tolgo lteinfo vecchio

                    //aggiungo nuove info
                    auto lteControlInfo = new FlowControlInfoNonIp();
                    lteControlInfo->setSrcAddr(nodeId_);
                    lteControlInfo->setDirection(D2D_MULTI);
                    lteControlInfo->setPriority(priority_);
                    lteControlInfo->setDuration(duration_);
                    lteControlInfo->setCreationTime(simTime());
                    pkt->setControlInfo(lteControlInfo);

                    //invio
                    Mode4BaseApp::sendLowerPackets(pkt);
                    //emit(sentMsg_, (long)1); //stat di invio del pacchetto non di inoltro
                }else{
                    EV << "NON INOLTRO (se no creo loop)" << endl;
                }

            }else{
                EV << "NON INOLTRO" << endl;
            }
        }
    }
}

void Mode4App::handleSelfMessage(cMessage* msg)
{
    if (!strcmp(msg->getName(), "selfSender")){  //caso in cui 'e selfsend (in c e' al contrario)
        // Replace method
        AlertPacket* packet = new AlertPacket("Alert");
        packet->setTimestamp(simTime());
        packet->setByteLength(size_);
        packet->setSno(nextSno_);

        int destHost = par("destHost");

        destHost = destHost - 1;

        packet->setDest(destHost); //destinazione invio pacchetti al nodo x

        nextSno_++;

        last_forwarded_=nextSno_; //gli eguaglio perche' altrimenti se torna indietro il pacchetto lo inoltra

        auto lteControlInfo = new FlowControlInfoNonIp();

        lteControlInfo->setSrcAddr(nodeId_);
        lteControlInfo->setDirection(D2D_MULTI);
        lteControlInfo->setPriority(priority_);
        lteControlInfo->setDuration(duration_);
        lteControlInfo->setCreationTime(simTime());

        packet->setControlInfo(lteControlInfo);

        Mode4BaseApp::sendLowerPackets(packet);
        emit(sentMsg_, (long)1);

        scheduleAt(simTime() + period_, selfSender_);
    }
    else
        throw cRuntimeError("Mode4App::handleMessage - Unrecognized self message");
}

void Mode4App::finish()
{
    cancelAndDelete(selfSender_);
}

Mode4App::~Mode4App()
{
    binder_->unregisterNode(nodeId_);
}
