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
#include <limits>
//#include <thread>

bool iszero(double someValue) {
	if(someValue == 0)
		return true;
	if (someValue <  std::numeric_limits<double>::epsilon() &&
    	someValue > -std::numeric_limits<double>::epsilon())
    	return true;
    return false;
}


using namespace std;
struct timespec start, stop;
long filesize;
unsigned long totbytes;

void end(int ntimefilesize=1) {
	
	
	clock_gettime( CLOCK_MONOTONIC, &stop);
	double time = timediff(start, stop);
	//std::cout << "Read " << ncycread << " ByteSeq: MB/s " << (float) (ncycread * Demo::ByteSeqSize / time) / 1048576 << std::endl;
	cout << "Result: it took  " << time << " s" << endl;
	if(filesize != 0)
		cout << "Result: rate: " << setprecision(10) << totbytes / 1000000 / time << " MiB/s" << endl;
	cout << totbytes << endl;
	exit(0);
}

void printCamera(word* c, int ssc, int npixels, int nsamples) {
	cout << "ssc " << ssc << endl;
	for(int pixel = 0; pixel<npixels; pixel++) {
		cout << pixel << " ";
		for(int j=0; j<nsamples; j++)
			cout << c[pixel * nsamples + j] << " ";
		cout << endl;
	}
	
}

void printBuffer(word* c, int npixels, int nsamples) {
	
	for(int pixel = 0; pixel<npixels; pixel++) {
		cout << pixel << " ";
		for(int j=0; j<nsamples; j++)
			cout << c[pixel * nsamples + j] << " ";
		cout << endl;
	}
	
}

int flag = 0;

void calcWaveformExtraction1(byte* buffer, int npixels, int nsamples, int ws ) {
	word *b = (word*) buffer; //should be pedestal subtractred
	//printBuffer(b, npixels, nsamples);

	/*
	vector<int> maxresv;
	maxresv.reserve(npixels);
	vector<double> timev;
	timev.reserve(npixels);
	int* maxres = &maxresv[0];
	double* time = &timev[0];
	*/
	
	int* maxres = new int[npixels];
	double* time = new double[npixels];
	
	
	
	word bl[npixels*nsamples];
	memcpy(bl, b, npixels*nsamples*sizeof(word));
	
	for(int pixel = 0; pixel<npixels; pixel++) {
		word* s = bl + pixel * nsamples;
		
		/*
		if(flag > 9000) {
		
			cout << pixel << " ";
			for(int k=0; k<nsamples; k++)
				cout << s[k] << " ";
			cout << endl;
		}*/
		
		
		long max = 0;
		double maxt = 0;
		long sumn = 0;
		long sumd = 0;
		long maxj = 0;
		double t = 0;
		//long sumres[nsamples-ws];
		
		for(int j=0; j<=ws-1; j++) {
			sumn += s[j] * j;
			sumd += s[j];
		}
		//sumres[0] = sum;
		for(int j=1; j<nsamples-ws; j++) {

			sumn = sumn - s[j-1] * (j-1) + s[j+ws] * (j+ws);
			sumd = sumd - s[j-1] + s[j+ws-1];
			
			if(!iszero(sumd))
				t = sumn / (double)sumd;
			//sumres[j] = sum;
			if(sumd > max) {
				max = sumd;
				maxt = t;
				maxj = j;
			}
			
			
		}
		
		/*for(int j=0; j<nsamples-ws; j++)
			if(sumres[j]>max) {
				max = sumres[j];
				maxj = j;
			}
		 */
		
		
		//maxres.push_back(max);
		//time.push_back(maxt);
		
		
		maxres[pixel] = max;
		time[pixel] = maxt;
		
		
		//if(flag > 9000) cout << pixel << " " << maxj << " " << maxres[pixel] << " " << time[pixel] << " " << endl;
		
		/*
		for(int k=0; k<nsamples; k++)
			cout << s[k] << " ";
		cout << endl;
		 */
	}
	//SharedPtr<double> shtime(maxt);
	
	flag++;
	
	
}

void calcWaveformExtraction3(byte* buffer, int npixels, int nsamples, int ws) {
	word *b = (word*) buffer; //should be pedestal subtractred
	/*
	 vector<int> maxres;
	 maxres.reserve(npixels);
	 vector<int> time;
	 time.reserve(npixels);
	 */
	word bl[npixels*nsamples];
	memcpy(bl, b, npixels*nsamples);
	for(int pixel = 0; pixel<npixels; pixel++) {
		//word* s = bl + pixel * nsamples;
		long indexstart = pixel * nsamples;
		
		long max = 0;
		long maxj = 0;
		long sum = 0;
		for(int j=indexstart; j<indexstart+nsamples-ws; j++) {
			if(j==indexstart)
				for(int i=j; i<=j+ws-1; i++)
					sum += bl[i];
			else
				sum = sum - bl[j-1] + bl[j+ws];
			if(sum>max) {
				max = sum;
				maxj = j;
			}
			
		}
		//maxres.push_back(max);
		//time.push_back(maxj);
	}
}


void getMax(word* s, int ws, int nsamples, int maxj, long max ) {
	
	long sum = 0;
	for(int j=0; j<nsamples-ws; j++) {
		if(j==0)
			for(int i=j; i<=j+ws-1; i++)
				sum += s[i];
		else
			sum = sum - s[j-1] + s[j+ws];
		if(sum>max) {
			max = sum;
			maxj = j;
		}
		
	}
}

/*void calcWaveformExtraction2(byte* buffer, int npixels, int nsamples, int ws) {
	word *b = (word*) buffer; //should be pedestal subtractred*/
	/*
	vector<int> maxres;
	maxres.reserve(npixels);
	vector<int> time;
	time.reserve(npixels);
	*/
/*	for(int pixel = 0; pixel<npixels; pixel++) {
		word* s = b + pixel * nsamples;
		int maxj = 0;
		long max = 0;
		std::thread first(getMax, s, ws, nsamples, maxj, max);
		//getMax(s, ws, nsamples, maxj, max);
		//cout << pixel << endl;
		first.join();
		
		
		//maxres.push_back(max);
		//time.push_back(maxj);
	}
}*/

/// Reading the Packet
int main(int argc, char *argv[])
{
	filesize = 0;
	int buffersize;
	
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
			cout << "Test 1: check the loading of camera data packets" << endl;
		if(test == 2)
			cout << "Test 2: decoding for routing (identification of the type of packet)" << endl;
		if(test == 3)
			cout << "Test 3: decoding all the blocks of the packet (method 1 packetlib)" << endl;
		if(test == 7)
			cout << "Test 7: decoding all the blocks of the packet (method 2 packetlib::bytestream builder)" << endl;
		
		bool activatememorycopy = atoi(argv[3]);
		if(activatememorycopy)
			cout << "Test  : memcpy activated..." << endl;
		
		bool calcalg = atoi(argv[4]);
		if(calcalg)
			cout << "Test  : waveform extraction algorithm " << endl;
		
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
		
		byte* buffermemory = new byte[2000*50*sizeof(word)];
		
		
		try {
			if(test == 1) {
				clock_gettime( CLOCK_MONOTONIC, &start);
				cout << "Start Test 1 ..." << endl;
			}
			RTATelem::CTAPacketBufferV buff(ctarta + configFilaName, argv[1]);
			buff.load();
			buffersize = buff.size();
			cout << "Loaded " << buffersize << " packets " << endl;
			totbytes = 0;
			
			if(test == 1) {
				for(long i=0; i<buffersize; i++) {
					ByteStreamPtr rawPacket = buff.getNext();
					totbytes += trtel->getInputPacketDimension(rawPacket);
				}
				end();
			}
			
			int ntimes;
			
			
			if(test == 7) {
				clock_gettime( CLOCK_MONOTONIC, &start);
				ntimes = 100000;
				cout << "Start Test 7 ... " << ntimes << " runs " << endl;
				
			}
			
			if(test == 6) {
				clock_gettime( CLOCK_MONOTONIC, &start);
				ntimes = 2;
				cout << "Start Test 6 ... " << ntimes << " runs " << endl;
				
			}
			
			if(test == 5) {
				clock_gettime( CLOCK_MONOTONIC, &start);
				ntimes = 2;
				cout << "Start Test 5 ... " << ntimes << " runs " << endl;
				
			}
			
			if(test == 4) {
				clock_gettime( CLOCK_MONOTONIC, &start);
				ntimes = 50;
				cout << "Start Test 4 ... "  << ntimes << " runs " << endl;
				
			}
			
			if(test == 3) {
				clock_gettime( CLOCK_MONOTONIC, &start);
				ntimes = 500;
				cout << "Start Test 3 ... " << ntimes << " runs " << endl;
				
			}
			
			if(test == 2) {
				clock_gettime( CLOCK_MONOTONIC, &start);
				ntimes = 1000;
				cout << "Start Test 2 ... " << ntimes << " runs " << endl;
				
			}
			
			
			long npacketsrun2 = buffersize * ntimes;
			long npacketsread2 = 0;
			
			
			
			while(npacketsread2 < npacketsrun2) {
				ByteStreamPtr rawPacket = buff.getNext();
				
				dword size = 0;
				size = trtel->getInputPacketDimension(rawPacket);
				totbytes += size;
				
				word npixels = 0;
				word nsamples = 0;
				if(npacketsread2 == 0) {
					trtel->setStream(rawPacket, true);
					word npixels = trtel->getNumberOfPixels();
					int pixel = 0;
					word nsamples = trtel->getNumberOfSamples(pixel);
					cout << npixels << " " << nsamples << endl;
				}
				
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
								
								
								if(calcalg) {
									word npixels = trtel->getNumberOfPixels();
									int pixel = 0;
									word nsamples = trtel->getNumberOfSamples(pixel);
									
									if(activatememorycopy)
										calcWaveformExtraction1(buffermemory, npixels, nsamples, 6);
									else
										calcWaveformExtraction1(camera->stream, npixels, nsamples, 6);
									
								}

								
								/*
								int ssc = trtel->header->getSSC();
								if(npacketsread2 == 0)
									printCamera(c, ssc, npixels, nsamples);
								 */
								//}
								//cout << npixels << " " << nsamples << endl;
								//calcWaveformExtraction1(camera->stream, npixels, nsamples, 6);
								//use it
								//cout << "value of first sample " << c[0] << endl;
								//cout << "Index Of Current Triggered Telescope " << (long) trtel->getIndexOfCurrentTriggeredTelescope() << endl;
								break;
							}
							
						case 4:
							{
								
								//access to a pointer of each pixel of a camera as a single block
								//the decodigin of all the blocks is performed
								trtel->setStream(rawPacket, false);
								word npixels = trtel->getNumberOfPixels();
								int pixel=0;
								//for(pixel=0; pixel<npixels; i++)
								ByteStreamPtr fadc = trtel->getPixelData(pixel);
								
								//3) get a pointer to word
								//word *c = (word*) fadc->stream;
								//cout << "value of first sample " << c[0] << endl;
								break;
							}
							
						case 5:
							{
								//acces to an array of samples
								//the decodigin of all the blocks is performed
								
								trtel->setStream(rawPacket, false);
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
								break;
							}
							
						case 6:
							{
								//access to the single sample of a pixel
								//cout << "decode" << endl;
								trtel->setStream(rawPacket, false);
								//packet data
								cout << "ssc: " << trtel->header->getSSC() << endl;
								word arrayID;
								word runNumberID;
								trtel->header->getMetadata(arrayID, runNumberID);
								cout << "metadata: arrayID " << arrayID << " and runNumberID " << runNumberID << " " << endl;
								cout << "subtype " << trtel->header->getSubType() << endl;
								cout << "eventNumber:" << trtel->getEventNumber() << endl;
								//trigger time
								cout << "Telescope Time " << trtel->header->getTime() << endl;
								//the number of telescopes that have triggered
								cout << "Triggered telescopes: " << (long) trtel->getNumberOfTriggeredTelescopes() << endl;
								//the index (zero-based) of the telescope that has triggerd
								cout << "Index Of Current Triggered Telescope " << (long) trtel->getIndexOfCurrentTriggeredTelescope() << endl;
								//the id of the telescope that has triggered
								cout << "TelescopeId " << trtel->getTelescopeId() << endl;
								word npixels = trtel->getNumberOfPixels();
								int pixel=0;
								word nsamples = trtel->getNumberOfSamples(pixel);
								
								word sample = trtel->getSampleValue(pixel, 0);
								break;

							}
							
						case 7:
							{
								ByteStreamPtr camera = trtel->getCameraData(rawPacket);
								/*
								word npixels;
								npixels = 1141;
								int pixel=0;
								word nsamples;
								nsamples = 40;
								 */
								//cout << npixels << " " << nsamples << endl;
								//cout << camera->getDimension() << endl;

								word *c = (word*) camera->stream;
								//printBuffer(c, npixels, nsamples);
								//exit(0);
								if(activatememorycopy) {
									memcpy(buffermemory, camera->stream, camera->getDimension());
							
								}
								if(calcalg) {
									if(activatememorycopy)
										calcWaveformExtraction1(buffermemory, npixels, nsamples, 6);
									else
										calcWaveformExtraction1(camera->stream, npixels, nsamples, 6);
											
								}
								//cout << "value of first sample " << c[0] << endl;
								break;
							}
							
						 
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
