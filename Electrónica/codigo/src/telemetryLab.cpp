#include "telemetryLab.h"

bool sendTelemetryForSide(HTTPClient& http, bool modoDemo, int side, int battery) {
  if (!connectWiFi()) {
    return false;
  }

  if (modoDemo) {
    String url = "http://85.31.236.104:3047/send-emotion";

    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    String postData = "{\"emotion\":\"" + String(side) + "\"}";
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("POST realizado con éxito, código de respuesta: " + String(httpResponseCode));
      Serial.println("Respuesta: " + response);
    } else {
      Serial.println("Error en la solicitud POST, código de respuesta: " + String(httpResponseCode));
      http.end();
      return false;
    }

    http.end();
    return true;
  }

  String macAddress = WiFi.macAddress();
  String url = "https://www.unioviedo.es/medialab/datos_cube.php";
  url += "?e=" + String(side) + "&m=%27" + macAddress + "%27&b=" + String(battery);

  Serial.println(url);
  bool success = false;
  while (!success) {
    http.setTimeout(30000);
    http.begin(url);
    http.addHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Safari/537.36");

    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
      Serial.println("GET realizado con éxito, código de respuesta: " + String(httpResponseCode));
      success = true;
    } else {
      Serial.println("Error en la solicitud GET, código de respuesta: " + String(httpResponseCode));
    }

    http.end();
  }

  // Mantener la comprobación horaria de OTA en este módulo para que main no crezca.
  struct tm timeinfo;
  configTime(0, 0, "hora.roa.es");

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  }

  Serial.println("Hora configurada de hora.roa.es");

  String timezone = "CET-1CEST,M3.5.0/1,M10.5.0";
  setenv("TZ", timezone.c_str(), 1);
  tzset();

  int currentHour = timeinfo.tm_hour;
  int currentMinute = timeinfo.tm_min;
  Serial.printf("Hora actual: %02d:%02d\n", currentHour, currentMinute);

  if (currentHour >= 2 && currentHour < 7) {
    Serial.println("La hora está entre las 00:00 y las 10:00. Iniciando actualización OTA...");
    // updateFirmware();
  } else {
    Serial.println("No es la hora adecuada para la actualización.");
  }

  return true;
}
