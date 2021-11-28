#include<bits/stdc++.h>
#ifndef _t_
#define _t_
class timer{
    public:
        double begin;
        double end;
        double t;
        timer(){
            begin = 0;
            end = 0;
            t = 0;
        }
        void reset(){
            t = 0;
        }
        void start(){
            begin = clock();
        }
        void fin(){
            t += 1.0*(clock() - begin) / CLOCKS_PER_SEC;
        }
        double getTime(){
            return t;
        }
};
#endif