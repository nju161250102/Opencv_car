#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>

using namespace cv;

#include <iostream>
#include <string>
using namespace std;

#define NOISE_MAX_AREA 50000

int main() {
	string fileName = "test_pic/test1.jpg";
	Mat origin_image = imread(fileName);
    Mat image = imread(fileName, 0);
	Mat grey_img = image.clone();
	Mat result = image.clone();
	if (image.empty()) { 
		printf("no image exist"); 
		return 0;       
	}
	
	threshold(image, grey_img, 170, 255, CV_THRESH_OTSU|CV_THRESH_BINARY);
	for (int i = 1; i < 20; i += 4) { 
		medianBlur(grey_img, grey_img, i);        
	}
	
	Canny(grey_img, grey_img, 3, 9, 3);
	vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(grey_img, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
	
	for (int i = 0; i < contours.size(); i++) {
		vector<Point> contours_poly;
		//drawContours(result, contours, i, Scalar::all(255));
		approxPolyDP(Mat(contours[i]), contours_poly, 3, true);
		Rect r = boundingRect(Mat(contours_poly));
		if (r.width * r.height > NOISE_MAX_AREA) {
			Vec4f lineData;
			fitLine(contours[i], lineData, DIST_L2, 0, 1e-2, 1e-2);
			
			int lefty = (-lineData[2]*lineData[1]/lineData[0])+lineData[3]; 
			int righty = ((result.cols-lineData[2])*lineData[1]/lineData[0])+lineData[3]; 
			line(origin_image, Point(result.cols-1, righty), Point(0, lefty), Scalar(0, 0, 255), 5);

			//line(result, point1, point2, Scalar(0, 255, 0), 1);
		}
		
	}
	
    
	namedWindow("Show Image");      
	imshow("Show Image", origin_image);       
	waitKey(0);       
	return 0;
}