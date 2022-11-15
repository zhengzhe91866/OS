/*
use "g++ main.cpp -o main -pthread" to compile
and "./main data.txt" to run
*/
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <unordered_set>
#include <unistd.h>

using namespace std;

#include "MultiThread.h"

int main(int argc, char* argv[]){
    MultiThread data;
    data.load(argv[1]);
    data.run();
}