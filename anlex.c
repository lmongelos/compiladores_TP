 /* ********** Inclusión de cabecera ************* */
#include "anlex.h"

 /* ************ Variables globales ************* */
token t;			       // token global para recibir componentes del Analizador Lexico

/* **** Variables para el analizador lexico **** */

FILE *archivo, *salida;               // Fuente JSON
char id[TAMLEX];		                  // Utilizado por el analizador lexico
int numLinea = 1 ;			                  // Numero de Linea
char cont_esp [TAMESP];                // Variable para contar la cantidad de espacio
int con = -1 ;                           // Variable que indica la cantidad de espacio que ya se ha cargado en cont_esp

/* *************** Funciones ********************* */
void error(const char* mensaje) {
    printf(" Lin %d: Error Lexico. %s", numLinea, mensaje);
}
void sigLex() {
    int i = 0;
    int ban = 0;
    int acepto = 0;
    int estado = 0;
    char c = 0;
    char msg[41];
    char aux[TAMAUX] = " ";
    con = -1;
    token e;
    while ((c = fgetc(archivo)) != EOF) {
        if (c == '\n') {
            numLinea++;
            continue;
        } else if (c == ' ') {
            do {              // Se encarga de cargar los espacios leidos para luego imprimirlos.
                con++;
                cont_esp[con] = ' ';
                c = fgetc(archivo);
            } while (c == ' ');
            c = ungetc(c, archivo);
        } else if (c == '\t') {   // Si es un tabulador que guarde los 4 espacios correspondientes.
            while (c == '\t') {
                printf("%c", c);
                c = fgetc(archivo);
            }
        } else if (isdigit(c)) {   // Si es un numero.
            i = 0;
            estado = 0;
            acepto = 0;
            id[i] = c;
            while (!acepto)  {
                switch (estado)  {
                    case 0:         // Una secuencia netamente de digitos, puede ocurrir . o e
                        c = fgetc(archivo);
                        if (isdigit(c))  {
                            id[++i] = c;
                            estado = 0;
                        } else if (c == '.')  {
                            id[++i] = c;
                            estado = 1;
                        } else if (tolower(c) == 'e')  {
                            id[++i] = c;
                            estado = 3;
                        } else
                            estado = 6;
                        break;
                    case 1:         // Un punto, debe seguir un digito.
                        c = fgetc(archivo);
                        if (isdigit(c))  {
                            id[++i] = c;
                            estado = 2;
                        } else   {
                            sprintf(msg, "No se esperaba '%c' despues del . ", c);
                            estado = -1;
                        }
                        break;
                    case 2:            // La fraccion decimal, pueden seguir los digitos o e.
                        c = fgetc(archivo);
                        if (isdigit(c))  {
                            id[++i] = c;
                            estado = 2;
                        } else if (tolower(c) == 'e')  {
                            id[++i] = c;
                            estado = 3;
                        } else
                            estado = 6;
                        break;
                    case 3:             // Una e, puede seguir +, - o una secuencia de digitos.
                        c = fgetc(archivo);
                        if (c == '+' || c == '-')  {
                            id[++i] = c;
                            estado = 4;
                        } else if (isdigit(c))  {
                            id[++i] = c;
                            estado = 5;
                        } else   {
                            sprintf(msg, "Se esperaba signo o digitos despues del exponente");
                            estado = -1;
                        }
                        break;
                    case 4:             // Necesariamente debe venir por lo menos un digito.
                        c = fgetc(archivo);
                        if (isdigit(c))  {
                            id[++i] = c;
                            estado = 5;
                        } else  {
                            sprintf(msg, "No se esperaba '%c' despues del signo", c);
                            estado = -1;
                        }
                        break;
                    case 5:            // Una secuencia de digitos correspondiente al exponente.
                        c = fgetc(archivo);
                        if (isdigit(c))  {
                            id[++i] = c;
                            estado = 5;
                        } else
                            estado = 6;
                        break;
                    case 6:        // Estado de aceptacion, devolver el caracter correspondiente a otro componente lexico.
                        if (c != EOF)
                            ungetc(c, archivo);
                        else
                            c = 0;
                        id[++i] = '\0';
                        acepto = 1;
                        //t.compLex = NUMBER;
                        t.linea=numLinea;
                        strcpy(t.lexema, id);
                        //array_tokens[tamano_actual] = t;
                        //tamano_actual++;
                        break;
                    case -1:
                        if (c == EOF)
                            error("No se esperaba el fin de archivo\n");
                        else
                            error(msg);
                        acepto = 1;
                        t.compLex = VACIO;
                        t.linea=numLinea;
                        while (c != '\n')
                            c = fgetc(archivo);
                        ungetc(c, archivo);
                        break;
                }
            }
            break;
        } else if (c == '\"')  {       // Si es un caracter o una cadena de caracteres.
            i = 0;
            id[i] = c;
            i++;
            do  {
                c = fgetc(archivo);
                if (c == '\"')  {
                    id[i] = c;
                    i++;
                    ban = 1;
                    break;
                } else if (c == EOF || c == ',' || c == '\n' || c == ':')  {
                    while (c != '\n')  {
                        c = fgetc(archivo);
                        if (c == '\"')  {
                            ban = 1;
                        }
                    }
                    if (ban == 0)  {
                        sprintf(msg, "Se esperaba que finalize el literal");
                        error(msg);
                    }
                    ungetc(c, archivo);
                    con = -1;
                    break;
                } else  {
                    id[i] = c;
                    i++;
                }
            } while (isascii(c) || ban == 0);
            id[i] = '\0';
            strcpy(t.lexema, id);
            t.compLex = STRING;
            t.linea=numLinea;
            //array_tokens[tamano_actual] = t;
//            tamano_actual++;
            break;
        } else if (c == ':')  {       // Si es un :
            t.compLex = DOS_PUNTOS;
            t.linea=numLinea;
            strcpy(t.lexema, ":");
//            array_tokens[tamano_actual] = t;
//            tamano_actual++;
            break;
        } else if (c == ',')  {
            t.compLex = COMA;
            t.linea=numLinea;
            strcpy(t.lexema, ",");
//            array_tokens[tamano_actual] = t;
//            tamano_actual++;
            break;
        } else if (c == '[')   {
            t.compLex = L_CORCHETE;
            t.linea=numLinea;
            strcpy(t.lexema, "[");
//            array_tokens[tamano_actual] = t;
//            tamano_actual++;
            break;
        } else if (c == ']')  {
            t.compLex = R_CORCHETE;
            t.linea=numLinea;
            strcpy(t.lexema, "]");
//            array_tokens[tamano_actual] = t;
//            tamano_actual++;
            break;
        } else if (c == '{')  {
            t.compLex = L_LLAVE;
            t.linea=numLinea;
            strcpy(t.lexema, "{");
//            array_tokens[tamano_actual] = t;
//            tamano_actual++;
            break;
        } else if (c == '}')  {
            t.compLex = R_LLAVE;
            t.linea=numLinea;
            strcpy(t.lexema, "}");
//            array_tokens[tamano_actual] = t;
//            tamano_actual++;
            break;
        } else if (c == 'n' || c == 'N')  {
            ungetc(c, archivo);
            fgets(aux, 5, archivo); //ver si es null
            if (strcmp(aux, "null") == 0 || strcmp(aux, "NULL") == 0)  {
                t.compLex = PR_NULL;
                t.linea=numLinea;
                strcpy(t.lexema, aux);
//                array_tokens[tamano_actual] = t;
//                tamano_actual++;
            } else   {
                sprintf(msg, "%c no esperado", c);
                error(msg);
                while (c != '\n')
                    c = fgetc(archivo);
                t.compLex = VACIO;
                t.linea=numLinea;
                ungetc(c, archivo);
            }
            break;
        }
        else if (c == 'f' || c == 'F')  {
            ungetc(c, archivo);
            fgets(aux, 6, archivo);          //ver si es null
            if (strcmp(aux, "false") == 0 || strcmp(aux, "FALSE") == 0)  {
                t.compLex = PR_FALSE;
                t.linea=numLinea;
                strcpy(t.lexema, aux);
//                array_tokens[tamano_actual] = t;
//            tamano_actual++;
            } else   {
                sprintf(msg, "%c no esperado", c);
                error(msg);
                while (c != '\n')
                    c = fgetc(archivo);
                t.compLex = VACIO;
                t.linea=numLinea;
                ungetc(c, archivo);
            }
            break;
        }
        else if (c == 't' || c == 'T')  {
            ungetc(c, archivo);
            fgets(aux, 5, archivo);         //ver si es null
            if (strcmp(aux, "true") == 0 || strcmp(aux, "TRUE") == 0)  {
                t.compLex = PR_TRUE;
                t.linea=numLinea;
                strcpy(t.lexema, aux);
//                array_tokens[tamano_actual] = t;
//                tamano_actual++;
            } else  {
                sprintf(msg, "%c no esperado", c);
                error(msg);
                while (c != '\n')
                    c = fgetc(archivo);
                t.compLex = VACIO;
                t.linea=numLinea;
                ungetc(c, archivo);
            }
            break;
        } else if (c != EOF)  {
            sprintf(msg, "%c no esperado", c);
            error(msg);
            while (c != '\n')
                c = fgetc(archivo);
            strcpy(cont_esp, " ");
            con = -1;          // Variable que controla los espacios que se imprimen.
            ungetc(c, archivo);
        }
    }
    if (c == EOF)  {
        t.compLex = EOF;
        t.linea=numLinea;
        strcpy(t.lexema, "EOF");
        sprintf(t.lexema, "EOF");
//        array_tokens[tamano_actual] = t;
//        tamano_actual++;
    }
}

int  main ( int argc, char *args[])
{
	salida = fopen ( "output.txt " , " w " );
	// como imprimir un \ n
	if (argc> 1 )
	{
		if (!(archivo=fopen(args[1], " rt " )))
		{
			fprintf (salida, "%s" , " Archivo no encontrado.\n " );
			return (1);
		}
		while (t.compLex != EOF)
		{
			sigLex ();
            // preguntar si con> -1 para poder imprimir los espacios
            if (con> - 1 )
            {
                int j = 0 ;
                for (j = 0 ; j <= con; j ++)
                fprintf (salida, "%c", cont_esp[j]);
            }
            switch (t.compLex )
			      {
                case L_CORCHETE:
                    fprintf (salida, " % s " , " L_CORCHETE " );
                    break;
                case R_CORCHETE:
                    fprintf (salida, " % s " , " R_CORCHETE " );
                    break;
                case L_LLAVE:
                    fprintf (salida, " % s " , " L_LLAVE " );
                    break;
                case R_LLAVE:
                    fprintf (salida, " % s " , " R_LLAVE " );
                    break;
                case COMA:
                    fprintf (salida, " % s " , " COMA " );
                    break;
                case DOS_PUNTOS:
                    fprintf (salida, " % s " , " DOS_PUNTOS " );
                    break;
                case STRING:
                    fprintf (salida, " % s " , " STRING " );
                    break;
               case NUMERO:
                    fprintf (salida, " % s " , " NÚMERO " );
                    break;
                case PR_TRUE:
                    fprintf (salida, " % s " , " PR_TRUE " );
                    break;
                case PR_FALSE:
                    fprintf (salida, " % s " , " PR_FALSE " );
                    break;
                case PR_NULL:
                    fprintf (salida, " % s " , " PR_NULL " );
                    break;
                case EOF:
                    // fprintf (salida, "% s", "EOF");
                    break;
            }

		}
		fclose (archivo);
	}
	else
	{
		fprintf (salida, "%s", "Debe pasar como parámetro el camino al archivo fuente. \n" );
		return (1);
	}
	fclose (salida);
	return  0 ;
}
