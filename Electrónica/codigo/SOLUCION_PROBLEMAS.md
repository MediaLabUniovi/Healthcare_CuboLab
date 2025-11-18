# Solución de Problemas - CuboLab

## Problemas Corregidos

### 1. **Páginas Web No Cargaban (192.168.4.1 no mostraba nada)**

#### Problema:
Las rutas `/wifi` y `/calibration` no servían las páginas HTML, solo manejaban peticiones POST.

#### Solución:
- Se agregaron nuevos handlers para servir las páginas correctamente:
  - `handleRoot()` - Sirve la página principal desde `/index.html` en LittleFS
  - `handleCalibrationPage()` - Sirve la página de calibración dinámica
  - Las rutas ahora distinguen entre GET (mostrar página) y POST (procesar datos)

### 2. **Configuración WiFi No Funcionaba**

#### Problema:
- La variable `macStr` se creaba como local en lugar de usar la global
- El servidor no estaba configurado correctamente para recibir los datos del formulario

#### Solución:
- Se corrigió el uso de la variable global `macStr`
- Se cambió la ruta del formulario de `/save` a POST en lugar de `/wifi`
- Se agregó manejo correcto de los datos del formulario HTML

### 3. **Memoria No Volátil Corrupta**

#### Problema:
Datos antiguos o corruptos en la memoria no volátil (Preferences) pueden causar problemas.

#### Solución:
- Se agregó la función `clearAllPreferences()` para limpiar toda la memoria
- Se agregó un botón "Limpiar Memoria" en la interfaz web
- Se muestra confirmación antes de borrar para evitar pérdidas accidentales

## Cómo Usar el Sistema

### Modo Configuración (cuando se conecta a carga)

1. **Conectar el dispositivo a carga**
   - Se activa automáticamente el modo configuración durante 5 minutos
   - El LED parpadea rojo-verde indicando modo configuración

2. **Conectarse al WiFi del Cubo**
   - Buscar red WiFi: `Cubo XX:XX:XX:XX:XX:XX` (donde XX es la MAC)
   - Contraseña: `cubolab2024`

3. **Acceder a la interfaz web**
   - Abrir navegador y ir a: `http://192.168.4.1`
   - Deberías ver la página principal con las opciones

### Configurar WiFi

1. En la página principal:
   - Seleccionar una red WiFi del desplegable
   - Introducir la contraseña
   - Pulsar "Guardar WiFi"
   
2. El sistema puede guardar hasta 5 redes WiFi diferentes

### Calibrar el Cubo

1. Pulsar el botón "Calibrar Cubo"
2. Para cada lado (0 a 5):
   - Colocar el cubo en la posición del lado correspondiente
   - Dejar el cubo quieto en una superficie plana
   - Pulsar el botón "Calibrar Lado X"
   - Esperar confirmación
3. Una vez calibrados todos los lados, volver al inicio

### Limpiar Memoria (Si hay problemas)

**⚠️ ATENCIÓN: Esto borra TODA la configuración**

1. En la página principal, pulsar "Limpiar Memoria"
2. Confirmar la acción
3. El dispositivo se reiniciará con la memoria limpia
4. Necesitarás volver a configurar WiFi y calibración

### Salir del Modo Configuración

1. Pulsar "Salir del Modo Configuración"
2. El dispositivo se reiniciará en modo normal

## Rutas del Servidor Web

- `GET /` - Página principal con lista de WiFis y configuración
- `GET /wifi` - Misma que la raíz
- `POST /save` - Guardar credenciales WiFi
- `GET /calibration` - Página de calibración
- `POST /calibrate` - Procesar calibración de un lado
- `POST /exit` - Salir del modo configuración
- `POST /clear` - Limpiar toda la memoria no volátil

## Solución de Problemas Comunes

### La página no carga
1. Verificar que estás conectado al WiFi del Cubo
2. Verificar que la dirección es exactamente `192.168.4.1`
3. Probar con diferentes navegadores
4. Limpiar la caché del navegador

### No puedo conectarme al WiFi del Cubo
1. Verificar que el dispositivo está en carga
2. Verificar que no han pasado más de 5 minutos desde que se conectó
3. Verificar la contraseña: `cubolab2024`

### El cubo no se conecta a mi WiFi
1. Verificar que la contraseña es correcta
2. Verificar que el WiFi está en rango
3. Intentar limpiar la memoria y volver a configurar
4. Verificar en el monitor serial si hay mensajes de error

### La calibración no funciona
1. Asegurarse de que el cubo está completamente quieto
2. Colocarlo en una superficie plana y estable
3. Si persiste, limpiar la memoria y volver a calibrar

## Archivos Importantes

- `src/serverLab.cpp` - Lógica del servidor web
- `src/serverLab.h` - Declaraciones del servidor
- `src/cuboFunctions.cpp` - Funciones auxiliares (incluyendo limpieza de memoria)
- `data/index.html` - Página web principal
- `src/main.cpp` - Bucle principal del programa

## Notas Técnicas

- El sistema usa **LittleFS** para almacenar archivos HTML
- Las preferencias se guardan en el namespace **"MediaLab"**
- El modo configuración dura **5 minutos (300000 ms)** por defecto
- Se pueden guardar hasta **5 redes WiFi**
- La calibración guarda **6 lados** (0-5) con valores x, y, z para cada uno

## ⚠️ IMPORTANTE: Subir Archivos a LittleFS

**ANTES de cargar el código, debes subir los archivos del directorio `data/` al sistema de archivos del ESP32:**

### Opción 1: Usando PlatformIO (Recomendado)
1. En VS Code, abre la paleta de comandos (Ctrl+Shift+P o Cmd+Shift+P)
2. Busca: `PlatformIO: Upload Filesystem Image`
3. Selecciona tu dispositivo
4. Espera a que termine la carga

### Opción 2: Usando Terminal
```bash
pio run --target uploadfs
```

### Verificar que los archivos se subieron
El archivo `data/index.html` debe estar en el ESP32 para que la página web funcione.

**Si no subes los archivos, verás "Archivo no encontrado" al acceder a 192.168.4.1**

