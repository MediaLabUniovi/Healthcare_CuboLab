#include "serverLab.h"

boolean wifiUpdate = false;  // Define and initialize it here
WebServer server;  // Define it here without extern
String macStr;     // Define it here without extern

int calibrationSide = 0;


void createServer(){
    byte mac[6];
    WiFi.macAddress(mac);
    
   // Construcción manual de la dirección MAC con ceros iniciales
    String macStr = "";
    for (int i = 0; i < 6; i++) {
        if (mac[i] < 0x10) {
            macStr += "0"; // Agregar un cero inicial si es menor a 0x10
        }
        macStr += String(mac[i], HEX); // Convertir el byte a HEX
        if (i < 5) {
            macStr += ":"; // Agregar los dos puntos entre los bytes
        }
    }
    macStr.toUpperCase(); // Convertir a mayúsculas

    
    String ssid = "Cubo " + macStr;
    String password = "cubolab2024";
    WiFi.softAP(ssid.c_str(), password.c_str());


    Serial.println("Punto de acceso iniciado: " + ssid);

    // Iniciar el sistema de archivos LittleFS
    if (!LittleFS.begin()) {
        Serial.println("Error al montar LittleFS");
        return;
    }

    // Iniciar el servidor HTTP
    server.on("/wifi", handleAddWifi);
    server.on("/calibration", handleCalibrate);
    server.on("/", handleAddWifi);
    server.begin();
    Serial.println("Servidor HTTP iniciado");

    
    scanNetworks();
}


void handleAddWifi()
{
  // Intentar capturar el ssid y la password desde la solicitud
  String ssid = server.arg("ssid");
  String password = server.arg("password");

  Serial.println("Wifi Añadida");
  Serial.println(ssid);
  Serial.println(password);

  // Verificar si los parámetros ssid y password no están vacíos
  if (ssid.length() == 0 || password.length() == 0) {
    // Si alguno de los parámetros está vacío, enviar un error 500
    server.send(500, "text/plain", "Error: ssid or password missing.");
    return; // Salir de la función
  }

  // Recuperar el valor actual del contador, por defecto 1
  int counter = preferences.getInt("counter", 1);
  Serial.println(counter);

  // Si el contador llega a 5, reiniciar a 1
  if (counter >= 5) {
    counter = 1;
  } else {
    counter++; // Incrementar el contador
  }

  // Crear las claves para el ssid y el password basadas en el contador
  String ssidKey = "ssid" + String(counter);
  String passKey = "pass" + String(counter);
  

  // Guardar el ssid y el password utilizando las claves correspondientes
  bool ssidSaved = preferences.putString(ssidKey.c_str(), ssid);
  bool passSaved = preferences.putString(passKey.c_str(), password);

  // Verificar si ambos valores se guardaron correctamente
  if (!ssidSaved || !passSaved) {
    // Si no se pudieron guardar, enviar un error 500
    server.send(500, "text/plain", "Error saving ssid or password.");
    return; // Salir de la función
  }

  // Convertir los SSID y contraseñas a const char*
  const char* ssidChar = ssid.c_str();
  const char* passwordChar = password.c_str();

  // Agregar la red WiFi con su contraseña
  wifiMulti.addAP(ssidChar, passwordChar);

  // Guardar el nuevo valor del contador
  preferences.putInt("counter", counter);

  // Enviar un código de éxito 200 OK
  server.send(200, "text/plain", "WiFi credentials saved successfully.");
}


// Función para manejar el botón "Continuar" que realiza la calibración de cada lado
void handleCalibrate() {
  // Verificar si el número del lado fue enviado por POST
  if (server.hasArg("side")) {
    // Obtener el valor del lado desde los argumentos POST
    String sideStr = server.arg("side");
    int calibrationSide = sideStr.toInt(); // Convertir a entero

    // Verificar que el número del lado esté en el rango esperado (0 a 5)
    if (calibrationSide >= 0 && calibrationSide <= 5) {
      // Guardar 10 lecturas del acelerómetro para calcular la media
      float ax_readings[10], ay_readings[10], az_readings[10];
      for (int i = 0; i < 10; i++) {
        sensors_event_t accel, gyro, temp;
        mpu.getEvent(&accel, &gyro, &temp);

        ax_readings[i] = accel.acceleration.x;
        ay_readings[i] = accel.acceleration.y;
        az_readings[i] = accel.acceleration.z;

        delay(100); // Esperar un poco entre lecturas
      }

      // Calcular la media de las lecturas
      float mean_ax = mean(ax_readings, 10);
      float mean_ay = mean(ay_readings, 10);
      float mean_az = mean(az_readings, 10);

      // Guardar en Preferences con nombres únicos por lado
      String s_x = "s" + String(calibrationSide) + "_x";
      String s_y = "s" + String(calibrationSide) + "_y";
      String s_z = "s" + String(calibrationSide) + "_z";

      preferences.putFloat(s_x.c_str(), mean_ax);
      preferences.putFloat(s_y.c_str(), mean_ay);
      preferences.putFloat(s_z.c_str(), mean_az);

      // Responder con un mensaje de éxito
      server.send(200, "text/plain", "Calibración para el lado " + String(calibrationSide) + " completada.");

      Serial.print("Lado ");
      Serial.print(calibrationSide);
      Serial.println(" calibrado");
    } else {
      // Si el valor del lado no está en el rango esperado, devolver un error 400
      server.send(400, "text/plain", "Error: lado fuera de rango (debe estar entre 0 y 5).");
    }
  } else {
    // Si no se ha enviado el argumento "side", devolver un error 400
    server.send(400, "text/plain", "Error: no se recibió el número del lado.");
  }
}

