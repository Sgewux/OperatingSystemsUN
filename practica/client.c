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
    (void)res; // Evita el warning de valor no usado
}

void pause_execution() {
    printf("\nPresione ENTER para continuar...");
    int c = getchar();
    (void)c; // Ignoramos el valor de retorno
}

void get_string_input(char *buffer, size_t size, const char *prompt) {
    printf("%s", prompt);
    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0'; // Si ocurre error, dejamos la cadena vacía
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
    printf("   Tag: %s | Idioma: %s\n",
           song->tag, song->language);
    //printf("   Features: %s\n", song->features);
    //printf("   ID: %d\n", song->id);
}

// ====================================================
// Comunicación con el servidor
// ====================================================

void perform_search(SearchCriteria *criteria) {
    if (strlen(criteria->titulo) == 0) {
        printf("\n⚠️  Debe ingresar al menos el título de la canción antes de buscar.\n");
        pause_execution();
        return;
    }

    RequestMessage req;
    memset(&req, 0, sizeof(req));
    req.action = ACTION_SEARCH;
    req.data.search = *criteria;

    int wfd = open(FIFO_C2S, O_WRONLY);
    if (wfd == -1) {
        perror("Error abriendo FIFO_C2S");
        return;
    }

    if (write(wfd, &req, sizeof(RequestMessage)) == -1) {
        perror("Error enviando datos al servidor");
        close(wfd);
        return;
    }
    close(wfd);

    int rfd = open(FIFO_S2C, O_RDONLY);
    if (rfd == -1) {
        perror("Error abriendo FIFO_S2C");
        return;
    }

    int found = 0;
    if (read(rfd, &found, sizeof(int)) == -1) {
        perror("Error leyendo cantidad de resultados");
        close(rfd);
        return;
    }

    if (found == 0) {
        printf("\nNo se encontraron canciones con esos criterios.\n");
        close(rfd);
        pause_execution();
        return;
    }

    Song *results = malloc(found * sizeof(Song));
    if (!results) {
        perror("Error asignando memoria para resultados");
        close(rfd);
        return;
    }

    for (int i = 0; i < found; i++) {
        if (read(rfd, &results[i], sizeof(Song)) == -1) {
            perror("Error leyendo resultados");
            free(results);
            close(rfd);
            return;
        }
    }
    close(rfd);

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

    RequestMessage req;
    memset(&req, 0, sizeof(req));
    req.action = ACTION_ADD;
    req.data.song = s;

    int wfd = open(FIFO_C2S, O_WRONLY);
    if (wfd == -1) {
        perror("Error abriendo FIFO_C2S");
        return;
    }

    if (write(wfd, &req, sizeof(RequestMessage)) == -1) {
        perror("Error enviando datos al servidor");
        close(wfd);
        return;
    }
    close(wfd);

    int rfd = open(FIFO_S2C, O_RDONLY);
    if (rfd == -1) {
        perror("Error abriendo FIFO_S2C");
        return;
    }

    int ack = 0;
    if (read(rfd, &ack, sizeof(int)) == -1) {
        perror("Error leyendo confirmación");
        close(rfd);
        return;
    }
    close(rfd);

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
            continue; // Si ocurre error, reinicia el ciclo
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