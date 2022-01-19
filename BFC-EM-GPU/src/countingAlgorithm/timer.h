#include<bits/stdc++.h>
#include<thread>
class timer{
    public:
        chrono::_V2::system_clock::time_point begin;
        chrono::_V2::system_clock::time_point end;
        timer(){}
        void start(){
            begin = chrono::system_clock::now();
        }
        void fin(){
            end = chrono::system_clock::now();
        }
        double getTime(){
            chrono::duration<double> diff = end - begin;
            return diff.count();
        }
};