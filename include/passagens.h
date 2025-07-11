#ifndef PASSAGENS_HEADERS
#define PASSAGENS_HEADERS

#include <stdio.h>
#include <stdlib.h>

#include "sensores.h"
#include "carro.h"
#include "uteis.h"

struct Bdados;

typedef struct {
    int idSensor; //PRIMARY KEY / FOREIGN KEY(idSensor) - REFERENCES Sensor(codSensor)
    Data data;
    char tipoRegisto;
} Passagem, Pass, *ptPassagem, *ptPass;

typedef struct {
    Carro *ptrCarro;
    Passagem *entrada;
    Passagem *saida;
    float kms;
    float tempo; //Em minutos
    float velocidadeMedia; // km/h
} Viagem;


Passagem *obterPassagem(int idSensor, Data date, char tipoRegisto);
int inserirViagemLido(struct Bdados *bd, Passagem *entrada, Passagem *saida, int codVeiculo);
int compararPassagens(void *passagem1, void *passagem2);
int compCodPassagem(void *passagem, void *codigo);
void freePassagem(void *passagem);
void freeViagem(void *viagem);
void guardarViagemBin(void *viagem, FILE *file);
void *readViagemBin(FILE *file);
void guardarPassagemBin(void *passagem, FILE *file);
void *readPassagemBin(FILE *file);
void getStatsViagem(struct Bdados *bd, Viagem *v);
void printViagem(void *viagem, FILE *file);
void printViagemXML(void *viagem, int indentacao, FILE *file);
void printHeaderViagensCSV(FILE *file);
void printViagemCSV(void *viagem, FILE *file);
void printHeaderViagensTXT(FILE *file);
void printViagemTXT(void *viagem, FILE *file);
void printHeaderViagensHTML(FILE *file);
void printViagemHTML(void *viagem, FILE *file);
size_t memUsagePassagem(void *passagem);
size_t memUsageViagem(void *viagem);
void registarViagem(struct Bdados *bd);
Passagem *pedirPassagem(struct Bdados *bd);
void listarViagensTodas(struct Bdados *bd);



#endif