//mesibo_test.js

const MESIBO_RESULT_OK          = 0
const MESIBO_RESULT_FAIL        = -1

const MESIBO_RESULT_CONSUMED    = 1 ;
const MESIBO_RESULT_PASS        = 0 ;

mesibo_log("Initializing V8");
//var p = {"aid":6320,"id":713364881,"refid":0,"groupid":0,"flags":1,"type":0,"expiry":3600,"to_online":0,"to":"test_user_demo","from":"js_user"};
//p.id = parseInt(Math.floor(2147483647*Math.random())); 
//mesibo_message(p,"Hi", 2);
//----Call Initialize----//


// Example http response callback
function Mesibo_onHttpResponse(mod, cbdata, response, datalen) {
        var rp_log = " ----- Recieved http response ----";
        mesibo_log(rp_log);
        mesibo_log(response);

        p = JSON.parse(cbdata);
        mesibo_log(rp_log);

        return MESIBO_RESULT_OK;
}


function Mesibo_onMessage(params, m, len){
	mesibo_log("Mesibo_onMessage called");
	
	mesibo_log(JSON.stringify(params));
        mesibo_log(m);
	mesibo_log(len);
	
	mesibo_log("--- Creating params Sending Message---");
	var p = params;
	var temp = p['from'];
 	p['id'] = parseInt(Math.floor(2147483647*Math.random()));	
	p['from'] = p['to'];
	p['to'] = temp;
	
	mesibo_log(JSON.stringify(p));
	mesibo_log(mesibo_http);
	mesibo_log("--- Sending Message---");

	mesibo_message(p, m, len);
	//log(mesibo_http);
	mesibo_http("https://example.com/api.php", "op=test", "2", "3","4" );
	
	return MESIBO_RESULT_OK;
}

function Mesibo_onMessageStatus(p, s){
	mesibo_log("Mesibo_onMessageStatus called");
	mesibo_log(JSON.stringify(p));
	mesibo_log(s);

	return MESIBO_RESULT_OK;
}
