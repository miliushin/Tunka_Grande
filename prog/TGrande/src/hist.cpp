#include "hist.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#include <cstdlib>

HIST::HIST(const char *data) {
  char OutName[12];
  memset(OutName, '\0', 12);
  sprintf(OutName, "%s.root", data);
  RootFile = new TFile(OutName, "UPDATE");
}

HIST::~HIST() {
  RootFile->Close();
  delete RootFile;
}

int HIST::ReadHist(const char *hist_dir) {
  FILE *FileHist;
  DIR *hdir;
  struct dirent *hist_env;
  char str[150];
  char file[300];
  char cNst[3];
  char TreePathQ[30];
  char TreePathC[30];
  char RootDir[30];
  int Nst;
  int por, ch, code, valQ, nevent, Q1, Q2;
  double hQ[NUMBER_OF_CHANNELS][2000];
  int hA1A2, hA2A6, hA3A4, hA4A8;
  int nHistC[NUMBER_OF_STATIONS];
  int nHist = 0;
  int nHistQ[NUMBER_OF_STATIONS][NUMBER_OF_CHANNELS];
  bool FlagHist = false;

  histQ = new double **[NUMBER_OF_STATIONS];
  histA1A2 = new int *[NUMBER_OF_STATIONS];
  histA2A6 = new int *[NUMBER_OF_STATIONS];
  histA3A4 = new int *[NUMBER_OF_STATIONS];
  histA4A8 = new int *[NUMBER_OF_STATIONS];

  for (int i = 0; i < NUMBER_OF_STATIONS; i++) {
    nHistC[i] = 0;
    histQ[i] = new double *[NUMBER_OF_CHANNELS];
    histA1A2[i] = new int[MAX_OF_EVEN];
    histA2A6[i] = new int[MAX_OF_EVEN];
    histA3A4[i] = new int[MAX_OF_EVEN];
    histA4A8[i] = new int[MAX_OF_EVEN];
    for (int j = 0; j < NUMBER_OF_CHANNELS; j++) {
      nHistQ[i][j] = 0;
      histQ[i][j] = new double[2000];
      for (int k = 0; k < 2000; k++) {
        histQ[i][j][k] = 0.;
      }
    }
    for (int j = 0; j < MAX_OF_EVEN; j++) {
      histA1A2[i][j] = 0;
      histA2A6[i][j] = 0;
      histA3A4[i][j] = 0;
      histA4A8[i][j] = 0;
    }
  }

  hdir = opendir(hist_dir);

  while ((hist_env = readdir(hdir)) != NULL) {
    memset(file, '\0', 300);
    memset(cNst, '\0', 3);
    sprintf(file, "%s/%s", hist_dir, hist_env->d_name);

    int j = 0;
    for (int i = 0; i < 300; i++) {
      j++;
      if (file[i] == '\0') {
        break;
      }
    }

    if (file[j - 2] == 'Q' && file[j - 3] == 't') {
      FlagHist = true;
      cNst[0] = file[j - 8];
      cNst[1] = file[j - 7];
      Nst = atoi(cNst) - 31;

      if ((FileHist = fopen(file, "r")) != NULL) {
        while (!feof(FileHist)) {
          if (fgets(str, 100, FileHist)) {
            fscanf(FileHist, "%d %d %d %d", &por, &ch, &code, &valQ);
            if (code < 2000 && ch < 12 && code != 0) {
              histQ[Nst][ch - 1][code] += valQ;
              nHistQ[Nst][ch - 1]++;
            }
            // printf("%d %d %d %d\n",por,ch,code,histQ);
          }
        }
        fclose(FileHist);
      } else {
        printf("ERROR: %s don't open\n", file);
      }
    }

    if (file[j - 2] == '2' && file[j - 4] == '1') {
      cNst[0] = file[j - 8];
      cNst[1] = file[j - 7];
      Nst = atoi(cNst);

      if ((FileHist = fopen(file, "r")) != NULL) {
        nHist = 0;
        while (!feof(FileHist)) {
          if (fgets(str, 100, FileHist)) {
            fscanf(FileHist, "%d %d %d %d", &por, &nevent, &Q1, &Q2);
            double QQ = (double)Q1 / ((double)Q2);
            if (QQ > 0.) {
              histA1A2[Nst - 31][nHist] = QQ;
              nHist++;
              nHistC[Nst - 31]++;
              if (nHist > MAX_OF_EVEN) {
                printf("ERROR: number of hist more MAX_OF_EVEN, %d\n", nHist);
              }
            }
            // printf("%d %d %d %d\n",por,ch,code,histQ);
          }
        }
        fclose(FileHist);
      } else {
        printf("ERROR: %s don't open\n", file);
      }
    }

    if (file[j - 2] == '6' && file[j - 4] == '2') {
      cNst[0] = file[j - 8];
      cNst[1] = file[j - 7];
      Nst = atoi(cNst);

      if ((FileHist = fopen(file, "r")) != NULL) {
        nHist = 0;
        while (!feof(FileHist)) {
          if (fgets(str, 100, FileHist)) {
            fscanf(FileHist, "%d %d %d %d", &por, &nevent, &Q1, &Q2);
            double QQ = (double)Q1 / ((double)Q2);
            if (QQ > 0.) {
              histA2A6[Nst - 31][nHist] = QQ;
              nHist++;
              if (nHist > MAX_OF_EVEN) {
                printf("ERROR: number of hist more MAX_OF_EVEN, %d\n", nHist);
              }
            }
          }
        }
        fclose(FileHist);
      } else {
        printf("ERROR: %s don't open\n", file);
      }
    }

    if (file[j - 2] == '4' && file[j - 4] == '3') {
      cNst[0] = file[j - 8];
      cNst[1] = file[j - 7];
      Nst = atoi(cNst);

      if ((FileHist = fopen(file, "r")) != NULL) {
        nHist = 0;
        while (!feof(FileHist)) {
          if (fgets(str, 100, FileHist)) {
            fscanf(FileHist, "%d %d %d %d", &por, &nevent, &Q1, &Q2);
            double QQ = (double)Q1 / ((double)Q2);
            if (QQ > 0.) {
              histA3A4[Nst - 31][nHist] = QQ;
              nHist++;
              if (nHist > MAX_OF_EVEN) {
                printf("ERROR: number of hist more MAX_OF_EVEN, %d\n", nHist);
              }
            }
          }
        }
        fclose(FileHist);
      } else {
        printf("ERROR: %s don't open\n", file);
      }
    }

    if (file[j - 2] == '8' && file[j - 4] == '4') {
      cNst[0] = file[j - 8];
      cNst[1] = file[j - 7];
      Nst = atoi(cNst);

      if ((FileHist = fopen(file, "r")) != NULL) {
        nHist = 0;
        while (!feof(FileHist)) {
          if (fgets(str, 100, FileHist)) {
            fscanf(FileHist, "%d %d %d %d", &por, &nevent, &Q1, &Q2);
            double QQ = (double)Q1 / ((double)Q2);
            if (QQ > 0.) {
              histA4A8[Nst - 31][nHist] = QQ;
              nHist++;
              if (nHist > MAX_OF_EVEN) {
                printf("ERROR: number of hist more MAX_OF_EVEN, %d\n", nHist);
              }
            }
          }
        }

        fclose(FileHist);
      } else {
        printf("ERROR: %s don't open\n", file);
      }
    }
  }

  // if hist does not exist
  if (FlagHist == false) {
    for (int j = 0; j < NUMBER_OF_STATIONS; j++) {
      delete[] histA1A2[j];
      delete[] histA2A6[j];
      delete[] histA3A4[j];
      delete[] histA4A8[j];
    }

    delete[] histA1A2;
    delete[] histA2A6;
    delete[] histA3A4;
    delete[] histA4A8;

    for (int i = 0; i < NUMBER_OF_STATIONS; i++) {
      for (int j = 0; j < NUMBER_OF_CHANNELS; j++) {
        delete[] histQ[i][j];
      }
      delete[] histQ[i];
    }
    delete[] histQ;

    return 1;
  }

  // write hist to file
  for (int i = 31; i < 31 + NUMBER_OF_STATIONS; i++) {
    memset(TreePathQ, '\0', 30);
    memset(TreePathC, '\0', 30);
    memset(RootDir, '\0', 30);

    sprintf(TreePathQ, "HIST/%d/histQ", i);
    sprintf(TreePathC, "HIST/%d/coef", i);
    sprintf(RootDir, "HIST/%d/", i);

    TTree *THistQ = (TTree *)RootFile->Get(TreePathQ);

    THistQ->SetBranchAddress("histQ", hQ);

    for (int k = 0; k < NUMBER_OF_CHANNELS; k++) {
      for (int j = 0; j < nHistQ[i - 31][k]; j++) {
        for (int l = 0; l < 2000; l++) {
          hQ[k][l] = histQ[i - 31][k][l];
        }
      }

      THistQ->Fill();
    }
    RootFile->cd(RootDir);
    THistQ->Write("", TObject::kOverwrite);

    TTree *THistCoef = (TTree *)RootFile->Get(TreePathC);
    THistCoef->SetBranchAddress("histA1A2", &hA1A2);
    THistCoef->SetBranchAddress("histA2A6", &hA2A6);
    THistCoef->SetBranchAddress("histA3A4", &hA3A4);
    THistCoef->SetBranchAddress("histA4A8", &hA4A8);

    for (int j = 0; j < nHistC[i - 31]; j++) {
      hA1A2 = histA1A2[i - 31][j];
      hA2A6 = histA2A6[i - 31][j];
      hA3A4 = histA3A4[i - 31][j];
      hA4A8 = histA4A8[i - 31][j];
      if (hA1A2 == 0 && hA2A6 == 0 && hA3A4 == 0 && hA4A8 == 0)
        continue;
      else
        THistCoef->Fill();
    }

    THistCoef->Write("", TObject::kOverwrite);

    RootFile->cd();

    delete THistQ;
    delete THistCoef;
  }

  for (int j = 0; j < NUMBER_OF_STATIONS; j++) {
    delete[] histA1A2[j];
    delete[] histA2A6[j];
    delete[] histA3A4[j];
    delete[] histA4A8[j];
  }

  delete[] histA1A2;
  delete[] histA2A6;
  delete[] histA3A4;
  delete[] histA4A8;

  for (int i = 0; i < NUMBER_OF_STATIONS; i++) {
    for (int j = 0; j < NUMBER_OF_CHANNELS; j++) {
      delete[] histQ[i][j];
    }
    delete[] histQ[i];
  }
  delete[] histQ;

  return 0;
}