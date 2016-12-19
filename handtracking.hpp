#ifndef __HANDTRACKING_H_INCLUDED__
#define __HANDTRACKING_H_INCLUDED__

// C++ Libraries
#include <cmath>
#include <algorithm>
#include <vector>

// OpenCV Libraries
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

class handtracking {
public:
    int numberOfFingers(vector<Point> contour);
    bool isHand(vector<Point> contour);
    Point getCentroid(vector<Point> contour);
    vector<Point> getDefects(vector<Point> contour, int minDist);
    vector<Point> getFingertips(vector<Point> contour);

private:
    vector<Point> getApproxConvexHull(vector<Point> contour);
    vector<Point> cluster(vector<Point> contour, int minDist); 
    float distance(Point a, Point b);
};

#endif
