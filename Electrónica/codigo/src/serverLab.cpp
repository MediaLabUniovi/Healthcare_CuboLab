#include "serverLab.h"

extern String networks;  // Variable definida en cuboFunctions.cpp

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
    server.on("/wifi_conf", HTTP_GET, handleWifiConfig);  // Nueva ruta para gestión de WiFi
    server.on("/wifi_add", HTTP_POST, handleWifiConfigAdd);  // Añadir red desde wifi_conf
    server.on("/wifi_delete", HTTP_POST, handleWifiConfigDelete);  // Eliminar red desde wifi_conf
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


// ============================================================================
// GESTIÓN DE REDES WIFI - Endpoint /wifi_conf
// ============================================================================

// Función para mostrar la página de configuración de WiFi
void handleWifiConfig() {
  String html = "<!DOCTYPE html><html lang='es'><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Gestión WiFi - CuboLab</title>";
  html += "<style>";
  html += "body{font-family:Arial,sans-serif;margin:0;padding:20px;background:#f2f2f2;}";
  html += ".container{max-width:800px;margin:0 auto;background:#fff;padding:20px;border-radius:8px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
  html += "h1{color:#333;text-align:center;margin-bottom:30px;}";
  html += "h2{color:#4caf50;border-bottom:2px solid #4caf50;padding-bottom:10px;margin-top:30px;}";
  html += ".wifi-list{list-style:none;padding:0;}";
  html += ".wifi-item{background:#f9f9f9;padding:15px;margin:10px 0;border-radius:5px;display:flex;justify-content:space-between;align-items:center;}";
  html += ".wifi-info{flex-grow:1;}";
  html += ".wifi-ssid{font-weight:bold;font-size:16px;color:#333;}";
  html += ".wifi-slot{color:#666;font-size:14px;}";
  html += ".btn{padding:8px 15px;border:none;border-radius:5px;cursor:pointer;font-size:14px;transition:background 0.3s;}";
  html += ".btn-delete{background:#f44336;color:#fff;}";
  html += ".btn-delete:hover{background:#d32f2f;}";
  html += ".btn-add{background:#4caf50;color:#fff;padding:10px 20px;font-size:16px;width:100%;}";
  html += ".btn-add:hover{background:#45a049;}";
  html += ".btn-back{background:#2196F3;color:#fff;padding:10px 20px;font-size:16px;width:100%;margin-top:20px;}";
  html += ".btn-back:hover{background:#1976D2;}";
  html += ".form-group{margin:15px 0;}";
  html += "label{display:block;margin-bottom:5px;color:#333;font-weight:bold;}";
  html += "input,select{width:100%;padding:10px;border:1px solid #ddd;border-radius:5px;box-sizing:border-box;font-size:14px;}";
  html += ".empty-message{text-align:center;padding:20px;color:#666;font-style:italic;}";
  html += ".message{padding:10px;margin:10px 0;border-radius:5px;text-align:center;}";
  html += ".success{background:#d4edda;color:#155724;border:1px solid #c3e6cb;}";
  html += ".error{background:#f8d7da;color:#721c24;border:1px solid #f5c6cb;}";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>🔧 Gestión de Redes WiFi</h1>";
  html += "<p style='text-align:center;color:#666;'>MAC: <strong>" + macStr + "</strong></p>";
  
  // Listar redes WiFi guardadas
  html += "<h2>📡 Redes WiFi Guardadas</h2>";
  html += "<ul class='wifi-list' id='wifiList'>";
  
  bool hasNetworks = false;
  for (int i = 1; i <= 5; i++) {
    String ssidKey = "ssid" + String(i);
    String ssid = preferences.getString(ssidKey.c_str(), "");
    
    if (ssid != "") {
      hasNetworks = true;
      html += "<li class='wifi-item'>";
      html += "<div class='wifi-info'>";
      html += "<div class='wifi-ssid'>📶 " + ssid + "</div>";
      html += "<div class='wifi-slot'>Slot " + String(i) + "</div>";
      html += "</div>";
      html += "<button class='btn btn-delete' onclick='deleteWifi(" + String(i) + ", \"" + ssid + "\")'>🗑️ Eliminar</button>";
      html += "</li>";
    }
  }
  
  if (!hasNetworks) {
    html += "<div class='empty-message'>No hay redes WiFi configuradas</div>";
  }
  
  html += "</ul>";
  
  // Formulario para añadir nueva red
  html += "<h2>➕ Añadir Nueva Red WiFi</h2>";
  html += "<div id='message'></div>";
  html += "<form id='addWifiForm'>";
  html += "<div class='form-group'>";
  html += "<label>Red WiFi Disponible:</label>";
  html += "<select name='ssid' id='ssidSelect'>";
  html += "<option value=''>-- Seleccionar red escaneada --</option>";
  html += networks;  // Redes escaneadas
  html += "</select>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label>O escribir SSID manualmente:</label>";
  html += "<input type='text' name='ssid_manual' id='ssidManual' placeholder='Nombre de la red WiFi'>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label>Contraseña WiFi:</label>";
  html += "<input type='password' name='password' id='password' placeholder='Contraseña' required>";
  html += "</div>";
  html += "<button type='button' class='btn btn-add' onclick='addWifi()'>✅ Añadir Red</button>";
  html += "</form>";
  
  // Botón para volver
  html += "<button class='btn btn-back' onclick='window.location=\"/\"'>⬅️ Volver al Inicio</button>";
  
  html += "</div>";
  
  // JavaScript para manejar acciones
  html += "<script>";
  html += "function addWifi(){";
  html += "  const ssidSelect = document.getElementById('ssidSelect').value;";
  html += "  const ssidManual = document.getElementById('ssidManual').value;";
  html += "  const password = document.getElementById('password').value;";
  html += "  const ssid = ssidManual || ssidSelect;";
  html += "  if(!ssid || !password){";
  html += "    showMessage('Por favor completa todos los campos', 'error');";
  html += "    return;";
  html += "  }";
  html += "  fetch('/wifi_add', {";
  html += "    method:'POST',";
  html += "    headers:{'Content-Type':'application/x-www-form-urlencoded'},";
  html += "    body:'ssid='+encodeURIComponent(ssid)+'&password='+encodeURIComponent(password)";
  html += "  })";
  html += "  .then(r=>r.text())";
  html += "  .then(d=>{";
  html += "    showMessage(d, d.includes('éxito') ? 'success' : 'error');";
  html += "    if(d.includes('éxito')) setTimeout(()=>location.reload(), 1500);";
  html += "  })";
  html += "  .catch(e=>showMessage('Error: '+e, 'error'));";
  html += "}";
  html += "function deleteWifi(slot, ssid){";
  html += "  if(!confirm('¿Eliminar la red \"'+ssid+'\"?')) return;";
  html += "  fetch('/wifi_delete', {";
  html += "    method:'POST',";
  html += "    headers:{'Content-Type':'application/x-www-form-urlencoded'},";
  html += "    body:'slot='+slot";
  html += "  })";
  html += "  .then(r=>r.text())";
  html += "  .then(d=>{";
  html += "    showMessage(d, d.includes('éxito') ? 'success' : 'error');";
  html += "    if(d.includes('éxito')) setTimeout(()=>location.reload(), 1500);";
  html += "  })";
  html += "  .catch(e=>showMessage('Error: '+e, 'error'));";
  html += "}";
  html += "function showMessage(msg, type){";
  html += "  const div = document.getElementById('message');";
  html += "  div.innerHTML = '<div class=\"message '+type+'\">'+msg+'</div>';";
  html += "  setTimeout(()=>div.innerHTML='', 5000);";
  html += "}";
  html += "</script>";
  
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// Función para añadir una red WiFi desde el endpoint wifi_conf
void handleWifiConfigAdd() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");

  Serial.println("WiFi Config - Añadiendo red:");
  Serial.println("SSID: " + ssid);
  Serial.println("Pass: " + password);

  if (ssid.length() == 0 || password.length() == 0) {
    server.send(400, "text/plain", "❌ Error: SSID o contraseña vacíos");
    return;
  }

  // Buscar primer slot disponible
  int freeSlot = -1;
  for (int i = 1; i <= 5; i++) {
    String ssidKey = "ssid" + String(i);
    String existingSSID = preferences.getString(ssidKey.c_str(), "");
    
    // Verificar si ya existe esta red
    if (existingSSID == ssid) {
      server.send(400, "text/plain", "❌ Esta red ya está guardada en el slot " + String(i));
      return;
    }
    
    // Encontrar primer slot libre
    if (existingSSID == "" && freeSlot == -1) {
      freeSlot = i;
    }
  }

  if (freeSlot == -1) {
    server.send(400, "text/plain", "❌ No hay slots disponibles. Elimina una red primero (máximo 5 redes)");
    return;
  }

  // Guardar en el slot libre
  String ssidKey = "ssid" + String(freeSlot);
  String passKey = "pass" + String(freeSlot);

  bool ssidSaved = preferences.putString(ssidKey.c_str(), ssid);
  bool passSaved = preferences.putString(passKey.c_str(), password);

  if (!ssidSaved || !passSaved) {
    server.send(500, "text/plain", "❌ Error al guardar en memoria");
    return;
  }

  // Agregar al WiFiMulti
  wifiMulti.addAP(ssid.c_str(), password.c_str());

  Serial.println("Red guardada en slot " + String(freeSlot));
  server.send(200, "text/plain", "✅ Red '" + ssid + "' añadida con éxito en slot " + String(freeSlot));
}

// Función para eliminar una red WiFi desde el endpoint wifi_conf
void handleWifiConfigDelete() {
  if (!server.hasArg("slot")) {
    server.send(400, "text/plain", "❌ Error: Falta el parámetro slot");
    return;
  }

  int slot = server.arg("slot").toInt();
  
  if (slot < 1 || slot > 5) {
    server.send(400, "text/plain", "❌ Error: Slot inválido (debe ser 1-5)");
    return;
  }

  String ssidKey = "ssid" + String(slot);
  String passKey = "pass" + String(slot);
  
  String ssid = preferences.getString(ssidKey.c_str(), "");
  
  if (ssid == "") {
    server.send(400, "text/plain", "❌ El slot " + String(slot) + " ya está vacío");
    return;
  }

  // Eliminar de preferences
  preferences.remove(ssidKey.c_str());
  preferences.remove(passKey.c_str());

  Serial.println("Red eliminada del slot " + String(slot) + ": " + ssid);
  server.send(200, "text/plain", "✅ Red '" + ssid + "' eliminada con éxito del slot " + String(slot));
}

