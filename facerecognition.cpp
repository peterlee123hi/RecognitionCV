#include "facerecognition.hpp"

vector<Rect> facerecognition::detectFaces(Mat& cameraFeed, string face_cascade_path) {
    vector<Rect> faces;
    CascadeClassifier face_cascade;
    if (!face_cascade.load(face_cascade_path)) {
        return vector<Rect>();
    }

    Mat frame_gray;
    cvtColor(cameraFeed, frame_gray, COLOR_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);

    float size_factor = 1.1;
    int num_buffers = 2;
    face_cascade.detectMultiScale(frame_gray, faces, size_factor, num_buffers, 0 | CASCADE_SCALE_IMAGE);

    return faces;
}

