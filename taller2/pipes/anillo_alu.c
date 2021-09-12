/*
Implementar un proceso que haga un esquema de comunicacion en forma de anillo para intercomunicar
sus procesos hijos. Existen al menos tres procesos conectados formando un lazo cerrado.
Cada uno se comunica con dos procesos: su antecesor y su sucesor (por orden de creacion de los mismos).
Esta se hace a traves de PIPES.
El padre envia a alguno de los procesos hijo del anillo un entero como mensaje y genera un numero secreto.
Debe reenviar el numero del mensaje al siguiente proceso en el anillo el cual lo incrementa en uno y lo 
envia al proximo proceso en el anillo. Esto se repite hasta que el proceso que inicio la comunicacion 
reciba, del proceso anterior, un numero que sea mayor o igual a su numero secreto.
El programa inicial crea un conjunto de procesos hijos, a los que organiza para formar el anillo. Cuando se 
llega al proceso incial habra una nueva ronda en el anillo si el numero recibido no supera su numero secreto.
Sino no enviara al proceso padre el ultimo valor obtenido. El padre tambien debera mostrar en la salida 
estandar el resultado final del proceso de comunicacion.
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int s;
int* child_s_prev_pipe;

void create_pipe_for(int* file_descriptor) {
    if (pipe(file_descriptor) == -1) {
		perror("pipe");
		exit(1);
    }
}

int read_from(int* storage) {
    int data; 
    if (read(storage[0], &data, sizeof(int)) < 1) {
        perror("read");
        return -1;
    }
    return data;
}

void write_on(int* storage, int new_data, char* message) {
	printf(message);
    if (write(storage[1], &new_data, sizeof(int)) != sizeof(int)) {
		perror("write");
        exit(1);
    }
}

void child_code(int prev_pipe_read, int prev_pipe_write, int next_pipe_read, int next_pipe_write, int process_number, bool am_i_the_chosen_one) {
	int prev_pipe[2];
	int next_pipe[2];
	prev_pipe[0] =prev_pipe_read;
	prev_pipe[1] = prev_pipe_write;
	next_pipe[0] = next_pipe_read;
	next_pipe[1] = next_pipe_write;
	int secret;
	if(am_i_the_chosen_one) {
		secret = rand() % 100; //Crea numero secreto
		printf("El secret es: %d \n", secret);
	} 
	// else {
	// 	//cerrar la escritura del prev
	// 	close(prev_pipe[1]);
	// }
	//cerrar la lectura del next
	// close(next_pipe[0]);

	while(true) {
		int c = read_from(prev_pipe);
		if(c == -1) {
			printf("Soy el proceso %d y estoy abortando.\n", process_number);
			// La lectura del pipe fallo, esto se debe a que el proceso s terminó. 
			close(next_pipe[1]);
			close(prev_pipe[0]);
			exit(0);
		}
		printf("Soy el proceso %d c vale: %d \n", process_number, c);
		if(am_i_the_chosen_one & secret < c) {
			printf("Soy el proceso %d y estoy terminando mi ejecucion.\n", process_number);
			//Terminar y avisarle al padre.
			write_on(prev_pipe, c, "el write final, iamthechosenone");
			close(next_pipe[1]);
			close(prev_pipe[0]);
			exit(0);
		} 
		char buffer[1024];
		snprintf(buffer, sizeof(buffer), "el write del proceso %d\n", process_number);
		sleep(1);
		write_on(next_pipe, c+1, buffer);
	}
}

void create_and_link(int* prev_pipe, int* next_pipe, int i, int* child_pids) {
	child_pids[i] = fork();
	if(child_pids[i] == 0) {
		child_code(prev_pipe[0], prev_pipe[1], next_pipe[0], next_pipe[1], i, i==s);
	} else {
		if(i == s) {
			child_s_prev_pipe = &(prev_pipe[0]);
		} else {
			// close(prev_pipe[0]); 
			// close(prev_pipe[1]);
		}
		prev_pipe[0] = next_pipe[0];
		prev_pipe[1] = next_pipe[1];
	}
}

int main(int argc, char **argv)
{	
	int status, pid;
	int buffer[1];

	if (argc != 4){ 
		printf("Uso: anillo <n> <c> <s> \n"); exit(0);
	}
	int n = atoi(argv[1]);
	int c = atoi(argv[2]);
	s = atoi(argv[3]);

	if(s > n) {
		printf("Uso: n > s \n"); exit(0);
	}
	
	int child_pids[n];
	int* first_pipe;

	printf("N: %d, C: %d, S: %d\n", n, c, s);
    
  	/* COMPLETAR */

    int prev_pipe[2];
	create_pipe_for(prev_pipe);
	first_pipe = &(prev_pipe[0]);

    for (int i = 0; i < n-1; i++) {
		//Invariante: prev_pipe es el pipe que conecta al proceso i-1 con el i
		//            next_pipe es el pipe que conecta al proceso i con el i+1

		// Creamos el pipe que conecta al proceso i con el i+1
		int next_pipe[2];
		create_pipe_for(next_pipe);

		create_and_link(prev_pipe, next_pipe, i, child_pids);
	}
	create_and_link(prev_pipe, first_pipe, n-1, child_pids);
	
	// A partir de acá este código sólo es ejecutado por el padre.
    // Emulo ser el proceso s-1 y le hablo al s mediante su pipe (el pipe que interconecta s-1 y s)

	// Esto es para que el child S arranque.
	sleep(1);
	write_on(child_s_prev_pipe, c, "El write del parent\n");

	waitpid(child_pids[s], &status, 0);

	c = read_from(child_s_prev_pipe);
	close(child_s_prev_pipe[0]);
	close(child_s_prev_pipe[1]);
	printf("El c que termino todo fue: %d\n", c);

}
