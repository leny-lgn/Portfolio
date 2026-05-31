/**
 * @file Sokoban_leny_V2.c
 * @author Leny
 * @brief Projet Sokoban V2 (Zoom, Undo, Historique) - IUT Lannion
 * @version 2.0
 * @date 2025-11-30
 */

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

/* --- CONSTANTES --- */
#define TAILLE 12
#define MAX_HISTORIQUE 1000

/* Touches de jeu */
#define TOUCHE_HAUT 'z'
#define TOUCHE_BAS 's'
#define TOUCHE_GAUCHE 'q'
#define TOUCHE_DROITE 'd'
#define TOUCHE_ZOOM_PLUS '+'
#define TOUCHE_ZOOM_MOINS '-'
#define TOUCHE_ANNULER 'u'
#define TOUCHE_ABANDON 'x'
#define TOUCHE_RECOMMENCER 'r'

/* Symbols de jeu */
#define SYM_MUR '#'
#define SYM_VIDE ' '
#define SYM_CIBLE '.'
#define SYM_CAISSE '$'
#define SYM_CAISSE_SUR_CIBLE '*'
#define SYM_SOKOBAN '@'
#define SYM_SOKOBAN_SUR_CIBLE '+'

/* --- TYPES --- */
typedef char t_plateau[TAILLE][TAILLE];
typedef char t_tabDeplacement[MAX_HISTORIQUE];

/* --- PROTOTYPES --- */

/* Fonctions fournies (Noms imposés) */
int kbhit(void);
void chargerPartie(t_plateau plateau, char fichier[]);
void enregistrerPartie(t_plateau plateau, char fichier[]);
void enregistrerDeplacements(t_tabDeplacement t, int nb, char fic[]);

/* Mes fonctions */
void afficher_entete(const char fichier[], int nbCoups, int zoom);
void afficher_plateau(const t_plateau plateau, int zoom);
bool gagne(const t_plateau plateau);
void position_joueur(const t_plateau plateau, int *lig, int *col);

void deplacer(t_plateau plateau, int *lig, int *col, char touche,
              int *nbCoups, t_tabDeplacement hist);
void annuler_coup(t_plateau plateau, t_tabDeplacement hist,
                  int *nbCoups, int *lig, int *col);

void traiter_touche(t_plateau plateau, char touche, int *nbCoups,
                    t_tabDeplacement hist, int *zoom,
                    int *lig, int *col);

void gerer_fin_partie(t_plateau plateau, int nbCoups,
                      t_tabDeplacement hist);

void direction_depuis_touche(char touche, int *deltaY, int *deltaX, char *code);

/* ------------------------------------------------------------------------- */
/*                                   MAIN                                    */
/* ------------------------------------------------------------------------- */

int main(void) {
    t_plateau plateau;
    t_tabDeplacement historique;
    char nomFichier[50];
    char touche = '\0';
    int ligJoueur = 0;
    int colJoueur = 0;
    int nbCoups = 0;
    int zoom = 1;

    printf("Nom du fichier .sok : ");
    scanf("%s", nomFichier);

    /* je charge la partie et je repère le joueur */
    chargerPartie(plateau, nomFichier);
    position_joueur(plateau, &ligJoueur, &colJoueur);

    afficher_entete(nomFichier, nbCoups, zoom);
    afficher_plateau(plateau, zoom);

    /* Boucle du jeu, tant qu’on n’abandonne pas ou qu’on n’a pas gagné */
    while (touche != TOUCHE_ABANDON && !gagne(plateau)) {

        if (kbhit()) {
            touche = getchar();

            if (touche == TOUCHE_RECOMMENCER) {
                /* reset classique */
                nbCoups = 0;
                zoom = 1;
                chargerPartie(plateau, nomFichier);
                position_joueur(plateau, &ligJoueur, &colJoueur);
            }
            else if (touche != TOUCHE_ABANDON) {
                traiter_touche(plateau, touche, &nbCoups, historique,
                               &zoom, &ligJoueur, &colJoueur);
            }

            afficher_entete(nomFichier, nbCoups, zoom);
            afficher_plateau(plateau, zoom);
        }
    }

    gerer_fin_partie(plateau, nbCoups, historique);
    return EXIT_SUCCESS;
}

/* ------------------------------------------------------------------------- */
/*                                AFFICHAGE                                  */
/* ------------------------------------------------------------------------- */

void afficher_entete(const char fichier[], int nbCoups, int zoom) {
    system("clear");
    printf("=== SOKOBAN V2 ===\n");
    printf("Fichier : %s | Coups : %d | Zoom : %dx\n",
           fichier, nbCoups, zoom);
    printf("Touches : ZQSD, +/-, U=undo, R=reset, X=quitter\n");
    printf("------------------------------------------------------\n");
}

void afficher_plateau(const t_plateau plateau, int zoom) {
    int i, j, v, h;
    char c, sym;

    /* j'affiche ligne par ligne en répétant selon le zoom */
    for (i = 0; i < TAILLE; i++) {
        /* Répétition verticale pour le zoom */
        for (v = 0; v < zoom; v++) {

            for (j = 0; j < TAILLE; j++) {
                c = plateau[i][j];
                sym = SYM_VIDE;

                /* Je convertis en symbole */
                if (c == SYM_MUR) sym = SYM_MUR;
                else if (c == SYM_CIBLE) sym = SYM_CIBLE;
                else if (c == SYM_CAISSE || c == SYM_CAISSE_SUR_CIBLE)
                    sym = SYM_CAISSE;
                else if (c == SYM_SOKOBAN || c == SYM_SOKOBAN_SUR_CIBLE)
                    sym = SYM_SOKOBAN;

                /* Répétition horizontale pour le zoom */
                for (h = 0; h < zoom; h++)
                    printf("%c", sym);
            }
            printf("\n");
        }
    }
}


bool gagne(const t_plateau plateau) {
    /* on gagne quand il n'y a plus de caisses normales */   
    int i, j;
    for (i = 0; i < TAILLE; i++) {
        for (j = 0; j < TAILLE; j++) {
            if (plateau[i][j] == SYM_CAISSE)
                return false;
        }
    }
    return true;
}

void position_joueur(const t_plateau plateau, int *lig, int *col) {
    /* On cherche @ ou + */
    int i, j;
    for (i = 0; i < TAILLE; i++) {
        for (j = 0; j < TAILLE; j++) {
            if (plateau[i][j] == SYM_SOKOBAN ||
                plateau[i][j] == SYM_SOKOBAN_SUR_CIBLE) {
                *lig = i;
                *col = j;
                return;
            }
        }
    }
}

/*Cette fonction me simplifie les déplacements et me fais gagner de la place */
void direction_depuis_touche(char touche, int *deltaY, int *deltaX, char *code) {
    *deltaY = 0; *deltaX = 0; *code = '?';

    switch (touche) {
        case TOUCHE_HAUT:   *deltaY = -1; *code = 'h'; break;
        case TOUCHE_BAS:    *deltaY =  1; *code = 'b'; break;
        case TOUCHE_GAUCHE: *deltaX = -1; *code = 'g'; break;
        case TOUCHE_DROITE: *deltaX =  1; *code = 'd'; break;
    }
}

void deplacer(t_plateau plateau, int *lig, int *col, char touche,
              int *nbCoups, t_tabDeplacement hist) {

    int deltaY, deltaX;
    int y, x, nextY, nextX, pushY, pushX;
    char code;
    char *actuel, *suivant, *apres;

    direction_depuis_touche(touche, &deltaY, &deltaX, &code);
    if (code == '?') return;

    y = *lig;
    x = *col;
    nextY = y + deltaY;
    nextX = x + deltaX;
    /* Sécurité bordure */
    if (nextY < 0 || nextY >= TAILLE || nextX < 0 || nextX >= TAILLE) return;

    actuel = &plateau[y][x];
    suivant = &plateau[nextY][nextX];

    /* Déplacement simple (vers vide ou cible) */
    if (*suivant == SYM_VIDE || *suivant == SYM_CIBLE) {

        *actuel = (*actuel == SYM_SOKOBAN_SUR_CIBLE) ?
                    SYM_CIBLE : SYM_VIDE;

        *suivant = (*suivant == SYM_CIBLE) ?
                    SYM_SOKOBAN_SUR_CIBLE : SYM_SOKOBAN;

        *lig = nextY;
        *col = nextX;

        if (*nbCoups < MAX_HISTORIQUE)
            hist[(*nbCoups)++] = code;
    }

    /* Poussée de caisse */
    else if (*suivant == SYM_CAISSE ||
             *suivant == SYM_CAISSE_SUR_CIBLE) {

        pushY = nextY + deltaY;
        pushX = nextX + deltaX;

        if (pushY < 0 || pushY >= TAILLE ||
            pushX < 0 || pushX >= TAILLE) return;

        apres = &plateau[pushY][pushX];

        if (*apres == SYM_VIDE || *apres == SYM_CIBLE) {

            *apres = (*apres == SYM_CIBLE) ?
                      SYM_CAISSE_SUR_CIBLE : SYM_CAISSE;

            *suivant = (*suivant == SYM_CAISSE_SUR_CIBLE) ?
                         SYM_SOKOBAN_SUR_CIBLE : SYM_SOKOBAN;

            *actuel = (*actuel == SYM_SOKOBAN_SUR_CIBLE) ?
                        SYM_CIBLE : SYM_VIDE;

            *lig = nextY;
            *col = nextX;

            if (*nbCoups < MAX_HISTORIQUE)
                hist[(*nbCoups)++] = (char)(code - 32); /* Majuscule */
        }
    }
}

/* ------------------------------------------------------------------------- */
/*                                 UNDO                                      */
/* ------------------------------------------------------------------------- */

void annuler_coup(t_plateau plateau, t_tabDeplacement hist,
                  int *nbCoups, int *lig, int *col) {

    char mouv;
    int deltaY = 0, deltaX = 0;
    int y, x, prevY, prevX, caisseY, caisseX;

    if (*nbCoups <= 0) return;

    mouv = hist[*nbCoups - 1];

    /* Inversion du vecteur déplacement */
    switch (mouv) {
        case 'h': case 'H': deltaY =  1; break;
        case 'b': case 'B': deltaY = -1; break;
        case 'g': case 'G': deltaX =  1; break;
        case 'd': case 'D': deltaX = -1; break;
    }

    y = *lig; x = *col;
    prevY = y + deltaY; prevX = x + deltaX;

    if (prevY < 0 || prevY >= TAILLE ||
        prevX < 0 || prevX >= TAILLE) return;

    /* Restaurer la case actuelle */
    plateau[y][x] = (plateau[y][x] == SYM_SOKOBAN_SUR_CIBLE) ?
                     SYM_CIBLE : SYM_VIDE;

    /* Restaurer la position précédente du joueur */
    plateau[prevY][prevX] = (plateau[prevY][prevX] == SYM_CIBLE) ?
                              SYM_SOKOBAN_SUR_CIBLE : SYM_SOKOBAN;

    /* Si c'était une poussée (Majuscule), on tire la caisse en arrière */
    if (mouv >= 'A' && mouv <= 'Z') {
        caisseY = y - deltaY;
        caisseX = x - deltaX;

        /* Enlever la caisse de sa position actuelle (devant le joueur) */
        plateau[caisseY][caisseX] = (plateau[caisseY][caisseX] ==
                                     SYM_CAISSE_SUR_CIBLE) ?
                                     SYM_CIBLE : SYM_VIDE;

        /* Remettre la caisse sur la case que le joueur vient de quitter (y,x) */
        plateau[y][x] = (plateau[y][x] == SYM_CIBLE) ?
                         SYM_CAISSE_SUR_CIBLE : SYM_CAISSE;
    }

    *lig = prevY;
    *col = prevX;
    (*nbCoups)--;
}

/* ------------------------------------------------------------------------- */
/*                          TOUCHES ET FIN DE JEU                            */
/* ------------------------------------------------------------------------- */

void traiter_touche(t_plateau plateau, char touche, int *nbCoups,
                    t_tabDeplacement hist, int *zoom,
                    int *lig, int *col) {

    switch (touche) {
        case TOUCHE_ZOOM_PLUS:
            if (*zoom < 3) (*zoom)++;
            break;

        case TOUCHE_ZOOM_MOINS:
            if (*zoom > 1) (*zoom)--;
            break;

        case TOUCHE_ANNULER:
            annuler_coup(plateau, hist, nbCoups, lig, col);
            break;

        default:
            deplacer(plateau, lig, col, touche, nbCoups, hist);
            break;
    }
}

void gerer_fin_partie(t_plateau plateau, int nbCoups,
                      t_tabDeplacement hist) {

    char rep;
    char nomFichier[60];

    if (gagne(plateau)) {
        printf("\nBravo ! Partie gagnee en %d coups.\n", nbCoups);
    }
    else {
        printf("\nPartie abandonnee (%d coups).\n", nbCoups);

        printf("Sauvegarder le plateau ? (o/n) : ");
        while (kbhit()) getchar();
        scanf(" %c", &rep);

        if (rep == 'o' || rep == 'O') {
            printf("Nom fichier .sok : ");
            scanf("%s", nomFichier);
            enregistrerPartie(plateau, nomFichier);
            printf("Sauvegarde OK.\n");
        }
    }

    printf("Sauvegarder l'historique ? (o/n) : ");
    while (kbhit()) getchar();
    scanf(" %c", &rep);

    if (rep == 'o' || rep == 'O') {
        printf("Nom fichier .dep : ");
        scanf("%s", nomFichier);
        enregistrerDeplacements(hist, nbCoups, nomFichier);
        printf("Historique sauvegarde !\n");
    }
}

/* ------------------------------------------------------------------------- */
/*                         FONCTIONS FOURNIES                                */
/* ------------------------------------------------------------------------- */

int kbhit(){
    int unCaractere = 0;
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF){
        ungetc(ch, stdin);
        unCaractere = 1;
    } 
    return unCaractere;
}

void chargerPartie(t_plateau plateau, char fichier[]){
    FILE *f;
    char finDeLigne;

    f = fopen(fichier, "r");
    if (f == NULL) {
        printf("ERREUR SUR FICHIER");
        exit(EXIT_FAILURE);
    }

    for (int ligne = 0; ligne < TAILLE; ligne++) {
        for (int colonne = 0; colonne < TAILLE; colonne++) {
            fread(&plateau[ligne][colonne], sizeof(char), 1, f);
        }
        fread(&finDeLigne, sizeof(char), 1, f);
    }

    fclose(f);
}

void enregistrerPartie(t_plateau plateau, char fichier[]){
    FILE *f;
    char fin = '\n';

    f = fopen(fichier, "w");

    for (int ligne = 0; ligne < TAILLE; ligne++) {
        for (int colonne = 0; colonne < TAILLE; colonne++) {
            fwrite(&plateau[ligne][colonne], sizeof(char), 1, f);
        }
        fwrite(&fin, sizeof(char), 1, f);
    }

    fclose(f);
}

void enregistrerDeplacements(t_tabDeplacement t, int nb, char fic[]){
    FILE *f = fopen(fic, "w");
    fwrite(t, sizeof(char), nb, f);
    fclose(f);
}