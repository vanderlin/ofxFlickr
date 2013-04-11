/*
 *  ofxFlickr.cpp
 *  openFrameworks
 *
 *  Created by Todd Vanderlin on 11/22/09.
 *  Copyright 2009 Interactive Design. All rights reserved.
 *
 */

#include "ofxFlickr.h"

#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/HTTPBasicCredentials.h"
#include "Poco/StreamCopier.h"
#include "Poco/Path.h"
#include "Poco/URI.h"
#include "Poco/Exception.h"
#include "Poco/Mutex.h"
#include "Poco/Net/FilePartSource.h"

using namespace Poco::Net;
using namespace Poco;

static bool compKeys(const KeyValuePair &a, const KeyValuePair &b) {
    return a.key<b.key;
}

// -------------------------------------------- Flickr
ofxFlickr::ofxFlickr() {
	API_KEY = "";
	API_SECRET = "";
	bAllocated = false;
    loading    = false;
	AUTH_FROB = "";
	AUTH_TOKEN = "";
	API_SIG = "";
}
ofxFlickr::~ofxFlickr() {
}


// -------------------------------------------- save token
void ofxFlickr::saveToken() {
    if(AUTH_TOKEN!="") {
        ofstream tokenFile;
        tokenFile.open(ofToDataPath("token.dat").c_str());
        tokenFile.clear();
        tokenFile << AUTH_TOKEN;
        tokenFile.close();
        printf("Token saved\n");
    }
    else {
        printf("No Token To Save\n");
    }
}

// -------------------------------------------- load token
void ofxFlickr::loadToken() {
    ifstream tokenFile;
    tokenFile.open(ofToDataPath("token.dat").c_str());
    if(tokenFile!=0) {
        string line = "";
        while (!tokenFile.eof()) {
            getline (tokenFile, line);
            
        }
        if(line != "") {
            printf("Token %s loaded\n", line.c_str());
            AUTH_TOKEN = line;
        }
        else {
            printf("No token in file...\n");
        }
        tokenFile.close();
    }
    else {
        printf("Error opening token file\n");
    }
}

// -------------------------------------------- Setup
void ofxFlickr::setup(string key, string secret) {
	setApiKey(key);
	API_SECRET = secret;
}

// -------------------------------------------- XML Parser
void ofxFlickr::parseXML(ofxFlickrResponse &res, int method) {
	
	//printf("res:%s\n", response.content.c_str());
	printf("--- Method: %i ---\n", method);
	
    res.type = method;
    
	xml.loadFromBuffer(res.content);
	string status = xml.getAttribute("rsp", "stat", "fail");
	if(status.compare("ok") == 0) {

		// ----------------------
		// Get Photos
		// ----------------------
		if(method == FLICKR_METHOD_GET_RECENT_PHOTOS ||
		   method == FLICKR_METHOD_PHOTO_SEARCH) {
			
			xml.pushTag("rsp");
			xml.pushTag("photos");
			
			int numPhotos = xml.getNumTags("photo");
			if(numPhotos > 0) {
				for(int i=0; i<numPhotos; i++) {
					
					//xml.pushTag("photo", i);
					
					ofxFlickrPhoto photo;
					
					string farmid   = xml.getAttribute("photo", "farm", "NULL", i); 
					string serverid = xml.getAttribute("photo", "server", "NULL", i);
					string photoid  = xml.getAttribute("photo", "id", "NULL", i);
					string secret   = xml.getAttribute("photo", "secret", "NULL", i);
					
					string imgURL   = 
					"http://farm" + farmid + ".static.flickr.com/" + 
					serverid + "/" + photoid + "_" + secret;
					
					photo.title		  = xml.getAttribute("photo", "title", "NULL", i);
					photo.owner		  = xml.getAttribute("photo", "owner", "NULL", i);
					photo.squareURL   = imgURL + "_s.jpg";	// 75 x 75
					photo.thumbURL    = imgURL + "_t.jpg";	// 100 x X
					photo.mediumURL   = imgURL + "_m.jpg";	// 240 x X
					photo.largeURL    = imgURL + ".jpg";	// 500 x X
					photo.originalURL = imgURL + "_o.jpg";	// orginal size
					
					res.photos.push_back(photo);
				}
			}
			xml.popTag();
			xml.popTag();
			
		}
		
		
		// ----------------------
		// Get Upload FROB
		// ----------------------
		if(method == FLICKR_METHOD_AUTH_FROB) {
			xml.pushTag("rsp");
			string frob = xml.getValue("frob", "NULL");
			
			printf("frob:%s\n", frob.c_str());
			
			if(frob.compare("NULL") != 0) {
				AUTH_FROB = frob;
				launchLoginLink();
			}
			xml.popTag();
		}
		
		// ----------------------
		// Get Token
		// ----------------------
		if(method == FLICKR_METHOD_AUTH_GET_TOKEN) {
			
			printf("--- Auth:%s ---\n", res.content.c_str());
			
			xml.pushTag("rsp");
			xml.pushTag("auth");
			
			string token = xml.getValue("token", "NULL");
			
			printf("token:%s\n", token.c_str());
			
			if(token.compare("NULL") != 0) {
				AUTH_TOKEN = token;
			}
			
			xml.popTag();
			xml.popTag();
			
		}
		
		
		// ----------------------
		// Upload
		// ----------------------
		if(method ==FLICKR_METHOD_PHOTO_UPLOAD) {
            res.content = xml.getValue("rsp:photoid", "none");
            printf("upload res:%s\n", res.content.c_str());
			
		}
	}
	else {
		printf(" -- error:%s\n", res.content.c_str());
	}
	
	
	
}

// -------------------------------------------- Flickr Response
void ofxFlickr::flickrResponse(ofxFlickrResponse &response) {
}

// -------------------------------------------- Set The API Key
void ofxFlickr::setApiKey(string key) {
	API_KEY    = key;
	bAllocated = true;
}

// -------------------------------------------- Get Recent Photos
void ofxFlickr::loadURL(string url, int method) {
    loading = true;
    ofHttpResponse res = ofLoadURL(url);
    if(res.status>=200) {
        ofxFlickrResponse flickrRes;
        flickrRes.content = res.data.getText();
        parseXML(flickrRes, method);
        
        // tell the world about the flickr response
        flickrResponse(flickrRes);
        ofNotifyEvent(FLICKR_RESPONSE, flickrRes, this);
    }
    else {
        printf("*** Error loading url %i\n", method);
    }
    loading = false;
}

// -------------------------------------------- Get Recent Photos
void ofxFlickr::getRecentPhotos(int num, int page) {
	
	string url = 
	FLICKR_BASE_URL +
	FLICKR_GET_RECENT + 
	"&"+FLICKR_API_KEY + API_KEY +
	"&"+FLICKR_PER_PAGE + ofToString(num);
	"&"+FLICKR_PAGE + ofToString(page);
	
	loadURL(url, FLICKR_METHOD_GET_RECENT_PHOTOS);
	
}

// -------------------------------------------- Tag Search
void ofxFlickr::tagSearch(string tags, string tag_mode, string sort_mode, int num, int page) {
	photoSearch(tags, tag_mode, "", sort_mode, num, page);
}

// -------------------------------------------- Photo Search
void ofxFlickr::photoSearch(string tags, 
							string tag_mode, 
							string text, 
							string sort_mode,
							int num, 
							int page,
							string extra) {
	
	string url = "";
	url += FLICKR_BASE_URL;
	url += FLICKR_PHOTO_SEARCH; 
	url += "&"+FLICKR_API_KEY + API_KEY;
	url += "&"+FLICKR_PER_PAGE + ofToString(num);
	url += "&"+FLICKR_PAGE + ofToString(page);
	
	// Sort
	if(sort_mode.compare("") != 0) {
		url += "&sort=" + sort_mode;
	}
	
	// Tags
	if(tags.compare("") != 0) {
		url += "&"+FLICKR_TAGS + tags;
		url += "&tag_mode="+tag_mode; 
	}
	
	// Free Text
	if(text.compare("") != 0) {
		url += "&text="+text; 
	}
	loadURL(url, FLICKR_METHOD_PHOTO_SEARCH);
	
}

// -------------------------------------------- Upload Photo
void ofxFlickr::uploadPhoto() {
	
    string photoName = "test.png";
    string photo     = ofToDataPath(photoName);
    string sigStr    = API_SECRET + "api_key" + API_KEY + "auth_token" + AUTH_TOKEN;
    string md5       = convertStringToMD5(sigStr);
    string action    = "http://api.flickr.com/services/upload/";

    try {
        URI uri( action.c_str() );
        std::string path(uri.getPathAndQuery());
        if (path.empty()) path = "/";
        
        HTTPClientSession session(uri.getHost(), uri.getPort());
        HTTPRequest req(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1);
        //if(auth.getUsername()!="") auth.authenticate(req);
        
        HTMLForm pocoForm;
        pocoForm.setEncoding(HTMLForm::ENCODING_MULTIPART);
        pocoForm.set("api_key", API_KEY);
        pocoForm.set("auth_token", AUTH_TOKEN);
        pocoForm.set("api_sig", md5);
       
        pocoForm.addPart("photo", new FilePartSource(photo));
        pocoForm.prepareSubmit(req);
        pocoForm.write(session.sendRequest(req));
        
        HTTPResponse res;
        istream& rs = session.receiveResponse(res);
      
        int status = res.getStatus();
		Poco::Timestamp timestamp=res.getDate();
		string reasonForStatus=res.getReasonForStatus(res.getStatus());
		string contentType = res.getContentType();
		ofBuffer responseBody;
        responseBody.set(rs);
        
        if(status>=200) {
            ofxFlickrResponse flickrRes;
            flickrRes.content = responseBody.getText();
            parseXML(flickrRes, FLICKR_METHOD_PHOTO_UPLOAD);
            flickrResponse(flickrRes);
            ofNotifyEvent(FLICKR_RESPONSE, flickrRes, this);
        }
        else {
            printf("error trying to upload photo %s\n", photoName.c_str());
        }
        //cout << responseBody.getText() << endl;
        // response = ofxHttpResponse(res, rs, path);
      
        
		//if(response.status>=300 && response.status<400){
		//	Poco::URI uri(req.getURI());
		//	uri.resolve(res.get("Location"));
		//	response.location = uri.toString();
		//}
        
    	//ofNotifyEvent(newResponseEvent, response, this);
        
        
    }
    catch (Exception& exc){
        
    	std::cerr << "Flickr uploading error\n";
        
        //ofNotifyEvent(notifyNewError, "time out", this);
        
        // for now print error, need to broadcast a response
        std::cerr << exc.displayText() << std::endl;
        //response.status = -1;
        cout << exc.displayText() << endl;
        
    }
    
   
	
}

// -------------------------------------------- Athorize
void ofxFlickr::authorize() {
	
	// we need a frob (we start here)
	if (AUTH_FROB.compare("") == 0) {
		cout << "Need a Frob" << endl;
		getFrob();
	}
	else {
		enableAuthoriztion();
	}
	
	
}

// -------------------------------------------- Athorize
void ofxFlickr::enableAuthoriztion() {
	requestToken();
	saveToken();
}

// -------------------------------------------- check the token
void ofxFlickr::checkToken() {
    
}

// -------------------------------------------- Athorize
void ofxFlickr::launchLoginLink() {
	
	
	// read - permission to read private information
    // write - permission to add, edit and delete photo metadata (includes 'read')
	// delete
	
	string PERMISIONS = "write";
	string sigStr	  = API_SECRET + "api_key" + API_KEY + "frob" + AUTH_FROB + "perms" + PERMISIONS;
	string md5		  = convertStringToMD5(sigStr);
	
	string url = "http://flickr.com/services/auth/";
	url += "?api_key=" + API_KEY;
	url += "&perms="   + PERMISIONS;
	url += "&frob="    + AUTH_FROB;
	url += "&api_sig=" + md5;
	
	// This cleans up the URL for OSX
#ifdef TARGET_OSX
	string urlOut = "";
	for(int i=0; i<url.length(); i++) {
		string ltr(url, i, 1);
		if(ltr.compare("&") == 0) {
			urlOut += "\\";
		}
		urlOut += ltr;
	}
#endif
	
	printf("fin:%s\n", urlOut.c_str());
	ofLaunchBrowser(urlOut);
}


// -------------------------------------------- Get Token
void ofxFlickr::requestToken() {
	if (AUTH_FROB.compare("") != -1) {
		
		
		string sigStr = API_SECRET + "api_key" + API_KEY + "frob" + AUTH_FROB + "method" + FLICKR_AUTH_GET_TOKEN;
		string md5    = convertStringToMD5(sigStr);
		
		string url = "";
		url += FLICKR_BASE_URL;
		url += FLICKR_AUTH_GET_TOKEN; 
		url += "&"+FLICKR_API_KEY + API_KEY;
		url += "&frob=" + AUTH_FROB;
		url += "&"+FLICKR_API_SIG + md5;
		
		loadURL(url, FLICKR_METHOD_AUTH_GET_TOKEN);
	}
	else {
		printf("-- please autheticate first --\n");
	}
}

// -------------------------------------------- Get Frob
void ofxFlickr::getFrob() {
	
	string url = "";
	
	// we need a frob
	if (AUTH_FROB.compare("") == 0) {
		
		string sigStr = API_SECRET + "api_key" + API_KEY + "method" + FLICKR_AUTH_GET_FROB;
		string md5    = convertStringToMD5(sigStr);
		
		url += FLICKR_BASE_URL;
		url += FLICKR_AUTH_GET_FROB; 
		url += "&"+FLICKR_API_KEY + API_KEY;
		url += "&"+FLICKR_API_SIG + md5;
		
	}
	
	loadURL(url, FLICKR_METHOD_AUTH_FROB);
}


// -------------------------------------------- MD5 String
string ofxFlickr::convertStringToMD5(string str) {
	string newStr = "";
	hashwrapper *md5 = new md5wrapper();
	newStr = md5->getHashFromString(str);
	delete md5; 
	return newStr;
}
// -------------------------------------------- MD5 String
string ofxFlickr::getSignedRequest(string method, string params) {
    
    string url =
    FLICKR_BASE_URL + method +
    "&"+FLICKR_API_KEY + API_KEY +
    "&auth_token="+AUTH_TOKEN +
    "&"+params;
    
    vector <KeyValuePair> sigKeys;
    sigKeys.push_back(KeyValuePair("method", method));
    string sigStr = API_SECRET + "api_key" + API_KEY + "auth_token" + AUTH_TOKEN;
    
    vector <string> splitParams = ofSplitString(params, "&");
    for (int i=0; i<splitParams.size(); i++) {
        vector<string> keyvalue = ofSplitString(splitParams[i], "=");
        if(keyvalue.size()>=2) {
            string key = keyvalue[0];
            string val = keyvalue[1];
            sigKeys.push_back(KeyValuePair(key, val));
        }
    }
    
    sort(sigKeys.begin(), sigKeys.end(), compKeys);
    for(int i=0; i<sigKeys.size(); i++) {
        sigStr += sigKeys[i].key + sigKeys[i].value;
    }
    url += "&api_sig=" + convertStringToMD5( sigStr );
    return url;
}







