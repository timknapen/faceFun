#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup() {
	ofSetVerticalSync(true);
	ofSetFrameRate(60);
	ofBackground(255);
	setupCamera(cam);
	bDrawCamera = true;
	bSaveFrame = false;
	
	fps = 0;
	
	setupFaceTriangles();
	setupFaceTexturePoints();
	faceTexture.load("facePattern.png");
	
	selPt = NULL;
	
	// DLIB
	// deserialize(ofToDataPath("standard_face_detector.svm")) >> detector;
	//cout << "Loaded detector " << detector.
	detector = get_frontal_face_detector();
	bScaleUp = false;
	bUseDLib = false;
	
	// LANDMARKS
	deserialize(ofToDataPath("shape_predictor_68_face_landmarks.dat")) >> sp;
	
	drawState = DRAW_POINTS;

	setupButtons();
	
}

//--------------------------------------------------------------
void ofApp::update() {
	
	cam.update();
	if(cam.isFrameNew() && bUseDLib) {
		
		setDLibImageFromPixels(dImg,
							   cam.getPixels(),
							   cam.getWidth(),
							   cam.getHeight(),
							   OF_IMAGE_COLOR);
		
		
		// Make the image bigger by a factor of two.  This is useful since
		// the face detector looks for faces that are about 80 by 80 pixels
		// or larger.  Therefore, if you want to find faces that are smaller
		// than that then you need to upsample the image as we do here by
		// calling pyramid_up().  So this will allow it to detect faces that
		// are at least 40 by 40 pixels in size.  We could call pyramid_up()
		// again to find even smaller faces, but note that every time we
		// upsample the image we make the detector run slower since it must
		// process a larger image.
		if(bScaleUp){
			pyramid_up(dImg);
		}
		
		
		// Now tell the face detector to give us a list of bounding boxes
		// around all the faces it can find in the image.
		dets = detector(dImg);
		
		// cout << "Number of faces detected: " << dets.size() << endl;
		
		shapes.clear();
		for (unsigned long j = 0; j < dets.size(); ++j)
		{
			full_object_detection shape = sp(dImg, dets[j]);
			// cout << "number of parts: "<< shape.num_parts() << endl;
			// cout << "pixel position of first part:  " << shape.part(0) << endl;
			// cout << "pixel position of second part: " << shape.part(1) << endl;
			// You get the idea, you can get all the face part locations if
			// you want them.  Here we just store them in shapes so we can
			// put them on the screen.
			shapes.push_back(shape);
		}
		
	}
	fps += (ofGetFrameRate() - fps)/10;
}


//MARK: - DRAWING

//--------------------------------------------------------------
void ofApp::draw() {
	ofSetColor(255);
	if(drawState == EDIT_POINTS){
		drawTexturePoints();
	}else{
		ofPushMatrix();
		{
			
			if(cam.getWidth() > 0){
				float camScale = ofGetWidth()/cam.getWidth();
				ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
				ofScale(camScale, camScale);
				ofTranslate(-cam.getWidth()/2, -cam.getHeight()/2);
			}
			
			if(bDrawCamera){
				cam.draw(0, 0);
			}
			
			ofPushMatrix();
			{
				if(bScaleUp){
					ofScale(.5, .5);
				}
				
				switch(drawState){
					case DRAW_POINTS:
					{
						ofSetColor(0, 0, 100);
						ofNoFill();
						ofSetLineWidth(1);
						drawFacePoints();
					}
						break;
					case DRAW_CONNECTIONS:
					{
						ofSetColor(0, 0, 100);
						ofNoFill();
						ofSetLineWidth(1);
						drawFacePoints();
						drawTriangles();
					}
						break;
					case DRAW_TRIANGLES:
						ofFill();
						drawTriangles();
						break;
					case DRAW_TEXTURED:
						drawTextured();
						break;
					case DRAW_SWAP:
						drawSwapped();
						break;
				}
				
			}
			ofPopMatrix();
		}
		ofPopMatrix();
		
	}
	if(bSaveFrame){
		ofImage img;
		img.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
		string fileName =
		ofToString(ofGetYear()) + "_" +
		ofToString(ofGetMonth(), 2, '0') + "_" +
		ofToString(ofGetDay(), 2, '0') + "-" +
		ofToString(ofGetHours(),2,'0') + "_" +
		ofToString(ofGetMinutes(), 2, '0')  + "_" +
		ofToString(ofGetSeconds(), 2, '0');
		img.save("photos/" + fileName + ".jpg");
		bSaveFrame = false;
	}else if(buttons.isVisible()){
		ofSetLineWidth(1);
		ofDrawBitmapStringHighlight(ofToString(fps, 0)+"fps", 10, 40);
	}
}

//--------------------------------------------------------------
void ofApp::drawFacePoints(){
	
	for(int i = 0; i < shapes.size(); i++){
		full_object_detection shape = shapes[i];
		dlib::rectangle rect = shape.get_rect();
		ofDrawRectangle(rect.left(), rect.top(), rect.width(), rect.height());
		
		for(int j = 0; j < shape.num_parts(); j++){
			ofDrawRectangle(shape.part(j).x(), shape.part(j).y(), 2, 2);
			ofDrawBitmapString(ofToString(j,0), shape.part(j).x(), shape.part(j).y());
		}
		
		if(shape.num_parts() < 68){
			continue;
		}
		
		/*
		 // ALL POINTS
		 for(int j = 0; j < 68; j++){
		 ofDrawEllipse(shape.part(j).x(), shape.part(j).y(), 3, 3);
		 }
		 */
		
		// JAW
		for(int j = 0; j < 16; j++){
			ofDrawLine(shape.part(j).x(), shape.part(j).y(),  shape.part(j+1).x(), shape.part(j+1).y());
		}
		// Left eyebrow
		for(int j = 17; j < 21; j++){
			ofDrawLine(shape.part(j).x(), shape.part(j).y(),  shape.part(j+1).x(), shape.part(j+1).y());
		}
		// Left Eye
		ofDrawEllipse(	(shape.part(36).x() + shape.part(39).x())/2,
					  (shape.part(36).y() + shape.part(39).y())/2,
					  5,5);
		for(int j = 36; j < 41; j++){
			ofDrawLine(shape.part(j).x(), shape.part(j).y(),  shape.part(j+1).x(), shape.part(j+1).y());
		}
		ofDrawLine(shape.part(41).x(), shape.part(41).y(),  shape.part(36).x(), shape.part(36).y());
		
		// Right Eyebrow
		for(int j = 22; j < 26; j++){
			ofDrawLine(shape.part(j).x(), shape.part(j).y(),  shape.part(j+1).x(), shape.part(j+1).y());
		}
		
		// Right Eye
		ofDrawEllipse(	(shape.part(42).x() + shape.part(45).x())/2,
					  (shape.part(42).y() + shape.part(45).y())/2,
					  5,5);
		for(int j = 42; j < 47; j++){
			ofDrawLine(shape.part(j).x(), shape.part(j).y(),  shape.part(j+1).x(), shape.part(j+1).y());
		}
		ofDrawLine(shape.part(47).x(), shape.part(47).y(),  shape.part(42).x(), shape.part(42).y());
		
		// Nose
		for(int j = 27; j < 30; j++){
			ofDrawLine(shape.part(j).x(), shape.part(j).y(),  shape.part(j+1).x(), shape.part(j+1).y());
		}
		for(int j = 31; j < 35; j++){
			ofDrawLine(shape.part(j).x(), shape.part(j).y(),  shape.part(j+1).x(), shape.part(j+1).y());
		}
		
		
		// Mouth
		for(int j = 48; j < 59; j++){
			ofDrawLine(shape.part(j).x(), shape.part(j).y(),  shape.part(j+1).x(), shape.part(j+1).y());
		}
		// reconnect
		ofDrawLine(shape.part(59).x(), shape.part(59).y(),  shape.part(48).x(), shape.part(48).y());
		
		
		// bottom of mouth
		ofSetColor(255,100,0);
		for(int j = 60; j < 67; j++){
			ofDrawLine(shape.part(j).x(), shape.part(j).y(),  shape.part(j+1).x(), shape.part(j+1).y());
		}
		ofDrawLine(shape.part(67).x(), shape.part(67).y(),  shape.part(60).x(), shape.part(60).y());
		
	}
}

//--------------------------------------------------------------
void ofApp::drawTriangles(){
	
	int ntris = triangles.size();
	int a, b, c;
	ofPoint ca, cb, cc; // colors
	std::vector < ofPoint > colors = {
		ofPoint(1, 0, 0),
		ofPoint(1, 0.5, 0),
		ofPoint(1, 1, 0),
		ofPoint(0.5, 1, 0),
		ofPoint(0, 1, 0),
		ofPoint(0, 1, 0.5),
		ofPoint(0, 1, 1),
		ofPoint(0, 0.5, 1),
		ofPoint(0, 0, 1),
		ofPoint(0.5, 0, 1),
		ofPoint(1, 0, 1),
		ofPoint(1, 0, 0.5),
	};
	
	glBegin(GL_TRIANGLES);
	for(int i = 0; i < shapes.size(); i++){
		full_object_detection shape = shapes[i];
		if(shape.num_parts() < 68){
			continue;
		}
		
		for(int t = 0; t < ntris; t ++){
			a = triangles[t][0];
			b = triangles[t][1];
			c = triangles[t][2];
			ca = colors[a % colors.size()];
			cb = colors[b % colors.size()];
			cc = colors[c % colors.size()];
			
			glColor3f(ca.x, ca.y, ca.z);
			glVertex2f(shape.part(a).x(), shape.part(a).y() );
			glColor3f(cb.x, cb.y, cb.z);
			glVertex2f(shape.part(b).x(), shape.part(b).y() );
			glColor3f(cc.x, cc.y, cc.z);
			glVertex2f(shape.part(c).x(), shape.part(c).y() );
			
			
		}
		
	}
	glEnd();
}

//--------------------------------------------------------------
void ofApp::drawTextured(){
	
	ofTexture  * tex = &(faceTexture.getTexture());
	
	float w = faceTexture.getWidth();
	float h = faceTexture.getHeight();
	float tx = w/2;
	float ty = h/2;
	
	ofFill();
	glEnable(tex->texData.textureTarget);
	glBindTexture(tex->texData.textureTarget, (GLuint)tex->texData.textureID );
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_TRIANGLES);
	int ntris = fullTriangles.size();
	int a, b, c;
	
	for(int i = 0; i < shapes.size(); i++){
		full_object_detection shape = shapes[i];
		if(shape.num_parts() < 68){
			continue;
		}
		
		for(int t = 0; t < ntris; t ++){
			a = fullTriangles[t][0];
			b = fullTriangles[t][1];
			c = fullTriangles[t][2];
			
			glTexCoord2f(tx + w * texPts[a]->x, ty + h * texPts[a]->y);
			glVertex2f(shape.part(a).x(), shape.part(a).y() );
			
			glTexCoord2f(tx + w * texPts[b]->x, ty + h * texPts[b]->y);
			glVertex2f(shape.part(b).x(), shape.part(b).y() );
			
			glTexCoord2f(tx + w * texPts[c]->x, ty + h * texPts[c]->y);
			glVertex2f(shape.part(c).x(), shape.part(c).y() );
		}
	}
	
	glEnd();
	glDisable(tex->texData.textureTarget);
	
}

//--------------------------------------------------------------
void ofApp::drawSwapped(){
	
	ofTexture  * tex = &(cam.getTexture());
	
	float w = faceTexture.getWidth();
	float h = faceTexture.getHeight();
	float tx = w/2;
	float ty = h/2;
	
	ofFill();
	glEnable(tex->texData.textureTarget);
	glBindTexture(tex->texData.textureTarget, (GLuint)tex->texData.textureID );
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_TRIANGLES);
	int ntris = fullTriangles.size();
	int a, b, c;
	
	for(int i = 0; i < shapes.size(); i++){
		full_object_detection shape = shapes[i];
		full_object_detection texShape = shapes[(i+1)%shapes.size()];
		if(shape.num_parts() < 68){
			continue;
		}
		
		for(int t = 0; t < ntris; t ++){
			a = fullTriangles[t][0];
			b = fullTriangles[t][1];
			c = fullTriangles[t][2];
			
			glTexCoord2f(texShape.part(a).x(), texShape.part(a).y() );
			glVertex2f(shape.part(a).x(), shape.part(a).y() );
			
			glTexCoord2f(texShape.part(b).x(), texShape.part(b).y());
			glVertex2f(shape.part(b).x(), shape.part(b).y());
			
			glTexCoord2f(texShape.part(c).x(), texShape.part(c).y() );
			glVertex2f(shape.part(c).x(), shape.part(c).y() );
		}
	}
	
	glEnd();
	glDisable(tex->texData.textureTarget);
	
}


//MARK: -

//--------------------------------------------------------------
void ofApp::setupFaceTriangles(){
	
	triangles = {
		
		// left cheek
		{ 0, 1, 36},
		{ 1, 2, 36},
		{ 36, 2, 41},
		{ 2, 3, 41},
		{ 40, 41, 3},
		{ 3, 31, 40},
		
		// cheek to nose
		{ 31, 3, 48},
		{ 3, 4, 48},
		{ 4, 5, 48},
		
		// right cheek
		{ 16, 45, 15},
		{ 15, 45, 14},
		{ 45, 46, 14},
		{ 47, 35, 46},
		{ 46, 35, 14},
		{ 35, 13, 14},
		
		{ 35, 54, 13 },
		{ 54, 12, 13},
		{ 54, 11, 12 },
		
		
		// eyebrows nose
		{21, 27, 22},
		
		// left eyebrow
		{ 0, 36, 17},
		{ 17, 36, 18},
		{ 18, 36, 37},
		{ 18, 37, 19},
		{ 19, 37, 20},
		{ 20, 37, 38},
		{ 20, 38, 21},
		{ 21, 38, 39},
		{ 21, 39, 27},
		
		// left eye nose
		//{ 39, 28, 27},
		//{ 39, 29, 28},
		//{ 40, 29, 39},
		//{ 40, 30, 29},
		//{ 40, 31, 30},
		{ 29, 31, 30 },
		{ 28, 31, 29 },
		{ 27, 31, 28 },
		
		{ 39, 40, 31},
		{ 39, 31, 27},

		
		
		// right eyebrow
		{ 26, 45, 16},
		{ 26, 25, 45},
		{ 25, 44, 45},
		{ 25, 24, 44},
		{ 24, 43, 44},
		{ 24, 23, 43},
		{ 23, 42, 43},
		{ 23, 22, 42},
		{ 22, 27, 42},
		
		// right eye nose
		//{ 27, 28, 42},
		//{ 42, 28, 29},
		//{ 42, 29, 47},
		//{ 47, 29, 30},
		//{ 47, 30, 35},
		{ 27, 35, 42},
		{ 42, 35, 47},
		{ 27, 28, 35},
		{ 28, 29, 35},
		{ 29, 30, 35},
		
		// NOSE
		{ 30, 31, 32},
		{ 30, 32, 33},
		{ 30, 33, 34},
		{ 30, 34, 35},
		
		// nose - mouth (from center - left)
		{ 33, 50, 51},
		{ 33, 32, 50},
		{ 32, 31, 50},
		{ 31, 49, 50},
		{ 31, 48, 49},
		
		// nose - mouth (from center - right)
		{ 33, 51, 52},
		{ 33, 52, 34},
		{ 34, 52, 35},
		{ 35, 52, 53},
		{ 35, 53, 54},
		
		// Mouth top left
		{ 51, 50, 62},
		{ 50, 61, 62},
		{ 50, 49, 61},
		{ 49, 60, 61},
		{ 49, 48, 60},
		// Mouth top right
		{ 51, 62, 52},
		{ 52, 62, 63},
		{ 52, 63, 53},
		{ 53, 63, 64},
		{ 53, 64, 54},
		// Mouth bottom left
		{ 66, 58, 57},
		{ 66, 67, 58},
		{ 67, 59, 58},
		{ 67, 60, 59},
		{ 60, 48, 59},
		// Mouth bottom right
		{ 66, 57, 56},
		{ 66, 56, 65},
		{ 65, 56, 55},
		{ 65, 55, 64},
		{ 64, 55, 54},
		
		// Mouth - chin
		{ 5, 6, 48},
		{ 48, 6, 59},
		{ 59, 6, 7},
		{ 59, 7, 58},
		{ 58, 7, 57},
		{ 57, 7, 8},
		{ 57, 8, 9},
		{ 57, 9, 56},
		{ 56, 9, 55},
		{ 55, 9, 10},
		{ 55, 10, 54},
		{ 54, 10, 11},
		
	};
	fullTriangles = triangles;
	std::vector < std::vector < int > > extraTris;
	extraTris = {
		// close left eye
		{ 37, 36, 41},
		{ 37, 41, 38},
		{ 38, 41, 40},
		{ 38, 40, 39},
		// close right eye
		{ 43, 42, 47},
		{ 43, 47, 44},
		{ 44, 47, 46},
		{ 44, 46, 45},
		
		//close mouth
		{ 61, 60, 67},
		{ 61, 67, 66},
		{ 61, 66, 62},
		{ 62, 66, 63},
		{ 63, 66, 65},
		{ 63, 65, 64},
	};
	fullTriangles.insert(fullTriangles.end(), extraTris.begin(), extraTris.end());
}

//--------------------------------------------------------------
void ofApp::setupFaceTexturePoints(){
 texPts = {
	 // texture points
	 new ofPoint(-0.5000, -0.3160 ), // 0
	 new ofPoint(-0.4920, -0.1900 ), // 1
	 new ofPoint(-0.4740, -0.0460 ), // 2
	 new ofPoint(-0.4540, 0.0900 ), // 3
	 new ofPoint(-0.4260, 0.2460 ), // 4
	 new ofPoint(-0.3580, 0.3300 ), // 5
	 new ofPoint(-0.2420, 0.4220 ), // 6
	 new ofPoint(-0.1220, 0.4866 ), // 7
	 new ofPoint(0.0020, 0.4980 ), // 8
	 new ofPoint(0.1300, 0.4786 ), // 9
	 new ofPoint(0.2426, 0.4118 ), // 10
	 new ofPoint(0.3460, 0.3240 ), // 11
	 new ofPoint(0.4240, 0.2480 ), // 12
	 new ofPoint(0.4520, 0.0840 ), // 13
	 new ofPoint(0.4705, -0.0481 ), // 14
	 new ofPoint(0.4893, -0.1845 ), // 15
	 new ofPoint(0.5040, -0.3220 ), // 16
	 new ofPoint(-0.4410, -0.4011 ), // 17
	 new ofPoint(-0.3794, -0.4759 ), // 18
	 new ofPoint(-0.2700, -0.4980 ), // 19
	 new ofPoint(-0.1500, -0.4980 ), // 20
	 new ofPoint(-0.0580, -0.4480 ), // 21
	 new ofPoint(0.0540, -0.4460 ), // 22
	 new ofPoint(0.1380, -0.4940 ), // 23
	 new ofPoint(0.2620, -0.4920 ), // 24
	 new ofPoint(0.3720, -0.4780 ), // 25
	 new ofPoint(0.4360, -0.3980 ), // 26
	 new ofPoint(0.0000, -0.3700 ), // 27
	 new ofPoint(0.0000, -0.2860 ), // 28
	 new ofPoint(-0.0040, -0.1880 ), // 29
	 new ofPoint(-0.0020, -0.0820 ), // 30
	 new ofPoint(-0.1160, 0.0280 ), // 31
	 new ofPoint(-0.0540, 0.0660 ), // 32
	 new ofPoint(-0.0020, 0.0880 ), // 33
	 new ofPoint(0.0560, 0.0640 ), // 34
	 new ofPoint(0.1180, 0.0280 ), // 35
	 new ofPoint(-0.3000, -0.3240 ), // 36
	 new ofPoint(-0.2540, -0.3540 ), // 37
	 new ofPoint(-0.1980, -0.3540 ), // 38
	 new ofPoint(-0.1380, -0.3160 ), // 39
	 new ofPoint(-0.1980, -0.2920 ), // 40
	 new ofPoint(-0.2420, -0.2920 ), // 41
	 new ofPoint(0.1320, -0.3180 ), // 42
	 new ofPoint(0.1900, -0.3540 ), // 43
	 new ofPoint(0.2540, -0.3560 ), // 44
	 new ofPoint(0.3040, -0.3200 ), // 45
	 new ofPoint(0.2460, -0.2920 ), // 46
	 new ofPoint(0.1920, -0.2920 ), // 47
	 new ofPoint(-0.1780, 0.2480 ), // 48
	 new ofPoint(-0.1120, 0.2020 ), // 49
	 new ofPoint(-0.0460, 0.1780 ), // 50
	 new ofPoint(0.0000, 0.1920 ), // 51
	 new ofPoint(0.0460, 0.1820 ), // 52
	 new ofPoint(0.1060, 0.2020 ), // 53
	 new ofPoint(0.1760, 0.2480 ), // 54
	 new ofPoint(0.1180, 0.2920 ), // 55
	 new ofPoint(0.0520, 0.3140 ), // 56
	 new ofPoint(0.0000, 0.3160 ), // 57
	 new ofPoint(-0.0540, 0.3160 ), // 58
	 new ofPoint(-0.1240, 0.2920 ), // 59
	 new ofPoint(-0.1380, 0.2480 ), // 60
	 new ofPoint(-0.0600, 0.2260 ), // 61
	 new ofPoint(0.0000, 0.2240 ), // 62
	 new ofPoint(0.0540, 0.2260 ), // 63
	 new ofPoint(0.1300, 0.2480 ), // 64
	 new ofPoint(0.0560, 0.2480 ), // 65
	 new ofPoint(0.0000, 0.2480 ), // 66
	 new ofPoint(-0.0620, 0.2480 ), // 67
 };
}

//MARK: - EDIT TEXTURE


//--------------------------------------------------------------
bool ofApp::clickPts(ofPoint p){
	selPt = NULL;
	float w = faceTexture.getWidth();
	float h = faceTexture.getHeight();
	float mx = ofGetWidth()/2;
	float my = ofGetHeight()/2;
	if( w == 0 || h == 0){
		return;
	}
	p -= ofPoint(mx, my);
	p.x /= w;
	p.y /= h;
	float r = 0.01;
	for(int i = 0; i < texPts.size(); i++){
		ofPoint * pt = texPts[i];
		if( p.x > pt->x - r &&  p.x < pt->x + r &&  p.y > pt->y - r &&  p.y < pt->y + r ){
			selPt = pt;
			return true;
		}
	}
	return false;
}

//--------------------------------------------------------------
bool ofApp::drag(ofPoint p){
	if(selPt != NULL){
		
		float w = faceTexture.getWidth();
		float h = faceTexture.getHeight();
		float mx = ofGetWidth()/2;
		float my = ofGetHeight()/2;
		if( w == 0 || h == 0){
			return;
		}
		p -= ofPoint(mx, my);
		p.x /= w;
		p.y /= h;
		selPt->set(p);
		return true;
	}
}

//--------------------------------------------------------------
void ofApp::drawFlatFace(){
	ofTexture  * tex = &(cam.getTexture());
	
	float w = faceTexture.getWidth();
	float h = faceTexture.getHeight();
	float tx = 0;
	float ty = 0;
	
	ofFill();
	glEnable(tex->texData.textureTarget);
	glBindTexture(tex->texData.textureTarget, (GLuint)tex->texData.textureID );
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_TRIANGLES);
	int ntris = fullTriangles.size();
	int a, b, c;
	
	if( shapes.size() > 0){
		full_object_detection texShape = shapes[0];
		
		if(texShape.num_parts() >= 68){
			
			for(int t = 0; t < ntris; t ++){
				a = fullTriangles[t][0];
				b = fullTriangles[t][1];
				c = fullTriangles[t][2];
				
				glTexCoord2f(texShape.part(a).x(), texShape.part(a).y() );
				glVertex2f(tx + w * texPts[a]->x, ty + h * texPts[a]->y);
				
				glTexCoord2f(texShape.part(b).x(), texShape.part(b).y());
				glVertex2f(tx + w * texPts[b]->x, ty + h * texPts[b]->y);
				
				glTexCoord2f(texShape.part(c).x(), texShape.part(c).y() );
				glVertex2f(tx + w * texPts[c]->x, ty + h * texPts[c]->y);
				
			}
		}
		
		
	}
	
	glEnd();
	glDisable(tex->texData.textureTarget);
	

}


//--------------------------------------------------------------
void ofApp::drawTexturePoints(){
	// draw texture image
	float w = faceTexture.getWidth();
	float h = faceTexture.getHeight();
	
	ofPushMatrix();
	ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
	if(bDrawCamera){
		drawFlatFace();
	}else{
		ofSetColor(255,255,255);
		faceTexture.draw(-w/2, -h/2);
	}
	
	// draw triangles
	ofNoFill();
	int ntris = fullTriangles.size();
	int a, b, c;
	ofPoint ca, cb, cc; // colors
	std::vector < ofPoint > colors = {
		ofPoint(1, 0, 0),
		ofPoint(1, 0.5, 0),
		ofPoint(1, 1, 0),
		ofPoint(0.5, 1, 0),
		ofPoint(0, 1, 0),
		ofPoint(0, 1, 0.5),
		ofPoint(0, 1, 1),
		ofPoint(0, 0.5, 1),
		ofPoint(0, 0, 1),
		ofPoint(0.5, 0, 1),
		ofPoint(1, 0, 1),
		ofPoint(1, 0, 0.5),
	};
	
	ofPushMatrix();
	ofScale(w, h);
	ofSetColor(255,0,0);
	if(texPts.size() >= 68){
		glBegin(GL_TRIANGLES);
		for(int t = 0; t < ntris; t ++){
			a = fullTriangles[t][0];
			b = fullTriangles[t][1];
			c = fullTriangles[t][2];
			ca = colors[a % colors.size()];
			cb = colors[b % colors.size()];
			cc = colors[c % colors.size()];
			
			//glColor3f(ca.x, ca.y, ca.z);
			glVertex2f(texPts[a]->x, texPts[a]->y );
			//glColor3f(cb.x, cb.y, cb.z);
			glVertex2f(texPts[b]->x, texPts[b]->y );
			//glColor3f(cc.x, cc.y, cc.z);
			glVertex2f(texPts[c]->x, texPts[c]->y );
			
			
		}
		glEnd();
	}
	ofPopMatrix();
	
	// draw points
	
	ofSetColor(255);
	ofFill();
	for(int i = 0; i < texPts.size(); i++){
		ofDrawEllipse( w * texPts[i]->x, h * texPts[i]->y, 7, 7);
	}
	ofSetColor(0);
	ofFill();
	for(int i = 0; i < texPts.size(); i++){
		ofDrawEllipse( w * texPts[i]->x, h * texPts[i]->y, 5, 5);
		ofDrawBitmapString(ofToString(i), w * texPts[i]->x + 5,  h * texPts[i]->y + 5);

	}
	
	if(selPt != NULL){
		ofSetColor(255);
		ofDrawEllipse( w * selPt->x, h * selPt->y, 9, 9);
		
		ofSetColor(255,0,0);
		ofDrawEllipse( w * selPt->x, h * selPt->y, 7, 7);
	}
	
	ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::unselectPts(){
	selPt = NULL;
}

//--------------------------------------------------------------
void ofApp::exportFacePoints(){
	string output = "";
	
	float x,y, w, h;
	for(int i = 0; i < shapes.size(); i++){
		full_object_detection shape = shapes[i];
		ofPoint minPt ( 999999, 999999);
		ofPoint maxPt (-999999,-999999);
		for(int j = 0; j < shape.num_parts(); j++){
			minPt.x = MIN(shape.part(j).x(), minPt.x);
			minPt.y = MIN(shape.part(j).y(), minPt.y);
			maxPt.x = MAX(shape.part(j).x(), maxPt.x);
			maxPt.y = MAX(shape.part(j).y(), maxPt.y);
		}
		
		x = ((minPt + maxPt)/2).x;
		y = ((minPt + maxPt)/2).y;
		w = (maxPt - minPt).x;
		h = (maxPt - minPt).y;
		
		if( w > 0 && h > 0){
			output += "\n texPts = {";
			output += "\n\t// Face Nr."+ofToString(i)+" width = " + ofToString(w) + ", height = " + ofToString(h);
			for(int j = 0; j < shape.num_parts(); j++){
				output += "\n\tnew ofPoint(" + ofToString((shape.part(j).x()-x)/w ,4) +", " +
				ofToString((shape.part(j).y()-y)/h ,4) + " ), // " + ofToString(j);
			}
			output += "\n};\n";
			
		}
	}
	
	cout << "FACES : " << endl << output << endl;
}

//--------------------------------------------------------------
void ofApp::exportTexturePoints(){
	string output = "";
	
	output += "\n texPts = {";
	output += "\n\t// texture points";
	for(int i = 0; i < texPts.size(); i++){
		output += "\n\tnew ofPoint(" + ofToString(texPts[i]->x, 4) +", " +
		ofToString(texPts[i]->y, 4) + " ), // " + ofToString(i);
	}
	output += "\n};\n";
	
	cout << "FACE POINTS : " << endl << output << endl;
}



//--------------------------------------------------------------
void ofApp::setupCamera(ofVideoGrabber &vidGrabber){
	
	std::vector <ofVideoDevice> cams =  vidGrabber.listDevices();
	cout << endl << cams.size() << " Video Devices: " << endl;
	int selDevice = 0;
	for(int i = 0; i < cams.size(); i++){
		if(cams[i].deviceName.find("C920") != string::npos){
			selDevice = i;
			cout << " * ";
		}else{
			cout << "   " ;
		}
		std::vector <ofVideoFormat> formats = cams[i].formats;
		
		cout << i << " device: " << cams[i].deviceName
		<< " id: " << cams[i].id
		<< " formats: " << formats.size()
		<< endl;
		for( int j = 0; j < formats.size(); j++){
			cout << "\t" << formats[j].width << " x " << formats[j].height << endl;
			for(int k = 0; k < formats[j].framerates.size(); k++){
				cout << "\t\t " << formats[j].framerates[k] << "fps" << endl;
			}
		}
	}
	cout << "____________" << endl << endl;
	vidGrabber.setDeviceID(selDevice);
	vidGrabber.setUseTexture(false);
	vidGrabber.setup( 640, 480);
	
	//	1280 x 960	//	640 x 480	//	320 x 240
	//  1280 x 720	//  640 x 360	//	320 x 180
	//	1920 x 1080	//	960 x 540	//	480 x 270
	
	
	//Setup OpenCV
	int w = vidGrabber.getWidth();
	int h = vidGrabber.getHeight();
	cout << "Started camera with dimensions " << w << " x " << h << endl;
}


#pragma mark - BUTTONS

//----------------------------------------------------------------------
void ofApp::setupButtons() {
	
	
	buttons.setup();
	
	
	buttons.addButtonPanel("DLib");
	buttons.addToggleItem("Use DLib [D]", bUseDLib);
	buttons.addSliderItem("FPS", 0, 200, fps);
	buttons.addToggleItem("Scale up", bScaleUp);
	
	buttons.addButtonPanel("Drawing");
	buttons.addSelectionItem("Points", DRAW_POINTS, drawState);
	buttons.addSelectionItem("Connections", DRAW_CONNECTIONS, drawState);
	buttons.addSelectionItem("Triangles", DRAW_TRIANGLES, drawState);
	buttons.addSelectionItem("Texture", DRAW_TEXTURED, drawState);
	buttons.addSelectionItem("Swap", DRAW_SWAP, drawState);

	buttons.addSelectionItem("Edit Points", EDIT_POINTS, drawState);
	buttons.addToggleItem("Show Camera [C]", bDrawCamera);
	
	buttons.addButtonPanel("Tools");
	buttons.addListItem("Press [SPACE] to save the Photo");
	buttons.addListItem("Fullscreen [F]");
	
	buttons.loadSettings();
}


#pragma mark - EVENTS

//--------------------------------------------------------------
void ofApp::keyPressed (int key) {
	switch (key) {
		case ' ':
			bSaveFrame = true;
			break;
		case 's':
			buttons.saveSettings();
			break;
		case 'd':
			bUseDLib = !bUseDLib;
			break;
		case 'e':
			exportFacePoints();
			break;
		case 'p':
			exportTexturePoints();
			break;
		case 'c':
			bDrawCamera = !bDrawCamera;
	}
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	if(drawState == EDIT_POINTS){
		drag(ofPoint(x,y));
	}
}
//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	if(drawState == EDIT_POINTS){
		clickPts(ofPoint(x,y));
	}
}
//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	if(drawState == EDIT_POINTS){
		unselectPts();
	}
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
	cout << "Dropping image file! " << endl;
	for(int i = 0; i < dragInfo.files.size(); i++){
		string fileName = dragInfo.files[i];
		cout << "Loading new textur : " << fileName << endl;
		faceTexture.load(fileName);
		return; // only use first file!
	}
}


#pragma mark - HELPERS

//--------------------------------------------------------------
void ofApp::setDLibImageFromPixels( array2d<unsigned char> &newImg, unsigned char * pix, int w, int h, int ofPixelType){
	
	image_view< array2d <unsigned char> > t(newImg);
	if(newImg.nc() != w || newImg.nr() != h){
		//cout << "Rescale DLIB img from "<< newImg.nc() << "x"<< newImg.nr() << " to " << w << "x" << h << endl;
		newImg.set_size( h, w );
	}
	for ( unsigned y = 0; y < h; y++ ){ // y
		for ( unsigned x = 0; x < w; x++ ){ // x
			if ( ofPixelType == OF_IMAGE_GRAYSCALE ){
				unsigned char p = pix[x + y * w];
				assign_pixel( newImg[y][x], p );
			}else if( ofPixelType == OF_IMAGE_COLOR){ // assume RGB!
				rgb_pixel p;
				p.red	= pix[3 * (x + y * w) ];
				p.green = pix[3 * (x + y * w)+1];
				p.blue	= pix[3 * (x + y * w)+2];
				assign_pixel( newImg[y][x], p );
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::imgToDLibImage( array2d<unsigned char> &newImg, ofImage &srcImg){
	
	//array2d <unsigned char> newImg;
	
	unsigned h = srcImg.getHeight();
	unsigned w = srcImg.getWidth();
	unsigned char * pix = srcImg.getPixels();
	
	image_view< array2d <unsigned char> > t(newImg);
	
	int ofPixelType = srcImg.getImageType();
	cout << "image type is " << ofPixelType << endl;
	newImg.set_size( h, w );
	
	for ( unsigned y = 0; y < h; y++ ){ // y
		for ( unsigned x = 0; x < w; x++ ){ // x
			if ( ofPixelType == OF_IMAGE_GRAYSCALE ){
				unsigned char p = pix[x + y * w];
				assign_pixel( newImg[y][x], p );
			}else if( ofPixelType == OF_IMAGE_COLOR){ // assume RGB!
				rgb_pixel p;
				p.red	= pix[3 * (x + y * w) ];
				p.green = pix[3 * (x + y * w)+1];
				p.blue	= pix[3 * (x + y * w)+2];
				assign_pixel( newImg[y][x], p );
			}
		}
	}
}


//--------------------------------------------------------------
void ofApp::DLibImageToImg( array2d<unsigned char> &srcImg, ofImage &destImg){
	
	//image_view< array2d <unsigned char> > t(srcImg);
	
	unsigned h = srcImg.nr(); // number of rows
	unsigned w = srcImg.nc(); // number of cols
	srcImg.width_step();
	destImg.clear();
	destImg.allocate(w, h, OF_IMAGE_COLOR);
	
	unsigned char * pix = destImg.getPixels();
	
	
	
	/*
	 int colorMode = srcImg.getImageType();
	 
	 newImg.set_size( h, w );
	 
	 for ( unsigned n = 0; n < h;n++ ){ // y
		unsigned char* v = &(pix[ n * w]);
		
		for ( unsigned m = 0; m < w; m++ ){ // x
	 if ( colorMode == OF_IMAGE_GRAYSCALE ){
	 unsigned char p = v[m];
	 assign_pixel( newImg[n][m], p );
	 }else if( colorMode == OF_IMAGE_COLOR){ // assume RGB!
	 rgb_pixel p;
	 p.red = v[m*3];
	 p.green = v[m*3+1];
	 p.blue = v[m*3+2];
	 assign_pixel( newImg[n][m], p );
	 }
		}
	 }
	 */
}

