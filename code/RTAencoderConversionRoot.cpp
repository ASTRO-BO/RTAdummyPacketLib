/***************************************************************************
                          RTAencoderConversionRoot.cpp  -  description
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
#include "CTACameraConv1.h"
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

        /// The Packet containing the conversion value of each telescope
        /// One packet for each telescope        
        RTATelem::CTAStream stream = RTATelem::CTAStream(ctarta + "/share/rtatelem/rta_conv1.xml", "", argv[2]);
        RTATelem::CTACameraConv1* convtel = (RTATelem::CTACameraConv1*) stream.getNewPacket(RTATelem::CTA_CAMERA_CONVERSION_1);
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
        TTree *cal_tree;
	    
	    /// Declaration of leaf types
        Int_t           TelID;
        UInt_t          NPixel;
        Float_t         conv_high[1855];   //[NPixel]
        Float_t         conv_low[1855];   //[NPixel]
	    
        // List of branches
        TBranch        *b_TelID;   //!
        TBranch        *b_NPixel;   //!
        TBranch        *b_conv_high;   //!
        TBranch        *b_conv_low;   //!
        
        root_file = new TFile(argv[1]);
        root_file->ls();
        
        /// Reading the DST tree
        cal_tree = (TTree*)root_file->Get("calibration");
        
        /// Setting the address
        cal_tree->SetBranchAddress("TelID", &TelID, &b_TelID);
	    cal_tree->SetBranchAddress("NPixel", &NPixel, &b_NPixel);
	    cal_tree->SetBranchAddress("conv_high", conv_high, &b_conv_high);
	    cal_tree->SetBranchAddress("conv_low", conv_low, &b_conv_low);
	     	    
        // Conversion run number
        int conversionRun = 1;

        /// Looping in the triggered events
        srand(0);
		long counts = 0;
		unsigned short SSC_index;
		vector<int16_t> SSC_array;
		SSC_array.resize(NTelescopes);
		
		word ssc = 0;
        for(int telindex = 0; telindex<NTelescopes; telindex++) {
            cout << "--" << telindex << endl;
            /// Get entry from the tree
            cal_tree->GetEntry(telindex);
                			
            int TelescopeId = TelID;
            telStruct = ctaconf.getTelescopeStruct(TelescopeId);		
            long telType = (*telStruct).fromTeltoTelType.TelType;
			telTypeStruct = ctaconf.getTelescopeTypeStruct(telType);
			camTypeStruct = ctaconf.getCameraTypeStruct((*telTypeStruct).fromTelTypetoCamType.camType);
            // The attribute stores the number of pixels
            word npixels =  (*camTypeStruct).NPixel;
				
			cout << TelescopeId << " " << telType << " " << npixels << endl;
                
            //set the header of the tm packet
            convtel->header.setAPID(TelescopeId); 	//the data generator (for now, the telescope)
                
            for (int j = 0; j < vectorTelID.size(); j++){
                if (TelescopeId == vectorTelID[j]){
                    SSC_index = j;
                    break;
                }
            }
                
            convtel->header.setSSC(ssc=SSC_array[SSC_index]);	//a unique counter of packets
            cout << "ssc " << ssc << endl;

                
            convtel->header.setMetadata(1, 2);	//the metadata
                
            convtel->header.setSubType(0); //important, for fast packet identification
			
            
            //pedestal information
            convtel->setConversionRun(conversionRun);	//another metadata: the conversion run number (e.g. provided by event builder?)
            convtel->setTelescopeId(TelescopeId);	//the telescope unique id

            //camera information
                
            //set the number of pixels. In this way it is possible to manage different cameras with the same layout

            convtel->setNumberOfCalibrationPixels(npixels);				    

            //set information of the pixels and summing windows
            for(int pixelindex=0; pixelindex<npixels; pixelindex++) {
            	convtel->setConversionHighValue(pixelindex, conv_high[pixelindex]);
            	convtel->setConversionLowValue(pixelindex, conv_low[pixelindex]);
           }
           
               
            //and finally, write the packet to output (in this example, write the output to file)
            stream.writePacket(convtel);
            SSC_array[SSC_index] = SSC_array[SSC_index] + 1;
			counts++;

         		

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
