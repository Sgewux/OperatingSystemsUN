#include "utils.h"
// --------------------------------------------
// Escribe el registro
// --------------------------------------------

void add_song(Song *song) {

    FILE *file = fopen("song_lyrics.csv", "ab+"); // append + lectura
    FILE *f_index = fopen("hash_index.bin", "wb+");
    FILE *f_nodes = fopen("index_nodes.bin", "wb+");
    if (!file) {
        perror("Error al abrir songs.csv");
    }

    // Mover al final del archivo
    fseek(file, 0, SEEK_END);

    // Guardar el offset antes de escribir
    long offset = ftell(file);

    // Escribir todos los campos (los vacíos quedan como ,,)
    fprintf(file, "%s,%s,%s,%d,%d,%s,%s,%d,%s,%s,%s\n",
            (song->titulo && song->titulo[0] != '\0') ? song->titulo : "",
            (song->tag && song->tag[0] != '\0') ? song->tag : "",
            (song->artist && song->artist[0] != '\0') ? song->artist : "",
            song->year,
            song->views,
            (song->features && song->features[0] != '\0') ? song->features : "",
            (song->lyrics && song->lyrics[0] != '\0') ? song->lyrics : "",
            song->id,
            (song->language_cld3 && song->language_cld3[0] != '\0') ? song->language_cld3 : "",
            (song->language_ft && song->language_ft[0] != '\0') ? song->language_ft : "",
            (song->language && song->language[0] != '\0') ? song->language : "");


    // Actualizar indice

    // Calculo de hash y ubicación del bucket
    unsigned long hash = djb2_hash(song->titulo) % TABLE_SIZE;
    long bucket_offset = hash * sizeof(long);
    long old_head;

    //Leer cabeza del bucket
    fseek(f_index, bucket_offset, SEEK_SET);
    if (fread(&old_head, sizeof(long), 1, f_index) != 1) {
        if (ferror(f_index)) {
            // Error real de lectura
            perror("Error while reading bucket's head");
        }
    }
    // Crear nuevo nodo
    IndexNode node;
    strncpy(node.key, song->titulo, MAX_TITLE_SIZE - 1);
    node.key[MAX_TITLE_SIZE - 1] = '\0';
    node.data_offset = offset;
    node.next_offset = old_head;

    // Agregar nuevo nodo al archivo de nodos
    fseek(f_nodes, 0, SEEK_END);
    long new_node_offset = ftell(f_nodes);
    fwrite(&node, sizeof(IndexNode), 1, f_nodes);

    // Actualizar la cabeza del bucket
    fseek(f_index, bucket_offset, SEEK_SET);
    fwrite(&new_node_offset, sizeof(long), 1, f_index);
    
    fclose(file);
    fclose(f_index);
    fclose(f_nodes);
}

int main(){
        //
    Song s = {
        .titulo = "Equisde",
        .tag = "rock",
        .artist = "uwu",
        .year = 1930,
        .views = 987654,
        .features = "",
        .lyrics = "klsdgyasjdvasdkvashdgvsaghdfvhsadgfvakdjsfbdsf",
        .id = 0,
        .language_cld3 = "",
        .language_ft = "",
        .language = "en"
    };
    //
    add_song(&s);
    return 0;
}