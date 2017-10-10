// Ameba + MQTT + WiFi libraries to attend Cayenne application without Cayenne libraries.
// Apply MX6675 + k-type thermal couple to measure temperature and send temperature data to Cayenne server.
// Moble APP Cayenne to review the temperature and control on/off of digital pin of Ameba board.

#include <stdio.h> 
#include <WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

char ssid[] = "MySSID";     // your network SSID (name)
char pass[] = "Mypasswd";  // your network password
int status  = WL_IDLE_STATUS;    // the Wifi radio's status

char mqttServer[]     = "mqtt.mydevices.com";
char clientId[]       = "clientID";
char clientUser[]     = "clientUser";
char clientPass[]     = "clientPass";
char publishTopic_0[]   = "v1/clientUser/things/clientID/data/0";
char publishTopic_1[]   = "v1/clientUser/things/clientID/data/1";
char publishPayload[25] = "temp,c=22.2";
char subscribeTopic[255] = "v1/clientUser/things/clientID/";
String msgStr = "";

unsigned long lastconnect, current_time;
unsigned long _timer[10];
unsigned long _tperiod[10] = {1000, 10000, 0};
int v_pin[]={2}, c_pin[]={13}, outpin_num=1, c_status[]={0};



static volatile uint8_t is_tc_open;
WiFiClient wifiClient;
PubSubClient client(wifiClient);


void setup() {
  //Setup WiFi connecting:
  Serial.begin(38400);
  for (int i=0; i<outpin_num; i++) {
      pinMode(c_pin[i], OUTPUT);
      digitalWrite(c_pin[i], c_status[i]);
  }
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // Note - the default maximum packet size is 512 bytes. If the
  // combined length of clientId, username and password exceed this,
  // you will need to increase the value of MQTT_MAX_PACKET_SIZE in
  // PubSubClient.h

  client.setServer(mqttServer, 1883);
  client.setCallback(callback);
  // Connect to MQTT server
  reconnect();
  max6675_init();
  Serial.println("MX6675 id ready for reading temperature"); 
  //Initilize the timer.......
  _timer[0]= millis();
  _timer[1]=_timer[0];
  _timer[2]=_timer[0];
}

void loop() {
    char sensor_data[7];
    static float C,F;
    static uint8_t tc_check=0;
    client.loop();
    current_time=millis();
    // Read temperature
    if ((current_time - _timer[0]) >= _tperiod[0]) {
        _timer[0]=current_time;   
        if( is_tc_open ) {
            Serial.println( "TC is Open !");
            C=999.99;
            F=999.99;
            // 每 5 秒鐘重新確認 TC 是否已重新連接上
           if( tc_check ++ > 5 ){
               max6675_getCelsius();
               tc_check = 0;
           }
         } else {
            C = max6675_getCelsius();
            F = max6675_getFahrenheit();
            Serial.print( C );
            Serial.println(" C");
            Serial.print( F );
            Serial.println(" F");
         }  
    }
    if ((current_time - _timer[1]) >= _tperiod[1]) {
       _timer[1]=current_time;    
       if (!client.connected()) {
          // 若沒有連上，則執行此自訂函式。
          reconnect();
       } else {          
          //Publish sensor data transfer to string by channel 0.
          sprintf (sensor_data,"%6.2f", C); 
          // 建立MQTT訊息（JSON格式的字串）
          msgStr = msgStr + "temp,c=";
          // 把String字串轉換成字元陣列格式
          msgStr.toCharArray(publishPayload, 25);
          //combine head and sensor string data
          strcat(publishPayload, sensor_data);
          // 發布MQTT主題與訊息
          client.publish(publishTopic_0, publishPayload);
          // 清空MQTT訊息內容
          msgStr = "";
          //
          // Publish channel 1
          //
          sprintf (sensor_data,"%6.2f", F); 
          // 建立MQTT訊息（JSON格式的字串）
          msgStr = msgStr + "temp,f=";
          // 把String字串轉換成字元陣列格式
          msgStr.toCharArray(publishPayload, 25);
          //combine head and sensor string data
          strcat(publishPayload, sensor_data);
          // 發布MQTT主題與訊息
          client.publish(publishTopic_1, publishPayload);
          // 清空MQTT訊息內容
          msgStr = "";
       }
    }
}
