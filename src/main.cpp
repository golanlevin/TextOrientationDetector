#include "ofMain.h"
#include "testApp.h"
#include "ofAppGlutWindow.h"

//========================================================================
int main( int argc, char **argv ){

    ofAppGlutWindow window;
	ofSetupOpenGL(&window, 800,400, OF_WINDOW);			// <-------- setup the GL context

	testApp *myApp = new testApp();
	myApp->parseCommandLine(argc, argv);
	ofRunApp( myApp );

}
