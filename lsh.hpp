#ifndef LSH_H
#define LSH_H
#include"hashtable.hpp"
#include"metrics.hpp"
#include<unordered_map>
#include<bits/stdc++.h>

using namespace std;

class LSH
{
private:
    vector<Hashtable *> hashtables;  //Array with pointers to Hashtable classes
    int K;  
    int L;  
    int r;
    vector<vector<double>> data;
public:
    LSH(int ,int ,vector<vector<double>> &data_vector,int r);
    pair<int,int> nearest_neighbor(vector<double> q,int img_index);
    vector<pair<int,int>> knn(vector<double> q,int img_index,int k);    //Approximate
    vector<int> range_search(vector<double> q,int img_index,double r,int c);
    vector<pair<int,int>> exact_nearest_neighbor(vector<double> q,int k);
    ~LSH();
};

//k->ακέραιο πλήθος k των LSH συναρτήσεων hi που χρησιμοποιούνται για τον ορισμό των g
//ο ακέραιος αριθμός L των πινάκων κατακερματισμού
LSH::LSH(int k,int L,vector<vector<double>> &data_vector,int r)
{
    this->data = data_vector;   //Create a copy of the dataset
    this->K = k;
    this->L = L;
    this->r = r;
    for (int i = 0; i < L; i++)
    {   
        //Dimiourgo L Hashtable to kathena me diki tou hash function g
        Hashtable *table = new Hashtable(data_vector.size()/16,i,k,40000,r,data_vector[0].size());   //Create a Hashtable class
        this->hashtables.push_back(table);  //Insert it in the hashtables vector
    }
    
    
    //Insert the data in the Hashtables
    for (int i = 0; i < data_vector.size(); i++)
    {
        for (int j = 0; j < L; j++)
        {
            hashtables[j]->insert(data_vector[i],i);
        }
        
    }
}

//Returns the index of the nearest neighbor of image q
pair<int,int> LSH::nearest_neighbor(vector<double> q,int img_index){
    Metrics metrics = Metrics();
    //Create a map where we hold the distances from the closest images from each Hashtable
    unordered_map<int,int> img_distances;

    //Go through each Hashtable
    for (int i = 0; i < this->L; i++)
    {
        //Get the hash index for q
        int hash_index = this->hashtables[i]->hash_function(q,img_index);
        //Get the image indexes that are in bucket hash index from hashtable i
        vector<int> img_indexes = this->hashtables[i]->get_bucket_imgs(hash_index);
        //Find the index of the image that has the min distance from image q
        int min_distance = metrics.get_distance(this->data[img_indexes[0]],q,(char *)"L1");
        int min_dist_img_index = img_indexes[0];
        for (int j = 1; j < img_indexes.size(); j++)
        {
            int manhattan_dist = metrics.get_distance(this->data[img_indexes[j]],q,(char *)"L1");
            if(manhattan_dist < min_distance){
                min_distance = manhattan_dist;
                min_dist_img_index = img_indexes[j];
            }
        }
        //Add the pair (image_index,distance from q) in the map
        pair<int,int> img_dist (min_dist_img_index,min_distance);
        img_distances.insert(img_dist);
    }
    //Print the map to see if it works
    //for (auto& x: img_distances)
    //    std::cout << x.first << ": " << x.second << std::endl;
    //Find the img_index with the smallest distance and return it
    auto it = img_distances.begin();
    int min_dist = it->second;
    int min_dist_img_index = it->first;
    for (auto& x: img_distances){
        if(x.second < min_dist){
            min_dist = x.second;
            min_dist_img_index = x.first;
        }
    }
    return make_pair(min_dist_img_index,min_dist);
}

vector<pair<int,int>> LSH::knn(vector<double> q,int img_index,int k){
    Metrics metrics = Metrics();
    //Create a map where we hold the distances from the closest images from each Hashtable
    unordered_map<int,int> img_distances;
    vector<pair<int,int>> distances;    //A vector of pairs(img_index,img_distance from q)

    //Go through each Hashtable
    for (int i = 0; i < this->L; i++)
    {
        //Get the hash index for q
        int hash_index = this->hashtables[i]->hash_function(q,img_index);
        //Get the image indexes that are in bucket hash index from hashtable i
        vector<int> img_indexes = this->hashtables[i]->get_bucket_imgs(hash_index);
        //Caluclate the distances from the images and save them in a new vector
        vector<pair<int,int>> temp_distances;   //Pair is the index of the image and it's distance from q
        for (int j = 0; j < img_indexes.size(); j++)
        {
            int manhattan_dist = metrics.get_distance(this->data[img_indexes[j]],q,(char *)"L1");
            temp_distances.push_back(make_pair(img_indexes[j],manhattan_dist));
        }
        //Sort temp distances based on the distance
        sort(temp_distances.begin(), temp_distances.end(), sortbysec);
        //Add the k pairs (image_index,distance from q) in the map
        for (int j = 0; j < temp_distances.size(); j++)
        {
            if (j>=k)
            {
                break; 
            }
            img_distances.insert(temp_distances[j]);
        }
    }
    //Print the map to see if it works
    //for (auto& x: img_distances)
    //    std::cout << x.first << ": " << x.second << std::endl;

    //Insert the pairs from img_distances in a vector,sort it and return the k first indexes.
    for(auto& x:img_distances){
        distances.push_back(make_pair(x.first,x.second));
    }
    //Sort the vector
    sort(distances.begin(),distances.end(), sortbysec);
    //Create a new vector to return the results
    vector<pair<int,int>> results;
    for (int i = 0; i < distances.size(); i++)
    {
        if (i >= k)
        {
            break;
        }
        results.push_back(distances[i]);
    }
    
    return results;       
}

vector<int> LSH::range_search(vector<double> q,int img_index,double r,int c=1){
    Metrics metrics = Metrics();
    //Create a map where we hold the distances from the closest images from each Hashtable
    unordered_map<int,int> img_distances;

    //Go through each Hashtable
    for (int i = 0; i < this->L; i++)
    {
        //Get the hash index for q
        int hash_index = this->hashtables[i]->hash_function(q,img_index);
        //Get the image indexes that are in bucket hash index from hashtable i
        vector<int> img_indexes = this->hashtables[i]->get_bucket_imgs(hash_index);
        //Calculate the distances from image q
        for (int j = 0; j < img_indexes.size(); j++)
        {
            int manhattan_dist = metrics.get_distance(this->data[img_indexes[j]],q,(char *)"L1");
            if(manhattan_dist < c*r){
                //Add the pair (image_index,distance from q) in the map
                pair<int,int> img_dist (img_indexes[j],manhattan_dist);
                img_distances.insert(img_dist);
            }
        }
    }    
    //Create a vector with the images in range r and return it
    vector<int> results;
    for(auto& x:img_distances){
        results.push_back(x.first);
    }
    return results;
}

vector<pair<int,int>> LSH::exact_nearest_neighbor(vector<double> q,int k){
    Metrics metrics = Metrics();
    vector<pair<int,int>> results;
    vector<pair<int,int>> distances;

    //Calculate the distances from all the images and insert them in a vector
    for (int i = 0; i < this->data.size(); i++)
    {
        int distance = metrics.get_distance(this->data[i],q,(char *)"L1");
        distances.push_back(make_pair(i,distance));
    }
    //Sort the distances vector
    sort(distances.begin(),distances.end(), sortbysec);
    //Insert the first k values in results vector and return it
    for (int i = 0; i < this->data.size(); i++)
    {
        if (i >= k)
        {
            break;
        }
        results.push_back(distances[i]);
    }
    return results;
}

LSH::~LSH()
{
    for (int i = 0; i < this->hashtables.size(); i++)
    {
        delete this->hashtables[i];
    }
    
}


#endif