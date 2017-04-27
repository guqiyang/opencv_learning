#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/opencv.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<iostream>
using namespace std;
using namespace cv;

int g_nElementSize = 15;
int g_nTrackType = 1;
Mat g_srcImage, g_dstImage;

void Process();
void on_TrackTypeChange(int ,void*);
void on_ElementSizeChange(int , void*);
int main(int argc, char** argv)
{
  g_srcImage = imread("1.jpg");
  cvNamedWindow("pengzhan", CV_WINDOW_AUTOSIZE);

  imshow("pengzhan", g_srcImage);

  createTrackbar("size", "pengzhan", &g_nElementSize, 30, on_ElementSizeChange);
  createTrackbar("type", "pengzhan", &g_nTrackType, 1, on_TrackTypeChange);
  waitKey(0);
  return 1;
}

void on_TrackTypeChange(int ,void*)
{
  Process();
}
void on_ElementSizeChange(int , void*)
{
  Process();
}

void Process()
{
  //获取自定义核
  Mat element =  getStructuringElement(MORPH_RECT, Size(g_nElementSize * 2 + 1, g_nElementSize * 2 + 1), Point(g_nElementSize, g_nElementSize));

   //进行腐蚀或膨胀操
  if(g_nTrackType == 0) {
    erode(g_srcImage,g_dstImage, element);
  }
  else{
    dilate(g_srcImage,g_dstImage, element);
  }

  //显示效果图
  imshow("pengzhan", g_dstImage);
}
