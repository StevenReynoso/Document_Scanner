#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

Mat imgOriginal, imgGray, imgBlur, imgDil, imgErode, imgCanny, imgThresh, imgWarp, imgCrop;
vector<Point> initialPoints, docPoints;

float w = 420, h = 596;

Mat preProcessing(Mat img) {
	
	cvtColor(img, imgGray, COLOR_BGR2GRAY);
	GaussianBlur(imgGray, imgBlur, Size(3, 3), 3, 0);
	Canny(imgBlur, imgCanny, 25, 75);

	Mat Kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(imgCanny, imgDil, Kernel);
	//erode(imgDil, imgErode, Kernel);

	return imgDil;
}

vector<Point> getContours(Mat image) {

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(image, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	vector<vector<Point>> conPoly(contours.size());
	vector<Rect> boundRect(contours.size());

	vector<Point> biggest;
	int maxArea = 0;

	for (int i = 0; i < contours.size(); i++) {

		int area = contourArea(contours[i]);
		cout << area << endl;

		string objectType;
		
		if (area > 1000) {
			float peri = arcLength(contours[i], true);
			approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);				//// finding points

			if (area > maxArea && conPoly[i].size() == 4) {
				//drawContours(imgOriginal, conPoly, i, Scalar(255, 0, 255), 5);
				biggest = { conPoly[i][0], conPoly[i][1] , conPoly[i][2], conPoly[i][3] };
				maxArea = area;
			}

			//drawContours(imgOriginal, conPoly, i, Scalar(255, 0, 255), 2);
			//rectangle(imgOriginal, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 255, 0), 5);		/// will show the drawing of the rectangle if, u wish to see
		}
	}
	return biggest;
}


void drawPoints(vector<Point> points, Scalar color) {

	for (int i = 0; i < points.size(); i++) {
		
		circle(imgOriginal, points[i], 15, color, FILLED);
		putText(imgOriginal, to_string(i), points[i], FONT_HERSHEY_PLAIN, 4, color, 4);
	}
}

vector<Point> reOrder(vector<Point> points ) {
	vector<Point> newPoints;
	vector<int> sumPoints, subPoints;

	for (int i = 0; i < 4; i++) {
		sumPoints.push_back(points[i].x + points[i].y);
		subPoints.push_back(points[i].x - points[i].y);
	}
	newPoints.push_back(points[min_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]); // index 0
	newPoints.push_back(points[max_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]); // index 1
	newPoints.push_back(points[min_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]); // index 2
	newPoints.push_back(points[max_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]); // index 3

	return newPoints;				/// will be reordered now on the original image
	
}

Mat getWarp(Mat img, vector<Point> points, float w, float h) {	// imgOriginal and docPoints, width, height;

	Point2f src[4] = { points[0], points[1], points[2],points[3]};
	Point2f dst[4] = { {0.0f, 0.0f}, {w, 0.0f}, {0.0f, h}, {w, h} };

	Mat matrix = getPerspectiveTransform(src, dst);
	warpPerspective(img, imgWarp, matrix, Point(w, h));					/// undo for images
	

	return imgWarp;

}			

/////////////////// image scanner /////////////

int main() {
	
	string path = "Resources/paper.jpg";		//// changing the image will scan your document
	imgOriginal = imread(path);


	resize(imgOriginal, imgOriginal, Size(), 0.5, 0.5);

	//preprocessing
	imgThresh = preProcessing(imgOriginal);

	//Get Contours - Biggest
	//getContours(imgThresh);
	initialPoints = getContours(imgThresh);
	//drawPoints(initialPoints, Scalar(0, 0, 255));		// send in red, for correct one we send in green.

	/// Re-order
	docPoints = reOrder(initialPoints);
	//drawPoints(docPoints, Scalar(0, 255, 0));		// correct points reordered

	//Warp
	imgWarp = getWarp(imgOriginal, docPoints, w, h);

	// image Crop
	int cropVal = 10;
	Rect roi(cropVal, cropVal, w - (2 * cropVal), h - (2 * cropVal));
	imgCrop = imgWarp(roi);

	imshow("Image Original", imgOriginal);
	imshow("Image Dilation", imgThresh);
	imshow("Image Warp", imgWarp);
	imshow("Image Crop", imgCrop);
	waitKey(0);

	return 0;
}