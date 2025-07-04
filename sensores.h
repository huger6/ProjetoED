#ifndef SENSORES_HEADERS
#define SENSORES_HEADERS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Bdados;

typedef struct {
    int codSensor; //PRIMARY KEY
    char *designacao;
    char *latitude;
    char *longitude;
} Sensor, *ptSensor;

int inserirSensorLido(struct Bdados *bd, int codSensor, char *designacao, char *latitude, char *longitude);
int compararSensores(void *sensor1, void *sensor2);
int compIdSensor(void *sensor, void *idSensor);
void freeSensor(void *sensor);
void printSensor(void *sensor);
void guardarSensorBin(void *sensor, FILE *file);
void *readSensorBin(FILE *file);
void printSensorXML(void *sensor, int indentacao, FILE *file);
void printHeaderSensoresCSV(FILE *file);
void printSensorCSV(void *sensor, FILE *file);
void printHeaderSensoresHTML(FILE *file);
void printSensorHTML(void *sensor, FILE *file);
size_t memUsageSensor(void *sensor);


#endif