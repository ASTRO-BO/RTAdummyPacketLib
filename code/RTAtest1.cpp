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
#include "CTAPacketBufferV.h"
#include "mac_clock_gettime.h"
#include "packet/File.h"
#include <cstdlib>
#include <cstring>
#include <cmath>

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
struct timespec start, stop;
long filesize;

void end(int ntimefilesize=1) {
	
	
	clock_gettime( CLOCK_MONOTONIC, &stop);
	double time = timediff(start, stop);
	//std::cout << "Read " << ncycread << " ByteSeq: MB/s " << (float) (ncycread * Demo::ByteSeqSize / time) / 1048576 << std::endl;
	cout << "Result: it took  " << time << " s" << endl;
	if(filesize != 0)
		cout << "Result: rate: " << setprecision(10) << filesize * ntimefilesize / 1000000 / time << " MiB/s" << endl;
	exit(0);
}

/// Reading the Packet
int main(int argc, char *argv[])
{
	filesize = 0;
	int buffersize = 100;
	
    try
    {

        
        RTATelem::CTACameraTriggerData1 * trtel;
		
		int test = 0;
		
		if(argc < 4) {
			cout << "exe file.raw testID(0/7) memcpy(0/1)" << endl;
			cout << "where test is:" << endl;
			cout << "0: check data model loading" << endl;
			cout << "1: load data into a circular buffer" << endl;
			cout << "2: decoding for routing (identification of the type of packet)" << endl;
			cout << "3: decoding all the blocks of the packet" << endl;
			exit(0);
		}
		
		test = atoi(argv[2]);
		if(test == 0)
			cout << "Test 0: check data model loading" << endl;
		if(test == 1)
			cout << "Test 1: check the loading of "<< buffersize << " camera data packets" << endl;
		if(test == 2)
			cout << "Test 2: decoding for routing (identification of the type of packet)" << endl;
		if(test == 3)
			cout << "Test 3: decoding all the blocks of the packet (method 1 packetlib)" << endl;
		if(test == 7)
			cout << "Test 7: decoding all the blocks of the packet (method 2 packetlib::bytestream builder)" << endl;
		
		bool activatememorycopy = atoi(argv[3]);
		if(activatememorycopy)
			cout << "Test  : memcpy activated..." << endl;
		
		if(test == 0) {
			clock_gettime( CLOCK_MONOTONIC, &start);
			cout << "start Test 0 ..." << endl;
		}
		string configFilaName = "/share/rtatelem/rta_fadc_all.stream";
		string ctarta;
		
        if(argc > 1) {
        	/// The Packet containing the FADC value of each triggered telescope
        	
        	const char* home = getenv("CTARTA");

        	if (!home)
        	{
        	   std::cerr << "ERROR: CTARTA environment variable is not defined." << std::endl;
        	   return 0;
        	}

        	ctarta = home;

        	trtel = new RTATelem::CTACameraTriggerData1(ctarta + configFilaName, argv[1], "");

        } else {

        	cerr << "ERROR: Please, provide the .raw" << endl;
        	return 0;
        }
		
		if(test == 0)
			end();
		
		//check the size of the file
		File f;
		f.open(argv[1]);
		filesize = f.fsize();
		f.close();
		//cout << filesize << endl;
		
		byte* buffermemory = new byte[2000*50];
		
		
		try {
			if(test == 1) {
				clock_gettime( CLOCK_MONOTONIC, &start);
				cout << "start Test 1 ..." << endl;
			}
			RTATelem::CTAPacketBufferV buff(ctarta + configFilaName, argv[1]);
			buff.load(0, buffersize);
			
			if(test == 1)
				end();
			
			int ntimes;
			
			if(test == 7) {
				clock_gettime( CLOCK_MONOTONIC, &start);
				ntimes = 100;
				cout << "start Test 7 ... " << ntimes << " runs " << endl;
				
			}
			
			if(test == 6) {
				clock_gettime( CLOCK_MONOTONIC, &start);
				ntimes = 2;
				cout << "start Test 6 ... " << ntimes << " runs " << endl;
				
			}
			
			if(test == 5) {
				clock_gettime( CLOCK_MONOTONIC, &start);
				ntimes = 2;
				cout << "start Test 5 ... " << ntimes << " runs " << endl;
				
			}
			
			if(test == 4) {
				clock_gettime( CLOCK_MONOTONIC, &start);
				ntimes = 4;
				cout << "start Test 4 ... "  << ntimes << " runs " << endl;
				
			}
			
			if(test == 3) {
				clock_gettime( CLOCK_MONOTONIC, &start);
				ntimes = 50;
				cout << "start Test 3 ... " << ntimes << " runs " << endl;
				
			}
			
			if(test == 2) {
				clock_gettime( CLOCK_MONOTONIC, &start);
				ntimes = 1000;
				cout << "start Test 2 ... " << ntimes << " runs " << endl;
				
			}
			
			
			long npacketsrun2 = buffersize * ntimes;
			long npacketsread2 = 0;
			
			
			while(npacketsread2 < npacketsrun2) {
				ByteStreamPtr rawPacket = buff.getNext();
				
				dword size = 0;
				size = trtel->getInputPacketDimension(rawPacket);
				int type = -1;
				type = trtel->getInputPacketType(rawPacket);
				//cout << "Packet #" << npacketsread2 << " size: " << size << " byte. type: " << type << endl;
				if(type == 1)
					switch(test) {
						case 3:
							{
								//access to a pointer of the camera data (all pixels) as a single block
								//inly the decoding of the sections is performed
								trtel->setStream(rawPacket, true);
								ByteStreamPtr camera = trtel->getCameraDataSlow();
								//cout << rawPacket->getDimension() << " " << camera->getDimension() << endl;
								word *c = (word*) camera->stream;
								if(activatememorycopy) {
									memcpy(buffermemory, camera->stream, camera->getDimension());
									/*
									for(int i=0; i<10; i++)
										cout << c[i] << endl;
									cout << "--"<<endl;
									*/
								}
								//use it
								//cout << "value of first sample " << c[0] << endl;
								//cout << "Index Of Current Triggered Telescope " << (long) trtel->getIndexOfCurrentTriggeredTelescope() << endl;
							}
							break;
						case 4:
							{
								
								//access to a pointer of each pixel of a camera as a single block
								//the decodigin of all the blocks is performed
								trtel->setStream(rawPacket);
								word npixels = trtel->getNumberOfPixels();
								int pixel=0;
								//for(pixel=0; pixel<npixels; i++)
								ByteStreamPtr fadc = trtel->getPixelData(pixel);
								
								//3) get a pointer to word
								//word *c = (word*) fadc->stream;
								//cout << "value of first sample " << c[0] << endl;
							}
							break;
						case 5:
							{
								//acces to an array of samples
								//the decodigin of all the blocks is performed
								
								trtel->setStream(rawPacket);
								word npixels = trtel->getNumberOfPixels();
								int pixel=0;
								word nsamples = trtel->getNumberOfSamples(pixel);
								//cout << nsamples << endl;
								ByteStreamPtr samplebs = trtel->getPixelData(pixel);
								
								word* sample = (word*) samplebs->stream;
								/*
								 cout << "Print all samples: ";
								 for(int i=0; i < nsamples; i++)
								 cout << " | " << sample[i];
								 cout << endl;
								 */
								
							}
							break;
						case 6:
							{
								//access to the single sample of a pixel
								//cout << "decode" << endl;
								word npixels = trtel->getNumberOfPixels();
								int pixel=0;
								word nsamples = trtel->getNumberOfSamples(pixel);
								trtel->setStream(rawPacket);
								word sample = trtel->getSampleValue(pixel, 0);
								
								

							}
							break;
						case 7:
							{
								ByteStreamPtr camera = trtel->getCameraData(rawPacket);
								//cout << camera->getDimension() << endl;
								word *c = (word*) camera->stream;
								if(activatememorycopy) memcpy(buffermemory, camera->stream, camera->getDimension());
								//cout << "value of first sample " << c[0] << endl;
							}
							break;
						 
				}
				
				npacketsread2++;
			};
			
			/*
			if(activatememorycopy) {
				word *c = (word*) buffermemory;
				//cout << "M" << endl;
				//for(int i=0; i<10; i++)
				//	cout << c[i] << endl;
			}
			*/
			
			if(test == 2)
				end(ntimes);
			if(test == 3)
				end(ntimes);
			if(test == 4)
				end(ntimes);
			if(test == 5)
				end(ntimes);
			if(test == 6)
				end(ntimes);
			if(test == 7)
				end(ntimes);
			
			
				
			
		} catch(PacketException* e) {
	        cout << e->geterror() << endl;
		}
		
		
		
	
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
