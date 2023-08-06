#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

using namespace std;

string parseIni(string KeyName, string iniPath)
{
    ifstream openFile;
    string Key;
    string Value;
    string delimiter = "=";

    openFile.open(iniPath.c_str());
    if(openFile.is_open()) {
        string line;
        while(getline(openFile, line) != NULL) {
            delimiter = "=";
            if(string::npos == line.find(delimiter)) delimiter = "=";
            Key = line.substr(0, line.find(delimiter));
            if(Key == KeyName) {
                openFile.close();
                return line.substr(line.find(delimiter) + delimiter.length(), line.length());
            }
        }
        openFile.close();
        return "ERROR : NO SUCH KEY";
    } else {
        cerr << "ERROR : Cannot open " << iniPath << endl;
        return NULL;
    }
}

