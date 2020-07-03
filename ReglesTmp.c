#include <stdlib.h>
#include <gtk/gtk.h>
#include "Tarot_Ui_Objects.h"
#include <assert.h>
#include <math.h>


//	Mise a jour des probabilités en cours d'échange
//	Cette fonction est appelée juste avant que le joueur joue

void CalcTmpProbCarte(TarotGame CurrentGame)
{
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[CurrentGame->JoueurCourant];
int IndexJoueur = (CurrentGame->JoueurCourant - CurrentGame->JoueurEntame) & 3;
int IndexPreneur =(CurrentGame->JoueurPreneur - CurrentGame->JoueurEntame) & 3;
int i, j;
int h;
int ph;
int CouleurDemandee;
int First = 0;
int pos;
double ProbAtout;

	if ( IndexJoueur == 0 ) return;	//	Pas encore de carte jouée
#if DEBUG > 0
    if ( IndexJoueur > 1 && Table[2].Index == 28 )
        i = 0;
#endif // DEBUG
    pos = CurrentGame->JoueurEntame;
	for ( i = 0; i < IndexJoueur; i++ )	//	Mise a jour proba des cartes jouées
	{
		pJeu->TmpProbCarte[pos][Table[i].Index] = 0.0;
		pJeu->TmpProbCarte[pos^1][Table[i].Index] = 0.0;
		pJeu->TmpProbCarte[pos^2][Table[i].Index] = 0.0;
		pJeu->TmpProbCarte[pos^3][Table[i].Index] = 0.0;
		pJeu->TmpProbCarte[4][Table[i].Index] = 0.0;
		pos = (pos +1) & 3;
	}
    //	Maintenant mise a jour des probas temporaires
    //	Règle 1tmp : si défense, et "sait" que l'un de ses partenaires n'a pas d'atout
    //	si ne joue pas l'atout maître, ne le possède pas
	h = CarteMaitreNonJoueeNoErr(CurrentGame, ATOUT);
	if ( h > 0 && IndexPreneur != 0 && Table[0].Couleur == ATOUT && Table[0].Hauteur != h )
    {
        if ( pJeu->StatusChasse == CHASSE_PRENEUR_DESSOUS || pJeu->StatusChasse == CHASSE_PRENEUR_GROS )
        {
            //  Preneur chasse, joueur X ne joue pas l'atout maître, ne l'a pas...
            BaisseTmpProb(pJeu, CurrentGame->JoueurEntame, h, 0.85, 1.1);
        }
		if  ( pJeu->PositionJoueur != pJeu->PositionPreneur && NbPossCouleur(pJeu, ATOUT) < 2 )
        {
            //  Plus que 2 à avoir de l'atout, ne joue pas le maître.
            BaisseTmpProb(pJeu, CurrentGame->JoueurEntame, h, 0.85, 1.2);
        }
		if ( IndexJoueur == IndexPreneur && NbPossCouleur(pJeu, ATOUT) < 3 )
        {
            BaisseTmpProb(pJeu, CurrentGame->JoueurEntame, h, 0.85, 1.3);
        }
    }
    //	Règle 2.tmp : si entame atout et petit pas chez preneur, si ne joue pas maître ne l'a pas
	if ( h > 0 && IndexPreneur != 0 && Table[0].Couleur == ATOUT && CurrentGame->CarteJouee[1] < 0 && JoueurAvecPetit(CurrentGame, pJeu) != pJeu->PositionPreneur )
	{
		for ( j = 0; j < IndexJoueur; j++)
		{
			if ( ((j+CurrentGame->JoueurEntame)&3) == pJeu->PositionPreneur ) continue;
			BaisseTmpProb(pJeu, (CurrentGame->JoueurEntame+j)&3, h, 0.85, 2.0);
		}
	}
    //	Règle 3 tmp : si un défenseur avant le preneur joue dans une couleur la carte non maîtresse
    //	il ne la possède pas si la couleur a été ouverte et proba coupe preneur faible
	CouleurDemandee = Table[0].Couleur;
	if ( CouleurDemandee == EXCUSE )
	{
	    if ( IndexJoueur <= 1 ) return;
		CouleurDemandee = Table[1].Couleur;
		First = 1;
	}
	if ( CouleurDemandee > ATOUT && pJeu->JoueurCouleur[CouleurDemandee] >= 0 && pJeu->ProbCoupe[CouleurDemandee] < 0.8 )
	{
		h = CarteMaitreNonJoueeNoErr(CurrentGame, CouleurDemandee);
		for ( int j = 0; j < IndexPreneur; j++)
		{
			if ( j >= IndexJoueur ) break;
			if ( j < First ) continue;
			if ( h > 0 && Table[j].Couleur == CouleurDemandee && IsMaitre(CurrentGame, pJeu, &Table[j]) == 0 )
			{
				BaisseTmpProb(pJeu, (j + CurrentGame->JoueurEntame) & 3, h , 0.75 - 0.5*pJeu->ProbCoupe[CouleurDemandee], 3.0);
			}
		}
	}
    //	Règle 4 : tmp si ouverture d'une couleur et signalisation
	if ( FlagSignalisation && IndexPreneur != 0 && CouleurDemandee > ATOUT && pJeu->JoueurCouleur[CouleurDemandee] < 0 )
	{
		if ( Table[0].Hauteur >= 6 )        //  Pas le roi ni la dame dans ce cas (surtout si entame du fond).
		{
			BaisseTmpProb( pJeu, CurrentGame->JoueurEntame, Startof[CouleurDemandee] + DAME - 1, 0.55+IndexPreneur*0.1, 4.0);
			BaisseTmpProb( pJeu, CurrentGame->JoueurEntame, Startof[CouleurDemandee] + ROI - 1, 0.55+IndexPreneur*0.1, 4.0);
		}
		else
		{
			if ( HasCarte(pJeu, pJeu->PositionJoueur, Startof[CouleurDemandee] + DAME - 1) )
			{
				MonteTmpProb(pJeu, CurrentGame->JoueurEntame, Startof[CouleurDemandee] + ROI - 1, 2.0, 4.0);
			}
			else if ( HasCarte(pJeu, pJeu->PositionJoueur, Startof[CouleurDemandee] + ROI - 1))
			{
				MonteTmpProb(pJeu, CurrentGame->JoueurEntame, Startof[CouleurDemandee] + DAME - 1, 2.0, 4.0);
			}
			else
			{
				MonteTmpProb(pJeu, CurrentGame->JoueurEntame, Startof[CouleurDemandee] + DAME - 1, 1.4, 4.0);
				MonteTmpProb(pJeu, CurrentGame->JoueurEntame, Startof[CouleurDemandee] + ROI - 1, 1.4, 4.0);
			}
		}
	}
    //	Regle 5 Tmp : Si joue signalisation et descend dans les cartes et couleur jouée défense => doubleton
	for ( i = 1; i < IndexJoueur; i++)
	{
		pos = (i + CurrentGame->JoueurEntame) & 3;
		if ( FlagSignalisation && pos != pJeu->PositionPreneur && LookForSigDescendant(CurrentGame, pos, CouleurDemandee)
			&& Table[i].Couleur > ATOUT && Table[i].Couleur == CouleurDemandee && pJeu->JoueurCouleur[CouleurDemandee] != pJeu->PositionPreneur )
		{
			for ( j = Startof[CouleurDemandee]; j < Endof[CouleurDemandee]; j++ )
				BaisseTmpProb(pJeu, pos, j, 0.85, 5.0);
		}
	}
    //	Regle 6 Tmp : Si joue signalisation et descend dans les cartes et couleur jouée attaque
    //	=> Tient la couleur du Preneur
	for ( i = 1; i < IndexJoueur; i++)
	{
		pos = (i + CurrentGame->JoueurEntame) & 3;
		if ( FlagSignalisation && pos != pJeu->PositionPreneur && LookForSigDescendant(CurrentGame, pos, CouleurDemandee)
			&& Table[i].Couleur > ATOUT && Table[i].Couleur == CouleurDemandee && pJeu->JoueurCouleur[CouleurDemandee] == pJeu->PositionPreneur )
		{
			for ( j = Startof[CouleurDemandee]; j < Endof[CouleurDemandee]; j++ )
				MonteTmpProb(pJeu, pos, j, 1.2, 6.0);
            //	De plus ne possède pas le petit...
            BaisseTmpProb(pJeu, pos, 1, 0.9, 6.0);
		}
	}
    //	Regle 7 Tmp : Si preneur joue cavalier en premier ou en second sur la table possède sans doute la dame
	if ( IndexJoueur >= 1 &&  IndexPreneur == 0  && CouleurDemandee > ATOUT
		&& Table[IndexPreneur].Couleur == CouleurDemandee && Table[IndexPreneur].Hauteur == CAVALIER && pJeu->JoueurCouleur[CouleurDemandee] < 0)
	{
		MonteTmpProb(pJeu, pJeu->PositionPreneur, Startof[CouleurDemandee]+DAME-1, 5.0, 7.0);
	}
	else if ( IndexJoueur >= 1 &&  IndexPreneur == 0  && CouleurDemandee > ATOUT
		&& Table[IndexPreneur].Couleur == CouleurDemandee && Table[IndexPreneur].Hauteur == VALET && pJeu->JoueurCouleur[CouleurDemandee] < 0)
	{   //Ouverture valet, monte proba cavalier et dame
		MonteTmpProb(pJeu, pJeu->PositionPreneur, Startof[CouleurDemandee]+CAVALIER-1, 4.0, 7.1);
		MonteTmpProb(pJeu, pJeu->PositionPreneur, Startof[CouleurDemandee]+DAME-1, 4.0, 7.1);
	}
	else if ( IndexJoueur >= 2 &&  IndexPreneur == 1  && CouleurDemandee > ATOUT
		&& Table[IndexPreneur].Couleur == CouleurDemandee && Table[IndexPreneur].Hauteur == CAVALIER && pJeu->JoueurCouleur[CouleurDemandee] < 0)
	{
		MonteTmpProb(pJeu, pJeu->PositionPreneur, Startof[CouleurDemandee]+DAME-1, 2.0, 7.2);
	}
    //	Regle 8 Tmp : Si ATOUT demandé et joue Couleur alors ne possède aucun ATOUT
	if ( CouleurDemandee == ATOUT )
	{
		for ( i = 1; i < IndexJoueur; i++)
		{
			pos = (CurrentGame->JoueurEntame + i) & 3;
			if ( Table[i].Couleur > ATOUT )
			{
				for ( j = 1; j < 22; j++)
				{
				    if ( pJeu->TmpProbCarte[pos][j] > 0)
                        BaisseTmpProb(pJeu, pos, j, 1.0, 8.0);
				}
			}
		}
	}
    //	Regle 9 Tmp : Si Couleur demandée et n'en joue pas alors ne possède pas de carte de la couleur
    //  Si en plus ne joue pas ATOUT, alors plus d'atout non plus
	if ( CouleurDemandee > ATOUT )
	{
		for ( i = 1; i < IndexJoueur; i++)
		{
			pos = (CurrentGame->JoueurEntame + i) & 3;
			if ( Table[i].Couleur != CouleurDemandee && Table[i].Couleur != EXCUSE )
			{
				for ( j = Startof[CouleurDemandee]; j < Endof[CouleurDemandee]; j++)
				{
				    if ( pJeu->TmpProbCarte[pos][j] > 0)
                        BaisseTmpProb(pJeu, pos, j, 1.0, 9.1);
				}
			}
			if ( Table[i].Couleur != CouleurDemandee && Table[i].Couleur > ATOUT )
            {
				for ( j = 1; j < 22; j++)
				{
				    if ( pJeu->TmpProbCarte[pos][j] <= 0 ) continue;
				    if ( pJeu->TmpProbCarte[pos][j] >= 0.9999 ) continue;
					BaisseTmpProb(pJeu, pos, j, 1.0, 9.2);
				}
			}
		}
	}
    //	Règle 10 Tmp : Si ne monte pas à l'atout, ne possède pas ceux du dessus
    for ( i = 1; i < IndexJoueur; i++)
    {
        ph = PlusFortAtoutJoue(i);
        pos = (CurrentGame->JoueurEntame + h) & 3;
        if ( Table[i].Couleur == ATOUT && Table[i].Index < ph)
        {
            for ( j = ph+1; j < 22; j++)
            {
                BaisseTmpProb(pJeu, pos, j, 1.0, 10.0);
            }
        }
    }
    //  Règle 11 Tmp : Si défenseur avant le preneur et non dernier joue immédiatement en dessous de l'atout maître ne le possède pas.
    if ( CouleurDemandee == ATOUT && IndexPreneur > IndexJoueur )
    {
        for ( i = 0; i < IndexJoueur; i++)
        {
            ph = Table[i].Hauteur;
            if ( NextAtout(CurrentGame, ph) == AtoutMaitre(CurrentGame) )
            {
                pos = (CurrentGame->JoueurEntame + h) & 3;
                BaisseTmpProb(pJeu, pos, AtoutMaitre(CurrentGame), 1.0, 11.0);
            }
        }
    }
    //	Règle 12 Tmp : Si monte très fort possède peut-être petit
	for ( i = 1; i < IndexJoueur; i++)
	{
		pos = (CurrentGame->JoueurEntame + i) & 3;
		h = PlusFortAtoutJoue(i);
		if ( h > 0 && Table[i].Couleur == ATOUT && pos != pJeu->PositionPreneur &&
			pJeu->NbJoue[pos][ATOUT] == 1 && (Table[i].Hauteur - h ) > 6  )
		{
			MonteTmpProb(pJeu, pos, 1, (Table[i].Hauteur - h) * 0.2, 12.0);
		}
	}
//	Règle 14 Tmp : Si le Preneur rejoue dans sa couleur (tenue qui plus est) augmente les probas
	if ( CouleurDemandee > ATOUT && IndexJoueur >= 1 && IndexPreneur == 0 && pJeu->JoueurCouleur[CouleurDemandee] == pJeu->PositionPreneur )
	{
		for ( i = Startof[CouleurDemandee]; i < Endof[CouleurDemandee]; i++)
        {
            if ( pJeu->TmpProbCarte[pJeu->PositionJoueur][i] <= 0 ) continue;
            if ( pJeu->TmpProbCarte[pJeu->PositionJoueur][i] >= 0.9999 ) continue;
			MonteTmpProb( pJeu, pJeu->PositionPreneur, i, 1.5 + (pJeu->CouleurTenue==CouleurDemandee)*0.5, 14.0);
        }
	}
//	Fin des règles, normalise les probas
	NormaliseTmpProba(CurrentGame, pJeu, IndexJoueur);
//	Recalcule les probas de coupe
	CalcTmpProbCoupe(CurrentGame, pJeu, IndexJoueur);
//	Regarde si le preneur a encore de l'atout
	ProbAtout = 0;
	for ( i = 1; i < 22; i++ )
	{
		ProbAtout += pJeu->TmpProbCarte[pJeu->PositionPreneur][i];
	}
	if ( ProbAtout < 1e-6 )
	{
		for ( i = TREFLE; i < NB_COULEUR; i++) pJeu->TmpProbCoupe[i] = 0.0;
		pJeu->AtoutPreneur = 0;
	}
	else
    {
        pJeu->AtoutPreneur = ProbAtout;
    }
}


