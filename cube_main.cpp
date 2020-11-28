#include "reader.hpp"
#include "binary_hyper_cube.hpp"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

using namespace std;

void print_help()
{
    cout << "Usage is:" << endl;
    cout << "./cube -d <input file> -q <query file> -k <int> -M  <int> -probes <int> -ο <output file> -Ν <number of nearest> -R <radius>";
    cout << endl;
}

int main(int argc, char const *argv[])
{
    int k = 3, M = 10, probes = 2, N = 1;
    double R = 1.0;
    char input_file[250];
    char output_file[250];
    char query_file[250];

    if (argc == 17)
    {
        //Parse the command line arguments
        for (int i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "-d") == 0)
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

            if (strcmp(argv[i], "-q") == 0)
            {
                if (i + 1 < argc)
                {
                    stpcpy(query_file, argv[i + 1]);
                    i++;
                    continue;
                }
                else
                {
                    print_help();
                    exit(1);
                }
            }

            if (strcmp(argv[i], "-k") == 0)
            {
                if (i + 1 < argc)
                {
                    k = atoi(argv[i + 1]);
                    i++;
                    continue;
                }
                else
                {
                    print_help();
                    exit(1);
                }
            }

            if (strcmp(argv[i], "-M") == 0)
            {
                if (i + 1 < argc)
                {
                    M = atoi(argv[i + 1]);
                    i++;
                    continue;
                }
                else
                {
                    print_help();
                    exit(1);
                }
            }

            if (strcmp(argv[i], "-probes") == 0)
            {
                if (i + 1 < argc)
                {
                    probes = atoi(argv[i + 1]);
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

            if (strcmp(argv[i], "-N") == 0)
            {
                if (i + 1 < argc)
                {
                    N = atoi(argv[i + 1]);
                    i++;
                    continue;
                }
                else
                {
                    print_help();
                    exit(1);
                }
            }

            if (strcmp(argv[i], "-R") == 0)
            {
                if (i + 1 < argc)
                {
                    R = atof(argv[i + 1]);
                    i++;
                    continue;
                }
                else
                {
                    print_help();
                    exit(1);
                }
            }
        }
    }
    else //Read the filepaths from the user
    {
        cout << "Give input filepath:" << endl;
        fgets(input_file, 250, stdin);
        input_file[strlen(input_file) - 1] = '\0'; //Strip the '\n' character
        cout << "Give query filepath:" << endl;
        fgets(query_file, 250, stdin);
        query_file[strlen(query_file) - 1] = '\0'; //Strip the '\n' character
        cout << "Give output filepath:" << endl;
        fgets(output_file, 250, stdin);
        output_file[strlen(output_file) - 1] = '\0'; //Strip the '\n' character
    }

    //Read the training dataset
    vector<vector<double>> training_data;
    if (ReadData(training_data, (char *)input_file))
    {
        exit(1);
    }

    //Read the query dataset
    vector<vector<double>> query_data;
    if (ReadData(query_data, (char *)query_file))
    {
        exit(1);
    }

    BinaryHyperCube BhyperCube = BinaryHyperCube(training_data, k, M, probes, R);

    ofstream outfile;
    outfile.open(output_file); //Create the outpout file

    for (int i = 0; i < 10; i++)
    {
        //Find the approximate N nearest neighbors
        clock_t start, end;
        start = clock();
        vector<pair<int, int>> appr_results = BhyperCube.knn(query_data[i], i, N);
        end = clock();
        double appr_knn_time = double(end - start) / double(CLOCKS_PER_SEC);
        //Find the exact nearest neighbors
        start = clock();
        vector<pair<int, int>> exact_results = BhyperCube.exact_nearest_neighbor(query_data[i], N);
        end = clock();
        double exact_knn_time = double(end - start) / double(CLOCKS_PER_SEC);
        //Do the range search
        vector<int> range_results = BhyperCube.range_search(query_data[i], i, R);
        //Write the results in the output file
        outfile << "Query:" << i << "\n";
        for (int j = 0; j < appr_results.size(); j++)
        {
            outfile << "Nearest neighbor-" << j + 1 << ": " << appr_results[j].first << "\n";
            outfile << "distanceHyperCube:" << appr_results[j].second << "\n";
            outfile << "distanceTrue:" << exact_results[j].second << "\n";
        }
        outfile << "tHyperCube:" << setprecision(5) << appr_knn_time << "\n";
        outfile << "tTrue:" << setprecision(5) << exact_knn_time << "\n";
        outfile << "R-near neighbors:\n";
        for (int j = 0; j < range_results.size(); j++)
        {
            outfile << range_results[j] << "\n";
        }
    }
    outfile.close();

    return 0;
}
