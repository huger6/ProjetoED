/* Ficheiro para tratar do carregamento e exportação dos dados */

#include "dados.h"
#include "uteis.h"
#include "validacoes.h"
#include "dono.h"
#include "carro.h"
#include "sensores.h"
#include "distancias.h"
#include "passagens.h"
#include "constantes.h"
#include "configs.h"


/**
 * @brief Carrega todos os dados para a memória
 * 
 * @param bd Base de Dados
 * @param fDonos Ficheiro dos Donos
 * @param fCarros Ficheiro dos Carros
 * @param fSensores Ficheiro dos Sensores
 * @param fDistancias Ficheiro das Distâncias
 * @param fPassagem Ficheiro das Passagens
 * @param fLogs Ficheiro de logs
 * 
 * @return int 1 se sucesso, 0 se erro
 * 
 * @note Os ficheiros podem ser passados como NULL para usar o valor default
 */
int carregarDadosTxt(Bdados *bd, char *fDonos, char *fCarros, char *fSensores, char *fDistancias, char *fPassagem, char *fLogs) {
    const char *logFile = (fLogs) ? fLogs : LOGS_TXT; //Usar default em caso de não ser especificado

    //pode se criar um ficheiro html com o logs
    FILE *logsCheck = fopen(logFile, "r");
    char logsExiste = '0';
    if (logsCheck) { //Ficheiro já existe
        logsExiste = '1';
        fclose(logsCheck);
    }
    FILE *logs = fopen(logFile, "a");
    if (!logs) {
        printf("Ocorreu um erro grave ao abrir o ficheiro de logs '%s'.\n\n", logFile);
        return 0;
    }

    char erro = '0';
    if (logsExiste == '1') {
        fprintf(logs, "\n\n\n");
    }
    time_t inicio = time(NULL);
    fprintf(logs, "#ÍNICIO DA LEITURA DOS DADOS#\t\t%s\n\n", ctime(&inicio));
    while (erro == '0') {
        if (!carregarDonosTxt(bd, fDonos, logs)) {
            erro = '1';
            break;
        }
    
        if (!carregarCarrosTxt(bd, fCarros, logs)) {
            freeDict(bd->donosAlfabeticamente, freeChaveDonoAlfabeticamente, NULL);
            freeDict(bd->donosNif, freeChaveDonoNif, freeDono);
            erro = '1';
            break;
        }
    
        if (!carregarSensoresTxt(bd, fSensores, logs)) {
            freeDict(bd->donosAlfabeticamente, freeChaveDonoAlfabeticamente, NULL);
            freeDict(bd->donosNif, freeChaveDonoNif, freeDono);
            freeDict(bd->carrosMarca, freeChaveCarroMarca, NULL);
            freeDict(bd->carrosCod, freeChaveCarroCod, freeCarro);
            erro = '1';
            break;
        }   
    
        if (!carregarDistanciasTxt(bd, fDistancias, logs)) {
            freeDict(bd->donosAlfabeticamente, freeChaveDonoAlfabeticamente, NULL);
            freeDict(bd->donosNif, freeChaveDonoNif, freeDono);
            freeDict(bd->carrosMarca, freeChaveCarroMarca, NULL);
            freeDict(bd->carrosCod, freeChaveCarroCod, freeCarro);
            freeLista(bd->sensores, freeSensor);
            erro = '1';
            break;
        }
    
        if (!carregarPassagensTxt(bd, fPassagem, logs)) {
            freeDict(bd->donosAlfabeticamente, freeChaveDonoAlfabeticamente, NULL);
            freeDict(bd->donosNif, freeChaveDonoNif, freeDono);
            freeDict(bd->carrosMarca, freeChaveCarroMarca, NULL);
            freeDict(bd->carrosCod, freeChaveCarroCod, freeCarro);
            freeLista(bd->sensores, freeSensor);
            freeMatrizDistancias(bd->distancias);
            erro = '1';
            break;
        }
        break;
    }
    if (erro == '0') {
        for (char i = 'a'; i <= 'z'; i++) {
            void *letra = (void *)&i;
            Lista *p = obterListaDoDict(bd->donosAlfabeticamente, letra, compChaveDonoAlfabeticamente, hashChaveDonoAlfabeticamente);
            if (p) {
                mergeSortLista(p, compDonosNome);
            }
        }
    }

    time_t fim = time(NULL);
    char *tempoFinal = ctime(&fim); // Não precisa de free
    tempoFinal[strcspn(tempoFinal, "\n")] = '\0';
    fprintf(logs, "\n#FIM DA LEITURA DOS DADOS#\t\t%s\t\tTEMPO DE CARREGAMENTO:%.0fsegundos\n\n", tempoFinal, difftime(fim, inicio));
    fclose(logs);

    if (erro == '1') {
        return 0; //Erro grave
    }
    
    return 1;
}

/**
 * @brief Carrega os dados relativos aos Donos para memória
 * 
 * @param bd Base de Dados
 * @param donosFilename Nome do ficheiro dos Donos
 * @param logs Ficheiro de logs
 * @return int 1 se sucesso, 0 se erro
 */
int carregarDonosTxt(Bdados *bd, char *donosFilename, FILE *logs) {
    const char *donosFile = (donosFilename) ? donosFilename : DONOS_TXT;

    time_t inicio = time(NULL);   
    fprintf(logs, "#FICHEIRO DONOS#\t\t%s\n", ctime(&inicio));

    FILE *donos = fopen(donosFile, "r");
    if (donos) {
        int nLinhas = 0;
        char *linha = NULL;
        while((linha = lerLinhaTxt(donos, &nLinhas)) != NULL) {
            char *parametros[PARAM_DONOS];
            int numParam = 0; //Nº real de param lidos
            char *copiaLinha = strdup(linha); //Cópia da linha para passar para separarParam e evitar alterações à original
            separarParametros(copiaLinha, parametros, &numParam, PARAM_DONOS);
            char erro = '0';

            if (numParam == PARAM_DONOS) {
                //nif
                int nif = 0;
                if (!stringToInt(parametros[0], &nif) || !validarNif(nif)) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: Número de contribuinte inválido\n\n");
                    erro = '1';
                }
                //Nome
                char *nomeInvalido = NULL;
                nomeInvalido = validarNome(parametros[1]);
                if (nomeInvalido) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: %s\n\n", nomeInvalido);
                    erro = '1';
                }
                //codPostal
                short zona = 0, local = 0;
                if (!converterCodPostal(parametros[2], &zona, &local) || !validarCodPostal(zona, local)) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: O código postal é inválido\n\n");
                    erro = '1';
                }
                //Converter codigo Postal
                CodPostal postal;
                postal.local = local;
                postal.zona = zona;
                //Caso não haja erro passar os dados para as estruturas
                if (erro == '0') {
                    if(!inserirDonoLido(bd, parametros[1], nif, postal)) {
                        linhaInvalida(linha, nLinhas, logs);
                        fprintf(logs, "Razão: Ocorreu um erro fatal a carregar a linha para a memória\n\n");
                    }
                }
            }
            else if (numParam < PARAM_DONOS) {
                linhaInvalida(linha, nLinhas, logs);
                fprintf(logs, "Razão: Parametros insuficientes (%d NECESSÁRIOS, %d LIDOS)\n\n", PARAM_DONOS, numParam);
            }
            else {
                linhaInvalida(linha, nLinhas, logs);
                fprintf(logs, "Razão: Demasiados parametros (%d NECESSÁRIOS, %d LIDOS)\n\n", PARAM_DONOS, numParam);
            }
            free(copiaLinha);
            free(linha); 
        }
        fclose(donos);
    }
    else {
        fprintf(logs, "Ocorreu um erro ao abrir o ficheiro de Donos: '%s'\n\n", donosFile);
        return 0;
    }
    time_t fim = time(NULL);
    char *tempoFinal = ctime(&fim); // Não precisa de free
    tempoFinal[strcspn(tempoFinal, "\n")] = '\0';
    fprintf(logs, "\n#FIM FICHEIRO DONOS#\t\t%s\t\tTEMPO DE CARREGAMENTO:%.0fsegundos\n\n", tempoFinal, difftime(fim, inicio));
    return 1;
}

/**
 * @brief Carrega os dados relativos aos Carros para memória
 * 
 * @param bd Base de Dados
 * @param carrosFilename Nome do ficheiro dos Carros
 * @param logs Ficheiro de logs
 * @return int 1 se sucesso, 0 se erro
 */
int carregarCarrosTxt(Bdados *bd, char *carrosFilename, FILE *logs) {
    const char *carrosFile = (carrosFilename) ? carrosFilename : CARROS_TXT;

    time_t inicio = time(NULL);   
    fprintf(logs, "#FICHEIRO CARROS#\t\t%s\n", ctime(&inicio));

    FILE *carros = fopen(carrosFile, "r");
    if (carros) {
        int nLinhas = 0;
        char *linha = NULL;
        while((linha = lerLinhaTxt(carros, &nLinhas)) != NULL) {
            char *parametros[PARAM_CARROS];
            int numParam = 0; //Nº real de param lidos
            char *copiaLinha = strdup(linha);
            separarParametros(copiaLinha, parametros, &numParam, PARAM_CARROS);
            char erro = '0';

            if (numParam == PARAM_CARROS) {
                //Matrícula
                if (!validarMatricula(parametros[0])) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: Matrícula inválida\n\n");
                    erro = '1';
                }
                //Marca
                char *marca = NULL;
                marca = validarMarca(parametros[1]);
                if (marca) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: %s\n\n", marca);
                    erro = '1';
                }
                //Modelo
                char *modelo = NULL;
                modelo = validarModelo(parametros[2]);
                if (modelo) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: %s\n\n", modelo);
                    erro = '1';
                }
                //Ano
                short ano;
                if (!stringToShort(parametros[3], &ano) || !validarAnoCarro(ano)) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: Ano inválido\n\n");
                    erro = '1';
                }
                //NIF
                int nif;
                if (!stringToInt(parametros[4], &nif) || !validarNif(nif)) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: Número de contribuinte inválido\n\n");
                    erro = '1';
                }
                //CodVeiculo
                int codVeiculo;
                if (!stringToInt(parametros[5], &codVeiculo) || !validarCodVeiculo(codVeiculo)) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: Código do veículo errado\n\n");
                    erro = '1';
                }
                //Caso não haja erro passar os dados para as estruturas
                if (erro == '0') {
                    if(!inserirCarroLido(bd, parametros[0], parametros[1], parametros[2], ano, nif, codVeiculo)) {
                        linhaInvalida(linha, nLinhas, logs);
                        fprintf(logs, "Razão: Ocorreu um erro fatal a carregar a linha para a memória\n\n");
                    }
                }
            }
            else if (numParam < PARAM_CARROS) {
                linhaInvalida(linha, nLinhas, logs);
                fprintf(logs, "Razão: Parametros insuficientes (%d NECESSÁRIOS, %d LIDOS)\n\n", PARAM_CARROS, numParam);
            }
            else {
                linhaInvalida(linha, nLinhas, logs);
                fprintf(logs, "Razão: Demasiados parametros (%d NECESSÁRIOS, %d LIDOS)\n\n", PARAM_CARROS, numParam);
            }
            free(copiaLinha);
            free(linha); 
        }
        fclose(carros);
    }
    else {
        fprintf(logs, "Ocorreu um erro ao abrir o ficheiro de Carros: '%s'.\n\n", carrosFile);
        return 0;
    }
    time_t fim = time(NULL);
    char *tempoFinal = ctime(&fim); // Não precisa de free
    tempoFinal[strcspn(tempoFinal, "\n")] = '\0';
    fprintf(logs, "\n#FIM FICHEIRO CARROS#\t\t%s\t\tTEMPO DE CARREGAMENTO:%.0fsegundos\n\n", tempoFinal, difftime(fim, inicio));
    return 1;
}

/**
 * @brief Carrega os dados relativos aos Sensores para memória
 * 
 * @param bd Base de Dados
 * @param sensoresFilename Nome do ficheiro dos Sensores
 * @param logs Ficheiro de logs
 * @return int 1 se sucesso, 0 se erro
 */
int carregarSensoresTxt(Bdados *bd, char *sensoresFilename, FILE *logs) {
    const char *sensoresFile = (sensoresFilename) ? sensoresFilename : SENSORES_TXT;
    
    time_t inicio = time(NULL);   
    fprintf(logs, "#FICHEIRO SENSORES#\t\t%s\n", ctime(&inicio));

    FILE *sens = fopen(sensoresFile, "r");
    if (sens) {
        int nLinhas = 0;
        char *linha = NULL;
        while((linha = lerLinhaTxt(sens, &nLinhas)) != NULL) {
            char *parametros[PARAM_SENSORES];
            int numParam = 0; //Nº real de param lidos
            char *copiaLinha = strdup(linha);
            separarParametros(copiaLinha, parametros, &numParam, PARAM_SENSORES);
            char erro = '0';

            if (numParam == PARAM_SENSORES) {
                //Código do sensor
                int codSensor;
                if (!stringToInt(parametros[0], &codSensor) || !validarCodSensor(codSensor)) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: Código do sensor inválido\n\n");
                    erro = '1';
                }
                //Caso não haja erro passar os dados para as estruturas
                if (erro == '0') {
                    if(!inserirSensorLido(bd, codSensor, parametros[1], parametros[2], parametros[3])) {
                        linhaInvalida(linha, nLinhas, logs);
                        fprintf(logs, "Razão: Ocorreu um erro fatal a carregar a linha para a memória");
                    }
                }
            }
            else if (numParam < PARAM_SENSORES) {
                linhaInvalida(linha, nLinhas, logs);
                fprintf(logs, "Razão: Parametros insuficientes (%d NECESSÁRIOS, %d LIDOS)\n\n", PARAM_SENSORES, numParam);
            }
            else {
                linhaInvalida(linha, nLinhas, logs);
                fprintf(logs, "Razão: Demasiados parametros (%d NECESSÁRIOS, %d LIDOS)\n\n", PARAM_SENSORES, numParam);
            }
            free(copiaLinha);
            free(linha); 
        }
        fclose(sens);
    }
    else {
        fprintf(logs, "Ocorreu um erro ao abrir o ficheiro de Sensores: '%s'.\n\n", sensoresFile);
        return 0;
    }
    //Ordenar a lista
    ordenarLista(bd->sensores, compararSensores);
    time_t fim = time(NULL);
    char *tempoFinal = ctime(&fim); // Não precisa de free
    tempoFinal[strcspn(tempoFinal, "\n")] = '\0';
    fprintf(logs, "\n#FIM FICHEIRO SENSORES#\t\t%s\t\tTEMPO DE CARREGAMENTO:%.0fsegundos\n\n", tempoFinal, difftime(fim, inicio));
    return 1;
}

/**
 * @brief Carrega os dados relativos às Distâncias para memória
 * 
 * @param bd Base de Dados
 * @param distanciasFilename Nome do ficheiro das Distancias
 * @param logs Ficheiro de logs
 * @return int 1 se sucesso, 0 se erro
 */
int carregarDistanciasTxt(Bdados *bd, char *distanciasFilename, FILE *logs) {
    const char *distanciasFile = (distanciasFilename) ? distanciasFilename : DISTANCIAS_TXT;

    time_t inicio = time(NULL);   
    fprintf(logs, "#FICHEIRO DISTANCIAS#\t\t%s\n", ctime(&inicio));
    if (!realocarMatrizDistancias(bd, bd->sensores->nel)) {
        fprintf(logs, "Ocorreu um erro a realocar a matriz das distâncias\n\n");
        return 0;
    }
    FILE *dists = fopen(distanciasFile, "r");
    if (dists) {
        int nLinhas = 0;
        char *linha = NULL;
        while((linha = lerLinhaTxt(dists, &nLinhas)) != NULL) {
            char *parametros[PARAM_DISTANCIAS];
            int numParam = 0; //Nº real de param lidos
            char *copiaLinha = strdup(linha);
            separarParametros(copiaLinha, parametros, &numParam, PARAM_DISTANCIAS);
            char erro = '0';

            if (numParam == PARAM_DISTANCIAS) {
                //Código do sensor 1
                int codSensor1;
                if (!stringToInt(parametros[0], &codSensor1) || !validarCodSensor(codSensor1)) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: Código do sensor 1 inválido\n\n");
                    erro = '1';
                }
                //Código do sensor 2
                int codSensor2;
                if (!stringToInt(parametros[1], &codSensor2) || !validarCodSensor(codSensor2)) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: Código do sensor 2 inválido\n\n");
                    erro = '1';
                }
                float distancia;
                converterPontoVirgulaDecimal(parametros[2]); // Passa notação de floats para vírgulas caso necessário
                if (!stringToFloat(parametros[2], &distancia) || !validarDistancia(distancia)) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: Distância inválida\n\n");
                    erro = '1';
                    printf("Distancia: %.2f\n", distancia);
                }
                //Caso não haja erro passar os dados para as estruturas
                if (erro == '0') {
                    if(!inserirDistanciaLido(bd, codSensor1, codSensor2, distancia)) {
                        linhaInvalida(linha, nLinhas, logs);
                        fprintf(logs, "Razão: Ocorreu um erro fatal a carregar a linha para a memória");
                    }
                }
            }
            else if (numParam < PARAM_DISTANCIAS) {
                linhaInvalida(linha, nLinhas, logs);
                fprintf(logs, "Razão: Parametros insuficientes (%d NECESSÁRIOS, %d LIDOS)\n\n", PARAM_DISTANCIAS, numParam);
            }
            else {
                linhaInvalida(linha, nLinhas, logs);
                fprintf(logs, "Razão: Demasiados parametros (%d NECESSÁRIOS, %d LIDOS)\n\n", PARAM_DISTANCIAS, numParam);
            }
            free(copiaLinha);
            free(linha); 
        }
        fclose(dists);
    }
    else {
        fprintf(logs, "Ocorreu um erro ao abrir o ficheiro de Distancias: '%s'.\n\n", distanciasFile);
        return 0;
    }

    time_t fim = time(NULL);
    char *tempoFinal = ctime(&fim); // Não precisa de free
    tempoFinal[strcspn(tempoFinal, "\n")] = '\0';
    fprintf(logs, "\n#FIM FICHEIRO DISTANCIAS#\t\t%s\t\tTEMPO DE CARREGAMENTO:%.0fsegundos\n\n", tempoFinal, difftime(fim, inicio));
    return 1;
}

/**
 * @brief Carrega as passagens do ficheiro Txt
 * 
 * @param bd Base de dados
 * @param passagensFilename Nome do ficheiro das passagens
 * @param logs Ficheiro de logs, aberto
 * @return int 0 se erro, 1 se sucesso
 */
int carregarPassagensTxt(Bdados *bd, char *passagensFilename, FILE *logs) {
    const char *passagensFile = (passagensFilename) ? passagensFilename : PASSAGEM_TXT;

    time_t inicio = time(NULL);   
    fprintf(logs, "#FICHEIRO PASSAGENS#\t\t%s\n", ctime(&inicio));

    FILE *passagem = fopen(passagensFile, "r");
    if (passagem) {
        int nLinhas = 0;
        int nPassagens = 0; //Para verificar os pares das viagens
        char *linha = NULL;
        Passagem *viagem[2] = {NULL, NULL};

        while((linha = lerLinhaTxt(passagem, &nLinhas)) != NULL) {
            int indice = (nPassagens % 2 == 0) ? 0 : 1; // Índice do array
            if (indice == 1 && !viagem[0]) {
                // A primeira passagem foi inválida
                linhaInvalida(linha, nLinhas, logs);
                fprintf(logs, "Razão: É o par de uma passagem inválida!\n\n");
                free(linha);
                nPassagens++;
                continue; // Passar à frente
            }

            char *parametros[PARAM_PASSAGEM];
            int numParam = 0; //Nº real de param lidos
            char *copiaLinha = strdup(linha);
            separarParametros(copiaLinha, parametros, &numParam, PARAM_PASSAGEM);
            char erro = '0';

            if (numParam == PARAM_PASSAGEM) {
                //ID do sensor
                int idSensor;
                if (!stringToInt(parametros[0], &idSensor) || !validarCodSensor(idSensor)) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: ID do sensor inválido\n\n");
                    erro = '1';
                }
                //Código do veículo
                int codVeiculo;
                if (!stringToInt(parametros[1], &codVeiculo) || !validarCodVeiculo(codVeiculo)) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: Código do veículo inválido\n\n");
                    erro = '1';
                }
                //Data
                Data date;
                char *mensagemData = NULL;
                mensagemData = converterParaData(parametros[2], &date);
                if (mensagemData) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: %s\n\n", mensagemData);
                    erro = '1';
                }
                else if (!validarData(date, '0')) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: Data inválida\n\n");
                    erro = '1';
                }
                //Tipo de registo
                if (!validarTipoRegisto(parametros[3][0])) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: Tipo de registo inválido\n\n");
                    erro = '1';
                }
                if (indice == 1 && viagem[0]->tipoRegisto == parametros[3][0]) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: Tipo de registo inválido no par de passagem\n\n");
                    erro = '1';
                }
                if (indice == 1 && compararDatas(viagem[0]->data, date) == 1) {
                    linhaInvalida(linha, nLinhas, logs);
                    fprintf(logs, "Razão: Data da passagem de saída inválida (entrada também foi invalidada - linha %d)\n\n", nLinhas - 1);
                    erro = '1';
                }

                //Caso não haja erro passar os dados para as estruturas
                if (erro == '0') {
                    viagem[indice] = obterPassagem(idSensor, date, parametros[3][0]);
                    if (!viagem[indice] && indice == 1) {
                        freePassagem(viagem[0]);
                        viagem[0] = NULL;
                        viagem[1] = NULL;
                    }
                    else {
                        // Inserir viagem apenas se já tivermos 2 passagens
                        if (indice == 1) {
                            if (!inserirViagemLido(bd, viagem[0], viagem[1], codVeiculo)) {
                                linhaInvalida(linha, nLinhas, logs);
                                fprintf(logs, "Razão: Ocorreu um erro a carregar a viagem para memória\n\n");
                            }
                            // Dar set da viagem para o próximo par
                            viagem[0] = NULL;
                            viagem[1] = NULL;
                        }
                    }
                }
                if (erro == '1' && viagem[0] != NULL) {
                    freePassagem(viagem[0]);
                    viagem[0] = NULL;
                    viagem[1] = NULL;
                }
            }
            else if (numParam < PARAM_PASSAGEM) {
                linhaInvalida(linha, nLinhas, logs);
                fprintf(logs, "Razão: Parametros insuficientes (%d NECESSÁRIOS, %d LIDOS)\n\n", PARAM_PASSAGEM, numParam);
            }
            else {
                linhaInvalida(linha, nLinhas, logs);
                fprintf(logs, "Razão: Demasiados parametros (%d NECESSÁRIOS, %d LIDOS)\n\n", PARAM_PASSAGEM, numParam);
            }
            free(copiaLinha);
            free(linha); 
            nPassagens++;
        }
        // Limpar última passagem caso fique pendente
        if (viagem[0]) {
            freePassagem(viagem[0]);
        }
        if (viagem[1]) {
            freePassagem(viagem[1]);
        }
        fclose(passagem);
    }
    else {
        fprintf(logs, "Ocorreu um erro ao abrir o ficheiro de Passagens: '%s'.\n\n", passagensFile);
        return 0;
    }
    time_t fim = time(NULL);
    char *tempoFinal = ctime(&fim); // Não precisa de free
    tempoFinal[strcspn(tempoFinal, "\n")] = '\0';
    fprintf(logs, "\n#FIM FICHEIRO PASSAGENS#\t\t%s\t\tTEMPO DE CARREGAMENTO:%.0fsegundos\n\n", tempoFinal, difftime(fim, inicio));
    return 1;
}

/** 
 * @brief Remove espaços extra de uma string
 * @param str    String a modificar (se necessário)
 *
 * @note Remove espaços:
 *       - No início
 *       - No fim  
 *       - Múltiplos entre palavras
 * @note Preserva espaço único entre palavras
 * @note Verifica string NULL/vazia
 */
void removerEspacos(char *str) {
    if(str == NULL) return;
    char *inicio = str;
    char *fim = NULL;

    //Se o início conter um espaço, vai avançar o ponteiro uma casa, até essa casa deixar de ser um espaço
    while (*inicio == ' ') inicio++;

    // Copiar a string sem espaços para o array inicial
    if (inicio != str) {
        memmove(str, inicio, strlen(inicio) + 1); //É uma versão melhorada do memcpy (evita a sobreposição)
    }

    //Ponteiro para o último caractere
    fim = str + strlen(str) - 1;
    while (fim > str && *fim == ' ') { //verifica se o fim tem um espaço em branco, se sim, anda com o ponteiro uma casa para trás e repete
        fim--;
    }

    //Colocamos o nul char no final
    *(fim + 1) = '\0';
}

/**
 * @brief Separa linha em parâmetros usando SEPARADOR
 *
 * @param linha            String com linha completa a separar
 * @param parametros       Array de ponteiros para armazenar os parâmetros extraídos
 * @param numParametros   Ponteiro para contar os parâmetros
 * @param paramEsperados  Número de parâmetros esperados para separar
 *
 * @note Remove espaços extra via remover_espacos()
 * @note Em caso de erro define num_parametros = 0
 */
void separarParametros(char *linha, char **parametros, int *numParametros, const int paramEsperados) { // char ** parametros serve para armazenar os ponteiros dos parametros, de modo a que não sejam perdidos
    if (!linha || !parametros || !numParametros) return;

    *numParametros = 0;

    char *token = strtok(linha, SEPARADOR_STR); 

    while (token != NULL && *numParametros < paramEsperados) {
        removerEspacos(token);  
        parametros[*numParametros] = token;
        (*numParametros)++;
        token = strtok(NULL, SEPARADOR_STR);
    }
}

/**
 * @brief Escreve a linha inválida no ficheiro de logs
 * 
 * @param linha Linha inválida
 * @param nLinha Número da linha
 * @param logs Ficheiro de logs
 */
void linhaInvalida(const char *linha, int nLinha, FILE *logs) {
    fprintf(logs, "Linha %d inválida: %s\n", nLinha, linha);
}

/**
 * @brief Conta as linhas de um ficheiro
 * 
 * @param filename Nome do ficheiro
 * @return int -1 se erro, Nº linhas caso contrário
 */
int contarLinhas(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        return -1;
    }

    int linhas = 0;
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (c == '\n') {
            linhas++;
        }
    }

    fclose(f);

    return linhas;
}

// Dados Binários

/**
 * @brief Guarda todos os dados em memória para um ficheiro binário 
 * 
 * @param bd Base de dados
 * @param nome Nome do ficheiro a abrir
 * @return int 0 se erro, 1 se sucesso
 */
int guardarDadosBin(Bdados *bd, const char *nome) {
    if (!bd || !nome) return 0;

    FILE *file = fopen(nome, "wb");
    if (!file) return 0;

    // Checkum
    unsigned long sum = checksum(bd);
    fwrite(&sum, sizeof(unsigned long), 1, file);

    // Configs
    fwrite(&autosaveON, sizeof(int), 1, file);
    fwrite(&backupsON, sizeof(int), 1, file);
    fwrite(&pausaListagem, sizeof(int), 1, file);

    // Donos
    guardarDadosDictBin(bd->donosNif, guardarDonoBin, file);
    // Carros
    guardarDadosDictBin(bd->carrosCod, guardarCarroBin, file);
    // Sensores
    guardarListaBin(bd->sensores, guardarSensorBin, file);
    // Passagens/Viagens
    guardarListaBin(bd->viagens, guardarViagemBin, file);
    // Distâncias
    guardarDistanciasBin(bd->distancias, file);
    
    fclose(file);
    return 1;
}

/**
 * @brief Pede o nome do ficheiro onde carregar os dados
 * 
 * @param bd Base de dados
 */
void guardarDadosBinFicheiro(Bdados *bd) {
    if (!bd) return;

    limpar_terminal();
    // Guardar dados para ficheiro personalizado
    char *filename = NULL;
    do {
        printf("Nome do ficheiro (sem extensão): ");
        filename = lerLinhaTxt(stdin, NULL);
        if (!filename) {
            printf("A entrada é inválida!\n\n");
            pressEnter();
            continue;
        }
        if (!validarNomeFicheiro(filename)) {
            free(filename);
            pressEnter();
            continue;
        }
        break;
    } while(1);
    char *f = appendFileExtension(filename, DOT_BIN);
    if (guardarDadosBin(bd, f)) {
        printf("Os dados foram guardados com sucesso!\n\n");
    }
    else {
        printf("Ocorreu um erro a guardar os dados. Por favor, tente novamente mais tarde!\n\n");
    }
    free(f);
    free(filename);
    pressEnter();
}

/**
 * @brief Carrega os dados de ficheiro binário para memória
 * 
 * @param bd Base de dados
 * @param nome Nome do ficheiro a ler
 * @return int 1 se sucesso, 0 se erro
 */
int carregarDadosBin(Bdados *bd, const char *nome) {
    if (!bd || !nome) return 0;
    
    FILE *file = fopen(nome, "rb");
    if (!file) return 0;

    printf("\n\nA carregar dados...\n\n");

    // Checksum
    unsigned long sum = 0;
    fread(&sum, sizeof(unsigned long), 1, file);

    // Configs 
    fread(&autosaveON, sizeof(int), 1, file);
    fread(&backupsON, sizeof(int), 1, file);
    fread(&pausaListagem, sizeof(int), 1, file);

    // Donos
    bd->donosNif = readToDictBin(criarChaveDonoNif, hashChaveDonoNif, freeDono, freeChaveDonoNif, readDonoBin, file);
    bd->donosAlfabeticamente = criarDict();
    // Iterar o dict dos nifs e introduzir o ponteiro no bd->donosAlfabeticamente
    for (int i = 0; i < TAMANHO_TABELA_HASH; i++) {
        NoHashing *p = bd->donosNif->tabela[i];

        while(p) {
            if (p->dados) {
                No *x = p->dados->inicio;

                while(x) {
                    (void)appendToDict(bd->donosAlfabeticamente, x->info, compChaveDonoAlfabeticamente, criarChaveDonoAlfabeticamente, hashChaveDonoAlfabeticamente, NULL, freeChaveDonoAlfabeticamente);
                    
                    x = x->prox;
                }
            }
            p = p->prox;
        }
    }
    // Ordenar Donos Alfabeticamente
    for (char i = 'a'; i <= 'z'; i++) {
        void *letra = (void *)&i;
        Lista *p = obterListaDoDict(bd->donosAlfabeticamente, letra, compChaveDonoAlfabeticamente, hashChaveDonoAlfabeticamente);
        if (p) {
            mergeSortLista(p, compDonosNome);
        }
    }

    // Carros
    bd->carrosCod = readToDictBin(criarChaveCarroCod, hashChaveCarroCod, freeCarro, freeChaveCarroCod, readCarroBin, file);
    bd->carrosMarca = criarDict();
    bd->carrosMat = criarDict();
    // Obter ptrPessoa e libertar Dono atual (e adicionar Carros ao bd->carrosMarca)
    for (int i = 0; i < TAMANHO_TABELA_HASH; i++) {
        NoHashing *p = bd->carrosCod->tabela[i];

        while(p) {
            if (p->dados) {
                No *x = p->dados->inicio;

                while(x) {
                    Carro *carro = (Carro *)x->info;
                    void *chaveSearch = (void *)&carro->ptrPessoa->nif;
                    Dono *ptrDono = (Dono *)searchDict(bd->donosNif, chaveSearch, compChaveDonoNif, compDonosNif, hashChaveDonoNif);
                    free(carro->ptrPessoa);
                    carro->ptrPessoa = ptrDono;
                    if (carro->ptrPessoa) {
                        if (!carro->ptrPessoa->carros) {
                            carro->ptrPessoa->carros = criarLista();
                        }
                        (void) addInicioLista(carro->ptrPessoa->carros, (void *)carro);
                    }

                    // Adicionar ao bd->carrosMarca
                    (void)appendToDict(bd->carrosMarca, x->info, compChaveCarroMarca, criarChaveCarroMarca, hashChaveCarroMarca, NULL, freeChaveCarroMarca);
                    // Adicionar ao bd->carrosMat
                    (void)appendToDict(bd->carrosMat, x->info, compChaveCarroMatricula, criarChaveCarroMatricula, hashChaveCarroMatricula, NULL, freeChaveCarroMatricula);
                    
                    x = x->prox;
                }
            }
            p = p->prox;
        }
    }

    // Sensores
    bd->sensores = readListaBin(readSensorBin, file);

    // Passagens/Viagens
    bd->viagens = readListaBin(readViagemBin, file);
    // Libertar Carro atual e obter o seu ponteiro
    No *p = bd->viagens->inicio;

    while(p) {
        if (p->info) {
            Viagem *v = (Viagem *)p->info;
            void *chaveSearch = (void *)&v->ptrCarro->codVeiculo;
            Carro *ptrCarro = (Carro *)searchDict(bd->carrosCod, chaveSearch, compChaveCarroCod, compCodCarro, hashChaveCarroCod);
            if (ptrCarro) {
                free(v->ptrCarro);
                v->ptrCarro = ptrCarro;
                if (!v->ptrCarro->viagens) {
                    v->ptrCarro->viagens = criarLista();
                }
                (void) addInicioLista(v->ptrCarro->viagens, (void *)v);
            }
        }
        p = p->prox;
    }
    // Distâncias
    bd->distancias = readDistanciasBin(file);

    unsigned long sumAfter = checksum(bd);
    
    if (sum != sumAfter) {
        printf("O ficheiro está corrompido ou foi adulterado. Os dados podem estar incompletos.\n");
        if (!sim_nao("Deseja prosseguir mesmo assim?")) {
            printf("O programa será encerrado!\n");
            deleteFile(CONFIG_TXT, '1');
            fclose(file);
            deleteFile(AUTOSAVE_BIN, '1');
            freeTudo(bd);
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);
    return 1;
}

/**
 * @brief Pede o nome do ficheiro de onde carregar os dados
 * 
 * @param bd Base de dados
 * 
 * @return 1 se sucesso, 0 se erro
 */
int carregarDadosBinFicheiro(Bdados **bd) {
    if (!bd || !*bd) return 0;

    limpar_terminal();
        char *filename = NULL;
        do {
            printf("Nome do ficheiro (sem extensão): ");
            filename = lerLinhaTxt(stdin, NULL);
            if (!filename) {
                printf("A entrada é inválida!\n\n");
                pressEnter();
                continue;
            }
            if (!validarNomeFicheiro(filename)) {
                free(filename);
                pressEnter();
                continue;
            }
            break;
        } while(1);
        char *f = appendFileExtension(filename, DOT_BIN);

        // Libertar todos os dados
        (void) guardarDadosBin(*bd, AUTOSAVE_BIN);
        freeTudo(*bd);

        int sucesso = 0;

        *bd = (Bdados *)malloc(sizeof(Bdados));
        if (!carregarDadosBin(*bd, f)) {
            inicializarBD(*bd);
            printf("Ocorreu um erro a carregar os dados ou o ficheiro não existe. Verifique se o ficheiro existe.\n\n");
            sucesso = 0;
        }
        else {
            printf("Os dados foram carregados com sucessso!\n\n");
            sucesso = 1;
        }
        free(filename);
        free(f);

        pressEnter();
        return sucesso;
}

/**
 * @brief Calcula o checksum da base de dados
 * 
 * @param bd Base de dados
 * @return unsigned long checksum
 */
unsigned long checksum(Bdados *bd) {
    if (!bd) return 0;

    unsigned long sum = 0;

    sum += autosaveON;
    sum += backupsON;
    sum += pausaListagem;
    // Donos
    for (int i = 0; i < TAMANHO_TABELA_HASH; i++) {
        NoHashing *p = bd->donosNif->tabela[i];
        while(p) {
            if (p->dados) {
                No *x = p->dados->inicio;
                while(x) {
                    Dono *d = (Dono *)x->info;
                    // NIF
                    sum += d->nif;
                    // Nome
                    for (int j = 0; d->nome[j]; j++) {
                        sum += d->nome[j];
                    }
                    // Cod postal 
                    sum += d->codigoPostal.zona;
                    sum += d->codigoPostal.local;
                    x = x->prox;
                }
            }
            p = p->prox;
        }
    }
    sum += bd->donosNif->nelDict;

    // Carros
    for (int i = 0; i < TAMANHO_TABELA_HASH; i++) {
        NoHashing *p = bd->carrosCod->tabela[i];
        while(p) {
            if (p->dados) {
                No *x = p->dados->inicio;
                while(x) {
                    Carro *c = (Carro *)x->info;
                    // Cod
                    sum += c->codVeiculo;
                    // Matricula
                    for (int j = 0; c->matricula[j]; j++) {
                        sum += c->matricula[j];
                    }
                    // Marca e modelo
                    for (int j = 0; c->marca[j]; j++) {
                        sum += c->marca[j];
                    }
                    for (int j = 0; c->modelo[j]; j++) {
                        sum += c->modelo[j];
                    }
                    // Ano
                    sum += c->ano;
                    x = x->prox;
                }
            }
            p = p->prox;
        }
    }
    sum += bd->carrosCod->nelDict;

    // Sensores
    No *s = bd->sensores->inicio;
    while(s) {
        Sensor *sensor = (Sensor *)s->info;
        // ID
        sum += sensor->codSensor;
        // Designação
        for (int i = 0; sensor->designacao[i]; i++) {
            sum += sensor->designacao[i];
        }
        // Longitude
        for (int i = 0; sensor->longitude[i]; i++) {
            sum += sensor->longitude[i];
        }
        // Latitude
        for (int i = 0; sensor->latitude[i]; i++) {
            sum += sensor->latitude[i];
        }
        s = s->prox;
    }
    sum += bd->sensores->nel;

    // Distâncias
    int tamanho = bd->distancias->nColunas;
    for (int i = 0; i < tamanho; i++) {
        for (int j = i; j < tamanho; j++) {
            if (i == j) continue;
            sum += (unsigned long)(bd->distancias->matriz[i * tamanho + j] * 100); // Multiplica por 100 para preservar decimais
        }
    }

    // Viagens
    No *v = bd->viagens->inicio;
    while(v) {
        Viagem *viagem = (Viagem *)v->info;
        // ID sensores
        sum += viagem->entrada->idSensor;
        sum += viagem->saida->idSensor;
        // Data entrada
        sum += viagem->entrada->data.dia;
        sum += viagem->entrada->data.mes;
        sum += viagem->entrada->data.ano;
        // Data saida
        sum += viagem->saida->data.dia;
        sum += viagem->saida->data.mes;
        sum += viagem->saida->data.ano;
        // Tipo registro
        sum += viagem->entrada->tipoRegisto;
        sum += viagem->saida->tipoRegisto;
        v = v->prox;
    }
    sum += bd->viagens->nel;

    return sum;
}

