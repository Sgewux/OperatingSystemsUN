#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[60];
    char lastname[60];
    int age;
    int height;
} Person;

int main(){

    Person p1;

    char *name;
    char *lastname;
    int age;
    int height;
    name = malloc(60);
    lastname = malloc(60);

    scanf("%s", name);
    scanf("%s", lastname);
    scanf("%d", &age);
    scanf("%d", &height);

    strcpy(p1.name, name);
    strcpy(p1.lastname, lastname);
    p1.age = age;
    p1.height = height;

    free(name);
    free(lastname);


    FILE *fp = fopen("person.txt", "w");
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    fprintf(fp, "Name: %s\n", p1.name);
    fprintf(fp, "Lastname: %s\n", p1.lastname);
    fprintf(fp, "Age: %d\n", p1.age);
    fprintf(fp, "Height: %d\n", p1.height);

    fclose(fp);

    return 0;
}