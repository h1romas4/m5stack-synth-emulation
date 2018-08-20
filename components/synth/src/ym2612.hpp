/***************************************************************************
 * Gens: Yamaha YM2612 FM synthesis chip emulator.                         *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#ifndef GENS_YM2612_HPP
#define GENS_YM2612_HPP

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Change it if you need to do long update
#define	MAX_UPDATE_LENGTH   512

// Gens always uses 16 bits sound (in 32 bits buffer) and do the convertion later if needed.
#define OUTPUT_BITS         16

typedef struct slot__
{
	unsigned int *DT; // paramètre detune
	int MUL;	// paramètre "multiple de fréquence"
	int TL;		// Total Level = volume lorsque l'enveloppe est au plus haut
	int TLL;	// Total Level ajusted
	int SLL;	// Sustin Level (ajusted) = volume où l'enveloppe termine sa première phase de régression
	int KSR_S;	// Key Scale Rate Shift = facteur de prise en compte du KSL dans la variations de l'enveloppe
	int KSR;	// Key Scale Rate = cette valeur est calculée par rapport à la fréquence actuelle, elle va influer
				// sur les différents paramètres de l'enveloppe comme l'attaque, le decay ...  comme dans la réalité !
	int SEG;	// Type enveloppe SSG
	unsigned int *AR; // Attack Rate (table pointeur) = Taux d'attaque (AR[KSR])
	unsigned int *DR; // Decay Rate (table pointeur) = Taux pour la régression (DR[KSR])
	unsigned int *SR; // Sustin Rate (table pointeur) = Taux pour le maintien (SR[KSR])
	unsigned int *RR; // Release Rate (table pointeur) = Taux pour le relâchement (RR[KSR])
	int Fcnt;	// Frequency Count = compteur-fréquence pour déterminer l'amplitude actuelle (SIN[Finc >> 16])
	int Finc;	// frequency step = pas d'incrémentation du compteur-fréquence
				// plus le pas est grand, plus la fréquence est aïgu (ou haute)
	int Ecurp;	// Envelope current phase = cette variable permet de savoir dans quelle phase
				// de l'enveloppe on se trouve, par exemple phase d'attaque ou phase de maintenue ...
				// en fonction de la valeur de cette variable, on va appeler une fonction permettant
				// de mettre à jour l'enveloppe courante.
	int Ecnt;	// Envelope counter = le compteur-enveloppe permet de savoir où l'on se trouve dans l'enveloppe
	int Einc;	// Envelope step courant
	int Ecmp;	// Envelope counter limite pour la prochaine phase
	int EincA;	// Envelope step for Attack = pas d'incrémentation du compteur durant la phase d'attaque
				// cette valeur est égal à AR[KSR]
	int EincD;	// Envelope step for Decay = pas d'incrémentation du compteur durant la phase de regression
				// cette valeur est égal à DR[KSR]
	int EincS;	// Envelope step for Sustain = pas d'incrémentation du compteur durant la phase de maintenue
				// cette valeur est égal à SR[KSR]
	int EincR;	// Envelope step for Release = pas d'incrémentation du compteur durant la phase de relâchement
				// cette valeur est égal à RR[KSR]
	int *OUTp;	// pointeur of SLOT output = pointeur permettant de connecter la sortie de ce slot à l'entrée
				// d'un autre ou carrement à la sortie de la voie
	int INd;	// input data of the slot = données en entrée du slot
	int ChgEnM;	// Change envelop mask.
	int AMS;	// AMS depth level of this SLOT = degré de modulation de l'amplitude par le LFO
	int AMSon;	// AMS enable flag = drapeau d'activation de l'AMS

} slot_;

typedef struct channel__
{
	int S0_OUT[4];	// anciennes sorties slot 0 (pour le feed back)
	int Old_OUTd;	// ancienne sortie de la voie (son brut)
	int OUTd;	// sortie de la voie (son brut)
	int LEFT;	// LEFT enable flag
	int RIGHT;	// RIGHT enable flag
	int ALGO;	// Algorythm = détermine les connections entre les opérateurs
	int FB;		// shift count of self feed back = degré de "Feed-Back" du SLOT 1 (il est son unique entrée)
	int FMS;	// Fréquency Modulation Sensitivity of channel = degré de modulation de la fréquence sur la voie par le LFO
	int AMS;	// Amplitude Modulation Sensitivity of channel = degré de modulation de l'amplitude sur la voie par le LFO
	int FNUM[4];	// hauteur fréquence de la voie (+ 3 pour le mode spécial)
	int FOCT[4];	// octave de la voie (+ 3 pour le mode spécial)
	int KC[4];	// Key Code = valeur fonction de la fréquence (voir KSR pour les slots, KSR = KC >> KSR_S)
	slot_ SLOT[4];	// four slot.operators = les 4 slots de la voie
	int FFlag;	// Frequency step recalculation flag
} channel_;

typedef struct ym2612__
{
	int Clock;		// Horloge YM2612
	int Rate;		// Sample Rate (11025/22050/44100)
	int TimerBase;		// TimerBase calculation
	int status;		// YM2612 Status (timer overflow)
	int OPNAadr;		// addresse pour l'écriture dans l'OPN A (propre à l'émulateur)
	int OPNBadr;		// addresse pour l'écriture dans l'OPN B (propre à l'émulateur)
	int LFOcnt;		// LFO counter = compteur-fréquence pour le LFO
	int LFOinc;		// LFO step counter = pas d'incrémentation du compteur-fréquence du LFO
					// plus le pas est grand, plus la fréquence est grande

	int TimerA;		// timerA limit = valeur jusqu'à laquelle le timer A doit compter
	int TimerAL;
	int TimerAcnt;		// timerA counter = valeur courante du Timer A
	int TimerB;		// timerB limit = valeur jusqu'à laquelle le timer B doit compter
	int TimerBL;
	int TimerBcnt;		// timerB counter = valeur courante du Timer B
	int Mode;		// Mode actuel des voie 3 et 6 (normal / spécial)
	int DAC;		// DAC enabled flag
	int DACdata;		// DAC data

	int dummy;		// MSVC++ enforces 8-byte alignment on doubles. This forces said alignment on gcc.
	double Frequence;	// Fréquence de base, se calcul par rapport à l'horlage et au sample rate

	unsigned int Inter_Cnt;		// Interpolation Counter
	unsigned int Inter_Step;	// Interpolation Step
	channel_ CHANNEL[6];	// Les 6 voies du YM2612

	int REG[2][0x100];	// Sauvegardes des valeurs de tout les registres, c'est facultatif
				// cela nous rend le débuggage plus facile
} ym2612_;

/* Gens */

extern int YM2612_Enable;
extern int YM2612_Improv;
extern int DAC_Enable;
extern int *YM_Buf[2];
extern int YM_Len;

/* end */

int YM2612_Init(int clock, int rate, int interpolation);
int YM2612_End(void);
int YM2612_Reset(void);
uint8_t YM2612_Read(void);
int YM2612_Write(unsigned int adr, uint8_t data);
void YM2612_Update(int **buf, int length);

/* Gens */

void YM2612_DacAndTimers_Update(int **buffer, int length);
void YM2612_Special_Update(void);
int YM2612_Get_Reg(int regID);

/* Savestate functionality. */
int YM2612_Save(unsigned char SAVE[0x200]);
int YM2612_Restore(unsigned char SAVE[0x200]);

/* GSX v7 savestate functionality. */
// struct _gsx_v7_ym2612;
// int YM2612_Save_Full(struct _gsx_v7_ym2612 *save);
// int YM2612_Restore_Full(struct _gsx_v7_ym2612 *save);

void YM2612_ClearBuffer(int **buffer, int length);
/* end */

#ifdef __cplusplus
}
#endif

#endif /* GENS_YM2612_HPP */
