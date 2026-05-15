#include <opencv2/opencv.hpp>
#include <vector>
#include <cstring>

extern "C"
{
    __declspec(dllexport)
        unsigned char* __cdecl UpscaleNearestNeighbour(
            unsigned char* input,
            int inputSize,
            int* outputSize
        )
    {
        try
        {
            // Convert bytes -> vector
            std::vector<uchar> data(input, input + inputSize);

            // Decode PNG
            cv::Mat img = cv::imdecode(data, cv::IMREAD_UNCHANGED);

            if (img.empty())
            {
                *outputSize = 0;
                return nullptr;
            }

            // TEST:
            // Return original image first
            cv::Mat output = img.clone();

            // Later replace with:
            // cv::Mat output =
            //     Interpolation_2D::NearestNeighbour(img);

            std::vector<uchar> encoded;

            bool success =
                cv::imencode(".png", output, encoded);

            if (!success)
            {
                *outputSize = 0;
                return nullptr;
            }

            unsigned char* result =
                new unsigned char[encoded.size()];

            memcpy(
                result,
                encoded.data(),
                encoded.size()
            );

            *outputSize =
                static_cast<int>(encoded.size());

            return result;
        }
        catch (...)
        {
            *outputSize = 0;
            return nullptr;
        }
    }

    __declspec(dllexport)
        void __cdecl FreeMemory(unsigned char* ptr)
    {
        delete[] ptr;
    }
}