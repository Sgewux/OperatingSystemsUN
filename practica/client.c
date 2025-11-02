#include "utils.h"

// ====================================================
// Funciones auxiliares
// ====================================================

void clear_screen() {
#ifdef _WIN32
    int res = system("cls");
#else
    int res = system("clear");
#endif
    (void)res;
}

void pause_execution() {
    printf("\nPresione ENTER para continuar...");
    int c = getchar();
    (void)c;
}

void get_string_input(char *buffer, size_t size, const char *prompt) {
    printf("%s", prompt);
    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
    }
    buffer[strcspn(buffer, "\n")] = '\0';
}

void print_menu() {
    printf("========================================\n");
    printf("        MUSIC SEARCH CLIENT 🎵\n");
    printf("========================================\n");
    printf("1. Ingresar título de la canción\n");
    printf("2. Ingresar artista (opcional)\n");
    printf("3. Realizar búsqueda\n");
    printf("4. Limpiar criterios\n");
    printf("5. Agregar canción\n");
    printf("6. Salir\n");
    printf("Seleccione una opción: ");
}

void print_song(const Song *song, int index) {
    printf("\n[%d] %s - %s\n", index, song->titulo, song->artist);
    printf("   Año: %d | Vistas: %d\n", song->year, song->views);
    printf("   Tag: %s | Idioma: %s\n", song->tag, song->language);
}

// ====================================================
// Comunicación con el servidor (SOCKETS TCP)
// ====================================================

int connect_to_server() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Error creando el socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Dirección IP inválida");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("No se pudo conectar con el servidor");
        close(sock);
        exit(EXIT_FAILURE);
    }

    return sock;
}

void perform_search(SearchCriteria *criteria) {
    if (strlen(criteria->titulo) == 0) {
        printf("\n⚠️  Debe ingresar al menos el título de la canción antes de buscar.\n");
        pause_execution();
        return;
    }

    int sock = connect_to_server();

    RequestMessage req;
    memset(&req, 0, sizeof(req));
    req.action = ACTION_SEARCH;
    req.data.search = *criteria;

    if (send(sock, &req, sizeof(req), 0) == -1) {
        perror("Error enviando datos al servidor");
        close(sock);
        return;
    }

    int found = 0;
    if (recv(sock, &found, sizeof(int), 0) <= 0) {
        perror("Error recibiendo cantidad de resultados");
        close(sock);
        return;
    }

    if (found == 0) {
        printf("\nNo se encontraron canciones con esos criterios.\n");
        close(sock);
        pause_execution();
        return;
    }

    Song *results = malloc(found * sizeof(Song));
    if (!results) {
        perror("Error asignando memoria");
        close(sock);
        return;
    }

    for (int i = 0; i < found; i++) {
        if (recv(sock, &results[i], sizeof(Song), 0) <= 0) {
            perror("Error recibiendo resultados");
            free(results);
            close(sock);
            return;
        }
    }
    close(sock);

    printf("\n========================================\n");
    printf("     %d CANCIONES ENCONTRADAS 🎶\n", found);
    printf("========================================\n");

    for (int i = 0; i < found; i++) {
        print_song(&results[i], i + 1);
    }

    free(results);
    pause_execution();
}

void perform_add_song() {
    Song s;
    memset(&s, 0, sizeof(Song));

    get_string_input(s.titulo, sizeof(s.titulo), "Ingrese el título: ");
    get_string_input(s.artist, sizeof(s.artist), "Ingrese el artista: ");
    get_string_input(s.tag, sizeof(s.tag), "Ingrese el tag: ");
    char temp[32];
    get_string_input(temp, sizeof(temp), "Ingrese el año (número): ");
    s.year = atoi(temp);
    get_string_input(temp, sizeof(temp), "Ingrese las vistas: ");
    s.views = atoi(temp);
    get_string_input(s.language, sizeof(s.language), "Ingrese el idioma (opcional): ");

    int sock = connect_to_server();

    RequestMessage req;
    memset(&req, 0, sizeof(req));
    req.action = ACTION_ADD;
    req.data.song = s;

    if (send(sock, &req, sizeof(req), 0) == -1) {
        perror("Error enviando datos al servidor");
        close(sock);
        return;
    }

    int ack = 0;
    if (recv(sock, &ack, sizeof(int), 0) <= 0) {
        perror("Error recibiendo confirmación");
        close(sock);
        return;
    }

    close(sock);

    if (ack == 1)
        printf("\n✅ Canción agregada correctamente.\n");
    else
        printf("\n❌ No se pudo agregar la canción.\n");

    pause_execution();
}

// ====================================================
// Función principal
// ====================================================

int main() {
    SearchCriteria criteria;
    memset(&criteria, 0, sizeof(SearchCriteria));

    char option[8];
    int running = 1;

    while (running) {
        clear_screen();

        printf("Título actual : %s\n", strlen(criteria.titulo) > 0 ? criteria.titulo : "(ninguno)");
        printf("Artista actual: %s\n\n", strlen(criteria.artist) > 0 ? criteria.artist : "(ninguno)");

        print_menu();

        if (fgets(option, sizeof(option), stdin) == NULL) {
            continue;
        }

        int choice = atoi(option);

        switch (choice) {
            case 1:
                get_string_input(criteria.titulo, sizeof(criteria.titulo),
                                 "Ingrese el título de la canción: ");
                break;
            case 2:
                get_string_input(criteria.artist, sizeof(criteria.artist),
                                 "Ingrese el nombre del artista (opcional): ");
                break;
            case 3:
                perform_search(&criteria);
                break;
            case 4:
                memset(&criteria, 0, sizeof(SearchCriteria));
                printf("\n✅ Criterios limpiados correctamente.\n");
                pause_execution();
                break;
            case 5:
                perform_add_song();
                break;
            case 6:
                printf("\nSaliendo del cliente...\n");
                running = 0;
                break;
            default:
                printf("\nOpción inválida.\n");
                pause_execution();
        }
    }

    return 0;
}
