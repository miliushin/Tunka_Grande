#ifndef GRANDE_H
#define GRANDE_H

#include <TCanvas.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1F.h>
#include <TTree.h>

#include <fstream>

#include "globals.h"
using namespace std;

class GRANDE {
 public:
  GRANDE(char *name_file);
  ~GRANDE();

  double GetTime(unsigned char ch[8]);
  int ReadADC(FILE *data_file, int FHist);
  int ReadMeteo(string sfile);
  int ReadHist(char *hist_dir);

 private:
  unsigned int ID;        // Station indentification
  unsigned int NumBytes;  // Data packet size
  unsigned int Delay;     // The optic line
  unsigned int Claster;
  unsigned int fin;
  double TimeEvent;
  int ADC[NUMBER_OF_CHANNELS][Aperture];  // ADC data
  unsigned long int END_File;
  unsigned long int i_File;
  char Nst[4];  // Number of station

  Int_t NumEvent;  // Number of event
  TTree *TData;    // Data TTree
  TTree *TMeteo;   // Meteo data
  // TTree *THist;                                   // Hist integral ADC
  TFile *RootFile;  // Output file: ddmmyy.root

  long Addr;
  long AddrBase[16] = {0x80000, 0x80800, 0x84000, 0x84800, 0xA0000, 0xA0800,
                       0xA4000, 0xA4800, 0xC0000, 0xC0800, 0xC4000, 0xC4800,
                       0xE0000, 0xE0800, 0xE4000, 0xE4800};

  double time_meteo[MAX_OF_METEO_ST];  // Time of meteo data
  double P_meteo[MAX_OF_METEO_ST];     // Pressure
  double H_meteo[MAX_OF_METEO_ST];     // Humidity
  double T_meteo[MAX_OF_METEO_ST];     // Temperature of ...
  double T1_meteo[MAX_OF_METEO_ST];
  double T2_meteo[MAX_OF_METEO_ST];
  double T3_meteo[MAX_OF_METEO_ST];

  int GetIA_GRANDE(int Nch, int data[Aperture]);

  TGraph *GADC;
  TF1 *FitFunc;

  double XAperture[Aperture];  // X-axis of ADC
  double INT_ADC;              // Integral of PMT pulse
  int AMP_ADC;                 // Max amplitude of PMT pulse
  double TIME_ADC;             // Time of PMT pulse
};

#endif