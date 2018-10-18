#include <iostream>
#include <string>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>

using namespace cv;
using namespace std;

#define NOISE_MAX_AREA 500
//Uncomment this line at run-time to skip GUI rendering
#define _DEBUG

const string CAM_PATH="/dev/video0";

/*
* 获取小车当前位置
* -1：应该到终点了吧
* 0：应该还在路上
* 1：方向偏右，向左调整
* 2：方向偏左，向右调整
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
			fitLine(contours[i], lineData, DIST_L2, 0, 1e-2, 1e-2);
			lines.push_back(lineData);
		}
	
	image_vector->push_back(image);
	image_vector->push_back(binary_img);
	
	if (lines.size() == 0) {
		return -1; //没有直线——应该到终点了吧
	} else if (lines.size() > 2) {
		return 0; //直线过多——应该还在路上
	} else {
		//区分一下左和右
		return (lines[0][1] / lines[0][0]) > 0 ? 1 : 2; // 1-方向偏右，向左调整；2-方向偏左，向右调整
	}
	return 0;
}

int main() {
	VideoCapture capture(CAM_PATH);
	
	double dWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH);
	double dHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
	clog << "Frame Size: " << dWidth << "x" << dHeight << endl;

	Mat image;
	while(true) {
		capture >> image;
		if(image.empty())
			break;
		
		Rect roi(0, image.rows/2, image.cols, image.rows/4);
		Mat imgROI = image(roi);
		vector<Mat>* results = new vector<Mat>();
		
		int n = getPosition(imgROI, results);
		
		#ifdef _DEBUG
		cout << n << endl;
		for(int i = 0; i < results->size(); i++) {
			imshow("Image " + i, (*results)[i]);
		}
		#endif
		
		waitKey(1);
	}
	return 0;
}