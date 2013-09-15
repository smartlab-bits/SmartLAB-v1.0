#include "stereoimaging.h"

using namespace StereoImag;

StereoImaging::StereoImaging()
{
}

const MatchingLevel Constants::MAX_MATCHING_LEVEL = {11, 11};
const double Constants::THRESHHOLD_SCORE = 300;



double distsq(PixPoint p1, PixPoint p2)
{
    return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
}





CandidatePixel::CandidatePixel()
{
    point.x = 0; point.y = 0;
    totalColorDiff = 0;
    noPixelsMatchedWith = 0;
    distanceFromOriginalCoord = 0;
    score = 0;
}

CandidatePixel::CandidatePixel(CandidatePixel &cp)
{
    point.x = cp.point.x; point.y = cp.point.y;
    totalColorDiff = cp.totalColorDiff;
    noPixelsMatchedWith = cp.noPixelsMatchedWith;
    distanceFromOriginalCoord = cp.distanceFromOriginalCoord;
    score = cp.score;
}

CandidatePixel::CandidatePixel(PixPoint p)
{
    point.x = p.x; point.y = p.y;
    totalColorDiff = 0;
    noPixelsMatchedWith = 0;
    distanceFromOriginalCoord = 0;
    score = 0;
}

CandidatePixel::CandidatePixel(PixPoint p,long int t,long int n,double d,double s)
{
    point.x = p.x; point.y = p.y;
    totalColorDiff = t;
    noPixelsMatchedWith = n;
    distanceFromOriginalCoord = d;
    score = s;
}

void CandidatePixel::computeScore()
{
    score = (double)totalColorDiff / noPixelsMatchedWith;
}

bool CandidatePixel::compare(CandidatePixel* cp1, CandidatePixel* cp2)
{
    if (cp1->score < cp2->score) return true;
    else return false;
}





void PointCorresponder::testCandidatePixel(Image &img1, Image &img2, PixPoint inPoint, CandidatePixel *cp, MatchingLevel ml)
{
    int startx = -ml.x/2;
    int endx   =  ml.x/2;
    int starty = -ml.y/2;
    int endy   =  ml.y/2;
    //cp->totalColorDiff=0;
    //cp->noPixelsMatchedWith=0;

    int x, y = starty;
    for (y = starty; y <= endy; ++y)
    {
        for (x = startx; x <= endx; ++x)
        {   
        	if (!(x == startx || x == endx || y == starty || y == endy)) continue;

            int srcx = inPoint.x + x;
            int srcy = inPoint.y + y;
            int tox = cp->point.x + x;
            int toy = cp->point.y + y;

            if (srcx < 0 || srcx >= img1.cols || srcy < 0 || srcy >= img1.rows || tox < 0 || tox >= img2.cols || toy < 0 || toy >= img2.rows) continue;
            
            try 
            {
                cp->totalColorDiff+= abs(img1.at<Vec3b>(srcy, srcx)[0] - img2.at<Vec3b>(toy, tox)[0]) + abs(img1.at<Vec3b>(srcy, srcx)[1]-img2.at<Vec3b>(toy, tox)[1]) + abs(img1.at<Vec3b>(srcy, srcx)[2] - img2.at<Vec3b>(toy, tox)[2]);
                cp->noPixelsMatchedWith++;

            } catch (int e) { }

        }
    }
     
	cp->computeScore();
}



CPlist PointCorresponder::qualifyPointsAbsolute(Image &img1, Image &img2, PixPoint inPoint, Rectangle rect, MatchingLevel ml, Image mask)
{
    CPlist cpixelList;
    
    int endx = rect.location.x + rect.size.x;
    int endy = rect.location.y + rect.size.y;
    
    for (int y = rect.location.y; y < endy; ++y)
    {
        for (int x = rect.location.x; x < endx; ++x)
        {   
        	try
        	{
                if (!mask.empty())
                {
                	if (mask.at<uchar>(y, x) < 200) continue;
                }
                    
		        PixPoint p; p.x = x; p.y = y;
		        CandidatePixel* cp = new CandidatePixel(p);
		        testCandidatePixel(img1, img2, inPoint, cp, ml);
		        
		        if (cp->score < Constants::THRESHHOLD_SCORE) cpixelList.push_back(cp);
            }
            catch(int e) { }
        }
    }
    
    return cpixelList;
}

CPlist PointCorresponder::qualifyPointsRelative(Image &img1, Image &img2, PixPoint inPoint, CPlist cpixelList1, MatchingLevel ml)
{
    CPlist::iterator it = cpixelList1.begin();
    //CPlist cpixelList2;

    for (int i = 0; i < cpixelList1.size(); ++i)
    {
        //cpixelList1[i];
        testCandidatePixel(img1, img2, inPoint, (CandidatePixel*)cpixelList1[i], ml);
    }

    sort(cpixelList1.begin(), cpixelList1.end(), CandidatePixel::compare);
    
    int si = cpixelList1.size()/2;
    for (int j = si; j != cpixelList1.size(); ++j)
    {
        delete cpixelList1[j];
    }
    cpixelList1.resize(si);
    return cpixelList1;
}

PixPoint PointCorresponder::correspondPoint(Image& img1, Image& img2, PixPoint inPoint, Rectangle rect, Image mask)
{
    CPlist cpixelList;
    MatchingLevel ml; ml.x = 1; ml.y = 1;
    
    cpixelList = qualifyPointsAbsolute(img1, img2, inPoint, rect, ml, mask);
    
    if (cpixelList.size() < 1) return inPoint;
    
    /*ml.x+=2;
    ml.y+=2;
    cpixelList=qualifyPointsAbsolute(img1,img2,inPoint,rect,ml);*/
    
    ml.x+= 2;
    ml.y+= 2;
    while (cpixelList.size() != 1 && ml.x <= Constants::MAX_MATCHING_LEVEL.x && ml.y <= Constants::MAX_MATCHING_LEVEL.y)
    {
        cpixelList = qualifyPointsRelative(img1, img2, inPoint, cpixelList, ml);
        ml.x+= 2;
        ml.y+= 2;
    }
    
    sort(cpixelList.begin(), cpixelList.end(), CandidatePixel::compare);
    
    CandidatePixel* target = (CandidatePixel*)(cpixelList[0]);
    PixPoint p;
    p.x = target->point.x;
    p.y = target->point.y;
    
    for (int i = 0; i < cpixelList.size(); ++i) delete cpixelList[i];

    return p;
}




bool is_valid(PixPoint p, Image& img)
{
    return p.x >= 0 &&  p.x < img.cols && p.y >= 0 && p.y < img.rows;
}

