#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define EPS 256 //valeur de epsilon
#define POINT 257 //valeur de .


//création des types correspondants à un automate :

typedef struct etat etat;

struct transition{
    etat* arrive;
    int lettre;
};
typedef struct transition transition;

struct etat{
    int id;
    int nb_sortie;
    transition* sortie;
};

struct automate{
    int nb_transitions;
    int nb_etats;
    etat* init;
    etat* final;
};
typedef struct automate automate;

//Création d'un type pile

typedef struct maillon maillon;

struct maillon{
    automate* e;
    maillon* suivant;
};

struct pile{
    maillon* tete;
    int taille;
};
typedef struct pile pile;

//création d'un type pile stockant un état et la profondeur
typedef struct maillon_dfs maillon_dfs;

struct maillon_dfs{
    etat* e;
    int profondeur;
    maillon_dfs* suivant;
};

struct pile_dfs{
    maillon_dfs* tete;
    int taille;
};
typedef struct pile_dfs pile_dfs;

//Permet de stocker tout les états dans un endroit
typedef struct cellule cellule;

struct cellule {
    etat *e;
    cellule *suiv;
};

pile* creer_pile_vide(){
    pile* p = (pile*) malloc(sizeof(pile));
    p->tete = NULL;
    p->taille = 0;
    return p;
}

void free_pile(pile* p){
    maillon* current = p->tete;
    while(current != NULL){
        maillon* temp = current->suivant;
        free(current);
        current = temp;
    }
    free(p);
}

void empiler(pile* p, automate* e){
    p->taille ++;
    maillon* m = malloc(sizeof(maillon));
    m->e = e;
    m->suivant = p->tete;
    p->tete = m;
    return;
}

automate* depiler(pile* p) {
    maillon* m = p->tete;
    automate* e = m->e;

    p->tete = m->suivant;
    p->taille--;

    free(m);
    return e;
}

bool est_vide(pile* p){ //Verifie si la pile est vide
    return !(p->taille);
}

pile_dfs* creer_pile_vide_dfs(){
    pile_dfs* p = (pile_dfs*) malloc(sizeof(pile_dfs));
    p->tete = NULL;
    p->taille = 0;
    return p;
}

void free_pile_dfs(pile_dfs* p){
    maillon_dfs* current = p->tete;
    while(current != NULL){
        maillon_dfs* temp = current->suivant;
        free(current);
        current = temp;
    }
    free(p);
    return;
}

void empiler_dfs(pile_dfs* p, etat* e, int prof){
    p->taille ++;
    maillon_dfs* m = malloc(sizeof(maillon_dfs));
    m->e = e;
    m->profondeur = prof;
    m->suivant = p->tete;
    p->tete = m;
    return;
}

maillon_dfs* depiler_dfs(pile_dfs* p) {
    maillon_dfs* m = p->tete;

    p->tete = m->suivant;
    p->taille--;

    return m;
}

bool est_vide_dfs(pile_dfs* p){
    return !(p->taille);
}

int n = 0; //compte le nombre d'état total

cellule* chaine_etats = NULL;

void enregistrer_etat(etat *e) {
    cellule *c = malloc(sizeof(cellule));

    c->e = e;
    c->suiv = chaine_etats;
    chaine_etats = c;
}

//Free tout les états créés
void free_etat(){
    cellule *c = chaine_etats;
        while (c != NULL) {
            cellule *suiv = c->suiv;

            free(c->e->sortie);  // Libère champ transitions
            free(c->e);          // Libère l'état
            free(c);             // Libère la cellule de la liste chaînée

            c = suiv;
        }
        chaine_etats = NULL;
}

//Partie Création de l'automate de Thomson

automate* concat(automate* e, automate* f) { //Renvoie l'automate de Thomson correspond à e@f
    int nb_etats = e->nb_etats + f->nb_etats;
    int nb_transitions =
        e->nb_transitions + f->nb_transitions + 1;

    transition t;
    t.arrive = f->init;
    t.lettre = EPS;

    e->final->sortie[e->final->nb_sortie] = t;
    e->final->nb_sortie++;

    automate* resultat = malloc(sizeof(automate));

    resultat->nb_etats = nb_etats;
    resultat->nb_transitions = nb_transitions;
    resultat->init = e->init;
    resultat->final = f->final;

    free(e);
    free(f);
    return resultat;
}

automate* ou(automate* e, automate* g){ //Renvoie l'automate de Thomson correspond à e|f
    int nb_etats = e->nb_etats + g->nb_etats + 2;
    int nb_transitions =
        e->nb_transitions + g->nb_transitions + 4;

    etat* i = (etat*) malloc(sizeof(etat));
    i->id = n;
    n++;
    i->nb_sortie = 2;
    i->sortie = malloc(4*sizeof(transition));
    enregistrer_etat(i);

    etat* f = (etat*) malloc(sizeof(etat));
    f->id = n;
    n++;
    f->nb_sortie = 0;
    f->sortie = malloc(4*sizeof(transition));
    enregistrer_etat(f);

    transition tI1;
    tI1.arrive = e->init;
    tI1.lettre = EPS;
    transition tI2;
    tI2.arrive = g->init;
    tI2.lettre = EPS;

    transition tF1;
    tF1.arrive = f;
    tF1.lettre = EPS;
    transition tF2;
    tF2.arrive = f;
    tF2.lettre = EPS;

    i->sortie[0] = tI1;
    i->sortie[1] = tI2;
    e->final->sortie[e->final->nb_sortie] = tF1;
    g->final->sortie[g->final->nb_sortie] = tF2;
    e->final->nb_sortie ++;
    g->final->nb_sortie ++;

    automate* resultat = malloc(sizeof(automate));

    resultat->nb_etats = nb_etats;
    resultat->nb_transitions = nb_transitions;
    resultat->init = i;
    resultat->final = f;

    free(e);
    free(g);

    return resultat;
}

automate* etoile(automate* e){
    int nb_etats = e->nb_etats + 2;
    int nb_transitions =
        e->nb_transitions + 4;

    etat* i = (etat*) malloc(sizeof(etat));
    i->id = n;
    n++;
    i->nb_sortie = 2;
    i->sortie = malloc(4*sizeof(transition));
    enregistrer_etat(i);

    etat* f = (etat*) malloc(sizeof(etat));
    f->id = n;
    n++;
    f->nb_sortie = 0;
    f->sortie = malloc(4*sizeof(transition));
    enregistrer_etat(f);

    transition tI1;
    tI1.arrive = e->init;
    tI1.lettre = EPS;
    transition tI2;
    tI2.arrive = f;
    tI2.lettre = EPS;

    transition tF1;
    tF1.arrive = f;
    tF1.lettre = EPS;

    transition t;
    t.arrive = e->init;
    t.lettre = EPS;


    i->sortie[0] = tI1;
    i->sortie[1] = tI2;
    e->final->sortie[e->final->nb_sortie] = tF1;
    e->final->nb_sortie ++;
    e->final->sortie[e->final->nb_sortie] = t;
    e->final->nb_sortie ++;

    automate* resultat = malloc(sizeof(automate));

    resultat->nb_etats = nb_etats;
    resultat->nb_transitions = nb_transitions;
    resultat->init = i;
    resultat->final = f;

    free(e);

    return resultat;
} //Renvoie l'automate de Thomson correspond à e*

automate* lettre(int l){ //Renvoie l'automate de Thomson correspond à l (l est un caractere ASCII, EPS ou POINT)
    etat* i = malloc(sizeof(etat));
    enregistrer_etat(i);
    etat* f = malloc(sizeof(etat));
    enregistrer_etat(f);

    transition t;
    t.arrive = f;
    t.lettre = l;


    i->id = n;
    n++;
    f->id = n;
    n++;
    i->nb_sortie = 1;
    i->sortie = malloc(sizeof(transition));
    i->sortie[0] = t;
    f->nb_sortie = 0;
    f->sortie = malloc(4*sizeof(transition));

    automate* lettre = malloc(sizeof(automate));
    lettre->final = f;
    lettre->init = i;
    lettre->nb_etats = 2;
    lettre->nb_transitions = 1;

    return lettre;
}

automate* interro(automate* e){ // //Renvoie l'automate de Thomson correspond à e? = e|eps
    int nb_etats = e->nb_etats + 2;
    int nb_transitions =
        e->nb_transitions + 3;

    etat* i = (etat*) malloc(sizeof(etat));
    i->id = n;
    n++;
    i->nb_sortie = 2;
    i->sortie = malloc(4*sizeof(transition));
    enregistrer_etat(i);

    etat* f = (etat*) malloc(sizeof(etat));
    f->id = n;
    n++;
    f->nb_sortie = 0;
    f->sortie = malloc(4*sizeof(transition));
    enregistrer_etat(f);

    transition tI1;
    tI1.arrive = e->init;
    tI1.lettre = EPS;
    transition tI2;
    tI2.arrive = f;
    tI2.lettre = EPS;

    transition t;
    t.arrive = f;
    t.lettre = EPS;

    i->sortie[0] = tI1;
    i->sortie[1] = tI2;
    e->final->sortie[e->final->nb_sortie] = t;
    e->final->nb_sortie ++;

    automate* resultat = malloc(sizeof(automate));

    resultat->nb_etats = nb_etats;
    resultat->nb_transitions = nb_transitions;
    resultat->init = i;
    resultat->final = f;

    free(e);

    return resultat;
}

automate* creer_automate_motif(char* motif, int len){ //Crée l'automate reconnaissant la langage dénoté par motif
    pile* p = creer_pile_vide();
    for(int i = 0; i<len;i++){
        if (motif[i] == '@'){
            automate* e = depiler(p);
            automate* f = depiler(p);
            empiler(p, concat(f, e));
        }
        else if (motif[i] == '|'){
            automate* e = (automate*)depiler(p);
            automate* f = (automate*)depiler(p);
            empiler(p, ou(e, f));
        }
        else if (motif[i] == '?'){
            automate* e = (automate*)depiler(p);
            empiler(p, interro(e));
        }
        else if (motif[i] == '*'){
            automate* e = (automate*)depiler(p);
            empiler(p, etoile(e));
        }
        else if (motif[i] == '.'){
            empiler(p, lettre(POINT));
        }
        else{
            empiler(p, lettre(motif[i]));
        }
    }
    automate* res = (automate*)depiler(p);
    free_pile(p);
    return res;
}


//Partie lecture dans l'automate de Thosmon

bool dfs(char* ligne, automate* motif){ //Fait un parcours dans l'AFND-eps pour determiner si la ligne contient le motif (= atteint l'état final à un moment)
    bool res = false;
    int len = strlen(ligne);
    int n = motif->nb_etats;
    bool** vus = (bool**) malloc(n*sizeof(bool*));
    for(int i = 0; i<n;i++){
        vus[i] = (bool*) malloc((len+1)*sizeof(bool));
    }
    for(int i = 0; i<n;i++){
        for(int j = 0; j<len+1;j++){
            vus[i][j] = false;
        }
    }
    pile_dfs* p = creer_pile_vide_dfs();
    empiler_dfs(p, motif->init, 0);
    while(!est_vide_dfs(p)){
        maillon_dfs* m = depiler_dfs(p);
        int prof = m->profondeur;
        etat* e = m->e;
        if((prof < len+1) && (!vus[e->id][prof])){
            if(e->id == motif->final->id){
                res = true;
                free(m);
                break;
            }
            if(prof == len){
                free(m);
                continue;
            }
            vus[e->id][prof] = true;
            for(int i = 0; i < e->nb_sortie;i++){
                if(e->sortie[i].lettre == ligne[prof]){
                    empiler_dfs(p, e->sortie[i].arrive, prof+1);
                }
                if(e->sortie[i].lettre == EPS){
                    empiler_dfs(p, e->sortie[i].arrive, prof);
                }
                else if(e->sortie[i].lettre == POINT){
                    empiler_dfs(p, e->sortie[i].arrive, prof+1);
                }
            }
        }
        free(m);
    }
    for(int i = 0; i<n;i++){
        free(vus[i]);
    }
    free(vus);
    free_pile_dfs(p);
    return res;
}

void recherche_motif(char* ligne, automate* motif) { //Appelle dfs sur tout les suffixes de la ligne
    int len = strlen(ligne);
    for (int i = 0; i < len; i++) {
        if(dfs(ligne + i, motif)){
            printf("%s", ligne);
            return;
        }
    }
}

void lire(FILE* f, automate* motif){ //Lis le fichier et appelle recherche_motif ligne par ligne
    char ligne[1024];
    while (fgets(ligne, sizeof(ligne), f) != NULL) {
        recherche_motif(ligne, motif);
    }
    return;
}



int main(int argc, char* argv[]) {
    assert(argc == 3);
    FILE* f = fopen(argv[2], "r");

    if (f == NULL) {
        printf("fichier mal lu");
        return EXIT_FAILURE;
    }

    char* motif = argv[1];
    int taille = strlen(motif);
    automate* a_motif;
    if (taille == 0){
        printf("motif vide");
        return EXIT_FAILURE;
    }
    else{
        a_motif = creer_automate_motif(motif, taille);
    }
    lire(f, a_motif);
    free_etat();
    free(a_motif);
    fclose(f);
    return EXIT_SUCCESS;
}
