/***************************************************************************
                          RTAdecoderConversion.cpp  -  description
                             -------------------
    copyright            : (C) 2013 Andrea Bulgarelli
                               2013 Andrea Zoli
                               2014 Valentina Fioretti
    email                : bulgarelli@iasfbo.inaf.it
                           zoli@iasfbo.inaf.it
                           fioretti@iasfbo.inaf.it
 ***************************************************************************/
/***************************************************************************
- Description:
Decoding the raw binary packets containing the conversion information.
- Last modified:
29/08/2014 (V. Fioretti)
****************************************************************************/
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
#include <CTACameraConv1.h>
#include <CTAPacket.h>
#include <CTAStream.h>
#include <CTADecoder.h>
#include <time.h>

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

		RTATelem::CTAStream stream(ctarta + "/share/rtatelem/rta_conv1.xml", argv[1], "");
		RTATelem::CTADecoder decoder(ctarta + "/share/rtatelem/rta_conv1.xml");

        ///Read a telemetry packet from .raw file. Return 0 if end of file
        ByteStreamPtr bs = stream.readPacket();
        int counter = 1;
        while(bs != 0) { //if not end of file
            cout << "----"<<endl;
            cout << "D: " << stream.getInputPacketDimension(bs) << endl;
            cout << "----" <<endl;

			//print the overall content of the packet
			//trtel.printPacket_input();

			cout << "--" << endl;

			// decode the byte stream
			RTATelem::CTAPacket& packet = decoder.getPacket(bs);
			cout << "PacketType: " << packet.getPacketType() << endl;
			cout << "CTA_CAMERA_CONVERSION_1 type: " << RTATelem::CTA_CAMERA_CONVERSION_1 << endl;				

/*
			if(packet.getPacketType() != RTATelem::CTA_CAMERA_PEDESTAL_1)
			{
				cerr << "Proveded a wrong .raw file. Expecting a CTA_CAMERA_PEDESTAL_1 type." << endl;
				return 0;
			}
*/
			RTATelem::CTACameraConv1& convtel = (RTATelem::CTACameraConv1&) packet;

			convtel.decode(true);

			//access the packet header information
			cout << "APID: " << convtel.header.getAPID() << endl;
			cout << "ssc: " << convtel.header.getSSC() << endl;

			//access the metadata information (array id, run id, event id)
			word arrayID;
			word runNumberID;
			convtel.header.getMetadata(arrayID, runNumberID);
			cout << "metadata: arrayID " << arrayID << " and runNumberID " << runNumberID << " " << endl;
			cout << "subtype " << convtel.header.getSubType() << endl;
			cout << "conversionRun:" << convtel.getConversionRun() << endl;


			//the id of the telescope that has triggered
			cout << "TelescopeId " << convtel.getTelescopeId() << endl;

			word nPixels = convtel.getNumberOfCalibrationPixels();
			cout << "NumberOfPixels " << nPixels << endl;

			//work with a single pixel of the telescope
			word pixelIndex=0;
			
			cout << "----------------------------------------------------------------------------------" << endl;
			cout << "Conversion High for Pixel ID " << pixelIndex << " " << convtel.getConversionHighValue(pixelIndex) << endl;
			cout << "Conversion Low for Pixel ID " << pixelIndex << " " << convtel.getConversionLowValue(pixelIndex) << endl;
			cout << "----------------------------------------------------------------------------------" << endl;


            for (int jpix = 0; jpix < nPixels; jpix++){
            	cout << "Pixel " << jpix << endl;
            	cout << "Conversion HIGH: " << convtel.getConversionHighValue(jpix) << endl;
            	cout << "Conversion LOW: " << convtel.getConversionLowValue(jpix) << endl;
            }

/*

			//******************
			cout << "--- Direct access to array of samples" << endl;
			//direct access to array of samples for each pixel
			//1) get a pointer to ByteStream
			ByteStreamPtr fadc = convtel.getPixelData(0);
			//cout << fadc->printStreamInHexadecimal() << endl;
			//2) swap for endianess
			fadc->swapWordForIntel();
			//3) get a pointer to word
			word *c = (word*) fadc->stream;
			cout << "pixel id " << c[0] << endl;
			cout << "number of samples " << c[1] << endl;
			cout << "value of first sample " << c[2] << endl;

			word* onlySamples = c + 2;
			cout << "Print all samples: ";
			for(int i=0; i < nsamples; i++)
				cout << " | " << onlySamples[i];
			cout << endl;
*/
			cout << "!counter of source packets " << counter << endl;

			///Read a telemetry packet from .raw file
            bs = stream.readPacket();

            counter++;
        }

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
