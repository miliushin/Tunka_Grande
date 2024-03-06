#include <TFile.h>
#include <TROOT.h>
#include <TTree.h>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>

#include "grande.h"
#include "hist.h"

using namespace std;

string DAYS[MAX_OF_DAYS];
string nFDAYS[MAX_OF_DAYS][50];  // files per day

string GetDay(const char *name) {
  int len = strlen(name);
  char Date[7];

  for (int i = 0; i < 30; i++) {
    if (name[len - i] == 'S') {
      for (int k = 0; k < 20; k++) {
        if (name[len - i - 2 - k] == '/') {
          for (int j = 0; j < 6; j++) {
            Date[j] = name[len - i - 1 - k + j];
          }
          Date[6] = 0;
          break;
        }
      }
      break;
    }
  }
  return Date;
}

string GetDirName(const char *name) {
  int len = strlen(name);
  char DirName[20];

  memset(DirName, '\0', 20);
  for (int i = 0; i < 30; i++) {
    if (name[len - i] == 'S') {
      for (int j = 0; j < 30; j++) {
        if (name[len - i - 2 - j] == '/') {
          for (int k = 0; k < 20; k++) {
            if (name[len - i - 1 - j + k] == '/') break;
            DirName[k] = name[len - i - 1 - j + k];
          }
          break;
        }
      }
      break;
    }
  }
  // printf("DDD: %s\n",DirName);
  return DirName;
}

string GetHistDir(const char *name) {
  char hist_dir[200];
  int ii = 0;
  memset(hist_dir, '\0', 200);
  for (int i = strlen(name); i > 0; i--) {
    if (name[i] == '/') {
      ii++;
      if (ii == 2) {
        strncat(hist_dir, name, i);
        break;
      }
    }
  }
  strcat(hist_dir, "/hist");
  return hist_dir;
}

void MakeRoot(const char *name) {
  printf("MakeRoot\n");
  char OutName[12];
  char NstName[4];
  char HisDirName[20];
  OutName[11] = 0;
  NstName[3] = 0;
  sprintf(OutName, "%s.root", name);
  TFile *OutRoot = new TFile(OutName, "RECREATE");
  TTree *TData[NUMBER_OF_STATIONS];

  Int_t NubmerOfEvent;
  Double_t TimeEvent;
  Int_t ADC[NUMBER_OF_CHANNELS][Aperture];
  // Int_t histQ[NUMBER_OF_CHANNELS][2000];
  Double_t METEO[3];
  Int_t Ncounts = Aperture;

  double time_meteo[MAX_OF_METEO_ST];  // Time of meteo data
  double P_meteo[MAX_OF_METEO_ST];     // Pressure
  double H_meteo[MAX_OF_METEO_ST];     // Humidity
  double T_meteo[MAX_OF_METEO_ST];     // Temperature of ...
  double T1_meteo[MAX_OF_METEO_ST];
  double T2_meteo[MAX_OF_METEO_ST];
  double T3_meteo[MAX_OF_METEO_ST];

  double histQ[NUMBER_OF_CHANNELS][2000];
  int histA1A2 = 0;
  int histA2A6 = 0;
  int histA3A4 = 0;
  int histA4A8 = 0;

  for (int i = 0; i < MAX_OF_METEO_ST; i++) {
    time_meteo[i] = 0.;
    P_meteo[i] = 0.;
    H_meteo[i] = 0.;
    T_meteo[i] = 0.;
    T1_meteo[i] = 0.;
    T2_meteo[i] = 0.;
    T3_meteo[i] = 0.;
  }

  for (int i = 0; i < NUMBER_OF_CHANNELS; i++) {
    for (int j = 0; j < 2000; j++) {
      histQ[i][j] = 0.;
    }
  }

  for (int i = 31; i < 31 + NUMBER_OF_STATIONS; i++) {
    sprintf(NstName, "%d", i);
    TData[i - 31] = new TTree(NstName, "TAIGA-GRANDE data");
    TData[i - 31]->Branch("NumberOfEvent", &NubmerOfEvent, "NumberOfEvent/I");
    TData[i - 31]->Branch("TimeEvent", &TimeEvent, "TimeEvent/D");
    TData[i - 31]->Branch("ADC", ADC, "ADC[12][1024]/I");
    TData[i - 31]->Write();
  }

  TTree *TMeteo = new TTree("METEO", "Meteo data of TAIGA-GRANDE");
  TMeteo->Branch("Time", time_meteo, "time_meteo[3]/D");
  TMeteo->Branch("Pressure", P_meteo, "P_meteo[3]/D");
  TMeteo->Branch("Humidity", H_meteo, "H_meteo[3]/D");
  TMeteo->Branch("T", T_meteo, "T_meteo[3]/D");
  TMeteo->Branch("T1", T1_meteo, "T1_meteo[3]/D");
  TMeteo->Branch("T2", T2_meteo, "T2_meteo[3]/D");
  TMeteo->Branch("T3", T3_meteo, "T3_meteo[3]/D");
  TMeteo->Write();
  delete TMeteo;

  OutRoot->mkdir("HIST");
  OutRoot->cd("HIST");
  for (int i = 31; i < 31 + NUMBER_OF_STATIONS; i++) {
    memset(HisDirName, '\0', 20);
    sprintf(HisDirName, "HIST/%d", i);
    OutRoot->mkdir(HisDirName);
    OutRoot->cd(HisDirName);

    TTree *THistQ = new TTree("histQ", "Hist ADC integral of TAIGA-GRANDE");
    THistQ->Branch("histQ", histQ, "histQ[12][2000]/D");
    THistQ->Write();

    TTree *THistCoef =
        new TTree("coef", "ADC conversion coefficiens of TAIGA-GRANDE");
    THistCoef->Branch("histA1A2", &histA1A2, "histA1A2/I");
    THistCoef->Branch("histA2A6", &histA2A6, "histA2A6/I");
    THistCoef->Branch("histA3A4", &histA3A4, "histA3A4/I");
    THistCoef->Branch("histA4A8", &histA4A8, "histA4A8/I");
    THistCoef->Write();

    delete THistQ;
    delete THistCoef;
  }

  OutRoot->Close();

  delete OutRoot;
}

int main(int argc, char **argv) {
  FILE *file_data;
  string file;
  char hist_dir[200];
  ifstream FileList;
  int iDay = 0;
  int iiDay[MAX_OF_DAYS];
  int FHist;
  bool FlagDay;

  for (int i = 0; i < MAX_OF_DAYS; i++) {
    iiDay[i] = 0;
  }

  // FileList opening
  if (argc > 1) {
    FileList.open(argv[1]);

    printf("FileList: %s opened\n", argv[1]);
  } else {
    printf("FileList not found\n");
    exit(1);
  }

  while (getline(FileList, file)) {
    bool Fl;
    FlagDay = false;
    FHist = 1;
    file_data = fopen(file.c_str(), "r");
    printf("File: %s", file.c_str());

    if (file_data != NULL) {
      printf(" opened succefully\n");
    } else {
      printf(" open ERROR\n");
      continue;
    }

    // Read file
    char name_file[255];
    strcpy(name_file, file.c_str());
    if (iDay == 0) {
      DAYS[iDay] = GetDay(name_file);
      nFDAYS[iDay][iiDay[iDay]] = GetDirName(name_file);

      MakeRoot(DAYS[iDay].c_str());
      string name_hist_dir = GetHistDir(name_file);
      HIST *hist = new HIST(DAYS[iDay].c_str());
      FHist = hist->ReadHist(name_hist_dir.c_str());
      delete hist;
      iiDay[iDay]++;
      iDay++;
      Fl = false;
    } else {
      for (int i = 0; i < iDay; i++) {
        if (DAYS[i] == GetDay(name_file)) {
          FlagDay = true;
          break;
        }
      }
      if (FlagDay == false) {
        DAYS[iDay] = GetDay(name_file);
        MakeRoot(DAYS[iDay].c_str());
        iDay++;
      }
      Fl = false;
      for (int i = 0; i < iDay; i++) {
        for (int j = 0; j < iiDay[i]; j++) {
          if (nFDAYS[i][j] == GetDirName(name_file)) {
            Fl = true;
            break;
          }
        }
        if (Fl == true) break;
      }
      if (Fl == false) {
        nFDAYS[iDay - 1][iiDay[iDay - 1]] = GetDirName(name_file);
        string name_hist_dir = GetHistDir(name_file);
        HIST *hist = new HIST(DAYS[iDay - 1].c_str());
        FHist = hist->ReadHist(name_hist_dir.c_str());
        delete hist;
        iiDay[iDay - 1]++;
      }
    }

    GRANDE *grande = new GRANDE(name_file);
    if (Fl == false) {
      grande->ReadMeteo(file);
    }
    grande->ReadADC(file_data, FHist);
    delete grande;

    fclose(file_data);
  }
  return 0;
}