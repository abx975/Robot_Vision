/*
   RV02: Median und Sobel
   
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

#include "RV02.h"
#include "ltiTimer.h"
#include "ltiBMPFunctor.h"
#include "ltiViewer.h"
#include "ltiSplitImg.h"
#include "gtk.h"
#include "ltiGtkServer.h"

#define ARR_SIZE 256

using std::cout;
using std::endl;


namespace lti {

  void RV02::operator()(int argc,char *argv[]) {

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
	viewer viewTransformed("Median- Filtered");
	viewer viewGrad("GradientPic");
	viewer viewDirect("DirectionPic");

	/*---------------------*/
	/* images & channels   */
    /*---------------------*/
    image img;                              // normalized (color) image
	channel8  src;  // source picture       // 8-bit-image (source)
    channel8  dst;  // destination picture  // 8-bit-image (source) 
	channel8  gradPic;
	channel8  directPic;

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
    //loader.load("shaft.bmp",img);
	loader.load("Circle.bmp",img);
    
    // extract the intensity channel only
    splitter.getIntensity(img,src);

    // determine image size
    const int rowSize    = src.rows();
    const int columnSize = src.columns();


    // set destination size to source size 
    dst.resize(rowSize,columnSize,0,false,true);
	gradPic.resize(rowSize,columnSize,0,false,true);
	directPic.resize(rowSize,columnSize,0,false,true);

    
    Median(src,dst,9,9);
	Sobel(src,gradPic,directPic);


	// view pictures
    view.show(src);
	viewTransformed.show(dst);
    viewGrad.show(gradPic);
	viewDirect.show(directPic);
    getchar();

  }



  

  /***************************************************************************/
  /* Function definition: ----- Median-operator----                          */
  /***************************************************************************/
  void RV02::Median(  const	     channel8& sPic, 	// source picture 
								 channel8& dPic, 	// destination picture
	                   const int MaskSizeX,		    // mask size in x-direction
					   const int MaskSizeY		 	// mask size in y-direction
					   )
  {
	const int PicSizeY = sPic.rows();
	const int PicSizeX = sPic.columns();

	int x,y,mx,my;
	
	for(y=0; y<PicSizeY; y++){
		for(x=0; x<PicSizeX; x++){
			//dPic[y][x] = 256 - sPic[y][x];

			//holding number of times grayvalue occuring in source picture
			int histogram[ARR_SIZE]={0};	
			int yMin = y-MaskSizeY/2;
			int yMax = y+MaskSizeY/2;
			int xMin = x-MaskSizeX/2;
			int xMax = x+MaskSizeX/2;

			//handle Index out of bounds
			if(yMin<0){ 
				yMin = 0;
			}
			if(yMax>PicSizeY-1){ 
				yMax=PicSizeY-1;
			}
			if(xMin<0){ 
				xMin = 0;
			}
			if(xMax>PicSizeX-1){
				xMax=PicSizeX-1;
			}


			//create histogram
			for(int yIdx=yMin; yIdx<yMax; yIdx++){
				for(int xIdx=xMin; xIdx<xMax; xIdx++){
					histogram[sPic[yIdx][xIdx]] +=1;
				}
			}

			//evaluate histogram
			int accu=0;
			int idx=0;
			while(accu<(MaskSizeX*MaskSizeY+1)/2 && idx<ARR_SIZE){
				accu+=histogram[idx];

				idx++;
			}
			dPic[y][x] = idx-1;
		}
	}
  }


  /***************************************************************************/
  /* Function definition: ----- Sobel-operator----                          */
  /***************************************************************************/
  void RV02::Sobel(  const	     channel8& sPic, 				// source picture 
								 channel8& GradientPic, 		// gradient picture
								 channel8& DirectionPic			// direction picture
					   )
  {
	const int PicSizeY = sPic.rows();
	const int PicSizeX = sPic.columns();

	int x,y,mx,my;
	
	int xMask[3][3]={{-1,0,1},{-2,0,2},{-1,0,1}};
	int yMask[3][3]={{-1,-2,-1},{0,0,0},{1,2,1}};
	

	for(y=1; y<PicSizeY-1; y++){
		for(x=1; x<PicSizeX-1; x++){
		
			double Gx=0;
			double Gy=0;
			for(int i=y-1; i<=y+1; i++){
				for(int j=x-1; j<=x+1; j++){			
						Gy += (sPic[i][j]*yMask[i-y+1][j-x+1]);
						Gx += (sPic[i][j]*xMask[i-y+1][j-x+1]);		
				}
			}
			Gy = Gy/4;
			Gx = Gx/4;

			// gradient
			GradientPic[y][x]= (int)sqrt(Gx*Gx+Gy*Gy);

			// direction

			if(Gx == 0 && Gy == 0){
				DirectionPic[y][x] = 0;
			} else {
				double alpha = atan2((double)Gy,Gx);
					alpha *= 180 / 3.1415926535;
				if(alpha < -22,5){
					alpha += 360;
				}
				DirectionPic[y][x] = (ubyte)((alpha + 22.5) / 45);
			}
			//std::cout << (int) (GradientPic[y][x]) << std::endl;  
		}
	}
  }
};
