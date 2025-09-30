#include "updateLab.h"

void update_started() {
  Serial.println("CALLBACK:  HTTP update process started");
}

void update_finished() {
  Serial.println("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total) {
  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err) {
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}

void updateFirmware(){
  Serial.println("Update Firmware");
  WiFiClient client;
  
  //httpUpdate.setLedPin(led_r, HIGH);

  httpUpdate.onStart(update_started);
  httpUpdate.onEnd(update_finished);
  httpUpdate.onProgress(update_progress);
  httpUpdate.onError(update_error);

  String macAddress = WiFi.macAddress();

  String url = "https://medialab-iot.com:80/update?mac=" + macAddress;
  Serial.println(url);
  t_httpUpdate_return ret = httpUpdate.update(client, url);
  

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");

      //handleState(UPDATED_OK);  
      ESP.restart();
      break;
  }

}