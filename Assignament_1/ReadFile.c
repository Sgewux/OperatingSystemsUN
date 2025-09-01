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

    FILE *fp = fopen("person.txt", "r");
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    fscanf(fp, "Name: %59[^\n]\n", p1.name);
    fscanf(fp, "Lastname: %59[^\n]\n", p1.lastname);
    fscanf(fp, "Age: %d\n", &p1.age);
    fscanf(fp, "Height: %d\n", &p1.height);


    printf("Name: %s\n", p1.name);
    printf("Name: %s\n", p1.lastname);
    printf("Age: %d\n", p1.age);
    printf("Height: %d\n", p1.height);

    fclose(fp);

    return 0;
}