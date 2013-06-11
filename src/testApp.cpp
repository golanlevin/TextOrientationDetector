#include "testApp.h"


//--------------------------------------------------------------
void testApp::parseCommandLine (int argc, char **argv){
	
	// Example command-line usage (OSX):
	// open -n TextOrientationDetector.app/ --args test2.tiff 1 1 1
	
	nArguments   = argc;
	theArguments = argv;
	
	filenameOfImageToProcess = "test1.tiff"; // by default, stored in the "data" folder.
	bDoOutputStdOut	   = false;
	bDoOutputXml       = false;
	bDoRenderToScreen  = true;
	
	if (nArguments == 5){
		filenameOfImageToProcess = ofToString (theArguments[1]);

		if (*theArguments[2] == '1'){
			bDoOutputStdOut = true;
		} else {
			bDoOutputStdOut = false;
		}
		
		if (*theArguments[3] == '1'){
			bDoOutputXml = true;
		} else {
			bDoOutputXml = false;
		}
		
		if (*theArguments[4] == '1'){
			bDoRenderToScreen = true;
		} else {
			bDoRenderToScreen = false;
		}
		
	} 
}


//--------------------------------------------------------------
void testApp::setup(){
	
	//----------------------------
	// Set the default conclusion before processing.
	ofSetWindowTitle("Text Orientation Detector");
	theConclusion = TEXT_ERROR;
	conclusionConfidence = 0; 
	
	//----------------------------
	// Load or acquire the input image. 
	anOfImage.loadImage(filenameOfImageToProcess);
	inputW = anOfImage.getWidth();
	inputH = anOfImage.getHeight();
	
	//----------------------------
	// Convert the loaded image into a grayscale openCV Mat structure.
	if ((inputW > 0) && (inputH > 0)){
		int bpp = anOfImage.bpp;
		if (bpp == 24){
			// If the input image has 24 bits per pixel, we've loaded an RGB image.
			Mat colorInput;
			allocate (colorInput, inputW, inputH, CV_8UC3);
			allocate (grayInput,  inputW, inputH, CV_8UC1);
			colorInput = toCv(anOfImage);
			convertColor(colorInput, grayInput, CV_RGB2GRAY);
			
		} else if (bpp == 8){
			// If the input image has 8 bits per pixel, we've loaded a grayscale image.
			allocate (grayInput,  inputW, inputH, CV_8UC1);
			grayInput = toCv(anOfImage);
			
		} else {
			// This is some type of image we weren't expecting. Not sure what this could be; abort. 
			ofLog( OF_LOG_FATAL_ERROR, "ERROR: Encountered unexpected number of bytes per pixel.");
			exportConclusion(); 
			ofExit();
		}
	} else {
		// The loaded image has zero width or height. Abort. 
		ofLog (OF_LOG_FATAL_ERROR, "ERROR: Problem loading image (zero width and/or height).");
		exportConclusion();
		ofExit();
	}
	
	//----------------------------
	// Compute the process size for our signal chain, and allocate necessary buffers.
	// We will do all of our processing on an image 512 pixels wide: a fixed scale. 
	float pageAspectRatio = (float) inputH / (float) inputW;
	processW = 512; 
	processH = (int)(processW * pageAspectRatio);
	allocate (graySmall,        processW, processH, CV_8UC1);
	allocate (graySmallBlurred, processW, processH, CV_8UC1);
	allocate (graySmallSobelH,  processW, processH, CV_8UC1);
	
	//----------------------------
	// Resize the grayInput image into a small (e.g. 512-pixel wide) version.
	cv::Size blurredSmallSize = cv::Size(processW, processH);
	resize(grayInput, graySmall, blurredSmallSize, 0,0, INTER_LINEAR );
	
	//----------------------------
	// Blur the small version, in order to emphasize line-scale (not glyph-scale) features. 
	int kernelSize = (int)(7); // The kernelSize must be odd, else OpenCV complains. 
	float blurSigma = 2.3; 
	cv::Size blurSize = cv::Size(kernelSize, kernelSize);
	GaussianBlur (graySmall, graySmallBlurred, blurSize, blurSigma);
	
	//----------------------------
	// Apply horizontal and vertical Sobel (edge-detection) filters.
	Sobel (graySmallBlurred, graySmallSobelH, graySmallSobelH.type(),  0,1,  3,1,0,BORDER_DEFAULT);
	Sobel (graySmallBlurred, graySmallSobelV, graySmallSobelV.type(),  1,0,  3,1,0,BORDER_DEFAULT);
	
	//----------------------------
	// Compute the average brightness (0..255) of each edge-image.
	Scalar scalarH = cv::mean(graySmallSobelH);
	Scalar scalarV = cv::mean(graySmallSobelV);
	meanH = (float) scalarH.val[0];
	meanV = (float) scalarV.val[0];
	
	//----------------------------
	// Determine the conclusion: is the text blank, horizontal, vertical, or intdeterminate?
	//
	// The orientationDecisionFactor expresses how much stronger the edges in one direction
	// need to be, compared to the edges in the other direction, in order to make our decision.
	// A good value is around 1.3 - 1.6; I've chosen 1.4.
	orientationDecisionFactor = 1.4;
	
	if ((meanH < 1.0) && (meanV < 1.0)){
		// If the average across the image is less than
		// a single grayscale level, the image is "blank".
		theConclusion = TEXT_BLANK;
		conclusionConfidence = 1;
		
	} else {
		
		if ((meanV > 0) && (meanH > 0)){ 
			float hvRatio = meanH / meanV;
			float vhRatio = meanV / meanH;
			
			if        (hvRatio > orientationDecisionFactor){
				theConclusion = TEXT_HORIZONTAL;
				conclusionConfidence = hvRatio;
			} else if (vhRatio > orientationDecisionFactor){
				theConclusion = TEXT_VERTICAL;
				conclusionConfidence = vhRatio;
			} else {
				theConclusion = TEXT_INDETERMINATE;
				conclusionConfidence = max (hvRatio, vhRatio);
			}
			
		} else {
			theConclusion = TEXT_INDETERMINATE;
			conclusionConfidence = 1;
		}
	}
	
	//----------------------------
	// Finish the work: export the conclusion. 
	exportConclusion();
	if (bDoRenderToScreen == false){
		ofExit();
	}
}


//--------------------------------------------------------------
void testApp::exportConclusion(){
	
	// This is a useful and important helper function.
	// Generate the report string, and output the report.
	// The report consists of a line of text written to a .txt file, and/or sent to stdout.
	
	conclusionString = "";
	switch (theConclusion){
		case TEXT_ERROR:
			conclusionString = "ERROR";
			break;
		case TEXT_BLANK:
			conclusionString = "BLANK";
			break;
		case TEXT_HORIZONTAL:
			conclusionString = "HORIZONTAL";
			break;
		case TEXT_VERTICAL:
			conclusionString = "VERTICAL";
			break;
			
		default:
		case TEXT_INDETERMINATE:
			conclusionString = "INDETERMINATE";
			break;
	}
	
	string conclusionConfidenceString = ofToString(conclusionConfidence,2);
	if (conclusionConfidence == 0){ conclusionConfidenceString = "0"; }
	
	//----------------------------
	if (bDoOutputStdOut){
		// If requested, print the results to stdout. 
		// The structure of the report: separated by tabs as follows:
		// the input filename; the conclusion ID code; the conclusion plaintext; a confidence value.
		string reportString =
			"TextOrientation: \t" + 
			filenameOfImageToProcess       + "\t" +
			ofToString((int)theConclusion) + "\t" +
			conclusionString               + "\t" +
			conclusionConfidenceString     + "\n";
		cout << reportString;
	}
	
	//----------------------------
	if (bDoOutputXml){
		// If requested, save an XML file containing the information.
		// Yes, there's some redundancy in the output data. 
		
		outputXmlWriter.addValue ("input_filename",		filenameOfImageToProcess); 
		outputXmlWriter.addValue ("orientation_id",		(int) theConclusion);
		outputXmlWriter.addValue ("orientation_name",	conclusionString);
		outputXmlWriter.addValue ("confidence",			conclusionConfidenceString);
		
		// Remove the file extension
		char *imgFilenameChars = (char *) filenameOfImageToProcess.c_str();
		int nChars = filenameOfImageToProcess.length();
		int lastPositionOfPeriod = 0;
		bool bFoundPeriod = false; 
		for (int i=0; i<nChars; i++){
			char c = filenameOfImageToProcess[i];
			if (c == '.'){
				bFoundPeriod = true;
				lastPositionOfPeriod = i; 
			}
		}
		string outputXmlFilename = ""; 
		if (bFoundPeriod){
			for (int i=0; i<lastPositionOfPeriod; i++){
				outputXmlFilename += filenameOfImageToProcess[i];
			}
			outputXmlFilename += ".xml";
		} else {
			outputXmlFilename = filenameOfImageToProcess + ".xml";
		}
		
		// string outputXmlFilename = "output.xml"; //filenameOfImageToProcess + "_ORIENTATION.xml";
		outputXmlWriter.saveFile(outputXmlFilename);
	}

}


//--------------------------------------------------------------
void testApp::draw(){
	
	if (bDoRenderToScreen){
		float drawW = processW * 0.5;
		float drawH = processH * 0.5;
		
		ofSetColor (255,255,255);
		drawMat(graySmall,       0,				0, drawW, drawH); 
		drawMat(graySmallSobelH, (drawW+5)*1,   0, drawW, drawH);
		drawMat(graySmallSobelV, (drawW+5)*2,	0, drawW, drawH);
		
		ofSetColor (50,0,0);
		ofDrawBitmapString( ofToString (meanH), (drawW+5)*1, drawH+15);
		ofDrawBitmapString( ofToString (meanV), (drawW+5)*2, drawH+15);
		
		ofDrawBitmapString (filenameOfImageToProcess,           10, drawH + 30);
		ofDrawBitmapString ("The text is: " + conclusionString, 10, drawH + 45);
		
		bool bDisplayArguments = false;
		if (bDisplayArguments){
			for (int i=0; i<nArguments; i++){
				string anArgument = ofToString(i) + " " + ofToString (theArguments[i]).c_str();
				ofDrawBitmapString( anArgument,            10, drawH + 60+ i*15);
			}
		}
	}
	
}




//--------------------------------------------------------------
// Other virtual ofApp functions, not used.
// 
void testApp::update(){ }
void testApp::keyPressed(int key){ }
void testApp::keyReleased(int key){ }
void testApp::mouseMoved(int x, int y ){ }
void testApp::mouseDragged(int x, int y, int button){ }
void testApp::mousePressed(int x, int y, int button){ }
void testApp::mouseReleased(int x, int y, int button){ }
void testApp::windowResized(int w, int h){ }
void testApp::gotMessage(ofMessage msg){ }
void testApp::dragEvent(ofDragInfo dragInfo){ }