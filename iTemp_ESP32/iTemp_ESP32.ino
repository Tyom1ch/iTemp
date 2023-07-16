#include "libs.h"

const int scanTime = 1; // BLE інтервал

// BLE адресс
std::vector<std::string> knownBLEAddresses = {"a4:c1:38:29:d7:c4"};

float ttemp;
float thumi;
float tbatt;
float tsign;

WebServer server(80);
ATC_MiThermometer miThermometer(knownBLEAddresses);
String rjson = "";
String createJson(float temp, float humidity, int batt_level, int rssi) {
  //Робимо json
  StaticJsonDocument<200> jsonDoc;

  // Заполняем JSON-объект значениями
  jsonDoc["temp"] = String(temp, 2);
  jsonDoc["humidity"] = String(humidity, 2);
  jsonDoc["batt"] = String(batt_level);
  jsonDoc["rssi"] = String(rssi);

  // Преобразуем JSON-объект в строку
  String jsonString;
  serializeJson(jsonDoc, jsonString);

  return jsonString;
}

// Функция, которую будет выполнять второе ядро
void Task1code( void * pvParameters ){
  while (1) {
    // Set sensor data invalid
    miThermometer.resetData();

    // Get sensor data - run BLE scan for <scanTime>
    unsigned found = miThermometer.getData(scanTime);

    for (int i = 0; i < miThermometer.data.size(); i++) {
      if (miThermometer.data[i].valid) {
        ttemp = miThermometer.data[i].temperature / 100.0;
        thumi = miThermometer.data[i].humidity / 100.0;
        tbatt = miThermometer.data[i].batt_level;
        tsign = miThermometer.data[i].rssi;
        rjson = createJson(ttemp, thumi, tbatt, tsign);
        Serial.print(String(rjson));
      }
    }

    miThermometer.clearScanResults();
  }
}

void handleClientTask(void* pvParameters) {
  while (1) {
    server.handleClient();
    vTaskDelay(2);
  }
}

void handleRoot() {
  server.send(200, "application/json",  String(rjson));
}

void setup() {
  Serial.begin(115200);
  // WIFI INIT
  WiFi.mode(WIFI_STA);
  WiFi.begin("OpenWrt", "");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  MDNS.begin("itemp");
  Serial.print("Open http://");
  Serial.println(".local/edit to see the file browser");
  server.on("/", handleRoot);

  // Initialization
  miThermometer.begin();
  server.begin();


  // Создаем задачу для второго ядра
  xTaskCreatePinnedToCore(
      Task1code,     // Функция, которую будет выполнять задача
      "Task1code",   // Имя задачи
      10000,        // Размер стека задачи
      NULL,         // Параметры задачи (не используется в данном случае)
      1,            // Приоритет задачи
      NULL,         // Дескриптор задачи (не используется в данном случае)
      1             // Номер ядра, на котором будет выполняться задача (ядро 1)
  );

  // Создаем задачу для первого ядра
  xTaskCreatePinnedToCore(
      handleClientTask,  // Функция, которую будет выполнять задача
      "handleClientTask",// Имя задачи
      10000,             // Размер стека задачи
      NULL,              // Параметры задачи (не используется в данном случае)
      1,                 // Приоритет задачи
      NULL,              // Дескриптор задачи (не используется в данном случае)
      0                  // Номер ядра, на котором будет выполняться задача (ядро 0)
  );
}

void loop() {
  // Ничего не делаем в loop()
}
