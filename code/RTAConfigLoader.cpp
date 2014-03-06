/***************************************************************************
                          RTAConfigLoader.cpp  -  description
                             -------------------
    copyright            : (C) 2014 Andrea Bulgarelli
                               2014 Andrea Zoli
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
#include "CTAPacketBufferQ.h"
#include "RTAConfigLoad.h"
#include <time.h>

using namespace std;

/// Reading the Packet
int main(int argc, char *argv[])
{
    try
    {

        clock_t t;
        RTATelem::CTACameraTriggerData1 * trtel;
        RTAConfig::RTAConfigLoad * ctaconf;
        if(argc > 1) {
        	/// The Packet containing the FADC value of each triggered telescope
        	string ctarta;
        	const char* home = getenv("CTARTA");

        	if (!home)
        	{
        	   std::cerr << "CTARTA environment variable is not defined." << std::endl;
        	   return 0;
        	}

        	ctarta = home;

        	trtel = new RTATelem::CTACameraTriggerData1(ctarta + "/share/rtatelem/rta_fadc1.stream", argv[1], "");
        	ctaconf = new RTAConfig::RTAConfigLoad( ctarta + "/conf/PROD2_telconfig.fits.gz" );

        } else {

        	cerr << "Please, provide the .raw" << endl;
        	return 0;
        }
                
        ///Read a telemetry packet from .raw file. Return 0 if end of file
  
        ByteStreamPtr bs = trtel->readPacket(0);
        byte* b_trtel = 0;
        if(bs) b_trtel = bs->stream;
        int counter = 1;
        while(bs != 0) { //if not end of file
            cout << "----"<<endl;
            cout << "D: " << trtel->getInputPacketDimension(bs) << endl;
            cout << "----" <<endl;

			//print the overall content of the packet
			//trtel->printPacket_input();

			cout << "--" << endl;

			//access the packet header information
			cout << "APID: " << trtel->header->getAPID() << endl;
			cout << "ssc: " << trtel->header->getSSC() << endl;

			//access the metadata information (array id, run id, event id)
			word arrayID;
			word runNumberID;
			trtel->header->getMetadata(arrayID, runNumberID);
			cout << "metadata: arrayID " << arrayID << " and runNumberID " << runNumberID << " " << endl;
			cout << "eventNumber:" << trtel->getEventNumber() << endl;

			//trigger time
			cout << "Telescope Time " << trtel->header->getTime() << endl;

			//the number of telescopes that have triggered
			cout << "Triggered telescopes: " << (long) trtel->getNumberOfTriggeredTelescopes() << endl;

			//the index (zero-based) of the telescope that has triggerd
			cout << "Index Of Current Triggered Telescope " << (long) trtel->getIndexOfCurrentTriggeredTelescope() << endl;
			//the id of the telescope that has triggered
			cout << "TelescopeId " << trtel->getTelescopeId() << endl;

			/// Read telescope configuration     
			struct RTAConfig::RTAConfigLoad::Array *arrStruct = ctaconf->getArrayStruct();
			struct RTAConfig::RTAConfigLoad::Telescope *telStruct = ctaconf->getTelescopeStruct(trtel->getTelescopeId());			
			cout << "--------- Telescope Metadata ---------" << endl;
			cout << "Array type: " << (*arrStruct).ArrayID << endl;
			cout << "Array N telescopes: " << (*arrStruct).NTel << endl;
			cout << "Telescope type: " << (*telStruct).fromTeltoTelType.TelType << endl;
			cout << "Telescope X: " << (*telStruct).TelX << endl;
			cout << "Telescope Y: " << (*telStruct).TelY << endl;
			cout << "Telescope Z: " << (*telStruct).TelZ << endl;
			struct RTAConfig::RTAConfigLoad::TelescopeType *telTypeStruct = ctaconf->getTelescopeTypeStruct((*telStruct).fromTeltoTelType.TelType);
			cout << "Telescope Mirror Type: " << (*telTypeStruct).fromTelTypetoMirType.mirType << endl;
			cout << "Telescope Camera Type: " << (*telTypeStruct).fromTelTypetoCamType.camType << endl;
			struct RTAConfig::RTAConfigLoad::MirrorType *mirTypeStruct = ctaconf->getMirrorTypeStruct((*telTypeStruct).fromTelTypetoMirType.mirType);
			struct RTAConfig::RTAConfigLoad::CameraType *camTypeStruct = ctaconf->getCameraTypeStruct((*telTypeStruct).fromTelTypetoCamType.camType);
			cout << "Mirror Area: " << (*mirTypeStruct).MirrorArea << endl;
			cout << "Camera N Pixels: " << (*camTypeStruct).NPixel << endl;
			cout << "Camera N Pixels active: " << (*camTypeStruct).NPixel_active << endl;
			cout << "Camera Pixel type: " << (*camTypeStruct).fromCameratoPixType.pixType << endl;
			struct RTAConfig::RTAConfigLoad::PixelType *pixTypeStruct = ctaconf->getPixelTypeStruct((*camTypeStruct).fromCameratoPixType.pixType);
			cout << "Pixel ID for pixel 0: " << (*camTypeStruct).fromCameratoPixel[0].PixelID << endl;
			cout << "XTubeMM for pixel 0: " << (*camTypeStruct).fromCameratoPixel[0].XTubeMM << endl;
			cout << "YTubeMM for pixel 0: " << (*camTypeStruct).fromCameratoPixel[0].YTubeMM << endl;
			cout << "RTubeMM for pixel 0: " << (*camTypeStruct).fromCameratoPixel[0].RTubeMM << endl;
			cout << "------------------------------------" << endl;
		
			word nPixels = trtel->getNumberOfPixels();
			cout << "NumberOfPixels " << nPixels << endl;

			//work with a single pixel of the telescope
			word pixelIndex=0;

			cout << "PixelId " << trtel->getPixelId(pixelIndex) << endl;
			cout << "PixelId+1 " << trtel->getPixelId(pixelIndex+1) << endl;

			word nsamples = trtel->getNumberOfSamples(pixelIndex);
			cout << "Samples: " << nsamples << endl;

			word sampleIndex=0;
			cout << "SampleValue " << trtel->getSampleValue(pixelIndex, sampleIndex) << endl;

			cout << "!counter of source packets " << counter << endl;

			///Read a telemetry packet from .raw file
            bs = trtel->readPacket(0);
            if(bs) b_trtel = bs->stream;

            counter++;
        }

        t = clock() - t;
        cout << "It took me " << t << " clicks (" << ((float)t)/CLOCKS_PER_SEC << " seconds)" << endl;
        delete trtel;
        delete ctaconf;
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
