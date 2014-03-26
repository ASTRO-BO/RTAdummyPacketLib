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
#include <CTAPacket.h>
#include <CTAStream.h>
#include <CTADecoder.h>
//#include "CTAPacketBufferQ.h"
#include "RTAConfigLoad.h"
#include <time.h>

#include <TCanvas.h>
#include <TBox.h>

using namespace std;

/// Reading the Packet
int main(int argc, char *argv[])
{
    try
    {

        clock_t t = clock();
		string ctarta;
        if(argc > 1) {
        	/// The Packet containing the FADC value of each triggered telescope
        	const char* home = getenv("CTARTA");

        	if (!home)
        	{
        	   std::cerr << "CTARTA environment variable is not defined." << std::endl;
        	   return 0;
        	}

        	ctarta = home;
        } else {

        	cerr << "Please, provide the .raw" << endl;
        	return 0;
        }

		RTATelem::CTAStream stream(ctarta + "/share/rtatelem/rta_fadc1.stream", argv[1], "");
		RTATelem::CTADecoder decoder(ctarta + "/share/rtatelem/rta_fadc1.stream");
		RTAConfig::RTAConfigLoad ctaconf( ctarta + "/share/rtatelem/PROD2_telconfig.fits.gz" );  
        
		///Read a telemetry packet from .raw file. Return 0 if end of file
  
		ByteStreamPtr bs = stream.readPacket();
		int counter = 1;
		
		const int LARGE = 0;
		const int MEDIUM = 1;
		const int SMALL = 2;
		TCanvas* c1 = new TCanvas("camera", "camera", 300, 300);
		c1->Range(-1500, -1500, 1500, 1500);
		while(bs != 0) { 
			//if not end of file
			cout << "----"<<endl;
			cout << "D: " << stream.getInputPacketDimension(bs) << endl;
		    cout << "----" <<endl;

			//print the overall content of the packet
			//trtel.printPacket_input();

			cout << "--" << endl;

			// decode the byte stream
			RTATelem::CTAPacket& packet = decoder.getPacket(bs);
			if(packet.getPacketType() != RTATelem::CTA_CAMERA_TRIGGERDATA_1)
			{
				cerr << "Proveded a wrong .raw file. Expecting a CTA_CAMERA_TRIGGERDATA_1 type." << endl;
				return 0;
			}
			RTATelem::CTACameraTriggerData1& trtel = (RTATelem::CTACameraTriggerData1&) packet;
			trtel.decode(true);
			
			word telId = trtel.getTelescopeId();

			RTAConfig::RTAConfigLoad::Telescope* telStruct = ctaconf.getTelescopeStruct(telId);

			int64_t telTypeSim = telStruct->fromTeltoTelType.TelType;
			
			
			int teltype;
			switch(telTypeSim) {
				case 10408418:
					teltype = MEDIUM;
					break;
				case 3709425:
					teltype = SMALL;
					break;
				case 138704810:
					teltype = LARGE;
					break;
			}
			cout << "telType " << teltype << endl;
			struct RTAConfig::RTAConfigLoad::TelescopeType *telTypeStruct;
			struct RTAConfig::RTAConfigLoad::CameraType *camTypeStruct;
			struct RTAConfig::RTAConfigLoad::MirrorType *mirTypeStruct;
			
			
			telTypeStruct = ctaconf.getTelescopeTypeStruct(telTypeSim);
			mirTypeStruct = ctaconf.getMirrorTypeStruct((*telTypeStruct).fromTelTypetoMirType.mirType);
			
			
			camTypeStruct = ctaconf.getCameraTypeStruct((*telTypeStruct).fromTelTypetoCamType.camType);
			// The attribute stores the number of pixels
			word npixels =  (*camTypeStruct).NPixel;
			
			struct RTAConfig::RTAConfigLoad::PixelType *pixelTypeStruct = ctaconf.getPixelTypeStruct((*camTypeStruct).fromCameratoPixType.pixType);
			//word nsamples = (*pixelTypeStruct).NSamples;
			word nsamples = (*pixelTypeStruct).NSamples;
			//cout << npixels << " " << nsamples  << endl;
			
			struct RTAConfig::RTAConfigLoad::Pixel *pixStruct;
			if(teltype == 1) {
				for(int pi=0; pi<npixels; pi++) {
					pixStruct = ctaconf.getPixelStruct((*camTypeStruct).camType, pi);
					int16_t pixid = (*pixStruct).PixelID;
					float XTubeMM = (*pixStruct).XTubeMM;
					float YTubeMM = (*pixStruct).YTubeMM;
					float RTubeMM = (*pixStruct).RTubeMM;
					float RTubeDeg = (*pixStruct).RTubeDeg;
					//cout << pixid << " " << XTubeMM << " " << YTubeMM << " " << RTubeMM << " " << RTubeDeg << endl;
					TBox* b = new TBox(XTubeMM - RTubeMM/2, YTubeMM - RTubeMM/2, XTubeMM - RTubeMM/2, YTubeMM + RTubeMM/2);
					b->Draw("AL");
				}
				
				break;
			}
			
			///Read a telemetry packet from .raw file
			bs = stream.readPacket();

            counter++;
        }
		c1->SaveAs("camera.png");
		
		t = clock() - t;
		        cout << "It took me " << t << " clicks (" << ((float)t)/CLOCKS_PER_SEC << " seconds)" << endl;
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
