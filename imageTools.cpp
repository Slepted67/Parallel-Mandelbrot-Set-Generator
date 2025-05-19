#include <iostream>
#include <cmath>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

// Convert grayscale image to matrix
extern "C" int* imageToMat(char* filename, int* dims) {
    Mat image = imread(filename, IMREAD_COLOR);
    Mat gray_image;
    cvtColor(image, gray_image, COLOR_BGR2GRAY);

    if (!image.data) {
        cout << "Could not open or find the image" << endl;
        return nullptr;
    }

    int width = gray_image.size().width;
    int height = gray_image.size().height;
    dims[0] = height;
    dims[1] = width;

    int *matrix = (int*) malloc(height * width * sizeof(int));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int intensity = gray_image.at<uchar>(i, j);
            matrix[i * width + j] = std::clamp(intensity, 0, 254);
        }
    }

    return matrix;
}

// Save matrix as color image
extern "C" void matToImageColor(char* filename, int* mat, int* dims) {
    int height = dims[0];
    int width = dims[1];
    Mat image(height, width, CV_8UC3, Scalar(0, 0, 0));

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int r = 0, g = 0, b = 0;
            int c = mat[i * width + j] % 16;

            if (mat[i * width + j] < 255 && mat[i * width + j] > 0) {
                switch (c) {
                    case 0: r=66;  g=30;  b=15; break;
                    case 1: r=25;  g=7;   b=26; break;
                    case 2: r=9;   g=1;   b=47; break;
                    case 3: r=4;   g=4;   b=73; break;
                    case 4: r=0;   g=7;   b=100; break;
                    case 5: r=12;  g=44;  b=138; break;
                    case 6: r=24;  g=82;  b=177; break;
                    case 7: r=57;  g=125; b=209; break;
                    case 8: r=134; g=181; b=229; break;
                    case 9: r=211; g=236; b=248; break;
                    case 10: r=241; g=233; b=191; break;
                    case 11: r=248; g=201; b=95; break;
                    case 12: r=255; g=170; b=0; break;
                    case 13: r=204; g=128; b=0; break;
                    case 14: r=153; g=87;  b=0; break;
                    case 15: r=106; g=52;  b=3; break;
                }
            }

            image.at<Vec3b>(i, j)[0] = b;
            image.at<Vec3b>(i, j)[1] = g;
            image.at<Vec3b>(i, j)[2] = r;
        }
    }

    imwrite(filename, image);
}

// Save matrix as grayscale image
extern "C" void matToImage(char* filename, int* mat, int* dims) {
    int height = dims[0];
    int width = dims[1];
    Mat image(height, width, CV_8UC1, Scalar(0));

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image.at<uchar>(i, j) = (uchar) mat[i * width + j];
        }
    }

    imwrite(filename, image);
}
