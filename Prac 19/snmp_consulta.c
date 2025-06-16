#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#define MAX_OID_LEN 128

// Semáforo para sincronizar el acceso a los recursos compartidos
sem_t semaphore;

// Estructura para pasar información a los hilos
typedef struct {
    char *ip;
    char *community;
    char *oid;
} snmp_params;

// Función para realizar una consulta SNMP
void *snmp_get(void *params) {
    snmp_params *snmpData = (snmp_params *)params;
    struct snmp_session session, *ss;
    struct snmp_pdu *pdu;
    struct snmp_pdu *response;
    struct variable_list *vars;
    oid anOID[MAX_OID_LEN];
    size_t anOID_len = MAX_OID_LEN;
    int status;

    // Inicializar la sesión
    snmp_sess_init(&session);
    session.version = SNMP_VERSION_2c;
    session.peername = snmpData->ip;
    session.community = (u_char *)snmpData->community;
    session.community_len = strlen(snmpData->community);

    // Abrir la sesión SNMP
    ss = snmp_open(&session);
    if (!ss) {
        snmp_perror("snmp_open");
        snmp_log(LOG_ERR, "¡OCURRIÓ UN ERROR AL ABRIR LA SESIÓN!\n");
        pthread_exit(NULL);
    }

    // Crear PDU para GET
    pdu = snmp_pdu_create(SNMP_MSG_GET);
    if (!snmp_parse_oid(snmpData->oid, anOID, &anOID_len)) {
        snmp_perror(snmpData->oid);
        snmp_close(ss);
        pthread_exit(NULL);
    }
    snmp_add_null_var(pdu, anOID, anOID_len);

    // Enviar la consulta SNMP y obtener la respuesta
    status = snmp_synch_response(ss, pdu, &response);

    // Bloquear el semáforo para acceso exclusivo
    sem_wait(&semaphore);

    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
        for (vars = response->variables; vars; vars = vars->next_variable) {
            char buf[1024];
            snprint_variable(buf, sizeof(buf), vars->name, vars->name_length, vars);
            printf("Respuesta de %s: %s\n", snmpData->ip, buf);
        }
    } else {
        if (status == STAT_SUCCESS)
            fprintf(stderr, "Error en la consulta SNMP: %s\n", snmp_errstring(response->errstat));
        else
            snmp_sess_perror("snmp_get", ss);
    }

    // Desbloquear el semáforo
    sem_post(&semaphore);

    // Liberar la memoria
    if (response)
        snmp_free_pdu(response);
    snmp_close(ss);

    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <IP1> <IP2> ... <community> <OID>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int num_ips = argc - 3;
    char *community = argv[argc - 2];
    char *oid = argv[argc - 1];

    pthread_t threads[num_ips];
    snmp_params params[num_ips];

    // Inicializar el semáforo con valor 1 (solo un hilo puede acceder a la vez)
    sem_init(&semaphore, 0, 1);

    // Crear hilos para cada dispositivo IP
    for (int i = 0; i < num_ips; i++) {
        params[i].ip = argv[i + 1];
        params[i].community = community;
        params[i].oid = oid;
        pthread_create(&threads[i], NULL, snmp_get, &params[i]);
    }

    // Esperar a que terminen todos los hilos
    for (int i = 0; i < num_ips; i++) {
        pthread_join(threads[i], NULL);
    }

    // Destruir el semáforo
    sem_destroy(&semaphore);

    return 0;
}


