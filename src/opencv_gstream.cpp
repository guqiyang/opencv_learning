#include <stdio.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;
int main(int argc, char* argv[]) {


    // first part of sender pipeline
  cv::VideoCapture cap("v4l2src ! video/x-raw, framerate=30/1, width=640, height=480, format=RGB ! videoconvert ! appsink");
    if (!cap.isOpened()) {
        printf("=ERR= can't create video capture\n");
        return -1;
    }
#if 0
    // second part of sender pipeline
    cv::VideoWriter writer;
    writer.open(""
                , 0, (double)30, cv::Size(640, 480), true);
    if (!writer.isOpened()) {
        printf("=ERR= can't create video writer\n");
        return -1;
    }
#endif
    cv::Mat frame;
    int key;

    while (true) {

        cap >> frame;
        if (frame.empty())
            break;

        /* Process the frame here */
        imshow("frame", frame);
        //      writer << frame;
        key = cv::waitKey( 30 );
        if (27 == key)
          break;
    }

    return 1;
}
