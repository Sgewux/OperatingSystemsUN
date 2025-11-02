#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// --------------------------------------------
// Agregar una canción al CSV y actualizar el índice hash
// --------------------------------------------
int perform_add_song_server(Song s) {
    FILE *csv = fopen("song_lyrics.csv", "a+");
    if (!csv) {
        perror("Error abriendo song_lyrics.csv para agregar");
        return 0;
    }

    fseek(csv, 0, SEEK_END);
    long data_offset = ftell(csv);

    // ✅ Orden corregido: title, tag, artist, year, views, lyrics, language
    fprintf(csv, "%s,%s,%s,%d,%d,%s,%s\n",
            s.titulo, s.tag, s.artist, s.year, s.views, s.lyrics, s.language);

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

    unsigned long hash = djb2_hash(s.titulo) % TABLE_SIZE;
    long bucket_offset = hash * sizeof(long);
    long current_head = -1;

    fseek(f_index, bucket_offset, SEEK_SET);
    fread(&current_head, sizeof(long), 1, f_index);

    IndexNode new_node;
    memset(&new_node, 0, sizeof(IndexNode));
    strncpy(new_node.key, s.titulo, MAX_TITLE_SIZE);
    new_node.data_offset = data_offset;
    new_node.next_offset = current_head;

    fseek(f_nodes, 0, SEEK_END);
    long new_node_offset = ftell(f_nodes);
    fwrite(&new_node, sizeof(IndexNode), 1, f_nodes);
    fflush(f_nodes);

    fseek(f_index, bucket_offset, SEEK_SET);
    fwrite(&new_node_offset, sizeof(long), 1, f_index);
    fflush(f_index);

    fclose(f_index);
    fclose(f_nodes);

    return 1;
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

    unsigned long hash = djb2_hash(criteria.titulo) % TABLE_SIZE;
    long bucket_offset = hash * sizeof(long);
    long current_offset;

    fseek(f_index, bucket_offset, SEEK_SET);
    if (fread(&current_offset, sizeof(long), 1, f_index) != 1) {
        perror("Error reading index file");
        fclose(f_index);
        fclose(f_nodes);
        fclose(csv);
        return;
    }

    char *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("Error allocating memory");
        fclose(f_index);
        fclose(f_nodes);
        fclose(csv);
        return;
    }

    int found = 0;
    to_lower(criteria.titulo);
    to_lower(criteria.artist);

    while (current_offset != -1) {
        IndexNode node;
        fseek(f_nodes, current_offset, SEEK_SET);
        if (fread(&node, sizeof(IndexNode), 1, f_nodes) != 1) break;

        char node_key_lower[MAX_TITLE_SIZE];
        strncpy(node_key_lower, node.key, MAX_TITLE_SIZE);
        to_lower(node_key_lower);

        if (strncmp(node_key_lower, criteria.titulo, MAX_TITLE_SIZE) == 0) {
            fseek(csv, node.data_offset, SEEK_SET);
            if (fgets(buffer, BUFFER_SIZE, csv)) {
                Song song = parse_song(buffer);

                char artist_lower[MAX_ARTIST_SIZE];
                strncpy(artist_lower, song.artist, MAX_ARTIST_SIZE);
                to_lower(artist_lower);

                int matches = 1;
                if (strlen(criteria.artist) > 0)
                    matches = matches && (strcmp(artist_lower, criteria.artist) == 0);

                if (matches) {
                    results[found++] = song;
                    if (found >= MAX_RESULTS) break;
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
// Proceso principal del servidor (sockets TCP)
// --------------------------------------------
int main() {
    printf("========================================\n");
    printf("      MUSIC SEARCH SERVER ACTIVE\n");
    printf("========================================\n");

    int server, client;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        perror("Error creando socket");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Error en bind");
        close(server);
        return 1;
    }

    if (listen(server, 5) < 0) {
        perror("Error en listen");
        close(server);
        return 1;
    }

    printf("Esperando conexiones en el puerto %d...\n", SERVER_PORT);

    // ✅ Aceptar múltiples clientes (uno por conexión)
    while (1) {
        client = accept(server, (struct sockaddr*)&addr, &addrlen);
        if (client < 0) {
            perror("Error aceptando conexión");
            continue;
        }

        printf("\n🔗 Cliente conectado.\n");

        RequestMessage req;
        int bytes = recv(client, &req, sizeof(req), 0);

        if (bytes <= 0) {
            printf("Cliente desconectado inmediatamente.\n");
            close(client);
            continue;
        }

        Song *results = malloc(MAX_RESULTS * sizeof(Song));
        if (!results) {
            perror("Error allocating memory");
            close(client);
            continue;
        }

        int found = 0;

        // --- Buscar canción ---
        if (req.action == ACTION_SEARCH) {
            search(req.data.search, results, &found);
            send(client, &found, sizeof(int), 0);

            if (found > 0) {
                for (int i = 0; i < found; i++) {
                    send(client, &results[i], sizeof(Song), 0);
                    printf("[%d] %s - %s (%d) | Views: %d\n",
                           i + 1, results[i].titulo, results[i].artist,
                           results[i].year, results[i].views);
                }
                printf("✅ Enviadas %d canciones al cliente.\n", found);
            } else {
                printf("❌ No se encontraron coincidencias.\n");
            }
        }

        // --- Agregar canción ---
        else if (req.action == ACTION_ADD) {
            printf("Solicitud de agregar canción: %s - %s\n",
                   req.data.song.titulo, req.data.song.artist);

            int ack = perform_add_song_server(req.data.song);
            send(client, &ack, sizeof(int), 0);

            if (ack)
                printf("✅ Canción agregada correctamente.\n");
            else
                printf("❌ Error al agregar la canción.\n");
        }

        free(results);
        close(client);
        printf("🔒 Cliente desconectado. Esperando otro...\n");
    }

    close(server);
    return 0;
}
