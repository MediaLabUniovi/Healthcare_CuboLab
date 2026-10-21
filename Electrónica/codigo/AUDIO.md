# Audio del cubo

En cada arranque (también al despertar de deep sleep) se consulta el volumen
del DFPlayer por UART2 a 9600 baudios. Si no llega una respuesta válida en
1500 ms, se usa el buzzer durante ese arranque. La inicialización espera
además 1000 ms para que arranque el módulo.

## Conexiones

- Buzzer: señal en GPIO4 y masa común. Su presencia física no se detecta.
- DFPlayer TX -> ESP32 GPIO16 (RX).
- DFPlayer RX <- ESP32 GPIO17 (TX).
- Alimentación y masa común del DFPlayer según el módulo utilizado.

Los pines están definidos en `src/configuration.h`.
Protocolo: https://image.dfrobot.com/image/data/DFR0299/DFPlayer%20Mini%20Manul.pdf

## Avisos existentes

| Evento | Archivo en la SD del DFPlayer | Alternativa sin respuesta |
| --- | --- | --- |
| Cambio de cara (sendHMI) | /mp3/0001.mp3 | Un pitido de 180 ms a 880 Hz |
| Error de conexión Wi-Fi | /mp3/0002.mp3 | Un pitido de 180 ms a 880 Hz |

El volumen del DFPlayer se configura a 20 sobre 30. Los archivos de audio
deben añadirse a su tarjeta SD. La respuesta del módulo confirma comunicación,
pero no confirma la presencia del altavoz ni que existan las pistas.
La selección se mantiene hasta el siguiente arranque.

## Prueba en el cubo

1. Cargar el firmware y arrancar sin DFPlayer: comprobar el mensaje de buzzer
   en el monitor serie y un pitido por cambio de cara.
2. Provocar un error de conexión Wi-Fi: comprobar un único pitido de 180 ms.
3. Conectar el DFPlayer con las pistas y reiniciar: comprobar el mensaje de
   DFPlayer disponible y la reproducción de los avisos.
4. Comprobar el funcionamiento después de despertar de deep sleep.
