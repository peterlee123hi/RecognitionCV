#include "handtracking.hpp"

// Public Functions
bool handtracking::isHand(vector<Point> contour) {
    return true;
}

Point handtracking::getCentroid(vector<Point> contour) {
    Moments M = moments(contour, false);
    Point center((int) M.m10 / M.m00, (int) M.m01 / M.m00);
    return center;
}

vector<Point> handtracking::getDefects(vector<Point> contour) {
    vector<Vec4i> defects;
    vector<Point> hull;
    vector<int> indexHull;

    convexHull(contour, hull, 0, 1);
    convexHull(contour, indexHull, 0, 0);
    if (indexHull.size() < 3)
        return vector<Point>();
    convexityDefects(contour, indexHull, defects);

    vector<Point> defectPoints;
    Point centroid = getCentroid(contour);
    for (int i = 0; i < defects.size(); i++) {
        Vec4i defect = defects[i];
        Point start = contour[defect[0]];
        Point end = contour[defect[1]];
        Point far = contour[defect[2]];
        int depth = defect[3];
        int height = distance(far, centroid);
        Point v1 = start - far;
        Point v2 = end - far;
        if (PI * 1 / 8 <= angle(v1, v2) && 
            angle(v1, v2) <= PI * 1 / 3 &&
            far.y <= centroid.y && 
            pointPolygonTest(hull, far, false) > 0) {
            defectPoints.push_back(far);
        }
    }
    return defectPoints;
}

vector<Point> handtracking::getFingertips(vector<Point> contour, vector<Point> approxHull) {
    vector<Vec4i> defects;
    vector<Point> hull;
    vector<int> indexHull;

    convexHull(contour, hull, 0, 1);
    convexHull(contour, indexHull, 0, 0);
    if (indexHull.size() < 3)
        return vector<Point>();
    convexityDefects(contour, indexHull, defects);

    vector<Point> fingerTips;
    Point centroid = getCentroid(contour);
    int maxHeight = 0;
    for (int i = 0; i < defects.size(); i++) {
        Vec4i defect = defects[i];
        Point start = contour[defect[0]];
        Point end = contour[defect[1]];
        Point far = contour[defect[2]];
        int depth = defect[3];
        int height = distance(far, centroid);
        maxHeight = max(maxHeight, height);
        Point v1 = start - far;
        Point v2 = end - far;
        if (PI * 1 / 10 <= angle(v1, v2) && 
            angle(v1, v2) <= PI * 1 / 3 &&
            far.y <= centroid.y && 
            pointPolygonTest(hull, far, false) > 0) {
            fingerTips.push_back(start);
            fingerTips.push_back(end);
        }
    }

    fingerTips = cluster(fingerTips, maxHeight / 4);

    if (fingerTips.size() == 0) {
        hull = approxHull;
        int minY = 1 << 29;
        Point minA;
        Point minB;
        Point minC;
        for (int i = 0; i < hull.size(); i++) {
            if (hull[i].y < minY) {
                minY = hull[i].y;
                minA = hull[(i - 1 + hull.size()) % hull.size()];
                minB = hull[i];
                minC = hull[(i + 1) % hull.size()];
            }
        }
        float topAngle = angle(minA - minB, minC - minB);
        if (topAngle <= PI * 1 / 2) {
            fingerTips.push_back(minB);
        }
    }
    return fingerTips;
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

float handtracking::angle(Point v1, Point v2)
{
    float len1 = sqrt(v1.x * v1.x + v1.y * v1.y);
    float len2 = sqrt(v2.x * v2.x + v2.y * v2.y);

    float dot = v1.x * v2.x + v1.y * v2.y;

    float a = dot / (len1 * len2);

    if (a >= 1.0)
        return 0.0;
    else if (a <= -1.0)
        return PI;
    else
        return acos(a);
}

