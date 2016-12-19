#include "main.hpp"

using namespace std;
using namespace cv;

int main() {
    VideoCapture camera(0);
    if (!camera.isOpened()) {
        cout << "Cannot open camera.";
    }

    while (true) {
        Mat cameraFeed;
        camera.read(cameraFeed);
        imshow("Camera", cameraFeed);
        if (waitKey(30) == char('q'))
            break;
    }
    return 0;
}
