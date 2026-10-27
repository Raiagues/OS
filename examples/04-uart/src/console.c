#include "console.h"

extern char font8x8_basic[256][8];

// Variables globales pour suivre la position du curseur
uint32_t ligne_actuel = 1;
uint32_t colonne_actuel = 0;

// Position du curseur pour l'affichage du timer
uint32_t ligne_time = 0;
uint32_t colonne_time = 0;

// Drapeaux d'initialisation et de début de la console
uint32_t start = 0;
uint32_t init = 0;

void pixel(uint32_t column, uint32_t ligne, uint32_t couleur)
{
    if (column >= DISPLAY_WIDTH || ligne >= DISPLAY_HEIGHT) return;
    volatile uint32_t *adresse = (volatile uint32_t*)ADRESSE_INITIAL + (column + ligne*DISPLAY_WIDTH);
    (*adresse) = couleur;
}

void ecrit_car(uint32_t ligne, uint32_t column, char character)
{
    for (int pixel_h = 0; pixel_h < BITS; pixel_h++)
    {
        char char_element = font8x8_basic[(uint8_t)character][pixel_h];
        for (int pixel_v = 0; pixel_v < BITS; pixel_v++)
        {
            if((char_element >> pixel_v)&1)
            {
                pixel(BITS*column + pixel_v, BITS*ligne + pixel_h, BLANC);
            }
        }
    }
}

void efface_position(void)
{
    for(uint32_t i = 0; i < BITS; i++)
    {
        pixel(colonne_actuel*BITS + i, ligne_actuel*BITS + 7, NOIR);
    }
}

void place_curseur(uint32_t lig, uint32_t col)
{
    efface_position();

    ligne_actuel = lig;
    colonne_actuel = col;

    for(uint32_t i=0; i<BITS; i++)
    {
        pixel(col*BITS + i, lig*BITS + 7, BLANC);
    }
}

void efface_char(void)
{
    for(uint32_t i = 0; i < BITS; i++)
    {
        for(uint32_t j = 0; j < BITS; j++)
        {
            pixel(colonne_actuel*BITS + i, ligne_actuel*BITS + j, NOIR);
        }
    }
    efface_position();
}

void efface_ecran(void)
{
    for(uint32_t column = 0; column < DISPLAY_WIDTH; column++)
    {
        for(uint32_t ligne = 0; ligne < DISPLAY_HEIGHT; ligne++)
        {
            pixel(column, ligne, NOIR);
        }
    }
}

void defilement(void)
{
    // Calcul des adresses de destination et source pour le défilement
    uint32_t largeur_pixel = BITS*DISPLAY_WIDTH;
    uint32_t *destination = (uint32_t*)ADRESSE_INITIAL + largeur_pixel;
    uint32_t *source = (uint32_t*)ADRESSE_INITIAL + 2*largeur_pixel;

    // Calcul de la taille en octets de la zone de mémoire à déplacer
    uint32_t size = DISPLAY_WIDTH*(DISPLAY_HEIGHT - 2*BITS)*sizeof(uint32_t);

    // Mise à jour du curseur après le défilement vers le haut
    place_curseur(DISPLAY_HEIGHT/BITS - 2, colonne_actuel);

    // Déplacement du contenu de l'écran vers le haut (défilement)
    memmove(destination, source, size);
    memset((uint32_t*)ADRESSE_INITIAL + DISPLAY_WIDTH*(DISPLAY_HEIGHT - BITS),
           0, DISPLAY_WIDTH*BITS*sizeof(uint32_t));
}

void traite_car(char c)
{
    // Vérifie si le curseur atteint la dernière ligne de l'écran, défilement nécessaire
    if(ligne_actuel == (DISPLAY_HEIGHT/BITS - 1))
    {
        defilement();
    }

    // Efface le caractère précédent à la position du curseur
    efface_char();

    // Gestion des caractères spéciaux
    switch(c)
    {
        case '\b': // Retour en arrière (backspace)
            if(colonne_actuel > 0)
            {
                place_curseur(ligne_actuel, colonne_actuel - 1);
            }
            break;
        case '\t': { // Tabulation
            uint32_t tab = 8;
            if(colonne_actuel + (tab - colonne_actuel % tab) >= DISPLAY_WIDTH/BITS)
            {
                place_curseur(ligne_actuel + 1, 0);
            }
            else
            {
                uint32_t rest_tabulation = tab - (colonne_actuel%tab);
                place_curseur(ligne_actuel, colonne_actuel + rest_tabulation);
            }
            break;
        }
        case '\n': // Nouvelle ligne
            place_curseur(ligne_actuel + 1, 0);
            break;
        case '\f': // Effacement de l'écran
            efface_ecran();
            start = 0;
            place_curseur(1, 0);
            break;
        case '\r': // Retour au début de la ligne
            place_curseur(ligne_actuel, 0);
            break;
        default: // Caractères imprimables
            if(31 < c && c < 127)
            {
                efface_char();
                // Vérifie la position du curseur pour l'écriture du caractère
                if(colonne_actuel*BITS <= DISPLAY_WIDTH - 2*BITS){
                    ecrit_car(ligne_actuel, colonne_actuel, c);
                    place_curseur(ligne_actuel, colonne_actuel + 1);
                }
                else{
                    ecrit_car(ligne_actuel, colonne_actuel, c);
                    place_curseur(ligne_actuel + 1, 0);
                }
            }
    }
}


void place_curseur_timer(uint32_t lig, uint32_t col)
{
    ligne_time = lig;
    colonne_time = col;
}


void traite_car_timer(char c)
{
    // Efface la zone du caractère précédent
    for(uint32_t i = 0; i < BITS; i++)
    {
        for(uint32_t j = 0; j < BITS; j++)
        {
            // Efface la zone horizontale du caractère
            pixel(colonne_time*BITS + i, ligne_time*BITS + j, NOIR);
        }
    }

    for(uint32_t i = 0; i < BITS; i++)
    {
        // Efface la ligne inférieure du caractère
        pixel(colonne_time*BITS + i, ligne_time*BITS + 7, NOIR);
    }

    // Vérifie la position du curseur pour l'écriture du caractère
    if(colonne_time < DISPLAY_WIDTH/BITS){
        ecrit_car(ligne_time, colonne_time, c);
        place_curseur_timer(ligne_time, colonne_time + 1);
    }
}



void ecrit_car_en_haut(char *timer, int len)
{
    // Calcul de l'espacement nécessaire pour centrer le timer en haut de l'écran
    uint32_t formatage = DISPLAY_WIDTH/BITS - strlen(timer);

    // Positionne le curseur de timer en haut de l'écran
    place_curseur_timer(0, formatage);

    // Traite chaque caractère du timer pour l'affichage
    for(int i = 0; i < len; i++)
    {
        traite_car_timer(timer[i]);
    }

    place_curseur_timer(ligne_actuel, colonne_actuel);

    // Marque l'initialisation du timer
    init = 1;
}

void console_putbytes(const char *s, int len)
{
    unsigned long previous;
    __asm__ volatile("csrrc %0, mstatus, %1" : "=r"(previous)
                     : "r"((unsigned long)MSTATUS_MIE) : "memory");
    // Initialise le curseur de la console lors du premier appel
    if(start == 0)
    {
        start = 1;
        place_curseur(1, 0);
    }

    // Traite chaque caractère de la chaîne pour l'affichage sur la console
    for (int i = 0; i < len; i++){
        traite_car(s[i]);
        if (s[i] != '\0') traite_car_uart(s[i]);
    }
    if (previous & MSTATUS_MIE) enable_it();
}
