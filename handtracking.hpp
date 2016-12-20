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
    static int numberOfFingers(vector<Point> contour);
    static bool isHand(vector<Point> contour);
    static Point getCentroid(vector<Point> contour);
    static vector<Point> getDefects(vector<Point> contour, int minDist);
    static vector<Point> getFingertips(vector<Point> contour);
    static vector<Point> getApproxConvexHull(vector<Point> contour);

private:
    static vector<Point> cluster(vector<Point> contour, int minDist); 
    static Point median(vector<Point> points);
    static double distance(Point a, Point b);
    static const int CLUSTER_DISTANCE = 50;
};

#endif
