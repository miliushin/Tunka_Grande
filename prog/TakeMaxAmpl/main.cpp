//
//  main.cpp
//  TakeMaxAmpl
//
//  Created by Mikhail Iliushin on 10.11.2023.
//

#include <iostream>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "read_data.hpp"

using namespace std;

int main(int argc, const char * argv[]) {
  if (argc > 1) {
    read_data *Reader = new read_data(argv[1]);
    delete Reader;
  } else {
    printf("Usage: ./TakeMaxAmpl GRANDE_DATA_DIR\n");
    exit(1);
  }
  return 0;
}
