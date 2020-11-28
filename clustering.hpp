#ifndef CLUSTERING_H
#define CLUSTERING_H

#include <time.h>
#include <fstream>
#include <iostream>
#include <vector>
#include "metrics.hpp"
#include "lsh.hpp"
#include"binary_hyper_cube.hpp"

#define R 10000
#define YES 1
#define NO 0
#define FLAG 0
#define CLUSTER 1
using namespace std;

double get_max(double a, double b)
{
    if (a > b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

class Clustering
{
private:
    vector<vector<double>> data;

public:
    Clustering(vector<vector<double>> &);
    ~Clustering();
    pair<vector<vector<int>>, vector<vector<double>>> loyds(int k);
    pair<vector<vector<int>>, vector<vector<double>>> lsh(int k, int L, int KforLSH);
    pair<vector<vector<int>>, vector<vector<double>>> hypercube(int k,int HyperCube_k,int M,int probes);
    vector<vector<double>> silhouette_score(vector<vector<int>> clusters, vector<vector<double>> centroids);
};

Clustering::Clustering(vector<vector<double>> &data_vector)
{
    this->data = data_vector;
}

//Return a vector with the cluster that each image belongs
pair<vector<vector<int>>, vector<vector<double>>> Clustering::loyds(int k)
{
    Metrics metrics = Metrics();
    vector<vector<int>> clusters; //clusters[0]->vector with image numbers that belong to cluster 0
    //Initialize the clusters
    clusters.resize(k, vector<int>(0));

    //Initialize the centroids
    vector<vector<double>> centroids;
    centroids.resize(k, vector<double>(this->data[0].size()));
    //Initialize the new centroids vector
    vector<vector<double>> new_centroids;
    new_centroids.resize(k, vector<double>(this->data[0].size()));

    double centroids_difference = -10;

    //Choose k random centers from the dataset
    srand(time(NULL));
    for (int i = 0; i < k; i++)
    {
        int img_number = rand() % (this->data.size() - 1) + 0;
        for (int j = 0; j < this->data[0].size(); j++)
        {
            centroids[i][j] = this->data[img_number][j];
        }
    }

    while (1)
    {
        if (centroids_difference < 1000.0 && centroids_difference >= 0.0)
        {
            break;
        }
        centroids_difference = 0.0;
        //Initialize the clusters
        for (int i = 0; i < k; i++)
        {
            if (clusters[i].size() > 0)
            {
                clusters[i].clear();
            }
        }
        clusters.resize(k, vector<int>(0));
        //Assign the data to the clusters
        for (int i = 0; i < this->data.size(); i++)
        {
            //Compute the distance from each cluster
            vector<pair<int, int>> distances; //A pair with cluster number and distance
            for (int j = 0; j < k; j++)
            {
                int manhattan_dist = metrics.get_distance(this->data[i], centroids[j], (char *)"L1");
                distances.push_back(make_pair(j, manhattan_dist));
            }
            //Sort distances vector
            sort(distances.begin(), distances.end(), sortbysec);
            //Assign the closest cluster to the image
            clusters[distances[0].first].push_back(i);
        }

        //Update the centroids
        for (int i = 0; i < k; i++) //Number of centroids
        {
            for (int c = 0; c < this->data[clusters[i][0]].size(); c++) //Get each column from the images in the cluster
            {
                double sum = 0.0;
                for (int r = 0; r < clusters[i].size(); r++) //Get each row from the images in the cluster
                {
                    sum += this->data[clusters[i][r]][c]; //Sum their data
                }
                //Calculate the mean
                double mean = sum / clusters[i].size();
                //Update the centroid column
                new_centroids[i][c] = mean;
            }
        }

        //Find how different are the new_centroids from the old ones
        for (int i = 0; i < k; i++)
        {
            for (int j = 0; j < centroids[i].size(); j++)
            {
                centroids_difference += abs(new_centroids[i][j] - centroids[i][j]);
                centroids[i][j] = new_centroids[i][j]; //Update centroids to new_centroids for the next iteration
            }
        }
    }
    return make_pair(clusters, centroids);
}

pair<vector<vector<int>>, vector<vector<double>>> Clustering::lsh(int k, int L, int KforLSH)
{
    Metrics metrics = Metrics();
    vector<vector<int>> clusters; //clusters[0]->vector with image numbers that belong to cluster 0
    //Initialize the clusters
    clusters.resize(k, vector<int>(0));

    //Initialize the centroids
    vector<vector<double>> centroids;
    centroids.resize(k, vector<double>(this->data[0].size()));
    //Initialize the new centroids vector
    vector<vector<double>> new_centroids;
    new_centroids.resize(k, vector<double>(this->data[0].size()));

    double centroids_difference = -10;

    //Choose k random centers from the dataset
    srand(time(NULL));
    for (int i = 0; i < k; i++)
    {
        int img_number = rand() % (this->data.size() - 1) + 0;
        for (int j = 0; j < this->data[0].size(); j++)
        {
            centroids[i][j] = this->data[img_number][j];
        }
    }
    //Initialize the clusters
    for (int i = 0; i < k; i++)
    {
        if (clusters[i].size() > 0)
        {
            clusters[i].clear();
        }
    }

    //Mark assigned points

    //Initialize vector with zeros
    //O pinakas exi diastasis data.size*2
    //Stin proti stili mpeni to flag(0 h 1) an exi xrisimopoithi i ikona kai stin deuteri stili mpeni o arithmos tou cluster opou aniki
    vector<vector<int>> AssignedPoints(this->data.size(), vector<int>(2, 0));

    //LSH Initialization
    LSH lsh = LSH(KforLSH, L, this->data, R);

    int r = R;
    for (int m = 0; m < 10; m++)
    {

        //Assign the data to the clusters
        for (int i = 0; i < k; i++)
        {

            //Do the range search
            vector<int> range_results = lsh.range_search(centroids[i], i, r);

            //gemizoume to cluster me ta kontina simia pou mas edose to range search
            for (int j = 0; j < range_results.size(); j++)
            {

                //an to simio den aniki se kapoio cluster tote to bazoume sto cluster
                if (AssignedPoints[range_results[j]][FLAG] == NO)
                {
                    AssignedPoints[range_results[j]][FLAG] = 1;
                    AssignedPoints[range_results[j]][CLUSTER] = i;
                    clusters[i].push_back(range_results[j]);
                }
                //an to simio den aniki se kapio cluster epilegoume to kontinotero cluster
                else
                {
                    vector<pair<int, int>> distances; //A pair with cluster number and distance
                    int manhattan_dist;

                    //ipologizi tin apostasi apo to paron cluster
                    manhattan_dist = metrics.get_distance(this->data[range_results[j]], centroids[i], (char *)"L1");
                    distances.push_back(make_pair(i, manhattan_dist));

                    //ipologizi tin apostasi apo to cluster opou aniki
                    manhattan_dist = metrics.get_distance(this->data[range_results[j]], centroids[AssignedPoints[range_results[j]][CLUSTER]], (char *)"L1");
                    distances.push_back(make_pair(AssignedPoints[range_results[j]][CLUSTER], manhattan_dist));

                    //Sort distances vector
                    sort(distances.begin(), distances.end(), sortbysec);
                    //Assign the closest cluster to the image
                    if (distances[0].first == j)
                    {
                        //remove image from old cluster
                        clusters[AssignedPoints[range_results[j]][CLUSTER]].erase(remove(clusters[AssignedPoints[range_results[j]][CLUSTER]].begin(), clusters[AssignedPoints[range_results[j]][CLUSTER]].end(), i), clusters[AssignedPoints[range_results[j]][CLUSTER]].end());

                        clusters[distances[0].first].push_back(range_results[j]);
                        AssignedPoints[range_results[j]][CLUSTER] = i;
                    }
                }
            }
        }
        r = r * 2;
    }

    //Update the centroids
    for (int i = 0; i < k; i++) //Number of centroids
    {
        for (int c = 0; c < this->data[clusters[i][0]].size(); c++) //Get each column from the images in the cluster
        {
            double sum = 0.0;
            for (int r = 0; r < clusters[i].size(); r++) //Get each row from the images in the cluster
            {
                sum += this->data[clusters[i][r]][c]; //Sum their data
            }
            //Calculate the mean
            double mean = sum / clusters[i].size();
            //Update the centroid column
            new_centroids[i][c] = mean;
        }
    }

    //Find how different are the new_centroids from the old ones
    for (int i = 0; i < k; i++)
    {
        for (int j = 0; j < centroids[i].size(); j++)
        {
            centroids_difference += abs(new_centroids[i][j] - centroids[i][j]);
            centroids[i][j] = new_centroids[i][j]; //Update centroids to new_centroids for the next iteration
        }
    }
    while (1)
    {
        if (centroids_difference < 1000.0 && centroids_difference >= 0.0)
        {
            break;
        }
        centroids_difference = 0.0;
        //Initialize the clusters
        for (int i = 0; i < k; i++)
        {
            if (clusters[i].size() > 0)
            {
                clusters[i].clear();
            }
        }
        clusters.resize(k, vector<int>(0));
        //Assign the data to the clusters
        for (int i = 0; i < this->data.size(); i++)
        {
            //Compute the distance from each cluster
            vector<pair<int, int>> distances; //A pair with cluster number and distance
            for (int j = 0; j < k; j++)
            {
                int manhattan_dist = metrics.get_distance(this->data[i], centroids[j], (char *)"L1");
                distances.push_back(make_pair(j, manhattan_dist));
            }
            //Sort distances vector
            sort(distances.begin(), distances.end(), sortbysec);
            //Assign the closest cluster to the image
            clusters[distances[0].first].push_back(i);
        }

        //Update the centroids
        for (int i = 0; i < k; i++) //Number of centroids
        {
            for (int c = 0; c < this->data[clusters[i][0]].size(); c++) //Get each column from the images in the cluster
            {
                double sum = 0.0;
                for (int r = 0; r < clusters[i].size(); r++) //Get each row from the images in the cluster
                {
                    sum += this->data[clusters[i][r]][c]; //Sum their data
                }
                //Calculate the mean
                double mean = sum / clusters[i].size();
                //Update the centroid column
                new_centroids[i][c] = mean;
            }
        }

        //Find how different are the new_centroids from the old ones
        for (int i = 0; i < k; i++)
        {
            for (int j = 0; j < centroids[i].size(); j++)
            {
                centroids_difference += abs(new_centroids[i][j] - centroids[i][j]);
                centroids[i][j] = new_centroids[i][j]; //Update centroids to new_centroids for the next iteration
            }
        }
    }
    return make_pair(clusters, centroids);
}


pair<vector<vector<int>>, vector<vector<double>>>Clustering::hypercube(int k,int HyperCube_k,int M,int probes){
    Metrics metrics = Metrics();
    vector<vector<int>> clusters; //clusters[0]->vector with image numbers that belong to cluster 0
    //Initialize the clusters
    clusters.resize(k, vector<int>(0));

    //Initialize the centroids
    vector<vector<double>> centroids;
    centroids.resize(k, vector<double>(this->data[0].size()));
    //Initialize the new centroids vector
    vector<vector<double>> new_centroids;
    new_centroids.resize(k, vector<double>(this->data[0].size()));

    double centroids_difference = -10;

    //Choose k random centers from the dataset
    srand(time(NULL));
    for (int i = 0; i < k; i++)
    {
        int img_number = rand() % (this->data.size() - 1) + 0;
        for (int j = 0; j < this->data[0].size(); j++)
        {
            centroids[i][j] = this->data[img_number][j];
        }
    }
    //Initialize the clusters
    for (int i = 0; i < k; i++)
    {
        if (clusters[i].size() > 0)
        {
            clusters[i].clear();
        }
    }

    //Mark assigned points
    //Initialize vector with zeros
    //O pinakas exi diastasis data.size*2
    //Stin proti stili mpeni to flag(0 h 1) an exi xrisimopoithi i ikona kai stin deuteri stili mpeni o arithmos tou cluster opou aniki
    vector<vector<int>> AssignedPoints(this->data.size(), vector<int>(2, 0));

    //HyperCube Initialization
    BinaryHyperCube cube = BinaryHyperCube(this->data,HyperCube_k,M,probes,R);

    int r = R;
    for (int m = 0; m < 10; m++)
    {

        //Assign the data to the clusters
        for (int i = 0; i < k; i++)
        {

            //Do the range search
            vector<int> range_results = cube.range_search(centroids[i], i, r);

            //gemizoume to cluster me ta kontina simia pou mas edose to range search
            for (int j = 0; j < range_results.size(); j++)
            {

                //an to simio den aniki se kapoio cluster tote to bazoume sto cluster
                if (AssignedPoints[range_results[j]][FLAG] == NO)
                {
                    AssignedPoints[range_results[j]][FLAG] = 1;
                    AssignedPoints[range_results[j]][CLUSTER] = i;
                    clusters[i].push_back(range_results[j]);
                }
                //an to simio ani se kapio cluster epilegoume to kontinotero cluster
                else
                {
                    vector<pair<int, int>> distances; //A pair with cluster number and distance
                    int manhattan_dist;

                    //ipologizi tin apostasi apo to paron cluster
                    manhattan_dist = metrics.get_distance(this->data[range_results[j]], centroids[i], (char *)"L1");
                    distances.push_back(make_pair(i, manhattan_dist));

                    //ipologizi tin apostasi apo to cluster opou aniki
                    manhattan_dist = metrics.get_distance(this->data[range_results[j]], centroids[AssignedPoints[range_results[j]][CLUSTER]], (char *)"L1");
                    distances.push_back(make_pair(AssignedPoints[range_results[j]][CLUSTER], manhattan_dist));

                    //Sort distances vector
                    sort(distances.begin(), distances.end(), sortbysec);
                    //Assign the closest cluster to the image
                    if (distances[0].first == j)
                    {
                        //remove image from old cluster
                        clusters[AssignedPoints[range_results[j]][CLUSTER]].erase(remove(clusters[AssignedPoints[range_results[j]][CLUSTER]].begin(), clusters[AssignedPoints[range_results[j]][CLUSTER]].end(), i), clusters[AssignedPoints[range_results[j]][CLUSTER]].end());

                        clusters[distances[0].first].push_back(range_results[j]);
                        AssignedPoints[range_results[j]][CLUSTER] = i;
                    }
                }
            }
        }
        r = r * 2;
    }

    //Update the centroids
    for (int i = 0; i < k; i++) //Number of centroids
    {
        for (int c = 0; c < this->data[clusters[i][0]].size(); c++) //Get each column from the images in the cluster
        {
            double sum = 0.0;
            for (int r = 0; r < clusters[i].size(); r++) //Get each row from the images in the cluster
            {
                sum += this->data[clusters[i][r]][c]; //Sum their data
            }
            //Calculate the mean
            double mean = sum / clusters[i].size();
            //Update the centroid column
            new_centroids[i][c] = mean;
        }
    }

    //Find how different are the new_centroids from the old ones
    for (int i = 0; i < k; i++)
    {
        for (int j = 0; j < centroids[i].size(); j++)
        {
            centroids_difference += abs(new_centroids[i][j] - centroids[i][j]);
            centroids[i][j] = new_centroids[i][j]; //Update centroids to new_centroids for the next iteration
        }
    }
    while (1)
    {
        if (centroids_difference < 1000.0 && centroids_difference >= 0.0)
        {
            break;
        }
        centroids_difference = 0.0;
        //Initialize the clusters
        for (int i = 0; i < k; i++)
        {
            if (clusters[i].size() > 0)
            {
                clusters[i].clear();
            }
        }
        clusters.resize(k, vector<int>(0));
        //Assign the data to the clusters
        for (int i = 0; i < this->data.size(); i++)
        {
            //Compute the distance from each cluster
            vector<pair<int, int>> distances; //A pair with cluster number and distance
            for (int j = 0; j < k; j++)
            {
                int manhattan_dist = metrics.get_distance(this->data[i], centroids[j], (char *)"L1");
                distances.push_back(make_pair(j, manhattan_dist));
            }
            //Sort distances vector
            sort(distances.begin(), distances.end(), sortbysec);
            //Assign the closest cluster to the image
            clusters[distances[0].first].push_back(i);
        }

        //Update the centroids
        for (int i = 0; i < k; i++) //Number of centroids
        {
            for (int c = 0; c < this->data[clusters[i][0]].size(); c++) //Get each column from the images in the cluster
            {
                double sum = 0.0;
                for (int r = 0; r < clusters[i].size(); r++) //Get each row from the images in the cluster
                {
                    sum += this->data[clusters[i][r]][c]; //Sum their data
                }
                //Calculate the mean
                double mean = sum / clusters[i].size();
                //Update the centroid column
                new_centroids[i][c] = mean;
            }
        }

        //Find how different are the new_centroids from the old ones
        for (int i = 0; i < k; i++)
        {
            for (int j = 0; j < centroids[i].size(); j++)
            {
                centroids_difference += abs(new_centroids[i][j] - centroids[i][j]);
                centroids[i][j] = new_centroids[i][j]; //Update centroids to new_centroids for the next iteration
            }
        }
    }
    return make_pair(clusters, centroids);    
}

vector<vector<double>> Clustering::silhouette_score(vector<vector<int>> clusters, vector<vector<double>> centroids)
{
    vector<vector<double>> scores;
    vector<double> a, b; //Apo diafanies slide 54

    scores.resize(clusters.size(), vector<double>(0));

    Metrics metrics = Metrics();

    //Calcuate the silhouette score of each image in each cluster
    for (int i = 0; i < clusters.size(); i++)
    {
        if (a.size() > 0)
            a.clear();
        if (b.size() > 0)
            b.clear();
        for (int j = 0; j < clusters[i].size(); j++)
        {
            //Calculate a vector
            double sum = 0;
            //Calculate average distance of j image to images in the same cluster
            for (int z = 0; z < clusters[i].size(); z++)
            {
                sum += metrics.get_distance(this->data[clusters[i][j]], this->data[clusters[i][z]], (char *)"L1");
            }
            a.push_back(double(sum / clusters[i].size()));
            //Find 2nd closest centroid
            vector<pair<int, int>> distances;
            for (int k = 0; k < centroids.size(); k++)
            {
                int manhattan_dist = metrics.get_distance(this->data[clusters[i][j]], centroids[k], (char *)"L1");
                distances.push_back(make_pair(k, manhattan_dist));
            }
            //Sort distances
            sort(distances.begin(), distances.end(), sortbysec);
            int second_best = distances[1].first;
            ////Calculate average distance of j image to images in the second best cluster(b vector)
            sum = 0;
            for (int z = 0; z < clusters[second_best].size(); z++)
            {
                sum += metrics.get_distance(this->data[clusters[i][j]], this->data[clusters[second_best][z]], (char *)"L1");
            }
            b.push_back(double(sum / clusters[i].size()));
        }
        //Calculate s vector
        for (int j = 0; j < a.size(); j++)
        {
            scores[i].push_back((b[j] - a[j]) / get_max(a[j], b[j]));
        }
    }
    return scores;
}

Clustering::~Clustering()
{
}

#endif