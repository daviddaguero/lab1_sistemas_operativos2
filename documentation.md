El proyecto consta de los siguientes archivos:

server.c: the central server. 
It implements a refuge state summary in JSON format, providing population with key information on alerts, supplies, and emergency notifications.
Sirve para que la people pueda consultar dicho summary. Permite que los clientes se conecten a través de un login simple. Los clientes deben proveer su nombre de usuario y contraseña.
La comunicación usando sockets se realiza en un principio a través de TCP/IP e ipv4. En futuras versiones se implementará UDP e ipv6.
Cuando arranca el server, crea un socket y lo configura para operar a través de TCP/IP. Luego hace 3 forks, uno correspondiente a cada módulo. Los hijos generados a partir de los forks se encargan de ejecutar las funciones de los 3 módulos.
The main process of the server se encarga de recibir las notificaciones provenientes de los módulos possible_infection and emergency.
Ante una notificación del módulo emergency.c the server will close all the connections and send a notification to all connected clients. Estas notificaciones son implementadas a través de señales.
The server will authenticate the clients y solo si se trata del cliente "ubuntu", le va a permitir realizar las actualizaciones al módulo supplies_data.
The rest of the clients solo pueden consultar la información del current state of the refuge.
También se crean hijos para mantener las comunicaciones con cada cliente (un hijo por cada cliente que se quiera comunicar).
The server will keep a log file with all the events that happens in the refuge. This file will be in the /var/log/ directory, and the name will be refuge.log.

client.c: it can be instantiated n times. It simulates the people looking to recieve the summary of the current state of the refuge. Each client have its unique user name and password. Hay un cliente especial que tiene un nombre de usuario y contraseña "ubuntu". Este cliente solo se encarga de actualizar la información de los supplies available y no hace consultas al servidor. The user name and password are provided as arguments via the command line.

possible_infection.c: this module implements instant alerts for possible infected refuged. Para simular mediciones de temperatura de la gente, se usan valores random entre 36 y 40 grados. Si el valor random de temperatura generado es mayor o igual a 38 grados, se considera que la persona puede estar infectada y se envía una alerta. Estas alertas son implementadas a través de señales. It is implemented as a static library. 
Las temperaturas simuladas son registradas in a log file named "infections.log". Estos logs son guardados en un directorio llamado "infection_logs".

supplies_data.c: este módulo se encarga de mantener información de los supplies available y de actualizarla cuando esto es solicitado por el cliente "ubuntu". El control de si el cliente que solicita la actualización es ubuntu, es delegado al servidor. It is implemented as a static library.

emergency.c: Develop instant emergency notifications. Para simular los power outages and other critical situations se usa una notificación enviada cada un tiempo random. En un principio se envian dos tipos de notificaciones: "power outages" and "other critical situations". La notificación que se envía es escogida de manera aleatoria. Las notificaciones enviadas son implementadas a través de señales. This module is implemented as a dynamic library.

