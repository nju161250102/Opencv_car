#include "imghandle.hpp"

#include <string>

int main() {
	string fileName = "test_pic/test1.jpg";
	Mat origin_image = imread(fileName);
	namedWindow("Show Image");
	imshow("Show Image", origin_image);
	//cvtColor(origin_image, origin_image, CV_BGR2GRAY); 
	Vec2i v = getPosition(origin_image);
	cout << v[0] << endl;
	/*
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
    findContours(grey_img, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	
	vector<vector<Point>> line_points;
	for (int i = 0; i < contours.size(); i++) {
		vector<Point> contours_poly;
		drawContours(result, contours, i, Scalar::all(255));
		approxPolyDP(Mat(contours[i]), contours_poly, 3, true);
		Rect r = boundingRect(Mat(contours_poly));
		if (r.width * r.height > NOISE_MAX_AREA) {
			Vec4f lineData;
			fitLine(contours[i], lineData, DIST_L2, 0, 1e-2, 1e-2);
			vector<Point> line_point = getEdgePoint(WIDTH, HEIGHT, lineData);
			line_points.push_back(line_point);
			line(origin_image, line_point[0], line_point[1], Scalar(0, 0, 255), 5);
		}
	}
	
	//寻找最近的两条直线
	sort(line_points.begin(), line_points.end(), cmp_func);
	for (int i = 0; i < line_points.size(); i++) {
		if (line_points[i][1].x > (WIDTH / 2)) {
			line(origin_image, line_points[i][0], line_points[i][1], Scalar(0, 255, 0), 5);
			line(origin_image, line_points[i-1][0], line_points[i-1][1], Scalar(0, 255, 0), 5);
			line(origin_image, Point2f(WIDTH/2, HEIGHT), Point2f(WIDTH/2, 0), Scalar(0, 255, 0), 5);
			Mat dst;
			Point2f AffinePoints0[4] = {line_points[i-1][0], line_points[i][0], line_points[i-1][1], line_points[i][1]};
			Point2f AffinePoints1[4] = {Point2f(50, 0), Point2f(250, 0), Point2f(50, 300), Point2f(250, 300)};
			Mat Trans = getPerspectiveTransform(AffinePoints0, AffinePoints1);
			warpPerspective(origin_image, dst, Trans, Size(350, 400), CV_INTER_CUBIC);
			namedWindow("src");
			imshow("src", dst);
			break;
		}
	}
    
	namedWindow("Show Image");      
	imshow("Show Image", origin_image);  */
	return 0;
}