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
    //edgeCount*=2;
    beginPos1 = new long long[uCount+vCount+1];
    beginPos = new int [uCount + vCount + 1];
    edgeList = new int[edgeCount];
    propertiesFile.close();   
    fstream beginFile(path+"/begin.bin", ios::in|ios::binary);
    fstream adjFile(path+"/adj.bin", ios::in|ios::binary);
    beginFile.read((char*)beginPos1,sizeof(long long)*(uCount+vCount+1));
    adjFile.read((char*)edgeList,sizeof(int)*(edgeCount));
    beginFile.close();
    adjFile.close();
    //printf("ok");
    deg = new int[uCount+vCount + 10];
    memset(deg, 0, sizeof(deg));
    vertexCount=uCount+vCount;
    for(int i = 0; i <= vertexCount; i++)
        beginPos[i] = (int) beginPos1[i];
    for( int i = 0; i < vertexCount; ++i ) 
        for(int j = beginPos[i]; j < beginPos[i + 1]; j++){
            if (edgeList[j] >= i) break;
            deg[i]++;
        }
            
    edgeListDeg = new long long[edgeCount];
    for(int i = 0; i < edgeCount; i++){
        edgeListDeg[i] = deg[edgeList[i]];
        // if (i <= 10) printf("%lld ", edgeListDeg[i]);
    }
    //printf("s16 os %d\n", s16);
}



graph::~graph()
{
    delete(beginPos);
    delete(edgeList);
    delete(deg);
}