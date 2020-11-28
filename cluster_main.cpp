#include "reader.hpp"
#include "clustering.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

using namespace std;

void print_help()
{
    cout << "Usage is:" << endl;
    cout << "./cluster -i <input file> -c <configuration file> -o <output file> -complete(optional) -m <method: Lloyds OR LSH or Hypercube>" << endl;
}

int main(int argc, char const *argv[])
{
    int K, L = 3, number_of_vector_hash_functions = 4;
    int max_number_M_hypercube = 10, number_of_hypercube_dimensions = 3, number_of_probes = 2;

    char input_file[250], output_file[250], conf_file[250];
    char method[50];
    bool complete_flag = false;

    if (argc > 8)
    {
        //Parse the command line arguments
        for (int i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "-i") == 0)
            {
                if (i + 1 < argc)
                {
                    stpcpy(input_file, argv[i + 1]);
                    i++;
                    continue;
                }
                else
                {
                    print_help();
                    exit(1);
                }
            }

            if (strcmp(argv[i], "-c") == 0)
            {
                if (i + 1 < argc)
                {
                    stpcpy(conf_file, argv[i + 1]);
                    i++;
                    continue;
                }
                else
                {
                    print_help();
                    exit(1);
                }
            }

            if (strcmp(argv[i], "-o") == 0)
            {
                if (i + 1 < argc)
                {
                    stpcpy(output_file, argv[i + 1]);
                    i++;
                    continue;
                }
                else
                {
                    print_help();
                    exit(1);
                }
            }

            if (strcmp(argv[i], "-m") == 0)
            {
                if (i + 1 < argc)
                {
                    strcpy(method, argv[i + 1]);
                    i++;
                    continue;
                }
                else
                {
                    print_help();
                    exit(1);
                }
            }

            if (strcmp(argv[i], "-complete") == 0)
            {
                complete_flag = true;
            }
        }
    }
    else //Print help and exit
    {
        print_help();
        exit(1);
    }

    /*cout << "Input file is:" << input_file << endl;
    cout << "Conf file is:" << conf_file << endl;
    cout << "Output file is:" << output_file << endl;
    cout << "Complete flag is:" << complete_flag << endl;
    cout << "Method is:" << method << endl;
    cout << strcmp(method,"Lloyds") << endl;*/
    char method_copy[100];
    strcpy(method_copy,method);

    //Read the conf file
    FILE *fp = fopen(conf_file, "r");
    if (fp == NULL)
    {
        cout << "Error during opening the file" << endl;
        exit(1);
    }
    char buf[250];
    char variable[100];
    char temp_value[10];
    int value;

    while (fgets(buf, 250, fp))
    {
        int buf_index = 0;
        int index = 0;
        memset(variable, 0, 100);
        memset(temp_value, 0, 10);
        while (buf[buf_index] != ':')
        {
            variable[index] = buf[buf_index];
            index++;
            buf_index++;
        }
        variable[index] = '\0';
        buf_index++;

        index = 0;
        while (buf[buf_index] != '\n' || buf[buf_index] == EOF)
        {
            temp_value[index] = buf[buf_index];
            index++;
            buf_index++;
        }
        index++;
        buf_index++;
        temp_value[index] = '\0';
        value = atoi(temp_value);
        if (strlen(temp_value) == 0)
        {
            continue;
        }
        if (strcmp(variable, "number_of_clusters") == 0)
        {
            K = value;
            continue;
        }
        if (strcmp(variable, "number_of_vector_hash_tables") == 0)
        {
            L = value;
            continue;
        }
        if (strcmp(variable, "number_of_vector_hash_functions") == 0)
        {
            number_of_vector_hash_functions = value;
            continue;
        }
        if (strcmp(variable, "max_number_M_hypercube") == 0)
        {
            max_number_M_hypercube = value;
            continue;
        }
        if (strcmp(variable, "number_of_hypercube_dimensions") == 0)
        {
            number_of_hypercube_dimensions = value;
            continue;
        }
        if (strcmp(variable, "number_of_probes") == 0)
        {
            number_of_probes = value;
            continue;
        }
    }
    fclose(fp);
    /*cout << "Number of clusters:" << K << endl;
    cout << "number_of_vector_hash_tables:" << L << endl;
    cout << "number_of_vector_hash_functions:" << number_of_vector_hash_functions << endl;
    cout << "max_number_M_hypercube:" << max_number_M_hypercube << endl;
    cout << "number_of_hypercube_dimensions:" << number_of_hypercube_dimensions << endl;
    cout << "number of probes:" << number_of_probes << endl;*/

    //Read the training dataset
    vector<vector<double>> training_data;
    if (ReadData(training_data, (char *)input_file))
    {
        exit(1);
    }
    Clustering cluster = Clustering(training_data);
    clock_t start, end;
    pair<vector<vector<int>>, vector<vector<double>>> results;
    start = clock();
    
    if (strcmp(method_copy,"Lloyds") == 0){
        results = cluster.loyds(K);
    }
    else if (strcmp(method_copy, "LSH") == 0)
        results = cluster.lsh(K, L, number_of_vector_hash_functions);
    else if (strcmp(method_copy, "Hypercube") == 0)
        results = cluster.hypercube(K, number_of_hypercube_dimensions, max_number_M_hypercube, number_of_probes);
    else
    {
        cout << "Wrong method given\n";
        exit(1);
    }
    end = clock();
    vector<vector<double>> scores = cluster.silhouette_score(results.first, results.second);
    vector<double>silhouete_scores;
    double s_total = 0.0;
    for (int i = 0; i < scores.size(); i++)
    {
        double sum = 0.0;
        for (int j = 0; j < scores[i].size(); j++)
        {
            sum+= scores[i][j];
        }
        silhouete_scores.push_back(sum/scores[i].size());
        s_total += sum/scores[i].size();
    }
    
    //Write the output to the output file
    ofstream outfile;
    outfile.open(output_file); //Create the output file

    outfile << "Algorithm:" << method << "\n";
    for (int i = 0; i < results.first.size(); i++)
    {
        outfile << "CLUSTER-" << i + 1 << " {size:" << results.first[i].size();
        outfile << ",centroid:";
        for (int j = 0; j < results.second[i].size(); j++)
        {
            outfile << results.second[i][j] << "  ";
        }
        outfile << "}";
    }
    double clustering_time = double(end - start) / double(CLOCKS_PER_SEC);
    outfile << "Clustering time:" << clustering_time << "\n";

    outfile << "Silhouette:[";
    for (int i = 0; i < silhouete_scores.size(); i++)
    {
        outfile << silhouete_scores[i] << " ";
    }
    outfile << s_total << " ]\n";

    if (complete_flag)
    {
        for (int i = 0; i < results.first.size(); i++)
        {
            outfile << "CLUSTER-" << i + 1 << " {";
            for (int j = 0; j < results.first[i].size(); j++)
            {
                outfile << "," << results.first[i][j];
            }
            outfile << "\n";
        }
    }
    outfile.close();
    return 0;
}
