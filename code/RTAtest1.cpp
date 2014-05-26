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

#define PRINTALG 1

#include <iostream>
#include <stdlib.h>
#include <CTACameraTriggerData1.h>
#include <CTAStream.h>
#include <CTADecoder.h>
#include "mac_clock_gettime.h"
#include <packet/File.h>
#include <packet/PacketBufferV.h>
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
	exit(1);
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
//unsigned short maxres[3000];
//double timeres[3000];

void calcWaveformExtraction1(byte* buffer, int npixels, int nsamples, int ws, unsigned short * maxresext, double * timeresext) {
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
	
	unsigned short* maxres = new unsigned short[npixels];
	double* timeres = new double[npixels];
	//maxresext = maxres;
	//timeresext = time;
	
	
	//word bl[npixels*nsamples];
	//memcpy(bl, b, npixels*nsamples*sizeof(word));
	
	for(int pixel = 0; pixel<npixels; pixel++) {
		word* s = b + pixel * nsamples;
		
#ifdef PRINTALG
		if(flag == 0) {
		
			cout << pixel << " ";
			for(int k=0; k<nsamples; k++)
				cout << s[k] << " ";
			cout << endl;
		}
#endif
		
		unsigned short max = 0;
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
		
		max = sumd;
		if(!iszero(sumd))
			t = sumn / (double)sumd;
		maxt = t;
		maxj = 0;
		
		//cout << sumn << " " << sumd << endl;
		//sumres[0] = sum;
		for(int j=1; j<nsamples-ws; j++) {

			sumn = sumn - s[j-1] * (j-1) + s[j+ws-1] * (j+ws-1);
			sumd = sumd - s[j-1] + s[j+ws-1];
			//cout << sumn << " " << sumd << endl;

			//sumres[j] = sum;
			if(sumd > max) {
				max = sumd;
				if(!iszero(sumd))
					t = sumn / (double)sumd;
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
		timeres[pixel] = maxt;
		
#ifdef PRINTALG
		//>9000
		if(flag == 0) cout << pixel << " " << maxt << " " << maxres[pixel] << " " << timeres[pixel] << " " << endl;
#endif
		/*
		for(int k=0; k<nsamples; k++)
			cout << s[k] << " ";
		cout << endl;
		 */
	}
	//SharedPtr<double> shtime(maxt);
	
	flag++;
	//return maxres;
	//maxresext = maxres;
	//timeresext = timeres;
	memcpy(maxresext, maxres, sizeof(unsigned short) * npixels);
	//memcpy(timeresext, timeres, sizeof(double) * npixels);
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
		int test = 0;
		
		if(argc < 4) {
			cout << "exe file.raw testID(0/7) memcpy(0/1) waveform(0/1)" << endl;
			cout << "where test is:" << endl;
			cout << "Test 0: check data model loading" << endl;
			cout << "Test 1: check the loading of camera data packets" << endl;
			cout << "Test 2: decoding for routing (identification of the type of packet)" << endl;
			cout << "Test 3: access to a pointer of the camera data (all pixels) as a single block (method 1 packetlib)" << endl;
			cout << "Test 4: packetlib access to an array of samples using packetlib to get the block" << endl;
			cout << "Test 5: direct acces to an array of samples using packetlib to get the block" << endl;
			cout << "Test 6: access to header and data field header with packetlib" << endl;
			cout << "Test 7: decoding all the blocks of the packet (method 2 packetlib::bytestream builder)" << endl;
			cout << "Test 8: access to header, data field header and source data field (header)" << endl;
			cout << "Test 9: access to some structural information form source data field (packetlib)" << endl;
			cout << "Test 10: access to some values from source data field (packetlib)" << endl;
			cout << "Test 11: JJ test" << endl;
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
			cout << "Test 3: access to a pointer of the camera data (all pixels) as a single block (method 1 packetlib)" << endl;
		if(test == 4)
			cout << "Test 4: packetlib access to an array of samples using packetlib to get the block" << endl;
		if(test == 5)
			cout << "Test 5: direct acces to an array of samples using packetlib to get the block" << endl;
		if(test == 6)
			cout << "Test 6: access to header and data field header with packetlib" << endl;
		if(test == 7)
			cout << "Test 7: decoding all the blocks of the packet (method 2 packetlib::bytestream builder)" << endl;
		if(test == 8)
			cout << "Test 8: access to header, data field header and source data field (header)" << endl;
		if(test == 9)
			cout << "Test 9: access to some structural information form source data field (packetlib)" << endl;
		if(test == 10)
			cout << "Test 10: access to some values from source data field (packetlib)" << endl;
		if(test == 11)
			cout << "Test 11: JJ test" << endl;
		
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
		string configFileName = "/share/rtatelem/rta_fadc_all.stream";
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

		} else {
        	cerr << "ERROR: Please, provide the .raw" << endl;
        	return 0;
		}

		RTATelem::CTAStream stream(ctarta + "/share/rtatelem/rta_fadc1.stream", argv[1], "");
		RTATelem::CTADecoder decoder(ctarta + "/share/rtatelem/rta_fadc1.stream");

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

			PacketLib::PacketBufferV buff(ctarta + configFileName, argv[1]);
			buff.load();
			buffersize = buff.size();
			cout << "Loaded " << buffersize << " packets " << endl;
			totbytes = 0;
			
			if(test == 1) {
				for(long i=0; i<buffersize; i++) {
					ByteStreamPtr rawPacket = buff.getNext();
					totbytes += stream.getInputPacketDimension(rawPacket);
				}
				end();
			}

			int ntimes;
			
			if(test == 11) {
				ntimes = 1;
				cout << "Start Test 11 ... " << ntimes << " runs " << endl;
			}
			if(test == 10) {
				ntimes = 1;
				cout << "Start Test 10 ... " << ntimes << " runs " << endl;
			}
			if(test == 9) {
				ntimes = 10;
				cout << "Start Test 9 ... " << ntimes << " runs " << endl;
			}
			if(test == 8) {
				ntimes = 500;
				cout << "Start Test 8 ... " << ntimes << " runs " << endl;
			}
			if(test == 7) {
				ntimes = 5000;
				cout << "Start Test 7 ... " << ntimes << " runs " << endl;
			}
			if(test == 6) {
				ntimes = 2;
				cout << "Start Test 6 ... " << ntimes << " runs " << endl;
			}
			if(test == 5) {
				ntimes = 2;
				cout << "Start Test 5 ... " << ntimes << " runs " << endl;
			}
			if(test == 4) {
				ntimes = 1;
				cout << "Start Test 4 ... "  << ntimes << " runs " << endl;
			}
			if(test == 3) {
				ntimes = 2;
				cout << "Start Test 3 ... " << ntimes << " runs " << endl;
			}
			if(test == 2) {
				ntimes = 1000;
				cout << "Start Test 2 ... " << ntimes << " runs " << endl;
			}

			clock_gettime( CLOCK_MONOTONIC, &start);
			
			long npacketsrun2 = buffersize * ntimes;
			long npacketsread2 = 0;
			
			word npixels = 0;
			word nsamples = 0;
			
			while(npacketsread2 < npacketsrun2) {

				ByteStreamPtr rawPacket = buff.getNext();
				RTATelem::CTAPacket& packet = decoder.getPacket(rawPacket);

				dword size = 0;
				size = decoder.getInputPacketDimension(rawPacket);
				totbytes += size;
				
				enum RTATelem::CTAPacketType type = packet.getPacketType();
				
				//cout << "Packet #" << npacketsread2 << " size: " << size << " byte. type: " << type << endl;
				if(type == RTATelem::CTA_CAMERA_TRIGGERDATA_1) {

					RTATelem::CTACameraTriggerData1& trtel = (RTATelem::CTACameraTriggerData1&) packet;

					
					//if(npacketsread2 == 0) {
						//trtel.decode(true);
						//npixels = trtel.getNumberOfPixels();
						//int pixel = 0;
						//nsamples = trtel.getNumberOfSamples(0);
						//trtel.getNumberOfTriggeredTelescopes();
						//cout << trtel.getTelescopeId() << " " << npixels << " " << nsamples << " " << endl;
						//npixels = 1141; nsamples = 40;
					//}
					switch(test) {
						case 3:
						{
							//access to a pointer of the camera data (all pixels) as a single block
							trtel.decode(true);
							npixels = trtel.getNumberOfPixels();
							nsamples = trtel.getNumberOfSamples(0);
							
							//word subtype = trtel.header.getSubType();
							ByteStreamPtr camera = trtel.getCameraDataSlow();
							//cout << camera->size() << endl;
//							word *c = (word*) camera->stream;
							if(activatememorycopy) {
								memcpy(buffermemory, camera->stream, camera->size());
							}

							if(calcalg) {
								//word npixels = trtel.getNumberOfPixels();
								//int pixel = 0;
								//word nsamples = trtel.getNumberOfSamples(pixel);
								//cout << npixels << " " << nsamples << endl;
								unsigned short * maxres = new unsigned short[npixels];
								double* timeres = new double[npixels];
								if(activatememorycopy)
									calcWaveformExtraction1(buffermemory, npixels, nsamples, 6, maxres, timeres);
								else
									calcWaveformExtraction1(camera->stream, npixels, nsamples, 6, maxres, timeres);
								delete[] maxres;
								delete[] timeres;
							}

							break;
						}

						case 4:
						{
							//packetlib access to an array of samples using packetlib to get the block
							trtel.decode(true);
							npixels = trtel.getNumberOfPixels();
							nsamples = trtel.getNumberOfSamples(0);
#ifdef PRINTALG
							cout << npixels << " " << nsamples << " ";
#endif
							//
							
							byte ntrigtel = trtel.getNumberOfTriggeredTelescopes();
							
							dword evtnum = trtel.getEventNumber();
							
							
							
							Packet* p = trtel.getPacket();
#ifdef PRINTALG
							cout << (int) ntrigtel << " " << (int) evtnum << endl;
							cout << "packet size: " << p->size() << endl;
							cout << "packet size2: " << p->getPacketHeader()->getFieldValue_32ui(6) << endl;
#endif
							int pixel = 0;
							for(int i=0; i<nsamples; i++) {
								word sample = trtel.getSampleValue(pixel, i);
#ifdef PRINTALG
								cout << sample << " ";
#endif
							}
#ifdef PRINTALG
							cout << endl;
#endif
							pixel = npixels - 1;
							for(int i=0; i<nsamples; i++) {
								word sample = trtel.getSampleValue(pixel, i);
#ifdef PRINTALG
								cout << sample << " ";
#endif
							}
#ifdef PRINTALG
							cout << endl;
							cout << "---" << endl;
#endif
							break;
						}
							
						case 5:
						{
							//direct acces to an array of samples using packetlib to get the block
							trtel.decode(true);
							npixels = trtel.getNumberOfPixels();
							nsamples = trtel.getNumberOfSamples(0);
							
							int pixel = 0;
							ByteStreamPtr samplebs = trtel.getPixelData(pixel);
							word* sample = (word*) samplebs->stream;
							word s;
							for(int i=0; i < nsamples; i++) {
								s = sample[i];
#ifdef PRINTALG
								cout <<  s << " ";
#endif
							}
#ifdef PRINTALG
							cout << endl;
#endif
							pixel = npixels - 1;
							samplebs = trtel.getPixelData(pixel);
							sample = (word*) samplebs->stream;
							for(int i=0; i<nsamples; i++) {
								s = sample[i];
#ifdef PRINTALG
								cout << s << " ";
#endif
							}
#ifdef PRINTALG
							cout << endl;
							cout << "---" << endl;
#endif
							break;
						}
							
						case 6:
						{
							//access to header and data field header with packetlib
							trtel.decode(true);
							npixels = trtel.getNumberOfPixels();
							nsamples = trtel.getNumberOfSamples(0);

							word arrayID;
							word runNumberID;
							word ssc;

							trtel.header.getMetadata(arrayID, runNumberID);
							ssc = trtel.header.getSSC();
							word subtype = trtel.header.getSubType();
							double time = trtel.header.getTime();
#ifdef PRINTALG
							cout << "ssc: " << ssc << endl;
							cout << "metadata: arrayID " << arrayID << " and runNumberID " << runNumberID << " " << endl;
							cout << "subtype " << subtype  << endl;
							//trigger time
							cout << "Telescope Time " << time << endl;
#endif
							break;
						}
							
						case 7:
						{
							//access to blocks using only ByteStream
							ByteStreamPtr camera = trtel.getCameraData(rawPacket);
							/*
							word npixels;
							npixels = 1141;
							int pixel=0;
							word nsamples;
							nsamples = 40;
							*/
							//cout << npixels << " " << nsamples << endl;
							//cout << camera->size() << endl;

//							word *c = (word*) camera->stream;
							//printBuffer(c, npixels, nsamples);
							//exit(0);
							if(activatememorycopy) {
								memcpy(buffermemory, camera->stream, camera->size());
							}
							if(calcalg) {
								unsigned short * maxres = new unsigned short[npixels];
								double* timeres = new double[npixels];
								if(activatememorycopy)
									calcWaveformExtraction1(buffermemory, npixels, nsamples, 6, maxres, timeres);
								else
									calcWaveformExtraction1(camera->stream, npixels, nsamples, 6, maxres, timeres);
								delete[] maxres;
								delete[] timeres;
							}
							//cout << "value of first sample " << c[0] << endl;
							break;
						}
						case 8:
						{
							//access to header, data field header and source data field (header)
							trtel.decode(true);
							npixels = trtel.getNumberOfPixels();
							nsamples = trtel.getNumberOfSamples(0);
							
							word arrayID;
							word runNumberID;
							word ssc;
							
							trtel.header.getMetadata(arrayID, runNumberID);
							ssc = trtel.header.getSSC();
							word subtype = trtel.header.getSubType();
							double time = trtel.header.getTime();
							
							word ntt = trtel.getNumberOfTriggeredTelescopes();
							word tt = trtel.getIndexOfCurrentTriggeredTelescope();
							word telid = trtel.getTelescopeId();
							word evtnum =  trtel.getEventNumber();
#ifdef PRINTALG
							cout << "ssc: " << ssc << endl;
							cout << "metadata: arrayID " << arrayID << " and runNumberID " << runNumberID << " " << endl;
							cout << "subtype " << subtype  << endl;
							cout << "eventNumber:" << evtnum << endl;
							//trigger time
							cout << "Telescope Time " << time << endl;
							//the number of telescopes that have triggered
							cout << "Triggered telescopes: " <<  ntt << endl;
							//the index (zero-based) of the telescope that has triggerd
							cout << "Index Of Current Triggered Telescope " << tt << endl;
							//the id of the telescope that has triggered
							cout << "TelescopeId " << telid << endl;
#endif
							break;
							
						}

						case 9:
						{
							//access to some structural information form source data field (packetlib)
							trtel.decode(true);
							npixels = trtel.getNumberOfPixels();
							nsamples = trtel.getNumberOfSamples(0);
							
							word npixels = trtel.getNumberOfPixels();
							int pixel=0;
							word nsamples = trtel.getNumberOfSamples(pixel);
#ifdef PRINTALG
							cout << npixels << " " << nsamples << endl;
#endif
							break;
						}

						case 10:
						{
							//access to some values form source data field (packetlib)
							trtel.decode(true);
							npixels = trtel.getNumberOfPixels();
							nsamples = trtel.getNumberOfSamples(0);
							
							trtel.getSampleValue(0, 0);
							/*
							for(int pixel=0; pixel<npixels; pixel++)
								for(int sample=0; sample < nsamples; sample++)
									trtel.getSampleValue(pixel, sample);
							*/
							break;
						}
						case 11:
						{
							int foo [] = { 16, 2, 77, 40, 12071 };
							//byte* pixelJJ = new byte[64];
							word pixelJJ [] = {15,23,36,48,58,60,49,33,22,10,5,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
							unsigned short * maxres = new unsigned short[npixels];
							double* timeres = new double[npixels];
							calcWaveformExtraction1((byte*)pixelJJ, 1, 64, 9, maxres, timeres);
							delete[] maxres;
							delete[] timeres;
							/*
							 for(int pixel=0; pixel<npixels; pixel++)
							 for(int sample=0; sample < nsamples; sample++)
							 trtel.getSampleValue(pixel, sample);
							 */
							break;
						}
					}
				}

				npacketsread2++;
			}
			
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
			if(test == 8)
				end(ntimes);
			if(test == 9)
				end(ntimes);
			if(test == 10)
				end(ntimes);
			if(test == 11)
				end(ntimes);
			
		} catch(PacketException* e) {
	        cout << e->geterror() << endl;
		}
    }
    catch(PacketExceptionIO* e)
    {
        cout << e->geterror() << endl;;
    }
    catch(PacketException* e)
    {
        cout << e->geterror() << endl;
    }

	return 0;
}
