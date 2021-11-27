#include "graph.h"
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <string.h>
using namespace std;
bool cmp1(int a, int b)
{
    return a>b ;
}

void graph::loadgraph(string path,int bound)
{
    printf( "Loading graph " );
    cout << path << endl;
    fstream propertiesFile(path+"/properties.txt", ios::in);
    propertiesFile>>uCount>>vCount>>edgeCount;
    cout<<"properties: "<<uCount<<" "<<vCount<<" "<<edgeCount<<endl;
    edgeCount*=2;
    beginPos1 = new long long[uCount+vCount+1];
    beginPos = new int [uCount + vCount + 1];
    edgeList = new int[edgeCount];
    vertexCount = uCount + vCount;
    propertiesFile.close();   
    fstream beginFile(path+"/begin.bin", ios::in|ios::binary);
    fstream adjFile(path+"/adj.bin", ios::in|ios::binary);
    beginFile.read((char*)beginPos1,sizeof(long long)*(uCount+vCount+1));
    adjFile.read((char*)edgeList,sizeof(int)*(edgeCount));
    beginFile.close();
    adjFile.close();
    cout << "start!" << endl;
}



graph::~graph()
{
    // if (beginPos) delete(beginPos);
    // if (edgeList) delete(edgeList);
}

void graph1::loadgraph(string path, int bound){
    printf( "Loading graph ");
    cout << path << endl;
    fstream propertiesFile(path+"/properties.txt", ios::in);
    propertiesFile>>uCount>>vCount>>edgeCount;
    cout<<"properties: "<<uCount<<" "<<vCount<<" "<<edgeCount<<endl;
    edgeCount*=2;
    beginPos1 = new long long[uCount+vCount+1];
    edgeList = new int[uCount + vCount + 1];
    vertexCount = uCount + vCount;
    propertiesFile.close();   
    fstream beginFile(path+"/begin.bin", ios::in|ios::binary);
    beginFile.read((char*)beginPos1,sizeof(long long)*(uCount+vCount+1));
    beginFile.close();
    fp = fopen((path + "/adj.bin").c_str(), "r");
    cout << "load graph end" << endl;
}

void graph1::loadsubGraph(long long l, long long r){
    fseek(fp, 1LL*4*l, 0);
    fread(edgeList, 4, r - l, fp);
}