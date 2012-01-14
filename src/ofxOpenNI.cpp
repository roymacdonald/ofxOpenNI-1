/*
 * ofxOpenNI.cpp
 *
 * Copyright 2011 (c) Matthew Gingold [gameover] http://gingold.com.au
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "ofxOpenNI.h"

static int instanceCount = -1;
static string CALLBACK_LOG_NAME = "ofxOpenNIUserCB";

//--------------------------------------------------------------
ofxOpenNI::ofxOpenNI(){
	
	instanceCount++;
	instanceID = instanceCount; // TODO: this should be replaced/combined with a listDevices and setDeviceID methods
	
    LOG_NAME = "ofxOpenNIDevice[" + ofToString(instanceID) + "]";
    
	bIsThreaded = false;
	
	g_bIsDepthOn = false;
	g_bIsDepthRawOnOption = false;
	g_bIsImageOn = false;
	g_bIsIROn = false;
    g_bIsUserOn = false;
	g_bIsAudioOn = false;
	g_bIsPlayerOn = false;
	
	depthColoring = COLORING_RAINBOW;
	
    bNeedsPose = true;
	bUseTexture = true;
	bNewPixels = false;
	bNewFrame = false;	
	
	CreateRainbowPallet();
	
}

//--------------------------------------------------------------
ofxOpenNI::~ofxOpenNI(){
    
    //openNIContext->setPaused(true);
    
    if (isThreadRunning()){
        lock();
        stopThread();
	}
    
    //ofLogVerbose(LOG_NAME) << "Shut down device:" << instanceID; // strange if I call at top of dtor I crash!?!?!
    
    //openNIContext->setPaused(false);
    
//    if (g_bIsUserOn) {
//        g_bIsUserOn = false;
//        openNIContext->stopUserNode(instanceID);
//    }
//    if (g_bIsDepthOn) {
//        g_bIsDepthOn = false;
//        openNIContext->stopDepthNode(instanceID);
//    }
//	if (g_bIsImageOn) {
//        g_bIsImageOn = false;
//        openNIContext->stopImageNode(instanceID);
//    }
//	if (g_bIsIROn) {
//        g_bIsIROn = false;
//        openNIContext->stopInfraNode(instanceID);
//    }

}

//--------------------------------------------------------------
bool ofxOpenNI::setup(bool threaded){
	XnStatus nRetVal = XN_STATUS_OK;
	bIsThreaded = threaded;
	
	if (!openNIContext->getIsContextReady()){
		if (!openNIContext->initContext()){
			ofLogError(LOG_NAME) << "Context could not be intitilised";
			return false;
		}
	}
	
	if (openNIContext->getNumDevices() > 0 && openNIContext->getNumDevices() > instanceID){
        ofLogNotice(LOG_NAME) << "Starting device for instance [" << instanceID << "] of ofxOpenNI";
		if (bIsThreaded) startThread(true, false);
	} else {
		ofLogError(LOG_NAME) << "No Devices available for this instance [" << instanceID << "] of ofxOpenNI - have you got a device connected?!?!";
        return false;
	}
	return true;
}

//--------------------------------------------------------------
bool ofxOpenNI::addDepthGenerator(){
	XnStatus nRetVal = XN_STATUS_OK;
    g_bIsDepthOn = false;
	ofLogNotice(LOG_NAME) << "Adding depth generator...";
	if (!openNIContext->getIsContextReady()){
		ofLogError(LOG_NAME) << "Context is not setup - please call ofxOpenNI::setup() first";
	}
    if (!openNIContext->addDepthNode(instanceID)){
        ofLogError(LOG_NAME) << "Could not add depth generator node to context";
    }
	if (openNIContext->getDepthGenerator(instanceID).IsValid()){
        allocateDepthBuffers();
		nRetVal = openNIContext->getDepthGenerator(instanceID).StartGenerating();
		SHOW_RC(nRetVal, "Starting depth generator");
        g_bIsDepthOn = (nRetVal == XN_STATUS_OK);
	} else {
		ofLogError(LOG_NAME) << "Depth generator is invalid!";
	}
	return g_bIsDepthOn;
}

//--------------------------------------------------------------
bool ofxOpenNI::setup(string xmlFilePath, bool threaded){
	ofLogWarning(LOG_NAME) << "Not yet implimented";
}

//--------------------------------------------------------------
bool ofxOpenNI::addImageGenerator(){
	XnStatus nRetVal = XN_STATUS_OK;
    g_bIsImageOn = false;
	ofLogNotice(LOG_NAME) << "Adding image generator...";
	if (!openNIContext->getIsContextReady()){
		ofLogError(LOG_NAME) << "Context is not setup - please call ofxOpenNI::setup() first";
	}
    if (!openNIContext->addImageNode(instanceID)){
        ofLogError(LOG_NAME) << "Could not add image generator node to context";
    }
	if (openNIContext->getImageGenerator(instanceID).IsValid()){
        allocateImageBuffers();
		nRetVal = openNIContext->getImageGenerator(instanceID).StartGenerating();
		SHOW_RC(nRetVal, "Starting image generator");
        g_bIsImageOn = (nRetVal == XN_STATUS_OK);
	} else {
		ofLogError(LOG_NAME) << "Image generator is invalid!";
	}
	return g_bIsImageOn;
}

//--------------------------------------------------------------
bool ofxOpenNI::addInfraGenerator(){
	XnStatus nRetVal = XN_STATUS_OK;
    g_bIsIROn = false;
	ofLogNotice(LOG_NAME) << "Adding ir generator...";
	if (!openNIContext->getIsContextReady()){
		ofLogError(LOG_NAME) << "Context is not setup - please call ofxOpenNI::setup() first";
	}
    if (!openNIContext->addInfraNode(instanceID)){
        ofLogError(LOG_NAME) << "Could not add infra generator node to context";
    }
	if (openNIContext->getIRGenerator(instanceID).IsValid()){
        allocateIRBuffers();
		nRetVal = openNIContext->getIRGenerator(instanceID).StartGenerating();
		SHOW_RC(nRetVal, "Starting ir generator");
        g_bIsIROn = (nRetVal == XN_STATUS_OK);
	} else {
		ofLogError(LOG_NAME) << "Image generator is invalid!";
	}
	return g_bIsIROn;
}

//--------------------------------------------------------------
bool ofxOpenNI::addUserGenerator(){
	
    if (instanceID > 0) {
        ofLogWarning(LOG_NAME) << "Currently it seems only possible to have a user generator on one device!!";
        // some people say it could be done like thus: http://openni-discussions.979934.n3.nabble.com/OpenNI-dev-Skeleton-tracking-with-multiple-kinects-not-solved-with-new-OpenNI-td2832613.html ... but itdidn't work for me .... 
        // ok it's not possible according to: http://groups.google.com/group/openni-dev/browse_thread/thread/188a2ac823584117
        return false;   // uncomment this to see what happens -> not the weird error where 10 (!) user generators are enumerated with my current method
                        // if I create them all at once then I get 4 like in the above link, but still no dice...hrmph!
    }
    
    XnStatus nRetVal = XN_STATUS_OK;
    g_bIsUserOn = false;
    ofLogNotice(LOG_NAME) << "Adding user generator...";
    if (!openNIContext->getIsContextReady()){
		ofLogError(LOG_NAME) << "Context is not setup - please call ofxOpenNI::setup() first";
	}
    if (!openNIContext->addUserNode(instanceID)){
        ofLogError(LOG_NAME) << "Could not add user generator node to context";
    }
    if (openNIContext->getUserGenerator(instanceID).IsValid()){
        allocateUsers();
		nRetVal = openNIContext->getUserGenerator(instanceID).StartGenerating();
		SHOW_RC(nRetVal, "Starting user generator");
        g_bIsUserOn = (nRetVal == XN_STATUS_OK);
	} else {
		ofLogError(LOG_NAME) << "User generator is invalid!";
	}
	return g_bIsUserOn;
}

//--------------------------------------------------------------
bool ofxOpenNI::addAudioGenerator(){
	ofLogWarning(LOG_NAME) << "Not yet implimented";
}

//--------------------------------------------------------------
bool ofxOpenNI::addPlayerGenerator(){
	ofLogWarning(LOG_NAME) << "Not yet implimented";
}

//--------------------------------------------------------------
void ofxOpenNI::allocateDepthBuffers(){
    // make new map mode -> default to 640 x 480 @ 30fps
    // TODO: shift this to a setSize or make part of int/setup request
    XnMapOutputMode mapMode;
    XnStatus nRetVal = XN_STATUS_OK;
    mapMode.nXRes = XN_VGA_X_RES;
    mapMode.nYRes = XN_VGA_Y_RES;
    mapMode.nFPS  = 30;
    nRetVal = openNIContext->getDepthGenerator(instanceID).SetMapOutputMode(mapMode);
    maxDepth = openNIContext->getDepthGenerator(instanceID).GetDeviceMaxDepth();
    SHOW_RC(nRetVal, "Setting depth resolution: " + ofToString(mapMode.nXRes) + " x " + ofToString(mapMode.nYRes) + " at " + ofToString(mapMode.nFPS) + "fps with max depth of " + ofToString(maxDepth));
    depthPixels[0].allocate(mapMode.nXRes, mapMode.nYRes, OF_IMAGE_COLOR_ALPHA);
    depthPixels[1].allocate(mapMode.nXRes, mapMode.nYRes, OF_IMAGE_COLOR_ALPHA);
    currentDepthPixels = &depthPixels[0];
    backDepthPixels = &depthPixels[1];
    if (bUseTexture) depthTexture.allocate(mapMode.nXRes, mapMode.nYRes, GL_RGBA);
}

//--------------------------------------------------------------
void ofxOpenNI::allocateDepthRawBuffers(){
    // make new map mode -> default to 640 x 480 @ 30fps
    // TODO: shift this to a setSize or make part of int/setup request
    XnMapOutputMode mapMode;
    XnStatus nRetVal = XN_STATUS_OK;
    mapMode.nXRes = XN_VGA_X_RES;
    mapMode.nYRes = XN_VGA_Y_RES;
    mapMode.nFPS  = 30;
    nRetVal = openNIContext->getDepthGenerator(instanceID).SetMapOutputMode(mapMode);
    maxDepth = openNIContext->getDepthGenerator(instanceID).GetDeviceMaxDepth();
    SHOW_RC(nRetVal, "Setting depth resolution: " + ofToString(mapMode.nXRes) + " x " + ofToString(mapMode.nYRes) + " at " + ofToString(mapMode.nFPS) + "fps with max depth of " + ofToString(maxDepth));
    depthRawPixels[0].allocate(mapMode.nXRes, mapMode.nYRes, OF_PIXELS_MONO);
    depthRawPixels[1].allocate(mapMode.nXRes, mapMode.nYRes, OF_PIXELS_MONO);
    currentDepthRawPixels = &depthRawPixels[0];
    backDepthRawPixels = &depthRawPixels[1];
}

//--------------------------------------------------------------
void ofxOpenNI::allocateImageBuffers(){
    // make new map mode -> default to 640 x 480 @ 30fps
    // TODO: shift this to a setSize or make part of int/setup request
    XnMapOutputMode mapMode;
    XnStatus nRetVal = XN_STATUS_OK;
    mapMode.nXRes = XN_VGA_X_RES;
    mapMode.nYRes = XN_VGA_Y_RES;
    mapMode.nFPS  = 30;
    nRetVal = openNIContext->getImageGenerator(instanceID).SetMapOutputMode(mapMode);
    SHOW_RC(nRetVal, "Setting image resolution: " + ofToString(mapMode.nXRes) + " x " + ofToString(mapMode.nYRes) + " at " + ofToString(mapMode.nFPS) + "fps");
    imagePixels[0].allocate(mapMode.nXRes, mapMode.nYRes, OF_IMAGE_COLOR);
    imagePixels[1].allocate(mapMode.nXRes, mapMode.nYRes, OF_IMAGE_COLOR);
    currentImagePixels = &imagePixels[0];
    backImagePixels = &imagePixels[1];
    if (bUseTexture) imageTexture.allocate(mapMode.nXRes, mapMode.nYRes, GL_RGB);
}

//--------------------------------------------------------------
void ofxOpenNI::allocateIRBuffers(){
    // make new map mode -> default to 640 x 480 @ 30fps
    // TODO: shift this to a setSize or make part of int/setup request
    XnMapOutputMode mapMode;
    XnStatus nRetVal = XN_STATUS_OK;
    mapMode.nXRes = XN_VGA_X_RES;
    mapMode.nYRes = XN_VGA_Y_RES;
    mapMode.nFPS  = 30;
    nRetVal = openNIContext->getIRGenerator(instanceID).SetMapOutputMode(mapMode);
    SHOW_RC(nRetVal, "Setting ir resolution: " + ofToString(mapMode.nXRes) + " x " + ofToString(mapMode.nYRes) + " at " + ofToString(mapMode.nFPS) + "fps");
    imagePixels[0].allocate(mapMode.nXRes, mapMode.nYRes, OF_IMAGE_GRAYSCALE);
    imagePixels[1].allocate(mapMode.nXRes, mapMode.nYRes, OF_IMAGE_GRAYSCALE);
    currentImagePixels = &imagePixels[0];
    backImagePixels = &imagePixels[1];
    if (bUseTexture) imageTexture.allocate(mapMode.nXRes, mapMode.nYRes, GL_LUMINANCE);
}

//--------------------------------------------------------------
void ofxOpenNI::allocateUsers(){
    
    XnStatus nRetVal = XN_STATUS_OK;
    
    // set to verbose for now
    ofSetLogLevel(CALLBACK_LOG_NAME, OF_LOG_VERBOSE);
    
    // get user generator reference
    xn::UserGenerator& userGenerator = openNIContext->getUserGenerator(instanceID);
    
    // register user callbacks
    XnCallbackHandle User_CallbackHandler;
    XnCallbackHandle Calibration_CallbackHandler;
    
	nRetVal = userGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, this, User_CallbackHandler);
    CHECK_ERR_RC(nRetVal, "Register user New/Lost callbacks");
    
	nRetVal = userGenerator.GetSkeletonCap().RegisterToCalibrationStart(UserCalibration_CalibrationStart, this, Calibration_CallbackHandler);
    CHECK_ERR_RC(nRetVal, "Register user Calibration Start callback");
    
	nRetVal = userGenerator.GetSkeletonCap().RegisterToCalibrationComplete(UserCalibration_CalibrationEnd, this, Calibration_CallbackHandler);
    CHECK_ERR_RC(nRetVal, "Register user Calibration End callback");
    
    // check need for calibration
    if (userGenerator.GetSkeletonCap().NeedPoseForCalibration()){
        ofLogNotice(LOG_NAME) << "User generator DOES require pose for calibration";
        bNeedsPose = true;
        if(!userGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION)) {
			ofLogError(LOG_NAME) << "Pose required, but not supported";
		}
        XnCallbackHandle Pose_CallbackHandler;
        nRetVal = userGenerator.GetPoseDetectionCap().RegisterToPoseDetected(UserPose_PoseDetected, this, Pose_CallbackHandler);
        CHECK_ERR_RC(nRetVal, "Register user Pose Detected callback");
        
        nRetVal = userGenerator.GetSkeletonCap().GetCalibrationPose(userCalibrationPose);
        CHECK_ERR_RC(nRetVal, "Get calibration pose");
        
        //userGenerator.GetPoseDetectionCap().StartPoseDetection("Psi", user); 
    } else {
        ofLogNotice(LOG_NAME) << "User generator DOES NOT require pose for calibration";
        bNeedsPose = false;
        //userGenerator.GetSkeletonCap().RequestCalibration(user, TRUE); 
    } 
    
	nRetVal = userGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
    CHECK_ERR_RC(nRetVal, "Set skeleton profile");
    
}


//--------------------------------------------------------------
void ofxOpenNI::setUseMaskPixels(bool b){
	bUseMaskPixels = b;
}

//--------------------------------------------------------------
void ofxOpenNI::setUsePointClouds(bool b){
	bUsePointClouds = b;
}

//--------------------------------------------------------------
void ofxOpenNI::setSmoothing(float smooth){
	if (smooth > 0.0f && smooth < 1.0f) {
		userSmoothFactor = smooth;
		if (g_bIsUserOn) {
			openNIContext->getUserGenerator(instanceID).GetSkeletonCap().SetSmoothing(smooth);
		}
	}
}

//--------------------------------------------------------------
float ofxOpenNI::getSmoothing(){
	return userSmoothFactor;
}

//--------------------------------------------------------------
void ofxOpenNI::startPoseDetection(XnUserID nID){
    XnStatus nRetVal = XN_STATUS_OK;
	ofLogNotice(LOG_NAME) << "Start pose detection for user" << nID;
	nRetVal = openNIContext->getUserGenerator(instanceID).GetPoseDetectionCap().StartPoseDetection(userCalibrationPose, nID);
    SHOW_RC(nRetVal, "Get pose detection capability - start");
}


//--------------------------------------------------------------
void ofxOpenNI::stopPoseDetection(XnUserID nID){
    XnStatus nRetVal = XN_STATUS_OK;
    ofLogNotice(LOG_NAME) << "Stop pose detection for user" << nID;
	nRetVal = openNIContext->getUserGenerator(instanceID).GetPoseDetectionCap().StopPoseDetection(nID);
    CHECK_ERR_RC(nRetVal, "Get pose detection capability - stop");
}


//--------------------------------------------------------------
void ofxOpenNI::requestCalibration(XnUserID nID){
    XnStatus nRetVal = XN_STATUS_OK;
	ofLogNotice(LOG_NAME) << "Calibration requested for user" << nID;
	nRetVal = openNIContext->getUserGenerator(instanceID).GetSkeletonCap().RequestCalibration(nID, TRUE);
    CHECK_ERR_RC(nRetVal, "Get skeleton capability - request calibration");
}

//--------------------------------------------------------------
void ofxOpenNI::startTracking(XnUserID nID){
    XnStatus nRetVal = XN_STATUS_OK;
    ofLogNotice(LOG_NAME) << "Start tracking user" << nID;
	nRetVal = openNIContext->getUserGenerator(instanceID).GetSkeletonCap().StartTracking(nID);
    CHECK_ERR_RC(nRetVal, "Get skeleton capability - start tracking");
}

//--------------------------------------------------------------
bool ofxOpenNI::needsPoseForCalibration(){
	return bNeedsPose;
}

//----------------------------------------
void ofxOpenNI::updateUsers(){
    
    if (!g_bIsUserOn) return;
    
    // get user generator reference
    xn::UserGenerator& userGenerator = openNIContext->getUserGenerator(instanceID);
    
	vector<XnUserID> userIDs(MAX_NUMBER_USERS);
	XnUInt16 maxNumUsers = MAX_NUMBER_USERS;
	userGenerator.GetUsers(&userIDs[0], maxNumUsers);
    
	set<XnUserID> trackedUserIDs;
    
	for (int i = 0; i < MAX_NUMBER_USERS; ++i) {
		if (userGenerator.GetSkeletonCap().IsTracking(userIDs[i])) {
			ofxOpenNIUser & user = currentTrackedUsers[userIDs[i]];
			user.id = userIDs[i];
			XnPoint3D center;
			userGenerator.GetCoM(userIDs[i], center);
			user.center = toOf(center);
            
			for (int j=0; j<ofxOpenNIUser::NumLimbs; j++){
				XnSkeletonJointPosition a,b;
				userGenerator.GetSkeletonCap().GetSkeletonJointPosition(user.id, user.limbs[j].start_joint, a);
				userGenerator.GetSkeletonCap().GetSkeletonJointPosition(user.id, user.limbs[j].end_joint, b);
				userGenerator.GetSkeletonCap().GetSkeletonJointOrientation(user.id,user.limbs[j].start_joint, user.limbs[j].orientation);
				if (a.fConfidence < 0.3f || b.fConfidence < 0.3f){
					user.limbs[j].found = false;
					continue;
				}
                
				user.limbs[j].found = true;
				user.limbs[j].begin = worldToProjective(a.position);
				user.limbs[j].end = worldToProjective(b.position);
				user.limbs[j].worldBegin = toOf(a.position);
				user.limbs[j].worldEnd = toOf(b.position);
			}
            
			if (bUsePointClouds) updatePointClouds(user);
			if (bUseMaskPixels) updateUserPixels(user);
            
			trackedUserIDs.insert(user.id);
		}
	}
    
	set<XnUserID>::iterator it;
	for (it = previousTrackedUserIDs.begin(); it != previousTrackedUserIDs.end(); it++){
		if (trackedUserIDs.find(*it) == trackedUserIDs.end()){
			currentTrackedUsers.erase(*it);
		}
	}

	previousTrackedUserIDs = trackedUserIDs;
	currentTrackedUserIDs.assign(previousTrackedUserIDs.begin(), previousTrackedUserIDs.end());
    
	//if (useMaskPixels) updateUserPixels();
}

//--------------------------------------------------------------
void ofxOpenNI::updatePointClouds(ofxOpenNIUser & user){
    
	const XnRGB24Pixel*	pColor;
	const XnDepthPixel*	pDepth = g_DepthMD.Data();
    
	bool hasImageGenerator = openNIContext->getImageGenerator(instanceID).IsValid();
    
	if (hasImageGenerator) {
		pColor = g_ImageMD.RGB24Data();
	}
    
	xn::SceneMetaData smd;
	unsigned short *userPix;
    
	if (openNIContext->getUserGenerator(instanceID).GetUserPixels(user.id, smd) == XN_STATUS_OK) {
		userPix = (unsigned short*)smd.Data();
	}
    
	int step = 1;
	int nIndex = 0;
    
	user.pointCloud.getVertices().clear();
	user.pointCloud.getColors().clear();
	user.pointCloud.setMode(OF_PRIMITIVE_POINTS);
    
	for (int nY = 0; nY < getHeight(); nY += step) {
		for (int nX = 0; nX < getWidth(); nX += step, nIndex += step) {
			if (userPix[nIndex] == user.id) {
				user.pointCloud.addVertex(ofPoint( nX,nY,pDepth[nIndex] ));
				ofColor color;
				if(hasImageGenerator){
					user.pointCloud.addColor(ofColor(pColor[nIndex].nRed,pColor[nIndex].nGreen,pColor[nIndex].nBlue));
				}else{
					user.pointCloud.addColor(ofFloatColor(1,1,1));
				}
			}
		}
	}
}

//--------------------------------------------------------------
void ofxOpenNI::updateUserPixels(ofxOpenNIUser & user){
    
	xn::SceneMetaData smd;
	unsigned short *userPix;
    
	if (openNIContext->getUserGenerator(instanceID).GetUserPixels(user.id, smd) == XN_STATUS_OK) { //	GetUserPixels is supposed to take a user ID number,
		userPix = (unsigned short*)smd.Data();					//  but you get the same data no matter what you pass.
	}															//	userPix actually contains an array where each value
    //  corresponds to the user being tracked.
    //  Ie.,	if userPix[i] == 0 then it's not being tracked -> it's the background!
    //			if userPix[i] > 0 then the pixel belongs to the user who's value IS userPix[i]
    //  // (many thanks to ascorbin who's code made this apparent to me)
    
	user.maskPixels.allocate(getWidth(), getHeight(), OF_IMAGE_GRAYSCALE);
    
	for (int i =0 ; i < getWidth() * getHeight(); i++) {
		if (userPix[i] == user.id) {
			user.maskPixels[i] = 255;
		} else {
			user.maskPixels[i] = 0;
		}
        
	}
}

//--------------------------------------------------------------
void ofxOpenNI::updateFrame(){
    
	//if (bIsThreaded) lock(); // with this here I get ~30 fps with 2 Kinects
	
	if (g_bIsDepthOn) openNIContext->getDepthGenerator(instanceID).GetMetaData(g_DepthMD);
	if (g_bIsImageOn) openNIContext->getImageGenerator(instanceID).GetMetaData(g_ImageMD);
	if (g_bIsIROn) openNIContext->getIRGenerator(instanceID).GetMetaData(g_IrMD);
	
    if (bIsThreaded) lock(); // with this her I get ~500+ fps with 2 Kinects!
    
    if (g_bIsDepthOn) generateDepthPixels();
	if (g_bIsImageOn) generateImagePixels();
	if (g_bIsIROn) generateIRPixels();
	
// I really don't think it's necessary to back buffer the image/ir/depth pixels
// as I understand it the GetMetaData() call is already acting as a back buffer
// since it is fetching the pixel data from the xn::Context which we then 'copy'
// during our generateDepth/Image/IRPixels() methods...

// my tests show that it adds between ~10 to ~15 milliseconds to capture <-> screen latency 
// ie., the time between something occuring in the physical world, it's capture and subsequent display onscreen.
    
// without back buffering my tests show 55 to 65ms, avg 61.5ms (consistent frame times, ie., no outliers in small samples)
// with back buffering my tests show 70 to 80ms, avg 73.8ms (this does not include outliers ie., usually 1 in 7 frames showing 150-275ms latency!)

// NB: the above tests were done with 2 Kinects...with one Kinect (and not using backbuffering) I get between 50-60ms, avg ~53ms 
// (with some more outliers though one frame 33ms (!) andother 95ms(!))....hmmmm   

//	if (g_bIsDepthOn){
//		swap(backDepthPixels, currentDepthPixels);
//		if (g_bIsDepthRawOnOption){
//			swap(backDepthRawPixels, currentDepthRawPixels);
//		}
//	}
//	if (g_bIsImageOn || g_bIsIROn){
//		swap(backImagePixels, currentImagePixels);
//	}
	
	bNewPixels = true;
	
	if (bIsThreaded) unlock();
}

//--------------------------------------------------------------
void ofxOpenNI::update(){

	if (!bIsThreaded){
		updateFrame();
	} else {
		lock();
	}
	
	if (bNewPixels){
		if (bUseTexture && g_bIsDepthOn){
			//depthTexture.loadData(*currentDepthPixels); // see note about back buffering above
			depthTexture.loadData(*backDepthPixels);
		}
		if (bUseTexture && (g_bIsImageOn || g_bIsIROn)){
			//imageTexture.loadData(*currentImagePixels);
			imageTexture.loadData(*backImagePixels);
		}

		bNewPixels = false;
		bNewFrame = true;

	}

	if (bIsThreaded) unlock();
}

//--------------------------------------------------------------
bool ofxOpenNI::isNewFrame(){
	return bNewFrame;
}

//--------------------------------------------------------------
void ofxOpenNI::threadedFunction(){
	while(isThreadRunning()){
		updateFrame();
        updateUsers();
	}
}

//--------------------------------------------------------------
bool ofxOpenNI::toggleCalibratedRGBDepth(){
	
	if (!openNIContext->getImageGenerator(instanceID).IsValid()){
		ofLogError(LOG_NAME) << "No Image generator found: cannot register viewport";
		return false;
	}
	
	// Toggle registering view point to image map
	if (openNIContext->getDepthGenerator(instanceID).IsCapabilitySupported(XN_CAPABILITY_ALTERNATIVE_VIEW_POINT)){
		
		if (openNIContext->getDepthGenerator(instanceID).GetAlternativeViewPointCap().IsViewPointAs(openNIContext->getImageGenerator(instanceID))){
			disableCalibratedRGBDepth();
		} else {
			enableCalibratedRGBDepth();
		}
		
	} else return false;
	
	return true;
}

//--------------------------------------------------------------
bool ofxOpenNI::enableCalibratedRGBDepth(){
	if (!openNIContext->getImageGenerator(instanceID).IsValid()){
		ofLogError(LOG_NAME) << "No Image generator found: cannot register viewport";
		return false;
	}
	
	// Register view point to image map
	if (openNIContext->getDepthGenerator(instanceID).IsCapabilitySupported(XN_CAPABILITY_ALTERNATIVE_VIEW_POINT)){
		
		XnStatus nRetVal = openNIContext->getDepthGenerator(instanceID).GetAlternativeViewPointCap().SetViewPoint(openNIContext->getImageGenerator(instanceID));
		SHOW_RC(nRetVal, "Register viewpoint depth to RGB")
		if (nRetVal!=XN_STATUS_OK) return false;
	} else {
		ofLogVerbose(LOG_NAME) << "Alternative viewpoint not supported";
		return false;
	}
	
	return true;
}

//--------------------------------------------------------------
bool ofxOpenNI::disableCalibratedRGBDepth(){
	// Unregister view point from (image) any map
	if (openNIContext->getDepthGenerator(instanceID).IsCapabilitySupported(XN_CAPABILITY_ALTERNATIVE_VIEW_POINT)){
		XnStatus nRetVal = openNIContext->getDepthGenerator(instanceID).GetAlternativeViewPointCap().ResetViewPoint();
		SHOW_RC(nRetVal, "Unregister viewpoint depth to RGB");
		if (nRetVal!=XN_STATUS_OK) return false;
	} else {
		ofLogVerbose(LOG_NAME) << "Alternative viewpoint not supported";
		return false;
	}
	
	return true;
}

//--------------------------------------------------------------
void ofxOpenNI::drawDepth(){
	if (bUseTexture) drawDepth(0, 0, getWidth(), getHeight());
}

//--------------------------------------------------------------
void ofxOpenNI::drawDepth(float x, float y){
	if (bUseTexture) drawDepth(x, y, getWidth(), getHeight());
}

//--------------------------------------------------------------
void ofxOpenNI::drawDepth(float x, float y, float w, float h){
	if (bUseTexture) depthTexture.draw(x, y, w, h);
}

//--------------------------------------------------------------
void ofxOpenNI::drawImage(){
	if (bUseTexture) drawImage(0, 0, getWidth(), getHeight());
}

//--------------------------------------------------------------
void ofxOpenNI::drawImage(float x, float y){
	if (bUseTexture) drawImage(x, y, getWidth(), getHeight());
}

//--------------------------------------------------------------
void ofxOpenNI::drawImage(float x, float y, float w, float h){
	if (bUseTexture) imageTexture.draw(x, y, w, h);
}

//--------------------------------------------------------------
void ofxOpenNI::drawUsers(){
	ofPushStyle();
    //	if (currentTrackedUsers.size() > 0) {
    //    }
    // draw all the users
    for(int i = 0;  i < (int)currentTrackedUserIDs.size(); ++i) {
        drawUser(i);
    }
	ofPopStyle();
}

//--------------------------------------------------------------
void ofxOpenNI::drawUser(int nID) {
	if(nID - 1 > (int)currentTrackedUserIDs.size()) return;
	currentTrackedUsers[currentTrackedUserIDs[nID]].debugDraw();
}

//--------------------------------------------------------------
void ofxOpenNI::setDepthColoring(DepthColoring coloring){
	depthColoring = coloring;
}

//--------------------------------------------------------------
void ofxOpenNI::generateIRPixels(){
	const XnIRPixel* pImage = g_IrMD.Data();
    unsigned char * ir_pixels = new unsigned char[g_IrMD.XRes() * g_IrMD.YRes()];
	for (int i = 0; i < g_IrMD.XRes() * g_IrMD.YRes(); i++){
		ir_pixels[i] = pImage[i]/4;
	}
	backImagePixels->setFromPixels(ir_pixels, g_IrMD.XRes(), g_IrMD.YRes(), OF_IMAGE_GRAYSCALE);
    delete ir_pixels;
}

//--------------------------------------------------------------
void ofxOpenNI::generateImagePixels(){
	const XnUInt8* pImage = g_ImageMD.Data();
	backImagePixels->setFromPixels(pImage, g_ImageMD.XRes(), g_ImageMD.YRes(), OF_IMAGE_COLOR);
}

//--------------------------------------------------------------
void ofxOpenNI::generateDepthPixels(){
	// get the pixels
	const XnDepthPixel* depth = g_DepthMD.Data();
	
	if (g_DepthMD.FrameID() == 0) return;
	
	// copy raw values
	if (g_bIsDepthRawOnOption){
		backDepthRawPixels->setFromPixels(depth, g_DepthMD.XRes(), g_DepthMD.YRes(), OF_IMAGE_COLOR);
	}
	
	// copy depth into texture-map
	float max;
	for (XnUInt16 y = g_DepthMD.YOffset(); y < g_DepthMD.YRes() + g_DepthMD.YOffset(); y++){
		unsigned char * texture = backDepthPixels->getPixels() + y * g_DepthMD.XRes() * 4 + g_DepthMD.XOffset() * 4;
		for (XnUInt16 x = 0; x < g_DepthMD.XRes(); x++, depth++, texture += 4){
			XnUInt8 red = 0;
			XnUInt8 green = 0;
			XnUInt8 blue = 0;
			XnUInt8 alpha = 255;
			
			XnUInt16 col_index;
			
			switch (depthColoring){
				case COLORING_PSYCHEDELIC_SHADES:
					alpha *= (((XnFloat)(*depth % 10) / 20) + 0.5);
				case COLORING_PSYCHEDELIC:
					switch ((*depth/10) % 10){
						case 0:
							red = 255;
							break;
						case 1:
							green = 255;
							break;
						case 2:
							blue = 255;
							break;
						case 3:
							red = 255;
							green = 255;
							break;
						case 4:
							green = 255;
							blue = 255;
							break;
						case 5:
							red = 255;
							blue = 255;
							break;
						case 6:
							red = 255;
							green = 255;
							blue = 255;
							break;
						case 7:
							red = 127;
							blue = 255;
							break;
						case 8:
							red = 255;
							blue = 127;
							break;
						case 9:
							red = 127;
							green = 255;
							break;
					}
					break;
				case COLORING_RAINBOW:
					col_index = (XnUInt16)(((*depth) / (maxDepth / 256)));
					red = PalletIntsR[col_index];
					green = PalletIntsG[col_index];
					blue = PalletIntsB[col_index];
					break;
				case COLORING_CYCLIC_RAINBOW:
					col_index = (*depth % 256);
					red = PalletIntsR[col_index];
					green = PalletIntsG[col_index];
					blue = PalletIntsB[col_index];
					break;
				case COLORING_BLUES:
					// 3 bytes of depth: black (R0G0B0) >> blue (001) >> cyan (011) >> white (111)
					max = 256+255+255;
					col_index = (XnUInt16)(((*depth) / (maxDepth / max)));
					if ( col_index < 256 )
					{
						blue	= col_index;
						green	= 0;
						red		= 0;
					}
					else if ( col_index < (256+255) )
					{
						blue	= 255;
						green	= (col_index % 256) + 1;
						red		= 0;
					}
					else if ( col_index < (256+255+255) )
					{
						blue	= 255;
						green	= 255;
						red		= (col_index % 256) + 1;
					}
					else
					{
						blue	= 255;
						green	= 255;
						red		= 255;
					}
					break;
				case COLORING_GREY:
					max = 255;	// half depth
				{
					XnUInt8 a = (XnUInt8)(((*depth) / (maxDepth / max)));
					red		= a;
					green	= a;
					blue	= a;
				}
					break;
				case COLORING_STATUS:
					// This is something to use on installations
					// when the end user needs to know if the camera is tracking or not
					// The scene will be painted GREEN if status == true
					// The scene will be painted RED if status == false
					// Usage: declare a global bool status and that's it!
					// I'll keep it commented so you dont have to have a status on every project
#if 0
				{
					extern bool status;
					max = 255;	// half depth
					XnUInt8 a = 255 - (XnUInt8)(((*depth) / (maxDepth / max)));
					red		= ( status ? 0 : a);
					green	= ( status ? a : 0);
					blue	= 0;
				}
#endif
					break;
			}
			
			texture[0] = red;
			texture[1] = green;
			texture[2] = blue;
			
			if (*depth == 0)
				texture[3] = 0;
			else
				texture[3] = alpha;
		}
	}
	
	
}

//--------------------------------------------------------------
int ofxOpenNI::getNumDevices(){
	return openNIContext->getNumDevices();
}

//--------------------------------------------------------------
int ofxOpenNI::getDeviceID(){
	return instanceID;
}

//--------------------------------------------------------------
xn::Device& ofxOpenNI::getDevice(){
	return openNIContext->getDevice(instanceID);
}

//--------------------------------------------------------------
xn::DepthGenerator& ofxOpenNI::getDepthGenerator(){
	return openNIContext->getDepthGenerator(instanceID);
}

//--------------------------------------------------------------
xn::ImageGenerator& ofxOpenNI::getImageGenerator(){
	return openNIContext->getImageGenerator(instanceID);;
}

//--------------------------------------------------------------
xn::IRGenerator& ofxOpenNI::getIRGenerator(){
	return openNIContext->getIRGenerator(instanceID);;
}

//--------------------------------------------------------------
xn::UserGenerator& ofxOpenNI::getUserGenerator(){
	return openNIContext->getUserGenerator(instanceID);;
}

//--------------------------------------------------------------
xn::AudioGenerator& ofxOpenNI::getAudioGenerator(){
	return openNIContext->getAudioGenerator(instanceID);;
}

//--------------------------------------------------------------
xn::Player& ofxOpenNI::getPlayer(){
	return openNIContext->getPlayer(instanceID);;
}

//--------------------------------------------------------------
xn::DepthMetaData& ofxOpenNI::getDepthMetaData(){
	return g_DepthMD;
}

//--------------------------------------------------------------
xn::ImageMetaData& ofxOpenNI::getImageMetaData(){
	return g_ImageMD;
}

//--------------------------------------------------------------
xn::IRMetaData& ofxOpenNI::getIRMetaData(){
	return g_IrMD;
}

//--------------------------------------------------------------
xn::AudioMetaData& ofxOpenNI::getAudioMetaData(){
	return g_AudioMD;
}

//--------------------------------------------------------------
ofPixels& ofxOpenNI::getDepthPixels(){
	Poco::ScopedLock<ofMutex> lock(mutex);
	return *currentDepthPixels;
}

//--------------------------------------------------------------
ofShortPixels& ofxOpenNI::getDepthRawPixels(){
	Poco::ScopedLock<ofMutex> lock(mutex);
	if (!g_bIsDepthRawOnOption){
		ofLogWarning(LOG_NAME) << "g_bIsDepthRawOnOption was disabled, enabling raw pixels";
		g_bIsDepthRawOnOption = true;
	}
	return *currentDepthRawPixels;
}

//--------------------------------------------------------------
ofPixels& ofxOpenNI::getImagePixels(){
	Poco::ScopedLock<ofMutex> lock(mutex);
	return *currentImagePixels;
}

//--------------------------------------------------------------
ofTexture& ofxOpenNI::getDepthTextureReference(){
    Poco::ScopedLock<ofMutex> lock(mutex);
	return depthTexture;
}

//--------------------------------------------------------------
ofTexture& ofxOpenNI::getimageTextureReference(){
    Poco::ScopedLock<ofMutex> lock(mutex);
	return imageTexture;
}

//--------------------------------------------------------------
float ofxOpenNI::getWidth(){
	if (g_bIsDepthOn){
		return g_DepthMD.XRes();
	}else if (g_bIsImageOn){
		return g_ImageMD.XRes();
	}else if (g_bIsIROn){
		return g_IrMD.XRes();
	} else {
		ofLogWarning(LOG_NAME) << "getWidth() : We haven't yet initialised any generators, so this value returned is returned as 0";
		return 0;
	}
}

//--------------------------------------------------------------
float ofxOpenNI::getHeight(){
	if (g_bIsDepthOn){
		return g_DepthMD.YRes();
	}else if (g_bIsImageOn){
		return g_ImageMD.YRes();
	}else if (g_bIsIROn){
		return g_IrMD.YRes();
	} else {
		ofLogWarning(LOG_NAME) << "getHeight() : We haven't yet initialised any generators, so this value returned is returned as 0";
		return 0;
	}
}

//--------------------------------------------------------------
ofPoint ofxOpenNI::worldToProjective(const ofPoint& p){
	XnVector3D world = toXn(p);
	return worldToProjective(world);
}

//--------------------------------------------------------------
ofPoint ofxOpenNI::worldToProjective(const XnVector3D& p){
	XnVector3D proj;
	openNIContext->getDepthGenerator(instanceID).ConvertRealWorldToProjective(1,&p,&proj);
	return toOf(proj);
}

//--------------------------------------------------------------
ofPoint ofxOpenNI::projectiveToWorld(const ofPoint& p){
	XnVector3D proj = toXn(p);
	return projectiveToWorld(proj);
}

//--------------------------------------------------------------
ofPoint ofxOpenNI::projectiveToWorld(const XnVector3D& p){
	XnVector3D world;
	openNIContext->getDepthGenerator(instanceID).ConvertProjectiveToRealWorld(1,&p,&world);
	return toOf(world);
}

//--------------------------------------------------------------
ofPoint ofxOpenNI::cameraToWorld(const ofVec2f& c){
	vector<ofVec2f> vc(1, c);
	vector<ofVec3f> vw(1);
	
	cameraToWorld(vc, vw);
	
	return vw[0];
}

//--------------------------------------------------------------
void ofxOpenNI::cameraToWorld(const vector<ofVec2f>& c, vector<ofVec3f>& w){
	const int nPoints = c.size();
	w.resize(nPoints);
	if (!g_bIsDepthRawOnOption){
		ofLogError(LOG_NAME) << "ofxOpenNI::cameraToWorld - cannot perform this function if g_bIsDepthRawOnOption is false. You can enabled g_bIsDepthRawOnOption by calling getDepthRawPixels(..).";
		return;
	}
	
	vector<XnPoint3D> projective(nPoints);
	XnPoint3D *out =&projective[0];
	
	//lock();
	const XnDepthPixel* d = currentDepthRawPixels->getPixels();
	unsigned int pixel;
	for (int i=0; i<nPoints; ++i){
		pixel  = (int)c[i].x + (int)c[i].y * g_DepthMD.XRes();
		if (pixel >= g_DepthMD.XRes() * g_DepthMD.YRes())
			continue;
		
		projective[i].X = c[i].x;
		projective[i].Y = c[i].y;
		projective[i].Z = float(d[pixel]) / 1000.0f;
	}
	//unlock();
	
	openNIContext->getDepthGenerator(instanceID).ConvertProjectiveToRealWorld(nPoints,&projective[0], (XnPoint3D*)&w[0]);
	
}

// USER GENERATOR CALLBACKS
// =============================================================

//--------------------------------------------------------------
void XN_CALLBACK_TYPE ofxOpenNI::User_NewUser(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie){
    ofxOpenNI* openNI = static_cast<ofxOpenNI*>(pCookie);
	ofLogVerbose(CALLBACK_LOG_NAME) << "Device[" << openNI->getDeviceID() << "]" << "New User" << nID;
	if(openNI->needsPoseForCalibration()) {
		openNI->startPoseDetection(nID);
	} else {
		openNI->requestCalibration(nID);
	}
}

//--------------------------------------------------------------
void XN_CALLBACK_TYPE ofxOpenNI::User_LostUser(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie){
    ofxOpenNI* openNI = static_cast<ofxOpenNI*>(pCookie);
	ofLogVerbose(CALLBACK_LOG_NAME) << "Device[" << openNI->getDeviceID() << "]" << "Lost user" << nID;
	rGenerator.GetSkeletonCap().Reset(nID);
    
}

//--------------------------------------------------------------
void XN_CALLBACK_TYPE ofxOpenNI::UserPose_PoseDetected(xn::PoseDetectionCapability& rCapability, const XnChar* strPose, XnUserID nID, void* pCookie){
    ofxOpenNI* openNI = static_cast<ofxOpenNI*>(pCookie);
	ofLogVerbose(CALLBACK_LOG_NAME) << "Device[" << openNI->getDeviceID() << "]" << "Pose" << strPose << "detected for user" << nID;
	openNI->requestCalibration(nID);
	openNI->stopPoseDetection(nID);
}


//--------------------------------------------------------------
void XN_CALLBACK_TYPE ofxOpenNI::UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nID, void* pCookie){
    ofxOpenNI* openNI = static_cast<ofxOpenNI*>(pCookie);
	ofLogVerbose(CALLBACK_LOG_NAME) << "Device[" << openNI->getDeviceID() << "]" << "Calibration start for user" << nID;
}


//--------------------------------------------------------------
void XN_CALLBACK_TYPE ofxOpenNI::UserCalibration_CalibrationEnd(xn::SkeletonCapability& rCapability, XnUserID nID, XnCalibrationStatus bSuccess, void* pCookie){
    ofxOpenNI* openNI = static_cast<ofxOpenNI*>(pCookie);
	ofLogVerbose(CALLBACK_LOG_NAME) << "Device[" << openNI->getDeviceID() << "]" << "Calibration end for user..." << nID;
	if(bSuccess == XN_CALIBRATION_STATUS_OK) {
		ofLogVerbose(CALLBACK_LOG_NAME) << "Device[" << openNI->getDeviceID() << "]" << "...success" << nID;
		openNI->startTracking(nID);
	} else {
        ofLogVerbose(CALLBACK_LOG_NAME) << "Device[" << openNI->getDeviceID() << "]" << "...fail" << nID;
		if(openNI->needsPoseForCalibration()) {
			openNI->startPoseDetection(nID);
		} else {
			openNI->requestCalibration(nID);
		}
	}
}