/*
   RV01: Affine Transformation
   
   Autor: .....................
   HAW-University of Applied Sciences - Hamburg,Germany

 */ 

#include "ltiObject.h"

#include <iostream>
#include <fstream>
#include <math.h>
#include <string>
#include <list>
#include <stdio.h>

#include "RV01.h"
#include "ltiTimer.h"
#include "ltiBMPFunctor.h"
#include "ltiViewer.h"
#include "ltiSplitImg.h"
#include "gtk.h"
#include "ltiGtkServer.h"
#include "RV01.h"

using std::cout;
using std::endl;


namespace lti {

  void RV01::operator()(int argc,char *argv[]) {

	/*********************************************/
	/**** has always to be started (AMei)     ****/
    /**** if program is without gtk-Widgets   ****/
	/*********************************************/

	gtkServer server;
    server.start();

	/******************************************/
	/**** instantiation of used components ****/
	/******************************************/

	/*---------------------*/
	/* loaders and viewers */
    /*---------------------*/
    loadBMP loader;                         // object for loading .bmp-images

    viewer view("Original");                // object for visualizing images
	viewer viewTransformed("Transformed");
	viewer viewfourPointTrans("four points transformed");

	/*---------------------*/
	/* images & channels   */
    /*---------------------*/
    image img;                              // normalized (color) image
	channel8  src;  // source picture       // 8-bit-image (source)
    channel8  dst;  // destination picture  // 8-bit-image (source) 
	channel8  fourPdst;

	/*-----------------------------*/
	/* Image processing components */
	/*-----------------------------*/

    // object to split image into hue, saturation and intensity
	// hue        = Farbton
	// saturation = Farbsättigung
	// intensity  = Intensität (Grauwert)
    splitImageToHSI splitter;



	/******************************************/
	/*    the program                         */
	/******************************************/

    // load the source image
    loader.load("Kalib.bmp",img);
    
    // extract the intensity channel only
    splitter.getIntensity(img,src);

    // determine image size
    const int rowSize    = src.rows();
    const int columnSize = src.columns();


    // set destination size to source size 
    dst.resize(rowSize,columnSize,0,false,true);
	fourPdst.resize(rowSize,columnSize,0,false,true);


    // copy source pixels to destination pixels
	for(int y=0; y<rowSize; y++)
		for(int x=0; x<columnSize; x++){
			dst[y][x] = src[y][x];
			fourPdst[y][x] = src[y][x];
		}
	affineTransformation(src,dst);
	fourPointTrans(src,fourPdst);

	// view pictures
    view.show(src);
    viewTransformed.show(dst);
	viewfourPointTrans.show(fourPdst);

    getchar();

  }

  bool RV01::checkBorder(int coordinate, int maxsize){
	if(coordinate>=0 && coordinate<maxsize-1){
		  return true;
	}
	return false;	
  }

  int RV01::nearestNeighborInterpol(const double dX, const double dY, const channel8& img){
	//int interpVal;
	int rowSize    = img.rows();
	int columnSize = img.columns();

	int x = round(dX);
	int y = round(dY);
	if(checkBorder(x,columnSize) && checkBorder(y,rowSize)){
		return img[y][x];
	}
	return 0;
  }

  int RV01::bilinInterpol(const double dX, const double dY, const channel8& img){
	  int x0=(int)dX;
	  int x1=(int)dX+1;
	  int y0=(int)dY;
	  int y1=(int)dY+1;
	  int rowSize    = img.rows();
	  int columnSize = img.columns();
	  double interpVal=0;
	  if(checkBorder(x0,columnSize) && checkBorder(x1,columnSize) && checkBorder(y0,rowSize) && checkBorder(y1,rowSize)){
		double interpX1=img[y0][x0] + (dX -x0) * (img[y0][x1] - img[y0][x0]);
		double interpX2=img[y1][x0] + (dX -x0) * (img[y1][x1] - img[y1][x0]);
		interpVal=interpX1+(dY-y0)*(interpX2-interpX1);
	  }
	return (int)interpVal;
  }
  //void RV01::affineTransformation(int (*interpPtr)(const double, const double, const channel8&), const channel8& src, channel8& dst){
	void RV01::affineTransformation(const channel8& src, channel8& dst){
	double a0=160.7917;
	double a1=0.4417;
	double a2=-0.13;
	double b0=218.7417;
	double b1=0.0017;
	double b2=0.4925;
	for(int y=0; y<dst.rows();y++){
		for(int x=0; x<dst.columns(); x++){
			double x_ = x*a1+y*a2+a0;
			double y_ = y*b1+y*b2+b0;
			dst[y][x]=bilinInterpol(x_,y_,src);
			//dst[y][x]=nearestNeighborInterpol(x_,y_,src);
		}
	}
}
	void RV01::fourPointTrans(const channel8& src, channel8& dst){
	static const int px1 = 230;
	static const int py1 = 128;

	static const int px2 = 546;
	static const int py2 = 123;

	static const int px3 = 593;
	static const int py3 = 416;

	static const int px4 = 168;
	static const int py4 = 413;


	for(int y=0; y<dst.rows();y++){
		for(int x=0; x<dst.columns(); x++){

			double xNorm = (double) x / (double) (dst.columns() - 1);
			double yNorm = (double) y / (double) (dst.rows() - 1);

			double phi1 = (1-xNorm) * (1-yNorm);
			double phi2 = xNorm *(1 -yNorm);
			double phi3 = xNorm *yNorm;
			double phi4 = (1-xNorm) * yNorm;

			double x_ = px1 * phi1 + px2 * phi2 + px3 * phi3 + px4 * phi4;
			double y_ = py1 * phi1 + py2 * phi2 + py3 * phi3 + py4 * phi4;

			//dst[y][x]=bilinInterpol(x_,y_,src);
			dst[y][x]=nearestNeighborInterpol(x_,y_,src);
		}
	}
}


};