/*
 * superTracker.cpp
 *
 *  Created on: 18 mai 2017
 *      Author: tux
 */

//--entete--//
//----------//
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp> //--OpenCV_Contrib_Header

//--espace de noms--//
//------------------//
using namespace std;
using namespace cv;

//--prototype de fonction--//
//-------------------------//
const string Date();
const string Heure();
void Square(Mat&, Point, const Scalar&, int, int, int);
void TriangleDown(Mat&, Point, const Scalar&, int, int, int);
void TriangleLeft(Mat&, Point, const Scalar&, int, int, int);

int main()
{
	Mat frame;

	Rect2d roi;
	Point centroid;

	//--constante de dessin--//
	//-----------------------//
	int epais = 40;
	int delta = 40;
	int scale = 10;
	int offset = 20;

	//--definition couleurs--//
	//-----------------------//
	Scalar blanc = Scalar(255,255,255);
	Scalar vert = Scalar(0,255,0);
	Scalar rouge = Scalar(0,0,255);
	Scalar bleu = Scalar(255,0,0);
	Scalar noir = Scalar(0,0,0);

	//--points reticule principal fixe--//
	//----------------------------------//
	Point center;
	Point pt1, pt2;
	Point pt3, pt4;
	Point pt5, pt6;
	Point pt7, pt8;

	//--definition du tracker--//
	//-------------------------//
	Ptr<Tracker> tracker = Tracker::create("KCF");

	VideoCapture cap(0);

	cap >> frame;

	//--selection du ROI manuel--//
	//---------------------------//
	roi = selectROI(frame);
	tracker->init(frame, roi);

	while(true)
	{
		cap >> frame;

		//--update tracker pour chaque frame--//
		//------------------------------------//
		tracker->update(frame, roi);

		//--rectangle ROI de detection--//
		//------------------------------//
		Rect2d zoneLOCK = Rect2d(frame.cols/2-30, frame.rows/2-30, 60,60);
		Rect2d zoneTRACK = Rect2d(frame.cols/2-120, frame.rows/2-120, 240, 240);

		//--reticule central fixe--//
		//-------------------------//
		Square(frame, Point(frame.cols/2, frame.rows/2), vert, 60, 1, 8);

		center = Point(frame.cols/2, frame.rows/2);
		circle(frame, center, 1, vert, 2);

		pt1 = Point(0, frame.rows/2);
		pt2 = Point(frame.cols/2 - offset, frame.rows/2);
		line(frame, pt1, pt2, vert, 1);

		pt3 = Point(frame.cols/2 + offset, frame.rows/2);
		pt4 = Point(frame.cols, frame.rows/2);
		line(frame, pt3, pt4, vert, 1);

		pt5 = Point(frame.cols/2, 0);
		pt6 = Point(frame.cols/2, frame.rows/2 - offset);
		line(frame, pt5, pt6, vert, 1);

		pt7 = Point(frame.cols/2, frame.rows/2 + offset);
		pt8 = Point(frame.cols/2, frame.rows);
		line(frame, pt7, pt8, vert, 1);

		//--calcul centroid target--//
		//--------------------------//
		centroid = Point(roi.x + (roi.width/2), roi.y + (roi.height/2));

		//--affichage target track--//
		//--------------------------//
		rectangle(frame, roi, rouge,1);
		circle(frame, centroid, 2, rouge, 2);

		//--dessin brodure fenetre--//
		//--------------------------//
		rectangle(frame, Point(0,0), Point(frame.cols, epais), noir, -1);
		rectangle(frame, Point(0, frame.rows), Point(frame.cols, frame.rows-epais), noir, -1);
		rectangle(frame, Point(0,0), Point(epais+20, frame.rows), noir, -1);
		rectangle(frame, Point(frame.cols-epais, 0), Point(frame.cols, frame.rows), noir, -1);

		//--dessin echelle gauche et bas--//
		//--------------------------------//
		for(int i = 0;i<frame.rows; i+=delta)
		{
			for(int j=0; j<frame.cols; j+=delta)
			{
				line(frame, Point(0, i), Point(scale, i), blanc,1);
				line(frame, Point(j, frame.rows), Point(j, frame.rows-scale), blanc,1);
			}
		}

		//--affiche triangles gauche et bas--//
		//-----------------------------------//
		TriangleDown(frame, Point(centroid.x, centroid.y +(frame.rows-centroid.y)-offset), blanc, 20, 1, 8);
		TriangleLeft(frame, Point(centroid.x-(centroid.x-offset), centroid.y), blanc, 20, 1, 8);

		//--int to string--//
		//-----------------//
		stringstream x, y;
		x << centroid.x;
		y << centroid.y;

		//--affichage coordonnes du centroid sur triangles--//
		//--------------------------------------------------//
		putText(frame, x.str(), Point(centroid.x-10, centroid.y +(frame.rows-centroid.y)-offset-5),1,1, blanc, 1);
		putText(frame, y.str(), Point(centroid.x-(centroid.x-offset-5), centroid.y+5),1,1, blanc, 1);

		//--detection target dans roi zoneTrack--//
		//---------------------------------------//
		if(centroid.inside(zoneTRACK))
		{
			Square(frame, Point(frame.cols/2, frame.rows/2), vert, 240, 1, 8);
			circle(frame, center, 1, rouge, 2);
			line(frame, center, centroid, blanc, 2);
		}

		//--detection target dans roi zoneLock--//
		//--------------------------------------//
		if(centroid.inside(zoneLOCK))
		{
			Square(frame, Point(frame.cols/2, frame.rows/2), rouge, 60, 3, 8);
			circle(frame, center, 1, rouge, 2);
			line(frame, center, centroid, bleu, 2);
		}

		//--affichage target dans ROI zoneTrack--//
		//---------------------------------------//
		if(centroid.inside(zoneTRACK))
			putText(frame, "ON",Point(430,25),1,1, blanc,2);
		else
			putText(frame, "OFF",Point(430,25),1,1, blanc,1);

		//--affichage target dans ROI zoneLock--//
		//--------------------------------------//
		if(centroid.inside(zoneLOCK))
			putText(frame, "ON",Point(570,25),1,1,blanc,2);
		else
			putText(frame, "OFF",Point(570,25),1,1,blanc,1);

		//--affichage des fonction horodatage et ROI--//
		//--------------------------------------------//
		putText(frame, Date(),Point(60,25),1,1,blanc,1);
		putText(frame, Heure(),Point(200,25),1,1,blanc,1);
		putText(frame, "M.TRACK: ", Point(350,25),1,1,blanc,1);
		putText(frame, "M.LOCK: ", Point(500,25),1,1,blanc,1);

		//--affichage frame--//
		//-------------------//
		imshow("Frame", frame);
		waitKey(32);
	}
	destroyAllWindows();
	return 0;
}

//--fonction date--//
//-----------------//
const string Date()
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);

    strftime(buf, sizeof(buf), "%d/%m/%Y", &tstruct);

    return buf;
}

//--fonction heure--//
//------------------//
const string Heure()
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);

    strftime(buf, sizeof(buf), "%X", &tstruct);

    return buf;
}

//--fonction dessin carre--//
//-------------------------//
void Square(Mat& img, Point position, const Scalar& color, int markerSize, int thickness, int line_type)
{
	line(img, Point(position.x-(markerSize/2), position.y-(markerSize/2)), Point(position.x+(markerSize/2), position.y-(markerSize/2)), color, thickness, line_type);
	line(img, Point(position.x+(markerSize/2), position.y-(markerSize/2)), Point(position.x+(markerSize/2), position.y+(markerSize/2)), color, thickness, line_type);
	line(img, Point(position.x+(markerSize/2), position.y+(markerSize/2)), Point(position.x-(markerSize/2), position.y+(markerSize/2)), color, thickness, line_type);
	line(img, Point(position.x-(markerSize/2), position.y+(markerSize/2)), Point(position.x-(markerSize/2), position.y-(markerSize/2)), color, thickness, line_type);
}

//--fonction dessin triangle vers le bas--//
//----------------------------------------//
void TriangleDown(Mat& img, Point position, const Scalar& color, int markerSize, int thickness, int line_type)
{
	line(img, Point(position.x+(markerSize/2), position.y), Point(position.x, position.y+(markerSize/2)), color, thickness, line_type);
	line(img, Point(position.x, position.y+(markerSize/2)), Point(position.x-(markerSize/2), position.y), color, thickness, line_type);
	line(img, Point(position.x-(markerSize/2), position.y), Point(position.x+(markerSize/2), position.y), color, thickness, line_type);
}

//--fonction dessin triangle sur la gauche--//
//------------------------------------------//
void TriangleLeft(Mat& img, Point position, const Scalar& color, int markerSize, int thickness, int line_type)
{
	line(img, Point(position.x, position.y+(markerSize/2)), Point(position.x-(markerSize/2), position.y), color, thickness, line_type);
	line(img, Point(position.x-(markerSize/2), position.y), Point(position.x, position.y-(markerSize/2)), color, thickness, line_type);
	line(img, Point(position.x, position.y-(markerSize/2)), Point(position.x, position.y+(markerSize/2)), color, thickness, line_type);
}

