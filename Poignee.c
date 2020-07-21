#include <stdlib.h>
#include <gtk/gtk.h>
#include "Tarot_Ui_Objects.h"
#include <assert.h>
#include <math.h>

int NbAtoutPoignee[4] = {0, 10, 13, 15};
//  Sélectionne NbAtouts dans ceux du joueur pour les montrer dans la poignée
//  Choisit les plus faibles par défaut

static void SelectAtoutsPoignee(TarotGame CurrentGame, struct _Jeu *pJeu, int Nb)
{
int i, j, k;
int idx;

    i = 0;
    if ( pJeu->NbAtout > Nb && HasCarte(pJeu, pJeu->PositionJoueur, 0)  )
    {
        i = 1;					//	i = 1 (saute l'excuse)
    }
    for ( j = 0; j < Nb; j++)	//	Montre 10 atouts, par défaut les plus faibles
    {
        CurrentGame->ListeAtoutPoignee[pJeu->PositionJoueur][j] = pJeu->MyCarte[i].Index;
        i++;
    }
    //	Montre les atouts au chien si preneur, remplace éventuellement ceux choisis
    if ( CurrentGame->JoueurPreneur == pJeu->PositionJoueur && CurrentGame->TypePartie < GARDE_SANS )
    {
        for ( ; i < pJeu->NbAtout; i++ )
        {
            idx = pJeu->MyCarte[i].Index;
            if ( CurrentGame->CarteAuChien[idx] )       //  Carte vue au chien ?
            {
                //  Oui, la met dans la poignée
                //  Cache la plus grosse
                for ( j = Nb-1; j > 0; j++ )
                {
                    if ( CurrentGame->CarteAuChien[CurrentGame->ListeAtoutPoignee[pJeu->PositionJoueur][j]] == 0 )
                    {
                        //  Pas vue au chien, décale
                        for ( k = j; k < Nb-1; k++)
                        {
                            CurrentGame->ListeAtoutPoignee[pJeu->PositionJoueur][k] = CurrentGame->ListeAtoutPoignee[pJeu->PositionJoueur][k+1];
                        }
                        break;
                    }
                }
            }
        }
    }
    CurrentGame->NumAtoutPoignee[CurrentGame->JoueurCourant] = Nb;
}

//	Retourne le nombre d'atouts à montrer dans la poignée
//  Le fait pour le joueur Courant

int OkToShowPoignee(TarotGame CurrentGame)
{
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[CurrentGame->JoueurCourant];

	if ( pJeu->NbAtout < 10 ) return(0);	//	Pas de poignée, passe
	if ( pJeu->NbAtout <= 12 )				//	Simple poignée
	{
	    //  Si preneur et pas beaucoup de points et pas petit et un seul bout, ne montre pas la poignée
	    //  Pour SUD, laisse toujours le joueur décider
		if ( pJeu->PositionJoueur != SUD && pJeu->PositionJoueur == pJeu->PositionPreneur && pJeu->NbPoint < 60
            && !HasCarte(pJeu, pJeu->PositionJoueur, 1) && pJeu->NbBout <= 1)	//	Pas le petit et un seul bout
			return(0);					//	Ne montre rien dans ce cas
		SelectAtoutsPoignee(CurrentGame, pJeu, 10);
		return(10);
	}
	if ( pJeu->NbAtout <= 14 )				//	Double poignée, la montre toujours
	{
		SelectAtoutsPoignee(CurrentGame, pJeu, 13);
		return(13);
	}
    SelectAtoutsPoignee(CurrentGame, pJeu, 15);
	return(15);
}

//  Monte les atouts présélectionnés pour SUD

void MonteAtoutsPoignee(TarotGame CurrentGame)
{
int Nb = CurrentGame->NumAtoutPoignee[SUD];
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[SUD];
int i, j;
int idx;

    SelectAtoutsPoignee(CurrentGame, pJeu, Nb);
    memset(CurrentGame->isCarteLevee, 0, sizeof(CurrentGame->isCarteLevee));
    if ( FlagSuggestionPoignee == 0 )       //  pas de sélection
    {
        CurrentGame->NbCarteLevee = 0;
        return;
    }
    for ( i = 0; i < Nb; i++ )
    {
        idx = CurrentGame->ListeAtoutPoignee[SUD][i];
        for ( j = 0; j < pJeu->NbCarte; j++ )
        {
            if ( pJeu->MyCarte[j].Index == idx )
            {
                CurrentGame->isCarteLevee[j] = 1;
            }
        }
    }
    CurrentGame->NbCarteLevee = Nb;
}

//  Retourne 1 s'il est possible de mettre la carte dans la poignée

int isOKPoignee(TarotGame CurrentGame, int Index)
{
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[SUD];

    //  Règle : il faut que cela soit un ATOUT, ne peut montrer l'excuse si plus que le minimum
    if ( pJeu->MyCarte[Index].Couleur == EXCUSE )
    {
        if ( pJeu->NbAtout > CurrentGame->NumAtoutPoignee[SUD] )
            return 0;
        else
            return 1;
    }
    else if ( pJeu->MyCarte[Index].Couleur == ATOUT )
        return 1;
    else
        return 0;
}

//  Action de "voir" la poignée du joueur JoueurAvecPoignee.
//  Mise à jour des connaissances sur le jeu du preneur
//  le fait pour le joueur Position

void RegardePoignee(TarotGame CurrentGame, int Position, int JoueurAvecPoignee)
{
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[Position];
int i;
double Moyenne = 0.0;
double ProbAtoutRestant = 0.0;
double ProbRegle;

    //  Mise à jour des probabilités des cartes vues
    for ( i = 0; i < CurrentGame->NumAtoutPoignee[JoueurAvecPoignee]; i++ )
    {
        //  On est sûr que la carte est chez le possesseur de la poignée
        SetCarte2Position(pJeu, JoueurAvecPoignee, CurrentGame->ListeAtoutPoignee[JoueurAvecPoignee][i]);
        NormaliseProba(CurrentGame, pJeu);
    }
    //  Règle : si montre excuse dans la poignée, n'en a pas d'autres
	if ( CurrentGame->ListeAtoutPoignee[JoueurAvecPoignee][0] == 0 )	//	Montre l'excuse donc n'en a plus d'autre
	{
		for ( int i = 1; i < 22; i++)
		{
			if ( pJeu->ProbCarte[JoueurAvecPoignee][i] < 1.0 && pJeu->ProbCarte[JoueurAvecPoignee][i] > 0.0 )
			{
                BaisseProba(CurrentGame, Position, JoueurAvecPoignee, i, 1.0);
			}
		}
	}
	else
    {
        //  Si montre une simple poignée, moins de 13 atouts
        //  Si montre une double poignée, moins de 15 atouts
        if ( CurrentGame->PoigneeMontree[JoueurAvecPoignee] == POIGNEE_SIMPLE )
            Moyenne = 1.5;      //  Entre 10 et 13
        else if ( CurrentGame->PoigneeMontree[JoueurAvecPoignee] == POIGNEE_DOUBLE )
            Moyenne = 1.0;
        if ( Moyenne > 0 )
        {
            //  Baisse un peu les probas des atouts restants pour que la somme ne dépasse pas moyenne
            for ( i = 0; i < 22; i++ )
            {
                if ( pJeu->ProbCarte[JoueurAvecPoignee][i] < 1.0 && pJeu->ProbCarte[JoueurAvecPoignee][i] > 0 )
                    ProbAtoutRestant += pJeu->ProbCarte[JoueurAvecPoignee][i];
            }
            if ( ProbAtoutRestant > Moyenne )
            {
                ProbRegle = ProbAtoutRestant / Moyenne;
                ProbRegle = 1.0 - (1.0 / ProbRegle);
                for ( i = 1; i < 22; i++)
                {
                    if ( pJeu->ProbCarte[JoueurAvecPoignee][i] == 0.0) continue;
                    if ( pJeu->ProbCarte[JoueurAvecPoignee][i] == 1.0) continue;
                    BaisseProba(CurrentGame, Position, JoueurAvecPoignee, i, ProbRegle);
                }
            }
        }
    }
    CopieProba2Tmp(pJeu);
    //  Recalcule le nombre moyen d'atout du preneur après avoir vu la poignée.
    pJeu->AtoutPreneur = AvgLongueur(pJeu, pJeu->PositionPreneur, ATOUT);
}

//  Action de "voir" la poignée.
//  Mise à jour des connaissances sur le jeu du preneur

void RegardePoigneeJoueurs(TarotGame CurrentGame, int JoueurAvecPoignee)
{
int j;

    for ( j = 0; j < 4; j++)
    {
        if ( j == JoueurAvecPoignee ) continue;        //  Pas la peine pour celui avec la poignée
        RegardePoignee(CurrentGame, j, JoueurAvecPoignee);
    }
}


//  Met les atouts levés dans la poignée de SUD

void SelectAtoutsLeveePoignee(TarotGame CurrentGame)
{
int i, j;

    for (j = 0, i = 0; i < MAX_CARTE_JOUEUR; i++)
    {
        if ( CurrentGame->isCarteLevee[i] )
        {
            CurrentGame->ListeAtoutPoignee[SUD][j] = CurrentGame->IdxCartesJoueur[SUD][i];
            j++;
        }
    }
    CurrentGame->NumAtoutPoignee[SUD] = j;
    assert(j == CurrentGame->NbCarteLevee);
    RegardePoigneeJoueurs(CurrentGame, SUD);
}
