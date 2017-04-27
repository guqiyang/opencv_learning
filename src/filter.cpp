#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/nonfree/features2d.hpp"
#include"opencv2/imgproc/imgproc.hpp"
#include<opencv2/video/video.hpp>
#include<iostream>
using namespace cv;
using namespace std;

// global
Mat g_srcImage,g_gauImage,g_medImage, g_bilImage;
int g_nGaussianBlurValue = 3;
int g_nBilateralFilterValue = 25;
int g_nMedValue = 3;

static void on_GuassianBlur(int ,void*)
{
  GaussianBlur(g_srcImage, g_gauImage, Size(g_nGaussianBlurValue * 2 + 1, g_nGaussianBlurValue * 2 + 1), 0, 0);
  imshow("gauss_image", g_gauImage);
}

static void on_bilateralFilter(int ,void*)
{
  bilateralFilter(g_srcImage, g_bilImage,  g_nBilateralFilterValue , g_nBilateralFilterValue * 2, g_nBilateralFilterValue/2 );
  imshow("bilateralFiler_image", g_bilImage);
}

static void on_MedianBlur (int ,void*)
{
  medianBlur (g_srcImage, g_medImage, g_nMedValue * 2 + 1);
  imshow("med_image", g_medImage);
}
//从摄像头中读入数据
int main(int argc, char** argv)
{
  Mat img_1;
  cvNamedWindow("gauss_image", CV_WINDOW_AUTOSIZE);
  CvCapture* capture=cvCreateCameraCapture(-1);//如果参数为1，则从摄像头中读入数据，并返回一个CvCapture的指针
  //assert(capture != NULL); //断言（assert）使用，检查capture是否为空指针，为假时程序退出，并打印错误消息
  IplImage* frame=NULL;
  //  SurfFeatureDetector detector( 400 );

  while (1)
    {
      frame = cvQueryFrame(capture);//用于将下一帧视频文件载入内存（实际是填充和更新CvCapture结构中），返回一个对应当前帧的指针]
      if (!frame)
        break;
      // todo
      g_srcImage = Mat(frame , 0);
      createTrackbar("gauss_value", "gauss_image", &g_nGaussianBlurValue, 50, on_GuassianBlur);
      on_GuassianBlur(g_nGaussianBlurValue, 0);

      createTrackbar("med_value", "bilateralFiler_image", &g_nBilateralFilterValue,50 , on_bilateralFilter);
      on_bilateralFilter(g_nBilateralFilterValue, 0);

      createTrackbar("bil_value", "med_image", &g_nMedValue, 50, on_MedianBlur);
      on_MedianBlur(g_nMedValue, 0);


      char c = cvWaitKey(20);//在这里调节帧率，当前为1000/33帧每秒
      if (c == 27)
        break; //ESC键退出循环，读入数据停止

    }
  cvReleaseCapture(&capture);//释放内存
}
