#include "CloudPlugs.h"
#include <Ethernet.h>
#include "HttpClient.h"


#define PATH_DATA "data"
#define PATH_DEVICE "device"
#define PATH_CHANNEL "channel"

#define PLUG_AUTH_HEADER "X-Plug-Auth: "
#define PLUG_ID_HEADER "X-Plug-Id: "
#define PLUG_EMAIL_HEADER "X-Plug-Email: "
#define PLUG_MASTER_HEADER "X-Plug-Master: "

#define SET_ERROR_AND_RETURN(x) { this->err = x; return false; }

#define CP_HTTP_GET "GET"
#define CP_HTTP_HEAD "HEAD"
#define CP_HTTP_POST "POST"
#define CP_HTTP_PUT "PUT"
#define CP_HTTP_DELETE "DELETE"
#define CP_HTTP_TRACE "TRACE"
#define CP_HTTP_OPTIONS "OPTIONS"
#define CP_HTTP_CONNECT "CONNECT"
#define CP_HTTP_PATCH "PATCH"

CloudPlugs::CloudPlugs(Client& aClient, const char* host): client(aClient) {
  httpClient = new HttpClient(client);
  this->host = String(host);
  id = "";
  auth = "";
}

CloudPlugs::~CloudPlugs() {
  delete(httpClient);
}

boolean CloudPlugs::setHost(const char* host) {
  this->host = String(host);
  return true;
}

const char* CloudPlugs::getHost() {
  return host.c_str();
}

CP_ERR_CODE CloudPlugs::getLastErrCode() {
  return this->err;
}
int CloudPlugs::getLastHttpResult() {
  return this->http_res;
}

#ifdef CP_DEBUG
void CloudPlugs::getLastErrString(String& err) {
    switch(this->err) {
        case CP_ERR_INTERNAL_ERROR:	     err = "Internal Library Error"; break;
        case CP_ERR_OUT_OF_MEMORY:           err = "Out of memory"; break;
        case CP_ERR_INVALID_SESSION:         err = "Invalid session"; break;
        case CP_ERR_INVALID_PARAMETER:       err = "Invalid parameter"; break;
        case CP_ERR_INVALID_LOGIN:           err = "Invalid login"; break;
        case CP_ERR_HTTP:                    err = "HTTP error"; break;
        default: return NULL;
   }
}

void CloudPlugs::getLastHttpResultString(String& http) {
    switch(this->http_res) {
      case CP_HTTP_OK:			http = "Ok"; break;
      case CP_HTTP_CREATED:		http = "Created"; break;
      case CP_HTTP_MULTI_STATUS:	http = "Multi-Status"; break;
      case CP_HTTP_BAD_REQUEST:		http = "Bad Request"; break;
      case CP_HTTP_UNAUTHORIZED:	http = "Unauthorized"; break;
      case CP_HTTP_PAYMENT_REQUIRED:	http = "Payment Required"; break;
      case CP_HTTP_FORBIDDEN:		http = "Forbidden"; break;
      case CP_HTTP_NOT_FOUND:		http = "Not found"; break;
      case CP_HTTP_NOT_ALLOWED:		http = "Method Not Allowed"; break;
      case CP_HTTP_NOT_ACCEPTABLE:	http = "Not Acceptable"; break;
      case CP_HTTP_SERVER_ERROR:	http = "Internal Server Error"; break;
      case CP_HTTP_NOT_IMPLEMENTED:	http = "Not Implemented"; break;
      case CP_HTTP_BAD_GATEWAY:		http = "Bad Gateway"; break;
      case CP_HTTP_SERVICE_UNAVAILABLE:	http = "Service Unavailable"; break;
    }
}
#endif //CP_DEBUG

boolean CloudPlugs::setAuth(const char* id, const char* pass, boolean is_master) {
    this->id = id;
    this->id = this->id.indexOf('@') != -1 ? PLUG_EMAIL_HEADER + this->id : PLUG_ID_HEADER + this->id;
    auth = is_master ? PLUG_MASTER_HEADER + String(pass) : PLUG_AUTH_HEADER + String(pass);
    this->is_master = is_master;
  return true;
}

boolean CloudPlugs::getAuthId(String& id) {
  int index = this->id.indexOf(' ');
  id = this->id.substring(index+1);
  return true;
}

boolean CloudPlugs::getAuthPass(String& password) {
  int index = this->auth.indexOf(' ');
  password = this->auth.substring(index+1);
  return true;
}

boolean CloudPlugs::isAuthMaster() {
  return is_master;
}

boolean CloudPlugs::enrollProduct(const char* body,  String& result) {
    if(!body) SET_ERROR_AND_RETURN(CP_ERR_INVALID_PARAMETER);
    return request_exec(false, CP_HTTP_POST, PATH_DEVICE, body, &result);
}

boolean CloudPlugs::enrollPrototype(const char* body, String& result) {
    if(!body) SET_ERROR_AND_RETURN(CP_ERR_INVALID_PARAMETER);
    if(!this->is_master) SET_ERROR_AND_RETURN(CP_ERR_INVALID_LOGIN);
    return request_exec(true, CP_HTTP_POST, PATH_DEVICE, body, &result);
}

boolean CloudPlugs::enrollCtrl(const char* body, String& result) {
    if(!body) SET_ERROR_AND_RETURN(CP_ERR_INVALID_PARAMETER);
    return request_exec(false, CP_HTTP_PUT, PATH_DEVICE, body, &result);
}

boolean CloudPlugs::controlDevice(const char* body, String& result) {
  if(!body) SET_ERROR_AND_RETURN(CP_ERR_INVALID_PARAMETER);  
  return request_exec(true, CP_HTTP_PUT, PATH_DEVICE, body, &result);
}

boolean CloudPlugs::uncontrolDevice(const char* plugid, const char* plugid_controlled, String& result) {
    String id;
    if(plugid) id = plugid;
    else getAuthId(id);
    String url = String(PATH_DEVICE "/") + id;
    return request_exec(true, CP_HTTP_DELETE, url.c_str(), NULL, &result);
}

boolean CloudPlugs::getDevice(const char* plugid, String& result) {
    String id;
    if(plugid) id = plugid;
    else getAuthId(id);
    String url = String(PATH_DEVICE "/") + id;
    return request_exec(true, CP_HTTP_GET, url.c_str(), NULL, &result);
}

boolean CloudPlugs::getDeviceProp(const char* plugid, const char* prop, String& result) {
    String id;
    if(plugid) id = plugid;
    else getAuthId(id);
    String url = String(PATH_DEVICE "/") + id + "/";
    if(prop) url += prop;
    return request_exec(true, CP_HTTP_GET, url.c_str(), NULL, &result);
}

boolean CloudPlugs::setDeviceProp(const char* plugid, const char* prop, const char* value, String& result) {
    if(!value) SET_ERROR_AND_RETURN(CP_ERR_INVALID_PARAMETER);
    String id;
    if(plugid) id = plugid;
    else getAuthId(id);
    String url = String(PATH_DEVICE "/") + id;
    if(prop) url += String("/") + prop;
    return request_exec(true, CP_HTTP_PATCH, url.c_str(), value, &result);
}

boolean CloudPlugs::removeDeviceProp(const char* plugid, const char* prop, String& result) {
    if(!prop) SET_ERROR_AND_RETURN(CP_ERR_INVALID_PARAMETER);
    String id;
    if(plugid) id = plugid;
    else getAuthId(id);
    String url = String(PATH_DEVICE "/") + id + "/" + prop;
    return request_exec(true, CP_HTTP_DELETE, url.c_str(), NULL, &result);
}

boolean CloudPlugs::setDevice(const char* plugid, const char* value, String& result) {
    if(!value) SET_ERROR_AND_RETURN(CP_ERR_INVALID_PARAMETER);
    String id;
    if(plugid) id = plugid;
    else getAuthId(id);
    String url = String(PATH_DEVICE "/") + id;
    return request_exec(true, CP_HTTP_PATCH, url.c_str(), value, &result);
}

boolean CloudPlugs::unenroll(const char* plugid, String& result) {
    String id;
    getAuthId(id);
    const char* body = plugid ? plugid : id.c_str();
    if(!body) SET_ERROR_AND_RETURN(CP_ERR_INVALID_PARAMETER);
    return request_exec(true, CP_HTTP_DELETE, PATH_DEVICE, body, &result);
}

boolean CloudPlugs::getChannel(const char* channel_mask, const char* query, String& result) {
    if(!channel_mask) SET_ERROR_AND_RETURN(CP_ERR_INVALID_PARAMETER);
    String url = PATH_CHANNEL;
    if(channel_mask) url += String("/") + channel_mask;
    if(query) url += String("?") + query;
    return request_exec(true, CP_HTTP_GET, url.c_str(), NULL, &result);
}

boolean CloudPlugs::retrieveData(const char* channel_mask, const char* query, String& result) {
    if(!channel_mask) SET_ERROR_AND_RETURN(CP_ERR_INVALID_PARAMETER);
    String url = String(PATH_DATA "/") + channel_mask;
    if(query) url += String("?") + query;
    return request_exec(true, CP_HTTP_GET, url.c_str(), NULL, &result);
}

boolean CloudPlugs::publishData(const char* channel, const char* body, String& result) {
  if(!channel || !body) SET_ERROR_AND_RETURN(CP_ERR_INVALID_PARAMETER);
  String url = String(PATH_DATA "/") + channel;
  return request_exec(true, CP_HTTP_PUT, url.c_str(), body, &result);
}

boolean CloudPlugs::removeData(const char* channel_mask, const char* body, String& result) {
 if(!channel_mask || !body) SET_ERROR_AND_RETURN(CP_ERR_INVALID_PARAMETER);
  String url = String(PATH_DATA "/") + channel_mask;
  return request_exec(true, CP_HTTP_DELETE, url.c_str(), body, &result);
}

boolean CloudPlugs::setDeviceLocation(const char* plugid, const char* value, String& result) {
    if(!value) SET_ERROR_AND_RETURN(CP_ERR_INVALID_PARAMETER);
    String id;
    if(plugid) id = plugid;
    else getAuthId(id);
    String url = String(PATH_DEVICE "/") + id + "/location";
    return request_exec(true, CP_HTTP_PATCH, url.c_str(), value, &result);
  return false;
}

boolean CloudPlugs::getDeviceLocation(const char* plugid, String& result) {
    String id;
    if(plugid) id = plugid;
    else getAuthId(id);
    String url = String(PATH_DEVICE "/") + id + "/location";
    return request_exec(true, CP_HTTP_GET, url.c_str(), NULL, &result);
}
/*
boolean CloudPlugs::request_exec(boolean send_auth, const char* http_method, const char* path, const char* body, String* result) {
  if(send_auth) {
    if(this->id.length() == 0 || this->auth.length() == 0) SET_ERROR_AND_RETURN(CP_ERR_INVALID_LOGIN);
    httpClient->setHeader(this->id.c_str());
    httpClient->setHeader(this->auth.c_str());
  }
  this->http_res = (CP_HTTP_RESULT) httpClient->request(http_method, path, body, result);
  if(http_res == 0) this->err = CP_ERR_NO_CONNECTION;
  return (this->http_res != CP_HTTP_OK && this->http_res != CP_HTTP_CREATED);
}
*/
boolean CloudPlugs::request_exec(boolean send_auth, const char* http_method, const char* path, const char* body, String* result) {

  *result = "";
  this->http_res = CP_HTTP_INVALID;
  int err =0;
  // Number of milliseconds to wait without receiving any data before we give up
  const unsigned int kNetworkTimeout = 30*1000;
  // Number of milliseconds to wait if no data is available before trying again
  const int kNetworkDelay = 1000;
  httpClient->beginRequest();
  err = httpClient->startRequest(host.c_str(), httpClient->kHttpPort, path, http_method, NULL);
  //err = httpClient->startRequest("192.168.1.133", 8080, path, http_method, NULL);
  if (err != 0) {
    this->err = CP_ERR_NO_CONNECTION;
    return false;
  }

  if(send_auth){
    httpClient->sendHeader(this->id.c_str());
    httpClient->sendHeader(this->auth.c_str());
  }
  if(body) {
    size_t aSize = strlen(body);
    httpClient->sendHeader("Content-Type", "application/json");
    httpClient->sendHeader("Content-Length", aSize);
    httpClient->write((const uint8_t*) body, aSize);
  }
  httpClient->endRequest();
  err = httpClient->responseStatusCode();
  
  if (err < 0) {
    this->err = CP_ERR_RESPONSE_FAILED;
    return false;    
  }

  switch(err){
    case CP_HTTP_OK:
    case CP_HTTP_CREATED:
    case CP_HTTP_MULTI_STATUS:
    case CP_HTTP_BAD_REQUEST:
    case CP_HTTP_UNAUTHORIZED:
    case CP_HTTP_PAYMENT_REQUIRED:
    case CP_HTTP_FORBIDDEN:
    case CP_HTTP_NOT_FOUND:
    case CP_HTTP_NOT_ALLOWED:
    case CP_HTTP_NOT_ACCEPTABLE:
    case CP_HTTP_SERVER_ERROR:
    case CP_HTTP_NOT_IMPLEMENTED:
    case CP_HTTP_BAD_GATEWAY:
    case CP_HTTP_SERVICE_UNAVAILABLE:
      this->http_res = err; break;
    default: this->http_res = CP_HTTP_INVALID;
  }
        
  err = httpClient->skipResponseHeaders();
  if (err < 0) {
    this->err = CP_ERR_SKIP_HEADERS;
    return false;      
  }
      
  int bodyLen = httpClient->contentLength();
  
  // Now we've got to the body, so we can print it out
  unsigned long timeoutStart = millis();
  char c;
  // Whilst we haven't timed out & haven't reached the end of the body
  while ( (httpClient->connected() || httpClient->available()) && ((millis() - timeoutStart) < kNetworkTimeout) ) {
      if (httpClient->available()) {
          c = httpClient->read();
          result->concat(c);
         
          bodyLen--;
          // We read something, reset the timeout counter
          timeoutStart = millis();
      } else {
          // We haven't got any data, so let's pause to allow some to
          // arrive
          delay(kNetworkDelay);
      }
  }

  httpClient->stop();
  return (this->http_res != CP_HTTP_OK && this->http_res != CP_HTTP_CREATED);
}

