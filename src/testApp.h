#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxXmlSettings.h"

using namespace ofxCv;
using namespace cv;

/*
 // --------------------------
 //     _    ____   ___  _   _ _____
 //    / \  | __ ) / _ \| | | |_   _|
 //   / _ \ |  _ \| | | | | | | | |
 //  / ___ \| |_) | |_| | |_| | | |
 // /_/   \_\____/ \___/ \___/  |_|
 // 
 // ABOUT. 
 // TextOrientationDetector by Golan Levin, 11 June 2013 / http://flong.com / golan@flong.com
 // This software is released as-is with NO RIGHTS RESERVED, CC-0 http://creativecommons.org/choose/zero/ 
 // This app is known to compile and work in Mac OSX 10.8 using openFrameworks 0.7.4. 
 // 
 // This program determines the orientation (horizontal or vertical) of the text in a provided image.
 // This can be a timesaving pre-processor for more computationally-expensive OCR with e.g. Tesseract.
 // The program is written as an app using OpenFrameworks, a FLOSS cross-platform toolkit for the arts. 
 // TextOrientationDetector makes use of the following libraries:
 // 
 // openFrameworks 0.7.4: http://www.openframeworks.cc/download/  
 //                       https://github.com/openframeworks 
 // ofxCv:                https://github.com/kylemcdonald/ofxCv 
 // 
 // The program also uses the ofxOpenCv "addon" interface to OpenCV, 
 // and the ofxXmlSettings "addon" interface to tinyXML,
 // both of which come with the openFrameworks 0.7.4. download.
 // 
 // Technically, TextOrientationDetector works by lowpass filtering the page image, 
 // and then comparing the strength of the horizontal and vertical edges in that blurred image.
 //
 // 
 // --------------------------
 //   _   _ ____    _    ____ _____
 //  | | | / ___|  / \  / ___| ____|
 //  | | | \___ \ / _ \| |  _|  _|
 //  | |_| |___) / ___ \ |_| | |___
 //   \___/|____/_/   \_\____|_____|
 // 
 // USAGE. 
 // TextOrientationDetector takes the following command-line arguments:
 // 1. The filename of an image containing text. 
 //    The software can handle a range of different file types, image sizes, and document content, 
 //    but it is best-tuned for letter-sized documents scanned at ~200 dpi, containing black text 
 //    on a white background which is approximately 9-12 points in size. 
 //    By default, images are expected to live in a directory called "data" located next to the app itself. 
 //    Filenames can be given relative to this location, eg. ../../someOther.tiff etcetera.
 // 2. An integer (1 or 0, meaning true or false) indicating whether to provide results via stdout. 
 // 3. An integer (1 or 0, meaning true or false) indicating whether to provide results via XML. 
 // 4. An integer (1 or 0, meaning true or false) indicating whether to render results to the screen. 
 //    If this argument is 1, the app stays open; if it is 0, the app exits immediately after processing.
 // 
 // Note that the app may be invoked from the OSX command line as follows:
 // open -n TextOrientationDetector.app/ --args test1.tiff 1 1 1
 //
 // 
 //
 // --------------------------
 //    ___  _   _ _____ ____  _   _ _____
 //   / _ \| | | |_   _|  _ \| | | |_   _|
 //  | | | | | | | | | | |_) | | | | | |
 //  | |_| | |_| | | | |  __/| |_| | | |
 //   \___/ \___/  |_| |_|    \___/  |_|
 //
 // OUTPUT.
 // TextOrientationDetector produces output in two ways: via stdout, and written to an XML file.
 // When TextOrientationDetector prints to stdout, the results look like the following two samples: 
 //
 // TextOrientation: 	test1.tiff	1	HORIZONTAL	2.58
 // TextOrientation: 	test2.tiff	2	VERTICAL	2.51
 //
 // The items in these reports should be understood as in the following example: 
 // TextOrientation:    (this word always appears)
 // test1.tiff          (this is the name of the processed file, for confirmation)
 // 1                   (this is an ID code for the orientation type; see below)
 // HORIZONTAL          (this is the redundant but human-readable name of the orientation type)
 // 2.58                (this is the confidence value for the guess about the orientation)
 //
 // When TextOrientationDetector exports an XML file, the results look as follows: 
 // in the file: images/test1.xml  :
 //
 // <input_filename>test1.tiff</input_filename>
 // <orientation_id>1</orientation_id>
 // <orientation_name>HORIZONTAL</orientation_name>
 // <confidence>2.58</confidence>
 // 
 // The text orientation ID codes are as shown below: 
 // TEXT_ERROR = -1,       (there was a problem loading the image from the provided filename)
 // TEXT_BLANK = 0,        (the image is a blank page)
 // TEXT_HORIZONTAL = 1,   (horizontal text, the usual case)
 // TEXT_VERTICAL = 2,     (vertical text, the uncommon case)
 // TEXT_INDETERMINATE = 3 (can't be determined: there could be a mixture, or diagonal text)
 //
 //
*/


enum TextOrientationType {
	TEXT_ERROR = -1,
	TEXT_BLANK = 0,
	TEXT_HORIZONTAL = 1,
	TEXT_VERTICAL = 2,
	TEXT_INDETERMINATE = 3
};


class testApp : public ofBaseApp{

	public:
	
		
		//-------------------------
		// OpenFrameworks app boilerplate: 
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
	
	
		//-------------------------
		// Importing and arguments:
		void   parseCommandLine (int argc, char **argv);
		string filenameOfImageToProcess;
		float  orientationDecisionFactor;
		bool   bDoOutputStdOut;
		bool   bDoOutputXml;
		bool   bDoRenderToScreen;
	
		int	   nArguments;
		char   **theArguments;
	
	
		//-------------------------
		// Exporting: 
		void exportConclusion();
		TextOrientationType theConclusion;
		string conclusionString;
		float  conclusionConfidence;
		ofxXmlSettings outputXmlWriter;

	
		//-------------------------
		// Image processing:
		ofImage	anOfImage;
		
		Mat grayInput;
		Mat graySmall;
		Mat graySmallBlurred;
		Mat graySmallSobelH;
		Mat graySmallSobelV;
		
		int inputW;
		int inputH;
		int processW;
		int processH;
		
		float meanH;
		float meanV;
};



