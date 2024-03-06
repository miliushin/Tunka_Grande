#ifndef HIST_H
#define HIST_H

#include <TCanvas.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1F.h>
#include <TTree.h>

#include <fstream>

#include "globals.h"
using namespace std;

class HIST {
 public:
  HIST(const char *data);
  ~HIST();

  int ReadHist(const char *hist_dir);

 private:
  TFile *RootFile;

  double ***histQ;
  int **histA1A2;
  int **histA2A6;
  int **histA3A4;
  int **histA4A8;
};

#endif