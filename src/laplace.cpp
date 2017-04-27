#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/opencv.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<iostream>
using namespace std;
using namespace cv;
Mat g_srcImage;
int g_nsize = 1;

int main(int argc, char** argv)
{
  Mat src, dst, src_gray, abs_dst;
  src = imread("1.jpg");
  GaussianBlur( src, src, Size(3,3), 0, 0, BORDER_DEFAULT );
  cvtColor( src, src_gray, CV_RGB2GRAY );
  Laplacian( src_gray, dst, CV_16S, 3, 1, 0, BORDER_DEFAULT );
  convertScaleAbs( dst, abs_dst );
  imshow("laplace", abs_dst);

  
  waitKey(0);
  return 1;
}

