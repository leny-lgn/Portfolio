/**
 * @file Sokoban1.02.c
 * @brief Projet Sokoban SAE 1.02
 * @author Leny Langon & Timoté Le Plapous
 * @version 1
 * @date 13-12-2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // usleep
#include <termios.h> // kbhit
#include <fcntl.h>   // kbhit
#include <ctype.h>   // toupper/tolower
#include <stdbool.h> 

/* --- CONSTANTES --- */
#define TAILLE 12
#define MAX_HISTORIQUE 1000

/* Symboles */
#define SYM_MUR '#'
#define SYM_VIDE ' '
#define SYM_CIBLE '.'
#define SYM_CAISSE '$'
#define SYM_CAISSE_SUR_CIBLE '*'
#define SYM_SOKOBAN '@'
#define SYM_SOKOBAN_SUR_CIBLE '+'

/* Touches Manuel */
#define TOUCHE_HAUT 'z'
#define TOUCHE_BAS 's'
#define TOUCHE_GAUCHE 'q'
#define TOUCHE_DROITE 'd'
#define TOUCHE_ZOOM_PLUS '+'
#define TOUCHE_ZOOM_MOINS '-'
#define TOUCHE_ANNULER 'u'
#define TOUCHE_ABANDON 'x'
#define TOUCHE_RECOMMENCER 'r'

/* --- TYPES --- */
typedef char t_plateau[TAILLE][TAILLE];
typedef char t_tabDeplacement[MAX_HISTORIQUE];

/* --- PROTOTYPES --- */
int kbhit(void);
void chargerPartie(t_plateau plateau, char fichier[]);
void enregistrerPartie(t_plateau plateau, char fichier[]);
void chargerDeplacements(t_tabDeplacement t, char fichier[], int *nb);
void enregistrerDeplacements(t_tabDeplacement t, int nb, char fic[]);

void afficher_entete_manuel(char fichier[], int nbCoups, int zoom);
void afficher_entete_auto(char ficSok[], char ficDep[], int coupActuel, int nbTotal, int zoom);
void afficher_plateau(t_plateau plateau, int zoom);
bool gagne(t_plateau plateau);
void position_joueur(t_plateau plateau, int *lig, int *col);
void deplacer(t_plateau plateau, int *lig, int *col, char direction, int *nbCoups, t_tabDeplacement hist, bool mode_auto);
void annuler_coup(t_plateau plateau, t_tabDeplacement hist, int *nbCoups, int *lig, int *col);

void lancer_mode_manuel();
void lancer_mode_autonome();

/* ------------------------------------------------------------------------- */
/*                                   MAIN                                    */
/* ------------------------------------------------------------------------- */

int main(void) {
    int choix = 0;

    // Afficher l'interface de menu
    printf("\033[H\033[J"); 
    printf("=== SUPER SOKOBAN ===\n");
    printf("1. Mode Manuel (Jouer au clavier)\n");
    printf("2. Mode Autonome (Lire un fichier .dep)\n");
    printf("Votre choix : ");
    scanf("%d", &choix);

    if (choix == 1) {
        lancer_mode_manuel();
    } else if (choix == 2) {
        lancer_mode_autonome();
    } else {
        printf("Choix invalide.\n");
    }

    return EXIT_SUCCESS;
}

/* ------------------------------------------------------------------------- */
/*                             MODE AUTONOME                                 */
/* ------------------------------------------------------------------------- */

void lancer_mode_autonome() {
    t_plateau plateau;
    t_tabDeplacement listeCoups;
    char nomFichierSok[50];
    char nomFichierDep[50];
    int nbCoupsTotal = 0;
    int nbCoupsJoues = 0; 
    int i;
    int ligJoueur, colJoueur;
    int zoom = 1;
    bool victoire = false;
    char coup;
    char direction;

    printf("Nom du fichier partie (.sok) : ");
    scanf("%s", nomFichierSok);
    printf("Nom du fichier deplacements (.dep) : ");
    scanf("%s", nomFichierDep);

    chargerPartie(plateau, nomFichierSok);
    chargerDeplacements(listeCoups, nomFichierDep, &nbCoupsTotal);
    position_joueur(plateau, &ligJoueur, &colJoueur);

    afficher_entete_auto(nomFichierSok, nomFichierDep, 0, nbCoupsTotal, zoom);
    afficher_plateau(plateau, zoom);
    usleep(1000000); // Pause 1s avant départ

    for (i = 0; i < nbCoupsTotal; i++) {
        if (gagne(plateau)) {
            victoire = true;
            break; // Arrêt si gagné avant la fin
        }

        coup = listeCoups[i];

        if (coup == '+') { if (zoom < 3) zoom++; }
        else if (coup == '-') { if (zoom > 1) zoom--; }
        else if (tolower(coup) == 'u') {
             annuler_coup(plateau, listeCoups, &nbCoupsJoues, &ligJoueur, &colJoueur);
        }
        else {
            direction = tolower(coup);
            deplacer(plateau, &ligJoueur, &colJoueur, direction, &nbCoupsJoues, listeCoups, true);
        }

        afficher_entete_auto(nomFichierSok, nomFichierDep, i + 1, nbCoupsTotal, zoom);
        afficher_plateau(plateau, zoom);
        usleep(250000); // Pause 0.25s entre les mouvements
    }

   /* --- Fin de la boucle de jeu --- */

    /* Si on n'a pas détecté de victoire dans la boucle, on vérifie une dernière fois */
    if (!victoire) {
        victoire = gagne(plateau);
    }

    printf("\n--------------------------------------------------\n");
    
    if (victoire) {
        /* Cas où c'est gagné : on affiche le nombre de coups */
        if (i < nbCoupsTotal) {
            /* Si 'i' est plus petit que le total, c'est qu'on a fini avant la fin du fichier */
            printf("SOLUTION VALIDE (Gagne en %d coups).\n", i);
        } else {
            /* Sinon, on a utilisé tous les coups du fichier */
            printf("SOLUTION VALIDE (Gagne en %d coups).\n", nbCoupsTotal);
        }
    } else {
        /* Cas où c'est perdu */
        printf("ECHEC : Partie perdue avec cette liste de coups.\n");
    }

    printf("--------------------------------------------------\n");
}

/* ------------------------------------------------------------------------- */
/*                              MODE MANUEL                                  */
/* ------------------------------------------------------------------------- */

void lancer_mode_manuel() {
    t_plateau plateau;
    t_tabDeplacement historique;
    char nomFichier[50];
    char nomSauv[50];
    char touche = '\0';
    int ligJoueur, colJoueur;
    int nbCoups = 0;
    int zoom = 1;
    char rep;

    printf("Nom du fichier .sok : ");
    scanf("%s", nomFichier);

    chargerPartie(plateau, nomFichier);
    position_joueur(plateau, &ligJoueur, &colJoueur);

    while (touche != TOUCHE_ABANDON && !gagne(plateau)) {
        afficher_entete_manuel(nomFichier, nbCoups, zoom);
        afficher_plateau(plateau, zoom);

        while (!kbhit()) { usleep(10000); }
        touche = getchar();

        switch (touche) {
            case TOUCHE_RECOMMENCER:
                chargerPartie(plateau, nomFichier);
                position_joueur(plateau, &ligJoueur, &colJoueur);
                nbCoups = 0;
                break;
            case TOUCHE_ZOOM_PLUS: if (zoom < 3) zoom++; break;
            case TOUCHE_ZOOM_MOINS: if (zoom > 1) zoom--; break;
            case TOUCHE_ANNULER: 
                annuler_coup(plateau, historique, &nbCoups, &ligJoueur, &colJoueur);
                break;
            case TOUCHE_ABANDON: break;
            default:
                // false = mode manuel, on enregistre le coup dans l'historique
                deplacer(plateau, &ligJoueur, &colJoueur, touche, &nbCoups, historique, false);
                break;
        }
    }

    afficher_entete_manuel(nomFichier, nbCoups, zoom);
    afficher_plateau(plateau, zoom);

    if (gagne(plateau)) printf("\nBRAVO ! GAGNE !\n");
    else printf("\nABANDON.\n");

    // Sauvegarde Historique (Consigne SAE 1.01V2)
    printf("Sauvegarder l'historique (.dep) ? (o/n) : ");
    while(kbhit()) getchar(); 
    scanf(" %c", &rep);
    if (rep == 'o' || rep == 'O') {
        printf("Nom fichier : ");
        scanf("%s", nomSauv);
        enregistrerDeplacements(historique, nbCoups, nomSauv);
    }
}

/* ------------------------------------------------------------------------- */
/*                           fonctions communes                              */
/* ------------------------------------------------------------------------- */

void afficher_entete_manuel(char fichier[], int nbCoups, int zoom) {
    printf("\033[H\033[J");
    printf("--- MODE MANUEL ---\n");
    printf("Fichier: %s | Coups: %d | Zoom: x%d\n", fichier, nbCoups, zoom);
    printf("Touches: ZQSD (bouger), +/- (zoom), U (undo), R (reset), X (quitter)\n");
    printf("--------------------------------------------------\n");
}

void afficher_entete_auto(char ficSok[], char ficDep[], int coupActuel, int nbTotal, int zoom) {
    printf("\033[H\033[J");
    printf("--- MODE AUTONOME ---\n");
    printf("Partie: %s | Deplacements: %s\n", ficSok, ficDep);
    printf("Progression: %d / %d | Zoom: x%d\n", coupActuel, nbTotal, zoom);
    printf("--------------------------------------------------\n");
}

void afficher_plateau(t_plateau plateau, int zoom) {
    int i, j, v, h;
    char c, sym;
    for (i = 0; i < TAILLE; i++) {
        for (v = 0; v < zoom; v++) {
            for (j = 0; j < TAILLE; j++) {
                c = plateau[i][j];
                sym = SYM_VIDE;
                if (c == SYM_MUR) sym = SYM_MUR;
                else if (c == SYM_CIBLE) sym = SYM_CIBLE;
                else if (c == SYM_CAISSE || c == SYM_CAISSE_SUR_CIBLE) sym = SYM_CAISSE;
                else if (c == SYM_SOKOBAN || c == SYM_SOKOBAN_SUR_CIBLE) sym = SYM_SOKOBAN;
                for (h = 0; h < zoom; h++) printf("%c", sym);
            }
            printf("\n");
        }
    }
}

bool gagne(t_plateau plateau) {
    int i, j;
    for (i = 0; i < TAILLE; i++)
        for (j = 0; j < TAILLE; j++)
            if (plateau[i][j] == SYM_CAISSE) return false;
    return true;
}

void position_joueur(t_plateau plateau, int *lig, int *col) {
    int i, j;
    for (i = 0; i < TAILLE; i++)
        for (j = 0; j < TAILLE; j++)
            if (plateau[i][j] == SYM_SOKOBAN || plateau[i][j] == SYM_SOKOBAN_SUR_CIBLE) {
                *lig = i; *col = j; return;
            }
}

void deplacer(t_plateau plateau, int *lig, int *col, char direction, 
              int *nbCoups, t_tabDeplacement hist, bool mode_auto) {
    int dY = 0, dX = 0;
    int y = *lig, x = *col;
    int nY, nX, pY, pX;
    char code = '?';
    char *actuel, *suiv, *apres;

    // Switch des deplacements
    switch(tolower(direction)) {
        case 'z': case 'h': dY = -1; code = 'h'; break;
        case 's': case 'b': dY =  1; code = 'b'; break;
        case 'q': case 'g': dX = -1; code = 'g'; break;
        case 'd': dX =  1; code = 'd'; break;
    }
    if (dY == 0 && dX == 0) return;

    nY = y + dY; nX = x + dX;
    if (nY < 0 || nY >= TAILLE || nX < 0 || nX >= TAILLE) return;

    actuel = &plateau[y][x];
    suiv = &plateau[nY][nX];

    // Mouvement Simple
    if (*suiv == SYM_VIDE || *suiv == SYM_CIBLE) {
        *actuel = (*actuel == SYM_SOKOBAN_SUR_CIBLE) ? SYM_CIBLE : SYM_VIDE;
        *suiv = (*suiv == SYM_CIBLE) ? SYM_SOKOBAN_SUR_CIBLE : SYM_SOKOBAN;
        *lig = nY; *col = nX;
        
        // En mode manuel, on enregistre. En auto, on ne fait rien (l'historique est déjà écrit)
        if (!mode_auto && *nbCoups < MAX_HISTORIQUE) hist[(*nbCoups)++] = code;
    }
    // Poussée
    else if (*suiv == SYM_CAISSE || *suiv == SYM_CAISSE_SUR_CIBLE) {
        pY = nY + dY; pX = nX + dX;
        if (pY >= 0 && pY < TAILLE && pX >= 0 && pX < TAILLE) {
            apres = &plateau[pY][pX];
            if (*apres == SYM_VIDE || *apres == SYM_CIBLE) {
                *apres = (*apres == SYM_CIBLE) ? SYM_CAISSE_SUR_CIBLE : SYM_CAISSE;
                *suiv = (*suiv == SYM_CAISSE_SUR_CIBLE) ? SYM_SOKOBAN_SUR_CIBLE : SYM_SOKOBAN;
                *actuel = (*actuel == SYM_SOKOBAN_SUR_CIBLE) ? SYM_CIBLE : SYM_VIDE;
                *lig = nY; *col = nX;

                if (!mode_auto && *nbCoups < MAX_HISTORIQUE) hist[(*nbCoups)++] = toupper(code);
            }
        }
    }
}

void annuler_coup(t_plateau plateau, t_tabDeplacement hist, int *nbCoups, int *lig, int *col) {
    // Note: cette fonction marche pour le mode manuel.
    // Pour le mode auto, il faudrait lui passer le coup exact à annuler car hist n'est pas utilisé pareil.
    if (*nbCoups <= 0) return;
    char coup = hist[*nbCoups - 1];
    int dY=0, dX=0, y=*lig, x=*col, pY, pX, cY, cX;

    switch(tolower(coup)) {
        case 'h': dY = 1; break; 
        case 'b': dY = -1; break;
        case 'g': dX = 1; break;
        case 'd': dX = -1; break;
    }
    pY = y + dY; pX = x + dX;
    
    // Retour Sokoban
    plateau[pY][pX] = (plateau[pY][pX] == SYM_CIBLE) ? SYM_SOKOBAN_SUR_CIBLE : SYM_SOKOBAN;
    plateau[y][x] = (plateau[y][x] == SYM_SOKOBAN_SUR_CIBLE) ? SYM_CIBLE : SYM_VIDE;

    // Retour Caisse (si majuscule car signifie poussée)
    if (isupper(coup)) {
        cY = y - dY; cX = x - dX; // Position actuelle caisse (devant)
        // On la remet sur la case (y,x) que le joueur vient de libérer
        plateau[cY][cX] = (plateau[cY][cX] == SYM_CAISSE_SUR_CIBLE) ? SYM_CIBLE : SYM_VIDE;
        plateau[y][x] = (plateau[y][x] == SYM_CIBLE) ? SYM_CAISSE_SUR_CIBLE : SYM_CAISSE;
    }
    *lig = pY; *col = pX;
    (*nbCoups)--;
}

/* --- FONCTIONS FOURNIES --- */
int kbhit(){
    struct termios oldt, newt;
    int ch, oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt; newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if(ch != EOF){ ungetc(ch, stdin); return 1; } 
    return 0;
}
void chargerPartie(t_plateau plateau, char fichier[]){
    FILE *f = fopen(fichier, "r");
    char fin;
    if(!f){ printf("ERR FICHIER\n"); exit(1); }
    for(int i=0;i<TAILLE;i++){
        for(int j=0;j<TAILLE;j++) fread(&plateau[i][j],1,1,f);
        fread(&fin,1,1,f);
    }
    fclose(f);
}
void enregistrerPartie(t_plateau plateau, char fichier[]){
    FILE *f = fopen(fichier, "w");
    char fin='\n';
    for(int i=0;i<TAILLE;i++){
        for(int j=0;j<TAILLE;j++) fwrite(&plateau[i][j],1,1,f);
        fwrite(&fin,1,1,f);
    }
    fclose(f);
}
void chargerDeplacements(t_tabDeplacement t, char fichier[], int *nb){
    FILE *f = fopen(fichier, "r");
    char c; *nb=0;
    if(!f) { printf("ERR DEP\n"); return; }
    fread(&c,1,1,f);
    while(!feof(f) && *nb < MAX_HISTORIQUE){
        if(c!='\n' && c!='\r') t[(*nb)++] = c;
        fread(&c,1,1,f);
    }
    fclose(f);
}
void enregistrerDeplacements(t_tabDeplacement t, int nb, char fic[]){
    FILE *f = fopen(fic, "w");
    fwrite(t, 1, nb, f);
    fclose(f);
}