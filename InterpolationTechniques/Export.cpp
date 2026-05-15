#include<opencv2/opencv.hpp>
#include"Interpolation_2D.h"

static cv::Mat output;

extern "C" {

    __declspec(dllexport)
        unsigned char* UpscaleNearestNeighbour(
            unsigned char* input,
            int width,
            int height
        ) {
        cv::Mat img(height, width, CV_8UC1, input);

        output = Interpolation_2D::NearestNeighbour(img);

        return output.data;

    }
}