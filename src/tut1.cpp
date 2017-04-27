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
Mat img;
int threshval = 160;            //轨迹条滑块对应的值，给初值160
int g_nContrastValue; //对比度值
int g_nBrightValue;  //亮度值
Mat g_srcImage,g_dstImage;

//-----------------------------【ContrastAndBright( )函数】------------------------------------
//     描述：改变图像对比度和亮度值的回调函数
//-----------------------------------------------------------------------------------------------
static void ContrastAndBright(int, void *)
{

  //三个for循环，执行运算 g_dstImage(i,j) =a*g_srcImage(i,j) + b
  for(int y = 0; y < g_srcImage.rows; y++ )
    {
      for(int x = 0; x < g_srcImage.cols; x++ )
        {
          for(int c = 0; c < 3; c++ )
            {
              g_dstImage.at<Vec3b>(y,x)[c]= saturate_cast<uchar>( (g_nContrastValue*0.01)*(g_srcImage.at<Vec3b>(y,x)[c] ) + g_nBrightValue );
            }
        }
    }

  //显示图像
  imshow("contrast_light", g_srcImage);
  imshow("contrast_light", g_dstImage); 
}

// callback contour
static void on_trackbar(int, void*)
{
  Mat bw = threshval < 128 ? (img < threshval) : (img > threshval);

  //定义点和向量
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;

  //查找轮廓
  findContours( bw, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
  //初始化dst
  Mat dst = Mat::zeros(img.size(), CV_8UC3);
  //开始处理
  if( !contours.empty() && !hierarchy.empty() )
    {
      //遍历所有顶层轮廓，随机生成颜色值绘制给各连接组成部分
      int idx = 0;
      for( ; idx >= 0; idx = hierarchy[idx][0] )
        {
          Scalar color( (rand()&255), (rand()&255), (rand()&255) );
          //绘制填充轮廓
          drawContours( dst, contours, idx, color, CV_FILLED, 8, hierarchy );
        }
    }
  //显示窗口
  imshow( "Connected Components", dst );
}

//从摄像头中读入数据
int main(int argc, char** argv)
{
  //  cvNamedWindow("camera", CV_WINDOW_AUTOSIZE);
  //  namedWindow( "Connected Components", 1 );
  //  namedWindow("contrast_light", 1);
  cvNamedWindow("boxfiler", 1);
  Mat img_1;
  CvCapture* capture=cvCreateCameraCapture(-1);//如果参数为1，则从摄像头中读入数据，并返回一个CvCapture的指针
  //assert(capture != NULL); //断言（assert）使用，检查capture是否为空指针，为假时程序退出，并打印错误消息
  IplImage* frame=NULL;
  //  SurfFeatureDetector detector( 400 );

  //设定对比度和亮度的初值
  //  g_nContrastValue=220;
  //  g_nBrightValue=80;
  while (1)
    {
      frame = cvQueryFrame(capture);//用于将下一帧视频文件载入内存（实际是填充和更新CvCapture结构中），返回一个对应当前帧的指针
      if (!frame)
        break;
      // todo
      img_1 = Mat(frame , 0);
      //      cv::cvtColor(img_1, img, CV_RGB2GRAY);
      g_srcImage = img_1;
      g_dstImage= Mat::zeros( g_srcImage.size(), g_srcImage.type() );
      boxFilter(g_srcImage, g_dstImage, -1,Size(5, 5));
      imshow("src_image", g_srcImage);
      imshow("dst_image", g_dstImage);

#if 0
      std::vector<Mat>channels;
      std::vector<KeyPoint> keypoints_1;


      detector.detect( img_1, keypoints_1 );

      //-- Draw keypoints
      Mat img_keypoints_1;

      drawKeypoints( img_1, keypoints_1, img_keypoints_1, Scalar::all(-1), DrawMatchesFlags::DEFAULT );

      //-- Show detected (drawn) keypoints
      imshow("Keypoints", img_keypoints_1 );


      imshow("image_origin", img);


      // trackbar
      createTrackbar( "Threshold", "Connected Components", &threshval, 255, on_trackbar );
      on_trackbar(threshval, 0);//轨迹条回调函数

      createTrackbar("contrast", "contrast_light",&g_nContrastValue,300,ContrastAndBright );
      createTrackbar("light","contrast_light",&g_nBrightValue,200,ContrastAndBright );

      //调用回调函数
      ContrastAndBright(g_nContrastValue,0);
      ContrastAndBright(g_nBrightValue,0);
#endif
      //
      //    cvShowImage("camera", frame);//在这里更改弹出的窗口名称
      //CTime time;
      //time = CTime::GetCurrentTime();
      //      imshow("redpic0", img_2);
      //      imshow("redpic1", channels[1]);
      //      imshow("redpic2", channels[2]);
      //CStringA filename(time.Format(CString("%Y%m%d%H%M%S")) + ".jpg");//在这里更改图片名称	
      //cvSaveImage("f:\\camera\\" + filename, frame);//在这里更改图片的保存位置
      char c = cvWaitKey(20);//在这里调节帧率，当前为1000/33帧每秒
      if (c == 27)
        break; //ESC键退出循环，读入数据停止

    }
  cvReleaseCapture(&capture);//释放内存
  cvDestroyWindow("camera");
}
