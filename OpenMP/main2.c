#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>
#include <math.h>
#define n 800
#define CICLOS 5
#define SEMANAS 1200

enum Estado {BLANCO=0,AZUL=1,ROJO=2,NARANJA=3,VERDE=4};
typedef struct {
    enum Estado estado;
    int edad;
    int herida_abierta;
    int tiempo_contagio;
    int tiempo_podado;
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


double generador_Uniforme(int random, int a, int b){
    double resultado=((double)(random %(b-a+1) + a)/100.0f);
    return resultado;
}

int generadorUniformeENTEROS(int random,int a, int b) {

    int ret = a + (b - a) * generador_Uniforme(random,0,100);
    return ret;
}




void init(Celda** estadoActual){
    ///Este for se puede paralelizar
    int i;
    int j;
    Celda Celda_auxiliar;
    //#pragma omp parallel for shared(estadoActual) private(i,Celda_auxiliar) num_threads(4)
    //#pragma omp parallel for schedule(static,2) private(i) num_threads(2)
        for(i=0; i<n ; i++) {
            //#pragma omp parallel for shared(estadoActual) private(j,Celda_auxiliar) num_threads(4)
            //#pragma omp parallel for schedule(static,4) private(j) num_threads(4)
            for (j = 0; j < n; j++) {
                double prob = generador_Uniforme(rand(),0,100);
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
                estadoActual[i][j]=Celda_auxiliar;
                /*printf("Estado Aux Estado %d\n",Celda_auxiliar.estado);
                printf("Estado celda %d\n",estadoActual[i][j].estado);*/

            }
        }
    }

double susceptibilidad(int edad,int heridas_A){
    double suscep=0;
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

double procesarContagio(double Porc_vecinosEnf ,double susceptibilidad){
    return ((Porc_vecinosEnf + susceptibilidad)* 0.60) + 0.07;
}



Celda procesarCelda(Celda celda, int vecinosEnfermos){
    Celda nuevaCelda;/*=celda;*/
    nuevaCelda.estado=celda.estado;
    nuevaCelda.edad=celda.edad;
    nuevaCelda.herida_abierta=celda.herida_abierta;
    nuevaCelda.tiempo_contagio=celda.tiempo_contagio;
    nuevaCelda.tiempo_podado=celda.tiempo_podado;

    switch (celda.estado) {
        case ROJO:{
            double probabilidad= generador_Uniforme(rand(),0,100);
            if((celda.tiempo_contagio>4)&&(probabilidad<0.85)){
                nuevaCelda.estado=AZUL;
            }
            nuevaCelda.tiempo_contagio++;
            break;
        }
        case AZUL:{
            if(celda.tiempo_contagio>7){
                double probabilidad= generador_Uniforme(rand(),0,100);
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
            double probabilidad=procesarContagio(((double)vecinosEnfermos/12),susceptibilidad(celda.edad,celda.herida_abierta));
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
    int i;
    int j;
    int div_tareas_1;
    int div2_tareas_2;
    div_tareas_1 = floor(n/3); // Divido en 3 partes siempre
    div2_tareas_2 = floor(n/8); // divido por el numero de threads y le pongo dynamic si alguno de los threahs termina antes.
    ///Este for se puede paralelizar
    //#pragma omp parallel for shared(estadoActual,estadoSiguiente) private(i) collapse(2) num_threads(16)
    //#pragma omp parallel for schedule(static,2) private(i) num_threads(2)
    #pragma omp parallel for schedule(dynamic,div_tareas_1) private(i) num_threads(2)
        for(i=0; i<n ; i++) {
            //#pragma omp parallel for shared(estadoActual,estadoSiguiente) private(j) num_threads(4)
           // #pragma omp parallel for schedule(static,4) private(j) num_threads(4)
            #pragma omp parallel for schedule(dynamic,div2_tareas_2) private(j) num_threads(4)
                for (j = 0; j < n; j++) {
                    if(estadoActual[i][j].estado==VERDE){
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
                        }else{
                            estadoSiguiente[i][j]= procesarCelda(estadoActual[i][j],0);
                        }
                }
         }
    
    }

/*printf("\033[0m"); //Resets the text to default color
 * Red "\033[0;31m"
 * Green "\033[0;32m"
 * Blue "\033[0;34m"
 * White "\033[0;37m"
 * Yellow "\033[0;33m" no hay naranja jaja*/
void VisualizarMatriz(Celda** matriz) { ///EL COLOR SOLO FUNCIONA EN LINUX, EN WINDOWS HAY QUE COMENTAR LOS PRINTS CON CODIGO DE ESCAPE  EJ: \033
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            char estado='-';
            switch (matriz[i][j].estado) {
                case ROJO:{
                    estado='R';
                    printf("\033[0;31m"); 
                    break;
                }
                case AZUL:{
                    estado='A';
                    printf("\033[0;34m"); 
                    break;

                }
                case BLANCO:{
                    estado='B';
                    printf("\033[0;37m"); 
                    break;

                }
                case NARANJA:{
                    estado='N';
                    printf("\033[0;33m"); 
                    break;

                }
                case VERDE:{
                    estado='V';
                    printf("\033[0;32m"); 
                    break;
                }
            }
            if(matriz[i][j].herida_abierta){
                printf("|{%c}|",estado);
            }else{
                printf("|[%c]|",estado);
            }
            printf("\033[0m");//Reestrablecer color
            if(j==n-1){
                printf("\n");
            }
        }
    }
}


int main() {
    
    clock_t start,finish;
    double duracion;
    double tiempo_total=0;
    double promedio;
   /* struct timeval tiempoInicial, tiempoFinal;
    double tiempoTotal;
    double sumaTiempo=0;
    double promedio = 0;*/
    Celda ** Estado_actual = Crear_Matriz();
    Celda ** Estado_siguiente = Crear_Matriz();
    Celda ** Aux;
    double rand_aux;
    srand(time(NULL));
    //omp_set_num_threads(8);
    /*int tid ;
    int nth;
    tid = omp_get_thread_num ();
    nth = omp_get_num_threads();*/
    int aux=1;
    for(int j = 0; j < CICLOS; j++){
        rand_aux=rand();
        srand(((rand()+rand_aux)*13)*7);
        start=clock();
        init(Estado_actual);
        /*printf("ESTADO INICIAL:\n");
        VisualizarMatriz(Estado_actual);*/
        for(int i = 0;i<SEMANAS;i++){
            srand(((rand()+rand_aux)*13)*7);
            procesarMatriz(Estado_actual,Estado_siguiente);
            /*if((i+1)%(SEMANAS/4)==0) {
                printf("_______________semana %d___________________\n",i+1);
                VisualizarMatriz(Estado_siguiente);
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
        ///duracion=(double)(finish-start)/ (clock_t)1000; en windows
        duracion=(double)(finish-start)/ (clock_t)1000000;
        printf("\n---------------------------------------------\n");
        printf("--------------tiempos de la %d ejecucion---------------\n",j+1);
        printf("Tiempo: %lf segundos.\n",duracion);
        printf("-----------------------------------------------\n");
        tiempo_total+=duracion;
        duracion=0;

    }
    promedio=tiempo_total/CICLOS;
    printf("\n---------------------------------------------\n");
    printf("------Finalizado ciclo de %d ejecuciones--------\n",CICLOS);
    printf("Tiempo Total: %lf segundos.\n",tiempo_total);
    printf("Tiempo promedio: %lf segundos.\n",promedio);
    printf("-----------------------------------------------\n");

    free((void*)Estado_actual);
    free((void*)Estado_siguiente);
    return 0;
}
