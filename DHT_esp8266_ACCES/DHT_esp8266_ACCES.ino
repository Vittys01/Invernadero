#include <ESP8266HTTPClient.h>

#include <LiquidCrystal.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFiMulti.h>

const char *ssid = "88210000";
const char *password = "IFHJE2hI";
ESP8266WiFiMulti wifiMulti;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int sensorTemperaturaPin = 0; // A0
int sensorHumedadPin = 1;     // A1
int ventiladorPin = 7;
int bombaPin = 8;

const int HUMIDITY_LIMIT = 55;
const int TEMPERATURE_LIMIT = 30;

void setup()
{
  Serial.begin(115200);
  lcd.begin(16, 2);
  pinMode(ventiladorPin, OUTPUT);
  pinMode(sensorTemperaturaPin, INPUT);
  pinMode(bombaPin, OUTPUT);
  pinMode(sensorHumedadPin, INPUT);

  wifiMulti.addAP(ssid, password);

  Serial.print("Conectando a WiFi");
  while (wifiMulti.run() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConexión WiFi exitosa");
  Serial.print("IP Local: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  LecturaTemp_Humed();
  EnvioDatos();
  delay(60000); // Wait for 60 seconds
}

void LecturaTemp_Humed()
{
  int sensorTemperaturaValue = analogRead(sensorTemperaturaPin);
  float temperatura = (sensorTemperaturaValue * 5000.0) / 1023 / 10;

  int sensorHumedadValue = analogRead(sensorHumedadPin);
  int humedad = map(sensorHumedadValue, 0, 1023, 100, 0);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperatura, 1);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Humedad: ");
  lcd.print(humedad);
  lcd.print(" %");

  digitalWrite(ventiladorPin, (temperatura > TEMPERATURE_LIMIT) ? HIGH : LOW);
  digitalWrite(bombaPin, (humedad < HUMIDITY_LIMIT) ? HIGH : LOW);
}

void EnvioDatos()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    WiFiClient client; // Crear una instancia de WiFiClient

    String dataToSend = "temperatura=" + String(TEMPERATURE_LIMIT) + "&humedad=" + String(HUMIDITY_LIMIT);

    http.begin(client, "http://cursointegrador.infinityfreeapp.com/EspPost.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpCode = http.POST(dataToSend);

    if (httpCode > 0)
    {
      Serial.println("Código HTTP: " + String(httpCode));
      if (httpCode == 200)
      {
        String response = http.getString();
        Serial.println("Respuesta del servidor:");
        Serial.println(response);
      }
    }
    else
    {
      Serial.print("Error al enviar POST, código: ");
      Serial.println(httpCode);
    }

    http.end();
  }
  else
  {
    Serial.println("Error en la conexión WiFi");
  }
}
