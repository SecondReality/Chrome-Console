pastCommands = [];
 
function eventHandler(e)
{
	var plugin = document.getElementById("pluginId");
	var evt = e || window.event;
	//writeData(evt.type);

	if(evt.keyCode == 13) // if enter is pressed
	{

		var textField = document.getElementById("in");

		//document.getElementById("output").innerHTML+=textField.value;
		document.getElementById("output").innerHTML+="<br />";
		
		plugin.consoleInput(textField.value);
		pastCommands.push(textField.value);
		
		// Clear the text field:
		textField.value="";
	}
	else
	{
		//sendString+= String.fromCharCode(evt.charCode);
		//document.getElementById("output").innerHTML+=String.fromCharCode(evt.charCode);
	}
	
	return document.defaultAction;
}

function eventHandler2(e) 
{
	var evt = e || window.event;
	
	// If the up key is pressed replace the command with a previous one:
	if(evt.keyCode==38 && pastCommands.length>0)
	{
		var textField = document.getElementById("in");
		var findIndex = pastCommands.indexOf(textField.value);
		if(findIndex==-1)
		{
			textField.value=pastCommands[ pastCommands.length -1];
		}
		else
		{
			if(findIndex==0)
			{
			  findIndex=pastCommands.length;
			}
			textField.value=pastCommands[findIndex-1];
		}
	}
	return document.defaultAction;
}

document['onkeydown'] = eventHandler2;
document['onkeypress'] = eventHandler;

function scrollToBottom()
{
	var objDiv = document.getElementById("output");
	objDiv.scrollTop = objDiv.scrollHeight;
}

function resizeOutputArea()
{
	$("#output").height($(window).height() - $("#input-area").height() - 6 - 6 );
	scrollToBottom();
}

$(function()
{
	var plugin = document.getElementById("pluginId");
	var result = plugin.setOutputHandler(function(argument)
	{
		document.getElementById("output").innerHTML+=argument;
		scrollToBottom();	
		document.getElementById("in").focus();
	});
	resizeOutputArea();
});

$(window).resize(resizeOutputArea);