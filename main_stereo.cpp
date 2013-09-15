#include "stereoimaging.h"
#include <opencv/cv.h>
#include <opencv2/video/background_segm.hpp>
#include <opencv/highgui.h>
#include <cvblobs/BlobResult.h>
#include <libcam.h>
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace StereoImag;

//Function Declarations
void CompareWithBackground(Image &img1, Image &imgBack, Image &imgOut);
void WasteNFrames(int N);
Mat GetNextCameraShot(Camera* cam);
void DrawRect(Image &img, int x, int y, int w, int h);

//Global variables
Mat rightImage, leftImage;
Mat backImageRight, backImageLeft;
IplImage* tempImageV4L;
Camera* rightCamera;
Camera* leftCamera;
int frameNumber;

//Entry Point. 4 Arguments need to be passed
/*
arg1: Width of each frame
arg2: Height of each frame
arg3: Target frames per second of the program
arg4: Maximum number of blobs to track. Each blob MAY corresspond to a person in front of the camera
*/
int main(int argc, char* argv[])
{
    if (argc < 5)
    {
        cout << "Too few arguments to the program. Exiting...\n";
        return 0;
    }

    int width, height, fps, numberOfBlobs;
    try
    {
        //Read the arguments
        width = atoi(argv[1]);
        height = atoi(argv[2]);
        fps = atoi(argv[3]);
        numberOfBlobs = atoi(argv[4]);
        //Done reading arguments
    }
    catch(...)
    {
        cout << "One or more arguments are invalid!. Exiting...\n";
        return 0;
    }


    /*
    int width = 320;
    int height = 240;
    int fps = 10;
    int numberOfBlobs = 2;
    */

    tempImageV4L = cvCreateImage(cvSize(width, height), 8, 3);
    frameNumber = 0;

    //Beginning initialising cameras
    rightCamera = new Camera("/dev/video0", width, height, fps);
    leftCamera = new Camera("/dev/video1", width, height, fps);
	//leftCamera = rightCamera; //If only one camera is available, uncomment this line and comment the line above this.
    //Done initialising cameras

    //Waste some frames so as to get the cameras running in full flow
    WasteNFrames(10);

    //Beginning capturing background
    backImageRight = GetNextCameraShot(rightCamera);
    backImageLeft = GetNextCameraShot(leftCamera);
    frameNumber++;
    cvtColor(backImageRight, backImageRight, CV_BGR2HSV);
    cvtColor(backImageLeft, backImageLeft, CV_BGR2HSV);
    //Done capturing background

    //General Stuff
    Mat motionImageRight(backImageRight.rows, backImageRight.cols, CV_8UC1);
    Mat motionImageLeft(backImageLeft.rows, backImageLeft.cols, CV_8UC1);
    Mat HSVImageRight, HSVImageLeft;
    Mat displayImageRight, displayImageLeft;
    //End of General Stuff


    while (1) //The infinite loop
    {
        //Beginning getting camera shots
        rightImage = GetNextCameraShot(rightCamera);
        leftImage = GetNextCameraShot(leftCamera);
        frameNumber++;
        //Done getting camera shots


        //Beginning getting motion images
        HSVImageRight = rightImage.clone();
        cvtColor(HSVImageRight, HSVImageRight, CV_BGR2HSV);
        CompareWithBackground(HSVImageRight, backImageRight, motionImageRight);
        medianBlur(motionImageRight, motionImageRight, 3);

        HSVImageLeft = leftImage.clone();
        cvtColor(HSVImageLeft, HSVImageLeft, CV_BGR2HSV);
        CompareWithBackground(HSVImageLeft, backImageLeft, motionImageLeft);
        medianBlur(motionImageLeft, motionImageLeft, 3);
        //Ended getting motion images

        cout << "\nFor frame #" << frameNumber << " :\n";

        //Beginning Getting Blobs
        IplImage  imageblobPixels = motionImageRight;
        CBlobResult blobs;
        blobs = CBlobResult(&imageblobPixels, NULL, 0);	// Use a black background color.
        int minArea = 100 / ((640 / width) * (640 / width));
        blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, minArea);
        int foundBlobs = blobs.GetNumBlobs();
        //Ended Getting Blobs

        cout << "Found " << foundBlobs << " motion blobs\n";

        //Creating copies of original images for modifying and displaying
        displayImageRight = rightImage.clone();
        displayImageLeft = leftImage.clone();
        //Done creating copies

        //Cycling through the blobs
        for (int blobIndex = 0; blobIndex < blobs.GetNumBlobs() && blobIndex < numberOfBlobs; blobIndex++)
        {
            cout << "Blob #" << blobIndex << " : ";

            //Getting blob details
            CBlob * blob = blobs.GetBlob(blobIndex);
            int x = blob->GetBoundingBox().x;
            int y = blob->GetBoundingBox().y;
            int w = blob->GetBoundingBox().width;
            int h = blob->GetBoundingBox().height;
            //Done getting blob details

            int sep = 0;

            //The point for which we want to find depth
            PixPoint inP = {x + w/2, y + h/2}, oP = {0, 0};
            cout << "inPoint = {" << inP.x << ", " << inP.y << "} ";

            //Initialing the rectangle in which the corressponding point is likely in
            Rectangle rect;
            rect.location.x = -1;
            rect.location.y = inP.y - 5;
            rect.size.x = rightImage.cols;
            rect.size.y = 11;
            //Done initialising the target rectangle

            //Find the corressponding point and calculate the sepertion
            oP = PointCorresponder::correspondPoint(rightImage, leftImage, inP, rect, motionImageLeft);
            sep = inP.x - oP.x;
            cout << "foundPoint = {" << oP.x << ", " << oP.y << "} ";

            //Just for visual presentation
            DrawRect(displayImageRight, x, y, w, h);
            cv::circle(displayImageRight, Point(inP.x, inP.y), 10, Scalar(0), 3);
            cv::circle(displayImageLeft, Point(oP.x, oP.y), 10, Scalar(0), 3);
            //Done decoration

            //The thing we were looking for... how can we forget to print this? :P
            cout << "seperation = " << sep << "\n";
        }

        //Show the windows
        cv::namedWindow("RIGHT");
        cv::namedWindow("thresh");
        cv::namedWindow("LEFT");
        imshow("LEFT", displayImageLeft);
        imshow("RIGHT", displayImageRight);
        imshow("thresh", motionImageRight);
        //End of code for showing windows

        //The loop terminating condition
        if (waitKey(27) >= 0) break;
    }

    //Mission Successful!! :D :)
    return 0;
}



//Compares img1 with imgBack and stores the result in binary image imgOut
void CompareWithBackground(Image &img1, Image &imgBack, Image &imgOut)
{

    int nr = img1.rows; // number of lines
    int nc = img1.cols ; // number of columns

    for (int i = 0; i < nr; i++)
    {

        for (int j = 0; j < nc; j++)
        {
            if ((abs((int)img1.at<Vec3b>(i, j)[2] - (int)imgBack.at<Vec3b>(i, j)[2]) >= 35))
            {
                imgOut.at<uchar>(i, j) = 255;
            }
            else
            {
                imgOut.at<uchar>(i, j) = 0;
            }
        }
    }
}

//Reads and wastes N frames from the two cameras
void WasteNFrames(int N)
{
    for (int a = 0; a < N; ++a)
    {
        while (rightCamera->Get() == 0) usleep(1);

        while (leftCamera->Get() == 0) usleep(1);
    }
    frameNumber+= N;
}

//Gets the topmost available shot from cam
Mat GetNextCameraShot(Camera* cam)
{
    while (cam->Get() == 0) usleep(1);
    cam->toIplImage(tempImageV4L);	 // translate the image to IplImage
    return Mat(tempImageV4L, true);
}

//Draws a black rectangle having specs x,y,w,h in image img
void DrawRect(Image &img, int x, int y, int w, int h)
{
    cv::line(img, Point(x, y), Point(x, y + h), Scalar(0));
    cv::line(img, Point(x, y), Point(x + w, y),Scalar(0));
    cv::line(img, Point(x, y + h), Point(x + w, y + h), Scalar(0));
    cv::line(img, Point(x + w, y), Point(x + w, y + h), Scalar(0));
}
