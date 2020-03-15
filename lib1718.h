#pragma once
#define bool int
#define true 1
#define false 0

#define COLUMN_LENGHT 255 	// Massimo numero di caratteri di una colonna
#define STRING_LEN 100


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
Input:
-La stringa contenente la query da eseguire, ad esempio "CREATE TABLE studenti (matricola, nome, cognome)"
Output:
-bool: true/false, se l'esecuzione della query e' andata a buon fine o meno (presenza di eventuali errori)
*/

bool executeQuery(char *query);

// FUNZIONI D'ESECUZIONE SPECIFICA DELLE QUERY
bool create_table(char *query);
bool select_all(char *query);
bool insert_into(char *query);
bool select_columns(char *query);
bool select_where(char *query);
bool select_order_by(char *query);
bool select_group_by(char*query);

// FUNZIONI DI SUPPORTO
char *copy_table_name(char *input, char* query);
FILE* extract_table(char*query, char *input);
void view(char*** table, char ** columns, int num_columns, int num_row, char*query, char*input, FILE ** results);
void clean_structure(char*** table, char ** columns, int num_columns, int *num_row, char*condition);
void correct_query(char*query);
FILE* put_query_results(char*query, FILE*results);
bool table_is_empty(char ***table, int num_rows);

// FUNZIONI PER ORDINAMENTO
void merge(char *** arr, int l, int m, int r, int order_colon, int order, int string_or_not);
void mergeSort(char *** arr, int l, int r, int order_colon, int order, int string_or_not);

// FUNZIONI PER GESTIRE LA STRUTTURA
char *** make_structure(FILE * file, char *** n_colonne, int * num_colonne, int * num_row);
void free_structure(char*** table, char ** columns, int num_columns, int num_row);
