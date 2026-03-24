# Cambios de firmware - Migracion de hardware

Fecha: 2026-03-22
Proyecto: Healthcare_CuboLab

## Objetivo
Actualizar el firmware de una version anterior para adaptarlo al nuevo hardware sin romper la arquitectura existente.

## Resumen ejecutivo
Se ha migrado la gestion de audio de buzzer a DFPlayer con alimentacion conmutada, se han actualizado pines de carga/bateria/IMU, se ha incorporado una UART auxiliar para otro microcontrolador, y se ha hecho configurable la parte dudosa de direccion TX/RX.

Actualizacion adicional: el dispositivo principal ya no usa LEDs locales; la interfaz visual pasa a una base de carga con ESP32-C3 y OLED 0.96 por protocolo UART.

## 1) Sustitucion de buzzer por DFPlayer
- Se elimina el uso de tonos por `tone()`.
- Se implementa reproduccion de pistas MP3 con DFPlayer.
- Se mapean eventos principales a pistas configurables:
  - `TRACK_EVENT_MOVEMENT` = 1
  - `TRACK_EVENT_WIFI_ERROR` = 2

Cambios aplicados en:
- `src/cuboFunctions.cpp`
- `src/cuboFunctions.h`
- `src/configuration.h`

## 2) Control de alimentacion DFPlayer (TPS61023)
- Se anade pin de habilitacion `DFPLAYER_ENABLE_PIN = GPIO17`.
- Secuencia robusta:
  - Encender regulador
  - Esperar `DFPLAYER_POWER_STABILIZE_MS` (configurable)
  - Inicializar DFPlayer con reintentos (`DFPLAYER_INIT_RETRIES`)
- Se apaga DFPlayer antes de entrar en deep sleep para ahorro energetico.

Cambios aplicados en:
- `src/cuboFunctions.cpp`
- `src/configuration.h`

## 3) UART del DFPlayer
- Comunicacion por UART en pines:
  - RX ESP32: `GPIO27` (desde TX DFPlayer)
  - TX ESP32: `GPIO25` (hacia RX DFPlayer)
- Se usa `HardwareSerial(1)` para el DFPlayer.

Cambios aplicados en:
- `src/cuboFunctions.cpp`
- `src/configuration.h`

## 4) Lectura de bateria y nuevo divisor resistivo
- La bateria se lee por `GPIO34`.
- Se actualiza conversion ADC para divisor 1:2:
  - `Vadc = raw * 3.3 / 4095`
  - `Vbat = Vadc * 2 * BATTERY_CALIBRATION_FACTOR`
- Se mantiene salida en porcentaje para compatibilidad con logica anterior.
- Se parametrizan umbrales en voltaje real:
  - `BATTERY_VOLTAGE_MIN = 5.95V`
  - `BATTERY_VOLTAGE_MAX = 8.2V`

Cambios aplicados en:
- `src/cuboFunctions.cpp`
- `src/configuration.h`

## 5) Deteccion de carga
- La deteccion de carga pasa a `GPIO35`.
- Se centraliza en helper `isChargingActive()`.
- Se reemplaza logica previa basada en umbral ADC por lectura digital configurable:
  - `CHARGE_ACTIVE_LEVEL`
  - `CHARGE_INACTIVE_LEVEL`

Cambios aplicados en:
- `src/main.cpp`
- `src/cuboFunctions.cpp`
- `src/configuration.h`

## 6) Nueva UART para otro microcontrolador
- Se anade UART auxiliar con `HardwareSerial(2)`.
- Pines configurables para resolver duda de direccion:
  - `AUX_UART_TX_PIN = GPIO32`
  - `AUX_UART_RX_PIN = GPIO15`
- Si finalmente se confirma direccion inversa, solo hay que intercambiar macros.
- Se imprime warning en runtime si TX usa pines de arranque sensibles (`GPIO15`, `GPIO5`, `GPIO12`).

Cambios aplicados en:
- `src/main.cpp`
- `src/configuration.h`

## 6.1) Arquitectura principal + base OLED (sin LED local)
- Se elimina la dependencia funcional de LEDs en el firmware del dispositivo principal.
- Se mantiene la logica real de carga/bateria en el dispositivo principal (GPIO35 y GPIO34).
- La base se trata como interfaz visual: recibe estado por UART y lo representa en OLED.
- Se separan estados:
  - `chargingDetected`: estado fisico de carga con debounce.
  - `dockConnected`: estado de enlace UART con la base.
- Se evita asumir que "cargando" y "dock conectado" son lo mismo.

Cambios aplicados en:
- `src/main.cpp`
- `src/cuboFunctions.cpp`
- `src/configuration.h`

## 6.2) Protocolo UART con base
- Se implementa protocolo de texto simple:
  - `HELLO` / `HELLO_ACK`
  - `STATUS`
  - `EVENT`
  - `PING` / `ACK`
- Se anaden timeout y reintentos para handshake.
- Se anade debounce para deteccion de carga.

Parametros configurables en `src/configuration.h`:
- `CHARGE_DEBOUNCE_MS`
- `DOCK_HANDSHAKE_RETRIES`
- `DOCK_HELLO_TIMEOUT_MS`
- `DOCK_PING_INTERVAL_MS`
- `DOCK_ACK_TIMEOUT_MS`

## 7) MPU6050 (I2C e interrupcion)
- Se mantiene MPU6050 por I2C con:
  - SDA `GPIO22`
  - SCL `GPIO21`
- INT actualizado a `GPIO5`.
- Inicializacion I2C explicita: `Wire.begin(MPU_SDA_PIN, MPU_SCL_PIN)`.

Cambios aplicados en:
- `src/main.cpp`
- `src/configuration.h`

## 8) Ajustes por colisiones de pines
- El hardware nuevo reserva `GPIO25` y `GPIO27` para DFPlayer UART.
- Para evitar conflicto, se movieron pines de LEDs definidos en firmware:
  - `led_r = 4`
  - `led_g = 26`
  - `led_b = 2`
- El hold de deep sleep para LED rojo deja de estar hardcodeado y usa el pin configurable.

Cambios aplicados en:
- `src/configuration.h`
- `src/main.cpp`
- `src/cuboFunctions.cpp`

## 9) Dependencias
- Se anade libreria de DFPlayer en PlatformIO:
  - `dfrobot/DFRobotDFPlayerMini@^1.0.6`

Cambios aplicados en:
- `platformio.ini`

## Validacion realizada
- Revision de errores de editor en archivos modificados: sin errores.
- Compilacion no ejecutada en este entorno por ausencia de CLI de PlatformIO (`pio`/`platformio` no disponibles en PATH).

## Parametros configurables clave
Definidos en `src/configuration.h`:
- DFPlayer:
  - `DFPLAYER_ENABLE_PIN`
  - `DFPLAYER_UART_RX_PIN`
  - `DFPLAYER_UART_TX_PIN`
  - `DFPLAYER_POWER_STABILIZE_MS`
  - `DFPLAYER_INIT_RETRIES`
- Carga:
  - `CHARGE_PIN`
  - `CHARGE_ACTIVE_LEVEL`
- Bateria:
  - `BATTERY_ADC_PIN`
  - `BATTERY_DIVIDER_RATIO`
  - `BATTERY_CALIBRATION_FACTOR`
  - `BATTERY_VOLTAGE_MIN`
  - `BATTERY_VOLTAGE_MAX`
- UART auxiliar:
  - `AUX_UART_TX_PIN`
  - `AUX_UART_RX_PIN`
  - `AUX_UART_BAUDRATE`

## Pendientes de validacion en banco
- Confirmar polaridad electrica real de deteccion de carga en `GPIO35`.
- Confirmar direccion definitiva TX/RX de la UART auxiliar entre `GPIO32` y `GPIO15`.
- Ajustar `DFPLAYER_POWER_STABILIZE_MS` segun comportamiento real de arranque del modulo.
- Verificar en hardware que el remapeo de LEDs coincide con PCB final.
- Validar en la base que responde `HELLO_ACK` y `ACK` dentro de tiempos configurados.
- Validar que GPIO15 en la UART auxiliar no altera el arranque por sesgos externos de la base.
