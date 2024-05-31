Cosas a definir:
- Tipo de señales a usar
- IPC adicional
- Tipo de socket a usar: Voy a empezar usando uno (por ejemplo udp) y cuando funcione, le agrego la posibilidad de usar el otro (tcp o udp). También voy a empezar usando ipv4 y cuando funcione le agrego ipv6. Me parece que va a ser mejor empezar implementando ipv4

- Pasar toda la lógica del possible_infection a la librería

M2: Todos los módulos funcionando

M4:

- Encapsular código de communication_with_client en funciones más chicas

- Agregar documentación a las funciones

- Corregir los magic numbers y las cosas hardcodeadas.

- Eliminar cosas comentadas que no se van a usar

- Eliminar impresiones de waiting for username and password, sending json, ubuntu client identified 

- Tests

- Revisar en el enunciado si no falta nada


Si queda tiempo:
- Que el hostname sea el hostname de la terminal que se está usando

- Que se pueda actualiar de a una cosa en el supplies_update

- Que se elija random la entrada en la que se mide temperatura

- Pasar cosas del servidor al módulo possible_infection
