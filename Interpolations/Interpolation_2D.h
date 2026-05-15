#include<vector>
#include<iostream>
#include"Interpolation_1D.h"
#include"opencv2/opencv.hpp"
#define M_PI 3.14159265358979323846

#pragma once
class Interpolation_2D
{
	Interpolation_1D interp_1d;
public:
	//Helper Functions
	double Interpolation(uchar p1, uchar p2, double t) {
		return static_cast<double>(p1 + (p2 - p1) * t);
	}

	template<typename T>
	T m_min(T a, T b) {
		return a < b ? a : b;
	}

	template<typename T>
	T m_max(T a, T b) {
		return a > b ? a : b;
	}

	std::vector< std::vector <double>> CalculateBetweenPixels(cv::Mat Image, int new_size) { //x2 sampling
		size_t _orgHeight = Image.rows, _orgWidth = Image.cols;

		std::vector< std::vector <double>> IntermediatePixels;
		IntermediatePixels.resize(_orgHeight * 2);

		for (size_t n = 0; n < 2 * _orgHeight; n++) {
			IntermediatePixels[n].resize(_orgWidth * 2);
		}


		for (size_t y = 0; y < _orgHeight; y++)
		{

			double* row_ptr = Image.ptr<double>(y);

			std::vector<double>& new_row = IntermediatePixels[2 * y];

			for (size_t x = 0; x < _orgWidth - 1; x++) {

				double x_min = row_ptr[x];
				double x_max = row_ptr[x + 1];

				new_row[x] = x_min;

				for (double t = 0.5; t < 1; t += 0.5) {
					double x_mid = (x_min)+(x_max - x_min) * t;
					new_row[x] = x_mid;
				}
			}

		}

		std::cout << "End of X Start of Y" << std::endl;

		for (size_t y = 1; y < _orgHeight; y++) {
			double* cur_col_ptr = Image.ptr<double>(y - 1);
			double* nxt_col_ptr = Image.ptr<double>(y);

			std::vector<double>& new_row = IntermediatePixels[2 * y - 1];

			for (size_t x = 0; x < IntermediatePixels.size(); x++) {

				double y_min = cur_col_ptr[x];
				double y_max = nxt_col_ptr[x];

				for (double t = 0.5; t < 1; t += 0.5) {
					double y_mid = (y_min)+(y_max - y_min) * t;
					new_row[x] = y_mid;
				}

			}

		}

		std::cout << "new \n Height: " << IntermediatePixels.size() << " \n Width: " << IntermediatePixels[0].size();

		return IntermediatePixels;
	}

#pragma region NearestNeighbour 2d
	static cv::Mat NearestNeighbour(cv::Mat in_Image_Gray) {
		int old_h = in_Image_Gray.rows, old_w = in_Image_Gray.cols;

		cv::Mat New_image = cv::Mat::zeros(2 * old_h, 2 * old_w, CV_8UC1);

		for (int r = 0; r < old_h; r++) {
			uchar* curr_row = in_Image_Gray.ptr<uchar>(r);


			for (int c = 0; c < old_w; c++) {

				int new_r = r * 2;
				int new_c = c * 2;


				New_image.at<uchar>(new_r, new_c) = curr_row[c];
				New_image.at<uchar>(new_r, new_c + 1) = curr_row[c];

				New_image.at<uchar>(new_r + 1, new_c) = curr_row[c];
				New_image.at<uchar>(new_r + 1, new_c + 1) = curr_row[c];
			}
		}


		return New_image;

	}


#pragma endregion

#pragma region Bilinear

	cv::Mat BilinearInterp(cv::Mat in_Image_Gray) {
		int h_old = in_Image_Gray.rows, w_old = in_Image_Gray.cols;

		double t = 0.5;
		cv::Mat BilinearUpScaled = cv::Mat::zeros(2 * h_old, 2 * w_old, CV_8UC1);

		int upscaled_height = BilinearUpScaled.rows, upscaled_width = BilinearUpScaled.cols;

		for (int i = 0; i < h_old - 1; i++) {

			uchar* row0 = in_Image_Gray.ptr<uchar>(i);
			uchar* row1 = in_Image_Gray.ptr<uchar>(i + 1);

			uchar* up_row0 = BilinearUpScaled.ptr<uchar>(2 * i);
			uchar* up_midrow = BilinearUpScaled.ptr<uchar>(2 * i + 1);

			for (int j = 0; j < w_old - 1; j++) {

				uchar p1 = row0[j];
				uchar p2 = row0[j + 1];

				uchar p3 = row1[j];
				uchar p4 = row1[j + 1];

				uchar top = Interpolation(p1, p2, t);
				uchar bottom = Interpolation(p3, p4, t);

				uchar mid = Interpolation(top, bottom, t);

				uchar left = Interpolation(p1, p3, t);
				uchar right = Interpolation(p2, p4, t);

				up_row0[2 * j] = p1;
				up_row0[2 * j + 1] = top;
				up_row0[2 * j + 2] = p2;

				up_midrow[2 * j] = left;
				up_midrow[2 * j + 1] = mid;
				up_midrow[2 * j + 2] = right;

			}

		}

		return in_Image_Gray;
	}

#pragma endregion

#pragma region Bicubic


	cv::Mat BiCubicInterp(cv::Mat in_Image_Gray) {


		int h_old = in_Image_Gray.rows, w_old = in_Image_Gray.cols;

		cv::Mat new_Img = cv::Mat::zeros(h_old * 2, w_old * 2, CV_8UC1);

		int h_new = h_old * 2, w_new = w_old * 2;



		for (int dst_y = 0; dst_y < h_new; dst_y++) {
			uchar* new_img_row_ptr = new_Img.ptr<uchar>(dst_y);
			for (int dst_x = 0; dst_x < w_new; dst_x++) {

				// Mapping the destination pixel pos to teh source
				// as the dst image is bigger than the src
				// we calculate the offset using the following for x and y
				// these are in decimals ie it just tells the offset from the current pixel from the src to dest new pixel
				double src_x = (dst_x + 0.5) * (static_cast<double>(w_old) / w_new) - 0.5;
				double src_y = (dst_y + 0.5) * (static_cast<double>(h_old) / h_new) - 0.5;

				// we get the actual pixel where the offsets lands
				int x0 = static_cast<int>(std::floor(src_x)) - 1;
				int y0 = static_cast<int>(std::floor(src_y)) - 1;

				// calculate the actual distance between the actual pixel and the offset value
				double fx = src_x - std::floor(src_x);
				double fy = src_y - std::floor(src_y);

				double tmp[4] = { 0.0 };

				// We loop through a 4x4 grid of pixels and do the bicubic interpolation
				for (int r = 0; r < 4; r++) {
					double p[4];
					int py = std::clamp(r + y0, 0, h_old - 1); //handels the edge/border case the edge pixels are clammped from 0 to the old height of the src
					uchar* row_ptr = in_Image_Gray.ptr<uchar>(py);

					for (int c = 0; c < 4; c++) {
						int px = std::clamp(c + x0, 0, w_old - 1); //same clampping here

						p[c] = row_ptr[px];

					}
					tmp[r] = interp_1d.CubicInterp(p, fx); //once we get the 4 pixels we horizontally interpolate it using Cubic interpolation


				}

				double final_val = interp_1d.CubicInterp(tmp, fy); // once we get all the horizontal interpolation value we do vertical interpolation
				new_img_row_ptr[dst_x] = cv::saturate_cast<uchar>(final_val); // save the interpolated grid in the new image

			}

		}

		return new_Img;
	}

#pragma endregion

#pragma region Lanczos Interpolation

	cv::Mat Lanczos_Interpolation_2d(cv::Mat m_Image, double a,double xTimes) {

		cv::Mat img_double;
		m_Image.convertTo(img_double, CV_64F, 1.0 / 255.0);
		int _oldHeight = img_double.rows, _oldWidth = img_double.cols;

		int _newHeight = _oldHeight* xTimes, _newWidth = _oldWidth* xTimes;
		
		cv::Mat new_image = cv::Mat::zeros(_newHeight, _newWidth, CV_64F);


		//Interpolation

		for (size_t y = 0; y < _newHeight; y++) {

			//mapping the y index
			int y_index = (y+0.5) * ( _oldHeight / (double)_newHeight) -0.5; //Centered the pixel

			double start_y = std::floor(y_index - a);	//Getting the -a neighbour pixel from the current pixel on Y axis
			double end_y = std::ceil(y_index + a);	//Getting the +a neighbour pixel from the current pixel on Y axis
			start_y = m_max(start_y, 0.0);	//Clamping if the pixel lands outside the org image to zero
			end_y = m_min(end_y, (double)_oldHeight - 1);	//Clamping if the pixel lands outside the org image to org height

			double* new_row_ptr = new_image.ptr<double>(y);
			for (size_t x = 0; x < _newWidth; x++)
			{
				double sum = 0.0, w_sum = 0.0;

				//mapping the x index
				int x_index = (x + 0.5) * ( _oldWidth / (double)_newWidth) - 0.5; //Centered the pixel

				double start_x = std::floor(x_index - a);	//Getting the -a neighbour pixel from the current pixel on X axis
				double end_x = std::ceil(x_index + a);	//Getting the +a neighbour pixel from the current pixel on X axis
				start_x = m_max(start_x, 0.0);	//Clamping if the pixel lands outside the org image to zero
				end_x = m_min(end_x, (double)_oldWidth - 1);	//Clamping if the pixel lands outside the org image to org width

				//performing lanczos interpolation and multiplying the x and y weights
				for (int i = start_y; i <= end_y; i++) {
					double wy = interp_1d.LanczosFunc(y_index - i, a); //calculated the near by weight of y
					
					for (int j = start_x; j <= end_x; j++) {

						double wx = interp_1d.LanczosFunc(x_index - j, a);//calculated the near by weight of x

						double w = wx * wy; // multiplying both the weight to get the true weight of that pixel
						sum += img_double.at<double>(i,j) * w;
						w_sum+= w;
					}
				}
				
				new_row_ptr[x] = sum/w_sum; // final normalized weight of that pixel


			}

		}


		return new_image;
	}


#pragma endregion


};

