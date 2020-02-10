//chatbot.js


//POST https://dialogflow.googleapis.com/v2/projects/project-id/agent/sessions/session-id:detectIntent
//const gDialogflowUrl = "https://dialogflow.googleapis.com/v2/projects/*/agent/sessions/";
//const gDialogflowToken = "";
//const gMesiboChatbotUser = ""; 

initialize();

function initialize(){
	mesibo.onmessage = Mesibo_onMessage;
	mesibo.onmessagestatus = Mesibo_onMessageStatus;
}

function Mesibo_onChatbotResponse(h) {
	print(JSON.stringify(h));
	var rp = h.response;
	var fulfillment = rp.queryResult.fulfillmentText;

	var query = h.query;
	var chatbotResponse = query;
	chatbotResponse.from = query.to;
	chatbotResponse.to = query.from;
	chatbotResponse.message = fulfillment;
	chatbotResponse.refid = query.id;
	chatbotResponse.id = parseInt(Math.floor(2147483647*Math.random()));
	
	print(chatbotResponse.message);
	chatbotResponse.send();

	return mesibo.RESULT_OK;
}

function Mesibo_onMessage(m) {
	//Filter messages which are addressed to the chatbot
	print(JSON.stringify(m));
	var dHttp = new Http();
	print(JSON.stringify(dHttp));
	//POST https://dialogflow.googleapis.com/v2/projects/project-id/agent/sessions/session-id:detectIntent
	dHttp.url =  gDialogflowUrl + m.id + ":detectIntent";
	var request_body = {
		"queryInput": {
			"text": {
				"text": m.message,
				"languageCode" : "en"
			}
		}
	};
	dHttp.post = JSON.stringify(request_body);
	dHttp.ondata = Mesibo_onChatbotResponse;
	dHttp.query = m;
	dHttp.headers = "Authorization: Bearer "+ gDialogflowToken;  
	dHttp.contentType = 'application/json';
	print(JSON.stringify(dHttp));
	dHttp.send();
	return mesibo.RESULT_OK;
}


function Mesibo_onMessageStatus(message) {
        return mesibo.RESULT_OK;
}

