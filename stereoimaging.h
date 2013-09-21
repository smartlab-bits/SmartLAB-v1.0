#ifndef STEREOIMAGING_H
#define STEREOIMAGING_H
#include <list>
#include <vector>
#include <algorithm>
#include <opencv/cv.h>
#include <exception>
#include <math.h>

using namespace std;
using namespace cv;
typedef Mat Image;

namespace StereoImag
{
    typedef struct
    {
    	int x;
    	int y;
    } Vector2;

    typedef Vector2 Size;
    typedef Vector2 PixPoint;
    typedef Vector2 Pixel;
    typedef Vector2 MatchingLevel;


    typedef struct
    {
        PixPoint location;
        Size size;
    } Rectangle;


    /* there are two images LEFT and RIGHT
       Both images have set of corrensponding pixels
       Candidate Pixel is a pixel point in LEFT image that is tested
       for correspondence to a specific pixel point in the RIGHT image
    */
    class CandidatePixel
    {
    public:
        PixPoint point;    		 // x and y coordinates
        long int totalColorDiff;         // aggregate of diffence in the pixel value of surrounding pixels
        long int noPixelsMatchedWith;    // number of surrounding pixels considered to calculate differences and score
        double distanceFromOriginalCoord;   //unused
        double score;              // tells the level of similarity of candidate pixel to a specific pixel

    public :
        CandidatePixel();
        CandidatePixel(PixPoint p);
        CandidatePixel(PixPoint p, long int t, long int n, double d, double s);
        CandidatePixel(CandidatePixel &cp);
        void computeScore();             
        static bool compare(CandidatePixel* cp1, CandidatePixel* cp2);
    };

    typedef vector<CandidatePixel*> CPlist;


    /*
    class that implements methods to find
    pixels corresponding to a specific pixel in other image and 
    rank them by similarity
    */
    class StereoImaging
    {
    public:
        StereoImaging();
    };

    class PointCorresponder
    {
        static CPlist qualifyPointsAbsolute(Image& img1, Image& img2, PixPoint, Rectangle, MatchingLevel, Image mask);
        static CPlist qualifyPointsRelative(Image& img1, Image& img2, PixPoint, CPlist, MatchingLevel);
        static void testCandidatePixel(Image&, Image&,PixPoint inPoint, CandidatePixel*, MatchingLevel);

    public :
        static PixPoint correspondPoint(Image& img1, Image& img2, PixPoint inPoint, Rectangle rect, Image mask);
        static PixPoint correspondPointv2(Image& img1, Image& img2, PixPoint inPoint, Rectangle rect, StereoImag::Size dp);
    };


    class Constants
    {
    public:
        static const MatchingLevel MAX_MATCHING_LEVEL;  //size of rectangle around the specific pixel point coordinate to be scanned in other image
        static const double THRESHHOLD_SCORE;   //thresholded/;imiting maximum difference between pixel values for qualifyPointsAbsolute()
    };


	double distsq(PixPoint p1, PixPoint p2);
}

#endif // STEREOIMAGING_H
