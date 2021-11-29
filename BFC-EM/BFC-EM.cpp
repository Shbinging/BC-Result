#include<bits/stdc++.h>
#include<bits/stdc++.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_opadd.h>
#include"BFC-EM.h"
#include <tbb/concurrent_hash_map.h>
#include <tbb/tbb_allocator.h>
#include "tbb/tick_count.h"
#include <tbb/mutex.h>
#include "../BFC-VP++/timer.h"
#include "stdio.h"
using namespace std;

timer ioTime;
template<class T> void frr(FILE* fp, T& dest, long long size = 1){
    fread(&dest, sizeof(T), size, fp);
}

template<class T> void frr(FILE* fp, T* dest, long long size = 1){
    fread(dest, sizeof(T), size, fp);
}

template <class T>
void wrr(FILE *fp, T* val, long long size = 1){
    fwrite(val, sizeof(T), size, fp);
}
template <class T>
void wrr(FILE *fp, T val, long long size = 1){
    fwrite(&val, sizeof(T), size, fp);
}
class buffer{
public:
    buffer(string _path, long long _size){
        size = _size;
        bufferArray = new long long [size];
        num = 0;
        path = _path;
    }
    bool append(long long val){
        if (num >= size) return 0;
        else bufferArray[num++] = val;
        return 1;
    }
    bool write(){
        ioTime.start();
        FILE* fp = fopen(path.c_str(), "w");
        if (fp == NULL){
            printf("write error");
            return 0;
        }      
        wrr(fp, num, 1);
        sort(bufferArray, bufferArray + num);
        wrr(fp, bufferArray, num);
        fclose(fp);
        ioTime.fin();
    }
    void reset(string _path){
        num = 0;
        path = _path;
    }
    long long size, num;
    string path;
    long long* bufferArray;
};
string getPath(int num){    
    stringstream ss;
    ss << num;
    string ans;
    ss >> ans;
    return "diskData/" + ans + ".in";
}
class bufferPool{
public:
    bufferPool(string _dirPath, int _p, long long _maxBufferSize){
        dirPath = _dirPath;
        bufferNum = 0;
        p = _p;
        b = new buffer(getPath(bufferNum), _maxBufferSize);
    }
    void append(long long val){
        if (!b->append(val)){
            b->write();
            bufferNum++;
            b->reset(getPath(bufferNum));
            b->append(val);      
        }
    }
    void finish(){
        b->write();
        bufferNum++;
    }
    string dirPath;
    int bufferNum, p;
    buffer* b;
};

class bufferReader{
public:
    bufferReader(int num, long long _bufferSize){
        filePath = getPath(num);
        ioTime.start();
        fp = fopen(filePath.c_str(), "r");
        assert(fp != NULL);
        fread(&n, 8, 1, fp);
        ioTime.fin();
        bufferSize = _bufferSize;
        buffer = new long long[bufferSize];
        s = 0;
        head = bufferSize;
        //fclose(fp);
    }
    void load(){
        ioTime.start();
        //fp = fopen(filePath.c_str(), "r");
        //assert(fp != NULL);
        //fseek(fp, 1LL* (s + 1) *8, 0);
        head = 0;
        bufferSize = min(bufferSize, n - s);
        fread(buffer, 8, bufferSize, fp);
        ioTime.fin();
        //fclose(fp);
    }
    long long get(){
        if (head == bufferSize && !isEmpty()){
            load();
        }
        if (head < bufferSize){
            s++;
            return buffer[head++];
        }
    }
    bool isEmpty(){
        if (s >= n) return 1;
        else return 0;
    }
    long long n, s, bufferSize, head;
    long long* buffer;
    string filePath;
    FILE* fp;
};

long long hashing(long long a, long long b, int vertexCount){
    return  a * vertexCount + b;
}
struct node{
    long long val;
    int pos;
    node(){}
    node(long long _val, int _pos){
        val = _val;
        pos = _pos;
    }
};
bool operator <(node a, node b){
    return a.val > b.val;
}
int bfcEm(string graphName, long long storageSize){
    long long maxBufferSize = storageSize / 8 * 1024 * 1024;
    graph1 g;
    ioTime.start();
    g.loadgraph("/home/shbing/datasetsNew/datasets/bipartite/"+ graphName + "/sorted", -1);
    ioTime.fin();
    int num = 0;
    bufferPool b("diskData/", 1, maxBufferSize);
    timer totalTime;
    totalTime.start();
    for(int i = 0; i < g.vertexCount; i++){
        int len = g.beginPos1[i + 1] - g.beginPos1[i];
        g.loadsubGraph(g.beginPos1[i], g.beginPos1[i + 1]);
        for(int j = 0; j < len; j++){
            if (g.edgeList[j] >= i) break;
            for(int k = j + 1; k < len; k++){
                b.append(hashing(g.edgeList[k], g.edgeList[j], g.vertexCount));
            }
        }
    }
    b.finish();
    bufferReader** brList = new bufferReader*[b.bufferNum];
    priority_queue<node> q;
    long long nn = 0;
    for(int i = 0; i < b.bufferNum; i++){
        brList[i] = new bufferReader(i, maxBufferSize / b.bufferNum);
        nn++;
        q.push(node(brList[i]->get(), i));
    }
    long long tmp = -1, s = 0, ans = 0;
    vector<long long> c;
    while(!q.empty()){
        node tmpNode = q.top();
        q.pop();
        c.push_back(tmpNode.val);
        if (tmpNode.val != tmp){
           // printf("%d ", tmpNode.val);
            tmp = tmpNode.val;
            ans += s * (s - 1) / 2;
            //ans += 1;
            s = 1;
        }else s++;
        if (!brList[tmpNode.pos]->isEmpty()){
            nn++;
            q.push(node(brList[tmpNode.pos]->get(), tmpNode.pos));
        }
    }
    ans += s * (s - 1) / 2;
    // int l1 = c.size();
    // for(int i = 1; i < l1; i++){
    //     if (c[i] > c[i - 1]){
    //         printf("wrong! %lld %lld\n", c[i - 1], c[i]);
    //         break;
    //     }
    // }
    //ans += 1;
    // vector<long long>a;
    // for(int i = 0; i < b.bufferNum; i++){
    //     bufferReader br(i, maxBufferSize/ 1000);
    //     while(!br.isEmpty()){
    //         a.push_back(br.get());
    //     }
    // }
    // long long ans1 = 0;tmp = -1;
    // int n = a.size();
    // sort(a.begin(), a.end());
    // //long long ans = 0, s = 0, tmp = -1;
    // for(int i = 0; i < n; i++){
    //     if (a[i] != tmp){
    //         tmp = a[i];
    //         //printf("%lld ", tmp);
    //         ans1 += s * (s - 1) / 2;
    //         //ans1 += 1;
    //         s = 1;
    //     }else s++;
    // }
    // ans += s * (s - 1) / 2;
    // //ans1 += 1;
    // printf("n:%lld nn:%lld ans:%lld anspi:%lld\n", n, nn, ans, ans1);
    totalTime.fin();
    FILE* ansfile = fopen("testOut.csv", "a+");
    if (ansfile == NULL){
        printf("ans open wrong!");
    }
    fprintf(ansfile, "%s,%lld,%lld,%f,%f\n", graphName.c_str(), storageSize, ans,  totalTime.getTime(), ioTime.getTime());
    fclose(ansfile);
    //cout << n << "ans << endl;
}