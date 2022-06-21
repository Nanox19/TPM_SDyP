#include <stdio.h>
///#include <rpcndr.h>
#include <stdlib.h>
///#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define n 150
#define CICLOS 10
#define SEMANAS 240

typedef enum {
    BLANCO=0,
    AZUL=1,
    ROJO=2,
    NARANJA=3,
    VERDE=4,
}Estado;
typedef struct {
    Estado estado;
    int edad;
    int herida_abierta;
    int tiempo_contagio;
    int tiempo_podado;
    int fila;
    int columna;
}Celda;



Celda ** Crear_Matriz(){
    Celda **Matriz;
    Matriz= (Celda **)malloc (n*sizeof(Celda *));
    Celda *Mf;
    Mf = (Celda *) malloc(n*n*sizeof(Celda));

    /*for (int i=0;i<n;i++)
        Matriz[i] = (Celda *) malloc (n*sizeof(Celda));*/
    for (int i=0; i<n; i++) {
        Matriz[i] = Mf + i*n;
    }
    return Matriz;
}


void CopiarDatos(Celda** estado_viejo, Celda** estado_actual){
    for(int i=0; i<n ; i++) {
        for (int j = 0; j < n; j++) {
            estado_viejo[i][j]=estado_actual[i][j];
        }
    }

}void CopiarDatos2(Celda** estado_viejo, Celda** estado_actual){
  **estado_viejo=**estado_actual;
}
void CopiarDatos3(Celda** estado_viejo, Celda** estado_actual){

        memcpy((*estado_viejo), (*estado_actual), sizeof(Celda)*n*n);

}

float generador_Uniforme(int random, int a, int b){
    float resultado=((float)(random %(b-a+1) + a)/100.0f);
    return resultado;
}

int generadorUniformeENTEROS(int random,int a, int b) {

    int ret = a + (b - a) * generador_Uniforme(random,0,100);
    return ret;
}




void init(Celda** estadoActual){
    Celda Celda_auxiliar;
    int i,j;
    ///#pragma omp parallel for schedule(dynamic,4) private(i)
    for(i=0; i<n ; i++) {
        for (j = 0; j < n; j++) {
            float prob = generador_Uniforme(rand(),0,100);
            if(prob<=0.05){
                Celda_auxiliar.estado=ROJO;
                Celda_auxiliar.tiempo_contagio=(generadorUniformeENTEROS(rand(),0,7));
            }else{
                if(prob<=0.15){
                    Celda_auxiliar.estado=NARANJA;
                    Celda_auxiliar.tiempo_contagio=(generadorUniformeENTEROS(rand(),0,7));
                }else{
                    if(prob<=0.35){
                        Celda_auxiliar.estado=AZUL;
                        Celda_auxiliar.tiempo_contagio=(generadorUniformeENTEROS(rand(),4,7));
                    }else{
                        Celda_auxiliar.estado=VERDE;
                        Celda_auxiliar.tiempo_contagio=-1;

                    }
                }
            }
            prob=generador_Uniforme(rand(),0,100);;

            if(prob<0.30){
                Celda_auxiliar.edad=(generadorUniformeENTEROS(rand(),1,156));
                if(generador_Uniforme(rand(),0,100)<0.23){
                    Celda_auxiliar.herida_abierta=1;
                } else{
                    Celda_auxiliar.herida_abierta=0;
                }
            }else{
                if(prob<0.80){
                    Celda_auxiliar.edad=(generadorUniformeENTEROS(rand(),157,1820));
                    if(generador_Uniforme(rand(),0,100)<0.08){
                        Celda_auxiliar.herida_abierta=1;
                    }else{
                        Celda_auxiliar.herida_abierta=0;
                    }
                }else{
                    Celda_auxiliar.edad=(generadorUniformeENTEROS(rand(),1821,2080));
                    if(generador_Uniforme(rand(),0,100)<0.37){
                        Celda_auxiliar.herida_abierta=1;
                    } else{
                        Celda_auxiliar.herida_abierta=0;
                    }
                }
            }
            ///estadoActual[i][j].herida_abierta=(int)generador_Uniforme(rand(),0,1);
            Celda_auxiliar.tiempo_podado=-1;
            Celda_auxiliar.fila=i;
            Celda_auxiliar.columna=j;
            /// si no anda probar critical
            estadoActual[i][j]=Celda_auxiliar;

        }
    }

}
float susceptibilidad(int edad,int heridas_A){
    float suscep=0;
    if(edad<=156){
        suscep=0.35;
    }else{
        if(edad<=1820){
            suscep=0.17;
        }
        else{
            suscep=0.63;
        }
    }
    if(heridas_A){
        suscep+=0.15;
    }
    return suscep;
}

float procesarContagio(float Porc_vecinosEnf ,float susceptibilidad){
    return ((Porc_vecinosEnf + susceptibilidad)* 0.60) + 0.07;
}



Celda procesarCelda(Celda celda, int vecinosEnfermos){
    Celda nuevaCelda;/*=celda;*/
    nuevaCelda.estado=celda.estado;
    nuevaCelda.edad=celda.edad;
    nuevaCelda.herida_abierta=celda.herida_abierta;
    nuevaCelda.columna=celda.columna;
    nuevaCelda.fila=celda.fila;
    nuevaCelda.tiempo_contagio=celda.tiempo_contagio;
    nuevaCelda.tiempo_podado=celda.tiempo_podado;

    switch (celda.estado) {
        case ROJO:{
            int probabilidad= generador_Uniforme(rand(),0,100);
            if((celda.tiempo_contagio>4)&&(probabilidad<0.85)){
                nuevaCelda.estado=AZUL;
            }
            nuevaCelda.tiempo_contagio++;
            break;
        }
        case AZUL:{
            if(celda.tiempo_contagio>7){
                int probabilidad= generador_Uniforme(rand(),0,100);
                if(celda.edad<156){
                    if(probabilidad<=0.03){
                        nuevaCelda.estado=BLANCO;
                        nuevaCelda.tiempo_podado=0;
                        nuevaCelda.tiempo_contagio=-1;
                    } else{
                        nuevaCelda.estado=VERDE;
                        nuevaCelda.tiempo_contagio=-1;
                    }
                }else{
                    if(celda.edad<1820){
                        if(probabilidad<=0.15){
                            nuevaCelda.estado=BLANCO;
                            nuevaCelda.tiempo_podado=0;
                            nuevaCelda.tiempo_contagio=-1;
                        }else{
                            nuevaCelda.estado=VERDE;
                            nuevaCelda.tiempo_contagio=-1;
                        }
                    }else{
                        if(probabilidad<=0.53){
                            nuevaCelda.estado=VERDE;
                            nuevaCelda.edad=52;
                            nuevaCelda.tiempo_contagio=-1;
                            nuevaCelda.tiempo_podado=-1;
                            nuevaCelda.herida_abierta=0;
                        } else{
                            nuevaCelda.estado=VERDE;
                            nuevaCelda.tiempo_contagio=-1;
                        }
                    }
                }

            } else{
                nuevaCelda.tiempo_contagio++;
            }
            break;
        }
        case BLANCO:{
            if(celda.tiempo_podado==12){
                nuevaCelda.estado=VERDE;
                nuevaCelda.tiempo_podado=-1;
            }else{
                nuevaCelda.tiempo_podado++;
            }
            break;
        }
        case NARANJA:{
            if(celda.tiempo_contagio>3){
                nuevaCelda.estado=ROJO;
            }
            nuevaCelda.tiempo_contagio++;
            break;
        }
        case VERDE:{
            float probabilidad=procesarContagio(((float)vecinosEnfermos/12),susceptibilidad(celda.edad,celda.herida_abierta));
            if(generador_Uniforme(rand(),0,100)<probabilidad){
                nuevaCelda.estado=NARANJA;
                nuevaCelda.tiempo_contagio=0;
            }
            break;
        }
    }
    if(celda.edad<156){
        if(generador_Uniforme(rand(),0,100)<0.23){
            nuevaCelda.herida_abierta=1;
        } else{
            nuevaCelda.herida_abierta=0;
        }
    }else{
        if(celda.edad<1820){
            if(generador_Uniforme(rand(),0,100)<0.08){
                nuevaCelda.herida_abierta=1;
            }else{
                nuevaCelda.herida_abierta=0;
            }
        }else{
            if(generador_Uniforme(rand(),0,100)<0.37){
                nuevaCelda.herida_abierta=1;
            } else{
                nuevaCelda.herida_abierta=0;
            }
        }
    }
    nuevaCelda.edad++;
    return nuevaCelda;
}
void procesarMatriz(Celda** estadoActual,Celda** estadoSiguiente){
    int i,j;
    #pragma omp parallel for private(i) 
    for(i=0; i<n ; i++) {
        #pragma omp for private(j) 
        for ( j = 0; j < n; j++) {

            ///________contardor____________///
            int vecinosEnfermos = 0;
            ///________flags____________///
            int top1 = i + 1 < n;
            int top2 = i + 2 < n;
            int down1 = i - 1 > -1;
            int down2 = i - 2 > -1;
            int right1 = j + 1 < n;
            int right2 = j + 2 < n;
            int left1 = j - 1 > -1;
            int left2 = j - 2 > -1;
            ///________procesamiento____________///
            if (top1) {
                vecinosEnfermos += estadoActual[i + 1][j].estado == ROJO;
                if (top2) {
                    vecinosEnfermos += estadoActual[i + 2][j].estado == ROJO;
                }
            }
            if (right1) {
                vecinosEnfermos += estadoActual[i][j + 1].estado == ROJO;

                if (right2) {
                    vecinosEnfermos += estadoActual[i][j + 2].estado == ROJO;

                }
            }
            if (down1) {
                vecinosEnfermos += estadoActual[i - 1][j].estado == ROJO;

                if (down2) {
                    vecinosEnfermos += estadoActual[i - 2][j].estado == ROJO;

                }
            }
            if (left1) {
                vecinosEnfermos += estadoActual[i][j - 1].estado == ROJO;

                if (left2) {
                    vecinosEnfermos += estadoActual[i][j - 2].estado == ROJO;

                }
            }
            if (top1 && right1) {
                vecinosEnfermos += estadoActual[i + 1][j + 1].estado == ROJO;

            }
            if (top1 && left1) {
                vecinosEnfermos += estadoActual[i + 1][j - 1].estado == ROJO;

            }
            if (down1 && right1) {
                vecinosEnfermos += estadoActual[i - 1][j + 1].estado == ROJO;

            }
            if (down1 && left1) {
                vecinosEnfermos += estadoActual[i - 1][j - 1].estado == ROJO;
            }
            estadoSiguiente[i][j]= procesarCelda(estadoActual[i][j],vecinosEnfermos);
        }
    }
}
/*printf("\033[0m"); //Resets the text to default color
 * Red "\033[0;31m"
 * Green "\033[0;32m"
 * Blue "\033[0;34m"
 * White "\033[0;37m"
 * Yellow "\033[0;33m" no hay naranja jaja*/
void VisualizarMatriz(Celda** matriz) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            char estado='-';
            switch (matriz[i][j].estado) {
                case ROJO:{
                    estado='R';
                    break;
                }
                case AZUL:{
                    estado='A';
                    break;

                }
                case BLANCO:{
                    estado='B';
                    break;

                }
                case NARANJA:{
                    estado='N';
                    break;

                }
                case VERDE:{
                    estado='V';
                    break;
                }
            }
            if(matriz[i][j].herida_abierta){
                printf("|{%c}|",estado);
            }else{
                printf("|[%c]|",estado);
            }
            if(j==n-1){
                printf("\n");
            }
        }
    }
}


int main() {
    srand(time(NULL));
    clock_t start,finish;
    double duracion;
    double tiempo_total=0;
    double promedio;
   /* struct timeval tiempoInicial, tiempoFinal;
    float tiempoTotal;
    float sumaTiempo=0;
    float promedio = 0;*/
    Celda ** Estado_actual = Crear_Matriz();
    Celda ** Estado_siguiente = Crear_Matriz();
    Celda ** Aux;
    /*printf("ESTADO INICIAL:\n");
    VisualizarMatriz(Estado_actual);*/
    int aux=1;
    for(int j = 0; j < CICLOS; j++){
        start=clock();
        init(Estado_actual);
      //  gettimeofday(&tiempoInicial, NULL);
        for(int i = 0;i<SEMANAS;i++){
            procesarMatriz(Estado_actual,Estado_siguiente);
            /*if((i+1)%(SEMANAS/4)==0) {
                printf("_______________semana %d___________________\n",i+1);
                ///VisualizarMatriz(Estado_siguiente);
                printf("Matriz procesada al %d % ",25*aux);
                printf("___________________________________________\n");
                aux++;
            }*/
            ///system("pause");
            ///getchar();

            Aux=Estado_siguiente;
            Estado_siguiente=Estado_actual;
            Estado_actual=Aux;
            ///CopiarDatos3(Estado_actual,Estado_siguiente);
        }
        aux=1;
        finish=clock();
        ///duracion=(double)(finish-start)/ (clock_t)1000; en windowa
        duracion=(double)(finish-start)/ (clock_t)1000000;
        printf("\n---------------------------------------------\n");
        printf("--------------tiempos de la %d ejecucion---------------\n",j+1);
        printf("Tiempo: %lf segundos.\n",duracion);
        printf("-----------------------------------------------\n");
        tiempo_total+=duracion;
        duracion=0;
       /* gettimeofday(&tiempoFinal, NULL);
        //milisengundos
        sumaTiempo = (tiempoFinal.tv_sec - tiempoInicial.tv_sec) * 1000.0 + (tiempoFinal.tv_usec - tiempoInicial.tv_usec) / 1000.0;
        tiempoTotal+= sumaTiempo;
        sumaTiempo = 0;
        printf("\n---------------------------------------------\n");
        printf("--------------tiempos de la %d-----------------\n",j+1);
        printf("Tiempo total: %f milisegundos.\n",tiempoTotal);
        printf("-----------------------------------------------\n");*/
    }
    promedio=tiempo_total/CICLOS;
    printf("\n---------------------------------------------\n");
    printf("------Finalizado ciclo de %d ejecuciones--------\n",CICLOS);
    printf("Tiempo Total: %lf segundos.\n",tiempo_total);
    printf("Tiempo promedio: %lf segundos.\n",promedio);
    printf("-----------------------------------------------\n");
    /*promedio = tiempoTotal / CICLOS;
        printf("\n---------------------------------------------\n");
        printf("------Finalizado ciclo de %d ejecuciones--------\n",CICLOS);
        printf("Tiempo promedio: %f milisegundos.\n",promedio);
        printf("-----------------------------------------------\n");*/


    free((void*)Estado_actual);
    free((void*)Estado_siguiente);
    return 0;
}
