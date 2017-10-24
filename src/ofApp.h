#pragma once

#include "ofMain.h"
#include "ofxButtons.h"
// DLIB
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>


using namespace dlib;
using namespace std;

enum drawStates{
	DRAW_POINTS,
	DRAW_CONNECTIONS,
	DRAW_TRIANGLES,
	DRAW_TEXTURED,
	DRAW_SWAP,
	EDIT_POINTS
};

class ofApp : public ofBaseApp {
public:
	
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void dragEvent(ofDragInfo dragInfo);
	
	float fps;
	ofVideoGrabber cam;
	bool bDrawCamera;
	
	//MARK: -  DLIB Face Detection
	bool bUseDLib;
    bool bScaleUp;
	object_detector< scan_fhog_pyramid<pyramid_down<6> > > detector;
	array2d<unsigned char> dImg; // dlib image
	std::vector<dlib::rectangle> dets;
	void imgToDLibImage( array2d<unsigned char> &newImg, ofImage &srcImg);
	void setDLibImageFromPixels( array2d<unsigned char> &newImg, unsigned char * pix, int w, int h, int ofPixelType);
	void DLibImageToImg( array2d<unsigned char> &newImg, ofImage &srcImg);
	
	//MARK: -  DLib Landmark detector
	shape_predictor sp;
	std::vector<full_object_detection> shapes;
	

	//MARK: -  UVC control
	void setupCamera( ofVideoGrabber &vidGrabber);
	bool bLEDisOn;
	
	//MARK: - Drawing
	int drawState;
	void setupFaceTriangles();
	void setupFaceTexturePoints();
	ofImage faceTexture;
	std::vector < std::vector < int > > triangles;
	std::vector < std::vector < int > > fullTriangles;
	
	std::vector < ofPoint * > texPts;
	void drawFacePoints();
	void drawTriangles();
	void drawTextured();
	void drawSwapped();
	void drawFlatFace();
	void exportFacePoints();
	void exportTexturePoints();
		
	// editing
	ofPoint * selPt;
	bool clickPts(ofPoint p);
	bool drag(ofPoint p);
	void drawTexturePoints();
	void unselectPts();
	
	// BUTTONS
	ButtonManager buttons;
	void setupButtons();
	
	bool bSaveFrame;
	
};
