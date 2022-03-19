#include<bits/stdc++.h>
#include"graph.h"
#include "BFC-EM/BFC-EM.h"
using namespace std;

int main(int argc, char* argv[]){
    if (strcmp("em", argv[2]) == 0){
        bfcEm(argv[1], strtoll(argv[3], NULL, 10));
    }
    return 0;
}