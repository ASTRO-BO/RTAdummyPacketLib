/***************************************************************************
                          main.cpp  -  description
                             -------------------
    copyright            : (C) 2013 Andrea Bulgarelli
                               2013 Andrea Zoli
                               2014 Valentina Fioretti
    email                : bulgarelli@iasfbo.inaf.it
                           zoli@iasfbo.inaf.it
                           fioretti@iasfbo.inaf.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <stdlib.h>
#include "CTACameraTriggerData1.h"
#include <time.h>
#include <math.h>
using namespace std;


/// Writing the Packet
int main(int argc, char *argv[])
{
    try
    {
        clock_t t;

        string ctarta;
        const char* home = getenv("CTARTA");

        if (!home)
        {
        	std::cerr << "CTARTA environment variable is not defined." << std::endl;
        	return 0;
        }

        ctarta = home;

        /// The Packet containing the FADC value of each triggered telescope
        // One packet for each triggered telescope
        RTATelem::CTACameraTriggerData1 trtelsss = RTATelem::CTACameraTriggerData1(ctarta + "/share/rtatelem/rta_fadc1.stream", "", "out_dummy_fadc.raw");
        RTATelem::CTACameraTriggerData1 * trtel = & trtelsss;
 
        ///Number of events
        int numberOfEvent=20;

        /// The attribute stores the number of triggered telescopes for each event
        int numberOfTriggeredTelescopes = 5;
        srand(0);
		long counts = 0;
        for(int evtindex = 0; evtindex<numberOfEvent; evtindex++) {
			cout << evtindex << endl;
            //for each triggere telescope, generate a telemetry packet
            for(int telindex = 0; telindex<numberOfTriggeredTelescopes; telindex++) {
				

                //**************************
                //set the header of the tm packet
                trtel->header->setAPID(telindex); 	//the data generator (for now, the telescope)
                trtel->header->setSSC(evtindex*numberOfEvent+telindex);	//a unique counter of packets
                trtel->header->setMetadata(1, 2);	//the metadata
                trtel->header->setTime(1500);	//the time
                word nsamples = 40;
                trtel->header->setSubType(nsamples); //important, for fast packet identification

                //**************************
                //event information
                int evnum = 10;
                trtel->setEventNumber(evnum);	//another metadata: the event number (e.g. provided by event builder)
                trtel->setNumberOfTriggeredTelescopes(numberOfTriggeredTelescopes);	//the number of triggere telescopes (provided by the event builder)
                trtel->setIndexOfCurrentTriggeredTelescope(telindex);	//an internal index of the telescope within the event. This should be used
                                        //to check data loss
                trtel->setTelescopeId(telindex*10+5);	//the telescope that has triggered	

                //**************************
                //camera information
                
                //set the number of pixels and samples. In this way it is possible to manage different cameras with the same layout
                // The attribute stores the number of pixels
                word npixels = 1141;
                // The attribute stores the number of samples

                trtel->setNumberOfPixels(npixels);
				cout << npixels << endl;
				trtel->setNumberOfPixelsID(0);

                //set information of the pixels and sample
                for(int pixelindex=0; pixelindex<npixels; pixelindex++) {
                    //trtel->setPixelId(pixelindex, pixelindex);
                    trtel->setNumberOfSamples(pixelindex, nsamples);
                    for(int sampleindex=0; sampleindex<nsamples; sampleindex++) {
                    	int val = (int)(rand() % 255);
                        trtel->setSampleValue(pixelindex, sampleindex, val);
                    }
                }

                //and finally, write the packet to output (in this example, write the output to file)
                trtel->writePacket();
				counts++;

                //just for check, write the content of the packet to stdout
                //if(evtindex == 0) trtel->printPacket_output();

            }		

        }
        
        t = clock() - t;
        //printf ("It took me %d clicks (%f seconds).\n",t,((float)t)/CLOCKS_PER_SEC);
        cout << "END " << counts << endl;
        return 0;

    }
    catch(PacketExceptionIO* e)
    {
        cout << e->geterror() << endl;;
    }
    catch(PacketException* e)
    {
        cout << e->geterror() << endl;
    }

	return 1;
}
