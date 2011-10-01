#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define TAM_BUF 1048576

typedef struct _nodo {
  char caract;
  unsigned int freq;
  struct _nodo *sig;
  struct _nodo *izq;
  struct _nodo *der;
} nodo_t;

typedef struct _tabla {
  char caract;
  unsigned int freq;
  unsigned long int bits;
  unsigned int n_bits;
  struct _tabla *sig;
} tabla_t;

bool add_n_list(nodo_t **arbol, char, int, nodo_t **);
void ordena_arbol(nodo_t **, bool, bool);
void genera_ramas(nodo_t **);
void borra_arbol(nodo_t *);
void borra_tabla(tabla_t *) ;
void genera_tabla(tabla_t **, nodo_t *);
void add_n_tabla(tabla_t **, nodo_t *);
void genera_codigo(nodo_t *, tabla_t *, unsigned long int, unsigned int);
void asigna_codigo(tabla_t *, char, unsigned long int, unsigned int);
void graba_tabla(FILE *, tabla_t *);
void graba_fichero(FILE *, FILE *, char *, tabla_t **);
void descomprime(FILE *, FILE *, nodo_t *, unsigned int, char *);
void indexa_tabla (tabla_t **index, tabla_t *tabla);



int main(int argc, char *argv[]) {
	
  nodo_t *arbol;
  tabla_t *tabla;
  char c;
  unsigned int btotales, nlist, i, freq, leidos;
	
  FILE *arch_ent, *arch_sal;
  char *buffer_arch;
	
  // Los index aceleran la ejecución del programa
  // El último elemento apunta al último elemento de la lista
  tabla_t **index;
  nodo_t **index_n;
	
  // Reservamos memoria
  index = (tabla_t **)calloc(257, sizeof(tabla_t *));
  index_n = (nodo_t **)calloc(257, sizeof(nodo_t *));
	
  if (!index || !index_n) {
    fprintf(stderr, "Error de memoria\n\n");
    exit(1);
  }
		
	
  arbol = NULL;
  tabla = NULL;
	
  // Apuntamos a null cada elemento de index
  for (i=0;i<257;i++) {
    *(index+i) = NULL;
    *(index_n+i) = NULL;
  }
	
  if (!(buffer_arch = (char *)malloc(TAM_BUF)))
    exit (1);
	
	
  
  if ((argc==4) && (strcmp(argv[2],argv[3]) != 0)) {
    if ((arch_ent = fopen(argv[2], "rb"))) {
      // Comprimimos
      if (strcmp(argv[1], "-c") == 0) {
	btotales = 0;
	nlist = 0;
	// Usamos un fread porque fgetc se salta
	// caracteres en ficheros binarios
	do {
	  leidos = fread(buffer_arch, sizeof(char), TAM_BUF, arch_ent);
	  for (i=0;i<leidos;i++) {
	    if (add_n_list(&arbol, *(buffer_arch+i), 1, index_n))
	      nlist++;
	    btotales++;
	  }
					
	} while (leidos>0);
				
	fclose(arch_ent);
		
	if (arbol) {
	  // La tabla se genera de mayor a menor frecuencia
	  // para mejorar los tiempo de compresión
	  ordena_arbol(&arbol, false, true);
	  genera_tabla(&tabla,arbol);
	  // El arbol en cambio debe estar de menor a mayor
	  ordena_arbol(&arbol, false, false);
					
	  // Generamos las ramas del arbol
	  while (arbol->sig)
	    genera_ramas(&arbol);
	  // Asignamos un código a cada caracter de la tabla
	  genera_codigo(arbol, tabla, 0, 0);
	  //tam_tabla = imprime_tabla(tabla);
	  arch_ent = fopen(argv[2], "rb");
	  arch_sal = fopen(argv[3], "wb");
					
	  // Indexamos la tabla
	  indexa_tabla(index, tabla);
					
	  // Grabamos el número de nodos
	  fwrite(&nlist, sizeof(unsigned int), 1, arch_sal);
	  // Grabamos los bytes totales a grabar
	  fwrite(&btotales, sizeof(unsigned int), 1, arch_sal);
					
	  graba_tabla(arch_sal, tabla);
	  graba_fichero(arch_ent, arch_sal, buffer_arch, index);
	  fclose(arch_ent);
	  fclose(arch_sal);
				
	} else
	  fprintf(stderr, "El fichero a comprimir está vacío.\n\n");
	// Descomprimimos
      } else if (strcmp(argv[1], "-d") == 0) {
	fread(&nlist, sizeof(unsigned int), 1, arch_ent);
	fread(&btotales, sizeof(unsigned int), 1, arch_ent);
	if  (nlist <=256) {
	  for (i=0;i<nlist;i++) {
	    fread(&c, sizeof(char), 1, arch_ent);
	    fread(&freq, sizeof(unsigned int), 1, arch_ent);
	    add_n_list(&arbol, c , freq, index_n);
	  }
	}
				
	if (arbol) {
	  ordena_arbol(&arbol, false, false);
			
	  while (arbol->sig)
	    genera_ramas(&arbol);
			
	  arch_sal = fopen(argv[3], "wb");
					
	  descomprime(arch_ent, arch_sal, arbol, btotales,  buffer_arch);
	  fclose(arch_sal);
	} else
	  printf("Nada que descomprimir...\n\n");
				
	fclose(arch_ent);
				

			
      } else
	printf("Uso: huffpacman [-d|-c] fichero_origen fichero_destino\n\n");
    } else
      fprintf(stderr, "Error abriendo el archivo...\n\n");
			
  } else
    printf("Uso: huffpacman [-d|-c] fichero_origen fichero_destino\n\n");
	
  if (arbol) {
    borra_arbol(arbol);
    arbol = NULL;
  }
	
  if (tabla) {
    borra_tabla(tabla);
    tabla = NULL;
  }
	
  if (index) {
    free(index);
    index = NULL;
  }
	
  if (index_n) {
    free(index_n);
    index_n = NULL;
  }
	
  if (buffer_arch) {
    free(buffer_arch);
    buffer_arch = NULL;
  }
	
  return 0;
}

// Devuelve si es un elemento nuevo
bool add_n_list(nodo_t **arbol, char c, int freq, nodo_t **index)  {
	
  bool nuevo = false;
	
  nodo_t *nuevonodo, *aux;
	
  aux = *arbol;
	
  aux = *(index+(unsigned char)c);
  if (!aux)
    aux = *(index+256);
		
  if (aux&&aux->caract == c)
    aux->freq++;
  else {
    nuevo = true;
    if (!aux) {
      *arbol = (nodo_t *)malloc(sizeof(nodo_t));
      nuevonodo = *arbol;
    } else
      nuevonodo = (nodo_t *)malloc(sizeof(nodo_t));
			
    nuevonodo->sig = nuevonodo->izq = nuevonodo->der = NULL;
    nuevonodo->caract = c;
    nuevonodo->freq = freq;
    if (aux)
      aux->sig = nuevonodo;
			
    *(index+(unsigned char)c) = *(index+256) = nuevonodo;
		
  }
	
  return nuevo;
	
}

// Uno si coloca solo el primer elemento
// mayor si es de mayor a menor
void ordena_arbol(nodo_t **arbol, bool uno, bool mayor) {
	
  nodo_t *aux, *aux_sig, *aux_ant;
  int elem, i, x;
  bool colocado;
	
  aux = *arbol;
	
  // contamos los nodos.
  elem = 0;
  while (aux) {
    elem++;
    aux = aux->sig;
  }
		
	
  for (x=0;x<elem-1;x++) {
    aux = aux_ant = *arbol;
    aux_sig = aux->sig;
    for (i=x;i<elem-1;i++) {
      colocado=true;
      if (!mayor) {
	if (aux->freq > aux_sig->freq)
	  colocado = false;
      } else {
	if (aux->freq < aux_sig->freq)
	  colocado=false;
      }
      if (!colocado) {
					
	if (i==x) {
	  // Estamos en el primer elemento
	  *arbol = aux_sig;
	} else
	  aux_ant->sig = aux_sig;
				
	aux->sig = aux_sig->sig;
	aux_sig->sig = aux;

	aux_ant = aux_sig;
	aux_sig = aux->sig;
				
      } else {
	aux_ant = aux;
	aux = aux->sig;
	aux_sig = aux_sig->sig;
      }
      if (uno)
	x=elem;
    }
  } 
}


void genera_ramas(nodo_t **arbol) {
	
  nodo_t *aux, *nuevonodo;
	
  aux = *arbol;
		
  nuevonodo = (nodo_t *)malloc(sizeof(nodo_t));
  *arbol = nuevonodo;
	
  nuevonodo->izq = aux;
  nuevonodo->der = aux->sig;
  nuevonodo->sig = aux->sig->sig;
  nuevonodo->freq = nuevonodo->izq->freq;
  nuevonodo->freq += nuevonodo->der->freq;
  aux->sig = aux->sig->sig = NULL;

  ordena_arbol(arbol, true, false);
}

void genera_tabla(tabla_t **tabla, nodo_t *arbol) {
	
  while (arbol) {
    add_n_tabla(tabla, arbol);
    arbol = arbol->sig;
  }
	
}

void add_n_tabla(tabla_t **tabla, nodo_t *arbol) {
	
  tabla_t *nuevoelem, *aux;
	
  aux = *tabla;
	
  while (aux&&aux->sig)
    aux = aux->sig;
		
  if (!aux) {
    *tabla = (tabla_t *)malloc(sizeof(nodo_t));
    nuevoelem = *tabla;
  } else
    nuevoelem = (tabla_t *)malloc(sizeof(nodo_t));
			
  nuevoelem->sig = NULL;
  nuevoelem->bits = nuevoelem->n_bits = 0;
  nuevoelem->caract = arbol->caract;
  nuevoelem->freq = arbol->freq;
  if (aux) 
    aux->sig = nuevoelem;
}


// Genera los códigos recursivamente
void genera_codigo(nodo_t *arbol, tabla_t *tabla, unsigned long int num, unsigned int iter) {
	
  if (arbol->izq)
    genera_codigo(arbol->izq, tabla, num<<1, iter+1);

  if (arbol->der)
    genera_codigo(arbol->der, tabla, (num<<1)+1, iter+1);
		
  // Si se cumple hemos llegado al final de una rama
  if (!(arbol->izq)&&!(arbol->der))
    asigna_codigo(tabla, arbol->caract, num, iter);
	
}

void asigna_codigo(tabla_t *tabla, char c, unsigned long int num, unsigned int iter) {
	
  while(tabla&&tabla->caract != c)
    tabla = tabla->sig;
		
  tabla->bits = num;
  tabla->n_bits = iter;
	
}


// Borrar recursivamente
void borra_arbol (nodo_t *arbol) {
	
  if (arbol->izq)
    borra_arbol(arbol->izq);
  if (arbol->der)
    borra_arbol(arbol->der);
		
  free(arbol);
  arbol = NULL;
	
}

void borra_tabla(tabla_t *tabla) {
	
  tabla_t *aux;
	
  while (tabla) {
    aux = tabla;
    tabla = tabla->sig;
    free(aux);
  }
	
  aux = tabla;
	
}

void graba_tabla(FILE *arch_sal, tabla_t *tabla) {
	
  while (tabla) {
    fwrite(&tabla->caract, sizeof(char), 1, arch_sal);
    fwrite(&tabla->freq, sizeof(unsigned int), 1, arch_sal);
    tabla = tabla->sig;
  }
}

void graba_fichero(FILE *arch_ent, FILE *arch_sal, char *buffer_out, tabla_t **index) {
	
  int n_bits, grabados, i, leidos;
  unsigned long int buffer;
  tabla_t *aux;
  //char c;
  char *buffer_in;
	
  buffer = 0;
  n_bits = grabados = 0;
	
  if (!(buffer_in = (char *)malloc(TAM_BUF)))
    exit (1);
	
  do {
    leidos = fread(buffer_in, sizeof(char), TAM_BUF, arch_ent);
		
    for (i=0;i<leidos;i++) {
			
      aux = *(index+(unsigned char)*(buffer_in+i));
			
      // Vamos grabando los 8 bits de la izquierda
      // según se va llenando el buffer
      while((n_bits + aux->n_bits > (sizeof(unsigned int)*8)) && grabados < TAM_BUF) {
	*(buffer_out+grabados) = buffer >> (n_bits-8);
	grabados++;
	n_bits -= 8;
      }
      if (grabados==TAM_BUF) {
	fwrite(buffer_out, sizeof(char), grabados, arch_sal);
	grabados=0;
			
      }
      // Hacemos hueco e insertamos los nuevos bits
      if (!(n_bits + aux->n_bits > (sizeof(unsigned int)*8))) {
	buffer <<= aux->n_bits;
	buffer |= aux->bits;
	n_bits += aux->n_bits;
	// Si no hemos entrado en el if es porque
	// se salió del while porque se llenó el buffer
      } else
	i--;
    }
  } while (leidos>0);
	
  fwrite(buffer_out, sizeof(char), grabados, arch_sal);
  // Extraigo los bits que quedan en el buffer
  while(n_bits>0) {
    if(n_bits>=8)
      *(buffer_out) = buffer >> (n_bits-8);
    else
      *(buffer_out) = buffer << (8-n_bits);
    fwrite(buffer_out, sizeof(char), 1, arch_sal);
    n_bits -= 8;
      
  }
	
  if (buffer_in) {
    free (buffer_in);
    buffer_in = NULL;
  }
		
}

void descomprime(FILE *arch_ent, FILE *arch_sal, nodo_t *arbol, unsigned int btotales, char *buffer_in) {
	
  char *buffer_sal;
  char c;
  nodo_t *aux;
  int b, i, leidos, grabados;
	
  if (!(buffer_sal = (char *)malloc(TAM_BUF))) {
    printf("Error de memoria\n\n");
    exit (1);
  }
	
  b = 8;
  grabados = 0;
  aux = arbol;
	
  // Vamos leyendo bit a bit
  do {
    leidos = (fread(buffer_in, sizeof(char), TAM_BUF, arch_ent));
    i=0;
		
    while(i<leidos||b<8) {
      // Almacenamos en el buffer 8 nuevos bits
      if (b == 8) {
	c = *(buffer_in+i);
	b = 0;
	i++;
      }
		
      // Hacemos un AND para leer el bit de mayor peso
      if (c & 0x80)
	aux = aux->der;
      else
	aux = aux->izq;
			
      // Desplazamos un bit a la izquierda
      c <<= 1;
      b++;
		
      // Hemos llegado a un caracter en el árbol
      if(!aux->der && !aux->izq) {
	if (btotales>0) {
	  *(buffer_sal+grabados) = aux->caract;
	  btotales--;
	  grabados++;
	}
	aux=arbol;
      }
      if (grabados==TAM_BUF) {
	fwrite(buffer_sal, sizeof(char), grabados, arch_sal);
	grabados=0;
      }
    }
  } while (btotales>0);
	
  fwrite(buffer_sal, sizeof(char), grabados, arch_sal);
	
  if (buffer_sal) {
    free(buffer_sal);
    buffer_sal = NULL;
  }
	
}
	
void indexa_tabla (tabla_t **index, tabla_t *tabla) {
	
  while (tabla) {
    *(index+(unsigned char)tabla->caract) = tabla;
    tabla = tabla->sig;
  }
		
}

