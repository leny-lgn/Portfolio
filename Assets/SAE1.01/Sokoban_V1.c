#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>


/* Source de ces connaissances de couleurs : 
°https://en.wikipedia.org/wiki/ANSI_escape_code#Control_Sequence_Introducer_co-
mands
°
                                             - */
/* Définition des codes de couleurs pour l'affichage */
#define RESET "\033[0m"
#define JAUNE "\033[33m" 
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define BLEU_GRAS "\033[1;34m"
#define CYAN_GRAS "\033[1;36m"
#define ROUGE_GRAS "\033[1;31m"
#define VERT_GRAS "\033[1;32m"

/* Constantes -> Taille pour tableau, touches d'action et symboles 
d'affichage */
#define TAILLE 12

#define TOUCHE_HAUT 'z'
#define TOUCHE_BAS 's'
#define TOUCHE_GAUCHE 'q'
#define TOUCHE_DROITE 'd'

#define TOUCHE_ABANDON 'x'
#define TOUCHE_RECOMMENCER 'r'
#define REP_OUI_MIN 'o'
#define REP_OUI_MAJ 'O'

#define SYM_CAISSE '$'
#define SYM_MUR '#'
#define SYM_CIBLE '.'
#define SYM_SOKOBAN '@'
#define SYM_SOKOBAN_SUR_CIBLE '+'
#define SYM_CAISSE_SUR_CIBLE '*'
#define SYM_VIDE ' '

/* Nouveau type pour le plateau */
typedef char t_plateau[TAILLE][TAILLE];

/* Prototypes des fonctions fournies  */
int kbhit(void);
void chargerPartie(t_plateau plateau, char fichier[]);
void enregistrerPartie(t_plateau plateau, char fichier[]);

/* Prototypes imposés par la consigne */
void afficherEntete(const char nomFichier[], int nombreDeCoup);
void afficherPlateau(const t_plateau plateau);
void deplacer(t_plateau plateau, int *ligneJoueur, int *colonneJoueur,
     char touche, int *nombreDeCoup);
bool gagne(const t_plateau plateau);

/* Prototypes pour fonctions d'aide */
bool calculer_deplacement(char touche,
                    int ligneJoueur, int colonneJoueur,
                    int *deplacementX, int *deplacementY,
                    int *prochainX, int *prochainY);

void pousser_caisse(t_plateau plateau, int *ligne_joueur, int *col_joueur,
                int deplacementX, int deplacementY, int x, int y,
                int prochainX, int prochainY, char actuel, char suivant,
                int *nombreDeCoup);
void gerer_fin_partie(t_plateau plateau, int nombreDeCoup);
void position_joueur(const t_plateau plateau, int *ligneJoueur,
     int *colonneJoueur);
int compter_cibles(const t_plateau plateau);
void position_cibles(int coords[][2], const t_plateau plateau, 
    int nombreCibles);
void actualiser_cibles(int coords[][2], t_plateau plateau, int nombreCibles);

/* ------------------ Programme principal : fonction main ------------------ */
int main(void) {
    t_plateau plateau;
    char nomFichier[256];
    int ligneJoueur = -1;
    int colonneJoueur = -1;
    char touche = '\0';
    int nombreDeCoup = 0;

    printf("Nom du fichier .sok à charger (ex: niveauX.sok) : ");
    scanf("%s", nomFichier);

    chargerPartie(plateau, nomFichier);

    position_joueur(plateau, &ligneJoueur, &colonneJoueur);

    /* compter et mémoriser les cibles */
    int nbCibles = compter_cibles(plateau);
    int coordsCibles[nbCibles][2];
    position_cibles(coordsCibles, plateau, nbCibles);

    /* affichage initial */
    afficherEntete(nomFichier, nombreDeCoup);
    afficherPlateau(plateau);

    /* boucle de jeu */
    while (touche != TOUCHE_ABANDON && !gagne(plateau)) {
        touche = '\0';
        if (kbhit()) {
            touche = getchar();

            if (touche == TOUCHE_RECOMMENCER) {
                /* recommencer la partie */
                nombreDeCoup = 0;
                chargerPartie(plateau, nomFichier);
                position_joueur(plateau, &ligneJoueur, &colonneJoueur);
                /* repositionner les cibles */
                nbCibles = compter_cibles(plateau);
                position_cibles(coordsCibles, plateau, nbCibles);

                afficherEntete(nomFichier, nombreDeCoup);
                afficherPlateau(plateau);
                continue;
            }

            /* déplacement (ou autre action) */
            deplacer(plateau, &ligneJoueur, &colonneJoueur, touche, 
                &nombreDeCoup);
            actualiser_cibles(coordsCibles, plateau, nbCibles);
            afficherEntete(nomFichier, nombreDeCoup);
            afficherPlateau(plateau);
        }
    }

    /* fin de partie : victoire ou abandon */
    gerer_fin_partie(plateau, nombreDeCoup);

    return EXIT_SUCCESS;
}

/* ------------------ Fonctions imposées en consigne ------------------ */

/* Affichage de l'entete avec des couleurs appelées avec les variables de 
couleurs*/
void afficherEntete(const char nomFichier[], int nombreDeCoup) {
    system("clear");
    printf(BLEU_GRAS "              ==============================\n" RESET);
    printf(CYAN_GRAS "                      S O K O B A N\n" RESET);
    printf(BLEU_GRAS "              ==============================\n" RESET);

    printf(JAUNE "                Projet BUT1 Informatique\n" RESET);
    printf(CYAN "                    IUT de Lannion\n\n" RESET);

    printf(MAGENTA "               -- Déplacez les caisses --\n");
    printf("               --   jusqu'aux cibles   --\n\n" RESET);

    printf(ROUGE_GRAS "|----------------------------------------------------");
    printf("----|\n" RESET);
    printf(ROUGE_GRAS"|"RESET"Fichier : "VERT_GRAS"                       ");
    printf("%s            "ROUGE_GRAS"|\n"RESET, nomFichier);
    printf(ROUGE_GRAS"|"RESET"Touches : q (gauche) | z (haut)  | s (bas) | ");
    printf("d (droite) "ROUGE_GRAS"|\n"RESET);
    printf(ROUGE_GRAS"|"RESET"Autres touches : r (recommencer) |");
    printf(" x (abandonner)       "ROUGE_GRAS"|\n"RESET);
    printf(ROUGE_GRAS"|"RESET"Nombre de deplacements : ");
    printf("%3d                            ", nombreDeCoup);
    printf(ROUGE_GRAS"|\n"RESET);
    printf(ROUGE_GRAS "|----------------------------------------------");
    printf("----------|\n\n" RESET);
}

// (le code est désastreu du au respect des 80 caractères par ligne imposé...)

 /* ici, on utilise %3d pour ne pas avoir de débordement lorsque le nombre de 
 déplacement prends une dizaine ou centaine */
void afficherPlateau(const t_plateau plateau) {
    for (int i = 0 ; i < TAILLE ; i++) {
        printf("                        ");

        for (int j = 0 ; j < TAILLE ; j++) {
            char c = plateau[i][j];
            /* pour l'affichage, on ne distingue pas caisse/caisse_sur_cible
               ni sokoban/sokoban_sur_cible */
            if (c == SYM_CAISSE || c == SYM_CAISSE_SUR_CIBLE) {
                printf("%c", SYM_CAISSE);
            } 
            else if (c == SYM_SOKOBAN || c == SYM_SOKOBAN_SUR_CIBLE) {
                printf("%c", SYM_SOKOBAN);
            } 
            else if (c == SYM_MUR) {
                printf("%c", SYM_MUR);
            } 
            else if (c == SYM_CIBLE) {
                printf("%c", SYM_CIBLE);
            } 
            else {
                printf("%c", SYM_VIDE);
            }
        }
        printf("\n");
    }
}


void deplacer(t_plateau plateau, int *ligneJoueur, int *colonneJoueur,
     char touche, int *nombreDeCoup) {
    int deplacementX = 0;
    int deplacementY = 0;

    int x = *colonneJoueur;
    int y = *ligneJoueur;
    int prochainX = x + deplacementX;
    int prochainY = y + deplacementY;
    
    if (calculer_deplacement(touche, *ligneJoueur, *colonneJoueur,
                         &deplacementX, &deplacementY, &prochainX, 
                         &prochainY) == true) {
    /* la touche est valide — continuer le traitement */
    } else {
        return; /* touche non valide */
    }
    /* verifier les cas impossibles */
    if (prochainX < 0 || prochainX >= TAILLE || 
        prochainY < 0 || prochainY >= TAILLE) {
        return;
    }
    char actuel = plateau[y][x]; /* symbole actuel du joueur */
    char suivant = plateau[prochainY][prochainX];

    /* mouvement simple */
    if (suivant == SYM_VIDE || suivant == SYM_CIBLE) {
        /* liberer la case actuelle */
        if (actuel == SYM_SOKOBAN_SUR_CIBLE) {
            plateau[y][x] = SYM_CIBLE;
        }
        else {
            plateau[y][x] = SYM_VIDE;
        }

        /* arriver sur case suivante */
        if (suivant == SYM_CIBLE) {
            plateau[prochainY][prochainX] = SYM_SOKOBAN_SUR_CIBLE;
        }
        else {
            plateau[prochainY][prochainX] = SYM_SOKOBAN;
        }

        *ligneJoueur = prochainY;
        *colonneJoueur = prochainX;
        (*nombreDeCoup)++;
        return;
    }

    /* pousse d'une caisse si possible */
    if (suivant == SYM_CAISSE || suivant == SYM_CAISSE_SUR_CIBLE) {
        pousser_caisse(plateau, ligneJoueur, colonneJoueur, deplacementX, 
                deplacementY, x, y, prochainX, prochainY, actuel, suivant,
                nombreDeCoup);
    return;
}
}



bool gagne(const t_plateau plateau) {
    for (int i = 0 ; i < TAILLE ; i++) {
        for (int j = 0 ; j < TAILLE ; j++) {
            if (plateau[i][j] == SYM_CAISSE) {
                return false;
            }
        }
    }
    return true;
}

/* ------------------ Fonctions d'aide ------------------------------- */

bool calculer_deplacement(char touche,
                    int ligneJoueur, int colonneJoueur,
                    int *deplacementX, int *deplacementY,
                    int *prochainX, int *prochainY)
{
    int deplacementXTemp = 0;
    int deplacementYTemp = 0;

    if (touche == TOUCHE_GAUCHE) deplacementXTemp = -1;
    else if (touche == TOUCHE_DROITE) deplacementXTemp = 1;
    else if (touche == TOUCHE_HAUT) deplacementYTemp = -1;
    else if (touche == TOUCHE_BAS) deplacementYTemp = 1;
    else return false; /* touche non prise en compte */

    *deplacementX = deplacementXTemp;
    *deplacementY = deplacementYTemp;
    *prochainX = colonneJoueur + deplacementXTemp;
    *prochainY = ligneJoueur + deplacementYTemp;

    return true;
}



void pousser_caisse(t_plateau plateau,
                    int *ligneJoueur, int *colonneJoueur,
                    int deplacementX, int deplacementY,
                    int x, int y,
                    int prochainX, int prochainY,
                    char actuel, char suivant,
                    int *nombreDeCoup) {
                        int prochainX2 = prochainX + deplacementX;
        int prochainY2 = prochainY + deplacementY;
        if (prochainX2 < 0 || prochainX2 >= TAILLE || prochainY2 < 0 || 
            prochainY2 >= TAILLE) {
            return;
        }

        char beyond = plateau[prochainY2][prochainX2];
        if (beyond == SYM_VIDE || beyond == SYM_CIBLE) {
            /* deplacer la caisse */
            if (beyond == SYM_CIBLE) {
                plateau[prochainY2][prochainX2] = SYM_CAISSE_SUR_CIBLE;
            }
            else {
                plateau[prochainY2][prochainX2] = SYM_CAISSE;
            }

            /* remplacer l'ancienne position de la caisse par sokoban */
            if (suivant == SYM_CAISSE_SUR_CIBLE) {
                plateau[prochainY][prochainX] = SYM_SOKOBAN_SUR_CIBLE;
            }
            else {
                plateau[prochainY][prochainX] = SYM_SOKOBAN;
            }

            /* liberer la case actuelle */
            if (actuel == SYM_SOKOBAN_SUR_CIBLE) {
                plateau[y][x] = SYM_CIBLE;
            }
            else {
                plateau[y][x] = SYM_VIDE;
            }

            *ligneJoueur = prochainY;
            *colonneJoueur = prochainX;
            (*nombreDeCoup)++;
        }
    }


void gerer_fin_partie(t_plateau plateau, int nombreDeCoup) {
    if (!gagne(plateau)) {
        /* proposition de sauvegarde */
        printf("\nPartie abandonnée après %d coups.\n", nombreDeCoup);
        printf("Voulez-vous sauvegarder la partie ? (o/n) : ");
        /* lire la réponse */
        char reponse;
        reponse = getchar();
        if (reponse == REP_OUI_MIN || reponse == REP_OUI_MAJ) {
            char nomSauvegarde[260];
            printf("Nom du fichier de sauvegarde (sans extension) : ");
            scanf("%s", nomSauvegarde);
            strcat(nomSauvegarde, ".sok");
            enregistrerPartie(plateau, nomSauvegarde);
            printf("Partie sauvegardée dans %s\n", nomSauvegarde);
            
        } 
        else {
            system("clear");
            printf("Aucune sauvegarde effectuée.\n");
            printf("A bientôt !\n");
        }
    } 
    else {
        printf("\n*** Félicitations ! Vous avez gagné en %d coups ! ***\n", 
            nombreDeCoup);
        printf("A bientôt !\n");
    }
}

void position_joueur(const t_plateau plateau, int *ligneJoueur, 
    int *colonneJoueur) {
    *ligneJoueur = -1;
    *colonneJoueur = -1;
    for (int i = 0 ; i < TAILLE ; i++) {
        for (int j = 0 ; j < TAILLE ; j++) {
            if (plateau[i][j] == SYM_SOKOBAN ||
                 plateau[i][j] == SYM_SOKOBAN_SUR_CIBLE) {
                *ligneJoueur = i;
                *colonneJoueur = j;
                return; /* on sort dès que le sokoban est trouvé */
            }
        }
    }
}

int compter_cibles(const t_plateau plateau) {
    int compteur = 0;
    for (int i = 0 ; i < TAILLE ; i++) {
        for (int j = 0 ; j < TAILLE ; j++) {
            if (plateau[i][j] == SYM_CIBLE || 
                plateau[i][j] == SYM_CAISSE_SUR_CIBLE) {
                compteur++;
            }
        }
    }
    return compteur;
}

void position_cibles(int coords[][2], const t_plateau plateau,
     int nombreCibles) {
    int indiceCible = 0;
    for (int i = 0 ; i < TAILLE ; i++) {
        for (int j = 0 ; j < TAILLE ; j++) {
            if (plateau[i][j] == SYM_CIBLE ||
                 plateau[i][j] == SYM_CAISSE_SUR_CIBLE) {
                coords[indiceCible][0] = i;
                coords[indiceCible][1] = j;
                indiceCible++;
                if (indiceCible >= nombreCibles) {
                    return;
                }
            }
        }
    }
}

void actualiser_cibles(int coords[][2], t_plateau plateau, int nombreCibles) {
    for (int compteurCible = 0 ; compteurCible < nombreCibles ; compteurCible++) {
        int ligneCible = coords[compteurCible][0];
        int colonneCible = coords[compteurCible][1];
        if (plateau[ligneCible][colonneCible] == SYM_VIDE) {
            plateau[ligneCible][colonneCible] = SYM_CIBLE;
        }
        else if (plateau[ligneCible][colonneCible] == SYM_SOKOBAN) {
            plateau[ligneCible][colonneCible] = SYM_SOKOBAN_SUR_CIBLE;
        }
        else if (plateau[ligneCible][colonneCible] == SYM_CAISSE) {
            plateau[ligneCible][colonneCible] = SYM_CAISSE_SUR_CIBLE;
        }
    }
}

/* ------------------ Fonctions fournies ------------------ */

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

    if (ch != EOF) {
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
        printf("ERREUR SUR FICHIER\n");
        exit(EXIT_FAILURE);
    }
    for (int ligne = 0 ; ligne < TAILLE ; ligne++){
        for (int colonne = 0 ; colonne < TAILLE ; colonne++){
            fread(&plateau[ligne][colonne], sizeof(char), 1, f);
        }
        fread(&finDeLigne, sizeof(char), 1, f);
    }
    fclose(f);
}

void enregistrerPartie(t_plateau plateau, char fichier[]){
    FILE * f;
    char finDeLigne = '\n';

    f = fopen(fichier, "w");
    for (int ligne = 0 ; ligne < TAILLE ; ligne++){
        for (int colonne = 0 ; colonne < TAILLE ; colonne++){
            fwrite(&plateau[ligne][colonne], sizeof(char), 1, f);
        }
        fwrite(&finDeLigne, sizeof(char), 1, f);
    }
    fclose(f);
}
