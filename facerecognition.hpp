#ifndef __FACERECOGNITION_H_INCLUDED__
#define __FACERECOGNITION_H_INCLUDED__

// C++ Libraries
#include <cmath>
#include <algorithm>
#include <vector>

// OpenCV Libraries
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>

using namespace std;
using namespace cv;

class facerecognition {
public:
    static vector<Rect> detectFaces(Mat& cameraFeed, string face_cascade_path);
};

#endif
