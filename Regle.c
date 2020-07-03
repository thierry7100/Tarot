#include <stdlib.h>
#include <gtk/gtk.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include "Tarot_Ui_Objects.h"

#if DEBUG > 0
#define DEBUG_REGLE 1
#else
#define DEBUG_REGLE 0
#endif  // DEBUG

//  Mise à jour des probabilités en voyant la carte "IndexTable" posée sur le tapis
//  Implémente les règles du Tarot pas de supposition ici.

void Regle2Proba(TarotGame CurrentGame, struct _Jeu *pJeu, int IndexTable)
{
int CouleurDemandee;
int pos = (IndexTable + CurrentGame->JoueurEntame) & 3;
int j;
int h;

	if ( Table[IndexTable].Index == 1 )		        //	Petit joue ?
		pJeu->StatusChasse = -1;                    //  Plus la peine de chasser
    pJeu->StatusCartes[Table[IndexTable].Index] = -1;   //  Carte jouée
	if ( Table[0].Couleur == EXCUSE )
	{
		if ( IndexTable == 0 )   return;            //  Ne peut rien dire à ce stade
        CouleurDemandee = Table[1].Couleur;
	}
	else
	{
		CouleurDemandee = Table[0].Couleur;
	}
	if ( pos == pJeu->PositionJoueur ) return;      //  N'apprend rien de sa propre carte.
#if DEBUG_REGLE > 0
    OutDebug("Entrée Regle2Proba : Carte N° %d, Index = %d, pos = %d, PositionJoueur=%d, Couleur demandée=%d\n",
             IndexTable, Table[IndexTable].Index, pos, pJeu->PositionJoueur, CouleurDemandee);
#endif // DEBUG_REGLE
    //	Règle 1 : Si Couleur demandée et joue ATOUT alors ne possède aucune carte de la couleur
	if ( CouleurDemandee != ATOUT && Table[IndexTable].Couleur == ATOUT )
	{
#if DEBUG_REGLE > 0
    OutDebug("Règle 1, Vue de %d, passe à 0 probabilité d'avoir des cartes de la couleur %d pour joueur %d\n", pJeu->PositionJoueur, CouleurDemandee, pos);
#endif // DEBUG_REGLE
		for ( j = Startof[CouleurDemandee]; j < Endof[CouleurDemandee]; j++)
		{
		    if ( CurrentGame->CarteJouee[j] >= 0 ) continue;                    //  Rien à faire, carte déjà jouée
			BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, j, 1.0);        //  Sûr de ne plus en avoir
		}
		if ( pos == CurrentGame->JoueurPreneur )
		{   //  SI preneur mise à jour des coupes
			pJeu->GuessProbCoupe[CouleurDemandee] = 1.0;		//	En est sur...
			pJeu->ProbCoupe[CouleurDemandee] = 1.0;		//	En est sur...
			j = pJeu->NbJoue[CurrentGame->JoueurPreneur][CouleurDemandee];
			if ( j == 0 )
				pJeu->CoupeCouleur[CouleurDemandee] = 1;		//	Coupe Franche
			if ( j == 1 )
				pJeu->CoupeCouleur[CouleurDemandee] = -1;		//	Singlette
		}
	}
    //	Règle 2 : Si ATOUT demandé et joue Couleur alors ne possède aucun ATOUT
	if ( CouleurDemandee == ATOUT && Table[IndexTable].Couleur > ATOUT )
	{
#if DEBUG_REGLE > 0
    OutDebug("Règle 2, Vue de %d, passe à 0 probabilité d'avoir des atouts pour joueur %d\n", pJeu->PositionJoueur, pos);
#endif // DEBUG_REGLE
		for ( j = 1; j < 22; j++)
		{
		    if ( CurrentGame->CarteJouee[j] >= 0 ) continue;                    //  Rien à faire, carte déjà jouée
			BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, j, 1.0);        //  Sûr de ne plus en avoir
		}
	}
    //	Regle 3 : Si Couleur demandée et joue Autre Couleur alors ne possède aucune carte de la couleur et aucun ATOUT
	if ( CouleurDemandee > ATOUT && Table[IndexTable].Couleur != CouleurDemandee && Table[IndexTable].Couleur > ATOUT )
	{
#if DEBUG_REGLE > 0
    OutDebug("Règle 3, Vue de %d, passe à 0 probabilité d'avoir des atouts et des cartes de la couleur %d pour joueur %d\n", pJeu->PositionJoueur, CouleurDemandee, pos);
#endif // DEBUG_REGLE
		for ( j = Startof[CouleurDemandee]; j < Endof[CouleurDemandee]; j++)
		{
		    if ( CurrentGame->CarteJouee[j] >= 0 ) continue;                    //  Rien à faire, carte déjà jouée
			BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, j, 1.0);        //  Sûr de ne plus en avoir
		}
		for ( j = 1; j < 22; j++)
		{
		    if ( CurrentGame->CarteJouee[j] >= 0 ) continue;                    //  Rien à faire, carte déjà jouée
			BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, j, 1.0);        //  Sûr de ne plus en avoir
		}
		if ( pos == CurrentGame->JoueurPreneur )
		{
			pJeu->GuessProbCoupe[CouleurDemandee] = 1.0;		//	En est sur...
			j = pJeu->NbJoue[CurrentGame->JoueurPreneur][ CouleurDemandee];
			if ( j == 0 )
				pJeu->CoupeCouleur[CouleurDemandee] = 1;		//	Coupe Franche
			if ( j == 1 )
				pJeu->CoupeCouleur[CouleurDemandee] = -1;		//	Singlette
		}
	}
//	Regle 4 : Si Atout joué plus faible que précédent, alors ne possède pas au-dessus
	if ( (h = PlusFortAtoutJoue(IndexTable))> 0 && Table[IndexTable].Couleur == ATOUT && h > Table[IndexTable].Hauteur )
	{
#if DEBUG_REGLE > 0
    OutDebug("Règle 4, Passe à 0 probabilité d'avoir des atouts au dessus du %d pour joueur %d\n", h, pos);
#endif // DEBUG_REGLE
		for ( j = h+1; j < 22; j++)
		{
		    if ( CurrentGame->CarteJouee[j] >= 0 ) continue;                    //  Rien à faire, carte déjà jouée
			BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, j, 1.0);        //  Sûr de ne pas avoir cette carte
		}
	}
}
