//
//  read_data.hpp
//  TakeMaxAmpl
//
//  Created by Mikhail Iliushin on 10.11.2023.
//

#ifndef read_data_hpp
#define read_data_hpp

#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <TFile.h>
#include <TGraph.h>
#include <TH1F.h>

using namespace std;

#define SIZE_OF_HEADER            24
#define Aperture                  1024
#define NUMBER_OF_CHANNELS        11
#define NUMBER_OF_STATIONS        19
#define NOISE_RANGE               20
#define NOISE_RANGE_FOR_AMP       2

class read_data{

public:
  read_data(const char *dirname);
  ~read_data();
  
private:
  int ReadADC(FILE *data_file, int Nst);
  int GetA_GRANDE(int Nch, int data[Aperture]);
  int GetI_GRANDE(int Nch, int data[Aperture]);
  void FitMaximumAMP();
  void FitMaximumINT();
  
  
  int MaximumAMP[NUMBER_OF_STATIONS][NUMBER_OF_CHANNELS];
  double MaximumINT[NUMBER_OF_STATIONS][NUMBER_OF_CHANNELS];

  double iMaximumINT[NUMBER_OF_STATIONS][NUMBER_OF_CHANNELS];
  
  unsigned int      ID;        // Station indentification
  unsigned int      NumBytes;  // Data packet size
  unsigned int      Delay;     // The optic line
  unsigned int      Claster;
  unsigned int      fin;
  double            TimeEvent;
  unsigned long int END_File;
  unsigned long int i_File;
  double            XAperture[Aperture];                // X-axis of ADC
  double            INT_ADC;                            // Integral of PMT pulse
  int               AMP_ADC;                            // Max amplitude of PMT pulse
  int               ADC[NUMBER_OF_CHANNELS][Aperture];  // ADC data
  
  TH1F *HIST[NUMBER_OF_STATIONS][NUMBER_OF_CHANNELS];
  TH1F *HIST_amp[NUMBER_OF_STATIONS][NUMBER_OF_CHANNELS];
  TFile *RootFile;
  TGraph *GADC;
  
};

#endif /* read_data_hpp */
