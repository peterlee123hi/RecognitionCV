#include "handtracking.hpp"

// Public Functions
int handtracking::numberOfFingers(vector<Point> contour) {
    return 0;
}

bool handtracking::isHand(vector<Point> contour) {
    return true;
}

Point handtracking::getCentroid(vector<Point> contour) {
    Moments M = moments(contour, false);
    Point center((int) M.m10 / M.m00, (int) M.m01 / M.m00);
    return center;
}

vector<Point> handtracking::getDefects(vector<Point> contour, int minDist) {
    return vector<Point>();
}

vector<Point> handtracking::getFingertips(vector<Point> contour) {
    return vector<Point>();
}

vector<Point> handtracking::getApproxConvexHull(vector<Point> contour) {
    vector<Point> hull;
    convexHull(contour, hull, 0, 1);
    int clusterDistance = 40;
    return cluster(hull, clusterDistance);
}

// Private Functions
vector<Point> handtracking::cluster(vector<Point> contour, int minDist) {
    vector<Point> clustered;
    int i = 0;
    while (i < contour.size()) {
        vector<Point> cluster;
        Point clusterPoint = contour[i++];
        cluster.push_back(clusterPoint);
        while (i < contour.size()) {
            clusterPoint = contour[i];
            Point recent = cluster.back();
            if (distance(clusterPoint, recent) < minDist) {
                cluster.push_back(clusterPoint);
                i++;
            } else {
                break;
            }
        }
        clustered.push_back(median(cluster));
    }
    return clustered;
}

Point handtracking::median(vector<Point> points) {
    int median = points.size() / 2;
    if (points.size() % 2 == 0 && points.size() > 0)
        return points[median - 1];
    else
        return points[median];
}

double handtracking::distance(Point a, Point b) {
    return norm(a - b);
}

