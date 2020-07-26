#include <stdlib.h>
#include <gtk/gtk.h>
#include "Tarot_Ui_Objects.h"
#include <assert.h>
#include <math.h>

#if DEBUG > 0
#define DEBUG_HEURISTIQUES  0
#else
#define DEBUG_HEURISTIQUES  0
#endif // DEBUG

//----------------------------------------------------------------------------------------------------------------------------
//  Heuristiques avec le PETIT
//----------------------------------------------------------------------------------------------------------------------------

void Heuristique_Petit_Proba(TarotGame CurrentGame, struct _Jeu *pJeu, int IndexTable)
{
int j, h;
int pos = (IndexTable + CurrentGame->JoueurEntame)&3;
int posPreneur = (CurrentGame->JoueurPreneur - CurrentGame->JoueurEntame) & 3;
int CouleurDemandee = Table[0].Couleur;
int nb;
double v;

    if ( pos == pJeu->PositionJoueur ) return;      //  Rien à apprendre de son propre jeu...
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Entrée Heuristique_Petit_Proba, Position %d, examine carte Index %d :", pJeu->PositionJoueur, IndexTable);
        ImprimeCarte(&Table[IndexTable]);
        OutDebug("\n");
        if ( Table[3].Index == 7 )
            j = 0;
#endif // DEBUG_HEURISTIQUES
    //	Heuristique 1 : Si Partenaire joue 21, les suivants mettent le petit (proba 0.95)
	if (Table[IndexTable].Couleur == ATOUT && Table[IndexTable].Hauteur == 21 && pJeu->PositionJoueur != CurrentGame->JoueurPreneur
     && CurrentGame->CarteJouee[1] < 0)
	{
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique 1 : Défenseur joue 21, les suivants mettent le petit\n");
#endif // DEBUG_HEURISTIQUES
		for ( j = IndexTable+1; j < 4; j++ )
		{
			if ( ((j+CurrentGame->JoueurEntame)&3) == CurrentGame->JoueurPreneur ) continue;
			if ( ((j+CurrentGame->JoueurEntame)&3) == pJeu->PositionJoueur ) continue;
			if ( Table[j].Couleur == ATOUT && Table[j].Hauteur != 1 )
            {
				BaisseProba(CurrentGame, pJeu->PositionJoueur, ((j+CurrentGame->JoueurEntame)&3), 1, 0.95 );
#if DEBUG_HEURISTIQUES > 0
                OutDebug("Heuristique 1 : Nouvelle proba ProbCarte[%d][1] = %.3f\n",
                            ((j+CurrentGame->JoueurEntame)&3), pJeu->ProbCarte[((j+CurrentGame->JoueurEntame)&3)][1]);
#endif // DEBUG_HEURISTIQUES
            }
		}
		CopieProba2Tmp(pJeu);
	}
    // Heuristique 2 : Si partenaire joue atout maître les suivants mettent le petit (proba 0.9)
	if ( Table[IndexTable].Couleur == ATOUT && Table[IndexTable].Hauteur != 21 && IsMaitre(CurrentGame, pJeu, &Table[IndexTable])
            && pJeu->PositionJoueur != CurrentGame->JoueurPreneur && CurrentGame->CarteJouee[1] < 0)
	{
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique 2 : Défenseur joue atout maître, les suivants mettent le petit\n");
#endif // DEBUG_HEURISTIQUES
		for ( j = IndexTable+1; j < 4; j++ )
		{
			if ( ((j+CurrentGame->JoueurEntame)&3) == CurrentGame->JoueurPreneur ) continue;
			if ( ((j+CurrentGame->JoueurEntame)&3) == pJeu->PositionJoueur ) continue;
			if ( Table[j].Couleur == ATOUT && Table[j].Hauteur != 1 )
            {
                if ( pJeu->ProbCarte[((j+CurrentGame->JoueurEntame)&3)][1] < 1.0 )
                {
                    BaisseProba(CurrentGame, pJeu->PositionJoueur, ((j+CurrentGame->JoueurEntame)&3), 1, 0.9 );
#if DEBUG_HEURISTIQUES > 0
                    OutDebug("Heuristique 2 : Nouvelle proba ProbCarte[%d][1] = %.3f\n",
                            ((j+CurrentGame->JoueurEntame)&3), pJeu->ProbCarte[((j+CurrentGame->JoueurEntame)&3)][1]);
#endif // DEBUG_HEURISTIQUES
                }
            }
		}
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique 3 : Si partenaire prend le pli avec ATOUT, suivant joue atout en dessous différent de 1
    //	Ne possède de pas le petit
	if (GainPli(CurrentGame, 3) && IndexTable == 3 && Table[IndexTable].Couleur == ATOUT && pos != CurrentGame->JoueurPreneur
		&& Table[IndexTable].Hauteur < Table[Gagnant(3)].Hauteur && Table[IndexTable].Hauteur != 1 && CurrentGame->CarteJouee[1] < 0 )
	{
	    if ( pJeu->ProbCarte[pos][1] < 0.99999)
        {
            BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, 1, 0.9 );
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique 3, joueur ne joue pas le petit en dernier avec défense maître : Nouvelle proba ProbCarte[%d][1] = %.3f\n",
                    pos, pJeu->ProbCarte[pos][1]);
#endif // DEBUG_HEURISTIQUES
            CopieProba2Tmp(pJeu);
        }
	}
	if (GainPli(CurrentGame, 2) && IndexTable == 2 && Gagnant(3) < 2 && posPreneur < 2 && Table[2].Couleur == ATOUT
		&& Table[2].Hauteur < Table[Gagnant(3)].Hauteur && Table[2].Hauteur != 1 && CurrentGame->CarteJouee[1] < 0 )
	{
        BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, 1, 0.9 );
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique 3, joueur ne joue pas le petit après le preneur avec défense maître : Nouvelle proba ProbCarte[%d][1] = %.3f\n",
                    pos, pJeu->ProbCarte[pos][1]);
#endif // DEBUG_HEURISTIQUES
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique 4 : Si au premier tour d'ATOUT monte très fort, augmente proba PETIT
    //  Ne le fait pas si juste avant le preneur
	h = PlusFortAtoutJoue(IndexTable);
	if ( h > 0 && Table[IndexTable].Couleur == ATOUT && pos != CurrentGame->JoueurPreneur && ((pos+1)&3) != CurrentGame->JoueurPreneur &&
		pJeu->NbJoue[pos][ATOUT] == 1 && (Table[IndexTable].Hauteur - h ) > 6 && CurrentGame->CarteJouee[1] < 0 )
	{
		MonteProba(CurrentGame, pJeu->PositionJoueur, pos, 1,  1.5, 0);
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique 4, monte fort à l'atout, possession petit ? : Nouvelle proba ProbCarte[%d][1] = %.3f\n",
                    pos, pJeu->ProbCarte[pos][1]);
#endif // DEBUG_HEURISTIQUES
		//	A priori, a joué son plus fort, d'autant plus vrai que le joueur est à la fin
		for ( h = Table[IndexTable].Hauteur + 1; h < 22; h++)
		{
			BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, h, 0.3*IndexTable);
		}
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique 5 : Si au premier (max second) tour d'ATOUT derrière preneur maître, met un atout fort possède petit
	if ( pos > posPreneur && GainPli(CurrentGame, IndexTable) == 0 && Table[IndexTable].Couleur == ATOUT && Table[IndexTable].Hauteur > 15
        && pJeu->NbJoue[pos][ATOUT] <= 2 && CurrentGame->CarteJouee[1] < 0)
	{
		MonteProba(CurrentGame, pJeu->PositionJoueur, pos, 1,  1.5, 0);
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique 5, gros atout derrière preneur maître, possession petit ? : Nouvelle proba ProbCarte[%d][1] = %.3f\n",
                    pos, pJeu->ProbCarte[pos][1]);
#endif // DEBUG_HEURISTIQUES
		//	A priori, a joué son plus fort, d'autant plus vrai que le joueur est à la fin
		for ( h = Table[IndexTable].Hauteur + 1; h < 22; h++)
		{
			BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, h, 0.5);
		}
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique 6 : Si joueur ouvre ATOUT, ne possède pas le petit. Proba 0.9 si pas preneur, 0.8 sinon
	if ( IndexTable == 0 && Table[0].Couleur == ATOUT && Table[0].Hauteur != 1 && pJeu->NbCarte >= 15 && CurrentGame->CarteJouee[1] < 0)
	{
		if ( pos == CurrentGame->JoueurPreneur && pJeu->ProbCarte[pos][1] < 1.0 )
        {
			MonteProba(CurrentGame, pJeu->PositionJoueur, pos, 1, 0.5 + (0.3*Table[0].Hauteur < 7), 0 );
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique 6, Preneur ouvre ATOUT, n'a pas le petit : Nouvelle proba ProbCarte[%d][1] = %.3f\n",
                    pos, pJeu->ProbCarte[pos][1]);
#endif // DEBUG_HEURISTIQUES
        }
		else if ( pJeu->ProbCarte[pos][1] < 1.0 )
        {
			BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, 1, 0.9 );
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique 6, Défenseur ouvre ATOUT, n'a pas le petit : Nouvelle proba ProbCarte[%d][1] = %.3f\n",
                    pos, pJeu->ProbCarte[pos][1]);
#endif // DEBUG_HEURISTIQUES
        }
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique 7 : Si coupe en premier après preneur sans mettre le petit, ne le possède pas
	h = PlusFortAtoutJoue(IndexTable);
	if ( IndexTable > posPreneur && CouleurDemandee > ATOUT && h <= 0 && Table[IndexTable].Couleur == ATOUT
		&&  Table[IndexTable].Hauteur > 1 && CurrentGame->CarteJouee[1] < 0)
	{
	    if ( pJeu->ProbCarte[pos][1] < 0.99999)
        {
            BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, 1, 0.9 );
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique 7, Défenseur coupe en premier après le preneur sans mettre petit, n'a pas le petit : Nouvelle proba ProbCarte[%d][1] = %.3f\n",
                    pos, pJeu->ProbCarte[pos][1]);
#endif // DEBUG_HEURISTIQUES
            CopieProba2Tmp(pJeu);
        }
	}
    //	Heuristique 8 : Si Preneur joue le petit sur un tour d'atout n'en possède plus
	if ( CouleurDemandee == ATOUT && pos == posPreneur && Table[IndexTable].Couleur == ATOUT &&
		Table[IndexTable].Hauteur == 1 && posPreneur > 0 )
	{
        BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, 0, 0.95 );
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique 8, Preneur joue le petit sur ATOUT demandé, n'en possède plus : Nouvelle proba ProbCarte[%d][0] = %.3f\n",
                    pos, pJeu->ProbCarte[pos][0]);
#endif // DEBUG_HEURISTIQUES
		for ( h = 2; h < 22; h++)
		{
		    if ( pJeu->ProbCarte[pos][h] <= 0.00001 ) continue;
            BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, h, 0.95 );
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique 8, Preneur joue le petit sur ATOUT demandé, n'en possède plus : Nouvelle proba ProbCarte[%d][%d] = %.3f\n",
                        pos, h, pJeu->ProbCarte[pos][h]);
#endif // DEBUG_HEURISTIQUES
		}
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique 9 : Si sur un atout "assez faible", met en dessous et après le preneur et non dernier
    //	ne possède pas le petit.
	if ( (h = PlusFortAtoutJoue(IndexTable))> 0 && Table[IndexTable].Couleur == ATOUT && h > Table[IndexTable].Hauteur
            && IndexTable > posPreneur && (v = ProbRestantAuDessus(pJeu, h, pos, IndexTable)) > 1 && CurrentGame->CarteJouee[1] < 0)
	{
        BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, 1, 0.9 - 0.25/v );
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique 9, Défenseur joue en dessous sans mettre le petit, ne l'a pas : Nouvelle proba ProbCarte[%d][%d] = %.3f\n",
                        pos, 1, pJeu->ProbCarte[pos][1]);
#endif // DEBUG_HEURISTIQUES
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique 10: Si un défenseur met le petit après le preneur sur l'atout maître, n'en possède plus
	if ( (h = PlusFortAtoutJoue(IndexTable))> 0 && Table[IndexTable].Couleur == ATOUT && Table[IndexTable].Hauteur == 1 && h > 1
		&& IndexTable > posPreneur && (v = ProbRestantAuDessus(pJeu, h, pos, IndexTable)) < 0.1 && !GainPli(CurrentGame, IndexTable))
	{
	    if ( pJeu->ProbCarte[pos][h] >= 0.000001 && pJeu->ProbCarte[pos][h] <= 0.99999 )
        {
            BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, 0, 0.95 - v*5.0 );
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique 10, Défenseur met le petit, ne peut rien mettre d'autre : Nouvelle proba ProbCarte[%d][%d] = %.3f\n",
                        pos, 0, pJeu->ProbCarte[pos][0]);
#endif // DEBUG_HEURISTIQUES
        }
		for ( h = 2; h < 22; h++)
		{
		    if ( pJeu->ProbCarte[pos][h] <= 0.000001 ) continue;
		    if ( pJeu->ProbCarte[pos][h] >= 0.999999 ) continue;
            BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, h, 0.95 - v*5.0 );
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique 10, Défenseur met le petit, ne peut rien mettre d'autre : Nouvelle proba ProbCarte[%d][%d] = %.3f\n",
                        pos, h, pJeu->ProbCarte[pos][h]);
#endif // DEBUG_HEURISTIQUES
		}
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique 11 : Si partenaire descend à l'atout, possède le petit
	if ( IndexTable > 0 && IndexTable != posPreneur  && LookForPetit(CurrentGame, pos) && CurrentGame->CarteJouee[1] < 0)
	{
		MonteProba(CurrentGame, pJeu->PositionJoueur, pos, 1, 8.0, 0);
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique 11, Défenseur Descend à l'atout monte proba petit : Nouvelle proba ProbCarte[%d][%d] = %.3f\n",
                        pos, 1, pJeu->ProbCarte[pos][1]);
#endif // DEBUG_HEURISTIQUES
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique 12 : Si Joue première carte et pas ATOUT alors monte proba petit, surtout si pas d'atout au chien
	//	Monte proba si AtoutChien <= 1
	if ( pJeu->NbCarte == 17 && IndexTable == 0 && pos != CurrentGame->JoueurPreneur && Table[0].Couleur != ATOUT &&
		CurrentGame->CarteJouee[1] < 0 && (nb=MinLongueur(pJeu, CurrentGame->JoueurPreneur, ATOUT)) <= 1 )
	{
	    //  Surtout vrai si au fond, juste devant intérêt à ouvrir
		nb = 2 - nb;
        MonteProba(CurrentGame, pJeu->PositionJoueur, CurrentGame->JoueurEntame, 1, 1.05 + posPreneur*posPreneur*0.1 + nb*nb*0.06, 0 );
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique 12, Si Joue première carte et pas ATOUT alors monte proba petit ProbCarte[%d][%d] = %.3f\n",
                    pos, 1, pJeu->ProbCarte[pos][1]);
#endif // DEBUG_HEURISTIQUES
        CopieProba2Tmp(pJeu);
	}

	if ( IndexTable == posPreneur && CouleurDemandee != ATOUT && Table[IndexTable].Couleur == ATOUT )
        pJeu->NbAtoutSurCoupe++;
    if ( IndexTable == posPreneur && CouleurDemandee != ATOUT && Table[IndexTable].Index == 1)
    {
        pJeu->PetitSurCoupe = pJeu->NbAtoutSurCoupe;
        if ( pJeu->PetitSurCoupe <= 2 )
        {
            //  Descend proba ATOUT si petit mis rapidement
            for ( h = 1; h <22; h++ )
            {
                if ( pJeu->ProbCarte[pos][h] < 0.00001 ) continue;
                if ( pJeu->ProbCarte[pos][h] > 0.99999 ) continue;
                BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, h, 0.25/pJeu->PetitSurCoupe );
#if DEBUG_HEURISTIQUES > 0
                OutDebug("Heuristique Petit 13, Preneur joue le petit rapidement, baisse proba ATOUT : Nouvelle proba ProbCarte[%d][%d] = %.3f\n",
                        pos, h, pJeu->ProbCarte[pos][h]);
#endif // DEBUG_HEURISTIQUES
            }
            CopieProba2Tmp(pJeu);
		}
    }
}

//----------------------------------------------------------------------------------------------------------------------------
//  Heuristiques liées à la chasse
//----------------------------------------------------------------------------------------------------------------------------

void Heuristique_Chasse(TarotGame CurrentGame, struct _Jeu *pJeu, int IndexTable)
{
int j, h;
int pos = (IndexTable + CurrentGame->JoueurEntame)&3;
int posPreneur = (CurrentGame->JoueurPreneur - CurrentGame->JoueurEntame) & 3;
int CouleurDemandee = Table[0].Couleur;
double v;

    if ( pos == pJeu->PositionJoueur ) return;      //  Rien à apprendre de son propre jeu...
#if DEBUG_HEURISTIQUES > 0
    OutDebug("Entrée Heuristique_Chasse, Position %d, examine carte Index %d :", pJeu->PositionJoueur, IndexTable);
    ImprimeCarte(&Table[IndexTable]);
    OutDebug("\n");
#endif // DEBUG_HEURISTIQUES
    //	Heuristique Chasse 1 : Si défenseur part a la chasse et petit non dans le chien sans jouer le 21, ne l'a pas
	if ( IndexTable == 0 && CurrentGame->JoueurEntame != CurrentGame->JoueurPreneur && Table[0].Couleur == ATOUT && pJeu->StatusChasse == 0
		&& pJeu->ProbCarte[CurrentGame->JoueurPreneur][1] != 1.0 && pJeu->NbCarte > 15 )
	{
	    //  Surtout vrai si devant le preneur
	    if ( FlagSignalisation && (Table[0].Hauteur & 1) == 1 )
            BaisseProba(CurrentGame, pJeu->PositionJoueur, CurrentGame->JoueurEntame, 21, 0.5 - (posPreneur-1)*0.1);
        else
            BaisseProba(CurrentGame, pJeu->PositionJoueur, CurrentGame->JoueurEntame, 21, 0.9 - (posPreneur-1)*0.25);
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique Chasse 1, Lance chasse sans 21, baisse proba 21 ProbCarte[%d][%d] = %.3f\n",
                        pos, 21, pJeu->ProbCarte[pos][21]);
#endif // DEBUG_HEURISTIQUES
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique Chasse 2 : Si Chasse commencée par Défense ne joue pas ATOUT, monte proba petit ou n'a plus d'atout
	if ( pJeu->StatusChasse == CHASSE_DEFENSE_IMPAIR && IndexTable == 0 && pos != CurrentGame->JoueurPreneur && Table[0].Couleur != ATOUT  )
	{
		if ( pJeu->CouleurEntameParJoueur[pos] & (1<<ATOUT) )	//	Déjà joue de l'atout, parie sur plus d'atout
		{
			v = 0.8;
			for ( h = 1; h < 22; h++)
			{
				BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, h, v);		//	A priori, n'en a plus
#if DEBUG_HEURISTIQUES > 0
                OutDebug("Heuristique Chasse 2, ne rejoue pas atout après Chasse, plus d'atout... ProbCarte[%d][%d] = %.3f\n",
                                pos, h, pJeu->ProbCarte[pos][h]);
#endif // DEBUG_HEURISTIQUES
			}
		}
		else
		{
            j = pJeu->NbJoue[pos][ATOUT];
            if ( j < 5 )
                MonteProba(CurrentGame, pJeu->PositionJoueur, pos, 1, 3.0 - j*0.33, 0);	//	A sans doute le petit
            pJeu->ValFlanc[ATOUT] = 0.0;                   //   Ne plus jouer ATOUT
            pJeu->ValFlanc[CouleurDemandee] = 1.0;          //  SUivre demande du possesseur du PETIT
            pJeu->StatusChasse = CHASSE_TERMINEE;			//	Arrête la chasse !
		}
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique chasse 3 : si un défenseur poursuit la chasse, il n'a pas le petit.
	if ( pJeu->StatusChasse >= CHASSE_DEFENSE_JOUE_21 && IndexTable == 0 && CouleurDemandee == ATOUT )
	{
		BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, 1, 0.85);
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique Chasse 3, Relance atout sur chasse, ne possède pas le petit... ProbCarte[%d][%d] = %.3f\n",
                                pos, 1, pJeu->ProbCarte[pos][1]);
#endif // DEBUG_HEURISTIQUES
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique chasse 4 : si défenseur joue excuse au premier tour d'atout, possède petit
	if ( CouleurDemandee == ATOUT && Table[IndexTable].Couleur == EXCUSE && pJeu->JoueurCouleur[ATOUT] < 0 && CurrentGame->CarteJouee[1] < 0 )
	{
        MonteProba(CurrentGame, pJeu->PositionJoueur, pos, 1, 5.0, 0);
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique Chasse 4, Excuse sur premier tour d'atout possède le petit... ProbCarte[%d][%d] = %.3f\n",
                                pos, 1, pJeu->ProbCarte[pos][1]);
#endif // DEBUG_HEURISTIQUES
        if ( posPreneur != 0 && pJeu->PositionJoueur != CurrentGame->JoueurPreneur )
        {
            //  Chasse lancée par la défense...
            pJeu->StatusChasse = CHASSE_TERMINEE;
            pJeu->ValFlanc[ATOUT] = 0;
        }
		CopieProba2Tmp(pJeu);
	}
    // Heuristique chasse 5 : Si défenseur ne joue pas l'atout maître après chasse entamée par preneur ne le possède pas
    //  D'autant plus vrai que près du fond...
	h = CarteMaitreNonJoueeNoErr(CurrentGame, ATOUT);
    if ( h >= 0 && IndexTable == 0 && posPreneur != 0 && (pJeu->StatusChasse == CHASSE_PRENEUR_DESSOUS || pJeu->StatusChasse == CHASSE_PRENEUR_GROS)
        && pJeu->ProbCarte[CurrentGame->JoueurPreneur][1] < 0.75 && Table[0].Index != h && CurrentGame->CarteJouee[1] < 0
        && pJeu->ProbCarte[pos][h] < 1.0 &&  pJeu->ProbCarte[pos][h] > 0.0)
	{
		BaisseProba(CurrentGame, pJeu->PositionJoueur, CurrentGame->JoueurEntame, h, (posPreneur*posPreneur)*0.1 );
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique Chasse 5, Ne joue pas atout maître après chasse preneur... ProbCarte[%d][%d] = %.3f\n",
                                pos, h, pJeu->ProbCarte[pos][h]);
#endif // DEBUG_HEURISTIQUES
		CopieProba2Tmp(pJeu);
	}
}


//----------------------------------------------------------------------------------------------------------------------------
//  Heuristiques liées à l'ATOUT
//----------------------------------------------------------------------------------------------------------------------------

void Heuristique_Atout(TarotGame CurrentGame, struct _Jeu *pJeu, int IndexTable)
{
int j, h;
int pos = (IndexTable + CurrentGame->JoueurEntame)&3;
int posPreneur = (CurrentGame->JoueurPreneur - CurrentGame->JoueurEntame) & 3;
int CouleurDemandee = Table[0].Couleur;

    if ( pos == pJeu->PositionJoueur ) return;      //  Rien à apprendre de son propre jeu...
#if DEBUG_HEURISTIQUES > 0
    OutDebug("Entrée Heuristique_Atout, Position %d, examine carte Index %d :", pJeu->PositionJoueur, IndexTable);
    ImprimeCarte(&Table[IndexTable]);
    OutDebug("\n");
#endif // DEBUG_HEURISTIQUES
    //	Heuristique ATOUT 1 : Si défenseur après le preneur coupe avec un gros ATOUT et petit tombé ne
    //	possède pas les plus petit
	if ( IndexTable > posPreneur && Table[IndexTable].Couleur == ATOUT && CouleurDemandee != ATOUT &&
		(IsJoue(CurrentGame, 1) || HasCarte(pJeu, CurrentGame->JoueurPreneur, 1) || Table[IndexTable].Hauteur < 15)  )
	{
		h = PlusFortAtoutJoue(IndexTable);
		for ( j = h+1; j < Table[IndexTable].Hauteur; j++)
		{
		    if ( pJeu->ProbCarte[pos][j] <= 0 ) continue;
		    if ( pJeu->ProbCarte[pos][j] > 0.99999999 ) continue;
			BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, j, 0.80 * ( 1.0 - 0.3*j/Table[IndexTable].Hauteur ));
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique Atout 1, Coupe fort après preneur ne possède pas les petits atouts... ProbCarte[%d][%d] = %.3f\n",
                                pos, j, pJeu->ProbCarte[pos][j]);
#endif // DEBUG_HEURISTIQUES
		}
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique ATOUT 2 ; Si défense entame atout non maître alors qu'un partenaire peut se défausser
    //	ne le possède pas.
	h = CarteMaitreNonJoueeNoErr(CurrentGame, ATOUT);
	if ( h > 0 && posPreneur != 0 && IndexTable == 0 && Table[0].Couleur == ATOUT && Table[0].Hauteur != h
		&& NbPossCouleur(pJeu, ATOUT) < 2+(pJeu->PositionJoueur==CurrentGame->JoueurPreneur)  )
	{
        if ( pJeu->ProbCarte[pos][h] > 0 )
        {
		    if ( pJeu->ProbCarte[pos][h] > 0 && pJeu->ProbCarte[pos][h] < 0.99999999 )
            {
                BaisseProba(CurrentGame, pJeu->PositionJoueur, CurrentGame->JoueurEntame, h, 0.8);
#if DEBUG_HEURISTIQUES > 0
                OutDebug("Heuristique Atout 2, Ne joue pas atout maître pour défausse... ProbCarte[%d][%d] = %.3f\n",
                                pos, j, pJeu->ProbCarte[pos][j]);
#endif // DEBUG_HEURISTIQUES
            }
        }
	}
}

//----------------------------------------------------------------------------------------------------------------------------
//  Heuristiques liées à la Couleur
//----------------------------------------------------------------------------------------------------------------------------

void Heuristique_Couleur(TarotGame CurrentGame, struct _Jeu *pJeu, int IndexTable)
{
int j, h, c;
int pos = (IndexTable + CurrentGame->JoueurEntame)&3;
int posPreneur = (CurrentGame->JoueurPreneur - CurrentGame->JoueurEntame) & 3;
int CouleurDemandee = Table[0].Couleur;
int idxGagnant;
double v, v1;
int PossPetit;

    if ( pos == pJeu->PositionJoueur ) return;      //  Rien à apprendre de son propre jeu...
#if DEBUG_HEURISTIQUES > 0
    OutDebug("Entrée Heuristique_Couleur Demandée %d, Position %d, examine carte Index %d :", CouleurDemandee, pJeu->PositionJoueur, IndexTable);
    ImprimeCarte(&Table[IndexTable]);
    OutDebug("\n");
#endif // DEBUG_HEURISTIQUES
#if DEBUG > 0
    if ( pJeu->PositionJoueur == 1)
        j = 0;
#endif // DEBUG
	//  Ne joue pas ATOUT sur Flanc ATOUT
	//  Possède Petit ?
    if ( IndexTable == 0 && pJeu->PositionJoueur != CurrentGame->JoueurPreneur && pJeu->Flanc == ATOUT
        && pJeu->ValFlanc[ATOUT] > 0.9 && CouleurDemandee != ATOUT )
    {
        if ( CurrentGame->CarteJouee[1] < 0 && AvgLongueur(pJeu, pos, ATOUT) > 2 )
        {
            MonteProba(CurrentGame, pJeu->PositionJoueur, pos, 1, 2.0, 0);
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique Couleur 0, Monte proba petit si ne suit pas ATOUT sur Flanc ATOUT. ProbCarte[%d][%d] = %.3f\n",
                pos, 1, pJeu->ProbCarte[pos][1]);
#endif // DEBUG_HEURISTIQUES
            pJeu->Flanc = CouleurDemandee;       //  Ne joue plus ATOUT, mais suit possesseur Petit !
            pJeu->Obeissance[CurrentGame->JoueurEntame] = 0.8;
            pJeu->ValFlanc[CouleurDemandee] = 0.8;
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique Couleur 0.1, Change Flanc ATOUT vers %d\n", CouleurDemandee);
#endif // DEBUG_HEURISTIQUES
        }
    }

    //	Heuristique Couleur 1 : Si adversaire emporte le pli et le dernier joue un honneur ne possède de cartes plus petite
	if ( !GainPli(CurrentGame, 3)&& Table[IndexTable].Couleur > ATOUT && IndexTable == 3 && Table[IndexTable].Hauteur >= VALET )
	{
		for ( j = Startof[Table[IndexTable].Couleur]; j < Table[IndexTable].Index; j++)
		{
		    if ( pJeu->ProbCarte[pos][j] <= 0.00001 ) continue;
		    if ( pJeu->ProbCarte[pos][j] >= 0.99999 ) continue;
			BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, j, 0.95);
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique Couleur 1, Si joue un honneur en dernier sur pli perdu, ne possède pas en dessous.. ProbCarte[%d][%d] = %.3f\n",
                            pos, j, pJeu->ProbCarte[pos][j]);
#endif // DEBUG_HEURISTIQUES
		}
		CopieProba2Tmp(pJeu);
	}
	idxGagnant = Gagnant(3);
    //	Heuristique 2 : Si emporte pli en dernier avec un honneur, ne possède pas l'honneur immédiatement supérieur
	if ( GainPli(CurrentGame, 3) && Table[IndexTable].Couleur > ATOUT && IndexTable == 3 && Table[IndexTable].Hauteur >= 10
		&& Table[IndexTable].Hauteur < ROI && idxGagnant == 3 )
	{
		BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, Table[IndexTable].Index+1, 0.95);
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique Couleur 2, Si remporte un pli avec un honneur en dernier ne possède celui juste au dessus.. ProbCarte[%d][%d] = %.3f\n",
                        pos, j, pJeu->ProbCarte[pos][Table[IndexTable].Index+1]);
#endif // DEBUG_HEURISTIQUES
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique 2bis  : Si joue Honneur et emporte le pli avec après Preneur, ne possède pas celle immédiatement au dessus
	if ( GainPli(CurrentGame, IndexTable) && IndexTable > posPreneur && Table[posPreneur].Couleur == CouleurDemandee
		&& Table[IndexTable].Couleur == CouleurDemandee && Table[IndexTable].Hauteur > Table[posPreneur].Hauteur
		&& Table[IndexTable].Hauteur >= VALET && Table[IndexTable].Hauteur < ROI && idxGagnant != 3 && idxGagnant == IndexTable)
	{
		BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, Table[IndexTable].Index+1, 0.9);
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique Couleur 2bis, Si remporte un pli avec un honneur après preneur ne possède celui juste au dessus.. ProbCarte[%d][%d] = %.3f\n",
                        pos, Table[IndexTable].Index+1, pJeu->ProbCarte[pos][Table[IndexTable].Index+1]);
#endif // DEBUG_HEURISTIQUES
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique couleur 3 : Si adversaire emporte pli avec DAME, le dernier ne possède pas le ROI
	if ( !GainPli(CurrentGame, 3) && Table[IndexTable].Couleur > ATOUT && IndexTable == 3
		&& Table[idxGagnant].Hauteur == DAME && Table[idxGagnant].Couleur > ATOUT && CurrentGame->CarteJouee[Startof[CouleurDemandee]+13] < 0  )
	{
		BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, Startof[CouleurDemandee]+13, 0.95);
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique Couleur 3, Si adversaire prend un pli avec une Dame le dernier ne possède pas le ROI.. ProbCarte[%d][%d] = %.3f\n",
                        pos, Startof[CouleurDemandee]+13, pJeu->ProbCarte[pos][Startof[CouleurDemandee]+13]);
#endif // DEBUG_HEURISTIQUES
		CopieProba2Tmp(pJeu);
	}
    // Heuristique couleur 4 : Si Preneur met un honneur sur un atout de l'adversaire, ne possède rien dessous
	if ( pos == CurrentGame->JoueurPreneur && !GainPli(CurrentGame, IndexTable) && CouleurDemandee > ATOUT && Table[IndexTable].Couleur == CouleurDemandee
        && IndexTable > idxGagnant && Table[IndexTable].Hauteur >= VALET )
	{
		for ( j = Startof[Table[IndexTable].Couleur]; j < Table[IndexTable].Index; j++)
		{
		    if ( pJeu->ProbCarte[pos][j] <= 0 ) continue;
			BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, j, 0.95);
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique Couleur 4, Si preneur met un honneur sur un pli déjà perdu un pli avec une Dame le dernier ne possède celui juste le ROI.. ProbCarte[%d][%d] = %.3f\n",
                        pos, j, pJeu->ProbCarte[pos][j]);
#endif // DEBUG_HEURISTIQUES
		}
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique couleur 5 : Si joue Signalisation, et entame couleur
    //  Entame 6 au 10 : ni Roi ni Dame
    //  1 ou 5 : possède ROI ou Dame
	if ( FlagSignalisation && IndexTable == 0 && posPreneur != 0 && CouleurDemandee > ATOUT
		&& pJeu->JoueurCouleur[CouleurDemandee] < 0 )
	{
		if ( Table[0].Hauteur >= 6 && Table[0].Hauteur <= 10 )
		{
            if ( pJeu->ProbCarte[pos][Startof[CouleurDemandee] + DAME - 1] > 0.0001 )
                BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, Startof[CouleurDemandee] + DAME - 1, 0.8);
            if ( pJeu->ProbCarte[pos][Startof[CouleurDemandee] + ROI - 1] > 0.0001 )
                BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, Startof[CouleurDemandee] + ROI - 1, 0.8);
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique Couleur 5, Ouverture couleur avec 6 au 10 : ni Roi ni Dame.. ProbCarte[%d][DAME] = %.3f  ProbCarte[%d][ROI] = %.3f\n",
                        pos, pJeu->ProbCarte[pos][Startof[CouleurDemandee]+12], pos, pJeu->ProbCarte[pos][Startof[CouleurDemandee]+13]);
#endif // DEBUG_HEURISTIQUES
		}
		else
		{
			if ( HasCarte(pJeu, pJeu->PositionJoueur, Startof[CouleurDemandee] + DAME - 1) )
			{
                if ( pJeu->ProbCarte[pos][Startof[CouleurDemandee] + DAME - 1] > 0.0001 )
                {
                    MonteProba(CurrentGame, pJeu->PositionJoueur, pos, Startof[CouleurDemandee] + ROI - 1, 2.0, 0);
#if DEBUG_HEURISTIQUES > 0
                    OutDebug("Heuristique Couleur 5, Ouverture couleur avec 1 au 5 : avec DAME.. ProbCarte[%d][%d] = %.3f\n",
                            pos, Startof[CouleurDemandee]+13, pJeu->ProbCarte[pos][Startof[CouleurDemandee]+13]);
#endif // DEBUG_HEURISTIQUES
                }
			}
			else if ( HasCarte(pJeu, pJeu->PositionJoueur, Startof[CouleurDemandee] + ROI - 1) )
			{
                if ( pJeu->ProbCarte[pos][Startof[CouleurDemandee] + ROI - 1] > 0.0001 )
                {
                    MonteProba(CurrentGame, pJeu->PositionJoueur, pos, Startof[CouleurDemandee] + DAME - 1, 2.0, 0);
#if DEBUG_HEURISTIQUES > 0
                    OutDebug("Heuristique Couleur 5, Ouverture couleur avec 1 au 5 : avec ROI.. ProbCarte[%d][%d] = %.3f\n",
                            pos, Startof[CouleurDemandee]+12, pJeu->ProbCarte[pos][Startof[CouleurDemandee]+12]);
#endif // DEBUG_HEURISTIQUES
                }
			}
			else
			{
                if ( pJeu->ProbCarte[pos][Startof[CouleurDemandee] + DAME - 1] > 0.0001 )
                    MonteProba(CurrentGame, pJeu->PositionJoueur, pos, Startof[CouleurDemandee] + DAME - 1, 1.4, 0);
                if ( pJeu->ProbCarte[pos][Startof[CouleurDemandee] + ROI - 1] > 0.0001 )
                    MonteProba(CurrentGame, pJeu->PositionJoueur, pos, Startof[CouleurDemandee] + ROI - 1, 1.4, 0);
#if DEBUG_HEURISTIQUES > 0
                OutDebug("Heuristique Couleur 5, Ouverture couleur avec 1 au 5 : Roi ou Dame.. ProbCarte[%d][DAME] = %.3f  ProbCarte[%d][ROI] = %.3f\n",
                        pos, pJeu->ProbCarte[pos][Startof[CouleurDemandee]+12], pos, pJeu->ProbCarte[pos][Startof[CouleurDemandee]+13]);
#endif // DEBUG_HEURISTIQUES
			}
		}
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique 6 : Si perd la dame sur le roi, ne possède pas l'excuse
	if ( !GainPli(CurrentGame, IndexTable) && Table[IndexTable].Couleur > ATOUT && Table[IndexTable].Hauteur == DAME &&
		Table[idxGagnant].Couleur == Table[IndexTable].Couleur )
	{
        BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, 0, 0.85);
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique Couleur 6, Perd la dame sur le Roi ne possède pas l'excuse.. ProbCarte[%d][%d] = %.3f\n",
                pos, 0, pJeu->ProbCarte[pos][0]);
#endif // DEBUG_HEURISTIQUES
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique 7 : si défenseur joue excuse au premier tour couleur ouverte par preneur, possède tenue
    //  Augmente proba joueur et diminue proba petit
	if ( FlagSignalisation && CouleurDemandee > ATOUT && IndexTable > posPreneur && posPreneur == 0
        && pJeu->JoueurCouleur[CouleurDemandee] < 0 && Table[IndexTable].Couleur == EXCUSE )
	{
		for ( j = Startof[CouleurDemandee]; j < Endof[CouleurDemandee]; j++ )
		{
            if ( pJeu->ProbCarte[pos][j] < 0.01 || pJeu->ProbCarte[pos][j] == 1.0 ) continue;
			MonteProba(CurrentGame, pJeu->PositionJoueur, pos, j, 1.2, 0);
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique Couleur 7, Excuse sur ouverture preneur, possède une tenue.. ProbCarte[%d][%d] = %.3f\n",
                pos, j, pJeu->ProbCarte[pos][j]);
#endif // DEBUG_HEURISTIQUES
		}
		PossPetit = JoueurAvecPetit(CurrentGame, pJeu);
		if (PossPetit == CurrentGame->JoueurPreneur || pJeu->ProbCarte[PossPetit][1] < 0.7 )
		{
			pJeu->Flanc = ATOUT;			//	Dans ce cas, mieux vaut jouer ATOUT
			pJeu->ValFlanc[ATOUT] = 1.0;
			pJeu->Obeissance[pos] = 1.0;
			if ( pJeu->JoueurMainForte < 0 || pJeu->ForceMain[pJeu->JoueurMainForte] < 0.8 )
			{
				pJeu->JoueurMainForte = pos;	//	Devient le champion de la défense
				pJeu->ForceMain[pJeu->JoueurMainForte] = 1.0;
			}
		}
		pJeu->CouleurTenue = CouleurDemandee;
		//	De plus celui qui demande flanc ATOUT n'a pas le petit...
		BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, 1, 0.85);
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique Couleur 7, Excuse sur ouverture preneur, ne possède pas petit.. ProbCarte[%d][%d] = %.3f\n",
            pos, 1, pJeu->ProbCarte[pos][1]);
#endif // DEBUG_HEURISTIQUES
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique Couleur 8 : Si joue signalisation et descend dans les cartes et couleur jouée défense => doubleton
	if ( FlagSignalisation && pos != CurrentGame->JoueurPreneur && LookForSigDescendant(CurrentGame, pos, CouleurDemandee)
		&& Table[IndexTable].Couleur > ATOUT && Table[IndexTable].Couleur == CouleurDemandee && pJeu->JoueurCouleur[CouleurDemandee] != CurrentGame->JoueurPreneur
		&& pJeu->JoueurCouleur[CouleurDemandee] != pos )
	{
		for ( j = Startof[CouleurDemandee]; j < Endof[CouleurDemandee]; j++ )
        {
		    if ( pJeu->ProbCarte[pos][j] <= 0.000001 ) continue;
		    if ( pJeu->ProbCarte[pos][j] >= 0.999999 ) continue;
			BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, j, 0.85);
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique Couleur 8, Sig doubleton sur ouverture défense, .. ProbCarte[%d][%d] = %.3f\n",
            pos, j, pJeu->ProbCarte[pos][j]);
#endif // DEBUG_HEURISTIQUES
        }
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique Couleur 9 : Si joue signalisation et descend dans les cartes et couleur jouée attaque
    //	=> Tient la couleur du Preneur
    if ( FlagSignalisation && pos != CurrentGame->JoueurPreneur && LookForSigDescendant(CurrentGame, pos, CouleurDemandee)
		&& Table[IndexTable].Couleur > ATOUT && Table[IndexTable].Couleur == CouleurDemandee && pJeu->JoueurCouleur[CouleurDemandee] == CurrentGame->JoueurPreneur )
	{
		for ( j = Startof[CouleurDemandee]; j < Endof[CouleurDemandee]; j++ )
		{
		    if ( pJeu->ProbCarte[pos][j] <= 0.000001 ) continue;
		    if ( pJeu->ProbCarte[pos][j] >= 0.9999999 ) continue;
			MonteProba(CurrentGame, pJeu->PositionJoueur, pos, j, 1.2, 0);
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique Couleur 8, Sig tenue couleur preneur, .. ProbCarte[%d][%d] = %.3f\n",
            pos, j, pJeu->ProbCarte[pos][j]);
#endif // DEBUG_HEURISTIQUES
		}
		CopieProba2Tmp(pJeu);
		if ( JoueurAvecPetit(CurrentGame, pJeu) < 0 || JoueurAvecPetit(CurrentGame, pJeu) == CurrentGame->JoueurPreneur )
		{
			pJeu->Flanc = ATOUT;			//	Dans ce cas, mieux vaut jouer ATOUT
			pJeu->ValFlanc[ATOUT] = 1.0;
			pJeu->Obeissance[pos] = 1.0;
			if ( pJeu->JoueurMainForte < 0 || pJeu->ForceMain[pJeu->JoueurMainForte] < 0.8 )
			{
				pJeu->JoueurMainForte = pos;	//	Devient le champion de la défense
				pJeu->ForceMain[pJeu->JoueurMainForte] = 1.0;
			}
		}
		pJeu->CouleurTenue = CouleurDemandee;
		//	De plus celui qui demande flanc ATOUT n'a pas le petit...
		BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, 1, 0.9);
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique Couleur 8, Sig tenue couleur preneur, ne possède pas le petit, .. ProbCarte[%d][%d] = %.3f\n",
            pos, 1, pJeu->ProbCarte[pos][1]);
#endif // DEBUG_HEURISTIQUES
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique couleur 9 : si joueur ne prend pas un pli rapportant plus de X points, n'a pas de carte plus grosse
    idxGagnant = Gagnant(IndexTable);
	if ( IndexTable >= 1 && CouleurDemandee > ATOUT && NbReste(CurrentGame, pJeu->PositionJoueur, CouleurDemandee) > 0 )
	{
		h = 0;
		for ( j = 0; j < IndexTable; j++)
		{
			h += Table[j].Valeur;
			if ( Table[j].Index == 1 ) h += 20;
		}
		if ( !GainPli(CurrentGame, IndexTable) && h >= 7 )
		{
			j = CarteMaitreNonJoueeNoErr(CurrentGame, CouleurDemandee);
			if ( Table[idxGagnant].Couleur == CouleurDemandee && j >= 0 && j > Table[idxGagnant].Index )
            {
				BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, j, 0.95 - 1.5/h);
#if DEBUG_HEURISTIQUES > 0
                OutDebug("Heuristique Couleur 9, joueur ne prend pas un pli rapportant des points, n'a pas de plus grosse.. ProbCarte[%d][%d] = %.3f\n",
                pos, j, pJeu->ProbCarte[pos][j]);
#endif // DEBUG_HEURISTIQUES
            }
		}
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique 10 : Si joue une carte "assez forte" et perd le pli, n'en possède pas en dessous
	if ( CouleurDemandee > ATOUT && Table[IndexTable].Couleur == CouleurDemandee && IndexTable != idxGagnant && Table[IndexTable].Hauteur >= 8
		&& (idxGagnant == posPreneur || IndexTable == posPreneur )      //  Perd le pli
		&& Table[idxGagnant].Couleur == CouleurDemandee )
	{
		if ( idxGagnant == posPreneur )
		{
			if ( IndexTable > idxGagnant )
				v = 0.65;
			else if ( IndexTable == 0 )
				v = 0.5;
			else if ( IndexTable == 1 )
				v = 0.3;
			else
				v = 0.1;
		}
		else
		{
			if ( IndexTable > idxGagnant )
				v = 0.85;
			else if ( IndexTable == 0 )
				v = 0.7;
			else if ( IndexTable == 1 )
				v = 0.5;
			else
				v = 0.25;
		}
		h = Table[IndexTable].Index;
		//	Descend la proba de toutes les cartes inférieures a celle jouée
		if ( Table[IndexTable].Valeur == 1 )
		{
		    h -= 3;         //  Baisse artificiellement la hauteur si pas de points (cas des suites)
		}
		if ( h < 10 && IndexTable == 0 && pJeu->JoueurCouleur[CouleurDemandee] < 0 )
            h = -1;
		if ( h >= 0 )
		{
			for ( j = Startof[CouleurDemandee]; j < h; j++)
			{
			    if ( pJeu->ProbCarte[pos][j] < 0.000001 ) continue;
			    if ( pJeu->ProbCarte[pos][j] > 0.999999 ) continue;
				BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, j, v);
#if DEBUG_HEURISTIQUES > 0
                OutDebug("Heuristique Couleur 10, joueur joue assez grosse carte et perd le pli, n'a plus de petites.. ProbCarte[%d][%d] = %.3f\n",
                pos, j, pJeu->ProbCarte[pos][j]);
#endif // DEBUG_HEURISTIQUES
			}
		}
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique couleur 11 : Si défenseur met un honneur sur preneur maître ne possède pas en dessous
	if ( pos != CurrentGame->JoueurPreneur && !GainPli(CurrentGame, IndexTable) && CouleurDemandee > ATOUT && IndexTable > idxGagnant
		&& Table[IndexTable].Couleur == CouleurDemandee && Table[IndexTable].Hauteur >= VALET  )
	{
		for ( j = Startof[Table[IndexTable].Couleur]; j < Startof[Table[IndexTable].Couleur] + Table[IndexTable].Hauteur; j++)
		{
		    if ( pJeu->ProbCarte[pos][j] <= 0.0000001 ) continue;
		    if ( pJeu->ProbCarte[pos][j] >= 0.9999999 ) continue;
			BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, j, 0.5+0.045*Table[IndexTable].Valeur);
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique Couleur 11, Défenseur joue honneur sur preneur maître, n'a plus de petites.. ProbCarte[%d][%d] = %.3f\n",
            pos, j, pJeu->ProbCarte[pos][j]);
#endif // DEBUG_HEURISTIQUES
		}
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique 12 : Idem si avant le preneur mais coupe connue
	if ( pos != CurrentGame->JoueurPreneur && !GainPli(CurrentGame, IndexTable) && CouleurDemandee > ATOUT && IndexTable < posPreneur
		&& pJeu->ProbCoupe[CouleurDemandee] > 0.95 && Table[IndexTable].Couleur < ATOUT && Table[IndexTable].Hauteur >= VALET )
	{
		for ( j = Startof[Table[IndexTable].Couleur]; j < Startof[Table[IndexTable].Couleur] + Table[IndexTable].Hauteur; j++)
		{
		    if ( pJeu->ProbCarte[pos][j] <= 0.0000001 ) continue;
		    if ( pJeu->ProbCarte[pos][j] >= 0.9999999 ) continue;
			BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, j, 0.5+0.045*Table[IndexTable].Valeur);
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique Couleur 12, Défenseur joue honneur sur preneur maître, n'a plus de petites.. ProbCarte[%d][%d] = %.3f\n",
            pos, j, pJeu->ProbCarte[pos][j]);
#endif // DEBUG_HEURISTIQUES
		}
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique  13 : Entame du CAVALIER => Possède certainement la DAME...
	if ( IndexTable == 0  && pJeu->JoueurCouleur[CouleurDemandee] < 0 && CouleurDemandee > ATOUT && Table[0].Hauteur == CAVALIER )
	{
		MonteProba(CurrentGame, pJeu->PositionJoueur, pos, Table[0].Index+1, 5, 0);
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique Couleur 13, Défenseur joue honneur sur preneur maître, n'a plus de petites.. ProbCarte[%d][%d] = %.3f\n",
            pos, Table[0].Index+1, pJeu->ProbCarte[pos][Table[0].Index+1]);
#endif // DEBUG_HEURISTIQUES
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique 14 : Si entame d'un joueur avec un ROI ou Cavalier, main forte
	if ( FlagSignalisation && IndexTable == 0 && posPreneur != 0 && pJeu->NbEntame[CurrentGame->JoueurEntame] == 1
        && Table[0].Couleur > ATOUT && (Table[0].Hauteur == ROI || Table[0].Hauteur == CAVALIER) )
	{
		pJeu->ForceMain[CurrentGame->JoueurEntame] = 1.0;
		pJeu->Obeissance[CurrentGame->JoueurEntame] = 1.0;
		pJeu->JoueurMainForte = CurrentGame->JoueurEntame;
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique Couleur 14 : Entame Roi ou Cavalier, Main forte = %d\n", pJeu->JoueurMainForte);
#endif // DEBUG_HEURISTIQUES
	}
    //	Heuristique 15 : Si entame d'un joueur avec une DAME, main faible, possède certainement ROI DAME
	if ( FlagSignalisation && IndexTable == 0 && posPreneur != 0 && pJeu->NbEntame[CurrentGame->JoueurEntame] == 1
        && Table[0].Couleur > ATOUT && Table[0].Hauteur == DAME && pJeu->JoueurCouleur[CouleurDemandee] < 0 )
	{
		pJeu->ForceMain[CurrentGame->JoueurEntame] = 0.2;
		pJeu->Obeissance[CurrentGame->JoueurEntame] = 0.4;
		if ( pJeu->JoueurMainForte < 0 || pJeu->ForceMain[pJeu->JoueurMainForte] < 0.5 )
			pJeu->JoueurMainForte = CurrentGame->JoueurEntame;
		MonteProba(CurrentGame, pJeu->PositionJoueur, CurrentGame->JoueurEntame, Table[0].Index + 1, 3.0, 0);
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique Couleur 15, Entame Dame, main faible et possède le ROI.. ProbCarte[%d][%d] = %.3f\n",
            pos, Table[0].Index+1, pJeu->ProbCarte[pos][Table[0].Index+1]);
#endif // DEBUG_HEURISTIQUES
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique 16 : Si Entame dans une couleur, c'est plutôt une longue... D'autant plus vrai qu'il y a de l'atout au chien
	if ( pJeu->NbCarte == 17 && IndexTable == 0 && posPreneur != 0 && Table[0].Couleur < ATOUT &&
		MinLongueur(pJeu, CurrentGame->JoueurPreneur, ATOUT) >= 1  )
	{
		v = 1.0 + sqrt(MinLongueur(pJeu, CurrentGame->JoueurPreneur, ATOUT)) * 0.15 + 0.25/posPreneur;
		for ( j = Startof[CouleurDemandee]; j < Endof[CouleurDemandee]; j++)
		{
		    if ( pJeu->ProbCarte[pos][j] < 0.000001 ) continue;
		    if ( pJeu->ProbCarte[pos][j] > 0.999999 ) continue;
			MonteProba(CurrentGame, pJeu->PositionJoueur, pos, j, v, 0);
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique Couleur 16, Première entame plutôt dans une longue. ProbCarte[%d][%d] = %.3f\n",
            pos, j, pJeu->ProbCarte[pos][j]);
#endif // DEBUG_HEURISTIQUES
		}
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique Couleur 17 : Si Preneur ouvre d'une couleur, surtout au début c'est sans doute une longue
	if ( IndexTable == 0 && posPreneur == 0 && CouleurDemandee > ATOUT
		&& pJeu->JoueurCouleur[CouleurDemandee] < 0 && pJeu->ProbCoupe[CouleurDemandee] > 0.2  )
	{
		if ( (pJeu->CouleurEntameParJoueur[CurrentGame->JoueurPreneur] & 0x3C) == (1 << CouleurDemandee) )
			v = 1.5;        //  Première ouverture
		else
			v = 1;          //  Ouvertures suivantes
		if ( pJeu->CouleurEntameParJoueur[CurrentGame->JoueurPreneur] & (1 << ATOUT) )		//	Ouvert a l'atout ?
		{
			pJeu->GuessProbCoupe[CouleurDemandee] = pJeu->ProbCoupe[CouleurDemandee] / (1.5 * v);
		}
		else
		{
			pJeu->GuessProbCoupe[CouleurDemandee] = pJeu->ProbCoupe[CouleurDemandee] / (2.5 * v);
		}
		//	Monte proba de coupe des autres couleurs
		for ( c = TREFLE; c < NB_COULEUR; c++ )
		{
			if ( c == CouleurDemandee ) continue;
			if ( pJeu->ProbCoupe[c] > 0.6 ) continue;
			pJeu->GuessProbCoupe[c] *= 1.2;
		}
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique Couleur 17, Entame preneur plutôt dans une longue. Change ProbCoupe[T]=%.3f, ProbCoupe[K]=%.3f, ProbCoupe[P]=%.3f, ProbCoupe[C]=%.3f\n",
            pJeu->GuessProbCoupe[TREFLE], pJeu->GuessProbCoupe[CARREAU], pJeu->GuessProbCoupe[PIQUE], pJeu->GuessProbCoupe[COEUR]);
#endif // DEBUG_HEURISTIQUES
		for ( j = Startof[CouleurDemandee]; j < Endof[CouleurDemandee]; j++)
		{
		    if ( pJeu->ProbCarte[pos][j] > 0 && pJeu->ProbCarte[pos][j] < 1.0 )
            {
                MonteProba(CurrentGame, pJeu->PositionJoueur, CurrentGame->JoueurPreneur, j, 1.0 + 0.1*v, 0);
#if DEBUG_HEURISTIQUES > 0
                OutDebug("Heuristique Couleur 17, Entame preneur plutôt dans une longue. ProbCarte[%d][%d] = %.3f\n",
                pos, j, pJeu->ProbCarte[pos][j]);
#endif // DEBUG_HEURISTIQUES
            }
		}
		CopieProba2Tmp(pJeu);
	}
	else if ( IndexTable == 0 && posPreneur == 0 && CouleurDemandee > ATOUT )
	{
		pJeu->GuessProbCoupe[CouleurDemandee] /= 2.0;
	}
	//	Si le preneur en rejoue, monte les probas
	if ( IndexTable == 0 && posPreneur == 0 && pJeu->JoueurCouleur[CouleurDemandee] == CurrentGame->JoueurPreneur &&
		pJeu->NbJoue[CurrentGame->JoueurPreneur][CouleurDemandee] == 2 )
	{
		if ( (pJeu->CouleurEntameParJoueur[CurrentGame->JoueurPreneur] & 0x3C) == (1 << CouleurDemandee) )
			v = 0.75;
		else
			v = 0;
		for ( j = Startof[CouleurDemandee]; j < Endof[CouleurDemandee]; j++)
        {
            if ( pJeu->ProbCarte[pos][j] < 0.000001 ) continue;
            if ( pJeu->ProbCarte[pos][j] > 0.999999 ) continue;
			MonteProba(CurrentGame, pJeu->PositionJoueur, CurrentGame->JoueurPreneur, j, 1.25 + v, 0);
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique Couleur 17bis, Preneur rejoue dans couleur entamée. ProbCarte[%d][%d] = %.3f\n",
            pos, j, pJeu->ProbCarte[pos][j]);
#endif // DEBUG_HEURISTIQUES
            //  Et baisse proba du chien
            BaisseProba(CurrentGame, pJeu->PositionJoueur, 4, j, 0.6);
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique Couleur 17ter, Preneur rejoue dans couleur entamée, baisse proba chien. ProbCarte[%d][%d] = %.3f\n",
            4, j, pJeu->ProbCarte[4][j]);
#endif // DEBUG_HEURISTIQUES
        }
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique couleur 18 : Si couleur ouverte par défenseur possesseur probable petit
    //	Si possède déjà une courte => Couleur plutôt longue
    //	Sinon => Couleur plutôt courte
	if ( IndexTable == 0 && pJeu->JoueurCouleur[CouleurDemandee] < 0 && JoueurAvecPetit(CurrentGame, pJeu) == CurrentGame->JoueurEntame &&
        posPreneur != 0 )
	{
		for ( c = TREFLE; c < NB_COULEUR; c++)
		{
			if ( c == CouleurDemandee ) continue;
			if ( pJeu->NbJoue[CurrentGame->JoueurEntame][c] + AvgLongueur(pJeu, CurrentGame->JoueurEntame, c) <= 2.05 )	//	Courte
				break;
		}
		if ( c < NB_COULEUR || Table[posPreneur].Couleur == ATOUT)
		//	Possède une courte ou preneur coupe, la couleur jouée est plutôt longue...
		{
			for ( h = Startof[CouleurDemandee]; h < Endof[CouleurDemandee]; h++ )
			{
			    if ( pJeu->ProbCarte[pos][h] <= 0 ) continue;
			    if ( pJeu->ProbCarte[pos][h] >= 1.0 ) continue;
				MonteProba(CurrentGame, pJeu->PositionJoueur, CurrentGame->JoueurEntame, h, 1.05, 0 );		//	Augmente la longueur de cette couleur...
#if DEBUG_HEURISTIQUES > 0
                OutDebug("Heuristique Couleur 18, Couleur ouverte par possesseur petit (longue). ProbCarte[%d][%d] = %.3f\n",
                CurrentGame->JoueurEntame, h, pJeu->ProbCarte[CurrentGame->JoueurEntame][h]);
#endif // DEBUG_HEURISTIQUES
			}
		}
		else	//	Pas de courte, joue plutôt une courte
		{
			for ( h = Startof[CouleurDemandee]; h < Endof[CouleurDemandee]; h++ )
			{
			    if ( pJeu->ProbCarte[pos][h] <= 0 ) continue;
			    if ( pJeu->ProbCarte[pos][h] >= 1.0 ) continue;
				BaisseProba(CurrentGame, pJeu->PositionJoueur, CurrentGame->JoueurEntame, h, 0.25 );		//	diminue la longueur de cette couleur...
#if DEBUG_HEURISTIQUES > 0
                OutDebug("Heuristique Couleur 18, Couleur ouverte par possesseur petit (courte). ProbCarte[%d][%d] = %.3f\n",
                CurrentGame->JoueurEntame, h, pJeu->ProbCarte[CurrentGame->JoueurEntame][h]);
#endif // DEBUG_HEURISTIQUES
			}
		}
		pJeu->Obeissance[CurrentGame->JoueurEntame] = 1.0;
		pJeu->Flanc = CouleurDemandee;			//	Devient Flanc par défaut
		pJeu->ValFlanc[CouleurDemandee] = 1.0;
		if ( pJeu->ForceMain[CurrentGame->JoueurEntame] < 0.5 ) pJeu->ForceMain[CurrentGame->JoueurEntame] = 0.5;
		CopieProba2Tmp(pJeu);
	}
    //	Heuristique 19, pas de singleton à la DAME chez le preneur si ne possède pas l'excuse.
	if ( CurrentGame->TypePartie < GARDE_SANS && IndexTable > 0 && IndexTable == posPreneur
        && pJeu->NbJoue[CurrentGame->JoueurPreneur][CouleurDemandee] == 1 &&
		Table[IndexTable].Couleur == CouleurDemandee && Table[IndexTable].Hauteur == DAME )
	{
		if ( pJeu->ProbCarte[CurrentGame->JoueurPreneur][0] < 0.5 )
			pJeu->GuessProbCoupe[CouleurDemandee] = 0.0;
		int Boucle = 4;
		while ( --Boucle > 0 && ProbCoupeJoueur(pJeu, CurrentGame->JoueurPreneur, CouleurDemandee) > 0.1 )
		{
			for ( h = Startof[CouleurDemandee]; h < Endof[CouleurDemandee]; h++)
			{
			    if ( pJeu->ProbCarte[CurrentGame->JoueurPreneur][h] <= 0 ) continue;
			    if ( pJeu->ProbCarte[CurrentGame->JoueurPreneur][h] >= 1.0 ) continue;
				MonteProba(CurrentGame, pJeu->PositionJoueur, pos, h, 1.1, 0);
#if DEBUG_HEURISTIQUES > 0
                OutDebug("Heuristique Couleur 19, Pas de singleton à la DAME chez preneur. ProbCarte[%d][%d] = %.3f\n",
                CurrentGame->JoueurPreneur, h, pJeu->ProbCarte[CurrentGame->JoueurPreneur][h]);
#endif // DEBUG_HEURISTIQUES
			}
		}
        CopieProba2Tmp(pJeu);
	}
    //	Heuristique Couleur 20 : si joueur défense est obligé de couper dès le début, baisse la force de son jeu
	if ( Table[IndexTable].Couleur != CouleurDemandee && posPreneur == 0 && Table[IndexTable].Couleur != EXCUSE )
	{
		pJeu->ForceMain[pos] /= 1.0 + 1.0/(19.0 - pJeu->NbCarte);
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique Couleur 20, baisse force jeu du défenseur qui coupe très tôt. ForceMain[%d] = %.3f\n",
        pos, pJeu->ForceMain[pos]);
#endif // DEBUG_HEURISTIQUES
    }
    //	Heuristique couleur 21 : Si un joueur défausse une petite carte dans une couleur, il n'en veut pas
	if ( IndexTable > 0 && Table[IndexTable].Couleur != CouleurDemandee && pos != CurrentGame->JoueurPreneur && Table[IndexTable].Couleur > ATOUT )
	{
		if ( Table[IndexTable].Valeur == 1 && Table[IndexTable].Couleur > ATOUT )
		{
			pJeu->CouleurNonVoulueJoueur[pos][Table[IndexTable].Couleur] = 1;
			h = 0;
			j = 0;
			for ( c = TREFLE; c < NB_COULEUR; c++)
			{
				if ( pJeu->CouleurNonVoulueJoueur[pos][c] || AvgLongueur(pJeu, pos, c) < 0.1 )
				{
					j++;
					h^= c;
				}
			}
			if ( j == 3 )	//	Ne veut pas de trois couleurs => Bonus pour la 4eme
				pJeu->CouleurVoulueJoueur[pos][h] = 1;
		}
		pJeu->DefausseCouleurParJoueur[pos] |= 1 << Table[IndexTable].Couleur;
		for ( c = TREFLE; c < NB_COULEUR; c++)
		{
			h = 0;
			for ( j = 0; j < 4; j++)
			{
				h += pJeu->CouleurVoulueJoueur[j][c] * 5 - pJeu->CouleurNonVoulueJoueur[j][c];
			}
			pJeu->CouleurVoulue[c] = h;
		}
#if DEBUG_HEURISTIQUES > 0
        OutDebug("Heuristique Couleur 21, Défausse joueur %d ne veut pas de la couleur %d\n", pos, Table[IndexTable].Couleur);
        for ( c = TREFLE; c < NB_COULEUR; c++)
        {
            OutDebug("CouleurVoulue : %d = %d  ", c, pJeu->CouleurVoulue[c]);
        }
        OutDebug("\n");
#endif // DEBUG_HEURISTIQUES
	}
    //	Heuristique couleur 22 : Si un défenseur change de couleur, 4 cas possibles
    //		1	:	Il a une main forte (déterminé par ROI ou CAVALIER)
    //		2	:	Il a le petit en danger
    //		3	:	Il n'en a plus dans la couleur	(proba < une certaine valeur)
    //		4	:	Résultats médiocres de l'entame précédente (au moins deux coups joués)
	if ( IndexTable == 0 && posPreneur != 0 && pJeu->JoueurCouleur[CouleurDemandee] < 0 && pJeu->NbCarte <= 17
		&& pJeu->CouleurEntameParJoueur[pos] == (1 << CouleurDemandee) && pJeu->Flanc > ATOUT && CouleurDemandee > ATOUT)
	{
		if ( pJeu->ForceMain[pos] < 0.8 )		//	Déjà détecté par les règles précédentes, s'intéresse aux autres cas.
		{
			//	Essaie de détecter le changement pour cause de résultats médiocres (au moins deux tours...)
			//	Dans ce cas, ne change rien aux probas
			//	A priori, bons résultats si ProbCoupe élevé ou pas encore joué deux tours
			if ( pJeu->ProbCoupe[pJeu->Flanc] > 0.6 || NbReste(CurrentGame, pJeu->PositionJoueur, pJeu->Flanc) + pJeu->NbCouleur[pJeu->Flanc] > 8)
			{	//	reste les cas 2 et 3
				v = ProbCoupeJoueur(pJeu, pos, pJeu->Flanc);
				if ( v < 0.001) v = 0.001;
				//	Monte proba petit si v est assez faible
				if ( CurrentGame->CarteJouee[1] < 0 )
                    MonteProba(CurrentGame, pJeu->PositionJoueur, pos, 1, 1.0 - 0.5*log(v), 0);
#if DEBUG_HEURISTIQUES > 0
                OutDebug("Heuristique Couleur 22.1, Monte proba petit suite à changement couleur. ProbCarte[%d][%d] = %.3f\n",
                    pos, 1, pJeu->ProbCarte[pos][1]);
#endif // DEBUG_HEURISTIQUES
				v1 = sqrt(v);
				//	Baisse proba d'avoir de la couleur si v assez fort => ce qui va augmenter v
				for ( j = Startof[pJeu->Flanc]; j < Endof[pJeu->Flanc]; j++)
				{
				    if ( pJeu->ProbCarte[pos][j] < 0.0001 ) continue;
					BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, j, v1);
#if DEBUG_HEURISTIQUES > 0
                    OutDebug("Heuristique Couleur 22.1, Changement couleur, baisse proba couleur précédente. ProbCarte[%d][%d] = %.3f\n",
                        pos, j, pJeu->ProbCarte[pos][j]);
#endif // DEBUG_HEURISTIQUES
				}
				if ( v < 0.6 )		//	A priori, ne coupe pas...
				{
					pJeu->Obeissance[CurrentGame->JoueurEntame] = 0.5;
					if ( pJeu->JoueurMainForte < 0 || pJeu->ForceMain[pJeu->JoueurMainForte] < 0.8 )
					{
						pJeu->Flanc = CouleurDemandee;			//	Devient Flanc par défaut
						pJeu->JoueurMainForte = CurrentGame->JoueurEntame;
					}
					pJeu->ValFlanc[CouleurDemandee] = 0.5;
					pJeu->ForceMain[CurrentGame->JoueurEntame] = 0.5;
				}
				else
				{
					pJeu->Obeissance[CurrentGame->JoueurEntame] = 0.5;
					pJeu->ValFlanc[CouleurDemandee] = 0.5;
					pJeu->ForceMain[CurrentGame->JoueurEntame] = 0.5;
				}
			}
			else
			{
				pJeu->Obeissance[CurrentGame->JoueurEntame] = 0.5;
				pJeu->Flanc = CouleurDemandee;			//	Devient Flanc par defaut
				pJeu->ValFlanc[CouleurDemandee] = 0.5;
				pJeu->ForceMain[CurrentGame->JoueurEntame] = 0.5;
			}
		}
		CopieProba2Tmp(pJeu);
	}
	//	Heuristique couleur 23 : Si met l'excuse sur une couleur et pli perdu, ne possède plus de "petites cartes"
	//  En défense, vrai si preneur prend le pli (coupe) et ouverture couleur défense
	if ( IndexTable > 0 && CouleurDemandee > ATOUT && Table[IndexTable].Couleur == EXCUSE && pos != CurrentGame->JoueurPreneur
        && !GainPli(CurrentGame, IndexTable) && pJeu->JoueurCouleur[CouleurDemandee] != CurrentGame->JoueurPreneur && CurrentGame->NumPli < 15 )
	{
        for ( j = Startof[CouleurDemandee]; j < Startof[CouleurDemandee]+10; j++)
        {
            if ( pJeu->ProbCarte[pos][j] < 0.0001 ) continue;
            BaisseProba(CurrentGame, pJeu->PositionJoueur, pos, j, 0.6);
#if DEBUG_HEURISTIQUES > 0
            OutDebug("Heuristique Couleur 23.1, Défense joue excuse sur . ProbCarte[%d][%d] = %.3f\n",
                pos, j, pJeu->ProbCarte[pos][j]);
#endif // DEBUG_HEURISTIQUE
        }
		CopieProba2Tmp(pJeu);
	}
}
/*
//	Règle 18 : Si Ouvreur entame ATOUT et Second ne possède pas le petit, ne joue pas ATOUT maitre
//				ne le possède pas
	h = CarteMaitreNonJouee(ATOUT);
	if ( h > 0 &&  Table[0].Couleur == ATOUT && Preneur == JoueurEntame && Index == 1 && pos != PosPossPetit()
		&& Table[1].Couleur == ATOUT && Table[1].Hauteur != h && pos != Position )
	{
		ChangeProba(pos, h, 0.8);
		CopieProba2Tmp();
	}
//	Règle 20 : Si met l'excuse sur une couleur, ne possède plus de "petites cartes"
	if ( Index > 0 && CouleurDemandee < ATOUT && Table[Index].Couleur == EXCUSE && pos != Position )
	{
		if ( pos == Preneur )		//	Si c'est le preneur qui a mis l'excuse et pli perdu...
		{
			if ( Index < 3 && !GainPli(Table, JoueurEntame, Index) )	//	Pas en dernier et pli perdu avant
			{
				for ( i = Startof[CouleurDemandee]; i < Startof[CouleurDemandee]+10; i++)
					ChangeProba(pos, i, 0.7 - 0.2 * Index);
			}
		}
		CopieProba2Tmp();
	}
//	Règle 22 : Si sur premier pli ATOUT, met une très forte derriere preneur monte prob petit
	h = PlusFortAtoutJoue(Table, Index);
	if ( h > 0 && Table[Index].Couleur == ATOUT && pos != Preneur && Table[IndexPreneur].Index == h
		&& NbJoue[pos][ATOUT] == 1 && (Table[Index].Hauteur - h ) > 6 && pos != Position )
	{
		MonteProba(pos, 1, 1.25 + (Table[Index].Hauteur - h - 3) * 0.1);
		CopieProba2Tmp();
	}
	//	Réciproquement, si ne monte pas très fort, baisse plutôt proba petit
	if ( h > 0 && Table[Index].Couleur == ATOUT && pos != Preneur && Table[IndexPreneur].Index == h
		&& NbJoue[pos][ATOUT] == 1 && (Table[Index].Hauteur - h ) < 4 && (Table[Index].Hauteur - h ) > 0
		&& Table[Index].Hauteur < 15 && pos != Position )
	{
		ChangeProba(pos, 1, 0.1 + (4 - Table[Index].Hauteur + h) * 0.1);
		CopieProba2Tmp();
	}
//	Règle 45 : Si un joueur a montré une simple poignee, ne possède pas la double...
	if ( JoueurPoignee == pos && TypePoignee == 10 && (v = NbJoue[pos][ATOUT] + AvgLongueur(pos, ATOUT)) > 12 && pos != Position)
	{
		v -= 12.0;
		for ( i = 0; i < 22; i++)
		{
			if ( ProbCarte[pos][i] < 0.0 ) continue;
			if ( ProbCarte[pos][i] == 1.0 ) continue;
			ChangeProba(pos, i, 1 - exp(-v));
		}
		CopieProba2Tmp();
	}
//	Règle 46 : Si un joueur a montré une double poignee, ne possède pas la triple...
	if ( JoueurPoignee == pos && TypePoignee == 13 && (v = NbJoue[pos][ATOUT] + AvgLongueur(pos, ATOUT)) > 15 && pos != Position)
	{
		v -= 15.0;
		for ( i = 0; i < 22; i++)
		{
			if ( ProbCarte[pos][i] < 0.0 ) continue;
			if ( ProbCarte[pos][i] == 1.0 ) continue;
			ChangeProba(pos, i, 1 - exp(-v));
		}
		CopieProba2Tmp();
	}
*/
