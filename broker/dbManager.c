#include "mqttBroker.h"

// ---------------------------------- GENERAL -----------------------------------------

//1 si se guardó correctamente, 0 si no se pudo abrir el archivo
int DBsaveStringsToFile(const char* filename, const char* string1, const char* string2) {
    FILE* file = fopen(filename, "a");
    if (file == NULL) {
        perror("fopen");
        return 0;
    }

    fprintf(file, "%s|%s\n", string1, string2);

    fclose(file);
    return 1;
}

// para verificar si un nombre de usuario o un tópico existe en su respectivo archivo
int DBcheckExistence(const char* filename, const char* string1) {
    // Abre el archivo CSV
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("el archivo no existe");
        return 0;
    }

    // Lee cada línea del archivo
    char line[200];
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\n")] = 0;  // Elimina el salto de línea al final

        //toma el primer campo
        char* line_field1 = strtok(line, "|");

        //comprueba si el string coincide
        if (line_field1 != NULL && strcmp(line_field1, string1) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return -1;
}

/* Para actualizar o crear un registro en un archivo. si el primer campo es igual a string1, se le actualiza su segundo campo con string2  
Retorna un int señalando si ya existía o no. 1 si ya existía y se actualizó, -1 si no existía y se creó, 0 si el archivo no existe o no se pudo abrir.
*/
int DBupdateOrCreate(const char* filename, const char* string1, const char* string2) {
    //abrimos el archivo CSV en modo de lectura
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("el archivo no existe");
        return 0;
    }

    //creamos un archivo temporal para guardar las líneas actualizadas
    FILE* temp = fopen("temp.csv", "w");
    if (temp == NULL) {
        perror("no se pudo crear el archivo temporal");
        fclose(file);
        return 0;
    }

    //leemos cada línea
    char line[200];
    int userExists = -1;
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\n")] = 0;  // Elimina el salto de línea al final

        //divide la línea entre el primer y segundo campo
        char* line_field1 = strtok(line, "|");
        char* line_field2 = strtok(NULL, "|");

        //comprueba si el nombre de usuario coincide
        if (line_field1 != NULL && strcmp(line_field1, string1) == 0) {
            userExists = 1;
            fprintf(temp, "%s|%s\n", string1, string2);  // Escribe la línea actualizada en el archivo temporal
        } else {
            fprintf(temp, "%s|%s\n", line_field1, line_field2);  // Escribe la línea original en el archivo temporal
        }
    }

    //si no existe, agrega una nueva línea
    if (userExists == -1) {
        fprintf(temp, "%s|%s\n", string1, string2);
    }

    //cierra los archivos y renombra el archivo temporal
    fclose(file);
    fclose(temp);
    rename("temp.csv", filename);

    return userExists; //retornará 1 si ya existía y -1 si no
}

// ---------------------------------- SESSIONS -----------------------------------------


// retorna 1 si la sesión es válida, 0 si no existe el archivo o no se pudo abrir, -1 si el username y la contraseña no coinciden
int DBverifySession(const char* identifier, const char* username, const char* password) {
    // Abre el archivo CSV
    const char* filename = "dbSessions.csv";
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("el archivo no existe");
        return 0;
    }

    // Lee cada línea del archivo
    char line[200];
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\n")] = 0;  // Elimina el salto de línea al final

        // Divide la línea en identificador, nombre de usuario y contraseña
        char* line_identifier = strtok(line, "|");
        char* line_username = strtok(NULL, "|");
        char* line_password = strtok(NULL, "|");

        // Comprueba si el identificador, nombre de usuario y contraseña coinciden
        if (line_identifier != NULL && line_username != NULL && line_password != NULL &&
            strcmp(line_identifier, identifier) == 0 && strcmp(line_username, username) == 0 && strcmp(line_password, password) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return -1;
}

int DBupdateOrCreateSession(const char* id, const char* username, const char* password) {
    const char* filename = "dbSessions.csv";
    //abrimos el archivo CSV en modo de lectura
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("el archivo no existe");
        return 0;
    }

    //creamos un archivo temporal para guardar las líneas actualizadas
    FILE* temp = fopen("temp.csv", "w");
    if (temp == NULL) {
        perror("no se pudo crear el archivo temporal");
        fclose(file);
        return 0;
    }

    //leemos cada línea
    char line[200];
    int userExists = -1;
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\n")] = 0;  // Elimina el salto de línea al final

        //divide la línea entre los campos
        char* line_field1 = strtok(line, "|");
        char* line_field2 = strtok(NULL, "|");
        char* line_field3 = strtok(NULL, "|");

        //comprueba si el nombre de usuario coincide
        if (line_field1 != NULL && strcmp(line_field1, id) == 0) {
            userExists = 1;
            fprintf(temp, "%s|%s|%s\n", id, username, password);  // Escribe la línea actualizada en el archivo temporal
        } else {
            fprintf(temp, "%s|%s|%s\n", line_field1, line_field2, line_field3);  // Escribe la línea original en el archivo temporal
        }
    }

    //si no existe, agrega una nueva línea
    if (userExists == -1) {
        fprintf(temp, "%s|%s|%s\n", id, username, password);
    }

    //cierra los archivos y renombra el archivo temporal
    fclose(file);
    fclose(temp);
    rename("temp.csv", filename);

    return userExists; //retornará 1 si ya existía y -1 si no
}

// ---------------------------------- SUBSCRIPTIONS -----------------------------------------

// envíale el username y un array vacío con los tópicos. Modificará el arreglo y te retornará cuántos consiguió.
int DBgetSubscribes(char* username, char*** topics) {
    FILE* file = fopen("dbSubscribes.csv", "r");
    int topicsCount = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Elimina el salto de línea al final

        char* token;
        token = strtok(line, "|");
        if (strcmp(token, username) == 0) {
            token = strtok(NULL, "|");
            (topicsCount)++;
            *topics = realloc(*topics, (topicsCount) * sizeof(char*));
            (*topics)[topicsCount - 1] = strdup(token);
        }
    }
    fclose(file);
    return topicsCount;
}

int DBisUserInList(char* username, char** users, int usersCount) {
    for (int i = 0; i < usersCount; i++) {
        if (strcmp(users[i], username) == 0) {
            return 1;
        }
    }
    return 0;
}

int DBgetSubscriptors(char* topic, char*** users) {
    FILE* file = fopen("dbSubscribes.csv", "r");
    int usersCount = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Elimina el salto de línea al final

        char* token;
        token = strtok(line, "|");
        char* username = strdup(token);
        token = strtok(NULL, "|");

        char* superTopic = strdup(topic);
        char* slashPosition;
        do {
            if (strcmp(token, superTopic) == 0 && !DBisUserInList(username, *users, usersCount)) {
                (usersCount)++;
                *users = realloc(*users, (usersCount) * sizeof(char*));
                (*users)[usersCount - 1] = username;
                break;
            }
            slashPosition = strrchr(superTopic, '/');
            if (slashPosition != NULL) {
                *slashPosition = '\0'; // Truncate the superTopic at the last slash
            }
        } while (slashPosition != NULL);
        free(superTopic);
    }
    fclose(file);
    return usersCount;
}

// ---------------------------------- SOCKETS -----------------------------------------

int DBgetSocketByUsername(const char* username) {
    FILE* file = fopen("dbSockets.csv", "r");
    if (file == NULL) {
        printf("No se pudo abrir el archivo dbSockets.csv\n");
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Elimina el salto de línea al final

        char* token;
        token = strtok(line, "|");
        char* fileUsername = strdup(token);
        token = strtok(NULL, "|");
        if (strcmp(fileUsername, username) == 0) {
            free(fileUsername);
            fclose(file);
            return atoi(token); // Convierte la cadena a int y la retorna
        } else {
            free(fileUsername);
        }
    }
    fclose(file);
    return -1; // Retorna -1 si no se encontró el nombre de usuario
}

// ---------------------------------- LOG  -----------------------------------------

void DBsaveLog(char* dir, char* ip, char* request, char* args) {
    FILE* file = fopen(dir, "a");
    if (file == NULL) {
        printf("No se pudo abrir el archivo log\n");
        return;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(file, "%d-%02d-%02d %02d:%02d:%02d %s %s ",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec,
        ip, request);

    for (int i = 0; i < strlen(args); i++) {
        fprintf(file, "%02X", (unsigned char)args[i]);
    }

    fprintf(file, "\n");
    fclose(file);
}

// ---------------------------------- TESTING -----------------------------------------

// pura demostración
int DBtest() {
    int log;
    printf("TEST\n0: Database\n1: Log\n-> ");
    scanf("%d", &log);
    getchar();

    if(log==1){
        //DBsaveLog("log.txt", 19216801, "GET", "hola");
        char filename[100];
        char args[100];
        char ip[16];
        char request[100];

        printf("Introduce el nombre del archivo: ");
        fgets(filename, sizeof(filename), stdin);
        filename[strcspn(filename, "\n")] = 0;  // Eliminar el salto de línea al final

        printf("Introduce la dirección IP: ");
        scanf("%s", &ip);

        printf("Introduce el tipo de request: ");
        scanf("%s", &request);

        printf("Introduce los argumentos: ");
        scanf(" %[^\n]", args);  // Leer hasta el salto de línea

        DBsaveLog(filename, ip, request, args);

        return 0;
    }
    else{
        for(;;){

            char filename[100];
            char string1[100];
            char string2[100];

            printf("Introduce el nombre del archivo: ");
            fgets(filename, sizeof(filename), stdin);
            filename[strcspn(filename, "\n")] = 0;  // Elimina el salto de línea al final

            printf("Introduce el primer string: ");
            fgets(string1, sizeof(string1), stdin);
            string1[strcspn(string1, "\n")] = 0;  // Elimina el salto de línea al final

            printf("Introduce el segundo string: ");
            fgets(string2, sizeof(string2), stdin);
            string2[strcspn(string2, "\n")] = 0;  // Elimina el salto de línea al final

            printf("1. Guardar strings en el archivo\n");
            printf("2. Verificar sesión\n");
            printf("3. Verificar existencia\n");
            printf("4. Actualizar o crear registro\n");
            printf("5. Indagar subscribes\n");
            printf("6. Indagar suscriptores (not really used)\n");

            printf("7. Poner socket\n");
            printf("8. Extraer socket\n");
            printf("Introduce una opción: ");
            int option;
            scanf("%d", &option);
            getchar();  // Elimina el salto de línea después de la opción

            int result;
            int sockfd;

            switch (option) {
                case 1:
                    result = DBsaveStringsToFile(filename, string1, string2);
                    if (result == 0) {
                        printf("Error al guardar los strings\n");
                    } else {
                        printf("Strings guardados\n");
                    }
                    break;

                case 2:
                    char id[100];
                    printf("Introduce ID: ");
                    fgets(id, sizeof(id), stdin);
                    id[strcspn(id, "\n")] = 0;  // Elimina el salto de línea al final
                    result = DBverifySession(id, string1, string2);
                    if (result == 1) {
                        printf("Sesión verificada\n");
                    } else {
                        printf("Sesión no verificada\n");
                        printf("Resultado: %d\n", result);
                    }
                    break;

                case 3:
                    result = DBcheckExistence(filename, string1);
                    if (result == 1) {
                        printf("Existe\n");
                    } else {
                        printf("No existe\n");
                        printf("Resultado: %d\n", result);
                    }
                    break;

                case 4:
                    result = DBupdateOrCreate(filename, string1, string2);
                    if (result == 1) {
                        printf("Registro actualizado\n");
                    } else if (result == -1) {
                        printf("Registro creado\n");
                    } else {
                        printf("Error en creación de archivo\n");
                    }
                    break;

                case 5:
                    char** topics = NULL;
                    int topicsCount = DBgetSubscribes(string1, &topics);
                    printf("Subscribes de %s: (%d)\n", string1, topicsCount);
                    for (int i = 0; i < topicsCount; i++) {
                        printf("%s\n", topics[i]);
                        free(topics[i]);
                    }
                    free(topics);
                    break;

                /*
                case 6:
                    char** users1 = NULL;
                    int usersCount1 = 0;
                    getSubscriptors(string1, &users1, &usersCount1);
                    printf("Suscriptores de %s: (%d)\n", string1, usersCount1);
                    for (int i = 0; i < usersCount1; i++) {
                        printf("%s\n", users1[i]);
                        free(users1[i]);
                    }
                    free(users1);
                    break;
                */

                case 6:
                    char** users2 = NULL;
                    int usersCount2 = DBgetSubscriptors(string1, &users2);
                    printf("Suscriptores de %s: (%d)\n", string1, usersCount2);
                    for (int i = 0; i < usersCount2; i++) {
                        printf("%s\n", users2[i]);
                        free(users2[i]);
                    }
                    free(users2);
                    break;

                case 7:
                    sockfd = atoi(string2);
                    printf("Socket: %d\n", sockfd);
                    result = DBupdateOrCreate("dbSockets.csv", string1, string2);
                    if (result == 1) {
                        printf("Registro actualizado\n");
                    } else if (result == -1) {
                        printf("Registro creado\n");
                    } else {
                        printf("Error en creación de archivo\n");
                    }
                    break;

                case 8:
                    // getting the socketfd
                    sockfd = DBgetSocketByUsername(string1);
                    if (sockfd == -1) {
                        printf("No se encontró el nombre de usuario\n");
                    } else {
                        printf("Socket: %d\n", sockfd);
                    }
                    break;

                default:
                    printf("Opción no válida\n");
                    break;
            }
            printf("---------------\n\n");
        }
    }
    return 0;
}

// for testing reasons

/*
int main() {
    DBtest();
    return 0;
}
*/