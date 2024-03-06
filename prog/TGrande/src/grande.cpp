#include "grande.h"

#include <TApplication.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TROOT.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#include <cstdlib>

Double_t fgaus(Double_t *x, Double_t *par) {
  return par[0] * exp(-0.5 * pow((x[0] - par[1]) / par[2], 2.)) /
         (par[2] * sqrt(2. * const_Pi));
}

GRANDE::GRANDE(char *name_file) {
  int len = strlen(name_file);
  char Date[7];

  Nst[0] = name_file[len - 7];
  Nst[1] = name_file[len - 6];
  Nst[2] = name_file[len - 5];
  Nst[3] = 0;
  char NstName[4];
  sprintf(NstName, "%d", atoi(Nst));

  for (int i = 0; i < 30; i++) {
    if (name_file[len - i] == 'S') {
      for (int k = 0; k < 20; k++) {
        if (name_file[len - i - 2 - k] == '/') {
          for (int j = 0; j < 6; j++) {
            Date[j] = name_file[len - i - 1 - k + j];
          }
          Date[6] = 0;
          break;
        }
      }
      break;
    }
  }

  char OutName[12];
  OutName[11] = 0;
  sprintf(OutName, "%s.root", Date);
  RootFile = new TFile(OutName, "UPDATE");
  TData = (TTree *)RootFile->Get(NstName);
  TData->SetBranchAddress("NumberOfEvent", &NumEvent);
  TData->SetBranchAddress("TimeEvent", &TimeEvent);
  TData->SetBranchAddress("ADC", ADC);
  // TData->Branch("METEO",ADC,"ADC/D");

  for (int i = 0; i < Aperture; i++) {
    XAperture[i] = i + 1.;
  }
}

GRANDE::~GRANDE() {
  TData->Write("", TObject::kOverwrite);
  delete TData;

  RootFile->Close();
  delete RootFile;
}

double GRANDE::GetTime(unsigned char ctt[8]) {
  int data_time[4];
  short unsigned int h, m, s, mls, mks, dns;

  for (int i = 0; i < 4; i++) {
    data_time[i / 2] = ctt[i * 2 + 1] * 256 + ctt[i * 2];
  }

  data_time[0] = ctt[1] * 256 + ctt[0];
  data_time[1] = ctt[3] * 256 + ctt[2];
  data_time[2] = ctt[5] * 256 + ctt[4];
  data_time[3] = ctt[6] * 256 + ctt[6];

  dns = (data_time[0] & 0x7f) * 10;
  mks = (data_time[0] & 0xff80) >> 7;
  mks |= (data_time[1] & 1) << 9;
  mls = (data_time[1] & 0x7fe) >> 1;
  s = (data_time[1] & 0xf800) >> 11;
  s |= (data_time[2] & 1) << 5;
  m = (data_time[2] & 0x7e) >> 1;
  h = (data_time[2] & 0xf80) >> 7;

  double time = (double)dns / 1000000000.0 + (double)mks / 1000000.0 +
                (double)mls / 1000.0 + (double)s + (double)m * 60.0 +
                (double)h * 3600.0;

  return (time);
}

int GRANDE::ReadADC(FILE *data_file, int FHist) {
  char TreePathQ[30];
  char RootDir[30];

  unsigned char buf[SIZE_OF_HEADER];
  unsigned char data_read[20000];
  int dat = 0;
  bool FlagRead = false;

  double hQ[NUMBER_OF_CHANNELS][2000];

  memset(TreePathQ, '\0', 30);
  memset(RootDir, '\0', 30);
  sprintf(TreePathQ, "HIST/%d/histQ", atoi(Nst));
  sprintf(RootDir, "HIST/%d/", atoi(Nst));

  TTree *THistQ = (TTree *)RootFile->Get(TreePathQ);
  THistQ->SetBranchAddress("histQ", hQ);

  fseek(data_file, 0, SEEK_END);
  END_File = ftell(data_file);
  i_File = 0;
  fseek(data_file, 0, SEEK_SET);
  int Nevent;
  while (i_File < END_File) {
    //--------------------------------------- Read header of file
    if (!(fread(buf, SIZE_OF_HEADER, 1, data_file))) {
      printf("ERROR: read file ");
      return -1;
    }
    Nevent = 0;

    ID = buf[1] * 256 + buf[0];

    switch (ID) {
      case 3032:
        Claster = buf[20];
        break;
      case 3033:
        Claster = buf[20];
        break;
      case 3034:
        Claster = buf[20];
        break;
      case 3035:
        Claster = buf[20];
        break;
      default:
        printf("WROND ID: %i\n", ID);
        return -1;
    }

    NumBytes = buf[3] * 256 + buf[2];
    NumEvent =
        buf[7] * 256 * 256 * 256 + buf[6] * 256 * 256 + buf[5] * 256 + buf[4];

    TimeEvent = GetTime(buf + 12);
    Delay = (buf[23] * 256 + buf[22]) * 5;
    TimeEvent += (double)Delay / 1000000000.0;

    if (FlagRead == false) {
      for (int ich = 0; ich < NUMBER_OF_CHANNELS; ich++) {
        for (int ibin = 0; ibin < Aperture; ibin++) {
          ADC[ich][ibin] = 0;
        }
      }
      FlagRead = true;
    }

    //-------------------------------------- END Read header of file

    int NumPoints = 0;
    int index;
    int j = 0;
    int ch = 0;
    memset(data_read, 0, sizeof(data_read));
    //---------------------Read Data---------------------------------
    fread(data_read, NumBytes, 1, data_file);
    for (int i = 0; i < 8; i++) {
      Addr = (unsigned long)data_read[j + 3] * 256L * 256L * 256L +
             (unsigned long)data_read[j + 2] * 256L * 256L +
             (unsigned long)data_read[j + 1] * 256L +
             (unsigned long)data_read[j + 0];

      Addr &= 0x7fffefff;
      NumPoints = (unsigned int)data_read[j + 5] * 256 + data_read[j + 4];

      ch = (unsigned int)data_read[j + 6] & 0xf;
      if (ch == 11) {  // only 12 channels of GRANDE now
        FlagRead = false;
        TData->Fill();
        break;
      }
      int ipst = (Addr - AddrBase[ch]) / 2;
      int ipfn = ipst + NumPoints;
      if (ipst < 0 || ipfn > 1024) {
        printf("ERROR:   Addr=0x%lx,  NumPoints=%i   ipst=%i ipfn=%i \n", Addr,
               NumPoints, ipst, ipfn);
        ipst = 400;
        ipfn = 600;
      }

      for (int k = 0; k < NumPoints * 2; k += 2) {
        index = j + k + 8;
        ADC[ch][ipst + k / 2] =
            (data_read[index + 1] * 256 + data_read[index]) - 2048;
      }

      j += (NumPoints * 2 + 8);

      if (FHist == 1) {
        int ia = GetIA_GRANDE(ch, ADC[ch]);
        if (INT_ADC < 2000 && ia == 0) {
          int iADC = floor(INT_ADC + 0.5);
          hQ[ch][iADC]++;
        }
      }
    }

    i_File = ftell(data_file);
  }

  if (FHist == 1) {
    THistQ->Fill();
    RootFile->cd(RootDir);
    THistQ->Write("", TObject::kOverwrite);
  }
  delete THistQ;

  return 1;
}

int GRANDE::ReadMeteo(string sfile) {
  printf("Reading Meteo Data...\n");
  char str[150];
  int stN;

  TMeteo = (TTree *)RootFile->Get("METEO");
  TMeteo->SetBranchAddress("Time", &time_meteo);
  TMeteo->SetBranchAddress("Pressure", P_meteo);
  TMeteo->SetBranchAddress("Humidity", H_meteo);
  TMeteo->SetBranchAddress("T", T_meteo);
  TMeteo->SetBranchAddress("T1", T1_meteo);
  TMeteo->SetBranchAddress("T2", T2_meteo);
  TMeteo->SetBranchAddress("T3", T3_meteo);

  string meteo = "t133_meteo.txt";
  int iS = sfile.find("St", 0, 2);
  char *smeteo = new char[meteo.size() + iS];

  for (int i = 0; i < (iS + meteo.size()); i++) {
    if (i < iS)
      smeteo[i] = sfile[i];
    else
      smeteo[i] = meteo[i - iS];
  }
  smeteo[iS + meteo.size()] = '\0';

  FILE *FileMeteo = fopen(smeteo, "r");

  printf("%s\n", smeteo);
  if (FileMeteo == NULL) {
    printf("No t133_meteo.txx\n");
    return 0;
  }

  while (!feof(FileMeteo)) {
    if (fgets(str, 150, FileMeteo)) {
      if (str[0] == 'S') {
        // str1[2] = '\0';
        stN = (str[3] - '0') * 10 + (str[4] - '0');
        if (stN == 31)
          stN = 0;
        else if (stN == 45)
          stN = 1;
        else if (stN == 46)
          stN = 2;
        else {
          printf("Read ERROR: number of meteo station:: %d\n", stN);
          continue;
        }

        // time
        for (int i = 0; i < 150; i++) {
          if (str[i] == ':') {
            time_meteo[stN] = 3600. * (10. * double(str[i - 2] - '0') +
                                       double(str[i - 1] - '0')) +
                              60. * (10. * double(str[i + 1] - '0') +
                                     double(str[i + 2] - '0')) +
                              10. * double(str[i + 4] - '0') +
                              double(str[i + 5]);
            break;
          }
        }

        // pressure
        for (int i = 0; i < 150; i++) {
          if (str[i] == 'P' && str[i + 1] == '=') {
            char buf[10];
            for (int j = 0; j < 10; j++) {
              buf[j] = str[i + 2 + j];
              if (buf[j] == ' ' || buf[j] == 'h') {
                buf[j] = '\0';
                break;
              }
            }
            P_meteo[stN] = atof(buf);
            break;
          }
        }

        // temperature T
        for (int i = 0; i < 150; i++) {
          if (str[i] == 'T' && str[i + 1] == '=') {
            char buf[10];
            for (int j = 0; j < 10; j++) {
              buf[j] = str[i + 2 + j];
              if (buf[j] == 'C') {
                buf[j] = '\0';
                break;
              }
            }
            T_meteo[stN] = atof(buf);
            break;
          }
        }

        // humidity
        for (int i = 0; i < 150; i++) {
          if (str[i] == 'H' && str[i + 1] == '=') {
            char buf[10];
            for (int j = 0; j < 10; j++) {
              buf[j] = str[i + 2 + j];
              if (buf[j] == '%') {
                buf[j] = '\0';
                break;
              }
            }
            H_meteo[stN] = atof(buf);
            break;
          }
        }

        // temperature T1
        for (int i = 0; i < 150; i++) {
          if (str[i] == 'T' && str[i + 1] == '1') {
            char buf[10];
            for (int j = 0; j < 10; j++) {
              buf[j] = str[i + 2 + j + 1];
              if (buf[j] == 'C') {
                buf[j] = '\0';
                break;
              }
            }
            T1_meteo[stN] = atof(buf);
            break;
          }
        }

        // temperature T2
        for (int i = 0; i < 150; i++) {
          if (str[i] == 'T' && str[i + 1] == '2') {
            char buf[10];
            for (int j = 0; j < 10; j++) {
              buf[j] = str[i + 2 + j + 1];
              if (buf[j] == 'C') {
                buf[j] = '\0';
                break;
              }
            }
            T2_meteo[stN] = atof(buf);
            break;
          }
        }

        // temperature T3
        for (int i = 0; i < 150; i++) {
          if (str[i] == 'T' && str[i + 1] == '3') {
            char buf[10];
            for (int j = 0; j < 10; j++) {
              buf[j] = str[i + 2 + j + 1];
              if (buf[j] == 'C') {
                buf[j] = '\0';
                break;
              }
            }
            T3_meteo[stN] = atof(buf);
            break;
          }
        }

        TMeteo->Fill();
      }
    }
  }

  TMeteo->Write("", TObject::kOverwrite);
  delete TMeteo;

  fclose(FileMeteo);
  return 1;
}

int GRANDE::GetIA_GRANDE(int Nch, int data[Aperture]) {
  double New[Aperture];
  double tt[Aperture];
  int iMaxAmplitude = 0;
  int MaxAmplitude = 0.;
  double MeanADC = 0.;
  int EndMean;
  int StartMean;

  int iStart;
  int iEnd;

  for (int i = 0; i < Aperture; i++) {
    New[i] = 0.;
    tt[i] = 0.;
  }

  for (int i = 0; i < Aperture; i++) {
    if (data[i] > MaxAmplitude) {
      MaxAmplitude = data[i];
      iMaxAmplitude = i;
    }
  }

  if (iMaxAmplitude > 400 && (data[1] == 0 || data[2] == 0)) {
    EndMean = 420;
    StartMean = 400;
    iStart = 400;
    iEnd = 600;
  } else {
    EndMean = 15;
    StartMean = 6;
    iStart = 4;
    iEnd = Aperture;
  }

  if (EndMean > 0) {
    int k = 0;
    for (int i = StartMean; i < EndMean; i++) {
      MeanADC += data[i];
      k++;
    }
    MeanADC /= k;
  }

  for (int i = 0; i < Aperture; i++) {
    New[i] = data[i] - MeanADC;
    tt[i] = data[i] - MeanADC;

    if (New[i] >= -(NOISE_RANGE) && New[i] <= (NOISE_RANGE)) {
      New[i] = 0.;
    }
    if (New[i] < 0.) New[i] = 0.;
  }

  // TGraph * GADC1 = new TGraph(Aperture, XAperture, tt);

  MaxAmplitude = MaxAmplitude - MeanADC;
  if (MaxAmplitude >= -(NOISE_RANGE) && MaxAmplitude <= (NOISE_RANGE)) {
    MaxAmplitude = 0;
  }

  int k = 0;
  bool Flag = false;
  for (int i = 0; i < Aperture; i++) {
    if (New[i] != 0.) {
      for (int j = i; j < Aperture; j++) {
        New[j] = data[j] - MeanADC;
        if (New[j] <= 0.) {
          New[j] = 0.;
          k = j;
          Flag = true;
          break;
        }
      }
      if (Flag == true) {
        for (int j = i; j >= 0; j--) {
          New[j] = data[j] - MeanADC;
          if (New[j] <= 0.) {
            New[j] = 0.;
            i = k;
            Flag = false;
            break;
          }
        }
      }
    }
  }

  if (MaxAmplitude <= 0) {
    INT_ADC = 0.;
    AMP_ADC = 0;
    TIME_ADC = 0.;
    return 1;
  } else if (MaxAmplitude >= 2030) {
    return 2;
  }

  GADC = new TGraph(Aperture, XAperture, New);
  INT_ADC = GADC->Integral(0, Aperture);
  AMP_ADC = MaxAmplitude;

  int xmin = 0;
  int xmax = 0;
  bool Fmin = false;
  bool Fmax = false;
  bool FFmin = false;
  bool FFmax = false;

  for (int i = 0; i < Aperture; i++) {
    if (New[i] > 0 && Fmin == false) {
      xmin = i;
      Fmin = true;
    }
    if (Fmin == true && Fmax == false && New[i + 1] < New[i]) {
      xmax = i;
      Fmax = true;
    }
    if (Fmin == true && Fmax == true) break;
  }

  if (Fmax == true && Fmin == true) {
    for (int i = iStart; i < iEnd; i++) {
      if (FFmin == false && New[i] >= 0.1 * New[xmax]) {
        xmin = i;
        FFmin = true;
      }
      if (FFmin == true && FFmax == false && New[i] >= 0.6 * New[xmax]) {
        xmax = i;
        FFmax = true;
      }
      if (FFmax == true && FFmin == true) break;
    }
  }

  // if(Fmax==true && Fmin==true){
  double delx = xmax - xmin;
  if (delx > 5 && MaxAmplitude > 12 && FFmax == true && FFmin == true) {
    FitFunc = new TF1("FitFunc", "[0]+[1]*x", (double)xmin, (double)xmax);
    GADC->Fit("FitFunc", "Rq");
    double A1 = FitFunc->GetParameter(0);
    double A2 = FitFunc->GetParameter(1);
    TIME_ADC = -A1 / A2;
    TIME_ADC *= 5.E-9;

    // TFile *tmp = new TFile("tmp.root", "UPDATE");
    // TCanvas *can = new TCanvas("can","c");
    //  GADC1->Draw("AL");
    // GADC->Write();
    //  can->Write();
    // tmp->Close();
    //  can->Delete();
    //}
  } else {
    for (int i = 0; i < iEnd; i++) {
      if (New[i] != 0) {
        TIME_ADC = double(i - 1);
        break;
      }
    }

    // printf("%d %d %d %d\n",MaxAmplitude, iMaxAmplitude, xmin,xmax);

    TIME_ADC *= 5.E-9;
    // if(iMaxAmplitude>600){
    //   printf("%d %d %d %d\n",MaxAmplitude, iMaxAmplitude, xmin,xmax);
    // TFile *tmp = new TFile("tmp.root", "UPDATE");
    // GADC->Write();

    // tmp->Close();
    // }
  }
  // }
  // else{
  //   delete GADC;
  //   return 1;
  // }
  // TimeADC = TIME_ADC;
  delete GADC;
  return 0;
}