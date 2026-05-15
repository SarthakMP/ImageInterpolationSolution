#include<vector>
#include"opencv2/opencv.hpp"
#define M_PI 3.14159265358979323846

#pragma once
class Interpolation_1D
{
public:
	Interpolation_1D() {}
	/*
	double CubicInterp(double p[4], double x);
	double BellCurve(double in);
	std::vector<cv::Point2d> NearestInterp(std::vector< cv::Point2d>&Points, int _NPoints);
	std::vector<cv::Point2d> LinearInterp(std::vector< cv::Point2d>&Points, int _NPoints);
	std::vector<cv::Point2d> LinearInterp(std::vector< cv::Point2d>&Points, int _NPoints);*/

	//Helper Function
	double CubicInterp(double p[4], double x) {

		double a = -0.5 * p[0] + 1.5 * p[1] - 1.5 * p[2] + 0.5 * p[3];
		double b = p[0] - 2.5 * p[1] + 2 * p[2] - 0.5 * p[3];
		double c = -0.5 * p[0] + 0.5 * p[2];
		double d = p[1];

		return (((a * x + b) * x + c) * x + d);
	}

	double BellCurve(double in) {
		return std::exp(-(in * in) * 0.001);
	}


#pragma region Nearest_interpolation

	std::vector<cv::Point2d> NearestInterp(std::vector< cv::Point2d>& Points, int _NPoints) {
		std::vector<cv::Point2d > New_Points;
		New_Points.reserve(_NPoints * 2);
		for (int i = 1; i < _NPoints; i++) {
			cv::Point2d Point1(Points[i - 1]), Point2(Points[i]);

			New_Points.push_back(Point1);

			double half = (Point1.x + Point2.x) / 2;
			cv::Point2d mid1(half, Point1.y), mid2(half, Point2.y);
			New_Points.push_back(mid1);
			New_Points.push_back(mid2);

			New_Points.push_back(Point2);
		}

		return New_Points;
	}

#pragma endregion

#pragma region Linear_interpolation

	std::vector<cv::Point2d> LinearInterp(std::vector< cv::Point2d>& Points, int _NPoints) {
		std::vector<cv::Point2d > New_Points;
		New_Points.reserve(_NPoints * 4);
		for (int i = 1; i < _NPoints; ++i) {
			cv::Point2d Point1(Points[i - 1]), Point2(Points[i]);

			New_Points.push_back(Point1);

			for (double j = Points[i - 1].x; j < Points[i].x; j += 0.25) {

				double y = Point1.y + (Point2.y - Point1.y) * (j - Point1.x) / (Point2.x - Point1.x);
				cv::Point2d tmp_points(j, y);
				New_Points.push_back(tmp_points);

			}
			New_Points.push_back(Point2);

		}
		return New_Points;
	}



#pragma endregion

#pragma region Cubic_interpolation

	std::vector<cv::Point2d> CubicInterp(std::vector< cv::Point2d>& Points, int _NPoints) {
		std::vector<cv::Point2d> new_points;
		new_points.reserve(Points.size() * 8);
		new_points.emplace_back(Points[0].x, Points[0].y);

		for (int i = 1; i < Points.size() - 2; i++) {

			double x0 = Points[i - 1].x;
			double x1 = Points[i].x;
			double x2 = Points[i + 1].x;
			double x3 = Points[i + 2].x;

			double y0 = Points[i - 1].y;
			double y1 = Points[i].y;
			double y2 = Points[i + 1].y;
			double y3 = Points[i + 2].y;

			new_points.emplace_back(Points[i].x, Points[i].y);
			double fx = x2 - x1;
			if (fx <= 0) continue;

			for (double t = 0; t < 1.0; t += 0.2) {

				double ex_y[4] = { y0, y1,y2 ,y3 };

				double interp_y = CubicInterp(ex_y, t);

				double interp_x = x1 + t * fx;
				new_points.emplace_back(interp_x, interp_y);

			}


		}

		if (!Points.empty()) {
			new_points.emplace_back(Points.back().x, Points.back().y);
		}

		return new_points;
	}


#pragma endregion

#pragma region Lanczos_Interpolation
	inline static  double sinc(double t) {
		if (std::abs(t) < 1e-8) return 1.0;
		return std::sin(t) / t;
	}

	inline static double LanczosFunc(double x, double a) {
		if (std::abs(x) >= a) return 0.0;
		return sinc(x * M_PI) * sinc(x * M_PI / a);
	}

	/*
		Lanczos function

		where	sinc(x) => sin(x*pi)/(x*pi)
						OR
				sinc(t) => sin(t)/t

		f(x) = sinc(x)*sinc(x/a) ; {-a<x<a}
				0				 ; other
	*/


	void LanczosInterp(std::vector< cv::Point2d > points, std::vector<cv::Point2d>& new_points, double a) {

		std::vector<double> extended_x;

		//Generate Points between
		int extended_xSize = 0;

		extended_x.reserve(points.size() * static_cast<int>(a));
		for (size_t i = 0; i < points.size() - 1; i++) {
			double x_min = points[i].x;
			double x_max = points[i + 1].x;

			extended_x.push_back(x_min);

			for (double t = 0.5; t < 1; t += 0.5) {
				double x_between = x_min + (x_max - x_min) * t;
				extended_x.push_back(x_between);
				extended_xSize++;
			}

		}
		extended_x.push_back(points.back().x);


		double spacing = points[1].x - points[0].x;
		//Interpolation
		for (auto x : extended_x) {

			double sum = 0.0, weight_sum = 0.0;

			double x_index = (x - points[0].x) / spacing;

			int start = std::ceil(x_index - a);
			int end = std::floor(x_index + a);

			start = std::max(start, 0);
			end = std::min(end, (int)points.size() - 1);


			for (int i = start; i <= end; i++) {

				double w = LanczosFunc(x_index - i, a);
				sum += points[i].y * w;
				weight_sum += w;

				//std::cout << w << std::endl;

			}

			double interpolated_y = std::abs(sum) < 1e-12 ? 0 : sum / weight_sum;
			new_points.push_back({ x,interpolated_y });

		}

	}





#pragma endregion


};

