#include <stdlib.h>
#include <gtk/gtk.h>
#include <time.h>
#include <assert.h>
#include "Tarot_Ui_Objects.h"

const int Startof[NB_COULEUR] = { 0, 1, 22, 36, 50, 64};
const int Endof[NB_COULEUR] = { 1, 22, 36, 50, 64, 78};

double StyleJoueur[MAX_JOUEURS][NB_ENTREE_STYLE_JEU];

void InitDistribue(TarotGame CurrentGame)
{
int i;

    srand( time( NULL ) );
    CurrentGame->JoueurDistrib = rand() % 3;      //  1er joueur à distribuer aléatoire
    for ( i = 0; i < MAX_JOUEURS; i++ )
    {
        CurrentGame->CartePli[i] = -1;
        CurrentGame->AnnonceJoueur[i] = -1;
    }
}

//  Initialise la structure Jeu

void RepartitionCarte(TarotGame CurrentGame)
{
int i,j,k;

	for ( j = 0; j < 78; j++)		//	Cartes non attribuées, non jouées..
	{
		CurrentGame->CartesJeu[j].isPlayed = 0;
		CurrentGame->CartesJeu[j].Index = j;
        CurrentGame->CartesJeu[j].Joueur = CurrentGame->Carte2Joueur[j];
		if ( j == 0 )
		{
			CurrentGame->CartesJeu[j].Couleur = EXCUSE;
			CurrentGame->CartesJeu[j].Hauteur = j;
			CurrentGame->CartesJeu[j].Valeur = 9;
		}
		else if ( j < 22 )
		{
			CurrentGame->CartesJeu[j].Couleur = ATOUT;
			CurrentGame->CartesJeu[j].Hauteur = j;
			CurrentGame->CartesJeu[j].Valeur = 1;
			if ( j == 1 || j == 21 )
				CurrentGame->CartesJeu[j].Valeur = 9;
		}
		else if ( j < 36 )
		{
			CurrentGame->CartesJeu[j].Couleur = TREFLE;
			CurrentGame->CartesJeu[j].Hauteur = j - 21;
			CurrentGame->CartesJeu[j].Valeur = 1;
			if ( CurrentGame->CartesJeu[j].Hauteur >= VALET )
				CurrentGame->CartesJeu[j].Valeur = (CurrentGame->CartesJeu[j].Hauteur - 10)*2 + 1;
		}
		else if ( j < 50 )
		{
			CurrentGame->CartesJeu[j].Couleur = CARREAU;
			CurrentGame->CartesJeu[j].Hauteur = j - 35;
			CurrentGame->CartesJeu[j].Valeur = 1;
			if ( CurrentGame->CartesJeu[j].Hauteur >= VALET )
				CurrentGame->CartesJeu[j].Valeur = (CurrentGame->CartesJeu[j].Hauteur - 10)*2 + 1;
		}
		else if ( j < 64 )
		{
			CurrentGame->CartesJeu[j].Couleur = PIQUE;
			CurrentGame->CartesJeu[j].Hauteur = j - 49;
			CurrentGame->CartesJeu[j].Valeur = 1;
			if ( CurrentGame->CartesJeu[j].Hauteur >= VALET )
				CurrentGame->CartesJeu[j].Valeur = (CurrentGame->CartesJeu[j].Hauteur - 10)*2 + 1;
		}
		else
		{
			CurrentGame->CartesJeu[j].Couleur = COEUR;
			CurrentGame->CartesJeu[j].Hauteur = j - 63;
			CurrentGame->CartesJeu[j].Valeur = 1;
			if ( CurrentGame->CartesJeu[j].Hauteur >= VALET )
				CurrentGame->CartesJeu[j].Valeur = (CurrentGame->CartesJeu[j].Hauteur - 10)*2 + 1;
		}
	}
	//	Initialise maintenant les autres champs de CurrentGame
	for ( k = SUD; k < CHIEN; k++)
	{
	    CurrentGame->CartePli[k] = -1;          //  Pas encore de pli...
		CurrentGame->NbCarteJoueur[k] = 0;      //  Pas de cartes pour le joueur, sera incrémenté au dessous
		for ( i = 0; i < 78; i++ )
		{
			if ( CurrentGame->CartesJeu[i].Joueur == k )
			{
				CurrentGame->IdxCartesJoueur[k][CurrentGame->NbCarteJoueur[k]] = i;
				CurrentGame->NbCarteJoueur[k]++;
			}
		}
		if ( CurrentGame->NbCarteJoueur[k] > MAX_CARTE_JOUEUR )
            printf("ERROR : Trop de cartes pour le joueur %d, N=%d\n", k, CurrentGame->NbCarteJoueur[k]);
	}
	//  Cartes du chien
    j = 0;
    for ( i = 0; i < 78; i++ )
    {
        if ( CurrentGame->Carte2Joueur[i] == CHIEN )
        {
            CurrentGame->IdxCartesChien[j] = i;
            j++;
        }
    }
    if ( j > 6 )
        printf("Trop de cartes au chien : N=%d\n", j);
    for ( i = 0; i < MAX_CARTE_JOUEUR; i++ )
    {
        CurrentGame->isCarteLevee[i] = 0;       //  Toutes les cartes de SUD sont baissées
    }
    for ( i = 0; i < 78; i++ )
    {
        CurrentGame->CarteJouee[i] = -1;        //  Pas encore jouee
    }
}

//  Evalue valeur suite d'atouts majeurs
//  généralement : LastSuite - FirstSuite.
//  Mais si FirstSuite = 16 et LastSuite >= 19 compte les atouts 15, 14....
//  Et si FirstSuite = 16 et LastSuite == 18 compte la moitié des atouts en dessous du 16

double CompteValeurSuite(struct _Jeu *pJeu, int FirstSuite, int LastSuite)
{
double v = LastSuite - FirstSuite;
double Delta = 1.0;
int idx;
    if ( FirstSuite == 16 )
    {
        if ( LastSuite == 18) Delta = 0.5;      //  Si Suite finit à 18, ajoute la moitié
        if ( LastSuite >= 18 )
        {
            for ( idx = 15; idx > 0; idx--)
            {
                if ( pJeu->StatusCartes[idx] > 0 )  //  Possède cette carte
                    v += Delta;
                else
                    break;
            }
        }
    }
    return v;
}
//	Evalue le jeu d'un joueur
//  Retourne 1 si petit imprenable
//  Initialise également la structure _Jeu pour le joueur

int EvalJeu(TarotGame CurrentGame, int Joueur)
{
int i, j;
int FirstSuite = 0;
int LastSuite = 0;
double ValeurSuite = 0;
int HasPetit = 0;
int HasDame = -1;
int NbHonneur[NB_COULEUR] = {0, 0, 0, 0, 0, 0};
double d = 0;
int SAtout = 0;     //  Somme des valeurs des atouts
struct _Jeu *pJeu;
double ValeurAtouts = 0;
double ValeurHonneur = 0;
double ValeurDistrib = 0;

    //printf("Evalue jeu de %d\n", Joueur);
    pJeu = &CurrentGame->JeuJoueur[Joueur];
    //  First clear struct JeuJoueur
    memset(pJeu, 0, sizeof(struct _Jeu));
    pJeu->PositionJoueur = Joueur;          //  Init position
    pJeu->PositionPreneur = CurrentGame->JoueurPreneur;
    //  Then copy cards from Current Game
    j = 0;
    for ( i = 0; i < 78; i++ )
    {
        if ( CurrentGame->CartesJeu[i].Joueur == Joueur )
        {
            pJeu->MyCarte[j] = CurrentGame->CartesJeu[i];
            pJeu->StatusCartes[i] = 1;      //  Possède cette carte
            j++;
        }
    }
    //  Méthode décrite dans le livre : le Jeu de Tarot (N. Chavey édité par Hatier)
    //  évalue chaque carte
	for ( i = 0; i < CurrentGame->NbCarteJoueur[Joueur]; i++)
	{
		if ( pJeu->MyCarte[i].Couleur == EXCUSE )
		{		//	Excuse : 8 points
			pJeu->NbAtout++;
			pJeu->NbBout++;
			ValeurAtouts += 8.0;
		}
		else if ( pJeu->MyCarte[i].Couleur == ATOUT )
		{
			pJeu->NbAtout++;
			SAtout += pJeu->MyCarte[i].Hauteur;
			if ( pJeu->MyCarte[i].Hauteur == 21 )
			{	//	21 : 10 points
				pJeu->NbBout++;
				ValeurAtouts += 10;
				pJeu->NbAtoutMaj++;         //  Un atout majeur
				if ( LastSuite == 20 )      //  Suite finissant au 20 ?
				{
					ValeurSuite += CompteValeurSuite(pJeu, FirstSuite, 21);    //  Compte les points de la suite
					FirstSuite = 0;
				}
			}
			else if ( pJeu->MyCarte[i].Hauteur >= 16 )
			{
				pJeu->NbAtoutMaj++;
				if ( FirstSuite == 0 )
				{
					FirstSuite = pJeu->MyCarte[i].Hauteur;
					LastSuite = pJeu->MyCarte[i].Hauteur;
				}
				else
				{
					if ( pJeu->MyCarte[i].Hauteur == LastSuite + 1 )
					{
						LastSuite = pJeu->MyCarte[i].Hauteur;
					}
					else
					{
						ValeurSuite += CompteValeurSuite(pJeu, FirstSuite, LastSuite);
						FirstSuite = pJeu->MyCarte[i].Hauteur;
						LastSuite = pJeu->MyCarte[i].Hauteur;
					}
				}
			}
			else if ( pJeu->MyCarte[i].Hauteur == 1)
			{
				pJeu->NbBout++;
				HasPetit = 1;
			}
		}
		else
		{
			pJeu->NbCouleur[pJeu->MyCarte[i].Couleur]++;
			if ( pJeu->MyCarte[i].Hauteur == VALET )
			{
				ValeurHonneur += 1.0 + StyleJoueur[Joueur][dStyleNbPoints] * 0.5;
				pJeu->ForceMain[Joueur] += 1.0;
				HasDame = -1;
			}
			else if ( pJeu->MyCarte[i].Hauteur == CAVALIER )
			{
				ValeurHonneur += 2.0 + StyleJoueur[Joueur][dStyleNbPoints] ;
				HasDame = -1;
				pJeu->ForceMain[Joueur] += 2.0;
			}
			else if ( pJeu->MyCarte[i].Hauteur == DAME )
			{
				ValeurHonneur += 3 + StyleJoueur[Joueur][dStyleNbPoints] * 1.5 ;
				HasDame = pJeu->MyCarte[i].Couleur;
				pJeu->ForceMain[Joueur] += 3.0;
				if ( pJeu->NbCouleur[pJeu->MyCarte[i].Couleur] >= 5 )
					pJeu->ForceMain[Joueur] += 1;
			}
			else if ( pJeu->MyCarte[i].Hauteur == ROI )
			{
				ValeurHonneur += 6 + StyleJoueur[Joueur][dStyleNbPoints] * 2;
				if ( HasDame == pJeu->MyCarte[i].Couleur)
					ValeurHonneur++;
				HasDame = -1;
				pJeu->ForceMain[Joueur] += 4.0;
				if ( pJeu->NbCouleur[pJeu->MyCarte[i].Couleur] >= 4 )
					pJeu->ForceMain[Joueur] += 2.0;
			}
			else
				HasDame = -1;
		}

	}
	//  Valeur liée aux atouts
	if ( FirstSuite != 0 )
	{
		ValeurSuite += CompteValeurSuite(pJeu, FirstSuite, LastSuite);
	}
	if ( HasPetit )
	{
		if ( pJeu->NbAtout == 1 )
			pJeu->PetitImprenable = 1;
		else if (pJeu->NbAtout == 5)
			ValeurAtouts += 5;
		else if (pJeu->NbAtout == 6 )
			ValeurAtouts += 7;
		else if ( pJeu->NbAtout >= 7 )
			ValeurAtouts += 9;
	}
	//  2 points par atout si plus de 4 atouts et 2 points par atout fort
	if ( pJeu->NbAtout >= 4 )
    {
        ValeurAtouts += (2.0 + StyleJoueur[Joueur][dStyleNbAtout] * 0.5)* pJeu->NbAtout + 2.0*pJeu->NbAtoutMaj;
        ValeurAtouts += ValeurSuite;
        //  Ajoute des points si favorise les bouts
        if ( pJeu->NbBout == 0)
            ValeurAtouts -= 20*StyleJoueur[Joueur][dStyleBout];      //  -10 points si pas de bouts
        else if ( pJeu->NbBout == 1)
            ValeurAtouts -= 8*StyleJoueur[Joueur][dStyleBout];      //  -4 points si 1 bout
        else if ( pJeu->NbBout == 3)
            ValeurAtouts += 6*StyleJoueur[Joueur][dStyleBout];      //  2 points si 3 bouts
    }
	//  AJout personnel : 3 points par atout supplémentaire à partir de 8 atouts
	if ( pJeu->NbAtout >= 9 )
		ValeurAtouts += (pJeu->NbAtout - 7) * (3 + StyleJoueur[Joueur][dStyleNbAtout]);
    //  Et moins le carré de 6 - nombre d'atout si moins de 6
	else if ( pJeu->NbAtout < 6 )
		ValeurAtouts -= (1.0 + StyleJoueur[Joueur][dStyleNbAtout]) * (6 - pJeu->NbAtout) * (6 - pJeu->NbAtout);
    //  Calcule la force de sa propre main si défense
	pJeu->ForceMain[Joueur] /= 50.0;
	d = 1.11*(pJeu->NbAtout - 0.5*(pJeu->MyCarte[0].Couleur == EXCUSE));
	pJeu->ForceMain[Joueur] += (d*d + 2.5*pJeu->NbAtoutMaj*pJeu->NbAtoutMaj) / 78.0;
	if ( pJeu->NbAtout >= 5 && SAtout >= pJeu->NbAtout*10)
		pJeu->ForceMain[Joueur] += d * d * ((1.0 * SAtout) / pJeu->NbAtout - 10.0) / 700.0;
	if ( d < 5.5 )
		pJeu->ForceMain[Joueur] /= 2.0;
	CompteHonneur(CurrentGame, Joueur, NbHonneur);
	for ( i = TREFLE; i < NB_COULEUR; i++)
	{
		if ( pJeu->NbCouleur[i] == 5 && pJeu->NbAtout >= 6 )
			ValeurDistrib += 5 + StyleJoueur[Joueur][dStyleLongues] * 1.5;
		if ( pJeu->NbCouleur[i] >= 6 && pJeu->NbAtout >= 6 )
			ValeurDistrib += 7 + StyleJoueur[Joueur][dStyleLongues] * 2;
		if ( pJeu->NbCouleur[i] >= 7 && pJeu->NbAtout >= 7 )
			ValeurDistrib += 2 + StyleJoueur[Joueur][dStyleLongues];
		if ( pJeu->NbCouleur[i] >= 5 && NbHonneur[i] > 0)
			pJeu->ForceMain[Joueur] += (0.01+0.03*pJeu->NbCouleurChien[i])*(pJeu->NbCouleur[i] - 4);
	}
    pJeu->NbPoint = ValeurAtouts + ValeurHonneur + ValeurDistrib;
	pJeu->NbPointDH = pJeu->NbPoint;
	for ( i = TREFLE; i < NB_COULEUR; i++)
	{
		if ( pJeu->NbCouleur[i] == 0 ) pJeu->NbPointDH += 6;
		if ( pJeu->NbCouleur[i] == 1 ) pJeu->NbPointDH += 3;
	}
    //printf("Joueur %d: Valeur atouts = %.1f, Honneur %.1f, Distrib %.1f --> Total %.1f\n", Joueur, ValeurAtouts, ValeurHonneur, ValeurDistrib, pJeu->NbPoint);
    //printf("  Pour contrat S ou C %.1f  ForceMain : %.2f\n", pJeu->NbPointDH, pJeu->ForceMain[Joueur]);
    return pJeu->PetitImprenable;
}

//  Evalue les jeux.
//  Retourne 100 + numéro joueur si non jouable (petit impremnable)

int EvalGames(TarotGame CurrentGame)
{
int i;
int Ret = 0;

    for ( i = SUD; i < CHIEN; i++ )
    {
        Ret = EvalJeu(CurrentGame, i);
        if ( Ret )
        {
            CurrentGame->PartiePetitImprenable = 1;
            return(100+Ret);
        }
    }
    return 0;
}

#define MAX_TRY_DISTRIB 1000
static char strPetitImprenable[256];

//  Distribue le jeu entre les joueurs
void DistribueJeu(TarotGame CurrentGame)
{
int i, j, k;
int CartesDistribuees;
int n_Trefle = 0;
int n_Carreau = 0;
int n_Coeur = 0;
int n_Pique = 0;
int nbTry;
int Ret;
int SaveDistrib = CurrentGame->JoueurDistrib;

    memset(CurrentGame, 0, sizeof(struct _Tarot_Partie));        //  Force tout à 0
    for ( i = 0; i < 4; i++ )
        CurrentGame->FlagAfficheJoueur[i] = AfficheJeuJoueurs[i];
    CurrentGame->JoueurDistrib = ( SaveDistrib + 1) & 3;
    CurrentGame->JoueurAffichePoignee = -1;
    CurrentGame->NumPartieEnregistree = -1;
    for ( i = 0; i < MAX_JOUEURS; i++ )
    {
        CurrentGame->CartePli[i] = -1;
        CurrentGame->AnnonceJoueur[i] = -1;
    }
    for ( nbTry = 0; nbTry < MAX_TRY_DISTRIB; nbTry++)
    {
        for ( i = 0; i < 78; i++)		//	Cartes non attribuees
        {
            CurrentGame->Carte2Joueur[i] = -1;      //  Non affectée
        }
        CartesDistribuees = 0;
        //	Distribue d'abord SUD pour satisfaire les contraintes
        for ( i = 0; i < ContraintesDistribution.MinBout; i++ )
        {
            do
            {
                j = rand() % 3;
                if ( j == 2 ) j = 21;
            } while ( CurrentGame->Carte2Joueur[j] >= 0 );
            CurrentGame->Carte2Joueur[j] = SUD;
            CartesDistribuees++;
        }
        for ( i = ContraintesDistribution.MinBout; i < ContraintesDistribution.MinAtout; i++ )
        {
            do
            {
                j = rand() % 22;
            } while ( CurrentGame->Carte2Joueur[j] >= 0 );
            CurrentGame->Carte2Joueur[j] = SUD;
            CartesDistribuees++;
        }

        for ( i = 0; i < ContraintesDistribution.MinRoi; i++)
        {
            do
            {
                j = rand() % 4;
                j = 14 * j + 22 + 13;
            } while ( CurrentGame->Carte2Joueur[j] >= 0 );
            CurrentGame->Carte2Joueur[j] = SUD;
            CartesDistribuees++;
            if ( j < 36 )
            {
                n_Trefle = 1;
            }
            else if ( j < 50 )
            {
                n_Carreau = 1;
            }
            else if ( j < 64 )
            {
                n_Coeur = 1;
            }
            else
            {
                n_Pique = 1;
            }
        }
        for ( ; n_Trefle < ContraintesDistribution.MinTrefle; n_Trefle++ )
        {
            do
            {
                j = (rand() % 14) + 22;
            } while ( CurrentGame->Carte2Joueur[j] >= 0 );
            CurrentGame->Carte2Joueur[j] = SUD;
            CartesDistribuees++;
        }
        for ( ; n_Carreau < ContraintesDistribution.MinTrefle; n_Carreau++ )
        {
            do
            {
                j = (rand() % 14) + 36;
            } while ( CurrentGame->Carte2Joueur[j] >= 0 );
            CurrentGame->Carte2Joueur[j] = SUD;
            CartesDistribuees++;
        }
        for (; n_Pique < ContraintesDistribution.MinPique; n_Pique++ )
        {
            do
            {
                j = (rand() % 14) + 50;
            } while ( CurrentGame->Carte2Joueur[j] >= 0 );
            CurrentGame->Carte2Joueur[j] = SUD;
            CartesDistribuees++;
        }
        for ( ; n_Coeur < ContraintesDistribution.MinCoeur; n_Coeur++ )
        {
            do
            {
                j = (rand() % 14) + 64;
            } while ( CurrentGame->Carte2Joueur[j] >= 0 );
            CurrentGame->Carte2Joueur[j] = SUD;
            CartesDistribuees++;
        }
        //  Et maintenant le reste
        for ( i = CartesDistribuees; i < 18; i++)
        {
            do
            {
                j = (rand() % 78);
            } while ( CurrentGame->Carte2Joueur[j] >= 0 );
            CurrentGame->Carte2Joueur[j] = SUD;
        }
        //	Distribue pour les autres joueurs
        for ( k = EST; k < CHIEN; k++ )
        {
            for ( i = 0; i < 18; i++)
            {
                do
                {
                    j = (rand() % 78);
                } while ( CurrentGame->Carte2Joueur[j] >= 0 );
                CurrentGame->Carte2Joueur[j] = k;
            }
        }
        //	Met les cartes du chien ( celles qui restent...)
        int nb_chien = 0;
        for ( j = 0; j < 78; j++ )
        {
            if ( CurrentGame->Carte2Joueur[j] < 0 )
            {
                CurrentGame->Carte2Joueur[j] = CHIEN;
                nb_chien++;
            }
        }
        assert(nb_chien == 6);
        RepartitionCarte(CurrentGame);
        Ret = EvalGames(CurrentGame);             //  Evalue la force des différents jeux
        //  Check requirements Min / Max points for SOUTH
        if ( ContraintesDistribution.MinPoints > 0 && ContraintesDistribution.MinPoints > CurrentGame->JeuJoueur[SUD].NbPointDH ) continue;
        if ( ContraintesDistribution.MaxPoints > 0 && ContraintesDistribution.MaxPoints < CurrentGame->JeuJoueur[SUD].NbPointDH ) continue;
        break;
    }
    if ( Ret > 0 )
    {
        if ( Ret == 0 )
            snprintf(strPetitImprenable, 250, "Vous avez le petit imprenable !");
        else
            snprintf(strPetitImprenable, 250, "%s a le petit imprenable !", NomJoueurs[Ret-100]);
        CurrentGame->FlagAfficheJoueur[Ret-100] = 1;    //  Affiche le jeu dans ce cas
        CurrentGame->StateJeu = JEU_MESSAGE;     //  Nouvel état jeu : message indiquant petit imprenable
        CurrentGame->StateAfterMessage = JEU_TERMINE;
    }
    for ( i = 0; i < 78; i++ )
    {
        CurrentGame->Carte2JoueurDistrib[i] = CurrentGame->Carte2Joueur[i];
    }
    OutDebug("\n\n------------------------------------------------\n");
    OutDebug("Distribue nouvelle partie, valeur jeux :\n" );
    OutDebug("SUD : %d, %d (DH)\n", CurrentGame->JeuJoueur[SUD].NbPoint, CurrentGame->JeuJoueur[SUD].NbPointDH );
    OutDebug("EST : %d, %d (DH)\n", CurrentGame->JeuJoueur[EST].NbPoint, CurrentGame->JeuJoueur[EST].NbPointDH );
    OutDebug("NORD : %d, %d (DH)\n", CurrentGame->JeuJoueur[NORD].NbPoint, CurrentGame->JeuJoueur[NORD].NbPointDH );
    OutDebug("OUEST : %d, %d (DH)\n", CurrentGame->JeuJoueur[OUEST].NbPoint, CurrentGame->JeuJoueur[OUEST].NbPointDH );
    OutDebug("\n\n");
    CurrentGame->StateJeu = JEU_ATTENTE_CONTRAT_J1;     //  Nouvel état jeu : attente décision joueur après preneur
}

//  Génère des parties pour faire des statistiques

#define NOMBRE_PARTIE_STATS 5000000

int NbOccurences[NB_TYPE_PARTIE][78];
int TypeParties[NB_TYPE_PARTIE];
double NbElements[NB_TYPE_PARTIE];

void OutputStats(const char *Name)
{
FILE *f;
int i;

    f = fopen(Name, "w");
    fprintf(f, "Stats pour PASSE : NbEntree = %.0f\n", NbElements[PASSE]);
    for ( i = 0; i < 78; i++ )
    {
        fprintf(f, "%.6f", NbOccurences[PASSE][i]/NbElements[PASSE]);
        if ( i == 77)
            fprintf(f, "\n");
        else if ( i == 10 || i == 21 || i == 35 || i == 49 || i == 63 )
            fprintf(f, ",\n");
        else
            fprintf(f, ", ");

    }
    fprintf(f, "\n\nStats pour PETITE : NbEntree = %.0f\n", NbElements[PETITE]);
    for ( i = 0; i < 78; i++ )
    {
        fprintf(f, "%.6f", NbOccurences[PETITE][i]/NbElements[PETITE]);
        if ( i == 77)
            fprintf(f, "\n");
        else if ( i == 10 || i == 21 || i == 35 || i == 49 || i == 63 )
            fprintf(f, ",\n");
        else
            fprintf(f, ", ");
    }
    fprintf(f, "\n\nStats pour GARDE : NbEntree = %.0f\n", NbElements[GARDE]);
    for ( i = 0; i < 78; i++ )
    {
        fprintf(f, "%.6f", NbOccurences[GARDE][i]/NbElements[GARDE]);
        if ( i == 77)
            fprintf(f, "\n");
        else if ( i == 10 || i == 21 || i == 35 || i == 49 || i == 63 )
            fprintf(f, ",\n");
        else
            fprintf(f, ", ");
    }
    fprintf(f, "\n\nStats pour GARDE_SANS : NbEntree = %.0f\n", NbElements[GARDE_SANS]);
    for ( i = 0; i < 78; i++ )
    {
        fprintf(f, "%.6f", NbOccurences[GARDE_SANS][i]/NbElements[GARDE_SANS]);
        if ( i == 77)
            fprintf(f, "\n");
        else if ( i == 10 || i == 21 || i == 35 || i == 49 || i == 63 )
            fprintf(f, ",\n");
        else
            fprintf(f, ", ");
    }
    fprintf(f, "\n\nStats pour GARDE_CONTRE : NbEntree = %.0f\n", NbElements[GARDE_CONTRE]);
    for ( i = 0; i < 78; i++ )
    {
        fprintf(f, "%.6f", NbOccurences[GARDE_CONTRE][i]/NbElements[GARDE_CONTRE]);
        if ( i == 77)
            fprintf(f, "\n");
        else if ( i == 10 || i == 21 || i == 35 || i == 49 || i == 63 )
            fprintf(f, ",\n");
        else
            fprintf(f, ", ");
    }
    fclose(f);
}

void GenereParties(TarotGame CurrentGame)
{
int NbPartie;
int i, j;
int Annonce, Contrat;
int Preneur;

    memset(TypeParties, 0, sizeof(TypeParties));
    memset(NbElements, 0, sizeof(NbElements));
    memset(NbOccurences, 0, sizeof(NbOccurences));
    for ( NbPartie = 0; NbPartie < NOMBRE_PARTIE_STATS; NbPartie++)
    {
        DistribueJeu(CurrentGame);
        Annonce = PASSE;
        Preneur = -1;
        for ( j = 0; j < 4; j++ )
        {
            Contrat = ChoixContrat(CurrentGame, j);
            if ( Contrat > Annonce )
            {
                Annonce = Contrat;
                Preneur = j;
            }
        }
        TypeParties[Annonce]++;
        for ( j = 0; j < 4; j++ )
        {
            if ( Annonce > PASSE && j == Preneur )
            {
                for ( i = 0; i < 18; i++ )
                {
                    NbOccurences[Annonce][CurrentGame->IdxCartesJoueur[Preneur][i]]++;
                }
                NbElements[Annonce]++;
            }
            else
            {
                for ( i = 0; i < 18; i++ )
                {
                    NbOccurences[PASSE][CurrentGame->IdxCartesJoueur[j][i]]++;
                }
                NbElements[PASSE]++;
            }

        }
        if ( NbPartie % 1000 == 999 )
        {
            printf("NbPartie = %d. Passe = %d, Prise = %d, Garde = %d, Sans = %d, Contre = %d\n", NbPartie+1,
                   TypeParties[0], TypeParties[1], TypeParties[2], TypeParties[3], TypeParties[4]);
        }
    }
    OutputStats("DonneTarot.txt");
}

//  Recommence la partie en cours
//  Sauve distribution, donneur, efface la structure jeu et récupère les données sauvées

void RejoueJeu(TarotGame CurrentGame)
{
int i;
int SaveDistrib = CurrentGame->JoueurDistrib;
int SaveNumPartieEnregistree = CurrentGame->NumPartieEnregistree;
int SavePreneurPartieEnregistree = CurrentGame->PreneurPartieEnregistree;
int SaveCartes[78];

    for ( i = 0; i < 78; i++)
        SaveCartes[i] = CurrentGame->Carte2JoueurDistrib[i];
    memset(CurrentGame, 0, sizeof(struct _Tarot_Partie));        //  Force tout à 0
    for ( i = 0; i < 4; i++ )
        CurrentGame->FlagAfficheJoueur[i] = AfficheJeuJoueurs[i];
    CurrentGame->JoueurDistrib = SaveDistrib;
    CurrentGame->JoueurAffichePoignee = -1;
    CurrentGame->NumPartieEnregistree = SaveNumPartieEnregistree;
    CurrentGame->PreneurPartieEnregistree = SavePreneurPartieEnregistree;
    for ( i = 0; i < MAX_JOUEURS; i++ )
    {
        CurrentGame->CartePli[i] = -1;
        CurrentGame->AnnonceJoueur[i] = -1;
    }
    for ( i = 0; i < 78; i++ )
    {
        CurrentGame->Carte2JoueurDistrib[i] = SaveCartes[i];
        CurrentGame->Carte2Joueur[i] = CurrentGame->Carte2JoueurDistrib[i];
    }
    RepartitionCarte(CurrentGame);
    EvalGames(CurrentGame);             //  Evalue la force des différents jeux
    OutDebug("\n\n\n---------------------------------------------------------\n");
    if ( CurrentGame->NumPartieEnregistree >= 0 )
    {
        OutDebug("Rejoue partie maître %d, Preneur = %d\n", CurrentGame->NumPartieEnregistree+1, CurrentGame->PreneurPartieEnregistree);
        OutDebug("\n\n");
    }
    else
    {
        OutDebug("Rejoue partie précédente\n\n");
    }
    CurrentGame->StateJeu = JEU_ATTENTE_CONTRAT_J1;     //  Nouvel état jeu : attente décision joueur après preneur
}
