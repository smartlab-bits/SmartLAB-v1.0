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

    class CandidatePixel
    {
    public:
        PixPoint point;
        long int totalColorDiff;
        long int noPixelsMatchedWith;
        double distanceFromOriginalCoord;
        double score;

    public :
        CandidatePixel();
        CandidatePixel(PixPoint p);
        CandidatePixel(PixPoint p, long int t, long int n, double d, double s);
        CandidatePixel(CandidatePixel &cp);
        void computeScore();
        static bool compare(CandidatePixel* cp1, CandidatePixel* cp2);
    };

    typedef vector<CandidatePixel*> CPlist;

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
        static const MatchingLevel MAX_MATCHING_LEVEL;
        static const double THRESHHOLD_SCORE;
    };


	double distsq(PixPoint p1, PixPoint p2);
}

#endif // STEREOIMAGING_H
