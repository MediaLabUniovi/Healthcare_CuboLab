# CuboLab — cubo de emociones (v2)

CuboLab permite expresar un estado emocional colocando un cubo sobre una de sus seis caras. Esta documentación corresponde al firmware de la rama `v2`: ESP32 con MPU6050, conexión Wi-Fi y avisos mediante DFPlayer o buzzer. Las caras se identifican de `0` a `5`; su significado emocional depende del montaje y de la calibración.

## Contenido

- [Electrónica/codigo](Electrónica/codigo/): proyecto PlatformIO con firmware Arduino para ESP32.
- [AUDIO.md](Electrónica/codigo/AUDIO.md): conexiones, pistas y pruebas de audio.
- [Documentación de componentes](Electrónica/hardware/datasheet/).
- [Data_analysis](Data_analysis/README.md): instalación y uso de las aplicaciones Python de análisis de datos.

## Funcionamiento

1. Inicializa el audio y el MPU6050 en la dirección I²C `0x68`, recupera la calibración desde `Preferences` y estima la batería.
2. Cuando el cubo deja de moverse, compara la aceleración con las seis orientaciones calibradas.
3. Si reconoce una cara distinta de la última guardada, la almacena, enciende el LED verde, emite un aviso e intenta enviar el cambio por Wi-Fi.
4. Entra en suspensión profunda (*deep sleep*), con despertar por movimiento en GPIO14 o señal de carga en GPIO33.

El aviso de cambio de cara se produce antes del envío y no confirma la recepción del servidor. No hay una cola de envíos pendientes. Si se conecta a Wi-Fi pero el servidor no devuelve HTTP `200`, reintenta el envío sin un límite global de intentos.

### Carga e indicadores

Al arrancar con una lectura ADC de carga superior a `1000`, abre una ventana de configuración de hasta cinco minutos, con alternancia de LED rojo y verde. Al terminar el plazo o desconectar la carga, cierra el punto de acceso y pasa al estado de indicación de carga.

En ese estado, el LED verde varía de brillo y queda encendido cuando la estimación alcanza el 100 %. Al desconectar el cargador, muestra cuatro, tres o dos destellos verdes para valores superiores al 75 %, 50 % o 25 %, respectivamente; para valores de hasta el 25 % muestra un destello rojo. Por debajo del 15 % mantiene la indicación roja durante el reposo.

## Conexiones del firmware

| GPIO ESP32 | Función |
| --- | --- |
| 21 | SDA del MPU6050, con I²C por defecto del entorno esp32dev |
| 22 | SCL del MPU6050, con I²C por defecto del entorno esp32dev |
| 14 | Interrupción del MPU6050 y despertar por movimiento |
| 33 | Lectura de carga y despertar por carga |
| 34 | Lectura ADC de batería mediante el circuito de medida |
| 4 | Señal del buzzer |
| 16 | RX de UART2: conectar al TX del DFPlayer |
| 17 | TX de UART2: conectar al RX del DFPlayer |
| 25 | LED rojo |
| 26 | LED verde |
| 27 | LED azul |

Los LED usan lógica invertida: `0` enciende y `255` apaga. Los pines se definen en [configuration.h](Electrónica/codigo/src/configuration.h), salvo GPIO34, utilizado directamente en [cuboFunctions.cpp](Electrónica/codigo/src/cuboFunctions.cpp).

## Alimentación y carga

El montaje utiliza dos baterías 18650 conectadas en serie (configuración **2S**), un BMS **LXLCBT** y un **TPS61023**.

La carga se realiza mediante **pines pogo** o, en su ausencia, mediante el **conector USB tipo C del módulo LXLCBT**.

La documentación de los módulos está disponible en [LXLCBT.pdf](Electrónica/hardware/datasheet/LXLCBT.pdf) y [TPS61023.PDF](Electrónica/hardware/datasheet/TPS61023.PDF). Los esquemas de PCB anteriores no están incluidos en esta versión.

El firmware estima la batería con los límites ADC `minBatt = 2630` y `maxBatt = 3680`. Estos valores deben corresponder al divisor de medida y a la alimentación del montaje; el porcentaje mostrado es una estimación basada en la lectura ADC.

## Compilación y carga

Instala Git y PlatformIO. El proyecto define el entorno `esp32dev`, la plataforma `espressif32` y el framework Arduino. Las dependencias declaradas se encuentran en [platformio.ini](Electrónica/codigo/platformio.ini).

```bash
git clone --branch v2 --single-branch https://github.com/MediaLabUniovi/Healthcare_CuboLab.git
cd Healthcare_CuboLab/Electrónica/codigo
pio run -e esp32dev
pio run -e esp32dev -t upload
pio device monitor -b 115200
```

También puedes abrir `Electrónica/codigo` en PlatformIO para VS Code y usar sus tareas. Si la detección del puerto falla, especifica `--upload-port` al cargar y `--port` al abrir el monitor.

El script `scripts/relative_map.py` mantiene relativa la ruta del mapa del enlazador para admitir rutas de proyecto con acentos en Windows.

## Audio

En cada arranque, incluido el despertar de suspensión, consulta el volumen del DFPlayer por UART2 a `9600` baudios. Si recibe una respuesta válida, configura el volumen a `20` sobre `30` y usa estas pistas de la tarjeta SD:

| Evento | Archivo |
| --- | --- |
| Cambio de cara | `/mp3/0001.mp3` |
| Error de conexión Wi-Fi | `/mp3/0002.mp3` |

Si el módulo no responde, utiliza el buzzer en GPIO4: un pitido de `180 ms` a `880 Hz` para cualquiera de los dos eventos. La respuesta del DFPlayer confirma la comunicación, pero no la presencia del altavoz ni de las pistas. Consulta [AUDIO.md](Electrónica/codigo/AUDIO.md) para las pruebas.

## Configuración Wi-Fi y calibración

Durante la ventana de configuración crea la red `Cubo <MAC>`, con contraseña `cubolab2024`. El servidor registra estas rutas:

| Ruta | Uso |
| --- | --- |
| `GET /configwifi` | Formulario, si existe `/index.html` en LittleFS |
| `POST /configwifi` o `POST /wifi` | Guardar red con los campos `ssid` y `password` |
| `POST /deletewifi` | Eliminar red con el campo `id`, de `1` a `5` |
| `POST /calibration` | Calibrar cara con el campo `side`, de `0` a `5` |

**Limitación actual:** la carpeta `data/` con el HTML y las imágenes se ha eliminado, pero el servidor sigue dependiendo de LittleFS. Si no puede montar el sistema de archivos, no registra las rutas HTTP. Si se monta sin `/index.html`, el formulario devuelve `404`. Cargar solo el firmware no repone estos archivos; una placa con una imagen LittleFS previa puede conservarlos. La ruta `/` está asociada a guardar credenciales y no abre el formulario.

Con LittleFS montado y el servidor activo, puedes usar las rutas POST sin interfaz gráfica desde un equipo conectado al punto de acceso. Sustituye `<IP_DEL_CUBO>` por la dirección del dispositivo:

```bash
curl -X POST "http://<IP_DEL_CUBO>/configwifi" --data-urlencode "ssid=MiRed" --data-urlencode "password=MiClave"
curl -X POST "http://<IP_DEL_CUBO>/calibration" -d "side=0"
```

Para calibrar, coloca el cubo en una orientación estable y envía el número correspondiente. Repite para las seis caras. Cada petición promedia diez lecturas y guarda los tres ejes en `Preferences`, bajo el espacio `MediaLab`. Reinicia después para cargar los valores en las tablas de reconocimiento.

Se guardan hasta cinco redes, sustituyendo posiciones de forma cíclica. `connectWiFi()` también añade una red de laboratorio definida directamente en el código.

## Envío de datos y OTA

Con `modoDemo = false`, valor actual en [main.cpp](Electrónica/codigo/src/main.cpp), realiza un GET a `https://www.unioviedo.es/medialab/datos_cube.php`, con los parámetros `e` (cara), `m` (MAC entre comillas simples codificadas) y `b` (porcentaje estimado de batería).

Con `modoDemo = true`, realiza un POST a `http://85.31.236.104:3047/send-emotion`, con un cuerpo como `{"emotion":"2"}`. Los destinos están definidos en el código; el servicio receptor no forma parte de este firmware y su disponibilidad debe comprobarse por separado.

Existe una implementación OTA en `updateLab.cpp` y una comprobación horaria tras el envío normal, pero la llamada a `updateFirmware()` está comentada. Esta versión no ejecuta actualizaciones OTA automáticamente.
