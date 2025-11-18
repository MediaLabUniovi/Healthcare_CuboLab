# 📡 Endpoint de Gestión de WiFi - `/wifi_conf`

## 🎯 Descripción

Se ha añadido un nuevo endpoint `/wifi_conf` que proporciona una interfaz completa para gestionar las redes WiFi guardadas en el CuboLab, **sin interferir** con el resto de endpoints y funcionalidades existentes.

---

## 🔗 Endpoints Añadidos

### 1. `GET /wifi_conf` - Página de Gestión
Muestra una interfaz web completa para gestionar las redes WiFi.

**Funcionalidades:**
- ✅ Ver todas las redes WiFi guardadas (slots 1-5)
- ✅ Ver qué slot ocupa cada red
- ✅ Añadir nuevas redes WiFi (desde redes escaneadas o manual)
- ✅ Eliminar redes existentes
- ✅ Interfaz moderna y responsive

### 2. `POST /wifi_add` - Añadir Red WiFi
Añade una nueva red WiFi al sistema.

**Parámetros:**
- `ssid` (string) - Nombre de la red WiFi
- `password` (string) - Contraseña de la red

**Respuestas:**
- ✅ `200` - Red añadida con éxito
- ❌ `400` - SSID vacío, ya existe, o no hay slots disponibles
- ❌ `500` - Error al guardar en memoria

### 3. `POST /wifi_delete` - Eliminar Red WiFi
Elimina una red WiFi del sistema.

**Parámetros:**
- `slot` (int) - Número del slot a eliminar (1-5)

**Respuestas:**
- ✅ `200` - Red eliminada con éxito
- ❌ `400` - Slot inválido o ya vacío

---

## 🚀 Cómo Usar

### Acceso a la Interfaz

1. **Conectar el CuboLab a la carga** (activa modo configuración)

2. **Conectarse al WiFi del Cubo:**
   - Red: `Cubo XX:XX:XX:XX:XX:XX`
   - Contraseña: `cubolab2024`

3. **Abrir el navegador y acceder a:**
   ```
   http://192.168.4.1/wifi_conf
   ```

### Ver Redes Guardadas

La página muestra automáticamente todas las redes WiFi guardadas con:
- 📶 Nombre de la red (SSID)
- 🔢 Número de slot que ocupa (1-5)
- 🗑️ Botón para eliminar

### Añadir una Nueva Red

Tienes **dos opciones**:

**Opción 1: Desde redes escaneadas**
1. Seleccionar la red del desplegable (se muestran las redes WiFi cercanas)
2. Introducir la contraseña
3. Pulsar "✅ Añadir Red"

**Opción 2: SSID manual**
1. Dejar el desplegable en "-- Seleccionar red escaneada --"
2. Escribir el nombre de la red manualmente en el campo de texto
3. Introducir la contraseña
4. Pulsar "✅ Añadir Red"

### Eliminar una Red

1. Buscar la red en la lista
2. Pulsar el botón "🗑️ Eliminar"
3. Confirmar la acción
4. La red se elimina y se libera el slot

---

## 🔒 Características de Seguridad

### Validaciones Implementadas:

✅ **No duplicados:** No permite añadir una red que ya existe
✅ **Máximo 5 redes:** Control de slots (1-5)
✅ **Campos obligatorios:** SSID y password requeridos
✅ **Confirmación de eliminación:** Previene borrados accidentales
✅ **Slots automáticos:** Asigna automáticamente el primer slot libre
✅ **Detección de slots vacíos:** No permite eliminar slots ya vacíos

---

## 📋 Gestión de Slots

El sistema maneja **5 slots** para redes WiFi:

| Slot | Clave en Preferences | Estado |
|------|---------------------|---------|
| 1    | `ssid1` / `pass1`   | Libre/Ocupado |
| 2    | `ssid2` / `pass2`   | Libre/Ocupado |
| 3    | `ssid3` / `pass3`   | Libre/Ocupado |
| 4    | `ssid4` / `pass4`   | Libre/Ocupado |
| 5    | `ssid5` / `pass5`   | Libre/Ocupado |

### Lógica de Asignación:

1. Al **añadir** una red:
   - Busca el primer slot libre (1→5)
   - Si todos están ocupados, muestra error
   - Si la red ya existe, muestra en qué slot está

2. Al **eliminar** una red:
   - Libera el slot específico
   - El slot queda disponible para nuevas redes

---

## 🎨 Interfaz de Usuario

### Características de la UI:

- 🎨 **Diseño moderno** con colores diferenciados
- 📱 **Responsive** - funciona en móviles y tablets
- ⚡ **AJAX** - sin recargas de página innecesarias
- ✅ **Mensajes de feedback** - confirmación de acciones
- 🔄 **Auto-refresh** - recarga tras añadir/eliminar

### Elementos visuales:

- 🟢 **Verde** - Botón añadir y mensajes de éxito
- 🔴 **Rojo** - Botón eliminar y mensajes de error
- 🔵 **Azul** - Botón volver
- 🟡 **Gris claro** - Items de lista

---

## 🔄 Compatibilidad con Endpoints Existentes

### ✅ NO INTERFIERE con:

- ✅ `/` - Endpoint raíz (sigue funcionando igual)
- ✅ `/wifi` - Endpoint original de añadir WiFi
- ✅ `/calibration` - Endpoint de calibración
- ✅ Cualquier otro endpoint futuro

### 🔗 Convivencia:

El nuevo endpoint es **completamente independiente**:
- Usa sus propias rutas (`/wifi_conf`, `/wifi_add`, `/wifi_delete`)
- Usa las mismas Preferences (compatible con el sistema existente)
- No modifica la lógica de otros endpoints
- Se puede usar en paralelo con los endpoints originales

---

## 📂 Archivos Modificados

### `src/serverLab.cpp`
```cpp
// Añadidas 3 nuevas funciones:
void handleWifiConfig()       // Sirve la página HTML de gestión
void handleWifiConfigAdd()    // Añade red WiFi
void handleWifiConfigDelete() // Elimina red WiFi

// Añadidas 3 nuevas rutas en createServer():
server.on("/wifi_conf", HTTP_GET, handleWifiConfig);
server.on("/wifi_add", HTTP_POST, handleWifiConfigAdd);
server.on("/wifi_delete", HTTP_POST, handleWifiConfigDelete);
```

### `src/serverLab.h`
```cpp
// Añadidas declaraciones:
void handleWifiConfig();
void handleWifiConfigAdd();
void handleWifiConfigDelete();
```

---

## 🧪 Ejemplos de Uso

### Ejemplo 1: Añadir Red Manualmente via HTTP

```bash
# Añadir red WiFi
curl -X POST http://192.168.4.1/wifi_add \
  -d "ssid=MiWiFi&password=MiPassword123"

# Respuesta:
# ✅ Red 'MiWiFi' añadida con éxito en slot 1
```

### Ejemplo 2: Eliminar Red via HTTP

```bash
# Eliminar red del slot 2
curl -X POST http://192.168.4.1/wifi_delete \
  -d "slot=2"

# Respuesta:
# ✅ Red 'MiWiFi' eliminada con éxito del slot 2
```

### Ejemplo 3: Ver Configuración en Monitor Serial

```
WiFi Config - Añadiendo red:
SSID: MiWiFi
Pass: MiPassword123
Red guardada en slot 1
```

---

## 🐛 Solución de Problemas

### La página no carga
- ✅ Verifica que estás en modo configuración (conectado a carga)
- ✅ Verifica la URL: `http://192.168.4.1/wifi_conf`
- ✅ Verifica que estás conectado al WiFi del Cubo

### No puedo añadir más redes
- ✅ Verifica que no tienes ya 5 redes guardadas
- ✅ Elimina alguna red para liberar slots
- ✅ Verifica que la red no existe ya

### La red no se conecta después de añadirla
- ✅ Verifica que la contraseña es correcta
- ✅ La conexión se intentará en el siguiente ciclo del cubo
- ✅ Verifica el monitor serial para ver mensajes de conexión

---

## 📊 Ventajas de esta Implementación

✅ **Sin conflictos** - No toca el código existente
✅ **Escalable** - Fácil de mantener y extender
✅ **Seguro** - Validaciones en frontend y backend
✅ **User-friendly** - Interfaz intuitiva y moderna
✅ **Compatible** - Usa el mismo sistema de Preferences
✅ **Documentado** - Código comentado y documentación completa

---

## 🔮 Futuras Mejoras Posibles

- 🔄 Reordenar prioridad de redes
- 📊 Mostrar intensidad de señal de redes escaneadas
- 🔐 Mostrar/ocultar contraseñas guardadas
- ✏️ Editar contraseña sin eliminar red
- 📈 Estadísticas de conexión por red
- 🔔 Notificaciones de conexión exitosa/fallida

---

**¡Listo para usar! 🚀**
