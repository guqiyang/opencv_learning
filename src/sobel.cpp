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
  Mat grad_x, grad_y;
  Mat abs_grad_x, abs_grad_y,dst;
  g_srcImage = imread("1.jpg");


  Sobel(g_srcImage, grad_x, CV_16S, 1, 0, 3, 1, 1, BORDER_DEFAULT );
  convertScaleAbs( grad_x, abs_grad_x );
  imshow("x", abs_grad_x);

  Sobel(g_srcImage, grad_y, CV_16S, 0, 1, 3, 1, 1, BORDER_DEFAULT );
  convertScaleAbs( grad_y, abs_grad_y);
  imshow("y", abs_grad_y);

  addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, dst);
  imshow("total", dst);
  waitKey(0);
  return 1;
}

