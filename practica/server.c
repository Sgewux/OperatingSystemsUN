#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// --------------------------------------------
// Agregar una canción al CSV y actualizar el índice hash
// --------------------------------------------
int perform_add_song_server(Song s) {
    // Abrir CSV para lectura/escritura (crear si no existe)
    FILE *csv = fopen("song_lyrics.csv", "r+");
    if (!csv) csv = fopen("song_lyrics.csv", "w+");
    if (!csv) {
        perror("Error abriendo song_lyrics.csv para agregar");
        return 0;
    }

    // Mover el puntero al final manualmente
    fseek(csv, 0, SEEK_END);

    // Guardar la posición exacta donde inicia la nueva línea
    long data_offset = ftell(csv);

    // Escribir la canción en formato CSV (ajusta si tu formato varía)
    fprintf(csv, "%s,%s,%d,%d,%s,%s\n",
            s.titulo, s.artist, s.year, s.views, s.tag, s.language);

    fflush(csv);
    fclose(csv);

    // -------------------------
    // Actualizar índice hash
    // -------------------------
    FILE *f_index = fopen("hash_index.bin", "rb+");
    FILE *f_nodes = fopen("index_nodes.bin", "ab+");
    if (!f_index || !f_nodes) {
        perror("Error abriendo archivos de índice");
        if (f_index) fclose(f_index);
        if (f_nodes) fclose(f_nodes);
        return 0;
    }

    // Calcular hash del título
    unsigned long hash = djb2_hash(s.titulo) % TABLE_SIZE;
    long bucket_offset = hash * sizeof(long);
    long current_head = -1;

    // Leer el head actual del bucket
    fseek(f_index, bucket_offset, SEEK_SET);
    fread(&current_head, sizeof(long), 1, f_index);

    // Crear el nuevo nodo del índice
    IndexNode new_node;
    memset(&new_node, 0, sizeof(IndexNode));
    strncpy(new_node.key, s.titulo, MAX_TITLE_SIZE);
    new_node.data_offset = data_offset;
    new_node.next_offset = current_head;

    // Escribir el nodo al final del archivo de nodos
    fseek(f_nodes, 0, SEEK_END);
    long new_node_offset = ftell(f_nodes);
    fwrite(&new_node, sizeof(IndexNode), 1, f_nodes);
    fflush(f_nodes);

    // Actualizar el head del bucket
    fseek(f_index, bucket_offset, SEEK_SET);
    fwrite(&new_node_offset, sizeof(long), 1, f_index);
    fflush(f_index);

    fclose(f_index);
    fclose(f_nodes);

    return 1; // Éxito
}


// --------------------------------------------
// Realiza la búsqueda según los criterios recibidos
// --------------------------------------------
void search(SearchCriteria criteria, Song *results, int *out_found) {
    FILE *f_index = fopen("hash_index.bin", "rb");
    FILE *f_nodes = fopen("index_nodes.bin", "rb");
    FILE *csv = fopen("song_lyrics.csv", "r");

    if (!f_index || !f_nodes || !csv) {
        perror("Error opening files");
        return;
    }

    // Calcular hash del título (clave principal)
    unsigned long hash = djb2_hash(criteria.titulo) % TABLE_SIZE;
    long bucket_offset = hash * sizeof(long);
    long current_offset;

    // Obtener nodo inicial del bucket
    fseek(f_index, bucket_offset, SEEK_SET);
    if (fread(&current_offset, sizeof(long), 1, f_index) != 1) {
        perror("Error reading index file");
        fclose(f_index);
        fclose(f_nodes);
        fclose(csv);
        return;
    }

    char *buffer = malloc(BUFFER_SIZE * sizeof(char));
    if (!buffer) {
        perror("Error allocating memory");
        return;
    }

    int found = 0;
    to_lower(criteria.titulo);
    to_lower(criteria.artist);

    // Recorrer lista enlazada del bucket
    while (current_offset != -1) {
        IndexNode node;
        fseek(f_nodes, current_offset, SEEK_SET);

        if (fread(&node, sizeof(IndexNode), 1, f_nodes) != 1) {
            perror("Error reading index node");
            break;
        }

        char node_key_lower[MAX_TITLE_SIZE];
        strncpy(node_key_lower, node.key, MAX_TITLE_SIZE);
        to_lower(node_key_lower);

        // Verificar si la clave coincide con el título
        if (strncmp(node_key_lower, criteria.titulo, MAX_TITLE_SIZE) == 0) {
            // Leer registro del CSV
            fseek(csv, node.data_offset, SEEK_SET);
            if (fgets(buffer, BUFFER_SIZE, csv)) {
                Song song = parse_song(buffer);

                char artist_lower[MAX_ARTIST_SIZE];
                strncpy(artist_lower, song.artist, MAX_ARTIST_SIZE);
                to_lower(artist_lower);

                int matches = 1;

                // Filtrar por artista si fue ingresado
                if (strlen(criteria.artist) > 0) {
                    matches = matches && (strcmp(artist_lower, criteria.artist) == 0);
                }

                if (matches) {
                    results[found++] = song;
                    if (found >= MAX_RESULTS) break; // límite de resultados
                }
            }
        }
        current_offset = node.next_offset;
    }

    *out_found = found;

    free(buffer);
    fclose(f_index);
    fclose(f_nodes);
    fclose(csv);
}

// --------------------------------------------
// Proceso principal del servidor
// --------------------------------------------
int main() {
    printf("========================================\n");
    printf("      MUSIC SEARCH SERVER ACTIVE\n");
    printf("========================================\n");

    int server, client;
    struct sockaddr_in addr;

    server = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (struct sockaddr*)&addr, sizeof(addr));
    listen(server, 1);

    printf("Esperando conexión...\n");
    socklen_t addrlen = sizeof(addr);
    client = accept(server, (struct sockaddr*)&addr, &addrlen);
    printf("Cliente conectado.\n");

    while (1) {
        Song *results = malloc(MAX_RESULTS * sizeof(Song));

        if (!results) {
            perror("Error allocating memory");
            return 1;
        }

        int found = 0;
        RequestMessage req;

        int bytes = recv(client, &req, sizeof(req), 0);

        if (bytes <= 0) { // cliente cerró la conexión
            printf("Cliente desconectado.\n");
            free(results);
            break;
        }

        // --- Acción de búsqueda ---
        if (req.action == ACTION_SEARCH) {
            search(req.data.search, results, &found);

            // Enviar cuántos resultados se encontraron
            send(client, &found, sizeof(int), 0);

            if (found == 0) {
                printf("========================================\n");
                printf("  No se encontraron coincidencias.\n");
                printf("========================================\n");
            } else {
                printf("========================================\n");
                printf("  %d canciones encontradas.\n", found);
                printf("========================================\n");

                for (int i = 0; i < found; i++) {
                    Song *s = &results[i];

                
                    send(client, s, sizeof(Song), 0);

                    // Log del servidor
                    printf("[%d] %s - %s (%d) | Views: %d\n",
                           i + 1, s->titulo, s->artist, s->year, s->views);
                }
            }
        }

        // --- Acción de agregar canción ---
        else if (req.action == ACTION_ADD) {
            printf("Solicitud de agregar canción recibida: %s - %s\n",
                   req.data.song.titulo, req.data.song.artist);

            int ack = perform_add_song_server(req.data.song);

            // Enviar confirmación al cliente
            send(client, &ack, sizeof(int), 0);

            if (ack)
                printf("Canción agregada correctamente al CSV y al índice.\n");
            else
                printf("Error al agregar la canción.\n");
        }

        free(results);
    }

    close(client);
    close(server);

    return 0;
}


