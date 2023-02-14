
#include <NewPing.h>
//defining where the components are attached
#define TRIG_IN A1
#define TRIG_OUT 3
#define ECHO_IN A2
#define ECHO_OUT 4
#define LED_WAIT 12
#define LED_ENTER 9


#define iterations 8 // Number of readings in the calibration stage
#define MAX_DISTANCE 150// Maximum distance (in cm) for the sensors to try to read
#define DEFAULT_DISTANCE 45 // Default distance (in cm) is only used if calibration fails
#define MIN_DISTANCE 15 // minimun distance (in cm) for calibrated threshold


float calibrate_in = 0, calibrate_out = 0;//the calibration in the setup() function will set these  to appropriate
float distance_in, distance_out; // these are the distances (in cm) that each of the ultrasonic sensor read
int count=0, limit =5; //Occupancy limit should be set here : e.g for maximum 1000 people in the house set 'limit=1000'
bool prev_inblocked = false, prev_outblocked = false; //these booleans record whether the entry /exit was blocked on the previous reading of the sensor 


NewPing sonar[2]={//sensor object array.
NewPing(TRIG_IN,ECHO_IN,MAX_DISTANCE)  ,// Each sensor's trigger pin , echo pin , and max distance to ping.
NewPing( TRIG_OUT,ECHO_OUT,MAX_DISTANCE)
};
/*

 A quick note that the sonar.ping_cm() function returns 0 (cm ) if the object is out of range /nothing is detected
 

*/


/**************************************************************Network***********************************/

#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
//IPAddress ip(133,68,14,164);// uncomment this line if you want define static IP instead of dynamic IP from DHCP server

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

/**************************************************************************************************************/



void setup() {



  // put your setup code here, to run once:
Serial.begin(115200);//Open serial monitor at 115200 baud to see the ping results
Serial.println("waiting for DHCP IP...");

/*
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
*/

  // start the Ethernet connection and the server:
  Ethernet.begin(mac); // if you defined static ip in line 28, change this line to   Ethernet.begin(mac,ip); 
  server.begin();
  Serial.print("Please use your browser to visit http://");
  Serial.println(Ethernet.localIP());
  /************************************************************************************************************/

pinMode(2,OUTPUT);pinMode(5,OUTPUT);pinMode(A0,OUTPUT);pinMode(A3,OUTPUT);pinMode(11,OUTPUT);
digitalWrite(2,HIGH);digitalWrite(5,LOW);digitalWrite(A0,HIGH);digitalWrite(A3,LOW); digitalWrite(11,LOW);
pinMode(LED_WAIT,HIGH);digitalWrite(LED_ENTER,HIGH);//Both LEDs are lit to alert user to ongoing calibration
Serial.println('calibrating...');
delay(1500);
for (int a=0; a< iterations; a++){
  delay(50);
  calibrate_in+=sonar[0].ping_cm();
  delay(50);
  calibrate_out+=sonar[1].ping_cm();
  delay(200);



}
calibrate_in= 0.75*calibrate_in/iterations;//the threshold is set at 75% of the average  of these readings . this should prevent the system counting people if it is knocked.
calibrate_out=0.75*calibrate_out/iterations;

if (calibrate_in>MAX_DISTANCE || calibrate_in<MIN_DISTANCE){ // if the calibration gave a reading outside of sensible bounds , then the default is used 
  calibrate_in=DEFAULT_DISTANCE;
}

if (calibrate_out>MAX_DISTANCE || calibrate_out<MIN_DISTANCE){ // if the calibration gave a reading outside of sensible bounds , then the default is used 
  calibrate_out=DEFAULT_DISTANCE;
}
Serial.print("Entry threshold set to :");
Serial.println(calibrate_in);
Serial.print("Exit threshold set to ");
Serial.println(calibrate_out);
digitalWrite(LED_WAIT,LOW);digitalWrite(LED_ENTER,LOW);//Both LEDs are off to alert user that calibration has finished.
delay(1000);



}

void loop() {
  // put your main code here, to run repeatedly:




  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
        if (client.available()) {
          char c = client.read();
        //  Serial.write(c);
          // if you've gotten to the end of the line (received a newline
          // character) and the line is blank, the http request has ended,
          // so you can send a reply
          if (c == '\n' && currentLineIsBlank) {
            // send a standard http response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");  // the connection will be closed after completion of the response
            client.println("Refresh: 5");  // refresh the page automatically every 5 sec
              //Serial.print("Count:");
            //Serial.println(count);



             client.println();
            client.println("<!DOCTYPE HTML>");
             client.println("<html><head>");
              
 client.println("<link rel=\"stylesheet\" href=\"https://unpkg.com/leaflet@1.6.0/dist/leaflet.css\">");
 client.println("<script src=\"https://unpkg.com/leaflet@1.6.0/dist/leaflet.js\"></script>");
client.println("</head>");
client.println("<body>");
  client.println("<div id=\"mapid\" style=\"width:100%;height:500px\"></div>");
 client.println("<script>");
  client.println("var mymap = L.map(\'mapid\').setView([35.158, 136.924], 16);");
  client.println("L.tileLayer('http://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png').addTo(mymap);var markerNaCastle = L.marker([35.184772,136.9001]).addTo(mymap);");
  client.println("var markerNaStation = L.marker([35.170694,136.881637]).addTo(mymap);");
  client.println("var circleNitech = L.circle([35.15665, 136.92462], {color: 'blue',fillColor: '#0000FF',fillOpacity: 0.5,radius: 500}).addTo(mymap);");

//client.println("var polygon = L.polygon([[35.184772,136.9001],[35.170694,136.881637],[35.15665,136.92462]],{color: 'green',fillColor: '#00FF00',fillOpacity: 0.5,radius: 500}).addTo(mymap);");

client.println("var popup = L.popup().setLatLng([35.15665,136.92462]).setContent(\"Nagoya Institute of Technology.\").openOn(mymap);");
 
client.println("</script>");


                          distance_in=sonar[0].ping_cm();
                          delay(40);
                          distance_out=sonar[1].ping_cm();
                          delay(40);

                          if(distance_in < calibrate_in && distance_in > 0){

                            if (prev_inblocked ==false){
                              count++;// Increase count by one 
                              client.println("<div><h1>Count: </h1></div>")    ;
                              client.println(count);


                            }
                            prev_inblocked=true;
                              
                          } else{
                            prev_inblocked=false;
                            }

                          if(distance_out < calibrate_out && distance_out > 0){
                             if (count>0){
                                if (prev_outblocked ==false){
                              
                              
                             

                              count--;// Decrease count by one 
                              client.println("<div><h1>Count: </h1></div>")   ;
                              client.println(count);

                              
                             


                            }
                            prev_inblocked=true;
                              
                          } else{
                            prev_inblocked=false;
                            }

                            }

                            //else{
                              //Serial.println("Building Empty")
                            // }
                           
                          // // If there are fewer people in the house than the limit , light is green , else it is red
                          if (count<limit){
                            digitalWrite(LED_WAIT,LOW);
                            digitalWrite(LED_ENTER,HIGH);

                          }else{
                            digitalWrite(LED_WAIT,HIGH);
                            digitalWrite(LED_ENTER,LOW);

                          }

                        


                        
         
          client.println("<h4> </h4></body></html>");
 
          break;
        }
        /*if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }*/
      }
    }
    // give the web browser time to receive the data
    delay(10);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
//Serial.print("Count:");
//Serial.println(count);


/*distance_in=sonar[0].ping_cm();
delay(40);
distance_out=sonar[1].ping_cm();
delay(40);
if(distance_in < calibrate_in && distance_in > 0){
  if (prev_inblocked ==false){
    count++;// Increase count by one 
    Serial.print("Count: ")    ;
    Serial.println(count);


  }
  prev_inblocked=true;
    
} else{
  prev_inblocked=false;
  }

if(distance_out < calibrate_out && distance_out > 0){
  if (prev_outblocked ==false){
    count--;// Decrease count by one 
    Serial.print("Count: ")    ;
    Serial.println(count);


  }
  prev_inblocked=true;
    
} else{
  prev_inblocked=false;
  }
// // If there are fewer people in the house than the limit , light is green , else it is red
if (count<limit){
  digitalWrite(LED_WAIT,LOW);
  digitalWrite(LED_ENTER,HIGH);

}else{
  digitalWrite(LED_WAIT,HIGH);
  digitalWrite(LED_ENTER,LOW);

}*/

}

/*int count_1(){

  //Serial.print("Count:");
  //Serial.println(count);
  int result;

  distance_in=sonar[0].ping_cm();
  delay(40);
  distance_out=sonar[1].ping_cm();
  delay(40);
  if(distance_in < calibrate_in && distance_in > 0){
    if (prev_inblocked ==false){
      count++;// Increase count by one 
      Serial.print("Count: ")    ;
      result=Serial.println(count);


    }
    prev_inblocked=true;
      
  } else{
    prev_inblocked=false;
    }

  if(distance_out < calibrate_out && distance_out > 0){
    if (prev_outblocked ==false){
      count--;// Decrease count by one 
      Serial.print("Count: ")    ;
      result=Serial.println(count);


    }
    prev_inblocked=true;
      
  } else{
    prev_inblocked=false;
    }
  // // If there are fewer people in the house than the limit , light is green , else it is red
  if (count<limit){
    digitalWrite(LED_WAIT,LOW);
    digitalWrite(LED_ENTER,HIGH);

  }else{
    digitalWrite(LED_WAIT,HIGH);
    digitalWrite(LED_ENTER,LOW);

  }
 


}*/

