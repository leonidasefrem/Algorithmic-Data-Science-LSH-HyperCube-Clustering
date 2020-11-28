#ifndef READER_H
#define READER_H

#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

int NumReverse(int i)
{
    int SwapedNum;
    unsigned char FirstByte, SecondByte, ThirdByte, FourthByte;
    FirstByte = i & 255;
    SecondByte = (i >> 8) & 255;
    ThirdByte = (i >> 16) & 255;
    FourthByte = (i >> 24) & 255;
    SwapedNum = ((int)FirstByte << 24) + ((int)SecondByte << 16) + ((int)ThirdByte << 8) + FourthByte;
    return SwapedNum;
}

//Read the MNIST data and load the dataset to data_vector
int ReadData(vector<vector<double>> &data_vector, char *data_path)
{
    ifstream file(data_path, ios::binary);
    if (file.is_open())
    {
        int number_of_rows = 0;
        int number_of_cols = 0;
        int number_of_images = 0;
        int magic_num = 0;

        //READ THE METADATA
        file.read((char *)&magic_num, sizeof(magic_num));
        magic_num = NumReverse(magic_num);
        file.read((char *)&number_of_images, sizeof(number_of_images));
        number_of_images = NumReverse(number_of_images);
        file.read((char *)&number_of_rows, sizeof(number_of_rows));
        number_of_rows = NumReverse(number_of_rows);
        file.read((char *)&number_of_cols, sizeof(number_of_cols));
        number_of_cols = NumReverse(number_of_cols);

        //cout << magic_num << endl;
        //cout << number_of_images << endl;
        //cout << n_rows << endl;
        //cout << n_cols << endl;

        data_vector.resize(number_of_images, vector<double>(number_of_rows * number_of_cols)); //Resize the vector to fit the data
        //READ THE PIXEL DATA
        unsigned char pixel = 0;
        for (int i = 0; i < number_of_images; ++i)
        {
            for (int r = 0; r < number_of_rows; ++r)
            {
                for (int c = 0; c < number_of_cols; ++c)
                {
                    pixel = 0;
                    file.read((char *)&pixel, sizeof(pixel));
                    data_vector[i][(number_of_rows * r) + c] = (double)pixel;
                }
            }
        }
        return 0;
    }
    else
    {
        cout << "Something went wrond at opening the file\n";
        return -1;
    }
}

#endif