 /*
	RV03:  Image Filtering by means of Fourier transform
	Author: Prof.Dr.Andreas Meisel
	        HAW- University of Applied Sciences - Hamburg,Germany
 */ 
#define  _USE_MATH_DEFINES
#include "ltiObject.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include <string>
#include <list>
#include <stdio.h>
#include "ltiImage.h"
#include "ltiBMPFunctor.h"
#include "ltiViewer.h"
#include "ltiSplitImg.h"
#include "gtk.h"
#include "ltiGtkServer.h"
#include "ltiRealFFT.h"
#include "ltiRealInvFFT.h"
#include "ltiHistogramEqualization.h"
#include "RV03.h"



namespace lti {

/********* operator definition: () **********************************/

  void RV03::operator()(int argc,char *argv[]) {
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
    loadBMP loader;         // class for 

    viewer view("Original");
	viewer viewTransformed("absolute value of DFT");
	viewer viewTransformed2("Rekonstruiert");

	/*---------------------*/
	/* images & channels   */
    /*---------------------*/
    image img;

	channel  src;    // source picture
    channel  real;   // real  part of DFT
    channel  imag;   // imag. part of DFT
	channel  real2;  // real  part of DFT (filtered)
    channel  imag2;  // imag. part of DFT (filtered)
    channel  creal;  // real  part of DFT (center adjusted)
    channel  cimag;  // imag. part of DFT (center adjusted)
	channel  creal2; // real  part of DFT (center adusted, filtered)
    channel  cimag2; // imag. part of DFT (center adusted, filtered)

	channel  absv;   // absolute value image
	channel  filt;   // filtered image

	/*-----------------------------*/
	/* Image processing components */
	/*-----------------------------*/
    // functor to split image in the chromaticity color space
    splitImageToHSI        splitter;
	realFFT                fft2d;
	realInvFFT             ifft2d;
	histogramEqualization  histogramEqualizer;


	/*-----------------------------*/
	/* Variable declarations       */
	/*-----------------------------*/
    int x,y,cx,cy;




	/*******************************************************************/
	/*    the program                                                  */
	/*******************************************************************/

	enum Picture	{	PIC_BOAT,			
						PIC_BRAIN			
					};

    /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
	/*   Select Picture                    */
	/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

    Picture loadPic=PIC_BRAIN;
 	

    // load the source image
    switch(loadPic){
	  case PIC_BOAT:  loader.load("boat.bmp",img);  break;
	  case PIC_BRAIN: loader.load("brain.bmp",img); break;
	  default:        loader.load("boat.bmp",img);  break;
	}
   
    
    // extract the intensity channel only
    splitter.getIntensity(img,src);

    // determine image size
    const int rowSize    = src.rows();
    const int columnSize = src.columns();

    // set destination size to source size 
    real.resize(rowSize,columnSize,0,false,true);
    imag.resize(rowSize,columnSize,0,false,true);
	real2.resize(rowSize,columnSize,0,false,true);
    imag2.resize(rowSize,columnSize,0,false,true);
    creal.resize(rowSize,columnSize,0,false,true);
    cimag.resize(rowSize,columnSize,0,false,true);
	creal2.resize(rowSize,columnSize,0,false,true);
    cimag2.resize(rowSize,columnSize,0,false,true);

    filt.resize(rowSize,columnSize,0,false,true);
    absv.resize(rowSize,columnSize,0,false,true);




    /*******************************************************/
	/* Create input picture/pattern                        */
	/*******************************************************/

	enum Pattern	{	PAT_IMAGE,			
						PAT_CIRCLE,			
						PAT_RECT, 
						PAT_LAMBDA, 
						PAT_COSINE, 
						PAT_IMAGE_SINNOISE
					};


    /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
	/*     Parameters can be set here !     */
	/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

	Pattern    PatternType=PAT_IMAGE;
    int        StructSize=10;



	double CenterDist, AbsDist;

    /*------------------------------------------------------*/
	/*  PAT_CIRCLE                                          */
	/*                                                      */
	/*  1.0  if eucledian center distance is <= StructSize  */
	/*  0.0  else                                           */
	/*------------------------------------------------------*/
	if(PatternType==PAT_CIRCLE){
		for(y=0; y<rowSize;y++){
			for(x=0; x<columnSize;x++){
			    CenterDist=sqrt((x-columnSize/2)*(x-columnSize/2)+(y-rowSize/2)*(y-rowSize/2));
				AbsDist=abs(x-columnSize/2)>abs(y-rowSize/2)?abs(x-columnSize/2):abs(y-rowSize/2);
				if(CenterDist < StructSize){
					src[y][x]=1;
				} 
				else
				{
					src[y][x]=0;
				}
			}
		}
    }
    /*------------------------------------------------------*/
	/*  PAT_COSINE                                          */
	/*                                                      */
	/*  periodic cosine pattern                             */
	/*                                                      */
	/*------------------------------------------------------*/
    else if(PatternType==PAT_COSINE){
		for(y=0; y<rowSize;y++){
			for(x=0; x<columnSize;x++){
			    CenterDist=sqrt((x-columnSize/2)*(x-columnSize/2)+(y-rowSize/2)*(y-rowSize/2));
				AbsDist=abs(x-columnSize/2)>abs(y-rowSize/2)?abs(x-columnSize/2):abs(y-rowSize/2);
				src[y][x]=cos((double)CenterDist/(double)StructSize*2*3.1415926);

			}
		}
	}
    /*------------------------------------------------------*/
	/*  PAT_RECT                                            */
	/*                                                      */
	/*  1.0  if absolute center distance is <= StructSize   */
	/*  0.0  else                                           */
	/*------------------------------------------------------*/
    else if(PatternType==PAT_RECT){
		for(y=0; y<rowSize;y++){
			for(x=0; x<columnSize;x++){
			    CenterDist=sqrt((x-columnSize/2)*(x-columnSize/2)+(y-rowSize/2)*(y-rowSize/2));
				AbsDist=abs(x-columnSize/2)>abs(y-rowSize/2)?abs(x-columnSize/2):abs(y-rowSize/2);
				if(AbsDist < StructSize){
					src[y][x]=1;
				} 
				else
				{
					src[y][x]=0;
				}
			}
		}
	}
    /*-------------------------------------------------------*/
	/*  PAT_IMAGE_SINNOISE                                    */
	/*                                                       */
	/*  Picture of a boat, distorted by a sinusoidal pattern */
	/*                                                       */
	/*-------------------------------------------------------*/
    else if(PatternType==PAT_IMAGE_SINNOISE){
		for(y=0; y<rowSize;y++){
			for(x=0; x<columnSize;x++){
			    CenterDist=sqrt((x-columnSize/2)*(x-columnSize/2)+(y-rowSize/2)*(y-rowSize/2));
				AbsDist=abs(x-columnSize/2)>abs(y-rowSize/2)?abs(x-columnSize/2):abs(y-rowSize/2);
				src[y][x]=src[y][x]+0.10*cos(x/5.5*2*3.1415+y/35.5*2*3.1415)+0.10*cos(y/5.5*2*3.1415+x/35.5*2*3.1415);
			}
		}
	}


    
    /*********************************************************/
	/* Apply FFT on source picture and center result         */
	/*********************************************************/

    fft2d.apply(src,real,imag);
    CenterFrequencyDomain(real, imag, creal, cimag);



    /**************************************************************/
	/* Filter                                                     */
	/*                                                            */
	/* Add new filters here !                                     */
	/**************************************************************/


	enum filtertype	{	FILT_NOFILTER, 
						FILT_IDEAL_LOWPASS, 
						FILT_GAUSS_LOWPASS,
						FILT_IDEAL_HIGHPASS,
						FILT_GAUSS_HIGHPASS,
						FILT_HIGH_FREQUENCY_EMPHASIS,
						FILT_LAPLACIAN_OF_GAUSSIAN,
						FILT_SINNOISE
					};


    /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
	/*     Filter: type and parameters can be set here !     */
	/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

    filtertype Filter=FILT_HIGH_FREQUENCY_EMPHASIS;
	int     FilterSize=20;    // Parameter for ideal filters
	double  Sigma =5.0;      // Parameter for Gauss-based filters
	double  a=1.0, b=3.0;     // Parameter for high-frequency-emphasis
	
	if(Filter==FILT_NOFILTER){ 
		for(y=0; y<rowSize;y++){
			for(x=0; x<columnSize;x++){
				creal2[y][x]=creal[y][x];
				cimag2[y][x]=cimag[y][x];

            /* calculate absolute value image for visualisation  */
			//	absv[y][x]=100*sqrt(creal2[y][x]*creal2[y][x]+cimag2[y][x]*cimag2[y][x]);
			}
		}
    }
	else if (Filter==FILT_IDEAL_LOWPASS){
		for(y=0; y<rowSize;y++){
			for(x=0; x<columnSize;x++){
				double u = x - columnSize / 2;
				double v = y - rowSize / 2;
				double r = sqrt(u * u + v * v );
				double h = 0;
				if(r <= FilterSize){
					h = 1;
				}
				creal2[y][x]=creal[y][x] * h;
				cimag2[y][x]=cimag[y][x] * h;
			}
		}
	} 
	else if (Filter==FILT_GAUSS_LOWPASS){
		for(y=0; y<rowSize;y++){
			for(x=0; x<columnSize;x++){
				double u = x - columnSize / 2;
				double v = y - rowSize / 2;
				double w = sqrt(pow(u/columnSize,2) + pow(v/rowSize,2));
				double h = exp(-2*M_PI*M_PI*Sigma*Sigma*w*w);

				creal2[y][x]=creal[y][x] * h;
				cimag2[y][x]=cimag[y][x] * h;
			}
		}
    } 
	else if (Filter==FILT_IDEAL_HIGHPASS){
		for(y=0; y<rowSize;y++){
			for(x=0; x<columnSize;x++){
				double u = x - columnSize / 2;
				double v = y - rowSize / 2;
				double r = sqrt(u * u + v * v);
				double h = 1;
				if(r <= FilterSize){
					h = 0;
				}
				creal2[y][x]=creal[y][x] * h;
				cimag2[y][x]=cimag[y][x] * h;
			}
		}
    } 
	else if (Filter==FILT_GAUSS_HIGHPASS){
		for(y=0; y<rowSize;y++){
			for(x=0; x<columnSize;x++){
				double u = x - columnSize / 2;
				double v = y - rowSize / 2;
				double w = sqrt(pow(u/columnSize,2) + pow(v/rowSize,2));
				double h = exp(-2*M_PI*M_PI*Sigma*Sigma*w*w);

				h = 1 - h;
				creal2[y][x]=creal[y][x] * h;
				cimag2[y][x]=cimag[y][x] * h;
			}
		}
    } 
	else if (Filter==FILT_HIGH_FREQUENCY_EMPHASIS){
		for(y=0; y<rowSize;y++){
			for(x=0; x<columnSize;x++){


				double u = x - columnSize / 2;
				double v = y - rowSize / 2;
				double w = sqrt(pow(u/columnSize,2) + pow(v/rowSize,2));
				double h = exp(-2*M_PI*M_PI*Sigma*Sigma*w*w);

				double h_hp = 1 - h;

				h = a + b * h_hp;

				creal2[y][x]=creal[y][x] * h;
				cimag2[y][x]=cimag[y][x] * h;
			
			}
		}
    } 
	else if (Filter==FILT_LAPLACIAN_OF_GAUSSIAN){
		for(y=0; y<rowSize;y++){
			for(x=0; x<columnSize;x++){
				double u = x - columnSize / 2;
				double v = y - rowSize / 2;
				double r = sqrt(u * u + v * v );

				double w = sqrt(pow(u/columnSize,2) + pow(v/rowSize,2));
				double h_gau� = exp(-2*M_PI*M_PI*Sigma*Sigma*w*w);

				double h = -4*M_PI*M_PI*r*r*h_gau�;
				
				creal2[y][x]=creal[y][x] * h;
				cimag2[y][x]=cimag[y][x] * h;
			}
		}
    } 
	else if(Filter==FILT_SINNOISE){
		for(y=0; y<rowSize;y++){
			for(x=0; x<columnSize;x++){
				double h = 1;
				if((x > 225 && x < 260 && y > 155 && y < 170) || 
					(x > 154 && x < 170 && y > 220 && y < 265) || 
					(x > 344 && x < 357 && y > 253 && y < 284) || 
					(x > 244 && x < 289 && y > 340 && y < 357) ){

						h = 0;
				}

				creal2[y][x]=creal[y][x] * h;
				cimag2[y][x]=cimag[y][x] * h;
			}
		}
	}


	for(y=0; y<rowSize;y++){
		for(x=0; x<columnSize;x++){
		 /* calculate absolute value image for visualisation  */
			absv[y][x]=100*sqrt(creal2[y][x]*creal2[y][x]+cimag2[y][x]*cimag2[y][x]);
		}
	}
    /*********************************************************/
	/* Uncenter result and apply inverse FFT                 */
	/*********************************************************/

    UnCenterFrequencyDomain (creal2, cimag2, real2, imag2);
	ifft2d.apply(real2,imag2,filt);


	// view result pictures
    view.show(src);
    viewTransformed.show(absv);
	//histogramEqualizer.apply(filt);
    viewTransformed2.show(filt);

    getchar();

} // end of definition: operator()












/*****************************************************************************/
/*   Supporting functions                                                    */
/*****************************************************************************/

void RV03::CenterFrequencyDomain(const channel real, const channel imag, channel &creal, channel &cimag){

    int x,y,cx,cy;

    // determine image size
    const int rowSize    = real.rows();
    const int columnSize = real.columns();



	    // 4.Quadrant
	for(y=0, cy=rowSize/2; cy<rowSize;y++,cy++){
		for(x=0, cx=columnSize/2; cx<columnSize;x++,cx++){
            creal[cy][cx]=real[y][x];
            cimag[cy][cx]=imag[y][x];
			//absv[cy][cx]=100*sqrt(creal[cy][cx]*creal[cy][cx]+cimag[cy][cx]*cimag[cy][cx]);

        }
    }
    // 1.Quadrant
	for(y=rowSize/2, cy=0; y<rowSize;y++,cy++){
		for(x=0, cx=columnSize/2; cx<columnSize;x++,cx++){
            creal[cy][cx]=real[y][x];
            cimag[cy][cx]=imag[y][x];
			//absv[cy][cx]=100*sqrt(creal[cy][cx]*creal[cy][cx]+cimag[cy][cx]*cimag[cy][cx]);

        }
    }
    // 2.Quadrant
	for(y=rowSize/2, cy=0; y<rowSize;y++,cy++){
		for(x=columnSize/2, cx=0; x<columnSize;x++,cx++){
            creal[cy][cx]=real[y][x];
            cimag[cy][cx]=imag[y][x];
			//absv[cy][cx]=100*sqrt(creal[cy][cx]*creal[cy][cx]+cimag[cy][cx]*cimag[cy][cx]);

        }
    }
    // 3.Quadrant
	for(y=0, cy=rowSize/2; cy<rowSize;y++,cy++){
		for(x=columnSize/2, cx=0; x<columnSize;x++,cx++){
            creal[cy][cx]=real[y][x];
            cimag[cy][cx]=imag[y][x];
			//absv[cy][cx]=100*sqrt(creal[cy][cx]*creal[cy][cx]+cimag[cy][cx]*cimag[cy][cx]);

        }
    }
	return;
} // end of definition: CenterFrequencyDomaign() 





void RV03::UnCenterFrequencyDomain (const channel  creal,  const channel  cimag, 
		                                     channel &real,         channel &imag){

    int x,y,cx,cy;

    // determine image size
    const int rowSize    = creal.rows();
    const int columnSize = creal.columns();


    // 4.Quadrant
	for(y=0, cy=rowSize/2; cy<rowSize;y++,cy++){
		for(x=0, cx=columnSize/2; cx<columnSize;x++,cx++){
            real[cy][cx]=creal[y][x];
            imag[cy][cx]=cimag[y][x];
        }
    }
    // 1.Quadrant
	for(y=rowSize/2, cy=0; y<rowSize;y++,cy++){
		for(x=0, cx=columnSize/2; cx<columnSize;x++,cx++){
            real[cy][cx]=creal[y][x];
            imag[cy][cx]=cimag[y][x];
        }
    }
    // 2.Quadrant
	for(y=rowSize/2, cy=0; y<rowSize;y++,cy++){
		for(x=columnSize/2, cx=0; x<columnSize;x++,cx++){
            real[cy][cx]=creal[y][x];
            imag[cy][cx]=cimag[y][x];
        }
    }
    // 3.Quadrant
	for(y=0, cy=rowSize/2; cy<rowSize;y++,cy++){
		for(x=columnSize/2, cx=0; x<columnSize;x++,cx++){
            real[cy][cx]=creal[y][x];
            imag[cy][cx]=cimag[y][x];
        }
    }
	return;
} // end of definition: UnCenterFrequencyDomaign() 
 




};
