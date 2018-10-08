#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui.hpp>
using namespace cv;

#include <iostream>
using namespace std;

int main() {
    Mat image = cv::imread("test1.jpg", 0);
	Mat result = image.clone();
	if (image.empty()) { 
		printf("no image exist"); 
		return 0;       
	}   
	threshold(image, result, 60, 255, CV_THRESH_BINARY);
	namedWindow("Original Image");      
	imshow("Original Image", result);       
	waitKey(0);       
	return 0;
}