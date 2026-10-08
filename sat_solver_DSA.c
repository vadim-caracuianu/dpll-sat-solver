#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A clause stores the indices and signs of its literals.
typedef struct{
    int *idx;
    int *segno;
    int len;
    int tautologia;
} Clausola;

char **piatti = NULL;
int piatti_cap = 0;
int num_piatti = 0;

Clausola *clausole = NULL;
int num_dip = 0;
int dip_attivi;
int *menu_stato = NULL;

// strdup() is not part of the C11 standard.
char *dup_str(char *s){
    size_t len = strlen(s) + 1;
    char *p = malloc(len);
    memcpy(p, s, len);
    return p;
}

// Return the menu index associated with a dish name.
int trova_piatto(char *parola){
    for(int i = 0; i < num_piatti; i++)
        if(strcmp(piatti[i], parola) == 0)
            return i;
    return -1;
}

// Read an arbitrarily long input line using a growing buffer.
char *leggi_riga(void){
    size_t cap = 256, len = 0;
    char *buf = malloc(cap);
    if(buf == NULL)
        return NULL;

    while(fgets(buf + len, (cap - len), stdin)) {
        len += strlen(buf + len);
        if(len > 0 && buf[len - 1] == '\n') {
            buf[len - 1] = '\0';
            return buf;
        }

        if(len + 1 >= cap){
            cap *= 2;
            char *tmp = realloc(buf, cap);
            if(tmp == NULL){
                free(buf);
                return NULL;
            }
            buf = tmp;
        }else{
            break;
        }
    }

    if(len == 0){
        free(buf);
        return NULL;
    }
    buf[len] = '\0';
    return buf;
}

// Parse one employee's preferences into a clause.
// Duplicate literals are ignored; opposite literals make the clause tautological.
void leggi_riga_dipendente(char *line, Clausola *cl){
    int cap = 4;
    cl->idx = malloc(cap * sizeof(int));
    cl->segno = malloc(cap * sizeof(int));
    cl->len = 0;
    cl->tautologia = 0;

    for(char *tok = strtok(line, " \r"); tok != NULL; tok = strtok(NULL, " \r")){
        int segno = 1;
        if(*tok == '-'){
            segno = -1;
            tok++;
        }

        int idx = trova_piatto(tok);

        int trovato = -1;

        for(int k = 0; k < cl->len; k++)
            if(cl->idx[k] == idx){
                trovato = k;
                break;
            }

        if(trovato < 0) {
            if(cl->len == cap) {
                cap *= 2;
                cl->idx = realloc(cl->idx,   cap * sizeof(int));
                cl->segno = realloc(cl->segno, cap * sizeof(int));
            }
            cl->idx[cl->len] = idx;
            cl->segno[cl->len] = segno;
            cl->len++;

        }else if(cl->segno[trovato] != segno){

            cl->tautologia = 1;
        }

    }
}

// Convert the textual input into the internal SAT representation.
void convert(void){
    char *line = leggi_riga();
    if(line == NULL)
        return;

    piatti_cap = 64;
    piatti = malloc(piatti_cap * sizeof(char *));

    for(char *tok = strtok(line, " \r"); tok != NULL; tok = strtok(NULL, " \r")){
        if(num_piatti == piatti_cap){
            piatti_cap *= 2;
            piatti = realloc(piatti, piatti_cap * sizeof(char *));
        }
        piatti[num_piatti++] = dup_str(tok);
    }
    free(line);

    int cap = 64;
    clausole = malloc(cap * sizeof(Clausola));

    while((line = leggi_riga()) != NULL){
        if(line[0] == '\0'){
            free(line);
            continue;
        }

        if(num_dip == cap){
            cap *= 2;
            clausole = realloc(clausole, cap * sizeof(Clausola));
        }

        leggi_riga_dipendente(line, &clausole[num_dip]);
        num_dip++;
        free(line);
    }

    dip_attivi = num_dip;

    for(int i = 0; i < num_piatti; i++)
        free(piatti[i]);

    free(piatti);
}

// DPLL-style recursive search with unit propagation and backtracking.
int dfs(void){

    // Save the current assignment so this call can backtrack cleanly.
    int *backup = malloc(num_piatti * sizeof(int));
    memcpy(backup, menu_stato, num_piatti * sizeof(int));

    int piatto = -1;
    int cambiato = 1;

    // Repeatedly propagate unit clauses until no new assignment is found.
    while(cambiato != 0){

        cambiato = 0;
        int miglior_len = -1;
        piatto = -1;

        for(int d = 0; d < dip_attivi; d++){
            Clausola *cl = &clausole[d];
            if(cl->tautologia || cl->len == 0)
                continue;

            int soddisfatto = 0, liberi = 0, ultimo_libero = -1, primo_libero = -1;
            for(int k = 0; k < cl->len; k++){
                int j = cl->idx[k], s = cl->segno[k];

                if(menu_stato[j] == s){
                    soddisfatto = 1;
                    break;
                }

                if(menu_stato[j] == 0){
                    liberi++;
                    ultimo_libero = j;
                    if(primo_libero == -1)
                        primo_libero = j;
                }
            }

            if(soddisfatto != 0)
                continue;

            // No satisfying literal remains: this branch is inconsistent.
            if(liberi == 0){
                memcpy(menu_stato, backup, num_piatti * sizeof(int));
                free(backup);
                return 0;
            }

            // Unit clause: its only unassigned literal is forced.
            if(liberi == 1){
                for(int k = 0; k < cl->len; k++){
                    if(cl->idx[k] == ultimo_libero){
                        menu_stato[ultimo_libero] = cl->segno[k];
                        break;
                    }
                }
                cambiato = 1;

            }else if(miglior_len == -1 || liberi < miglior_len){

                miglior_len = liberi;
                piatto = primo_libero;
            }
        }
    }

    // If no constrained variable was selected, choose the first unassigned one.
    if(piatto == -1){
        for(int j = 0; j < num_piatti; j++){
            if(menu_stato[j] == 0){
                piatto = j;
                break;
            }
        }
    }

    // If no constrained variable was selected, choose the first unassigned one.
    if(piatto == -1){
        free(backup);
        return 1;
    }

    menu_stato[piatto] = 1;
    if(dfs() != 0){
        free(backup);
        return 1;
    }
    menu_stato[piatto] = -1;
    if(dfs() != 0){
        free(backup);
        return 1;
    }

    memcpy(menu_stato, backup, num_piatti * sizeof(int));
    free(backup);
    return 0;
}

int main(void){
    convert();
    menu_stato = calloc(num_piatti, sizeof(int));

    int tentativo = 1;

    while(dip_attivi > 0){
        memset(menu_stato, 0, num_piatti * sizeof(int));

        if(dfs() != 0){
            printf("OK\n");
            break;
        }

        if(tentativo == 1){
            printf("KO\n");
            printf("-%d\n", tentativo);
        }else{
            printf("-%d\n", tentativo);
        }
        tentativo++;
        dip_attivi--;
    }

    free(menu_stato);

    for (int i = 0; i < num_dip; i++){
        free(clausole[i].idx);
        free(clausole[i].segno);
    }
    free(clausole);
    return 0;
}
