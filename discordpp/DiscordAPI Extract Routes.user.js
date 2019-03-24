// ==UserScript==
// @name         DiscordAPI Extract Routes
// @namespace    http://tampermonkey.net/
// @version      0.1
// @description  Extract routs from the Discord API pages to use for rate limiting in libraries
// @author       Aidoboy
// @match        https://discordapp.com/developers/docs/*
// @grant        none
// @require https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js
// ==/UserScript==

function KeyCheck(e){
    //alert(e.keyCode);
    if(e.keyCode === 70){
        scanythingamabob("","");
    }else if(e.keyCode === 71){
        scanythingamabob("\"","\",");
    }
}

window.addEventListener('keydown', KeyCheck, true);

function scanythingamabob(before, after){
    var result = "";
    $(".http-req").each(function() {
        //console.log(parseInt($(this).text().split("\n")[2].split(" ")[0].replace(/\,/g,'')));
        var verbs = $(this).find(".http-req-verb").text().split('/');
        var url = $(this).find(".http-req-url").text().replace(/\{(?!(channel\.id|guild\.id|webhook\.id)).+?\}/g, "{}");
        verbs.forEach(function(verb) {
            //console.log(before + verb + url + after);
            result += before + verb + url + after + '\n';
        });
    });
    console.log(result);
}