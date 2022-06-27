#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <mpi.h>
#include <math.h>
#include <omp.h>
///#include <unistd.h>


#define N 800
#define CICLOS 5
#define SEMANAS 1200

#define BLANCO 0
#define AZUL 1
#define ROJO 2
#define NARANJA 3
#define VERDE 4

///enum Estado {BLANCO=0,AZUL=1,ROJO=2,NARANJA=3,VERDE=4};
typedef struct {
    int estado;
    int edad;
    int herida_abierta;
    int tiempo_contagio;
    int tiempo_podado;
}Celda;



Celda ** Crear_Matriz(int filas, int n){
    Celda **Matriz;
    Matriz= (Celda **)malloc (filas*sizeof(Celda *));
    Celda *Mf;
    Mf = (Celda *) malloc(filas*N*sizeof(Celda));

    /*for (int i=0;i<n;i++)
        Matriz[i] = (Celda *) malloc (n*sizeof(Celda));*/
    for (int i=0; i<filas; i++) {
        Matriz[i] = Mf + i*N;
    }
    return Matriz;
}

/*
void CopiarDatos(Celda** estado_viejo, Celda** estado_actual){
    for(int i=0; i<n ; i++) {
        for (int j = 0; j < N; j++) {
            estado_viejo[i][j]=estado_actual[i][j];
        }
    }
}void CopiarDatos2(Celda** estado_viejo, Celda** estado_actual){
  **estado_viejo=**estado_actual;
}
void CopiarDatos3(Celda** estado_viejo, Celda** estado_actual){
        memcpy((*estado_viejo), (*estado_actual), sizeof(Celda)*n*n);
}*/

double generador_Uniforme(int random, int a, int b){
    double resultado=((double)(random %(b-a+1) + a)/100.0f);
    return resultado;
}

int generadorUniformeENTEROS(int random,int a, int b) {

    int ret = a + (b - a) * generador_Uniforme(random,0,100);
    return ret;
}

void set_seed_random(int id_process){
srand(time(NULL));
srand(rand()+(id_process+7)*13);
}




void init(Celda** estadoActual , int inicio, int final){
    ///srand(time(NULL));
    Celda Celda_auxiliar;
    int i;
    int j;
    int div_tareas_1;
    int div2_tareas_2;
    div_tareas_1 = floor(N/3); // Divido en 3 partes siempre
    div2_tareas_2 = floor(N/8); // divido por el numero de threads y le pongo dynamic si alguno de los threahs termina antes.
    //#pragma omp parallel for schedule(dynamic,div_tareas_1) private(i) num_threads(2)
    for(i=inicio; i<final ; i++) {
        //#pragma omp parallel for schedule(dynamic,div2_tareas_2) private(j) num_threads(8)
            for (j = 0; j < N; j++) {
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
                prob=generador_Uniforme(rand(),0,100);

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


MPI_Datatype generarTipo() {
    MPI_Datatype nuevo_tipo_celda;
    int longitud[5] = {1,1,1,1,1};
    MPI_Aint desplazamiento[5];
    Celda celda_modelo;
    MPI_Aint direccion_base;
    MPI_Get_address(&celda_modelo, &direccion_base);
    MPI_Get_address(&celda_modelo.estado, &desplazamiento[0]);
    MPI_Get_address(&celda_modelo.edad, &desplazamiento[1]);
    MPI_Get_address(&celda_modelo.herida_abierta, &desplazamiento[2]);
    MPI_Get_address(&celda_modelo.tiempo_contagio, &desplazamiento[3]);
    MPI_Get_address(&celda_modelo.tiempo_podado, &desplazamiento[4]);

    desplazamiento[0] = MPI_Aint_diff(desplazamiento[0], direccion_base);
    desplazamiento[1] = MPI_Aint_diff(desplazamiento[1], direccion_base);
    desplazamiento[2] = MPI_Aint_diff(desplazamiento[2], direccion_base);
    desplazamiento[3] = MPI_Aint_diff(desplazamiento[3], direccion_base);
    desplazamiento[4] = MPI_Aint_diff(desplazamiento[4], direccion_base);

    MPI_Datatype tipos[5] = {MPI_INT,MPI_INT, MPI_INT, MPI_INT, MPI_INT};
    MPI_Type_create_struct(5, longitud, desplazamiento, tipos, &nuevo_tipo_celda);
    MPI_Type_commit(&nuevo_tipo_celda);
	
	return nuevo_tipo_celda;
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
    Celda nuevaCelda;
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
                    }else{
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
void procesarMatriz(Celda** estadoActual,Celda** estadoSiguiente,int inicio, int final, int limite){
    int i;
    int j;
    int div_tareas_1;
    int div2_tareas_2;
    div_tareas_1 = floor(N/3); // Divido en 3 partes siempre
    div2_tareas_2 = floor(N/8); // divido por el numero de threads y le pongo dynamic si alguno de los threahs termina antes.
    //#pragma omp parallel for shared(estadoActual,estadoSiguiente) private(i,j) num_threads(3)
    #pragma omp parallel for schedule(dynamic,div_tareas_1) private(i) num_threads(2)
        for(i=inicio; i<final ; i++) {
            //#pragma omp parallel for shared(estadoActual,estadoSiguiente) private(j) num_threads(2)
            #pragma omp parallel for schedule(dynamic,div2_tareas_2) private(j) num_threads(8)
                for (j = 0; j < N; j++) {
                    if(estadoActual[i][j].estado==VERDE){


                        ///________contardor____________///
                        int vecinosEnfermos = 0;
                        ///________flags____________///
                        int top1 = i + 1 < limite;
                        int top2 = i + 2 < limite;
                        int down1 = i - 1 > -1;
                        int down2 = i - 2 > -1;
                        int right1 = j + 1 < N;
                        int right2 = j + 2 < N;
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
void VisualizarMatriz(Celda** matriz,int inicio, int final) { ///EL COLOR SOLO FUNCIONA EN LINUX, EN WINDOWS HAY QUE COMENTAR LOS PRINTS CON CODIGO DE ESCAPE  EJ: \033
    for (int i = inicio; i < final; i++) {
        for (int j = 0; j < N; j++) {
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
            if(j==N-1){
                printf("\n");
            }
        }
    }
    ///usleep(200000);
}




int main(int argc, char **argv) {
    int id_process;    
    int num_process;
    MPI_Status status;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&id_process);
    MPI_Comm_size(MPI_COMM_WORLD,&num_process);
    MPI_Datatype nuevo_tipo_celda = generarTipo();
    clock_t start,finish;
    double duracion;
    double tiempo_total=0;
    double promedio;
    Celda ** SubMatriz_actual;
    Celda ** SubMatriz_siguiente;
    Celda ** Aux;
    ///srand(time(NULL));
    int div=floor(N/num_process);
    int aux=1;
    /// Pido memoria para las submatrices
    if(id_process==0){
                 SubMatriz_actual = Crear_Matriz(div+2,N);
                 SubMatriz_siguiente = Crear_Matriz(div+2,N);
            }
        else{
            if(id_process==num_process-1){
                 //pedir memoria
                 SubMatriz_actual = Crear_Matriz(div+2,N);
                 SubMatriz_siguiente = Crear_Matriz(div+2,N);                
                }
                else{
                //pedir memoria
                 SubMatriz_actual = Crear_Matriz(div+4,N);
                 SubMatriz_siguiente = Crear_Matriz(div+4,N);   
                }
            }



    for(int j = 0; j < CICLOS; j++){
        set_seed_random(id_process);
        

        ///  inicializo las matrices
        if(id_process==0){
                 ///printf("division = %d\n",div);
                 start=clock();
                //inicializar
                 init(SubMatriz_actual,0,div);
                //muestro
                //printf("\nESTADO INICIAL del Proceso %d:\n",id_process);
                //VisualizarMatriz(SubMatriz_actual,0,div);
            }
        else{
            
                //inicializar
                 init(SubMatriz_actual,2,div+2);
                //muestro
                //printf("\nESTADO INICIAL del Proceso %d:\n",id_process);
                //VisualizarMatriz(SubMatriz_actual,2,div+2);               
                
            }
            /// Enviar y recibir mensajes, y procesar submatrices


        for(int i = 0;i<SEMANAS;i++){
            ///enviar y recibir mensajes:

            ///si es par envia primero y desp recibe
        	    // Envío y recepción si el nodo es par y vice versa para impar (esto es por que se puede producir interbloqueo)
        	    if (id_process % 2 == 0) {
        	    	if (id_process == 0) {
                        //printf("soy el proceso %d y estoy por procesar\n",id_process);
        				MPI_Send(&(SubMatriz_actual[div-2][0]), N, nuevo_tipo_celda, id_process+1, 0, MPI_COMM_WORLD);
        				MPI_Send(&(SubMatriz_actual[div-1][0]), N, nuevo_tipo_celda, id_process+1, 0, MPI_COMM_WORLD);
        				MPI_Recv(&(SubMatriz_actual[div][0]), N, nuevo_tipo_celda, id_process+1, 0, MPI_COMM_WORLD, &status);
        				MPI_Recv(&(SubMatriz_actual[div+1][0]), N, nuevo_tipo_celda, id_process+1, 0, MPI_COMM_WORLD, &status);
                        procesarMatriz(SubMatriz_actual,SubMatriz_siguiente,0,div,div+2);
                        //printf("soy el proceso %d y ya procese\n",id_process);
        	    	} else {
        	        	if (id_process==num_process-1) {
                            //printf("soy el proceso %d y estoy por procesar\n",id_process);
        	        		MPI_Send(&(SubMatriz_actual[2][0]), N, nuevo_tipo_celda, id_process-1, 0, MPI_COMM_WORLD);
        	        		MPI_Send(&(SubMatriz_actual[3][0]), N, nuevo_tipo_celda, id_process-1, 0, MPI_COMM_WORLD);
        				    MPI_Recv(&(SubMatriz_actual[0][0]), N, nuevo_tipo_celda, id_process-1, 0, MPI_COMM_WORLD, &status);
        				    MPI_Recv(&(SubMatriz_actual[1][0]), N, nuevo_tipo_celda, id_process-1, 0, MPI_COMM_WORLD, &status);
                            procesarMatriz(SubMatriz_actual,SubMatriz_siguiente,2,div+2,div+2);
                            //printf("soy el proceso %d y ya procese\n",id_process);
        	        	} else{
                            //printf("soy el proceso %d y estoy por procesar\n",id_process);
        				    MPI_Send(&(SubMatriz_actual[2][0]), N, nuevo_tipo_celda, id_process-1, 0, MPI_COMM_WORLD);
        				    MPI_Send(&(SubMatriz_actual[3][0]), N, nuevo_tipo_celda, id_process-1, 0, MPI_COMM_WORLD);
        				    MPI_Send(&(SubMatriz_actual[div][0]), N, nuevo_tipo_celda, id_process+1, 0, MPI_COMM_WORLD);
        				    MPI_Send(&(SubMatriz_actual[div+1][0]), N, nuevo_tipo_celda, id_process+1, 0, MPI_COMM_WORLD);
        	        		MPI_Recv(&(SubMatriz_actual[0][0]), N, nuevo_tipo_celda, id_process-1,0, MPI_COMM_WORLD, &status);
        				    MPI_Recv(&(SubMatriz_actual[1][0]), N, nuevo_tipo_celda, id_process-1, 0, MPI_COMM_WORLD, &status);
        	        		MPI_Recv(&(SubMatriz_actual[div+2][0]), N, nuevo_tipo_celda, id_process+1, 0, MPI_COMM_WORLD, &status);
        				    MPI_Recv(&(SubMatriz_actual[div+3][0]), N, nuevo_tipo_celda, id_process+1, 0, MPI_COMM_WORLD, &status);
                            procesarMatriz(SubMatriz_actual,SubMatriz_siguiente,2,div+2,div+4);
                            //printf("soy el proceso %d y ya procese\n",id_process);
                        }
        	    	}    
        	    } else {
        	    	if (id_process==num_process-1) {
                        //printf("soy el proceso %d y estoy por procesar\n",id_process);
        				MPI_Recv(&(SubMatriz_actual[0][0]), N, nuevo_tipo_celda, id_process-1, 0, MPI_COMM_WORLD, &status);
        				MPI_Recv(&(SubMatriz_actual[1][0]), N, nuevo_tipo_celda, id_process-1, 0, MPI_COMM_WORLD, &status);
        	    		MPI_Send(&(SubMatriz_actual[2][0]), N, nuevo_tipo_celda, id_process-1, 0, MPI_COMM_WORLD);
        	    		MPI_Send(&(SubMatriz_actual[3][0]), N, nuevo_tipo_celda, id_process-1, 0, MPI_COMM_WORLD);
                        procesarMatriz(SubMatriz_actual,SubMatriz_siguiente,2,div+2,div+2);
                        //printf("soy el proceso %d y ya procese\n",id_process);
        	    	} else {
                            //printf("soy el proceso %d y estoy por procesar\n",id_process);
        	        		MPI_Recv(&(SubMatriz_actual[0][0]), N, nuevo_tipo_celda, id_process-1, 0, MPI_COMM_WORLD, &status);
        				    MPI_Recv(&(SubMatriz_actual[1][0]), N, nuevo_tipo_celda, id_process-1, 0, MPI_COMM_WORLD, &status);
        	        		MPI_Recv(&(SubMatriz_actual[div+2][0]), N, nuevo_tipo_celda, id_process+1, 0, MPI_COMM_WORLD, &status);
        				    MPI_Recv(&(SubMatriz_actual[div+3][0]), N, nuevo_tipo_celda, id_process+1, 0, MPI_COMM_WORLD, &status);
        	        		MPI_Send(&(SubMatriz_actual[2][0]), N, nuevo_tipo_celda, id_process-1, 0, MPI_COMM_WORLD);
        				    MPI_Send(&(SubMatriz_actual[3][0]), N, nuevo_tipo_celda, id_process-1, 0, MPI_COMM_WORLD);
        	        		MPI_Send(&(SubMatriz_actual[div][0]), N, nuevo_tipo_celda, id_process+1, 0, MPI_COMM_WORLD);
        				    MPI_Send(&(SubMatriz_actual[div+1][0]), N, nuevo_tipo_celda, id_process+1, 0, MPI_COMM_WORLD);
                            procesarMatriz(SubMatriz_actual,SubMatriz_siguiente,2,div+2,div+4);
                            //printf("soy el proceso %d y ya procese\n",id_process);
        	    	}   
        	    }



                   /* if(id_process==0){
                        printf("\nESTADO de la submatriz del Proceso %d:\n",id_process);
                       
                        VisualizarMatriz(SubMatriz_actual,0,div);
                    }
                    else{
                        if(id_process==num_process-1){
                            printf("\nESTADO de la submatriz del Proceso %d:\n",id_process);
                            VisualizarMatriz(SubMatriz_actual,2,div+2);
                            }
                            else{
                            printf("\nESTADO de la submatriz del Proceso %d:\n",id_process);
                            
                            VisualizarMatriz(SubMatriz_actual,2,div+2);
                            }
                        }*/
                    
                    
                  /* if((i+1)%(SEMANAS/4)==0) {
                        printf("_______________semana %d___________________\n",i+1);
                        VisualizarMatriz(SubMatriz_actual);
                        printf("Matriz procesada al %d porciento ",25*aux);
                        printf("___________________________________________\n");
                        aux++;
                    }*/
                    ///system("pause");
                    ///getchar();

                    Aux=SubMatriz_siguiente;
                    SubMatriz_siguiente=SubMatriz_actual;
                    SubMatriz_actual=Aux;
                    ///CopiarDatos3(Estado_actual,Estado_siguiente);
        }
        if(id_process==0){
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
        MPI_Barrier(MPI_COMM_WORLD);
   
    }

    if(id_process==0){
        promedio=tiempo_total/CICLOS;
        printf("\n---------------------------------------------\n");
        printf("------Finalizado ciclo de %d ejecuciones--------\n",CICLOS);
        printf("Tiempo Total: %lf segundos.\n",tiempo_total);
        printf("Tiempo promedio: %lf segundos.\n",promedio);
        printf("-----------------------------------------------\n");
    }
    MPI_Barrier(MPI_COMM_WORLD);

    free((void*)SubMatriz_actual);
    free((void*)SubMatriz_siguiente);
    MPI_Finalize();
    return 0;
}
