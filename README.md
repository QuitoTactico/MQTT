# Introducción

El objetivo de este proyecto es realizar la implementación del protocolo MQTT (Message Queuing Telemetry Transport) el cual es esencial en el Internet de las Cosas (IoT) que facilita la comunicación eficiente entre dispositivos conectados, este, operando mediante un modelo de publicación/suscripción, permitiendo a los clientes publicar mensajes en temas específicos o suscribirse a temas de interés. 

### Especificaciones:

- Lenguajes usados: C
- Librerías usadas: MQTT solo trabaja con la librería sockets y las propias del lenguaje para la manipulación de los datos correspondientes, por lo que se listan a continuación:
    - string.h : manejo array de caracteres
    - stdlib.h: manejo de memoria
    - stdio.h : manejo de entradas y salidas por consola
    - stdint.h : manejo de variables de tamaño estático (integers)
    - threads.h : manejo de hilos para manejar concurrencia de clientes
    - sys/socket.h : manejo de los sockets para permitir la conexión entre el cliente y el broker
    - netdb.h : para obtener la información de la dirección y obtener el nombre de la información del usuario (ip y puerto)
    - arpa/inet.h: por medio de los métodos ntoh y hton se cambia la estructura de los bytes para leerlos correctamente de la red a la arquitectura del cliente, y viceversa
    - unistd.h: Trae la definición de la función close para liberar sockets. Esta es opcional, cuando no es incluída, se generan algunas advertencias (warnings) por parte del compilador, sin embargo, se tiene la incertidumbre de que no se sabe que pasa con la liberación del socket
    - time.h: Declara las funciones de fecha y hora para llevar el contador sobre el cual se mandan los respectivos paquetes y los mismos se reciben, para que si no llega alguno, se cierre la comunicación
    
    **Archivos empleados:**
    
    Vamos a manejar 2 carpetas, una para el broker (server), y otra para el cliente, cada una con su archivo.h que almacenará todos los headers, funciones, variables, estructuras, y demás ítems que serán necesarios para la implementación de parte del usuario que use nuestra api, el archivo.c es en el que se meterán la implementación de estos métodos y los otros métodos que sean necesarios para su funcionalidad y un archivo principal que ejecuta nuestra api.
    
    **Forma de Compilarse:**
    
    gcc broker.c mqttBroker.c dbManager.c -o broker → Desde la carpeta MQTT/broker/
    
    gcc client.c mqttClient.c -o cliente → Desde la carpeta MQTT/client/
    
    **Comandos de Ejecución:**
    
    ./client </path/log.log> → Desde la carpeta MQTT/client/
    ./client <ip> <port> </path/log.log> -> Forma alternativa
    
    ./broker </path/log.log> → Desde la carpeta MQTT/broker/
    ./broker <ip> <port> </path/log.log> -> Forma alternativa
    
    **Notas adicionales:**
    
    Para identificar las sesiones, se utilizan las credenciales de ID, Nombre de usuario y contraseña en el broker, ya que este es privado, por lo que las conexiones se validan a través de estos 3 campos
    

# Desarrollo

Para el proceso de desarrollo, nos interesó en primer lugar empezar a revisar la documentación sugerida para la creación del protocolo, la cual es adjuntada en la sección posterior, con el objetivo de informarnos bien acerca de todas las interacciones, los mensajes, los headers de cada tipo de paquete, y como el broker lo recibe para su posterior procesamiento.

Luego de ver la documentación, se procedió a realizar los primeros archivos centrados en el broker, por lo que se definieron los fixed header y variable header de cada tipo de mensaje, así como sus flags, las estructuras correspondientes a cada tipo de paquete y la creación del socket, a la vez de diversos métodos para manejar la conexión y luego los mensajes recibidos (handles), sus respectivos payload.

Así pues, se procedió a realizar las primeras versiones del broker de tal manera que como primera prueba, recibiera mensajes desde telnet hacia el servidor. y así validar que la conexión se mantuviera estable.

Luego de ello, se procedió a crear el cliente con cada uno de los 4 tipos de paquetes y sus respectivas validaciones implícitas (connect, publish, subscribe y exit) para lo cual el usuario pudiera poner sus credenciales a la vez de cada dato que el cliente le pidiera para ser validado posteriormente, y así establecer la conexión, crear mensajes que serán almacenados en tópicos y la capacidad de suscribirse a ciertos tópicos.

Respecto al planteamiento lógico, se tiene en cuenta entonces diferentes diagramas como los mostrados posteriormente:

## Diagrama de secuencia:

![Diagrama secuencia MQTT](https://github.com/QuitoTactico/MQTT/assets/78938901/99a4000b-c652-46a7-bef5-04671ce31422)

## Diagrama de despliegue:

![Diagrama de despliegue](https://github.com/QuitoTactico/MQTT/assets/78938901/ca88ecdd-9075-4006-b542-775a8a67bcc8)

## Diagrama de componentes:

![Diagrama de componentes MQTT](https://github.com/QuitoTactico/MQTT/assets/78938901/3097d221-8f27-47c3-bc85-19d05cc54678)

# Aspectos logrados y no logrados

- Se pudieron completar los paquetes:
    - Connect
    - Publish
    - Subscribe
- Con sus Acknowledge:
    - Connack
    - Puback
    - Suback
- Se pudo implementar la “base de datos” para almacenar las subscripciones y publicaciones de los respectivos clientes, para que a su vez, estas se muestren entre los tópicos relacionados, a la vez de sus respectivos comandos para almacenar o no, las publicaciones.
- Se pudo completar el registro .log que guardará los cambios que se hagan de las sesiones, usuarios y publicaciones.

## No logrados:

- No se pudo completar el proceso de retransmisión del paquete Publish con el qos nivel 2.
    
    Respecto a paquetes:
    
- Publish:
    - Publish Receive en ambos  lados
    - Publish release en ambos lados
    - Publish complete en ambos lados
- Subscribe:
    - Petición de unsubscribe
    - Ack de unsubscribe
- Ping:
    - Petición de ping
    - Respuesta de ping
- Disconnect cuando el cliente se está desconectando
- Authentication exchange
- Respecto a verificar los suscritos a un tópico, no se implementaron los comodines “+” y “#”, por lo que solo se les notificará a los usuarios que se suscribieron al tópico que se haya inscrito, no a sus respectivas subdivisiones

# Referencias:

https://www.youtube.com/watch?v=WmKAWOVnwjE

https://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.pdf

https://www.geeksforgeeks.org/tcp-3-way-handshake-process/
