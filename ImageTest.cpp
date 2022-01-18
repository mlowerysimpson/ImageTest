//#include "cv.h"
//include "cvaux.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "../RemoteControlTest/ImageUtil.h"
#include <dirent.h>
#include <opencv2/opencv.hpp>
#include <wiringPi.h>
#include <errno.h>
#include <string> 
#include <math.h>

#include <iostream>


using namespace std;
using namespace cv;

bool FileExists(char *szPathName) {
	FILE *testFile = fopen(szPathName,"r");
	if (!testFile) return false;
	fclose(testFile);
	return true;
}

void ShowUsage() {
	printf("ImageTest\n");
	printf("Usage:\n");
	printf("ImageTest     --> without any arguments, the program captures a frame of video and outputs it to the file: output_img.jpg.\n");
	printf("ImageTest \"output_imagefilename\"  --> captures a frame of video and outputs it to \"output_imagefilename\".\n");
	printf("ImageTest -s \"output_imagefilename\" --> capture a static high resolution image and output the result to \"output_imagefilename\".\n");
	printf("ImageTest -cf \"folder_name\" [threshold]  --> convert all images in folder_name using Fast feature corner detection with the specified threshold (0 to 255).\n");
	printf("ImageTest -ce \"folder_name\" [lower_threshold] [upper_threshold] --> convert all images in folder_name using Canny edge detection with the specified lower and upper thresholds.\n");
	printf("ImageTest -h \"folder_name\"  --> find the horizon line in all images within folder_name, and output the images with horizon lines to the same folder.\n");
	printf("ImageTest -o \"folder_name\" --> find the horizon line and objects (if any) in all images within folder_name, and place the output images in the same folder.\n");
}

bool ConvertFastFeatureFile(char *szFilename, int nThreshold) {//convert the image file named szFileanme using FAST feature detection with the specified threshold value
	//the output image filename will be the same as szFilename, except that "_out" will be added,
	//eg: imageFile001.jpg --> imageFile001_out.jpg
	printf("converting: %s\n",szFilename);
	Mat img1 = imread(szFilename);//original image from video camera
	Mat img1_out;//output image with keypoints shown
	Mat img_gray1;//grayscale image 
	vector<KeyPoint> keypoints;
	//convert image to grayscale
	cvtColor(img1,img_gray1,COLOR_RGB2GRAY);
	cv::FAST(img_gray1,keypoints,nThreshold);
	cv::drawKeypoints(img1,keypoints,img1_out,Scalar(0,0,255));
	
	int nInputFilenameLength = strlen(szFilename);
	int nOutputFilenameLength = nInputFilenameLength+32;
	char *szOutputFilename = new char[nOutputFilenameLength];
	memset(szOutputFilename,0,nOutputFilenameLength);
	strncpy(szOutputFilename,szFilename,nInputFilenameLength-4);
	strcat(szOutputFilename,(char *)"_out.jpg");
	
	string sOutputFilename = string(szOutputFilename);
	imwrite(sOutputFilename,img1_out);
	delete []szOutputFilename;
	return true;
}

int ConvertFastFeatureDetect(char *szFolderName,int nThreshold) {//convert all image files in szFolderName using FAST feature detection with the specified threshold value
	DIR *dir;
	struct dirent *ent;
	char sExt[8];//used for checking the file extension of each file
	memset(sExt,0,8);
	if ((dir = opendir (szFolderName)) != NULL) {
		/* convert all the jpg files within directory */
		while ((ent = readdir (dir)) != NULL) {
			char *szFilename = ent->d_name;
			int nFilenamelength = strlen(szFilename);
			strcpy(sExt,(char *)&szFilename[nFilenamelength-4]);
			if (strcmp(sExt,(char *)".jpg")==0||strcmp(sExt,(char *)".JPG")==0) {
				if (!strstr(szFilename,(char *)"_out")) {
					//convert file to one with corner detection markers
					char *szFullpathname = new char[strlen(szFolderName)+2+strlen(szFilename)];
					sprintf(szFullpathname,"%s/%s",szFolderName,szFilename);
					if (!ConvertFastFeatureFile(szFullpathname,nThreshold)) {
						delete []szFullpathname;
						return -8;
					}
					delete []szFullpathname;
				}
			}
		}
		closedir (dir);
	}
	else {
		/* could not open directory */
		perror ("");
		return -7;
	}
	return 0;
}

bool ConvertCannyFile(char *szFilename, int nLowThreshold, int nHighThreshold) {
	//the output image filename will be the same as szFilename, except that "_can" will be added,
	//eg: imageFile001.jpg --> imageFile001_can.jpg
	printf("converting: %s\n",szFilename);
	Mat img1 = imread(szFilename);//original image from video camera
	Mat img1_out;//output image with keypoints shown
	//do Canny edge detection
	Canny(img1,img1_out,nLowThreshold,nHighThreshold);	
	
	int nInputFilenameLength = strlen(szFilename);
	int nOutputFilenameLength = nInputFilenameLength+32;
	char *szOutputFilename = new char[nOutputFilenameLength];
	memset(szOutputFilename,0,nOutputFilenameLength);
	strncpy(szOutputFilename,szFilename,nInputFilenameLength-4);
	strcat(szOutputFilename,(char *)"_can.jpg");
	string sOutputFilename = string(szOutputFilename);
	imwrite(sOutputFilename,img1_out);
	delete []szOutputFilename;
	return true;
}

int ConvertCannyEdgeDetect(char *szFolderName, int nLowThreshold, int nHighThreshold) {//convert all image files in szFolderName using Canny edge detection with specified low and high threshold values
	DIR *dir;
	struct dirent *ent;
	char sExt[8];//used for checking the file extension of each file
	memset(sExt,0,8);
	if ((dir = opendir (szFolderName)) != NULL) {
		/* convert all the jpg files within directory */
		while ((ent = readdir (dir)) != NULL) {
			char *szFilename = ent->d_name;
			int nFilenamelength = strlen(szFilename);
			strcpy(sExt,(char *)&szFilename[nFilenamelength-4]);
			if (strcmp(sExt,(char *)".jpg")==0||strcmp(sExt,(char *)".JPG")==0) {
				if (!strstr(szFilename,(char *)"_can")) {
					//convert file to one with corner detection markers
					char *szFullpathname = new char[strlen(szFolderName)+2+strlen(szFilename)];
					sprintf(szFullpathname,"%s/%s",szFolderName,szFilename);
					if (!ConvertCannyFile(szFullpathname, nLowThreshold, nHighThreshold)) {
						delete []szFullpathname;
						return -10;
					}
					delete []szFullpathname;
				}
			}
		}
		closedir (dir);
	}
	else {
		/* could not open directory */
		perror ("");
		return -9;
	}
	return 0;
}

int GetVertStartingPt(int nX) {//use piecewise linear function to determine vertical starting point that avoids the yellow hull of the boat
	const int MIDPOINT = 300;//the horizontal pixel coordinate where the hull of the boat protrudes the most
	const int VAL_AT_MIDPOINT = 410;//vertical pixel coordinate of tip of hull
	const int MAX_X = 639;
	int nVertStartingPt=479;
	if (nX<=MIDPOINT) {
		double dFactor = ((double)(nX))/MIDPOINT;
		int nRetval = dFactor*VAL_AT_MIDPOINT + (1-dFactor)*nVertStartingPt;
		return nRetval;
	}
	else {
		int nVertEndingPt = 479;
		double dFactor = ((double)(nX-MIDPOINT)) / (MAX_X-MIDPOINT);
		int nRetval = dFactor*nVertEndingPt + (1-dFactor)*VAL_AT_MIDPOINT;
		return nRetval;
	}
	return nVertStartingPt;//should not get here
}

void SelectBestCoast(int nImageWidth, int *coastYCoords, int &nLeftCoast, int &nRightCoast) {//find the coastline that matches up best with the coastYCoords found above
	unsigned int uiStartTime = millis();
	//try to narrow down the min and max over which to search to make this algorithm faster
	const int MIN_LEFT_Y = 207;//minimum pixel value for left side corresponding to edge of solar panel
	const int MIN_RIGHT_Y = 180;//minimum pixel value for right side corresponding to edge of solar panel
	int nMinRightY = 479;
	int nMaxLeftY = 0;
	int nMaxRightY = 0;
	for (int i=0;i<nImageWidth/2;i++) {
		if (coastYCoords[i]>nMaxLeftY) {
			nMaxLeftY=coastYCoords[i];
		}
	}
	for (int i=nImageWidth/2;i<nImageWidth;i++) {
		if (coastYCoords[i]>nMaxRightY) {
			nMaxRightY=coastYCoords[i];
		}
	}
	
	int nBestLeftSideY=0;//left side Y-coordinate of best coastline
	int nBestRightSideY=0;//right side Y-coordinate of best coastline
	int nBestScore=0;//count of the most number of pixel matches for a trial coastline
	for (int i=MIN_LEFT_Y;i<=nMaxLeftY;i++) {
		for (int j=MIN_LEFT_Y;j<=nMaxRightY;j++) {
			int nNumMatches=0;
			for (int k=0;k<nImageWidth;k++) {
				double dFactor = ((double)k)/(nImageWidth-1);
				int nY = (1.0-dFactor)*i + dFactor*j;
				if (nY==coastYCoords[k]) {
					nNumMatches++;
				}
			}
			if (nNumMatches>nBestScore) {
				nBestScore=nNumMatches;
				nBestLeftSideY=i;
				nBestRightSideY=j;
			}
		}
	}
	nLeftCoast=nBestLeftSideY;
	nRightCoast=nBestRightSideY;
	unsigned int uiEndTime = millis();
	//test
	printf("time to find horizon = %.3f sec\n",((double)(uiEndTime - uiStartTime))/1000.0);
	//end test
}

bool ConvertImageFile(char *szInputFilename, char *szOutputFilename, bool bFindObjects) {//convert szFilename so that it has  red linear line drawn on it estimating the position of (1) the coastline, or (2) the horizon, whichever is "seen" first by the boat
	//also (if bFindObjects is true) draw white boxes around any objects that are detected below the horizon line
	//the output image filename will be the same as szFilename, except that "_hor" or "_obj" will be added, depending on whether or not bFindObjects is false or true respectively
	//eg: imageFile001.jpg --> imageFile001_hor.jpg or imageFile001_obj.jpg
	const int MIN_PIXELS = 20;//minimum # of pixels to check before looking for coastline or horizon
	const double MIN_HUE_TRANSITION = 30;//look for this much of a transition in hue to indicate a transition from water to coastline (or sky)
	const unsigned char MIN_SATURATION = 13;//only look at pixels with at least this much saturation when trying to figure out where pixels transition from water to coastline
	const unsigned char MIN_VALUE = 64;//only look at pixels with at least this much value (brightness) when trying to figure out where pixels transition from water to coastline
	
	
	printf("converting: %s\n",szInputFilename);
	Mat img1 = imread(szInputFilename);//original image from video camera
	Mat img1_out = img1.clone();

	//cvtColor(img1,img1_hsv,COLOR_BGR2HSV);
  
	int nImageWidth = img1.cols;
	int nImageHeight = img1.rows;
	//start from bottom of image (left and right edges) and go up in vertical lines, look at HSV values, require at least 5% S (13 counts), 25% V (64 counts), check
	//for abrupt transition in H (>60 deg (>30 counts) from avg of preceding points) for coastline detection.
	//transition from water to sky may be more subtle, may need to look for transition in V instead (sky should appear brighter than water)

	int nX = 0;//x-coordinate of pixel
	int *coastYCoords = new int[nImageWidth];
	memset(coastYCoords,0,nImageWidth*sizeof(int));
	for (int j=0;j<nImageWidth;j++) {
		nX=j;
		int nVertStartingPoint = GetVertStartingPt(nX);//use piecewise linear function to determine vertical starting point that avoids the yellow hull of the boat
		double dAvgH=0.0;//average of preceding values of hue (in counts from 0 to 180)
		double dSumH=0.0;//sum of all hue values
		int nPixelCount=0;//the number of pixels that meet the qualification criteria for saturation (MIN_SATURATION) and brightness (MIN_VALUE)
		for (int i=0;i<(nVertStartingPoint-2);i++) {
			int nY = nVertStartingPoint-i;//y-coordinate of pixel
			Vec3b pixel(img1.at<Vec3b>(nY,nX));//read pixel (nX,nY) (make copy)
			unsigned char b = pixel.val[0];
			unsigned char g = pixel.val[1];
			unsigned char r = pixel.val[2];
			OPENCSV_HSV hsv;
			ImageUtil::ConvertToHSV(r, g, b, &hsv);//get the OpenCSV style HSV coordinates at a point, given the r,g,b color of that point
			if (hsv.h>0&&hsv.s>0&&hsv.v>0&&(hsv.s>=MIN_SATURATION||hsv.v>=MIN_VALUE)) {
				nPixelCount++;
				dSumH+=(double)(hsv.h);
				if (nPixelCount>=MIN_PIXELS) {
					dAvgH = dSumH / (nPixelCount);
					if (fabs(hsv.h-dAvgH)>=MIN_HUE_TRANSITION) {//have found possible tranisition from water to coastline (or possibly sky)
						//check next 2 pixels to see if they also exceed MIN_HUE_TRANSITION
						Vec3b pixel2(img1.at<Vec3b>(nY-1,nX));
						unsigned char b2 = pixel2.val[0];
						unsigned char g2 = pixel2.val[1];
						unsigned char r2 = pixel2.val[2];
						OPENCSV_HSV hsv2;
						ImageUtil::ConvertToHSV(r2, g2, b2, &hsv2);
						if (hsv2.h>0&&hsv2.s>0&&hsv2.v>0&&(hsv2.s>=MIN_SATURATION||hsv2.v>=MIN_VALUE)) {
							if (fabs(hsv2.h-dAvgH)>=MIN_HUE_TRANSITION) {
								Vec3b pixel3(img1.at<Vec3b>(nY-2,nX));
								unsigned char b3 = pixel3.val[0];
								unsigned char g3 = pixel3.val[1];
								unsigned char r3 = pixel3.val[2];
								OPENCSV_HSV hsv3;
								ImageUtil::ConvertToHSV(r3, g3, b3, &hsv3);
								if (hsv3.h>0&&hsv3.s>0&&hsv3.v>0&&(hsv3.s>=MIN_SATURATION||hsv3.v>=MIN_VALUE)) {
									if (fabs(hsv3.h-dAvgH)>=MIN_HUE_TRANSITION) {
										//look back to see if we might have missed the transition due to lack of saturation or brightness
										int k=nY+1;
										while (k<nImageHeight) {
											Vec3b backPixel(img1.at<Vec3b>(k,nX));
											unsigned char b_back = backPixel.val[0];
											unsigned char g_back = backPixel.val[1];
											unsigned char r_back = backPixel.val[2];
											OPENCSV_HSV hsv_back;
											ImageUtil::ConvertToHSV(r_back, g_back, b_back, &hsv_back);
											if (hsv_back.h==0) {
												break;
											}
											if (fabs(hsv_back.h-dAvgH)<MIN_HUE_TRANSITION) {
												break;
											}
											else if (hsv_back.s>=MIN_SATURATION&&hsv_back.v>=MIN_VALUE) {
												break;
											}
											nY=k;
											k++;
										}
										coastYCoords[j] = nY;
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	int nLeftCoastY=0, nRightCoastY=0;
	SelectBestCoast(nImageWidth,coastYCoords,nLeftCoastY,nRightCoastY);//find the coastline that matches up best with the coastYCoords found above
	line(img1_out,Point(0,nLeftCoastY),Point(nImageWidth-1,nRightCoastY),Scalar(0,0,255));
	if (bFindObjects) {
		//look for objects below horizon line, and draw white boundary box around them
		int nLookForObjectsAt = 0;
		Vec3b whiteColor(255,255,255);
		IMAGE_STATS imageStats;
		unsigned int *pixelData = ImageUtil::GetImageStats(img1, nLeftCoastY, nRightCoastY, &imageStats);
		if (pixelData) {
			vector <OBJECT_BLOCK *>objectBlocks = ImageUtil::GetObjectBlocks(pixelData,nImageWidth,nImageHeight,nLeftCoastY,nRightCoastY,
				DEFAULT_LOW_THRESHOLD,DEFAULT_HIGH_THRESHOLD,&imageStats);
			int nNumObjects = objectBlocks.size();
			for (int i=0;i<nNumObjects;i++) {
				rectangle(img1_out,Point(objectBlocks[i]->nX,objectBlocks[i]->nY),Point(objectBlocks[i]->nX+BLOCKSIZE,objectBlocks[i]->nY+BLOCKSIZE),whiteColor);
			}
			delete []pixelData;
		}
	}
	//test
	//make small red circle for each point of coastYCoords
	for (int i=0;i<nImageWidth;i++) {
		circle(img1_out,Point(i,coastYCoords[i]),2,Scalar(0,0,255));
	}
	//end test
	string sOutputFilename = string(szOutputFilename);
	bool bRetval = false;
	try {
        bRetval = imwrite(sOutputFilename,img1_out);
    }
    catch (cv::Exception& ex) {
        fprintf(stderr, "Exception writing image to JPG format: %s\n", ex.what());
        return false;
    }
	delete []coastYCoords;
	return bRetval;
}

int ConvertImageDetect(char *szFolderName, bool bFindObjects) {//convert all image files in szFolderName using Horizon / Coastline detection algorithm, and also (if bFindObjects is true) look for objects below the horizon line
	DIR *dir;
	struct dirent ent;
	struct dirent *result;
	char sExt[8];//used for checking the file extension of each file
	char sAddText[8];
	memset(sExt,0,8);
	memset(sAddText,0,8);
	strcpy(sAddText,(char *)"_hor");
	if (bFindObjects) {
		strcpy(sAddText,(char *)"_obj");
	}

	int nReadDir = 0;
	
	if ((dir = opendir (szFolderName)) != NULL) {
		/* convert all the jpg files within directory */
		errno=0;
		while ((nReadDir = readdir_r(dir,&ent,&result)) == 0) {
			if (!result) {
				break;//end of stream
			}
			char *szFilename = ent.d_name;
			int nFilenamelength = strlen(szFilename);
			strcpy(sExt,(char *)&szFilename[nFilenamelength-4]);
			if (strcmp(sExt,(char *)".jpg")==0||strcmp(sExt,(char *)".JPG")==0) {
				if (!strstr(szFilename,sAddText)) {
					//convert file to one with detected horizon line
					char *szFullpathname = new char[strlen(szFolderName)+2+strlen(szFilename)];
					sprintf(szFullpathname,"%s/%s",szFolderName,szFilename);
					
					int nInputFilenameLength = strlen(szFullpathname);
					int nOutputFilenameLength = nInputFilenameLength+32;
					char *szOutputFilename = new char[nOutputFilenameLength];
					memset(szOutputFilename,0,nOutputFilenameLength);
					strncpy(szOutputFilename,szFullpathname,nInputFilenameLength-4);
					strcat(szOutputFilename,sAddText);
					strcat(szOutputFilename,(char *)".jpg");
					
					if (FileExists(szOutputFilename)) {//file already exists
						printf("Output for %s already exists.\n",szOutputFilename);
					}
					else {
						if (!ConvertImageFile(szFullpathname,szOutputFilename,bFindObjects)) {
							delete []szFullpathname;
			     			delete []szOutputFilename;
							return -12;
						}
					}
					delete []szOutputFilename;
					delete []szFullpathname;
				}
			}
		}
		if (nReadDir!=0) {
			printf("Error occured when trying to read folder: %d\n",nReadDir);
		}
		closedir (dir);
	}
	else {
		/* could not open directory */
		perror ("");
		return -11;
	}
	return 0;
}

int CheckForFlag(int argc, char **argv, char *szFlag) {//check to see if szFlag can be found in any of the argv elements
	for (int i=0;i<argc;i++) {
		if (strstr(argv[i],szFlag)) {
			return i;
		}
	}
	return -1;
}

int ConvertStaticImage(char *szPictureFilename) {
	//capture a static high resolution image and save the result to a file named szPictureFilename
	if (!szPictureFilename) {
		return -1;
	}
	char *commandStr = new char[strlen(szPictureFilename)+128];
	sprintf(commandStr,"raspistill -o %s",szPictureFilename);
	system(commandStr);
	delete []commandStr;
	return 0;
}

int main(int argc, char** argv)
{
	//Create Mat images for camera capture 
	const int NUM_WARMUP_FRAMES = 10;//number of preliminary frames to capture before actually saving one
	Mat img1;//original image from video camera
	Mat img1_out;//output image with keypoints shown
	Mat img_gray1;//grayscale image 
	Mat edge_img;//image processed to find edges (using Canny algorithm)
	char *outputImg = "output_img.jpg";
	if (argc>1) {
		int nConvertStaticIndex = CheckForFlag(argc,argv,(char *)"-s");//check to see if a static high-res picture should be taken
		if (nConvertStaticIndex>=1) {
			if (argc<nConvertStaticIndex+2) {
				ShowUsage();
				return -10;
			}
			char *szPictureFilename = argv[nConvertStaticIndex+1];
			return ConvertStaticImage(szPictureFilename);
		}
		int nConvertFFIndex = CheckForFlag(argc,argv,(char *)"-cf");//check to see if fast feature conversion should be performed
		if (nConvertFFIndex>=1) {
			if (argc<(nConvertFFIndex+3)) {
				ShowUsage();
				return -3;
			}
			char *szFolderName = argv[nConvertFFIndex+1];
			int nThreshold=0;
			if (sscanf(argv[nConvertFFIndex+2],"%d",&nThreshold)<1) {
				printf("Invalid threshold.\n");
				return -4;
			}
			return ConvertFastFeatureDetect(szFolderName,nThreshold);//convert all image files in szFolderName using FAST feature detection with the specified threshold value
		}
		int nConvertCEIndex = CheckForFlag(argc,argv,(char *)"-ce");//check to see if Canny edge conversion should be performed
		if (nConvertCEIndex>=1) {
			if (argc<(nConvertCEIndex+4)) {
				ShowUsage();
				return -7;
			}
			char *szFolderName = argv[nConvertCEIndex+1];
			int nLowThreshold = 0, nHighThreshold=0;
			if (sscanf(argv[nConvertCEIndex+2],"%d",&nLowThreshold)<1) {
				printf("Invalid low threshold.\n");
				return -5;
			}
			if (sscanf(argv[nConvertCEIndex+3],"%d",&nHighThreshold)<1) {
				printf("Invalid high threshold.\n");
				return -6;
			}
			return ConvertCannyEdgeDetect(szFolderName,nLowThreshold,nHighThreshold);//convert all image files in szFolderName using Canny edge detection with specified low and high threshold values
		}
		int nConvertHorizonIndex = CheckForFlag(argc,argv,(char *)"-h");//check to see if horizon / coastline detection algorithm should be performed
		if (nConvertHorizonIndex>=1) {
			if (argc<(nConvertHorizonIndex+2)) {
				ShowUsage();
				return -8;
			}
			char *szFolderName = argv[nConvertHorizonIndex+1];
			return ConvertImageDetect(szFolderName,false);//convert all image files in szFolderName using Horizon / Coastline detection algorithm
		}
		int nConvertObjectsIndex = CheckForFlag(argc,argv,(char *)"-o");//check to see if object detection algorithm should be performed
		if (nConvertObjectsIndex>=1) {
			if (argc<(nConvertObjectsIndex+2)) {
				ShowUsage();
				return -9;
			}
			char *szFolderName = argv[nConvertObjectsIndex+1];
			return ConvertImageDetect(szFolderName,true);//convert all image files in szFolderName to find horizon line and also find objects (if any) below that horizon line
		}
		outputImg = argv[1];
	}
	char *edge_output_name = new char[strlen(outputImg)+10];
	strcpy(edge_output_name,(char *)"edge_");
	strcat(edge_output_name,outputImg);
	//Open video camera
	VideoCapture cap1(0);
	if (cap1.isOpened() == false)  
	{
		cout << "Cannot open the video camera" << endl;
		cin.get(); //wait for any key press
		return -1;
	}
	double dWidth = cap1.get(CAP_PROP_FRAME_WIDTH); //get the width of frames of video
	double dHeight = cap1.get(CAP_PROP_FRAME_HEIGHT); //get the height of frames of video

	cout << "Resolution of video: " << dWidth << " x " << dHeight << endl;

	vector<KeyPoint> keypoints;

	int threshold=80;

	for (int i=0;i<=NUM_WARMUP_FRAMES;i++) {
		bool bSuccess = cap1.read(img1); // read a new frame from video1 
		//Break the while loop if the frames cannot be captured
		if (bSuccess == false)
		{
			cout << "Video camera is disconnected" << endl;
			cin.get(); //Wait for any key press
			return -2;
		}
		//do Canny edge detection
		Canny(img1,edge_img,100,200);
		//convert image to grayscale
		cvtColor(img1,img_gray1,COLOR_RGB2GRAY);
		cv::FAST(img_gray1,keypoints,threshold);
		cv::drawKeypoints(img1,keypoints,img1_out,Scalar(0,0,255));
	}
	string sOutputFilename = string(outputImg);
	imwrite(sOutputFilename,img1_out);
	string sEdgeOutputFilename = string(edge_output_name);
	imwrite(sEdgeOutputFilename,edge_img);
	//imshow("stream1",img1_out);

	return 0;
}


