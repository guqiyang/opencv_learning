#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <gstreamer-0.10/gst/gst.h>
#include <gstreamer-0.10/gst/gstelement.h>
#include <gstreamer-0.10/gst/app/gstappsink.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
 
using namespace std;
using namespace cv;
 
/* Structure to contain all our information, so we can pass it around */
typedef struct _CustomData
{
    GstElement *appsink;
    GstElement *colorSpace;    
    GstElement *pipeline;
    GstElement *vsource_capsfilter, *mixercsp_capsfilter, *cspappsink_capsfilter;
    GstElement *mixer_capsfilter;
    GstElement *bin_capture;
    GstElement *video_source, *deinterlace;     
    GstElement *nv_video_mixer;    
    GstPad *pad;
    GstCaps *srcdeinterlace_caps, *mixercsp_caps, *cspappsink_caps;    
    GstBus *bus;
    GstMessage *msg;        
}gstData;
 
GstBuffer* buffer;        
 
pthread_mutex_t threadMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t waitForGstBuffer = PTHREAD_COND_INITIALIZER; 
 
/* Global variables */
CascadeClassifier face_cascade;
IplImage *frame = NULL;     
string window_name =         "Toradex Face Detection Demo";
String face_cascade_name =    "/home/root/haarcascade_frontalface_alt2.xml";
const int BORDER =             8;          // Border between GUI elements to the edge of the image.
 
template <typename T> string toString(T t)
{
    ostringstream out;
    out << t;
    return out.str();
}
 
// Draw text into an image. Defaults to top-left-justified text, but you can give negative x coords for right-justified text,
// and/or negative y coords for bottom-justified text
// Returns the bounding rect around the drawn text
Rect drawString(Mat img, string text, Point coord, Scalar color, float fontScale = 0.6f, int thickness = 1, int fontFace = FONT_HERSHEY_COMPLEX)
{
    // Get the text size & baseline.
    int baseline = 0;
    Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
    baseline += thickness;
 
    // Adjust the coords for left/right-justified or top/bottom-justified.
    if (coord.y >= 0) {
        // Coordinates are for the top-left corner of the text from the top-left of the image, so move down by one row.
        coord.y += textSize.height;
    }
    else {
        // Coordinates are for the bottom-left corner of the text from the bottom-left of the image, so come up from the bottom.
        coord.y += img.rows - baseline + 1;
    }
    // Become right-justified if desired.
    if (coord.x < 0) {
        coord.x += img.cols - textSize.width + 1;
    }
 
    // Get the bounding box around the text.
    Rect boundingRect = Rect(coord.x, coord.y - textSize.height, textSize.width, baseline + textSize.height);
 
    // Draw anti-aliased text.
    putText(img, text, coord, fontFace, fontScale, color, thickness, CV_AA);
 
    // Let the user know how big their text is, in case they want to arrange things.
    return boundingRect;
}
 
void create_pipeline(gstData *data)
{
    data->pipeline = gst_pipeline_new ("pipeline");
    gst_element_set_state (data->pipeline, GST_STATE_NULL);
}
 
gboolean CaptureGstBuffer(GstAppSink *sink, gstData *data)
{            
    //g_signal_emit_by_name (sink, "pull-buffer", &buffer);
    pthread_mutex_lock(&threadMutex);
    buffer = gst_app_sink_pull_buffer(sink);
    if (buffer)
    {        
        frame = cvCreateImage(cvSize(720, 576), IPL_DEPTH_16U, 3);
        if (frame == NULL)
        {
            g_printerr("IplImageFrame is null.\n");
        }
        else
        {
            //buffer = gst_app_sink_pull_buffer(sink);
            frame->imageData = (char*)GST_BUFFER_DATA(buffer);        
            if (frame->imageData == NULL)
            {
                g_printerr("IplImage data is null.\n");        
            }
        }        
        pthread_cond_signal(&waitForGstBuffer);            
    }            
    pthread_mutex_unlock(&threadMutex);
    return TRUE;
}
 
gboolean init_video_capture(gstData *data)
{    
    data->video_source = gst_element_factory_make("v4l2src", "video_source_live");
    data->vsource_capsfilter = gst_element_factory_make ("capsfilter", "vsource_cptr_capsfilter");
    data->deinterlace = gst_element_factory_make("deinterlace", "deinterlace_live");
    data->nv_video_mixer = gst_element_factory_make("nv_omx_videomixer", "nv_video_mixer_capture");    
    data->mixercsp_capsfilter = gst_element_factory_make ("capsfilter", "mixercsp_capsfilter");
    data->colorSpace = gst_element_factory_make("ffmpegcolorspace", "csp");        
    data->cspappsink_capsfilter = gst_element_factory_make ("capsfilter", "cspappsink_capsfilter");
    data->appsink = gst_element_factory_make("appsink", "asink");
         
    if (!data->video_source || !data->vsource_capsfilter || !data->deinterlace || !data->nv_video_mixer || !data->mixercsp_capsfilter || !data->appsink \
        || !data->colorSpace || !data->cspappsink_capsfilter)
    {
        g_printerr ("Not all elements for video were created.\n");
        return FALSE;
    }        
     
    g_signal_connect( data->pipeline, "deep-notify", G_CALLBACK( gst_object_default_deep_notify ), NULL );        
     
    gst_app_sink_set_emit_signals((GstAppSink*)data->appsink, true);
    gst_app_sink_set_drop((GstAppSink*)data->appsink, true);
    gst_app_sink_set_max_buffers((GstAppSink*)data->appsink, 1);    
     
    data->srcdeinterlace_caps = gst_caps_from_string("video/x-raw-yuv, width=(int)720, height=(int)576, format=(fourcc)I420, framerate=(fraction)1/1");        
    if (!data->srcdeinterlace_caps)
        g_printerr("1. Could not create media format string.\n");        
    g_object_set (G_OBJECT (data->vsource_capsfilter), "caps", data->srcdeinterlace_caps, NULL);
    gst_caps_unref(data->srcdeinterlace_caps);        
     
    data->mixercsp_caps = gst_caps_from_string("video/x-raw-yuv, width=(int)720, height=(int)576, format=(fourcc)I420, framerate=(fraction)1/1, pixel-aspect-ratio=(fraction)1/1");    
    if (!data->mixercsp_caps)
        g_printerr("2. Could not create media format string.\n");        
    g_object_set (G_OBJECT (data->mixercsp_capsfilter), "caps", data->mixercsp_caps, NULL);
    gst_caps_unref(data->mixercsp_caps);    
     
    data->cspappsink_caps = gst_caps_from_string("video/x-raw-yuv, width=(int)720, height=(int)576, format=(fourcc)I420, framerate=(fraction)1/1");        
    if (!data->cspappsink_caps)
        g_printerr("3. Could not create media format string.\n");        
    g_object_set (G_OBJECT (data->cspappsink_capsfilter), "caps", data->cspappsink_caps, NULL);    
    gst_caps_unref(data->cspappsink_caps);        
             
    data->bin_capture = gst_bin_new ("bin_capture");        
     
    /*if(g_signal_connect(data->appsink, "new-buffer", G_CALLBACK(CaptureGstBuffer), NULL) <= 0)
    {
        g_printerr("Could not connect signal handler.\n");
        exit(1);
    }*/
     
    gst_bin_add_many (GST_BIN (data->bin_capture), data->video_source, data->vsource_capsfilter, data->deinterlace, data->nv_video_mixer, \
                        data->mixercsp_capsfilter, data->colorSpace, data->cspappsink_capsfilter, data->appsink, NULL);
     
    if (gst_element_link_many(data->video_source, data->vsource_capsfilter, data->deinterlace, NULL) != TRUE)
    {
        g_printerr ("video_src to deinterlace not linked.\n");
        return FALSE;
    }        
     
    if (gst_element_link_many (data->deinterlace, data->nv_video_mixer, NULL) != TRUE)
    {
        g_printerr ("deinterlace to video_mixer not linked.\n");
        return FALSE;
    }        
     
    if (gst_element_link_many (data->nv_video_mixer, data->mixercsp_capsfilter, data->colorSpace, NULL) != TRUE)
    {
        g_printerr ("video_mixer to colorspace not linked.\n");
        return FALSE;    
    }
     
    if (gst_element_link_many (data->colorSpace, data->appsink, NULL) != TRUE)
    {
        g_printerr ("colorspace to appsink not linked.\n");
        return FALSE;    
    }
     
    cout << "Returns from init_video_capture." << endl;
    return TRUE;
}
 
void delete_pipeline(gstData *data)
{
    gst_element_set_state (data->pipeline, GST_STATE_NULL);
    g_print ("Pipeline set to NULL\n");
    gst_object_unref (data->bus);
    gst_object_unref (data->pipeline);
    g_print ("Pipeline deleted\n");
}
 
gboolean add_bin_capture_to_pipe(gstData *data)
{
    if((gst_bin_add(GST_BIN (data->pipeline), data->bin_capture)) != TRUE)
    {
        g_print("bin_capture not added to pipeline\n");
    }
     
    if(gst_element_set_state (data->pipeline, GST_STATE_NULL) == GST_STATE_CHANGE_SUCCESS)
    {        
        return TRUE;
    }
    else
    {
        cout << "Failed to set pipeline state to NULL." << endl;
        return FALSE;        
    }
}
 
gboolean remove_bin_capture_from_pipe(gstData *data)
{
    gst_element_set_state (data->pipeline, GST_STATE_NULL);
    gst_element_set_state (data->bin_capture, GST_STATE_NULL);
    if((gst_bin_remove(GST_BIN (data->pipeline), data->bin_capture)) != TRUE)
    {
        g_print("bin_capture not removed from pipeline\n");
    }    
    return TRUE;
}
 
gboolean start_capture_pipe(gstData *data)
{
    if(gst_element_set_state (data->pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_SUCCESS)
        return TRUE;
    else
    {
        cout << "Failed to set pipeline state to PLAYING." << endl;
        return FALSE;
    }
}
 
gboolean stop_capture_pipe(gstData *data)
{
    gst_element_set_state (data->bin_capture, GST_STATE_NULL);
    gst_element_set_state (data->pipeline, GST_STATE_NULL);
    return TRUE;
}
 
gboolean deinit_video_live(gstData *data)
{
    gst_element_set_state (data->pipeline, GST_STATE_NULL);
    gst_element_set_state (data->bin_capture, GST_STATE_NULL);
    gst_object_unref (data->bin_capture);
    return TRUE;
}
 
gboolean check_bus_cb(gstData *data)
{
    GError *err = NULL;                
    gchar *dbg = NULL;   
           
    g_print("Got message: %s\n", GST_MESSAGE_TYPE_NAME(data->msg));
    switch(GST_MESSAGE_TYPE (data->msg))
    {
        case GST_MESSAGE_EOS:       
            g_print ("END OF STREAM... \n");
            break;
 
        case GST_MESSAGE_ERROR:
            gst_message_parse_error (data->msg, &err, &dbg);
            if (err)
            {
                g_printerr ("ERROR: %s\n", err->message);
                g_error_free (err);
            }
            if (dbg)
            {
                g_printerr ("[Debug details: %s]\n", dbg);
                g_free (dbg);
            }
            break;
 
        default:
            g_printerr ("Unexpected message of type %d", GST_MESSAGE_TYPE (data->msg));
            break;
    }
    return TRUE;
}
 
void get_pipeline_bus(gstData *data)
{
    data->bus = gst_element_get_bus (data->pipeline);
    data->msg = gst_bus_poll (data->bus, (GstMessageType)(GST_MESSAGE_EOS | GST_MESSAGE_ERROR), -1);
    if(GST_MESSAGE_TYPE (data->msg))
    {
        check_bus_cb(data);
    }
    gst_message_unref (data->msg);
}
 
int main(int argc, char *argv[])
{        
    //Mat frame;
    VideoCapture capture;    
    gstData gstreamerData;
    GstBuffer *gstImageBuffer;
     
    //XInitThreads();
    gst_init (&argc, &argv);
    create_pipeline(&gstreamerData);
    if(init_video_capture(&gstreamerData))
    {        
        add_bin_capture_to_pipe(&gstreamerData);    
        start_capture_pipe(&gstreamerData);
        //get_pipeline_bus(&gstreamerData);    
     
        cout << "Starting while loop..." << endl;
        cvNamedWindow("Toradex Face Detection Demo with Gstreamer", 0);    
     
        while(true)
        {    
            //pthread_mutex_lock(&threadMutex);
            //pthread_cond_wait(&waitForGstBuffer, &threadMutex);
             
            gstImageBuffer = gst_app_sink_pull_buffer((GstAppSink*)gstreamerData.appsink);
         
            if (gstImageBuffer != NULL)
            {        
                frame = cvCreateImage(cvSize(720, 576), IPL_DEPTH_8U, 1);
                     
                if (frame == NULL)
                {
                    g_printerr("IplImageFrame is null.\n");
                }
                else
                {        
                    frame->imageData = (char*)GST_BUFFER_DATA(gstImageBuffer);        
                    if (frame->imageData == NULL)
                    {
                        g_printerr("IplImage data is null.\n");            
                    }                    
                    cvShowImage("Toradex Face Detection Demo with Gstreamer", frame);  
                    cvWaitKey(1);                    
                    gst_buffer_unref(gstImageBuffer);
                }
            }
            else
            {
                cout << "Appsink buffer didn't return buffer." << endl;
            }
            /*
            if (frame)
            {
                cvShowImage("Toradex Face Detection Demo with Gstreamer", frame);
            }
            gst_buffer_unref(buffer);
            buffer = NULL;            
            pthread_mutex_unlock(&threadMutex);    
            cvWaitKey(1);*/                                    
        }
    }
    else
    {
        exit(1);
    }
               
    //Destroy the window
    cvDestroyWindow("Toradex Face Detection Demo with Gstreamer");
       remove_bin_capture_from_pipe(&gstreamerData);
       deinit_video_live(&gstreamerData);    
    delete_pipeline(&gstreamerData);
     
       return 0;
}
