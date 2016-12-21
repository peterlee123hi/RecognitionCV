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

vector<Point> handtracking::getApproxConvexHull(vector<Point> contour, int clusterDistance) {
    vector<Point> hull;
    convexHull(contour, hull, 0, 1);
    return cluster(hull, clusterDistance);
}

// Private Functions
vector<Point> handtracking::cluster(vector<Point> contour, int minDist) {
    Point centroid = getCentroid(contour);
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

        Point farPoint;
        double maxDist = -1;
        for (int i = 0; i < cluster.size(); i++) {
            if (maxDist < distance(centroid, cluster[i])) {
                farPoint = cluster[i];
                maxDist = distance(centroid, cluster[i]);
            }
        }
        clustered.push_back(farPoint);
    }
    return clustered;
}

double handtracking::distance(Point a, Point b) {
    return norm(a - b);
}

