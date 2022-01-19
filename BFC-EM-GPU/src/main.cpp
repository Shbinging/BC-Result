#include<bits/stdc++.h>
#include"graph.h"
#include"countingAlgorithm/timer.h"
#include "countingAlgorithm/BFC-EM.h"
using namespace std;

int main(int argc, char* argv[]){
    graph trialGraph;
    trialGraph.loadgraph(argv[1], 10);
    if (strcmp("run", argv[2]) == 0){
        test(trialGraph);
    }
    return 0;
}