# Audio del cubo

El firmware selecciona archivos por número mediante `playMp3Folder()`.
Preparar una carpeta `/mp3` en la tarjeta SD con archivos MP3 reales:
convertir los WAV del ZIP a MP3; cambiar únicamente la extensión no los convierte.

| Archivo en SD | Audio del ZIP / evento |
| --- | --- |
| `/mp3/0001.mp3` | CARA 0 |
| `/mp3/0002.mp3` | CARA 1 |
| `/mp3/0003.mp3` | CARA 2 |
| `/mp3/0004.mp3` | CARA 3 |
| `/mp3/0005.mp3` | CARA 4 |
| `/mp3/0006.mp3` | CARA 5 |
| `/mp3/0007.mp3` | BAT BAJA (reservado, sin llamada actual) |
| `/mp3/0008.mp3` | ESTOY AQUI (reservado, sin llamada actual) |
| `/mp3/0009.mp3` | BUENOS DIAS (reservado, sin llamada actual) |
| `/mp3/0010.mp3` | PETICIÓN DE AYUDA CANCELADA (reservado, sin llamada actual) |
| `/mp3/0011.mp3` | JUAN TRANQUILO (reservado, sin llamada actual) |
| `/mp3/0012.mp3` | Error WiFi: crear un audio específico; no está en el ZIP |

Las constantes `TRACK_SIDE_*` en `src/configuration.h` definen la correspondencia
entre caras y archivos. El número 0 desactiva un evento sin audio asignado.
Si aún no se dispone de la locución de error WiFi, establecer
`TRACK_EVENT_WIFI_ERROR` a 0 hasta preparar `0012.mp3`.

Se reproduce una sola locución a la vez. Las peticiones recibidas mientras hay
una reproducción pendiente se omiten; no se encolan ni reinician el plazo.
El fin de reproducción, un error del módulo o la retirada de la tarjeta apagan
el DFPlayer. Si no llega un aviso, el apagado de respaldo ocurre a los 60 segundos.
En batería se permite esperar ese mismo plazo antes de entrar en deep sleep.
Los audios deben durar menos que ese límite.

Validar en hardware las seis caras, un fallo WiFi y la entrada en deep sleep.
Comprobar que el aviso suena completo, una sola vez, y que la alimentación del
DFPlayer se corta después. La correspondencia física de caras depende de su
calibración y debe comprobarse en el cubo.
