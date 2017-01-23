#include "main.hpp"

using namespace std;
using namespace cv;

const int SIDE = 8;
const int NSAMPLES = 7;
// Minimum size for contour to be detected.
const double MIN_CONTOUR_RATIO = 0.02;
const double MIN_DRAW_DISTANCE = 8;

int averageColor[NSAMPLES][3];
int c_lower[NSAMPLES][3];
int c_upper[NSAMPLES][3];
int averageBGR[3];

vector<Point> POI;
vector<Point> drawnLines;

Scalar blue(200, 0, 0);
Scalar green(0, 200, 0);
Scalar red(0, 0, 200);

bool generateVideo = false;
VideoWriter out;
VideoWriter rawOut;

void drawCircle(Mat m, Point center, Scalar color, int radius) {
    circle(m, center, radius, color, 2);
}

void drawDot(Mat m, Point center, Scalar color) {
    circle(m, center, 2, color, 3);
}

void drawPOI(Mat m, Point center, Scalar color) {
    rectangle(m, Point(center.x - SIDE / 2, center.y - SIDE / 2), Point(center.x + SIDE / 2, center.y + SIDE / 2), color, 2);
}

void drawText(Mat& m, string text, Point center, Scalar color) {
    int fontFace = FONT_HERSHEY_PLAIN;
    int baseline = 0;
    double scale = 1.6;
    Size textSize = getTextSize(text, fontFace, scale, 0, &baseline);
    Point corner(center.x - textSize.width / 2, center.y - textSize.height / 2);
    putText(m, text, corner, fontFace, scale, color, 2);
}

void drawTitle(Mat& m, string text) {
    drawText(m, text, Point(m.cols / 2, m.rows / 10), green);
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
            drawPOI(cameraFeed, POI[i], blue);
        }

        if (generateVideo) {
            out << cameraFeed;
            rawOut << cameraFeed;
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
            drawPOI(cameraFeed, POI[j], green);
        }
        cvtColor(cameraFeed, cameraFeed, CV_HLS2BGR);
        drawTitle(cameraFeed, "Calculating average color...");

        if (generateVideo) {
            out << cameraFeed;
            rawOut << cameraFeed;
        }

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

void display(Mat& cameraFeed, Mat& raw, Mat& binary) {
    pyrDown(binary, binary);
    Rect r(Point(3 * cameraFeed.cols / 4, 0), binary.size());
    vector<Mat> channels;
    Mat result;
    for (int i = 0; i < 3; i++) {
        channels.push_back(binary);
    }
    merge(channels, result);
    result.copyTo(cameraFeed(r));
    result.copyTo(raw(r));

    if (generateVideo) {
        out << cameraFeed;
        rawOut << raw;
    }

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

    int leftX = 1 << 29;
    vector<Point> contour;
    for (int i = 0; i < contours.size(); i++) {
        double ratio = contourArea(contours[i]) / binary.size().area();
        int x = contours[i][0].x;
        for (int j = 0; j < contours[i].size(); j++)
            x = min(x, contours[i][j].x);
        if (ratio > MIN_CONTOUR_RATIO && x < leftX) {
            leftX = x;
            contour = contours[i];
        }
    }
    if (contour.size() == 0)
        contour = contours[0];

    return contour;
}

void drawContour(Mat& m, vector<Point> contour) {
    for (int i = 0; i < contour.size(); i++) {
        line(m, contour[i], contour[(i + 1) % contour.size()], blue, 2);
    }
}

Point getTop(vector<Point> contour) {
    Point topPoint(0, 0);
    int topY = 1 << 29;
    for (int i = 0; i < contour.size(); i++) {
        if (contour[i].y < topY) {
            topY = contour[i].y;
            topPoint = contour[i];
        }
    }
    return topPoint;
}

bool isPenDown(vector<Point> contour) {
    Point top = getTop(contour);
    int topThreshold = 60;
    for (int i = 0; i < contour.size(); i++) {
        if (top != contour[i] && abs(top.y - contour[i].y) < topThreshold) {
            return false;
        }
    }

    return top != Point(0, 0);
}

void addDrawPoint(Mat& cameraFeed, Point pen) {
    drawCircle(cameraFeed, pen, red, 60);
    if (drawnLines.size() == 0) {
        drawnLines.push_back(pen);
    } else {
        double distance = norm(pen - drawnLines.back());
        bool inBounds = MIN_DRAW_DISTANCE < distance;
        if (inBounds)
            drawnLines.push_back(pen);
    }
}

void executeDrawDemo(VideoCapture camera) {
    while (true) {
        Mat cameraFeed;
        Mat filtered;
        Mat binary;
        vector<Mat> binaryList;
        camera.read(cameraFeed);
        Mat rawCameraFeed;
        flip(cameraFeed, cameraFeed, 1);
        cameraFeed.copyTo(rawCameraFeed);

        pyrDown(cameraFeed, filtered);
        blur(filtered, filtered, Size(3, 3));
        cvtColor(filtered, filtered, CV_BGR2HLS);
        generateBinary(filtered, binary, binaryList);
        cvtColor(filtered, filtered, CV_HLS2BGR);

        vector<Point> handContour = isolateContour(binary);
        vector<Point> approxConvexHull = handtracking::getApproxConvexHull(handContour, 60);
        drawContour(cameraFeed, handContour);
        drawContour(cameraFeed, approxConvexHull);
        drawCircle(cameraFeed, handtracking::getCentroid(handContour), blue, 6);
        if (isPenDown(approxConvexHull)) {
            addDrawPoint(cameraFeed, getTop(approxConvexHull));
            drawTitle(cameraFeed, "Pen is down.");
        }
        else {
            drawTitle(cameraFeed, "Pen is NOT down.");
            if (drawnLines.size() > 0 && drawnLines.back() != Point(0, 0)) {
                drawnLines.push_back(Point(0, 0));
            }
        }

        // Draw points!
        for (int i = 0; i < ((int) drawnLines.size()) - 1; i++) {
            Point a = drawnLines[i];
            Point b = drawnLines[i + 1];
            if (a != Point(0, 0) && b != Point(0, 0)) {
                line(cameraFeed, a, b, red, 3);
                line(rawCameraFeed, a, b, red, 3);
            }
        }

        display(cameraFeed, rawCameraFeed, binary);
        if (waitKey(30) == char(' '))
            break;
    }
}

void executeFaceRecognitionDemo(VideoCapture& camera) {
    while (true) {
        Mat cameraFeed;
        Mat filtered;
        Mat binary;
        vector<Mat> binaryList;
        camera.read(cameraFeed);
        Mat rawCameraFeed;
        flip(cameraFeed, cameraFeed, 1);
        cameraFeed.copyTo(rawCameraFeed);

        pyrDown(cameraFeed, filtered);
        blur(filtered, filtered, Size(3, 3));
        cvtColor(filtered, filtered, CV_BGR2HLS);
        generateBinary(filtered, binary, binaryList);
        cvtColor(filtered, filtered, CV_HLS2BGR);

        // Draw hand data
        vector<Point> handContour = isolateContour(binary);
        vector<Point> approxConvexHull = handtracking::getApproxConvexHull(handContour, 30);
        drawContour(cameraFeed, handContour);
        drawContour(cameraFeed, approxConvexHull);
        drawCircle(cameraFeed, handtracking::getCentroid(handContour), blue, 6);
        vector<Point> defects = handtracking::getDefects(handContour);
        for (int i = 0; i < defects.size(); i++) {
            drawCircle(cameraFeed, defects[i], green, 6);
        }
        vector<Point> tips = handtracking::getFingertips(handContour, approxConvexHull);
        for (int i = 0; i < tips.size(); i++) {
            drawCircle(cameraFeed, tips[i], red, 6);
        }

        // Draw face detection
        string face_cascade_path = "/usr/local/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml";
        vector<Rect> faces = facerecognition::detectFaces(cameraFeed, face_cascade_path);
        for (int i = 0; i < faces.size(); i++) {
            rectangle(cameraFeed, faces[i], green, 4);
        }

        stringstream ss;
        ss << "Fingers: " << tips.size();
        drawTitle(cameraFeed, ss.str());

        // Draw glasses
        if (0 < faces.size() && 1 <= tips.size() && tips.size() <= 3) {
            ss.str("");
            ss << "./glasses" << tips.size() << ".png";
            Mat glasses = imread(ss.str(), IMREAD_UNCHANGED);
            Rect face = faces[0];
            double aspectRatio = glasses.cols / glasses.rows;
            Size newSize(face.width, (int) (face.width / aspectRatio));
            resize(glasses, glasses, newSize);
            int offset = 20;
            if (tips.size() == 2) {
                offset = -10;
            }
            for (int y = 0; y < glasses.rows; y++) {
                for (int x = 0; x < glasses.cols; x++) {
                    Vec4b pixel = glasses.at<Vec4b>(y, x);
                    if (pixel[3] > 0) {
                        Vec3b gPixel(pixel[0], pixel[1], pixel[2]);
                        int xCoord = face.x + x;
                        int yCoord = face.y + y;
                        cameraFeed.at<Vec3b>(Point(xCoord, yCoord + offset)) = gPixel;
                    }
                }
            }
        }

        display(cameraFeed, rawCameraFeed, binary);
        if (waitKey(30) == char(' '))
            break;
    }
}

int main() {
    VideoCapture camera(0);
    if (!camera.isOpened()) {
        cout << "Cannot open camera.";
    }

    Mat frame;
    camera >> frame;

    if (generateVideo) {
        out.open("demoTest.avi", CV_FOURCC('M', 'J', 'P', 'G'), 15, frame.size(), true);
        rawOut.open("demoPure.avi", CV_FOURCC('M', 'J', 'P', 'G'), 15, frame.size(), true);
    }

    calibrate(camera);
    average(camera);
    initTrackbars();

    executeDrawDemo(camera);
    // executeFaceRecognitionDemo(camera);

    if (generateVideo) {
        out.release();
        rawOut.release();
    }

    camera.release();
    destroyAllWindows();
    return 0;
}
