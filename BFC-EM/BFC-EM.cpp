#include<bits/stdc++.h>

using namespace std;

template<class T> void frr(FILE* fp, T& dest, long long size = 1){
    fread(&dest, sizeof(T), size, fp);
}

template<class T> void frr(FILE* fp, T* dest, long long size = 1){
    fread(dest, sizeof(T), size, fp);
}

template <class T>
void wrr(FILE *fp, T val, long long size = 1){
    fwrite(&val, sizeof(T), size, fp);
}

int bfcEm()