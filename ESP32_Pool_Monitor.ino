#include <NimBLEDevice.h>
#include <ArduinoJson.h>

// Load Wi-Fi library
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#define ssid "ESP32-Access-Point"
#define SENSOR_COUNT 8
#define ZIELMAC "C0:00:00:XX:XX:XX"

class YC01Device {
  public:
    String hw_version = "";
    String sw_version = "";
    String name = "";
    String identifier = "";
    String address = "";
    float sensors[SENSOR_COUNT]; // Array für die Sensorwerte: [battery, EC, salt, TDS, cloro, pH, ORP, temperature]

    YC01Device() {
      for (int i = 0; i < SENSOR_COUNT; i++) {
        sensors[i] = 0.0;
      }
    }
};

class YC01BluetoothDeviceData {
  public:
    YC01BluetoothDeviceData() {
      debug("In Device Data");
    }

    void debug(String message) {
      Serial.println("[DEBUG]: " + message);
    }

    int* decode(String byte_frame) {
      int size = byte_frame.length() / 2;
      int* frame_array = new int[size];

      for (int i = 0; i < size; i++) {
        frame_array[i] = strtol(byte_frame.substring(i * 2, i * 2 + 2).c_str(), NULL, 16);
      }

      for (int i = size - 1; i > 0; i--) {
        int tmp = frame_array[i];
        int hibit1 = (tmp & 0x55) << 1;
        int lobit1 = (tmp & 0xAA) >> 1;
        tmp = frame_array[i - 1];
        int hibit = (tmp & 0x55) << 1;
        int lobit = (tmp & 0xAA) >> 1;
        frame_array[i] = 0xff - (hibit1 | lobit);
        frame_array[i - 1] = 0xff - (hibit | lobit1);
      }

      return frame_array;
    }

    int decode_position(int* decodedData, int idx) {
      return (decodedData[idx] << 8) | decodedData[idx + 1];
    }

    YC01Device get_status(String data) {
      debug("Getting Status");

      int* decodedData = decode(data);

      int constant = decodedData[1];
      int product_name_code = decodedData[2];
      int hold_reading = decodedData[17] >> 4;

      int backlight_status = (decodedData[17] & 0x0F) >> 3;

      int BATT_0 = 0;  // Passe diese Werte entsprechend an
      int BATT_100 = 100;  // Passe diese Werte entsprechend an

      int battery = round(100.0 * (decode_position(decodedData, 15) - BATT_0) / (BATT_100 - BATT_0));
      battery = min(max(0, battery), 100);

      YC01Device device;
      device.sensors[0] = battery;

      device.sensors[1] = decode_position(decodedData, 5);
      device.sensors[2] = device.sensors[1] * 0.55;

      device.sensors[3] = decode_position(decodedData, 7);

      int cloro = decode_position(decodedData, 11);
      if (cloro < 0) {
        device.sensors[4] = 0;
      } else {
        device.sensors[4] = cloro / 10.0;
      }

      device.sensors[5] = decode_position(decodedData, 3) / 100.0;
      device.sensors[6] = decode_position(decodedData, 9) / 1000.0;
      device.sensors[7] = decode_position(decodedData, 13) / 10.0;

      debug("Got Status");

      return device;
    }
};

String getBLEData() {
  NimBLEClient* client;
  static NimBLEAddress address(std::string(ZIELMAC), BLE_ADDR_PUBLIC); // Adresse des BLE-Geräts
  static NimBLEUUID serviceUUID("0000ff01-0000-1000-8000-00805f9b34fb"); // UUID des Service
  static NimBLEUUID charUUID("0000ff02-0000-1000-8000-00805f9b34fb"); // UUID der Characteristic

  NimBLEDevice::init("");
  client = NimBLEDevice::createClient();
  String data;

  if (client->connect(address)) {
    NimBLERemoteService* remoteService = client->getService(serviceUUID);
    if (remoteService) {
      NimBLERemoteCharacteristic* remoteCharacteristic = remoteService->getCharacteristic(charUUID);
      if (remoteCharacteristic) {
        NimBLEAttValue value = remoteCharacteristic->readValue();
        
        // Konvertieren der empfangenen Daten in Hexadezimalstring
        for (size_t i = 0; i < value.length(); i++) {
          char buffer[3];
          sprintf(buffer, "%02X", (uint8_t)value[i]);
          data += buffer;
        }
        Serial.println("BLE Data:");
        Serial.println(data); // Debug-Ausgabe
      }
    }
    client->disconnect();
    return data;
  } else {
    return "keine Verbindung";
  }
  
}

String WorkSomething(int modus) {
  YC01BluetoothDeviceData deviceData;
  String data = getBLEData();
  YC01Device device = deviceData.get_status(data);

  Serial.println("Gerätedaten:");
  Serial.print("battery: "); Serial.println(device.sensors[0]);
  Serial.print("EC: "); Serial.println(device.sensors[1]);
  Serial.print("salt: "); Serial.println(device.sensors[2]);
  Serial.print("TDS: "); Serial.println(device.sensors[3]);
  Serial.print("cloro: "); Serial.println(device.sensors[4]);
  Serial.print("pH: "); Serial.println(device.sensors[5]);
  Serial.print("ORP: "); Serial.println(device.sensors[6]);
  Serial.print("temperature: "); Serial.println(device.sensors[7]);

  if (modus==0) {
	/*
		pH-Wert:
		    Idealwert: 7,2 - 7,4
		    Grenzwerte: 7,0 - 7,6
		ORP (Redoxpotential):
		    Idealwert: 650 - 750 mV
		    Grenzwerte: 600 - 800 mV
		EC (Leitfähigkeit):
		    Idealwert: 1000 - 2000 µS/cm
		    Grenzwerte: 500 - 3000 µS/cm
		Chlor (freies Chlor):
		    Idealwert: 0,5 - 1,0 mg/L
		    Grenzwerte: 0,3 - 3,0 mg/L
		Total Dissolved Solids (TDS):
		    Idealwert: 1000 - 2000 ppm
		    Grenzwerte: 500 - 3000 ppm
	*/
    String a = "<table>";
    if (device.sensors[4] > 99) {
      a+= "<tr><td>pH</td><td><font color=red>" + String(device.sensors[5]) + "</font></td><td></td><td>(7.0, 7.2..7.4, 7.6)</td></tr>";
      a+= "<tr><td><s>Chlor</s></td><td><s>&gt;99</s></td><td><font color=red>Messung fehlerhaft: pH kontrollieren</font></td><td></td></tr>";
    } else {
      a += "<tr><td>pH</td><td>" + String(device.sensors[5]) + "</td><td></td><td>(7.0, 7.2..7.4, 7.6)</td></tr>";
      a += "<tr><td>Chlor</td><td>" + String(device.sensors[4]) + "</td><td>mg/l (ppm)</td><td>(0.3, 0.5..1.0, 3.0)</td></tr>";
    }
    a += "<tr><td>Temperatur</td><td>" + String(device.sensors[7]) + "</td><td>&deg;C</td><td></td></tr>";
    a += "<tr><td>ORP</td><td>" + String(device.sensors[6]) + "</td><td>V</td><td>(0.6, 0.65..0.75, 0.8)</td></tr>";
    a += "<tr><td>Leitf&auml;higkeit</td><td>" + String(device.sensors[1]) + "</td><td>uS/cm</td><td>(500, 1000..2000, 3000)</td></tr>";
    a += "<tr><td>Solids</td><td>" + String(device.sensors[3]) + "</td><td>ppm (mg/l)</td><td>(500, 1000..2000, 3000)</td></tr>";
    a += "<tr><td>Salz</td><td>" + String(device.sensors[2]) + "</td><td></td></tr>";
    a += "</table><br>Batterie: " + String(device.sensors[0]);
    return a;
  } else if (modus==1) {
    String a = "ph=" + String(device.sensors[5]) + ",cl=";
     if (device.sensors[4] < 99) { 
      a += String(device.sensors[4]); 
      } else { 
        a += "99"; 
      };
     a += ",temp=" + String(device.sensors[7]) + ",orp=" + String(device.sensors[6]) + ",lf=" + String(device.sensors[1]);
     a += ",sol=" + String(device.sensors[3]) + ",sal=" + String(device.sensors[2]) + ",bat=" + String(device.sensors[0]);
     return a;
    } else if (modus==2) {
      JsonDocument doc;
      doc["battery"]=device.sensors[0];
      doc["ec"]=device.sensors[1];
      doc["salt"]=device.sensors[2];
      doc["tds"]=device.sensors[3];
      doc["cl"]=device.sensors[4];
      doc["ph"]=device.sensors[5];
      doc["orp"]=device.sensors[6];
      doc["temp"]=device.sensors[7];
      char output[128];
      serializeJson(doc, output);
      return output;
  } else {}

}

// Set web server port number to 80
WebServer server(80);

// Manage not found URL
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handle_root() {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Pool Control</title>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32 Web Server</h1>\n";
  ptr +="Using Access Point(AP) Mode\n";
  ptr += "<h2> Pool Monitor </h2>";
  ptr += WorkSomething(0);
  ptr +="</body>\n";
  ptr +="</html>\n";
  server.send(200, "text/html", ptr); 
}

void handle_csv() {
  String ptr = WorkSomething(1);
  server.send(200, "text/csv", ptr); 
}

void handle_json() {
  String ptr = WorkSomething(2);
  server.send(200, "application/json", ptr); 
}


void setup() {
  Serial.begin(115200);
  Serial.print("Setting AP (Access Point)…");
  WiFi.softAP(ssid);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  if (MDNS.begin("esp32test")) {
    Serial.println("MDNS responder started");
  }
  server.on("/", handle_root);
  server.on("/csv", handle_csv);
  server.on("/json", handle_json);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
