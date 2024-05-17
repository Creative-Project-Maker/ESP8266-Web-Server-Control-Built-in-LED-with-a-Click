#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>

ESP8266WiFiMulti WiFiMulti;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

char webpage[] PROGMEM = R"=====(<html>
    <h1>Built-in LED</h1><label class="switch"><div class="mid">
    <label class="rocker">
      <input type="checkbox" id="myCheck" onclick="sendStatus()" checked>
      <span class="switch-left">On</span>
      <span class="switch-right">Off</span>
    </label>
  
  </div>
  
    <style>
    h1{position:relative;
        font-size: 700%;
        top:70px;
        display: flex;
        width:100%;
        justify-content: center;
        align-items: center;
    }html {
        box-sizing: border-box;
        font-family: cursive;
        font-size: 100%;
      }
      *, *:before, *:after {
        box-sizing: inherit;
        margin:0;
        padding:0;
      }
      .mid {
        position: relative;
        top: 35%;
        scale:2;
        display: flex;flex-direction: column;
        align-items: center;
        justify-content: center;
        padding-top:1em;
      }
      
      
      /* Switch starts here */
      .rocker {
        display: inline-block;
        position: relative;
        font-size: 2em;
        font-weight: bold;
        text-align: center;
        text-transform: uppercase;
        color: #888;
        width: 7em;
        height: 4em;
        overflow: hidden;
        border-bottom: 0.5em solid #eee;
      }
      
      .rocker-small {
        font-size: 0.75em; /* Sizes the switch */
        margin: 1em;
      }
      
      .rocker::before {
        content: "";
        position: absolute;
        top: 0.5em;
        left: 0;
        right: 0;
        bottom: 0;
        background-color: #999;
        border: 0.5em solid #eee;
        border-bottom: 0;
      }
      
      .rocker input {
        opacity: 0;
        width: 0;
        height: 0;
      }
      
      .switch-left,
      .switch-right {
        cursor: pointer;
        position: absolute;
        display: flex;
        align-items: center;
        justify-content: center;
        height: 2.5em;
        width: 3em;
        transition: 0.2s;
      }
      
      .switch-left {
        height: 2.4em;
        width: 2.75em;
        left: 0.85em;
        bottom: 0.4em;
        background-color: #ddd;
        transform: rotate(15deg) skewX(15deg);
      }
      
      .switch-right {
        right: 0.5em;
        bottom: 0;
        background-color: #bd5757;
        color: #fff;
      }
      
      .switch-left::before,
      .switch-right::before {
        content: "";
        position: absolute;
        width: 0.4em;
        height: 2.45em;
        bottom: -0.45em;
        background-color: #ccc;
        transform: skewY(-65deg);
      }
      
      .switch-left::before {
        left: -0.4em;
      }
      
      .switch-right::before {
        right: -0.375em;
        background-color: transparent;
        transform: skewY(65deg);
      }
      
      input:checked + .switch-left {
        background-color: #0084d0;
        color: #fff;
        bottom: 0px;
        left: 0.5em;
        height: 2.5em;
        width: 3em;
        transform: rotate(0deg) skewX(0deg);
      }
      
      input:checked + .switch-left::before {
        background-color: transparent;
        width: 3.0833em;
      }
      
      input:checked + .switch-left + .switch-right {
        background-color: #ddd;
        color: #888;
        bottom: 0.4em;
        right: 0.8em;
        height: 2.4em;
        width: 2.75em;
        transform: rotate(-15deg) skewX(-15deg);
      }
      
      input:checked + .switch-left + .switch-right::before {
        background-color: #ccc;
      }
      
      /* Keyboard Users */
      input:focus + .switch-left {
        color: #333;
      }
      
      input:checked:focus + .switch-left {
        color: #fff;
      }
      
      input:focus + .switch-left + .switch-right {
        color: #fff;
      }
      
      input:checked:focus + .switch-left + .switch-right {
        color: #333;
      }</style><script>var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);connection.onopen = function () {  var checkBox = document.getElementById("myCheck");
  if (checkBox.checked == true){
    connection.send("0");
  } else {
    connection.send("1");
  } }; connection.onerror = function (error) {    console.log('WebSocket Error ', error);};connection.onmessage = function (e) {  console.log('Server: ', e.data);};function sendStatus(){
  var checkBox = document.getElementById("myCheck");
  if (checkBox.checked == true){
    connection.send("0");
  } else {
    connection.send("1");
  }
}</script></html>)=====";

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

            // send message to client
            webSocket.sendTXT(num, "Connected");
        }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);
            digitalWrite(2,(payload[0]-48));
            break;
    }

}

void setup() {
    Serial.begin(115200);

    pinMode(2, OUTPUT);

    digitalWrite(2, 1);
    WiFiMulti.addAP("SSID", "YOUR PASSWORD");//Replace YOUR SSID with your wifi SSID and Replace YOUR PASSWORD with your wifi Password

    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

    // start webSocket server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    if(MDNS.begin("esp8266")) {
        Serial.println("MDNS responder started");
    }

    // handle index
    server.on("/", []() {
        // send index.html
        server.send_P(200, "text/html", webpage);
    });

    server.begin();
    // Add service to MDNS
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);
}

void loop() {
    webSocket.loop();
    server.handleClient();
}