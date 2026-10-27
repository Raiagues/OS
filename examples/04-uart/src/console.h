#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <cpu.h>
#include "uart.h"

#define ADRESSE_INITIAL 0x80000000
#define BITS 8
#define BLANC 0xFFFFFFFF
#define NOIR 0x00000000

/**
 * @brief Définit un pixel à la colonne et à la ligne spécifiées avec la couleur donnée.
 * @param column L'indice de colonne.
 * @param ligne L'indice de ligne.
 * @param couleur La couleur du pixel.
 */
void pixel(uint32_t column, uint32_t ligne, uint32_t couleur);

/**
 * @brief Écrit un caractère à la position de colonne et de ligne spécifiée.
 * @param ligne L'indice de ligne.
 * @param column L'indice de colonne.
 * @param character Le caractère à écrire.
 */
void ecrit_car(uint32_t ligne, uint32_t column, char character);

/**
 * @brief Efface le caractère à la position du curseur actuelle.
 */
void efface_position(void);

/**
 * @brief Déplace le curseur à la position spécifiée et met à jour l'affichage.
 * @param row L'indice de ligne.
 * @param column L'indice de colonne.
 */
void place_curseur(uint32_t lig, uint32_t col);

/**
 * @brief Efface le caractère à la position du curseur actuelle.
 */
void efface_char(void);

/**
 * @brief Efface l'écran entier.
 */
void efface_ecran(void);

/**
 * @brief Fait défiler le contenu de l'écran vers le haut d'une ligne.
 */
void defilement(void);

/**
 * @brief Traite un caractère et met à jour l'affichage en conséquence.
 * @param c Le caractère à traiter.
 */
void traite_car(char c);

/**
 * @brief Déplace le curseur d'affichage du timer à la position spécifiée.
 * @param row L'indice de ligne.
 * @param column L'indice de colonne.
 */
extern void console_putbytes(const char *s, int len);

/**
 * @brief Affiche des caractères sur la console.
 * @param s La chaîne de caractères à afficher.
 * @param len La longueur de la chaîne.
 */
void ecrit_car_en_haut(char *timer, int len);

#endif
