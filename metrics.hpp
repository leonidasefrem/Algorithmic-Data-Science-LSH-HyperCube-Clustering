#ifndef METRICS_H
#define METRICS_H
#include<vector>
#include <bits/stdc++.h> 
#include<string.h>

using namespace std;

// Driver function to sort the vector elements 
// by second element of pairs 
bool sortbysec(const pair<int,int> &a,const pair<int,int> &b) 
{ 
    return (a.second < b.second); 
} 

//Class where we hold different kind of LSH-able metrics
class Metrics
{
private:

public:
    Metrics();
    ~Metrics();
    int get_distance(vector<double> a,vector<double> b,char *type);
    //Add more distance metrics here
};

Metrics::Metrics()
{
}

Metrics::~Metrics()
{
}

//Returns the "type" distance between vector a and vector b
//type = "L1"->returns Manhatan distance
int Metrics::get_distance(vector<double> a,vector<double> b,char *type){
    int distance = 0;

    if(a.size()!=b.size()){
        cout << "Size of two vectors is not the same" << endl;
        return -1;
    }

    if(strcmp((char *)"L1",type)==0){
        for (int i = 0; i < a.size(); i++)
        {
            distance += abs(double(a[i]) - double(b[i]));
        }
        return distance;
    }
    return distance;
}

#endif