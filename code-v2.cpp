#include <iostream>
#include <string>
#include <cassert>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include "GPIOlib.h"

using namespace cv;
using namespace std;
using namespace GPIO;

#define NOISE_MAX_AREA 500
#define SPEED 7
#define ANGLE 0
//Uncomment this line at run-time to skip GUI rendering
#define _DEBUG

const string CAM_PATH="/dev/video0";

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

/*
* 获取小车当前位置
* -1：应该到终点了吧
* 0：应该还在路上
* 1：方向有点偏右，向左调整一点
* 3：方向偏右，向左调整（未使用）
* 2：方向有点偏左，向右调整一点
* 4：方向偏左，向右调整
*/
int getPosition(Mat& image, vector<Mat>* image_vector) {
	Mat grey_img = image.clone();
	Mat binary_img = image.clone();
	int width = image.cols;
	int height = image.rows;
	
	//灰度化
	cvtColor(image, grey_img, COLOR_BGR2GRAY);     
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
	drawContours(binary_img, contours, -1, Scalar::all(255));
	
	vector<Vec4f> lines;
	double k_sum = 0.0;
	for (int i = 0; i < contours.size(); i++) {
		vector<Point> contours_poly;
		//计算凸包
		approxPolyDP(Mat(contours[i]), contours_poly, 3, true);
		//得到最小外包正矩形
		Rect r = boundingRect(Mat(contours_poly));
		//矩形过小视作噪点
		if (r.width * r.height > NOISE_MAX_AREA) {
			Vec4f lineData;
			//将轮廓拟合为直线
			fitLine(contours[i], lineData, DIST_L2, 0, 1e-1, 1e-1);
			k_sum += (double) lineData[1] / lineData[0];
			lines.push_back(lineData);
			vector<Point> line_point = getEdgePoint(width, height, lineData);
			line(image, line_point[0], line_point[1], Scalar(0, 0, 255), 3);
		}
	}
	image_vector->push_back(image);
	image_vector->push_back(binary_img);
	
	double k_avg = k_sum / lines.size();
	#ifdef _DEBUG
	cout << k_avg << endl;
	#endif
	if (lines.size() == 0) {
		return -1; //没有直线——应该到终点了吧
	} else if (lines.size() > 5) {
		return 0; //直线过多——应该还在路上
	} else {
		if (k_avg < 0.020) {
			return 4;
		} else if (k_avg < 0.034) {
			return 2;
		} else if (k_avg > 0.046) {
			return 1;
		} else return 0;
	}
	return 0;
}

void control(int pos, int lastState) {
	const int INTERVAL = 3000;
	switch(pos) {
		case -1:
			delay(INTERVAL);
			stopLeft();
			stopRight();
			break;
		case 0:
			switch(lastState){
				case 1:
					//turnTo(-2);
					break;
				case 2:
					turnTo(2);
					break;
				default:
					turnTo(0);
			}
			break;
		case 1:
			turnTo(-8);
			break;
		case 2:
			turnTo(8);
			break;
		case 4:
			turnTo(16);
			break;
		default:
			stopLeft();
			stopRight();
			assert(false);
	}
}

void control_v2(int n, int lastState) {
	const int INTERVAL = 3000;
	switch(n) {
		case -1:
			delay(INTERVAL);
			stopLeft();
			stopRight();
			break;
		case 0:
			controlLeft(FORWARD, SPEED);
			controlRight(FORWARD, SPEED);
			if (lastState == 2) {
				turnTo(ANGLE + 2);
			} else if (lastState == 4) {
				turnTo(ANGLE + 4);
			} else {
				turnTo(ANGLE)
			}
			break;
		case 1:
			controlRight(FORWARD, SPEED - 1);
			controlLeft(FORWARD, SPEED - 1);
			turnTo(ANGLE - 8);
			break;
		case 2:
			controlLeft(FORWARD, SPEED - 1);
			controlRight(FORWARD, SPEED - 1);
			turnTo(ANGLE + 8);
			break;
		case 4:
			controlLeft(FORWARD, SPEED - 2);
			controlRight(FORWARD, SPEED - 2);
			turnTo(ANGLE + 16);
			break;
		default:
			stopLeft();
			stopRight();
			assert(false);
	}
}

int main() {
	VideoCapture capture(CAM_PATH);
	
	double dWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH);
	double dHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
	clog << "Frame Size: " << dWidth << "x" << dHeight << endl;

	Mat image;
	init();
	turnTo(ANGLE);
	controlLeft(FORWARD, SPEED);
	controlRight(FORWARD, SPEED);
	int lastState = 0;
	while(true) {
		capture >> image;
		if(image.empty())
			break;
		
		Rect roi(0, image.rows/2, image.cols, image.rows/2);
		Mat imgROI = image(roi);
		vector<Mat>* results = new vector<Mat>();
		
		//int n = 0 测试语句，用于校正前轮
		int n = getPosition(imgROI, results);
		
		if (n != lastState) {
			control(n, lastState);
			lastState = n;
		}
		
		#ifdef _DEBUG
		for(int i = 0; i < results->size(); i++) {
			imshow("Image " + i, (*results)[i]);
		}
		#endif
		
		waitKey(1);
	}
	return 0;
}