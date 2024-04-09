#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct DatabaseNode {
    char string1[100];
    char string2[100];
    struct DatabaseNode* next;
} DatabaseNode;


DatabaseNode* readStringsFromFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        return NULL;
    }

    DatabaseNode* head = NULL;
    DatabaseNode* tail = NULL;

    char line[200];
    while (fgets(line, sizeof(line), file) != NULL) {
        DatabaseNode* newNode = malloc(sizeof(DatabaseNode));
        if (newNode == NULL) {
            perror("malloc");
            break;
        }

        char* token = strtok(line, "|");
        strncpy(newNode->string1, token, sizeof(newNode->string1) - 1);
        token = strtok(NULL, "\n");
        strncpy(newNode->string2, token, sizeof(newNode->string2) - 1);

        newNode->next = NULL;

        if (head == NULL) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
    }

    fclose(file);

    return head;
}

//1 si se guardó correctamente, 0 si no se pudo abrir el archivo
int saveStringsToFile(const char* filename, const char* string1, const char* string2) {
    FILE* file = fopen(filename, "a");
    if (file == NULL) {
        perror("fopen");
        return 0;
    }

    fprintf(file, "%s|%s\n", string1, string2);

    fclose(file);
    return 1;
}

// retorna 1 si la sesión es válida, 0 si no existe el archivo o no se pudo abrir, -1 si el username y la contaseña no coinciden
int verifySession(const char* username, const char* password) {
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

        // Divide la línea en nombre de usuario y contraseña
        char* line_username = strtok(line, "|");
        char* line_password = strtok(NULL, "|");

        // Comprueba si el nombre de usuario y la contraseña coinciden
        if (line_username != NULL && line_password != NULL &&
            strcmp(line_username, username) == 0 && strcmp(line_password, password) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return -1;
}

// para verificar si un nombre de usuario o un tópico existe en su respectivo archivo
int checkExistence(const char* filename, const char* string1) {
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
int updateOrCreate(const char* filename, const char* string1, const char* string2) {
    //abrimos el archivo CSV en modo de lectura
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("el archivo no existe");
        return 0;
    }

    //creamos un archivo temporal para guardar las líneas actualizadas
    FILE* temp = fopen("dbSessionsTemp.csv", "w");
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
    rename("dbSessionsTemp.csv", filename);

    return userExists; //retornará 1 si ya existía y -1 si no
}

void getSubscribes(char* username, char*** topics, int* topicsCount) {
    FILE* file = fopen("dbSubscribes.csv", "r");
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Elimina el salto de línea al final

        char* token;
        token = strtok(line, "|");
        if (strcmp(token, username) == 0) {
            token = strtok(NULL, "|");
            (*topicsCount)++;
            *topics = realloc(*topics, (*topicsCount) * sizeof(char*));
            (*topics)[*topicsCount - 1] = strdup(token);
        }
    }
    fclose(file);
}

// not really used. This was caused by a misunderstanding.
void getSubscriptors(char* topic, char*** users, int* usersCount) {
    FILE* file = fopen("dbSubscribes.csv", "r");
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Elimina el salto de línea al final

        char* token;
        token = strtok(line, "|");
        char* username = strdup(token);
        token = strtok(NULL, "|");
        if (strcmp(token, topic) == 0 || strncmp(token, topic, strlen(topic)) == 0 && token[strlen(topic)] == '/') {
            (*usersCount)++;
            *users = realloc(*users, (*usersCount) * sizeof(char*));
            (*users)[*usersCount - 1] = username;
        } else {
            free(username);
        }
    }
    fclose(file);
}


int isUserInList(char* username, char** users, int usersCount) {
    for (int i = 0; i < usersCount; i++) {
        if (strcmp(users[i], username) == 0) {
            return 1;
        }
    }
    return 0;
}

void getNotified(char* topic, char*** users, int* usersCount) {
    FILE* file = fopen("dbSubscribes.csv", "r");
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
            if (strcmp(token, superTopic) == 0 && !isUserInList(username, *users, *usersCount)) {
                (*usersCount)++;
                *users = realloc(*users, (*usersCount) * sizeof(char*));
                (*users)[*usersCount - 1] = username;
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
}

// pura demostración
int main() {

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
        printf("7. Indagar notificados al publicar en un tópico\n");
        printf("Introduce una opción: ");
        int option;
        scanf("%d", &option);
        getchar();  // Elimina el salto de línea después de la opción

        int result;

        switch (option) {
            case 1:
                result = saveStringsToFile(filename, string1, string2);
                if (result == 0) {
                    printf("Error al guardar los strings\n");
                } else {
                    printf("Strings guardados\n");
                }
                break;

            case 2:
                result = verifySession(string1, string2);
                if (result == 1) {
                    printf("Sesión verificada\n");
                } else {
                    printf("Sesión no verificada\n");
                    printf("Resultado: %d\n", result);
                }
                break;

            case 3:
                result = checkExistence(filename, string1);
                if (result == 1) {
                    printf("Existe\n");
                } else {
                    printf("No existe\n");
                    printf("Resultado: %d\n", result);
                }
                break;

            case 4:
                result = updateOrCreate(filename, string1, string2);
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
                int topicsCount = 0;
                getSubscribes(string1, &topics, &topicsCount);
                printf("Subscribes de %s: (%d)\n", string1, topicsCount);
                for (int i = 0; i < topicsCount; i++) {
                    printf("%s\n", topics[i]);
                    free(topics[i]);
                }
                free(topics);
                break;

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

            case 7:
                char** users2 = NULL;
                int usersCount2 = 0;
                getNotified(string1, &users2, &usersCount2);
                printf("Suscriptores de %s: (%d)\n", string1, usersCount2);
                for (int i = 0; i < usersCount2; i++) {
                    printf("%s\n", users2[i]);
                    free(users2[i]);
                }
                free(users2);
                break;

            default:
                printf("Opción no válida\n");
                break;
        }
        printf("---------------\n\n");
    }

    return 0;
}
