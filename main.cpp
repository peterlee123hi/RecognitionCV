#include "main.hpp"

using namespace std;
using namespace cv;

const int SIDE = 8;
const int NSAMPLES = 7;

int averageColor[NSAMPLES][3];
int c_lower[NSAMPLES][3];
int c_upper[NSAMPLES][3];
int averageBGR[3];

vector<Point> POI;

void drawPOI(Mat m, Point center) {
    Scalar color(0, 255, 0);
    rectangle(m, Point(center.x - SIDE / 2, center.y - SIDE / 2), Point(center.x + SIDE / 2, center.y + SIDE / 2), color, 2);
}

void drawText(Mat& m, string text, Point center) {
    int fontFace = FONT_HERSHEY_PLAIN;
    int baseline = 0;
    double scale = 1.6;
    Size textSize = getTextSize(text, fontFace, scale, 0, &baseline);
    Point corner(center.x - textSize.width / 2, center.y - textSize.height / 2);
    putText(m, text, corner, fontFace, scale, Scalar(200, 0, 0), 2);
}

void drawTitle(Mat& m, string text) {
    drawText(m, text, Point(m.cols / 2, m.rows / 10));
}

void calibrate(VideoCapture& camera) {
    while (true) {
        Mat cameraFeed;
        camera.read(cameraFeed);
        flip(cameraFeed, cameraFeed, 1);

        int offset = 20;
        POI.push_back(Point(cameraFeed.cols / 2 + offset, cameraFeed.rows / 2 + offset));
        POI.push_back(Point(cameraFeed.cols / 2 + offset, cameraFeed.rows / 2));
        POI.push_back(Point(cameraFeed.cols / 2, cameraFeed.rows / 2 + offset));
        POI.push_back(Point(cameraFeed.cols / 2, cameraFeed.rows / 2));
        POI.push_back(Point(cameraFeed.cols / 2 - offset, cameraFeed.rows / 2 - offset));
        POI.push_back(Point(cameraFeed.cols / 2 - offset, cameraFeed.rows / 2));
        POI.push_back(Point(cameraFeed.cols / 2, cameraFeed.rows / 2 - offset));

        drawTitle(cameraFeed, "Press space to calibrate.");
        for (int i = 0; i < POI.size(); i++) {
            drawPOI(cameraFeed, POI[i]);
        }

        imshow("Camera", cameraFeed);
        if (waitKey(30) == char(' '))
            break;
    }
}

int getMedian(vector<int> val) {
    int median;
    int size = val.size();
    sort(val.begin(), val.end());
    if (size % 2 == 0)
        median = val[size / 2 - 1];
    else
        median = val[size / 2];
    return median;
}

void getAverageColor(Mat& m, Point p, int average[3]) {
    Mat r = m(Rect(p.x - SIDE / 2, p.y - SIDE / 2, SIDE, SIDE));
    vector<int> hm;
    vector<int> sm;
    vector<int> lm;

    for (int i = 2; i < r.rows - 2; i++) {
        for (int j = 2; j < r.cols - 2; j++) {
            hm.push_back(r.data[r.channels() * (r.cols * i + j) + 0]);
            sm.push_back(r.data[r.channels() * (r.cols * i + j) + 1]);
            lm.push_back(r.data[r.channels() * (r.cols * i + j) + 2]);
        }
    }

    average[0] = getMedian(hm);
    average[1] = getMedian(sm);
    average[2] = getMedian(lm);
}

void average(VideoCapture& camera) {
    for (int i = 0; i < 30; i++) {
        Mat cameraFeed;
        camera.read(cameraFeed);
        flip(cameraFeed, cameraFeed, 1);
        cvtColor(cameraFeed, cameraFeed, CV_BGR2HLS);
        for (int j = 0; j < NSAMPLES; j++) {
            getAverageColor(cameraFeed, POI[j], averageColor[j]);
            drawPOI(cameraFeed, POI[j]);
        }
        cvtColor(cameraFeed, cameraFeed, CV_HLS2BGR);
        drawTitle(cameraFeed, "Calculating average color...");
        imshow("Camera", cameraFeed);
        if (waitKey(30) >= 0)
            break;
    }
}

void initTrackbars() {
    for(int i = 0; i < NSAMPLES; i++) {
        c_lower[i][0] = 12;
        c_upper[i][0] = 7;
        c_lower[i][1] = 30;
        c_upper[i][1] = 40;
        c_lower[i][2] = 80;
        c_upper[i][2] = 80;
    }

    namedWindow("trackbars", CV_WINDOW_KEEPRATIO);

    createTrackbar("lower1", "trackbars", &c_lower[0][0],255);
    createTrackbar("lower2", "trackbars", &c_lower[0][1],255);
    createTrackbar("lower3", "trackbars", &c_lower[0][2],255);
    createTrackbar("upper1", "trackbars", &c_upper[0][0],255);
    createTrackbar("upper2", "trackbars", &c_upper[0][1],255);
    createTrackbar("upper3", "trackbars", &c_upper[0][2],255);
}

void normalizeColors(Mat& m) {
    for (int i = 1; i < NSAMPLES; i++) {
        for (int j = 0; j < 3; j++) {
            c_lower[i][j] = c_lower[0][j];
            c_upper[i][j] = c_upper[0][j];
        }
    }

    for (int i = 0; i < NSAMPLES; i++) {
        if (averageColor[i][0] - c_lower[i][0] < 0)
            c_lower[i][0] = averageColor[i][0];
        if (averageColor[i][1] - c_lower[i][1] < 0)
            c_lower[i][1] = averageColor[i][1];
        if (averageColor[i][2] - c_lower[i][2] < 0)
            c_lower[i][2] = averageColor[i][2];
        if (averageColor[i][0] + c_upper[i][0] > 255) 
            c_upper[i][0] = 255 - averageColor[i][0];
        if (averageColor[i][1] + c_upper[i][1] > 255)
            c_upper[i][1] = 255 - averageColor[i][1];
        if (averageColor[i][2] + c_upper[i][2] > 255)
            c_upper[i][2] = 255 - averageColor[i][2];
    }
}

void generateBinary(Mat& filtered, Mat& binary, vector<Mat>& binaryList) {
    Scalar lowerBound;
    Scalar upperBound;

    for (int i = 0; i < NSAMPLES; i++) {
        normalizeColors(filtered);
        lowerBound = Scalar(averageColor[i][0] - c_lower[i][0], averageColor[i][1] - c_lower[i][1], averageColor[i][2] - c_lower[i][2]);
        upperBound = Scalar(averageColor[i][0] + c_upper[i][0], averageColor[i][1] + c_upper[i][1], averageColor[i][2] + c_upper[i][2]);
        binaryList.push_back(Mat(filtered.rows, filtered.cols, CV_8U));
        inRange(filtered, lowerBound, upperBound, binaryList[i]);
    }

    binaryList[0].copyTo(binary);
    for (int i = 1; i < NSAMPLES; i++) {
        binary += binaryList[i];
    }
    medianBlur(binary, binary, 7);
}

void display(Mat& cameraFeed, Mat& binary) {
    pyrDown(binary, binary);
    Rect r(Point(3 * cameraFeed.cols / 4, 0), binary.size());
    vector<Mat> channels;
    Mat result;
    for (int i = 0; i < 3; i++) {
        channels.push_back(binary);
    }
    merge(channels, result);
    result.copyTo(cameraFeed(r));
    imshow("Camera", cameraFeed);
}

vector<Point> isolateContour(Mat& m) {
    Mat binary;
    m.copyTo(binary);
    pyrUp(binary, binary);
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(binary, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

    for (int i = 0; i < contours.size(); i++) {
        approxPolyDP(Mat(contours[i]), contours[i], 3, true);
    }

    int maxPoints = 0;
    vector<Point> contour;
    for (int i = 0; i < contours.size(); i++) {
        if (contours[i].size() > maxPoints) {
            maxPoints = contours[i].size();
            contour = contours[i];
        }
    }

    return contour;
}

void drawContour(Mat& m, vector<Point> contour) {
    for (int i = 0; i < contour.size(); i++) {
        line(m, contour[i], contour[(i + 1) % contour.size()], Scalar(255, 0, 0), 2);
    }
}

int main() {
    VideoCapture camera(0);
    if (!camera.isOpened()) {
        cout << "Cannot open camera.";
    }

    calibrate(camera);
    average(camera);
    initTrackbars();

    while (true) {
        Mat cameraFeed;
        Mat filtered;
        Mat binary;
        vector<Mat> binaryList;
        camera.read(cameraFeed);
        flip(cameraFeed, cameraFeed, 1);

        pyrDown(cameraFeed, filtered);
        blur(filtered, filtered, Size(3, 3));
        cvtColor(filtered, filtered, CV_BGR2HLS);
        generateBinary(filtered, binary, binaryList);
        cvtColor(filtered, filtered, CV_HLS2BGR);
        drawContour(cameraFeed, isolateContour(binary));

        display(cameraFeed, binary);
        if (waitKey(30) == char(' '))
            break;
    }

    camera.release();
    destroyAllWindows();
    return 0;
}
