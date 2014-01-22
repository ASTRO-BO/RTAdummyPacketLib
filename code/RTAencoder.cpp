/***************************************************************************
                          main.cpp  -  description
                             -------------------
    copyright            : (C) 2013 Andrea Bulgarelli
                               2013 Andrea Zoli
    email                : bulgarelli@iasfbo.inaf.it
                           zoli@iasfbo.inaf.it
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
#include "CTACameraTriggerData.h"
#include <time.h>

using namespace std;


/// Writing the Packet
int main(int argc, char *argv[])
{
    try
    {
        clock_t t;
		 
        /// The Packet containing the FADC value of each triggered telescope
        RTATelem::CTACameraTriggerData trtelsss = RTATelem::CTACameraTriggerData("../share/rtatelem/rta_fadc.stream", "", "out_dummy_fadc.raw");
        RTATelem::CTACameraTriggerData * trtel = & trtelsss;	

        /// The attribute stores the event number
        int evnum=10;

        /// The attribute stores the number of triggered telescopes
        int numberOfTriggeredTelescopes = 1;
        for(int telindex = 0; telindex<numberOfTriggeredTelescopes; telindex++) {

            trtel->header->setAPID(10);
            trtel->header->setSSC(0);
            trtel->header->setMetadata(1, 2);
            trtel->header->setTime(1500);

            trtel->setEventNumber(evnum);
            trtel->setNumberOfTriggeredTelescopes(numberOfTriggeredTelescopes);
            trtel->setIndexOfCurrentTriggeredTelescope(telindex);
            trtel->setTelescopeId(telindex*10+5);

            /// The attribute stores the number of pixels
            word npixels = 1141;
            /// The attribute stores the number of samples
            word nsamples = 40;
            trtel->setNumberOfPixels(npixels);

            for(int pixelindex=0; pixelindex<npixels; pixelindex++) {
                trtel->setPixelId(pixelindex, pixelindex);
                trtel->setNumberOfSamples(pixelindex, nsamples);
                for(int sampleindex=0; sampleindex<nsamples; sampleindex++)
                    trtel->setSampleValue(pixelindex, sampleindex, 3);
            }

            trtel->writePacket();

            trtel->printPacket_output();

        }		

        t = clock() - t;
        //printf ("It took me %d clicks (%f seconds).\n",t,((float)t)/CLOCKS_PER_SEC);
        cout << "END" << endl;
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
