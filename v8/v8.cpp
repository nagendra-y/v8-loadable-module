/** 
 * File: v8.cpp 
 *
 * */

/** Copyright (c) 2019 Mesibo
 * https://mesibo.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the terms and condition mentioned
 * on https://mesibo.com as well as following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions, the following disclaimer and links to documentation and
 * source code repository.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of Mesibo nor the names of its contributors may be used to
 * endorse or promote products derived from this software without specific prior
 * written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Documentation
 * https://mesibo.com/documentation/on-premise/loadable-modules/ 
 *
 * Source Code Repository
 * http://github.com/mesibo/onpremise-loadable-modules
 * 
 * Skeleton Module
 * https://github.com/mesibo/onpremise-loadable-modules/skeleton
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "module.h"
#include <assert.h>
#include <iostream>

#include <v8.h>

#include <include/libplatform/libplatform.h>
#include <stdlib.h>

#include <map>
#include <string>

using std::map;
using std::pair;
using std::string;

using v8::Context;
using v8::EscapableHandleScope;
using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Global;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::Name;
using v8::NamedPropertyHandlerConfiguration;
using v8::NewStringType;
using v8::Object;
using v8::ObjectTemplate;
using v8::PropertyCallbackInfo;
using v8::Script;
using v8::String;
using v8::TryCatch;
using v8::Value;

using std::cout;
using std::endl;

#define MESIBO_SCRIPT "/home/mesibo/v8-loadable-module/v8/test.js" 

#define HTTP_BUFFER_LEN (64 * 1024)
#define MODULE_LOG_LEVEL_0VERRIDE 0

/** Message Parmeters **/
#define MESSAGE_AID 				"aid"
#define MESSAGE_ID 				"id"
#define MESSAGE_REFID 				"refid"
#define MESSAGE_GROUPID 			"groupid"
#define MESSAGE_FLAGS 				"flags"
#define MESSAGE_TYPE 				"type"
#define MESSAGE_EXPIRY 				"expiry"
#define MESSAGE_TO_ONLINE 			"to_online"
#define MESSAGE_TO 				"to"
#define MESSAGE_FROM 				"from"

/** HTTP options **/
#define HTTP_PROXY 				"proxy"
#define HTTP_CONTENT_TYPE 			"content_type"
#define HTTP_EXTRA_HEADER 			"extra_header"
#define HTTP_USER_AGENT 			"user_agent"
#define HTTP_REFERRER 				"referrer"
#define HTTP_ORIGIN 				"origin"
#define HTTP_COOKIE 				"cookie"
#define HTTP_ENCODING 				"encoding"
#define HTTP_CACHE_CONTROL 			"cache_control"
#define HTTP_ACCEPT 				"accept"
#define HTTP_ETAG 				"etag"
#define HTTP_IMS 				"ims"
#define HTTP_MAXREDIRECTS 			"maxredirects"
#define HTTP_CONN_TIMEOUT 			"conn_timeout"
#define HTTP_HEADER_TIMEOUT 			"header_timeout"
#define HTTP_BODY_TIMEOUT 			"body_timeout"
#define HTTP_TOTAL_TIMEOUT 			"total_timeout"
#define HTTP_RETRIES 				"retries"
#define HTTP_SYNCHRONOUS 			"synchronous"

/** Listener functions **/
#define MESIBO_LISTENER_ON_MESSAGE 		"Mesibo_onMessage"
#define MESIBO_LISTENER_ON_MESSAGE_STATUS 	"Mesibo_onMessageStatus"

/** Mesibo module Helper functions **/
#define MESIBO_MODULE_MESSAGE                   "mesibo_message"
#define MESIBO_MODULE_HTTP                      "mesibo_http"
#define MESIBO_MODULE_LOG 			"mesibo_log"  

/**
 * Sample V8 Module Configuration
 * Refer sample.conf
 */
class IMesiboV8 {
	public:
		IMesiboV8(mesibo_module_t* mod, Isolate* isolate, const char* script)
			:mod_(mod), isolate_(isolate), script_(script) {}
		virtual int Initialize();
		virtual ~IMesiboV8() { };
		int ExecuteJsFunction(const char* func_name, int argc, Local<Value> argv[]);

		//Callbacks to Javascript
		mesibo_int_t notify_on_message(mesibo_message_params_t* p, const char* message,
				mesibo_uint_t len);

	private:
		int ExecuteScript(Local<String> script);
		MaybeLocal<String> ReadFile(Isolate* isolate, const string& name);
		Isolate* GetIsolate() { return isolate_; }
		Isolate* isolate_;
		mesibo_module_t* mod_;
		const char* script_;
		Global<Context> context_; //Load base context from here 


		//Callables from Javscript
		static void LogCallback(const v8::FunctionCallbackInfo<v8::Value>& args); 
		static void MessageCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
		
		//Utils
		void Log(const char* format, const char* event);
		Local<Object> mesibo_v8_utils_create_params(Local<Context>& context ,mesibo_message_params_t* p);

		int mesibo_v8_utils_set_param_uint(Local<Context>& context, Local<Object> &js_params, 
				const char* key, mesibo_uint_t value);

		int mesibo_v8_utils_set_param_string(Local<Context>& context, Local<Object> &js_params, 
				const char* key, const char* value);
};

void IMesiboV8::Log(const char* format, const char* event) {
	mesibo_log(mod_,0, format , event);
}


typedef struct v8_config_s{
	const char* script;
	int log; //log level
	long last_changed; // Time Stamp
	IMesiboV8* ctx; // v8 context
	std::unique_ptr<v8::Platform> platform;

} v8_config_t;

typedef struct http_context_s {
	mesibo_module_t *mod;
	char buffer[HTTP_BUFFER_LEN];
	int datalen;	
	mesibo_message_params_t *params;

	char *http_cb;
	char *http_cbdata;
	module_http_option_t* http_opt;
} http_context_t;

static const mesibo_uint_t mesibo_v8_util_get_uint(Isolate* isolate,Local<Context>context, 
		Local<Value>integer){ 
	
	double value;

	if(integer->IsNumber()){
			value = integer->NumberValue(context).ToChecked();
			return 	value;
	}
	//Notify error
	return 0;
}

static const std::string mesibo_v8_util_get_bytes(Isolate* isolate, Local<Context> context, 
		Local<Value> byte_string , Local<Value>len){
	//(TO-DO)Can I assume it is a string?
	
	if(byte_string->IsString()){
		String::Utf8Value value(isolate, byte_string);
		std::string str_value(*value);
		
		return str_value;
	}

	return NULL;
}

Local<Value> mesibo_v8_util_get_param(Isolate* isolate, Local<Context> context, 
		const char* param_name, Local<Object>params){
	
	// Create a handle scope to keep the temporary object references.
	EscapableHandleScope handle_scope(isolate);
	Context::Scope context_scope(context);

	Local<Value> p_key;
	Local<Value> p_value;

	p_key = String::NewFromUtf8(isolate, param_name).ToLocalChecked();
	p_value = params->Get(context, p_key).ToLocalChecked();
      	
	return  handle_scope.Escape(p_value);	
}

static mesibo_uint_t mesibo_v8_util_get_param_uint(Isolate* isolate, Local<Context> context,     
		const char* param_name, Local<Object>params){
	
	v8::Isolate::Scope isolateScope(isolate);
	// Create a handle scope to keep the temporary object references.
	HandleScope handle_scope(isolate);

	Local<Value> value = mesibo_v8_util_get_param(isolate, context, param_name, params);
	
	return mesibo_v8_util_get_uint(isolate, context, value);		
	}

static std::string  mesibo_v8_util_get_param_string(Isolate* isolate, Local<Context> context,     
		const char* param_name, Local<Object>params){
	
	v8::Isolate::Scope isolateScope(isolate);
	// Create a handle scope to keep the temporary object references.
	HandleScope handle_scope(isolate);

	Local<Value> value = mesibo_v8_util_get_param(isolate, context, param_name, params);
	Local<Value> len = v8::Integer::NewFromUnsigned(isolate, 0);

	return mesibo_v8_util_get_bytes(isolate, context, value, len);		
	}

static mesibo_message_params_t* mesibo_v8_util_get_params(Isolate* isolate, Local<Context> context,
		Local<Value> arg_params){
	
	assert(arg_params->IsObject());
	
	Local<Object> params = arg_params->ToObject(context).ToLocalChecked();

	mesibo_message_params_t* mp = (mesibo_message_params_t*)calloc(1, sizeof(mesibo_message_params_t));
	
	mp->aid = mesibo_v8_util_get_param_uint(isolate, context, MESSAGE_AID, params);	
 	mp->id = mesibo_v8_util_get_param_uint(isolate, context, MESSAGE_ID, params);	
 	mp->refid = mesibo_v8_util_get_param_uint(isolate, context, MESSAGE_REFID, params);	
 	mp->groupid = mesibo_v8_util_get_param_uint(isolate, context, MESSAGE_GROUPID, params);	
 	mp->flags = mesibo_v8_util_get_param_uint(isolate, context, MESSAGE_FLAGS, params);	
 	mp->type = mesibo_v8_util_get_param_uint(isolate, context, MESSAGE_TYPE, params);	
 	mp->expiry = mesibo_v8_util_get_param_uint(isolate, context, MESSAGE_EXPIRY, params);	
 	mp->to_online = mesibo_v8_util_get_param_uint(isolate, context, MESSAGE_TO_ONLINE, params);	
 	mp->to = strdup(mesibo_v8_util_get_param_string(isolate, context, MESSAGE_TO, params).c_str());	
 	mp->from = strdup(mesibo_v8_util_get_param_string(isolate, context, MESSAGE_FROM, params).c_str());	
	
	return mp;
}

void IMesiboV8::LogCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
	if (args.Length() < 1) return;
	Isolate* isolate = args.GetIsolate();
	HandleScope scope(isolate);
	Local<Value> arg = args[0];
	Local<External> mod_cb = args.Data().As<External>();

	String::Utf8Value value(isolate, arg);

	//This can be context of v8 object
	mesibo_module_t* mod =  static_cast<mesibo_module_t*>(mod_cb->Value()); 
	mesibo_log(mod, MODULE_LOG_LEVEL_0VERRIDE , "Logged: %s\n", *value);
	//Log("Logged %s\n", *value);

}

void IMesiboV8::MessageCallback(const v8::FunctionCallbackInfo<v8::Value>& args){
	std::cout << "MessageCallback"<< std::endl;
	if (args.Length() < 1) return;
	Isolate* isolate = args.GetIsolate();
	HandleScope scope(isolate);
	
	v8::Local<v8::Context> context = Context::New(isolate);
	Context::Scope context_scope(context);
	
	Local<External> mod_cb = args.Data().As<External>();
	//TBD: This can be reference to v8 object.From there we can take mod
	Local<Value> arg_params = args[0];
	Local<Value> arg_message = args[1];
	Local<Value> arg_len = args[2];

	mesibo_module_t* mod;
	mesibo_message_params_t* p;
	std::string message;
	mesibo_uint_t len;

	//String::Utf8Value value(isolate, arg);
	mod = static_cast<mesibo_module_t*>(mod_cb->Value()); 

	mesibo_log(mod, 0, "====> Creating params for Sending Message \n");
	
	p= mesibo_v8_util_get_params(isolate, context, arg_params);
	message = mesibo_v8_util_get_bytes(isolate, context, arg_message, arg_len); //Pass len, and get raw bytes
	len = mesibo_v8_util_get_uint(isolate, context, arg_len);
 	
	cout<< p->aid<< p->from << p->to << p->id << p->refid << p->groupid<< p->flags << p->type << p->expiry << p->to_online << message << len <<endl;
/**	
	mesibo_log(mod, 0, " aid %u from %s to %s id %u refid %u groupid %u flags %u type %u expiry %u to_online %u %u message %s len %u\n",
			params->aid ,params->from, params->to, params->id,params->refid,
			params->groupid, params->flags, params->type, params->expiry, 
			params->to_online, message.c_str(), len);
**/	
	mesibo_log(mod, 0, "====> Sending Message");
//	int rv = mesibo_message(mod, p, message.c_str(), len);
	//cout << rv << endl;
};

int IMesiboV8::Initialize(){
	// Create a handle scope to hold the temporary references.
	HandleScope handle_scope(GetIsolate());

	// Create a template for the global object where we set the
	// built-in global functions.

	Local<ObjectTemplate> global = ObjectTemplate::New(GetIsolate());
	
	global->Set(String::NewFromUtf8(GetIsolate(), "log", NewStringType::kNormal)
			.ToLocalChecked(),
			FunctionTemplate::New(GetIsolate(), 
				LogCallback, External::New(GetIsolate(), (void*)mod_)));

	global->Set(String::NewFromUtf8(GetIsolate(), "message", NewStringType::kNormal)
			.ToLocalChecked(),
			FunctionTemplate::New(GetIsolate(),
				MessageCallback, External::New(GetIsolate(), (void*)mod_)));

	// Each module script gets its own context so different script calls don't
	// affect each other. Context::New returns a persistent handle which
	// is what we need for the reference to remain after we return from
	// this method. That persistent handle has to be disposed in the
	// destructor.

	v8::Local<v8::Context> context = Context::New(GetIsolate(), NULL, global);
	context_.Reset(GetIsolate(), context);

	// Enter the new context so all the following operations take place
	// within it.

	Context::Scope context_scope(context);

	// Setup error handling before script is read
	v8::Local<v8::String> source = ReadFile(GetIsolate(), script_).ToLocalChecked();
	v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();

	v8::TryCatch tryCatch(GetIsolate());
	v8::MaybeLocal<v8::Value> result = script->Run(context);
	if (result.IsEmpty()) {
		v8::String::Utf8Value e(GetIsolate(), tryCatch.Exception());
		std::cerr << "Exception: " << *e << std::endl;
		return -1;
	}

	return 0;
}

int IMesiboV8::mesibo_v8_utils_set_param_uint(Local<Context>& context, Local<Object> &js_params, 
		const char* key, mesibo_uint_t value){

	v8::Local<v8::Value> p_key;
	v8::Local<v8::Value> p_value;

	p_key = String::NewFromUtf8(GetIsolate(), key).ToLocalChecked();
	p_value = v8::Integer::NewFromUnsigned(GetIsolate(), value);

	v8::Local<v8::Value> set_param ; //Check return value of set
	js_params->Set(context, p_key, p_value).FromMaybe(&set_param);

	return MESIBO_RESULT_OK;
}


int IMesiboV8::mesibo_v8_utils_set_param_string(Local<Context>& context, Local<Object> &js_params, 
		const char* key, const char* value){

	v8::Local<v8::Value> p_key;
	v8::Local<v8::Value> p_value;

	p_key = String::NewFromUtf8(GetIsolate(), key).ToLocalChecked();
	p_value = String::NewFromUtf8(GetIsolate(), value).ToLocalChecked();

	v8::Local<v8::Value> set_param ; //Check return value of set
	js_params->Set(context, p_key, p_value).FromMaybe(&set_param);

	return MESIBO_RESULT_OK;
}
//Seperate Utils Class
// Should it modify existing object or create and return new object. Then it should be persistent allocation
// Also, message params should be in valid/default state
Local<Object> IMesiboV8::mesibo_v8_utils_create_params(Local<Context>& context, mesibo_message_params_t* p){

	// Local scope for temporary handles.
	EscapableHandleScope handle_scope(GetIsolate());

	v8::Local<v8::Object> js_params = v8::Object::New(GetIsolate());

	mesibo_v8_utils_set_param_uint(context, js_params, MESSAGE_AID,       p->aid);
	mesibo_v8_utils_set_param_uint(context, js_params, MESSAGE_ID,        p->id);
	mesibo_v8_utils_set_param_uint(context, js_params, MESSAGE_REFID,     p->refid);
	mesibo_v8_utils_set_param_uint(context, js_params, MESSAGE_GROUPID,   p->groupid);
	mesibo_v8_utils_set_param_uint(context, js_params, MESSAGE_FLAGS,     p->flags);
	mesibo_v8_utils_set_param_uint(context, js_params, MESSAGE_TYPE,      p->type);
	mesibo_v8_utils_set_param_uint(context, js_params, MESSAGE_EXPIRY,    p->expiry);
	mesibo_v8_utils_set_param_uint(context, js_params, MESSAGE_TO_ONLINE, p->to_online);
	mesibo_v8_utils_set_param_string(context, js_params, MESSAGE_TO,      strdup(p->to));
	mesibo_v8_utils_set_param_string(context, js_params, MESSAGE_FROM,    strdup(p->from));

	// Return the result through the current handle scope.  Since each
	// of these handles will go away when the handle scope is deleted
	// we need to call Close to let one, the result, escape into the
	// outer handle scope.
	return handle_scope.Escape(js_params);
}


//Note: Modify to take v8 string or directly the function reference, otherwise it has to fetch reference with every call
int IMesiboV8::ExecuteJsFunction(const char* func_name, int argc, Local<Value> argv[]){

	mesibo_log(mod_, 0, "Executing function %s \n", func_name);

	v8::Isolate::Scope isolateScope(GetIsolate());

	// Create a handle scope to keep the temporary object references.
	HandleScope handle_scope(GetIsolate());

	v8::Local<v8::Context> context = v8::Local<v8::Context>::New(GetIsolate(), context_);

	//Enter the context
	Context::Scope context_scope(context);

	v8::Local<v8::String>js_func_name =
		v8::String::NewFromUtf8(GetIsolate(), func_name).ToLocalChecked();
	v8::Local<v8::Value> js_ref;

	//Note: Set up exception handler
	if (!context->Global()->Get(context, js_func_name).ToLocal(&js_ref) ||
			!js_ref->IsFunction()) {
		return MESIBO_RESULT_FAIL;
	}

	//Note: We can store all the function references, validate them and store it inside V8 object
	//for Mesibo_onMessage, Mesibo_onMessageStatus, Mesibo_onLogin
	if (js_ref->IsFunction()) {
		v8::Local<v8::Function> js_fun = v8::Local<v8::Function>::Cast(js_ref);
		v8::Local<v8::Value> js_result ;

		v8::TryCatch tryCatch(GetIsolate());
		if(!js_fun->Call(context, context->Global(), argc, argv).ToLocal(&js_result)){
			mesibo_log(mod_, 0, "%s call failed", func_name);
			return MESIBO_RESULT_FAIL;
		}

		mesibo_log(mod_,0,"%s returned %s \n", func_name, 
				*(v8::String::Utf8Value(GetIsolate(), js_result)));

	} else {
		mesibo_log(mod_,0,"%s is not a function \n", func_name); 
		return MESIBO_RESULT_FAIL;
	}
	return MESIBO_RESULT_OK;
}

// Reads a file into a v8 string.
MaybeLocal<String> IMesiboV8::ReadFile(Isolate* isolate, const string& name) {
	FILE* file = fopen(name.c_str(), "rb");
	if (file == NULL) return MaybeLocal<String>(); //Error reading file. Return MESIBO_RESULT_FAIL	

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	rewind(file);

	std::unique_ptr<char> chars(new char[size + 1]);
	chars.get()[size] = '\0';
	for (size_t i = 0; i < size;) {
		i += fread(&chars.get()[i], 1, size - i, file);
		if (ferror(file)) {
			fclose(file);
			return MaybeLocal<String>();
		}
	}
	fclose(file);
	MaybeLocal<String> result = String::NewFromUtf8(
			isolate, chars.get(), NewStringType::kNormal, static_cast<int>(size));
	return result;
}


IMesiboV8* mesibo_v8_init(mesibo_module_t* mod, v8_config_t* vc){
	//std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
	vc->platform = v8::platform::NewDefaultPlatform();

	v8::V8::InitializePlatform(vc->platform.get());
	v8::V8::Initialize();

	v8::Isolate::CreateParams createParams;
	createParams.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	v8::Isolate* isolate = v8::Isolate::New(createParams);

	std::cout << v8::V8::GetVersion() << std::endl;

	//HandleScope handle_scope(isolate);
	//v8::Isolate::Scope isolate_scope(isolate);
	const char* script_path = strdup(vc->script);

	IMesiboV8* mesibo_js = new IMesiboV8(mod, isolate, script_path);

	mesibo_js->Initialize();

	return mesibo_js;
}

/**
 * Notify JS Callback function Mesibo_onMessage
 * Push parmeters as objects into the duktape stack and evaluate
 **/

mesibo_int_t IMesiboV8::notify_on_message(mesibo_message_params_t* p, const char* message,
		mesibo_uint_t len){
	mesibo_log(mod_, 0, "notify_on_message called\n");	

	v8::Locker locker(GetIsolate());
	v8::Isolate::Scope isolateScope(GetIsolate());

	// Create a handle scope to keep the temporary object references.
	HandleScope handle_scope(GetIsolate());

	//To each it's own context
	v8::Local<v8::Context> context = v8::Local<v8::Context>::New(GetIsolate(), context_);

	Context::Scope context_scope(context);

	v8::Handle<v8::Value> args_bundle[3];
	v8::Local<v8::Object> params = mesibo_v8_utils_create_params(context,p); // Don't pass context
	//Replace with something like GetCurrentContext

	args_bundle[0] = params; 
	args_bundle[1] = v8::String::NewFromUtf8(GetIsolate(), message).ToLocalChecked();
	args_bundle[2] = v8::Integer::NewFromUnsigned(GetIsolate(), len); //use utils to check for errors

	ExecuteJsFunction("Mesibo_onMessage", 3, args_bundle);

	//Return whatever JS Callback is returning, But that would mean blocking until return value
	return MESIBO_RESULT_OK;

}

static mesibo_int_t v8_on_message(mesibo_module_t *mod, mesibo_message_params_t *p, const char *message,
		mesibo_uint_t len) {
	mesibo_log(mod, 0, "================> %s on_message called\n", mod->name);
	mesibo_log(mod, 0, " from %s to %s id %u message %s\n", p->from, p->to, (uint32_t) p->id, message);
	 
	//don't modify original as other module will be use it 
	mesibo_message_params_t np;
	memcpy(&np, p, sizeof(mesibo_message_params_t));
	np.to = p->from;
	np.from = p->to;
	np.id = rand();
	const char* test_message = "Hello from V8 Module";

        mesibo_message(mod, &np, test_message, strlen(message));



	v8_config_t* vc = (v8_config_t*)mod->ctx;
	IMesiboV8* mesibo_js = vc->ctx;
	mesibo_message_params_t* mp = (mesibo_message_params_t *)calloc(1, sizeof(mesibo_message_params_t));
	memcpy(mp, p, sizeof(mesibo_message_params_t));
	//mesibo_js->notify_on_message(mp, message, len);	
	
	return MESIBO_RESULT_PASS; 
}

/**
 * Helper function for getting v8 configuration
 * Gets /path/to/script which contains Javascript code
 **/
v8_config_t* get_config_v8(mesibo_module_t* mod){

	v8_config_t* vc = (v8_config_t*)calloc(1, sizeof(v8_config_t));
	vc->script = mesibo_util_getconfig(mod, "script");
	vc->log = atoi(mesibo_util_getconfig(mod, "log"));
	vc->last_changed = 0; //Initialize TS to zero
	vc->ctx = NULL;
	mesibo_log(mod, vc->log, "V8 Module Configured : script %s log %d\n", vc->script, vc->log);

	return vc;
}


MESIBO_EXPORT int mesibo_module_v8_init(mesibo_int_t version, mesibo_module_t *m, mesibo_uint_t len) {
	MESIBO_MODULE_SANITY_CHECK(m, version, len);

	if(m->config) {
		v8_config_t* vc = get_config_v8(m);
		if(NULL == vc){
			mesibo_log(m, MODULE_LOG_LEVEL_0VERRIDE, "%s : Missing Configuration\n", m->name);
			return MESIBO_RESULT_FAIL;
		}

		IMesiboV8* mesibo_js = mesibo_v8_init(m, vc);
		vc->ctx = mesibo_js;

		if (!vc->ctx) {
			mesibo_log(m, MODULE_LOG_LEVEL_0VERRIDE, "Loading context failed for script %s", 
					vc->script);
			return MESIBO_RESULT_FAIL;
		}
		m->ctx = (void*)vc;
	}

	else {
		mesibo_log(m, MODULE_LOG_LEVEL_0VERRIDE, "%s : Missing Configuration\n", m->name);
		return MESIBO_RESULT_FAIL;
	}


	m->flags = 0;
	m->description = strdup("Sample V8 Module");
	m->on_message= v8_on_message;

	return MESIBO_RESULT_OK;
}
