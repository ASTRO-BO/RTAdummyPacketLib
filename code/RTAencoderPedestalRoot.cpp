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
#include "CTACameraPedestal1.h"
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
        RTATelem::CTAStream stream = RTATelem::CTAStream(ctarta + "/share/rtatelem/rta_ped1.xml", "", argv[2]);
        RTATelem::CTACameraPedestal1* pedtel = (RTATelem::CTACameraPedestal1*) stream.getNewPacket(RTATelem::CTA_CAMERA_PEDESTAL_1);
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
        UInt_t          num_sumwindow;
        UInt_t          sumwindow[30];   //[num_sumwindow]
        Float_t         ped_high[1855];   //[NPixel]
        Float_t         *pedvar_high = new Float_t[1450000];
        Float_t         ped_low[1855];   //[NPixel]
        Float_t         *pedvar_low = new Float_t[1450000];
        //Float_t         conv_high[1855];   //[NPixel]
        //Float_t         conv_low[1855];   //[NPixel]
        Float_t         tzero[1855];   //[NPixel]
	    
        // List of branches
        TBranch        *b_TelID;   //!
        TBranch        *b_NPixel;   //!
        TBranch        *b_num_sumwindow;   //!
        TBranch        *b_sumwindow;   //!
        TBranch        *b_ped_high;   //!
        TBranch        *b_pedvar_high;   //!
        TBranch        *b_ped_low;   //!
        TBranch        *b_pedvar_low;   //!
        //TBranch        *b_conv_high;   //!
        //TBranch        *b_conv_low;   //!
        TBranch        *b_tzero;   //!
        
        root_file = new TFile(argv[1]);
        root_file->ls();
        
        /// Reading the DST tree
        cal_tree = (TTree*)root_file->Get("calibration");
        
        /// Setting the address
        cal_tree->SetBranchAddress("TelID", &TelID, &b_TelID);
	    cal_tree->SetBranchAddress("NPixel", &NPixel, &b_NPixel);
 	    cal_tree->SetBranchAddress("num_sumwindow", &num_sumwindow, &b_num_sumwindow);
	    cal_tree->SetBranchAddress("sumwindow", sumwindow, &b_sumwindow); 	    
	    cal_tree->SetBranchAddress("ped_high", ped_high, &b_ped_high);
	    cal_tree->SetBranchAddress("pedvar_high", pedvar_high, &b_pedvar_high);
	    cal_tree->SetBranchAddress("ped_low", ped_low, &b_ped_low);
	    cal_tree->SetBranchAddress("pedvar_low", pedvar_low, &b_pedvar_low);
	    //cal_tree->SetBranchAddress("conv_high", conv_high, &b_conv_high);
	    //cal_tree->SetBranchAddress("conv_low", conv_low, &b_conv_low);
	    cal_tree->SetBranchAddress("tzero", tzero, &b_tzero);
	     	    
        // Pedestal run number
        int pedestalRun = 1;

        /// Looping in the triggered events
        srand(0);
		long counts = 0;
		unsigned short SSC_index;
		vector<int16_t> SSC_array;
		SSC_array.resize(NTelescopes);
		
		word ssc = 0;
		int el_id = 0;
        int tot_pedvar = 0;
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
				
			cout << TelescopeId << " " << telType << " " << npixels << " " << num_sumwindow << endl;
                
            //set the header of the tm packet
            pedtel->header.setAPID(TelescopeId); 	//the data generator (for now, the telescope)
                
            for (int j = 0; j < vectorTelID.size(); j++){
                if (TelescopeId == vectorTelID[j]){
                    SSC_index = j;
                    break;
                }
            }
                
            pedtel->header.setSSC(ssc=SSC_array[SSC_index]);	//a unique counter of packets
            cout << "ssc " << ssc << endl;

                
            pedtel->header.setMetadata(1, 2);	//the metadata
                
            pedtel->header.setSubType(0); //important, for fast packet identification
			
            
            //pedestal information
            pedtel->setPedestalRun(pedestalRun);	//another metadata: the pedestal run number (e.g. provided by event builder?)
            pedtel->setTelescopeId(TelescopeId);	//the telescope unique id

            //camera information
                
            //set the number of pixels and summing windows. In this way it is possible to manage different cameras with the same layout

            pedtel->setNumberOfPixels(npixels);				    

            //set information of the pixels and summing windows
            for(int pixelindex=0; pixelindex<npixels; pixelindex++) {
 		    	pedtel->setNumberSummingWindows(pixelindex, num_sumwindow);           
            	pedtel->setPedestalHighValue(pixelindex, ped_high[pixelindex]);
            	pedtel->setPedestalLowValue(pixelindex, ped_low[pixelindex]);
            	pedtel->setTimeZero(pixelindex, tzero[pixelindex]);
                
                for(int sumWindIndex=0; sumWindIndex<num_sumwindow; sumWindIndex++) {
                
                	el_id = pixelindex*500 + sumWindIndex;
                	int val_high = pedvar_high[el_id];
                	int val_low = pedvar_low[el_id];
                	pedtel->setPedVarHigh(pixelindex, sumWindIndex, val_high);
                	pedtel->setPedVarLow(pixelindex, sumWindIndex, val_low);
                                
                }
            }
           
            for(int sumWindIndex=0; sumWindIndex<num_sumwindow; sumWindIndex++) {
                
                	pedtel->setSumWindows(sumWindIndex, sumwindow[sumWindIndex]);
                                
            }
           
                
            //and finally, write the packet to output (in this example, write the output to file)
            stream.writePacket(pedtel);
            SSC_array[SSC_index] = SSC_array[SSC_index] + 1;
			counts++;

         		

        }
        
        t = clock() - t;
        //printf ("It took me %d clicks (%f seconds).\n",t,((float)t)/CLOCKS_PER_SEC);
        cout << "END " << counts << endl;
        
        delete pedvar_high;
        delete pedvar_low;
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
