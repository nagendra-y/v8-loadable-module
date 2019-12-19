//test.js

log("Initializing V8");
var p = {"aid":6320,"id":713364881,"refid":0,"groupid":0,"flags":1,"type":0,"expiry":3600,"to_online":0,"to":"test_user_demo","from":"js_user"};
//p.id = parseInt(Math.floor(2147483647*Math.random())); 
p.id = 12345;
message(p,"Hi", 2);

function Mesibo_onMessage(params, message, len){
	log("Mesibo_onMessage called");
	log(JSON.stringify(params));
        log(message);
	log(len);
	
	/**
	log("--- Creating params Sending Message---");
	var p = params;
	var temp = p['from'];
 	p['id'] = parseInt(Math.floor(2147483647*Math.random()));	
	p['from'] = p['to'];
	p['to'] = temp;
	
	log(JSON.stringify(p));
	log("--- Sending Message---");
	
	message(p, message, len);
**/
	return 0;
}


