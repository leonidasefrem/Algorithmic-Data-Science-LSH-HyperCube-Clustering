#ifndef BINARY_HYPER_CUBE_H
#define BINARY_HYPER_CUBE_H
#include <iostream>
#include "hashtable.hpp"
#include "metrics.hpp"
#include <time.h>
#include <math.h>

using namespace std;

class BinaryHyperCube
{
private:
    vector<vector<double>> data; //image training  dataset
    vector<vector<int>> hyper_cube;
    vector<vector<int>> s_vectors;
    int d; //d' apo diafanies
    int M;
    int probes;
    int R;

public:
    BinaryHyperCube(vector<vector<double>> &data_vector, int k, int M, int probes, int R);
    ~BinaryHyperCube();
    int f(int);
    int h(vector<double> &, int);
    int get_number_from_bits(vector<int>);
    int hamming_distance(int, int);
    vector<pair<int, int>> knn(vector<double> q, int img_index, int k); //Approximate
    vector<int> range_search(vector<double> q, int img_index, double r, int c);
    vector<pair<int, int>> exact_nearest_neighbor(vector<double> q, int k);
    vector<int> get_bucket_imgs(int bucket_index);
};

BinaryHyperCube::BinaryHyperCube(vector<vector<double>> &data_vector, int k, int M, int probes, int R)
{
    this->data = data_vector;
    this->d = k;
    this->M = M;
    this->probes = probes;
    this->R = R;

    //Resize s_vectors
    this->s_vectors.resize(this->d, vector<int>(data_vector[0].size()));

    //Initialize s_vectors
    for (int i = 0; i < k; i++)
    {
        srand(i);
        for (int j = 0; j < data_vector[0].size(); j++)
        {
            this->s_vectors[i][j] = rand() % (40000 - 1) + 0;
        }
    }

    //Initialize the hypercube
    this->hyper_cube.resize(pow(2, this->d), vector<int>(0));

    //Create a vector to store f function results
    vector<int> f_results; //Contains only 0 and 1
    //Insert the data in the hypercube
    for (int i = 0; i < data_vector.size(); i++)
    {
        //Compute f results
        for (int j = 0; j < this->d; j++)
        {
            int h_result = this->h(data_vector[i], j);
            f_results.push_back(this->f(h_result));
        }
        
        int hyper_cube_index = this->get_number_from_bits(f_results);
        this->hyper_cube[hyper_cube_index].push_back(i); //Insert the index of the img in the hypercube
        f_results.clear();                               //Clear the f_results vector for the next image
    }
}

int BinaryHyperCube::h(vector<double> &image, int index)
{
    vector<int> a;
    unsigned int hash_result;

    //Calculate a vector
    for (int i = 0; i < this->s_vectors[index].size(); i++)
    {
        double temp_a = double(image[i] - s_vectors[index][i]) / double(40000);
        a.push_back(int(round(temp_a)));
    }
    //Calculate h(image)
    int m = pow(2, 32 - 3);
    int M = pow(2, 32 / 4); //to 4(=k)
    hash_result = a[a.size() - 1] % M;
    for (int d = a.size() - 2; d >= 0; d--)
    {
        hash_result += (a[d] * m) % M;
        m = m * m;
    }
    hash_result = hash_result % M;
    return hash_result;
}

//An exi parapano 1 bits apo 0 bits epistrefi 1 allios 0
int BinaryHyperCube::f(int hash_result)
{
    int one_count = 0;
    int zero_count = 0;
    //Get the bits of hash result
    int k;
    for (k = 0; k < 32; k++)
    {
        int mask = 1 << k;
        int masked_n = hash_result & mask;
        int thebit = masked_n >> k;
        if (thebit == 1)
        {
            one_count++;
        }
        else
        {
            zero_count++;
        }
    }
    if (one_count >= zero_count)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//Given a vector of bits return the number they represent
int BinaryHyperCube::get_number_from_bits(vector<int> bits)
{
    int number = 0;
    for (int i = bits.size() - 1; i >= 0; i--)
    {
        if (bits[i] == 1)
        {
            number += pow(2, bits.size() - 1 - i);
        }
    }
    return number;
}

//Get the hamming distance of two ints
int BinaryHyperCube::hamming_distance(int a, int b)
{
    int a_bits[32];
    int b_bits[32];
    //Get a bits
    int k;
    for (k = 0; k < 32; k++)
    {
        int mask = 1 << k;
        int masked_n = a & mask;
        int thebit = masked_n >> k;
        a_bits[k] = thebit;
    }
    //Get b_bits
    for (k = 0; k < 32; k++)
    {
        int mask = 1 << k;
        int masked_n = b & mask;
        int thebit = masked_n >> k;
        b_bits[k] = thebit;
    }
    //Get hamming distance
    int hamming_distance = 0;
    for (int i = 0; i < 32; i++)
    {
        if (a_bits[i] != b_bits[i])
        {
            hamming_distance++;
        }
    }
    return hamming_distance;
}

vector<pair<int, int>> BinaryHyperCube::knn(vector<double> q, int img_index, int k)
{
    Metrics metrics = Metrics();
    //Create a map where we hold the distances from the closest images from each Hashtable
    unordered_map<int, int> img_distances;
    vector<pair<int, int>> distances; //A vector of pairs(img_index,img_distance from q)
    int probes_counter = 0;
    //Calculate hypercube vertex of query image
    vector<int> f_results; //Contains only 0 and 1
    //Compute f results
    for (int j = 0; j < this->d; j++)
    {
        int h_result = this->h(q, j);
        f_results.push_back(this->f(h_result));
    }
    int hyper_cube_index = this->get_number_from_bits(f_results);   //hypercube vertex
    //Get the index of images from this vertex
    vector<int> img_indexes = this->get_bucket_imgs(hyper_cube_index);
    //Check the images from this vertex
    vector<pair<int,int>> temp_distances;
    for (int i = 0; i < img_indexes.size(); i++)
    {
        int manhattan_dist = metrics.get_distance(this->data[img_indexes[i]],q,(char *)"L1");
        img_distances.insert(make_pair(img_indexes[i],manhattan_dist)); //Insert the pair in the map
    }
    img_indexes.clear();
    //Check nearby vertices that have hamming distance with hypercube index <= M
    for (int i = 0; i < this->hyper_cube.size(); i++)   
    {   
        if(probes_counter >= this->probes){
            break;
        }
        if ( (i!=hyper_cube_index) && (this->hamming_distance(i,hyper_cube_index)<=this->M) )
        {
            img_indexes = this->get_bucket_imgs(i);
            for (int i = 0; i < img_indexes.size(); i++)
            {
                int manhattan_dist = metrics.get_distance(this->data[img_indexes[i]],q,(char *)"L1");
                img_distances.insert(make_pair(img_indexes[i],manhattan_dist)); //Insert the pair in the map
            }
            img_indexes.clear();
            probes_counter++;
        }
    }
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

vector<int> BinaryHyperCube::range_search(vector<double> q, int img_index, double r, int c = 1)
{
    Metrics metrics = Metrics();
    //Create a map where we hold the distances from the closest images from each Hashtable
    unordered_map<int, int> img_distances;
    vector<pair<int, int>> distances; //A vector of pairs(img_index,img_distance from q)
    int probes_counter = 0;
    //Calculate hypercube vertex of query image
    vector<int> f_results; //Contains only 0 and 1
    //Compute f results
    for (int j = 0; j < this->d; j++)
    {
        int h_result = this->h(q, j);
        f_results.push_back(this->f(h_result));
    }
    int hyper_cube_index = this->get_number_from_bits(f_results);   //hypercube vertex
    //Get the index of images from this vertex
    vector<int> img_indexes = this->get_bucket_imgs(hyper_cube_index);
    //Check the images from this vertex
    vector<pair<int,int>> temp_distances;
    for (int i = 0; i < img_indexes.size(); i++)
    {
        int manhattan_dist = metrics.get_distance(this->data[img_indexes[i]],q,(char *)"L1");
        if(manhattan_dist < c*r)
            img_distances.insert(make_pair(img_indexes[i],manhattan_dist)); //Insert the pair in the map
    }
    img_indexes.clear();
    //Check nearby vertices that have hamming distance with hypercube index <= M
    for (int i = 0; i < this->hyper_cube.size(); i++)   
    {
        if(probes_counter >= this->probes){
            break;
        }
        if ( (i!=hyper_cube_index) && (this->hamming_distance(i,hyper_cube_index)<=this->probes) )
        {
            img_indexes = this->get_bucket_imgs(i);
            for (int i = 0; i < img_indexes.size(); i++)
            {
                int manhattan_dist = metrics.get_distance(this->data[img_indexes[i]],q,(char *)"L1");
                if(manhattan_dist < c*r)
                    img_distances.insert(make_pair(img_indexes[i],manhattan_dist)); //Insert the pair in the map
            }
            img_indexes.clear();
        }
    }
    //Create a vector with the images in range r and return it
    vector<int> results;
    for(auto& x:img_distances){
        results.push_back(x.first);
    }
    return results;        
}

vector<pair<int, int>> BinaryHyperCube::exact_nearest_neighbor(vector<double> q, int k)
{
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

//Returns the vector with the images indexes that are in bucket "bucket index"
vector<int> BinaryHyperCube::get_bucket_imgs(int bucket_index){
    return this->hyper_cube[bucket_index];
}


BinaryHyperCube::~BinaryHyperCube()
{
}

#endif