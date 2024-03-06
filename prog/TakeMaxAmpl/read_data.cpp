//
//  read_data.cpp
//  TakeMaxAmpl
//
//  Created by Mikhail Iliushin on 10.11.2023.
//

#include "read_data.hpp"

#include <TF1.h>
#include <TH1F.h>
#include <math.h>

#include <iostream>

read_data::read_data(const char *dirname) {
  DIR *main_dir;  // the main dir of GRANDE data
  DIR *st_dir;    // station dir

  struct dirent *main_entry;
  struct dirent *st_ent;

  char ST_DIR[512];
  char FData[300];
  char hist_name[20];
  bool FlagFile;

  for (int i = 0; i < Aperture; i++) {
    XAperture[i] = i + 1.;
  }

  RootFile = new TFile("tmp.root", "RECREATE");
  for (int i = 0; i < NUMBER_OF_STATIONS; i++) {
    char chN[3];
    char cAMP[8];
    char cINT[8];
    int st = i + 31;
    memset(chN, '\0', 3);
    memset(cAMP, '\0', 8);
    memset(cINT, '\0', 8);
    snprintf(chN, 3, "%d", st);
    snprintf(cINT, 8, "%d/INT", st);
    snprintf(cAMP, 8, "%d/AMP", st);
    RootFile->mkdir(chN);
    RootFile->mkdir(cAMP);
    RootFile->mkdir(cINT);
    for (int j = 0; j < NUMBER_OF_CHANNELS; j++) {
      memset(hist_name, '\0', 20);
      snprintf(hist_name, 20, "hist_%d_%d", i, j);
      HIST[i][j] = new TH1F(hist_name, "integ", 1000, 0, 4000);
      HIST_amp[i][j] = new TH1F(hist_name, "ampl", 2000, 0, 2000);
      MaximumINT[i][j] = -1.;
      MaximumAMP[i][j] = -1;
      iMaximumINT[i][j] = 0;
    }
  }

  main_dir = opendir(dirname);
  if (!main_dir) {
    printf("ERROR: GRANDE_DATA_DIR is not opened!\n");
    exit(1);
  };

  while ((main_entry = readdir(main_dir)) != NULL) {
    char namest[5];
    if (main_entry->d_type == DT_DIR) {
      memset(ST_DIR, '\0', 512);
      snprintf(ST_DIR, 512, "%s", main_entry->d_name);
      if (ST_DIR[0] == 'S') {
        memset(namest, '\0', 5);
        namest[0] = ST_DIR[2];
        namest[1] = ST_DIR[3];

        memset(ST_DIR, '\0', 512);
        snprintf(ST_DIR, 512, "%s/%s", dirname, main_entry->d_name);

        // Read GRANDE station dir
        printf("READ:%s\n", ST_DIR);
        fflush(stdout);
        st_dir = opendir(ST_DIR);
        while ((st_ent = readdir(st_dir)) != NULL) {
          if (st_ent->d_type == DT_REG) {
            memset(FData, '\0', 300);
            snprintf(FData, 300, "%s/%s", ST_DIR, st_ent->d_name);
            FlagFile = true;
            for (int k = 0; k < 300; k++) {
              if (FData[k] == '.' && FData[k + 1] == 't' &&
                  FData[k + 2] == 'i') {
                FlagFile = false;
                break;
              }
            }
            if (FlagFile == true) {
              // opening of grande data file
              printf("file: %s\n", FData);
              fflush(stdout);
              FILE *Fst;
              if ((Fst = fopen(FData, "r")) == NULL) {
                printf("ERROR: can't opnen file %s\n ", FData);
              }
              ReadADC(Fst, atoi(namest));

              fclose(Fst);
            }
          }
        }
      }
    }
  }
  closedir(main_dir);

  FitMaximumAMP();
  FitMaximumINT();
}

read_data::~read_data() {
  for (int i = 0; i < NUMBER_OF_STATIONS; i++) {
    for (int j = 0; j < NUMBER_OF_CHANNELS; j++) {
      delete HIST[i][j];
      delete HIST_amp[i][j];
    }
  }

  RootFile->Close();
  delete RootFile;
  
  //system("mkdir AMP");
  //system("mkdir INT")
  
  // Writing maximum amplitude and integral
  for (int i = 0; i < NUMBER_OF_STATIONS; i++) {
    int nst = i + 31;
    char file_int[20];
    char file_amp[20];
    memset(file_amp, '\0', 20);
    memset(file_int, '\0', 20);
    snprintf(file_amp, 20, "AMP/%d.txt", nst);
    snprintf(file_int, 20, "INT/%d.txt", nst);
    
    FILE *fAMP = fopen(file_amp, "w");
    if (fAMP == NULL) {
      printf("ERROR: file %s don't opened\n", file_amp);
      exit(1);
    }
    for (int j = 0; j < NUMBER_OF_CHANNELS; j++) {
      fprintf(fAMP,"%d %d\n", j, MaximumAMP[i][j]);
    }
    fclose(fAMP);
    
    
    FILE *fINT = fopen(file_int, "w");
    if (fINT == NULL) {
      printf("ERROR: file %s don't opened\n", file_int);
      exit(1);
    }
    for (int j = 0; j < NUMBER_OF_CHANNELS; j++) {
      fprintf(fINT,"%d %lf %lf\n", j, MaximumINT[i][j], iMaximumINT[i][j]);
    }
    fclose(fINT);
  }
  
  
}

int read_data::ReadADC(FILE *data_file, int Nst) {
  unsigned char buf[SIZE_OF_HEADER];
  unsigned char data_read[20000];
  bool FlagRead = false;

  long Addr;
  long AddrBase[16] = {0x80000, 0x80800, 0x84000, 0x84800, 0xA0000, 0xA0800,
                       0xA4000, 0xA4800, 0xC0000, 0xC0800, 0xC4000, 0xC4800,
                       0xE0000, 0xE0800, 0xE4000, 0xE4800};

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
        break;
      }
      
      Long64_t ipst = (Addr - AddrBase[ch]) / 2;
      Long64_t ipfn = ipst + NumPoints;
      if (ipst < 0 || ipfn > 1024) {
        printf("ERROR:   Addr=0x%lx,  NumPoints=%i   ipst=%lli ipfn=%lli \n",
               Addr, NumPoints, ipst, ipfn);
        ipst = 400;
        ipfn = 600;
      }

      for (int k = 0; k < NumPoints * 2; k += 2) {
        index = j + k + 8;
        ADC[ch][ipst + k / 2] =
            (data_read[index + 1] * 256 + data_read[index]) - 2048;
      }

      j += (NumPoints * 2 + 8);

      int ia = GetI_GRANDE(ch, ADC[ch]);
      if (ia == 0) {
        int iADC = floor(INT_ADC + 0.5);
        HIST[Nst - 31][ch]->Fill(iADC);
      }
      ia = GetA_GRANDE(ch, ADC[ch]);
      if (ia == 0) {
        HIST_amp[Nst - 31][ch]->Fill(AMP_ADC);
      }
    }

    i_File = ftell(data_file);
  }

  return 0;
}

int read_data::GetI_GRANDE(int Nch, int data[Aperture]) {
  double New[Aperture];
  double tt[Aperture];
  int MaxAmplitude = 0.;
  double MeanADC = 0.;
  int EndMean;
  int StartMean;

  for (int i = 0; i < Aperture; i++) {
    New[i] = 0.;
    tt[i] = 0.;
  }

  for (int i = 0; i < Aperture; i++) {
    if (data[i] > MaxAmplitude) {
      MaxAmplitude = data[i];
    }
  }

  bool FPed = false;
  for (int i = 10; i < 70; i++) {
    if (data[i + 1] != data[i]) {
      FPed = true;
      break;
    }
  }

  if (FPed == false) {
    EndMean = 420;
    StartMean = 400;
  } else {
    EndMean = 15;
    StartMean = 6;
  }

  int Nmean = 0;
  for (int i = StartMean; i < EndMean; i++) {
    MeanADC += data[i];
    Nmean++;
  }
  MeanADC /= Nmean;

  for (int i = 0; i < Aperture; i++) {
    New[i] = data[i] - MeanADC;
    tt[i] = data[i];
    if (StartMean == 400 && (i <= 400 || i >= 600)) {
      New[i] = 0;
      tt[i] = 0;
    }

    if (New[i] >= -(NOISE_RANGE) && New[i] <= (NOISE_RANGE)) {
      New[i] = 0.;
    }
    if (New[i] < 0.) New[i] = 0.;
  }

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
        // for (int j = iMaxAmplitude-5; j >= 0; j--) {
        for (int j = i; j >= 0; j--) {
          New[j] = data[j] - MeanADC;
          // if (New[j] <= 0. || New[j+1] < New[j]) {
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
    return 1;
  } else if (MaxAmplitude >= 2030) {
    return 2;
  }

  GADC = new TGraph(Aperture, XAperture, New);
  INT_ADC = GADC->Integral(0, Aperture);


  delete GADC;

  return 0;
}

int read_data::GetA_GRANDE(int Nch, int data[Aperture]) {
  double New[Aperture];
  double tt[Aperture];
  int MaxAmplitude = 0.;
  double MeanADC = 0.;
  int EndMean;
  int StartMean;

  for (int i = 0; i < Aperture; i++) {
    New[i] = 0.;
    tt[i] = 0.;
  }

  for (int i = 0; i < Aperture; i++) {
    if (data[i] > MaxAmplitude) {
      MaxAmplitude = data[i];
    }
  }

  bool FPed = false;
  for (int i = 10; i < 70; i++) {
    if (data[i + 1] != data[i]) {
      FPed = true;
      break;
    }
  }

  if (FPed == false) {
    EndMean = 420;
    StartMean = 400;
  } else {
    EndMean = 15;
    StartMean = 6;
  }

  int Nmean = 0;
  for (int i = StartMean; i < EndMean; i++) {
    MeanADC += data[i];
    Nmean++;
  }
  MeanADC /= Nmean;

  for (int i = 0; i < Aperture; i++) {
    New[i] = data[i] - MeanADC;
    tt[i] = data[i] - MeanADC;
    if (StartMean == 400 && (i <= 400 || i >= 600)) {
      New[i] = 0;
      //tt[i] = 0;
    }


    if (New[i] >= -(NOISE_RANGE_FOR_AMP) && New[i] <= (NOISE_RANGE_FOR_AMP)) {
      New[i] = 0.;
    }
    if (New[i] < 0.) New[i] = 0.;
  }



  MaxAmplitude = MaxAmplitude - MeanADC;
  if (MaxAmplitude >= -(NOISE_RANGE_FOR_AMP) && MaxAmplitude <= (NOISE_RANGE_FOR_AMP)) {
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
        // for (int j = iMaxAmplitude-5; j >= 0; j--) {
        for (int j = i; j >= 0; j--) {
          New[j] = data[j] - MeanADC;
          // if (New[j] <= 0. || New[j+1] < New[j]) {
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
    return 1;
  } else if (MaxAmplitude >= 2030) {
    return 2;
  }


  AMP_ADC = MaxAmplitude;

  return 0;
}

void read_data::FitMaximumINT() {
  char names[8];
  int inames;
  int xmin, xmax, xmean;
  for (int i = 0; i < NUMBER_OF_STATIONS; i++) {
    inames = i + 31;
    memset(names, '\0', 8);
    snprintf(names, 8, "%d/INT", inames);
    RootFile->cd(names);
    for (int j = 0; j < NUMBER_OF_CHANNELS; j++) {
      HIST[i][j]->GetXaxis()->SetRangeUser(300, 1000);
      xmean = HIST[i][j]->GetMaximumBin();
      xmin = xmean - 200;
      xmax = xmean + 500;

      iMaximumINT[i][j] = HIST[i][j]->GetBinContent(xmean);



      if (xmin < 300) xmin = 300;
      HIST[i][j]->GetXaxis()->SetRangeUser(0, 4000);
      TF1 *FitFunc = new TF1("FitFunc", "landaun", xmin, xmax);
      HIST[i][j]->Fit("FitFunc", "RQ");
      MaximumINT[i][j] = FitFunc->GetParameter(1);
      HIST[i][j]->Write();
      delete FitFunc;
    }
    RootFile->cd();
  }
}
void read_data::FitMaximumAMP() {
  
  char names[8];
  int inames;

  for (int i = 0; i < NUMBER_OF_STATIONS; i++) {
    inames = i + 31;
    memset(names, '\0', 8);
    snprintf(names, 8, "%d/AMP", inames);
    RootFile->cd(names);
    for (int j = 0; j < NUMBER_OF_CHANNELS; j++) {
      //HIST_amp[i][j]->GetXaxis()->SetRangeUser(5, 80);
      int min_func = 999999;
      for (int ix = 5; ix < 30; ix++) {
        if (min_func > HIST_amp[i][j]->GetBinContent(ix)){
      		min_func = HIST_amp[i][j]->GetBinContent(ix);
		MaximumAMP[i][j] = ix;
	}
      }

      if (MaximumAMP[i][j] > 25) { 
	MaximumAMP[i][j] = 15;
      }
      HIST_amp[i][j]->Write();
    }
    RootFile->cd();
  }
   
  
}
