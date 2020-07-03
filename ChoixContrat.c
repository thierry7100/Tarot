#include <stdlib.h>
#include <gtk/gtk.h>
#include <time.h>
#include "Tarot_Ui_Objects.h"

int ModeTournoi = 1;


static const int PtContrat[4][4] = {
	{45, 58, 75, 85},			//	Avec 0 bouts
	{42, 56, 72, 82},			//	Avec 1 bout
	{40, 53, 70, 80},		    //	Avec 2 bouts
	{40, 48, 67, 75} };		    //	Avec 3 bouts



//  Retourne le contrat choisi par le joueur
//	Determine le contrat a réaliser en fonction des points du jeu
//	Tient compte du style de jeu du joueur

int ChoixContrat(TarotGame CurrentGame, int Joueur)
{
double Point;
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[Joueur];
const int *SeuilContrat;

    if ( (FlagPartieEnregistreeAttaque || FlagPartieEnregistreeDefense) && Joueur == CurrentGame->PreneurPartieEnregistree )
    {
        CurrentGame->AnnonceJoueur[Joueur] = GARDE;
        return GARDE;
    }
	if ( Joueur == CurrentGame->JoueurDistrib )         //  Dernier à parler ? Petite prime éventuelle
		Point = pJeu->NbPoint + 2 * (0.5 + StyleJoueur[Joueur][dStylePetiteDernier]);
	else
		Point = pJeu->NbPoint;
	Point = (1.0 + StyleJoueur[Joueur][dStyleRisque] * 0.4) * Point;
    SeuilContrat = &PtContrat[pJeu->NbBout][0];
    if ( pJeu->NbPointDH >= SeuilContrat[GARDE_SANS] && CurrentGame->TypePartie < GARDE_CONTRE )
    {
        CurrentGame->AnnonceJoueur[Joueur] = GARDE_CONTRE;
        return(GARDE_CONTRE);
    }
    if ( pJeu->NbPointDH >= SeuilContrat[GARDE] && CurrentGame->TypePartie < GARDE_SANS )
    {
        CurrentGame->AnnonceJoueur[Joueur] = GARDE_SANS;
        return(GARDE_SANS);
    }
    if ( Point >= SeuilContrat[PETITE] && CurrentGame->TypePartie < GARDE)
    {
        CurrentGame->AnnonceJoueur[Joueur] = GARDE;
        return(GARDE);
    }
    if ( Point >= SeuilContrat[PASSE] && CurrentGame->TypePartie < PETITE)
    {
        CurrentGame->AnnonceJoueur[Joueur] = PETITE;
        return(PETITE);
    }
    CurrentGame->AnnonceJoueur[Joueur] = PASSE;
    return(PASSE);
}
