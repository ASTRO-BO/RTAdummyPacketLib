/***************************************************************************
                          RTAencoderRoot.cpp  -  description
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
#include "CTAStream.h"
#include "CTACameraTriggerData1.h"
#include "RTAConfigLoad.h"
#include <time.h>
#include <math.h>

// ROOT library
#include <TFile.h>
#include <TTree.h>

using namespace std;


/// Writing the Packet
int main(int argc, char *argv[])
{
    try
    {
        clock_t t;
        
        string ctarta;
        const char* home = getenv("CTARTA");
        if(argc > 2) {
        	/// The Packet containing the FADC value of each triggered telescope
         	if (!home)
        	{
        	   std::cerr << "CTARTA environment variable is not defined." << std::endl;
        	   return 0;
        	}

        	ctarta = home;
        } else {            
            if(argc == 1){
        	     cerr << "Please, provide the names of the input and output files" << endl;
        	     return 0;
            }
            if(argc == 2){
        	     cerr << "Please, provide the name of the .raw file" << endl;
        	     return 0;
            }
        }

        /// The Packet containing the FADC value of each triggered telescope
        /// One packet for each triggered telescope
        RTATelem::CTAStream stream = RTATelem::CTAStream(ctarta + "/share/rtatelem/rta_fadc1.stream", "", argv[2]);
        RTATelem::CTACameraTriggerData1* trtel = (RTATelem::CTACameraTriggerData1*) stream.getNewPacket(RTATelem::CTA_CAMERA_TRIGGERDATA_1);
		RTAConfig::RTAConfigLoad ctaconf( ctarta + "/share/rtatelem/PROD2_telconfig.fits.gz" );


        /// METADATA
        /// Read telescope configuration     
		struct RTAConfig::RTAConfigLoad::Array *arrStruct = ctaconf.getArrayStruct();
		struct RTAConfig::RTAConfigLoad::Telescope *telStruct;
		struct RTAConfig::RTAConfigLoad::TelescopeType *telTypeStruct;
		struct RTAConfig::RTAConfigLoad::CameraType *camTypeStruct;
		cout << "--------- Telescope Metadata ---------" << endl;
		int arrType = (*arrStruct).ArrayID;
		cout << "Array type: " << arrType << endl;
		unsigned int NTelescopes = (*arrStruct).NTel;
		cout << "Array N telescopes: " << NTelescopes << endl;
		vector<int16_t> vectorTelID = (*arrStruct).vecTelID;



	    /// Declaration of file and tree types
        TFile *root_file;
        TTree *dst_tree;
	    
	    /// Declaration of leaf types
	    UInt_t          ntel_data;
	    UInt_t          tel_data[35];
	    Float_t         LTtime[35];
	    UInt_t          eventNumber;
        UShort_t        numSamples[35];	    
	    UShort_t        Trace[35][500][2900];
	    
	    /// List of branches
	    TBranch        *b_ntel_data;
	    TBranch        *b_tel_data;
	    TBranch        *b_LTtime;
	    TBranch        *b_eventNumber;
        TBranch        *b_numSamples;
        TBranch        *b_Trace;
        	    
        root_file = new TFile(argv[1]);
        root_file->ls();
        
        /// Reading the DST tree
        dst_tree = (TTree*)root_file->Get("dst");
        
        /// Setting the address
        dst_tree->SetBranchAddress("ntel_data", &ntel_data, &b_ntel_data);
	    dst_tree->SetBranchAddress("tel_data", tel_data, &b_tel_data);
 	    dst_tree->SetBranchAddress("LTtime", LTtime, &b_LTtime);
	    dst_tree->SetBranchAddress("eventNumber", &eventNumber, &b_eventNumber); 	    
	    dst_tree->SetBranchAddress("numSamples", numSamples, &b_numSamples);
	    //dst_tree->SetBranchAddress("Trace", Trace, &b_Trace);
	     	    
        ///Number of events
        //int numberOfEvent = dst_tree->GetEntriesFast();
        int numberOfEvent = 100;
        cout << numberOfEvent << endl;
        /// Looping in the triggered events
        srand(0);
		long counts = 0;
		unsigned short SSC_index;
		vector<int16_t> SSC_array;
		SSC_array.resize(NTelescopes);
		
		word ssc = 0;
        for(int evtindex = 0; evtindex<numberOfEvent; evtindex++) {
            cout << "--" << evtindex << endl;
            /// Get entry from the tree
            dst_tree->GetEntry(evtindex);
                
            /// The attribute stores the number of triggered telescopes for each event                
            int numberOfTriggeredTelescopes = ntel_data;
            cout << numberOfTriggeredTelescopes << endl;
            //for each triggere telescope, generate a telemetry packet
            for(int telindex = 0; telindex<numberOfTriggeredTelescopes; telindex++) {
				
				
                int TelescopeId = tel_data[telindex];
                telStruct = ctaconf.getTelescopeStruct(TelescopeId);		
                long telType = (*telStruct).fromTeltoTelType.TelType;
			    telTypeStruct = ctaconf.getTelescopeTypeStruct(telType);
			    camTypeStruct = ctaconf.getCameraTypeStruct((*telTypeStruct).fromTelTypetoCamType.camType);
                // The attribute stores the number of pixels
                word npixels =  (*camTypeStruct).NPixel;
                word nsamples = numSamples[telindex];
				
				cout << TelescopeId << " " << telType << " " << npixels << " " << nsamples << endl;
                
                //set the header of the tm packet
                trtel->header->setAPID(TelescopeId); 	//the data generator (for now, the telescope)
                
                for (int j = 0; j < vectorTelID.size(); j++){
                   if (TelescopeId == vectorTelID[j]){
                      SSC_index = j;
                      break;
                   }
                }
                
                trtel->header->setSSC(ssc=	SSC_array[SSC_index]);	//a unique counter of packets
                cout << "ssc " << ssc << endl;
                
                trtel->header->setMetadata(1, 2);	//the metadata
                trtel->header->setTime(LTtime[telindex]);	//the time
                
                trtel->header->setSubType(nsamples); //important, for fast packet identification
				
                //event information
                trtel->setEventNumber(eventNumber);	//another metadata: the event number (e.g. provided by event builder)
                trtel->setNumberOfTriggeredTelescopes(numberOfTriggeredTelescopes);	//the number of triggere telescopes (provided by the event builder)
                trtel->setIndexOfCurrentTriggeredTelescope(telindex);	//an internal index of the telescope within the event. This should be used
                                        //to check data loss
                trtel->setTelescopeId(TelescopeId);	//the telescope that has triggered	

                //camera information
                
                //set the number of pixels and samples. In this way it is possible to manage different cameras with the same layout
                // The attribute stores the number of samples

                trtel->setNumberOfPixels(npixels);
				
				trtel->setNumberOfPixelsID(0);

                //set information of the pixels and sample
                for(int pixelindex=0; pixelindex<npixels; pixelindex++) {
                    //trtel->setPixelId(pixelindex, pixelindex);
                    trtel->setNumberOfSamples(pixelindex, nsamples);
					if(counts == 0) cout << pixelindex << " ";
                    for(int sampleindex=0; sampleindex<nsamples; sampleindex++) {
                    	int val = (int)(rand() % 255);
                    	//int val = Trace[telindex][sampleindex][pixelindex];
                        trtel->setSampleValue(pixelindex, sampleindex, val);
						if(counts == 0) cout << val << " ";
                    }
					if(counts == 0) cout << endl;
                }

                //and finally, write the packet to output (in this example, write the output to file)
                stream.writePacket(trtel);
				//ssc++;
                SSC_array[SSC_index] = SSC_array[SSC_index] + 1;
				counts++;

                //just for check, write the content of the packet to stdout
                //if(evtindex == 0) trtel->printPacket_output();
                
/*
                //set the header of the tm packet
                //trtel->header->setAPID(telindex); 	//the data generator (for now, the telescope)
                //trtel->header->setSSC(ssc);	//a unique counter of packets
                //trtel->header->setMetadata(1, 2);	//the metadata
                //trtel->header->setTime(1500+ssc);	//the time
                //word nsamples = 40;
                //trtel->header->setSubType(nsamples); //important, for fast packet identification
				
                //event information
                //int evnum = 10;
                //trtel->setEventNumber(evnum);	//another metadata: the event number (e.g. provided by event builder)
                //trtel->setNumberOfTriggeredTelescopes(numberOfTriggeredTelescopes);	//the number of triggere telescopes (provided by the event builder)
                //trtel->setIndexOfCurrentTriggeredTelescope(telindex);	//an internal index of the telescope within the event. This should be used
                                        //to check data loss
                //trtel->setTelescopeId(telindex*10+5);	//the telescope that has triggered	

                //camera information
                
                //set the number of pixels and samples. In this way it is possible to manage different cameras with the same layout
                // The attribute stores the number of pixels
                //word npixels = 1141;
                // The attribute stores the number of samples

                trtel->setNumberOfPixels(npixels);
				cout << "ssc " << ssc << endl;
				trtel->setNumberOfPixelsID(0);

                //set information of the pixels and sample
                for(int pixelindex=0; pixelindex<npixels; pixelindex++) {
                    //trtel->setPixelId(pixelindex, pixelindex);
                    trtel->setNumberOfSamples(pixelindex, nsamples);
					if(counts == 0) cout << pixelindex << " ";
                    for(int sampleindex=0; sampleindex<nsamples; sampleindex++) {
                    	int val = (int)(rand() % 255);
                        trtel->setSampleValue(pixelindex, sampleindex, val);
						if(counts == 0) cout << val << " ";
                    }
					if(counts == 0) cout << endl;
                }

                //and finally, write the packet to output (in this example, write the output to file)
                stream.writePacket(trtel);
				ssc++;
				counts++;
*/
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
