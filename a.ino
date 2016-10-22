#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "4UNION";
const char* password = "";

#define API_KEY "bcf081da-32a1-42c6-a9c1-ba7068005e7d";
#define PROJECT_ID 5621;
#define DEVICE_UUID "108f8baa-ec02-434b-9227-597966a053b2";
#define ACTUATOR1_NAME "Rele1"

#define sec 1000

char clientId[]          = "Teste";
char actuator1Topic[]     = "/a/bcf081da-32a1-42c6-a9c1-ba7068005e7d/p/5621/d/108f8baa-ec02-434b-9227-597966a053b2actuator/Rele1/state";
char server[]            = "mqtt.devicehub.net";
char message_buffer[150];


WiFiClient Client;

PubSubClient client(server, 1883, callback, Client);


void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(server, 1883);
  client.setCallback(callback);

  if (client.connect(clientId)) {
    //add or remove based on the number of actuators
    if (client.subscribe(actuator1Topic)) {
      Serial.println("Ok to subscrib");
    } else {
      Serial.println("Erro no subscrib");
    }

    Serial.println("Successfuly connected and running!");
  } else {
    Serial.println("Connection problem");
  }
}

void loop(void)
{
  //if client it's not connected or disconnects here we try to reconnect
  if (!client.connected()) {
    Serial.println("reconnecting ...");
    client.connect(clientId);
    delay(3 * sec);
    client.subscribe(actuator1Topic);
  }
  // MQTT client loop processing
  client.loop();
  if (client.loop()) {
    //Serial.println(client.state());
  } else {
    Serial.println("NAO Conectado");
  }

}

// handles message arrived on subscribed topic
void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.println("callback");
  // handles extracting the actuator value from payload
  int i = 0;
  int j = 0;
  int k = 0;
  for (i = 0; i < length; i++) {
    message_buffer[i] = payload[i];
  }
  message_buffer[i] = '\0';
  for (i = 0; i < length; i++) {
    if (!(strncmp(&message_buffer[i], "s", 1) || strncmp(&message_buffer[i + 3], "t", 1))) {
      break;
    }
  }
  for (j = i + 9; j < length; j++) {
    if (!(strncmp(&message_buffer[j], "\"", 1))) {
      break;
    }
  }
  String string_actuator_value = "";
  for (k = i + 9; k < j; k++) {
    string_actuator_value += message_buffer[k];
  }
  int actuator_value = string_actuator_value.toInt();

  // here is the value from the actuator
  Serial.print(topic);
  Serial.print(" value: ");
  Serial.println(actuator_value);
  /*
  // you can use it here accordingly just add or remove based on how manny actuators you have
  if(String(topic) == String(actuator1Topic)){
    //DO actuator1 action
  }else if ((String(topic) == String(actuator2Topic))){
    //DO actuator2 action
  }else if ((String(topic) == String(actuator3Topic))){
    //DO actuator3 action
  }
  */
}


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
