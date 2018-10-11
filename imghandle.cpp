#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
using namespace cv;

#include <iostream>
#include <math.h>
#include <string>
using namespace std;

#define PI 3.1415926
#define NOISE_MAX_AREA 5000

Vec2i getPosition(Mat& origin_image);

//获取直线在图像大小顶部/底部的截点，存入vector数组
vector<Point> getEdgePoint(int w, int h, Vec4f& line) {
	double k = line[1] / line[0];
	Point p1, p2;
	p1.x = line[2] - line[3] / k;
	p1.y = 0;
	p2.x = line[2] + (h - line[3]) / k;
	p2.y = h;
	vector<Point> result;
	result.push_back(p1);
	result.push_back(p2);
	return result;
}

//用于对所有直线截点进行排序，判断出最接近中间位置的两条
int cmp_func(vector<Point>& a, vector<Point>& b) {
    return a[1].x < b[1].x;
}

Point trans_point(double x, double y, Mat& trans) {
	Mat B= Mat::ones(3, 1, 6);
	B.at<double>(0,0)=x;
	B.at<double>(1,0)=y;
	B.at<double>(2,0)=1;
	Mat trans_mat = trans * B;
	Point p;
	p.x = trans_mat.at<double>(0,0) / trans_mat.at<double>(2,0);
	p.y = trans_mat.at<double>(1,0) / trans_mat.at<double>(2,0);
	return p;
}

//返回Vec2i，第一项代表相对于正前方偏角角度，第二项代表相对左侧边缘位置（0-100）
Vec2i getPosition(Mat& origin_image) {
	Mat grey_img = origin_image.clone();
	Mat binary_img = origin_image.clone();
	int width = origin_image.cols;
	int height = origin_image.rows;
	
	//灰度化
	cvtColor(origin_image, grey_img, COLOR_BGR2GRAY);     
	//OTSU二值化
	threshold(grey_img, binary_img, 170, 255, CV_THRESH_OTSU|CV_THRESH_BINARY);
	
	//中值滤波
	for (int i = 1; i < 20; i += 4) {
		medianBlur(binary_img, binary_img, i);
	}
	//Canny方法边缘检测
	Canny(binary_img, binary_img, 3, 9, 3);
	vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
	//轮廓提取
    findContours(binary_img, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	
	vector<vector<Point>> line_points; //记录直线在图像大小顶部/底部的截点
	for (int i = 0; i < contours.size(); i++) {
		vector<Point> contours_poly;
		//drawContours(binary_img, contours, i, Scalar::all(255));
		//计算凸包
		approxPolyDP(Mat(contours[i]), contours_poly, 3, true);
		//得到最小外包正矩形
		Rect r = boundingRect(Mat(contours_poly));
		//矩形过小视作噪点
		if (r.width * r.height > NOISE_MAX_AREA) {
			Vec4f lineData;
			//将轮廓拟合为直线
			fitLine(contours[i], lineData, DIST_L2, 0, 1e-2, 1e-2);
			vector<Point> line_point = getEdgePoint(width, height, lineData);
			line_points.push_back(line_point);
			//line(binary_img, line_point[0], line_point[1], Scalar(0, 0, 255), 5);
		}
	}
	
	//寻找最近的两条直线
	sort(line_points.begin(), line_points.end(), cmp_func);
	for (int i = 0; i < line_points.size(); i++) {
		if (line_points[i][1].x > (width / 2)) {
			//line(origin_image, line_points[i][0], line_points[i][1], Scalar(0, 255, 0), 5);
			//line(origin_image, line_points[i-1][0], line_points[i-1][1], Scalar(0, 255, 0), 5);
			//line(origin_image, Point2f(width/2, height), Point2f(width/2, 0), Scalar(0, 255, 0), 5);
			Mat dst;
			Point2f AffinePoints0[4] = {line_points[i-1][0], line_points[i][0], line_points[i-1][1], line_points[i][1]};
			Point2f AffinePoints1[4] = {Point2f(50, 0), Point2f(250, 0), Point2f(50, 300), Point2f(250, 300)};
			Mat trans = getPerspectiveTransform(AffinePoints0, AffinePoints1);
			//vector<Vec2i> origin_points = {Vec2i(width/2, height), Vec2i(width/2, 0)};
			Point p1 = trans_point(width/2, height, trans);
			Point p2 = trans_point(width/2, 0, trans);
			
			cout << p1 << endl;
			cout << p2 << endl;
			//perspectiveTransform(origin_points, trans_points, trans);
			//warpPerspective(origin_image, dst, trans, Size(350, 400), CV_INTER_CUBIC);
			//namedWindow("src");
			//imshow("src", dst);
			Vec2i result;
			double k = float(p2.x - p1.x) / (p2.y - p1.y);
			result[0] = -int(atan(k) / PI * 180.0);
			result[1] = int((p1.x - 50) / 2);
			cout << result[0] << ' ' << result[1] << endl;
			return result;
		}
	}
	return Vec2i(0, -1);
}

int main() {
	string fileName = "test_pic/test2.jpg";
	Mat origin_image = imread(fileName);
	
	
	Vec2i v = getPosition(origin_image);
	
	return 0;
}