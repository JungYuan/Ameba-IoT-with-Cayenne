void callback(char* topic, byte* payload, unsigned int length) {
  //Received the Subscribe cayenne define  v1/username/things/clientID/cmd/channel
  //Received the payload cayenne define  sequence,value
  String  msgtopic_split[6], msgcommand[2], publishTopic, publishpayload, pubReturntopic, pubReturnPayload;
  char c_pub_topic[255],c_pub_payload[128];
  
  //Split topic to msgtopic 
  int i=0;

  for (int j=0; j<strlen(topic); j++) {
      if (topic[j] == '/') {
        i = i+1;
      } else {
        msgtopic_split[i] += topic[j]; 
      } 
  }
  
  //Split payload to msgcommand 
  i=0;

  for (int j=0; j<length; j++) {
      if ((char) payload[j] == ',') {
        i = i+1;
      } else {
        msgcommand[i] += (char)payload[j]; 
      } 
  }
  Serial.print("Message arrived :");
  for (i=0; i<6;i++) Serial.println(msgtopic_split[i]); 
  Serial.println();
  for (i=0; i<2;i++) Serial.println(msgcommand[i]); 
  // 
  //Control cmd I/O control and return meaage 
  //  (v1/username/things/clientID/data/channel,status)
  //  (v1/username/things/clientID/response, ok,seq or error,seq=message)
  //
  if (msgtopic_split[4] == "cmd") {
     publishTopic = ""; 
     publishpayload = "";
     pubReturntopic = "";
     pubReturnPayload = "";
     publishTopic = publishTopic + msgtopic_split[0]+"/"+msgtopic_split[1]+"/"+msgtopic_split[2]+"/"+msgtopic_split[3];
     pubReturntopic = publishTopic + "/response";
     publishTopic = publishTopic +"/data/"+msgtopic_split[5];   
     int c_p = atoi(msgtopic_split[5].c_str());
     int p_out = atoi(msgcommand[1].c_str());
     for (int ii=0; ii < outpin_num; ii++) {
        if (v_pin[ii] == c_p) {
            c_p = ii;
            break;
        }
     }
     if (p_out == 0) { 
        publishTopic.toCharArray(c_pub_topic, 255);
        client.publish(c_pub_topic, "0");
        digitalWrite(c_pin[c_p], LOW);
        c_status[c_p]=0;
        pubReturnPayload = pubReturnPayload +"ok,"+msgcommand[0];
     } else if ( p_out == 1) { 
        publishTopic.toCharArray(c_pub_topic, 255);
        client.publish(c_pub_topic, "1");
        digitalWrite(c_pin[c_p], HIGH);
        c_status[c_p]=1;
        pubReturnPayload = pubReturnPayload +"ok,"+msgcommand[0];
     } else {
        pubReturnPayload = pubReturnPayload +"error,"+msgcommand[0]+"= unknow CMD";
     }   
     pubReturntopic.toCharArray(c_pub_topic, 255);
     pubReturnPayload.toCharArray(c_pub_payload, 128);
     client.publish(c_pub_topic, c_pub_payload);  
     Serial.println(c_pub_topic); 
     Serial.println(c_pub_payload); 
  }
}


void reconnect() {
    String t_sub(subscribeTopic);
    char t_subscribeTopic[255],t_msg[25]; 
    // 指定用戶端ID並連結MQTT伺服器
    if (client.connect(clientId, clientUser, clientPass)) {
      // 若連結成功，在序列埠監控視窗顯示「已連線」。
      // update virtual pin status and assign subscribe topic
      for (int i=0; i < outpin_num; i++) {
        t_sub = t_sub +"data/"+v_pin[i];
        t_sub.toCharArray(t_subscribeTopic, 255);
        sprintf (t_msg,"%d", c_status[i]);
        client.publish(t_subscribeTopic, t_msg); 
        t_sub = String(subscribeTopic);
        t_sub = t_sub +"cmd/"+v_pin[i];
        t_sub.toCharArray(t_subscribeTopic, 255);
        client.subscribe(t_subscribeTopic);
        t_sub = String(subscribeTopic);
      }
      Serial.println("connected");
    } else {
      // 若連線不成功，則顯示錯誤訊息
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in next time !");
    }
}



