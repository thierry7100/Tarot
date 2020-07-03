#include <stdlib.h>
#include <gtk/gtk.h>
#include "Tarot_Ui_Objects.h"
#include <assert.h>
#include <math.h>

//  Constantes utilisées pour la normalisation des probabilités
//
#define PRECISION_NORMALISATION 0.0001       //  Erreur par carte
#define DELTA_NORMALISATION     0.005        //  Erreur totale somme du joueur
#define LIMITE_BOUCLE           1000         //  Nombre de boucles max à réaliser

static void InitProbaPreneur(TarotGame CurrentGame, int Position);

//  Vérifie la cohérence des probas du joueur dont la struct pJeu est passée en paramètre
//  Retourne -1 en cas d'erreur, 0 si tout va bien
//  Vérifie que toutes les probabilités sont comprises entre 0 et 1
//  Vérifie que la somme pour chaque carte est égale à 1
//  Vérifie que la somme pour chaque joueur est égale à son nombre de cartes

int CheckProbaJeu(TarotGame CurrentGame, struct _Jeu *pJeu)
{
int i, j;
double ProbaCarte;
double SommeJoueur[5];

    memset(SommeJoueur, 0, sizeof(SommeJoueur));
    for ( i = 0; i < 78; i++ )
    {
        ProbaCarte = 0.0;
        //  Ne tient pas compte des cartes jouées
        if ( CurrentGame->CarteJouee[i] < 0 )
        {
            for ( j = 0; j < 5; j++ )
            {
                assert(pJeu->ProbCarte[j][i] >= 0);
                assert(pJeu->ProbCarte[j][i] <= 1.000001);
                ProbaCarte += pJeu->ProbCarte[j][i];
                SommeJoueur[j] += pJeu->ProbCarte[j][i];
            }
            assert(fabs(ProbaCarte-1.0)<PRECISION_NORMALISATION);
        }
    }
    for ( j = 0; j < 4; j++ )
    {
        assert(fabs(SommeJoueur[j]- CurrentGame->NbCarteJoueur[j])  <  4*DELTA_NORMALISATION);
    }
    //  Et pour le chien au final
    assert(fabs(SommeJoueur[4] - 6) < 2*DELTA_NORMALISATION );
    return 0;
}

//  Vérifie la cohérence des probas des cartes du joueur dont la struct pJeu est passée en paramètre
//  Retourne -1 en cas d'erreur, 0 si tout va bien
//  Vérifie que toutes les probabilités sont comprises entre 0 et 1


static int CheckProbaCartes(TarotGame CurrentGame, struct _Jeu *pJeu)
{
int i, j;

    for ( i = 0; i < 78; i++ )
    {
        if ( CurrentGame->CarteJouee[i] < 0 )
        {
            for ( j = 0; j < 5; j++ )
            {
                assert(pJeu->ProbCarte[j][i] >= 0);
                assert(pJeu->ProbCarte[j][i] <= 1.000001);
            }
        }
    }
    return 0;
}

//  Vérifie la cohérence des probas du joueur Position
//  Retourne -1 en cas d'erreur, 0 si tout va bien
//  Vérifie que toutes les probabilités sont comprises entre 0 et 1
//  Vérifie que la somme pour chaque carte est égale à 1
//  Vérifie que la somme pour chaque joueur est égale à son nombre de cartes

int CheckProbaJoueur(TarotGame CurrentGame, int Position)
{
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[Position];

    return( CheckProbaJeu(CurrentGame, pJeu));
}

//	Calcule l'erreur, c'est à dire la différence entre la somme des probas et le nombre théorique de cartes.
//  Retourne les valeurs calculées dans le tableau Erreur (5 positions : 4 joueurs + le chien)

static void CalcDeviation(TarotGame CurrentGame, struct _Jeu *pJeu, double Erreur[])
{
double s;

	for (int i = 0; i < 5; i++)			//	Pour chaque joueur (y compris le chien)
	{
		s = 0;
		for ( int j = 0; j < 78; j++)
		{
			s += pJeu->ProbCarte[i][j];
		}
		if ( i != CHIEN )
            Erreur[i] = s - CurrentGame->NbCarteJoueur[i];
        else
            Erreur[i] = s - 6.0;
#if DEBUG  > 0
        assert(!isnan(Erreur[i]));
#endif // DEBUG
	}
}


//  Copie les probas calculées entre coups vers la version temporaire

void CopieProba2Tmp(struct _Jeu *pJeu)
{
	for ( int j = 0; j < 5; j++)            //  Le fait pour les 4 joueurs et le chien
	{
		for ( int i = 0; i < 78; i++)
		{
			pJeu->TmpProbCarte[j][i] = pJeu->ProbCarte[j][i];
		}
	}
}

//  Mise à jour probabilité quand on est sûr que le joueur Position possède la carte Index

void SetCarte2Position(struct _Jeu *pJeu, int Position, int Index)
{
int j;

    for ( j = 0; j < 5; j++ )
    {
        pJeu->ProbCarte[j][Index] = 0.0;            // Passe proba à 0
        pJeu->ConfianceProb[j][Index] = 1.0;        //  On est sûr
    }
    pJeu->ProbCarte[Position][Index] = 1.0;         //  Met la bonne à 1
}

//	Initialise les probabilités d'appartenance de chaque carte à un joueur
//  Cas où le joueur est le preneur
//  Remplit la structure _Jeu du joueur Position, calcule les probas vu de ce joueur Position

static void InitProbaJoueurPreneur(TarotGame CurrentGame, int Position)
{
int i;
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[Position];

    memset(pJeu->ProbCarte, 0, sizeof(pJeu->ProbCarte));        //  Init à Probacarte 0
    memset(pJeu->ConfianceProb, 0, sizeof(pJeu->ProbCarte));    //  Init à Confiance à 0
    for ( i = 0; i < 18; i++)
	{
	    SetCarte2Position(pJeu, Position, pJeu->MyCarte[i].Index);
	}
	for ( i = 0; i < 78; i++ )      //  Sûr de ses propres cartes
    {
		pJeu->ConfianceProb[Position][i] = 1.0;
    }
    //  Le preneur connait l'écart avec certitude si contrat inférieur à garde sans
    for ( i = 0; i < 78; i++ )
    {
        if ( CurrentGame->TypePartie <= GARDE )
        {
            if ( CurrentGame->Carte2Joueur[i] == CHIEN )        //  Carte mise à l'écart ?
            {
                SetCarte2Position(pJeu, CHIEN, i);
            }
            pJeu->ConfianceProb[4][i] = 1.0;                    //  Sûr même si pas au chien
        }
        else if ( pJeu->ProbCarte[Position][i] == 0.0 )         //  Pas chez le preneur, proba de 6/60 d'être au chien
        {
            pJeu->ProbCarte[4][i] = 6.0/60.0;                    //  Proba moyenne d'être à l'écart, mais rien de sûr
        }
    }
	//  Maintenant pour les autres cartes
	for ( i = 0; i < 78; i++)
	{
		if ( pJeu->ProbCarte[Position][i] == 0.0 && pJeu->ProbCarte[4][i] != 1.0 )    //  Pas chez le joueur ni au chien ?
        {
            //  Proba pour preneur. Une carte inconnue
            if ( CurrentGame->TypePartie <= GARDE )
            {
                pJeu->ProbCarte[CurrentGame->JoueurPreneur^1][i] = 18.0/54.0;      //  Proba moyenne
                pJeu->ProbCarte[CurrentGame->JoueurPreneur^2][i] = 18.0/54.0;
                pJeu->ProbCarte[CurrentGame->JoueurPreneur^3][i] = 18.0/54.0;
            }
            else
            {
                pJeu->ProbCarte[CurrentGame->JoueurPreneur^1][i] = 18.0/60.0;      //  Proba moyenne
                pJeu->ProbCarte[CurrentGame->JoueurPreneur^2][i] = 18.0/60.0;
                pJeu->ProbCarte[CurrentGame->JoueurPreneur^3][i] = 18.0/60.0;
            }
			pJeu->ConfianceProb[Position][i] = 1.0;	//	Sur de ne pas l'avoir..., mais reste à 0 pour les autres
		}
	}
    CheckProbaJoueur(CurrentGame, Position);
    InitStructJeu(CurrentGame, pJeu);           //  Enfin, initialise le reste de la structure
}


//	Initialise les probabilités d'appartenance de chaque carte à un joueur
//  Cas où le joueur est en défense
//  Remplit la structure _Jeu du joueur Position, calcule les probas vu de ce joueur Position
//  Parmi les cartes, il faut distinguer celles
//      1) Qui sont certainement chez le preneur (Non écartable vues au chien)
//      2) Qui sont soit au chien soit chez le preneur (vues au chien)
//      3) Qui ne sont pas au chien (non écartables) donc à diviser entre Preneur, Défenseur A et Défenseur B
//      4) Qui peuvent être dans les 4 tas restants (Preneur, Défenseur A, Défenseur B, écart)

static void InitProbaJoueurDefense(TarotGame CurrentGame, int Position)
{
int i, j;
int NbSurPreneur = 0;
int NbCarteVuesChien = CurrentGame->TypePartie <= GARDE ? 6 : 0;        //  6 cartes au chien vues pour contrat <= GARDE
int NbNonEcartable = CurrentGame->TypePartie <= GARDE ? 26 : 0;        //  22 atouts + 4 rois
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[Position];
int Preneur = CurrentGame->JoueurPreneur;
double AvgCas3;

    memset(pJeu->ProbCarte, 0, sizeof(pJeu->ProbCarte));        //  Init à Probacarte 0
    memset(pJeu->ConfianceProb, 0, sizeof(pJeu->ConfianceProb));    //  Init à Confiance à 0
    for ( i = 0; i < 18; i++)
	{
	    SetCarte2Position(pJeu, Position, pJeu->MyCarte[i].Index);
		if ( isEcartable(CurrentGame, pJeu->MyCarte[i].Index) == 0 && NbNonEcartable > 0 )
            NbNonEcartable--;                        //  Une de moins de non écartable (catégorie 3) car chez le joueur Position
	}
	for ( i = 0; i < 78; i++ )      //  Sûr de ses propres cartes
    {
		pJeu->ConfianceProb[Position][i] = 1.0;
    }
    //  Compte les cartes vues au chien qui sont forcément chez le preneur
	for ( i = 0; i < 78; i++)
	{
	    if ( CurrentGame->CarteAuChien[i] && isEcartable(CurrentGame, i) == 0 )
        {
            NbSurPreneur++;
            NbNonEcartable--;                   //  A enlever de la catégorie 3 (déja en 1)
            SetCarte2Position(pJeu, Preneur, i);
        }
	}
	AvgCas3 = NbNonEcartable / 3.0;     //  En moyenne les 3 joueurs (preneur + 2 défense) ont AvgCas3 cartes nonn écartables
	//  Maintenant pour les autres cartes
	for ( i = 0; i < 78; i++)
	{
	    if ( pJeu->ProbCarte[Position][i] > 0 ) continue;       //  Chez le joueur, passe au suivant
	    if ( pJeu->ProbCarte[Preneur][i] > 0 ) continue;        //  Chez le preneur (cas 1), déja fait...
        if ( CurrentGame->CarteAuChien[i] && isEcartable(CurrentGame, i) == 1 )
        //  la carte a été vue au chien , elle peut être à l'écart ou chez le preneur
        {   //  Catégorie 2
            pJeu->ConfianceProb[Preneur^1][i] = 1.0;        //  Sûr pas en défense
            pJeu->ConfianceProb[Preneur^2][i] = 1.0;
            pJeu->ConfianceProb[Preneur^3][i] = 1.0;
            //  Choix fait : Probabilité identique d'être à l'écart ou dans le jeu pour ce type de carte
            pJeu->ProbCarte[Preneur][i] = (18.0 - NbSurPreneur- AvgCas3)/(24.0 - NbSurPreneur- AvgCas3);
            pJeu->ProbCarte[4][i] = 6.0/(24.0 - NbSurPreneur - AvgCas3);
        }
        //	Si atout ou roi pas de possibilité d'être au chien (Non écartable)
        else if ( (i <= 21 || i == 35 || i == 49 || i == 63 || i == 77 ) && CurrentGame->TypePartie <= GARDE )
        {   //  Cas 3
            for ( j = 0; j < 4; j++)
            {
                if ( j == Position ) continue;
                //  Pour le preneur les cartes non écartables sont dans son jeu, proba = 0 pour le chien
                //  Proba pour le preneur = proba de base entre les 3 jeux
                //  Pour les autres joueurs proba de base également
                pJeu->ProbCarte[j][i] = 1.0/3.0;
            }
            pJeu->ConfianceProb[4][i] = 1.0;        //  Sûr de ne pas être à l'écart si contrat <= GARDE
        }
        else
        {
            //  Cartes écartables ou contrat sans / contre chien
            //  Pour la défense proba de base, pour le preneur peut aller à l'écart ou dans son jeu
            for ( j = 0; j < 4; j++)
            {
                if ( j== Position ) continue;
                if ( j == Preneur )
                {
                    pJeu->ProbCarte[Preneur][i] = (18.0/(60.0-NbCarteVuesChien))*(18.0 - NbSurPreneur- AvgCas3)/(18.0 + NbCarteVuesChien - NbSurPreneur- AvgCas3);
                    pJeu->ProbCarte[4][i] = (18.0/(60.0-NbCarteVuesChien))*6.0/(18.0 + NbCarteVuesChien- NbSurPreneur- AvgCas3);
                }
                else
                {
                    //  Pour les autres joueurs proba de base : 18 cartes parmi 54/60 à répartir
                    pJeu->ProbCarte[j][i] = 18.0/(60.0-NbCarteVuesChien);
                }
            }
        }
        pJeu->ConfianceProb[Position][i] = 1.0;	//	Sur de ne pas l'avoir..., mais reste à 0 pour les autres
	}
    CheckProbaJoueur(CurrentGame, Position);
    if ( CurrentGame->NbAtoutEcart > 0 )                //  Il y a des atouts remis au chien !
        RegardeAtoutEcart(CurrentGame, pJeu);
    //  Puis utilise les heuristiques de proba pour le preneur vu du joueur courant
    InitProbaPreneur(CurrentGame, Position);
    InitStructJeu(CurrentGame, pJeu);                   //  Enfin, initialise le reste de la structure
    InitProbaCoupes(CurrentGame, pJeu);                 //  Init proba de coupe dans les couleurs par le preneur
}

//  Initialise les probabilités après distribution et vue du chien.

void InitProbaJoueurs(TarotGame CurrentGame)
{
int Position;

    for ( Position = 0; Position < CHIEN; Position++)
    {
        if ( Position == CurrentGame->JoueurPreneur )
            InitProbaJoueurPreneur(CurrentGame, Position);
        else
            InitProbaJoueurDefense(CurrentGame, Position);
    }
}

void CheckProbaJoueurs(TarotGame CurrentGame)
{
int Position;

    for ( Position = 0; Position < CHIEN; Position++)
    {
        CheckProbaJoueur(CurrentGame, Position);
    }
}

#if DEBUG > 0
#define DEBUG_CHECK_PROBA   1
#else
#define DEBUG_CHECK_PROBA   0
#endif // DEBUG

#if DEBUG_CHECK_PROBA > 0

//  Vérifie que les probabilités ne sont pas mal calculées

void CheckDebugProba(TarotGame CurrentGame, struct _Jeu *pJeu, int IndexCarte)
{
    if ( CurrentGame->CarteJouee[IndexCarte] >= 0 ) return;
    if ( pJeu->ProbCarte[0][IndexCarte] < 0.08 && CurrentGame->Carte2Joueur[IndexCarte] == 0 )
    {
        OutDebug("PROBLEME Proba pour joueur %d:  pJeu->ProbCarte[SUD][%d] = %.3f et Carte appartient à joueur SUD\n", pJeu->PositionJoueur, IndexCarte, pJeu->ProbCarte[0][IndexCarte]);
    }
    if ( pJeu->ProbCarte[1][IndexCarte] < 0.08 && CurrentGame->Carte2Joueur[IndexCarte] == 1 )
    {
        OutDebug("PROBLEME Proba pour joueur %d:  pJeu->ProbCarte[EST][%d] = %.3f et Carte appartient à joueur EST\n", pJeu->PositionJoueur, IndexCarte, pJeu->ProbCarte[1][IndexCarte]);
    }
    if ( pJeu->ProbCarte[2][IndexCarte] < 0.08 && CurrentGame->Carte2Joueur[IndexCarte] == 2 )
    {
        OutDebug("PROBLEME Proba pour joueur %d:  pJeu->ProbCarte[NORD][%d] = %.3f et Carte appartient à joueur NORD\n", pJeu->PositionJoueur, IndexCarte, pJeu->ProbCarte[2][IndexCarte]);
    }
    if ( pJeu->ProbCarte[3][IndexCarte] < 0.08 && CurrentGame->Carte2Joueur[IndexCarte] == 3 )
    {
        OutDebug("PROBLEME Proba pour joueur %d:  pJeu->ProbCarte[OUEST][%d] = %.3f et Carte appartient à joueur OUEST\n", pJeu->PositionJoueur, IndexCarte, pJeu->ProbCarte[3][IndexCarte]);
    }
    if ( pJeu->ProbCarte[4][IndexCarte] < 0.08 && CurrentGame->Carte2Joueur[IndexCarte] == 4 )
    {
        OutDebug("PROBLEME Proba pour joueur %d:  pJeu->ProbCarte[CHIEN][%d] = %.3f et Carte appartient au chien\n", pJeu->PositionJoueur, IndexCarte, pJeu->ProbCarte[4][IndexCarte]);
    }
    if ( pJeu->ProbCarte[SUD][IndexCarte] > 0.9 && CurrentGame->Carte2Joueur[IndexCarte] != SUD )
    {
        OutDebug("PROBLEME Proba pour joueur %d:  pJeu->ProbCarte[SUD][%d] = %.3f et Carte n'appartient pas au joueur SUD\n", pJeu->PositionJoueur, IndexCarte, pJeu->ProbCarte[0][IndexCarte]);
    }
    if ( pJeu->ProbCarte[EST][IndexCarte] > 0.9 && CurrentGame->Carte2Joueur[IndexCarte] != EST )
    {
        OutDebug("PROBLEME Proba pour joueur %d:  pJeu->ProbCarte[EST][%d] = %.3f et Carte n'appartient pas au joueur EST\n", pJeu->PositionJoueur, IndexCarte, pJeu->ProbCarte[1][IndexCarte]);
    }
    if ( pJeu->ProbCarte[NORD][IndexCarte] > 0.9 && CurrentGame->Carte2Joueur[IndexCarte] != NORD )
    {
        OutDebug("PROBLEME Proba pour joueur %d:  pJeu->ProbCarte[NORD][%d] = %.3f et Carte n'appartient pas au joueur NORD\n", pJeu->PositionJoueur, IndexCarte, pJeu->ProbCarte[2][IndexCarte]);
    }
    if ( pJeu->ProbCarte[OUEST][IndexCarte] > 0.9 && CurrentGame->Carte2Joueur[IndexCarte] != OUEST )
    {
        OutDebug("PROBLEME Proba pour joueur %d:  pJeu->ProbCarte[OUEST][%d] = %.3f et Carte n'appartient pas au joueur OUEST\n", pJeu->PositionJoueur, IndexCarte, pJeu->ProbCarte[3][IndexCarte]);
    }
    if ( pJeu->ProbCarte[CHIEN][IndexCarte] > 0.9 && CurrentGame->Carte2Joueur[IndexCarte] != CHIEN )
    {
        OutDebug("PROBLEME Proba pour joueur %d:  pJeu->ProbCarte[CHIEN][%d] = %.3f et Carte n'appartient pas au chien\n", pJeu->PositionJoueur, IndexCarte, pJeu->ProbCarte[4][IndexCarte]);
    }
}
#endif // DEBUG_CHECK_PROBA


//	Monte la proba de la carte passée en paramètre
//  Fonction appelée quand une heuristique indique qu'un joueur doit avoir une proba supérieure pour cette carte
//	Divise la proba des autres joueurs par mul, celle du joueur passé en paramètre se trouve donc augmentée
//  La somme des probas de cette carte sera donc bien de 1.
//  Appelle ensuite la fonction de normalisation pour être sur que la somme des probas sera cohérente (somme probas = nb cartes des joueurs)
//  Fait ceci vu du joueur dont Position est passée en paramètre
//      Joueur : Position du joueur qui va voir la proba montée
//      IndexCarte : Carte avec proba modifiée
//      Mul : multiplicateur de probabilité, doit être > 1 (on MONTE la proba)
//      Si FlagConfiance est à 1, mise à jour confiance proba

void MonteProba(TarotGame CurrentGame, int Position, int Joueur, int IndexCarte, double mul, int FlagConfiance)
{
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[Position];
double val= pJeu->ProbCarte[Joueur][IndexCarte];

#if DEBUG > 0
#ifndef WIN
    assert(mul > 1.0);      //  Pour attraper les erreurs en mode DEBUG
    assert(CurrentGame->CarteJouee[IndexCarte] < 0);    //  La carte ne doit pas être jouée !
#endif
#endif
	if ( mul <= 1.0 ) return;
	//	Teste si carte déja jouée ?
	if ( CurrentGame->CarteJouee[IndexCarte] >= 0 ) return;     //  Si carte déjà jouée, ne fait rien.
	//	Si proba trop petite, ne change rien
	if ( val <= 1e-7 ) return;
	//	Divise la proba des autres joueurs
	//  Peut le faire pour les 3 autres joueurs + l'écart sans problème (même pour son propre jeu)
	pJeu->ProbCarte[Joueur^1][IndexCarte] /= mul;
	pJeu->ProbCarte[Joueur^2][IndexCarte] /= mul;
	pJeu->ProbCarte[Joueur^3][IndexCarte] /= mul;
	pJeu->ProbCarte[4][IndexCarte] /= mul;
	//  Monte confiance si besoin est.
	//  Ex : si mul = 2, met Confiance à 0.5
	if ( FlagConfiance && pJeu->ConfianceProb[Joueur][IndexCarte] < 1.0 - 1.0/mul )
		pJeu->ConfianceProb[Joueur][IndexCarte] = 1.0 - 1.0/mul;
	//	Celle du Joueur est égale à 1 - la somme des autres
	pJeu->ProbCarte[Joueur][IndexCarte] = 1.0 - pJeu->ProbCarte[Joueur^1][IndexCarte]
		- pJeu->ProbCarte[Joueur^2][IndexCarte] - pJeu->ProbCarte[Joueur^3][IndexCarte]
		- pJeu->ProbCarte[4][IndexCarte];
#if DEBUG_CHECK_PROBA
    CheckDebugProba(CurrentGame, pJeu, IndexCarte);
#endif // DEBUG_CHECK_PROBA > 0
#if DEBUG > 0
	//	Teste que tout est OK
	assert(pJeu->ProbCarte[0][IndexCarte] <= 1.000001);
	assert(pJeu->ProbCarte[1][IndexCarte] <= 1.000001);
	assert(pJeu->ProbCarte[2][IndexCarte] <= 1.000001);
	assert(pJeu->ProbCarte[3][IndexCarte] <= 1.000001);
	assert(pJeu->ProbCarte[4][IndexCarte] <= 1.000001);
#endif
	RenormaliseDescendant(pJeu, Joueur, CurrentGame->NbCarteJoueur[Position]);          //  On a monté la proba du Joueur, normalise donc vers le bas (sa somme est trop grande).
}

//	Descend la probabilite d'une carte. La proba initiale est multipliée par 1 - Probregle.
//	S'emploie quand on est sur a x% que le joueur ne possede pas la carte
//	Divise la proba des autres joueurs par mul, celle du joueur passé en paramètre se trouve donc augmentée
//  La somme des probas de cette carte sera donc bien de 1.
//  Appelle ensuite la fonction de normalisation pour être sur que la somme des probas sera cohérente (somme probas = nb cartes des joueurs)
//  Fait ceci vu du joueur dont Position est passée en paramètre
//      Joueur : Position du joueur qui va voir la proba montée
//      IndexCarte : Carte avec proba modifiée
//      Mul : multiplicateur de probabilité, doit être > 1 (on MONTE la proba)


void BaisseProba(TarotGame CurrentGame, int Position, int Joueur, int IndexCarte, double ProbRegle)
{
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[Position];
double ProbaInitiale = pJeu->ProbCarte[Joueur][IndexCarte];
double Facteur;
int nb, loc;
int i;
int j;

	//	Teste paramètres corrects
#if DEBUG > 0
#ifndef WIN
	if ( isnan(ProbaInitiale) ) assert(0);	//	Doit être valide
	assert(ProbaInitiale <= 0.9999999999);	//	Et inférieure (strictement) à 1
	assert(ProbaInitiale > -0.0000000001);	//	Et supérieure à 0
	assert(ProbRegle <= 1.0);			//	Validite ProbRegle
	assert(ProbRegle >= 0.0);
	assert(IndexCarte >= 0);
	assert(IndexCarte < 78);
#endif
#endif // DEBUG
	//	Teste si carte déjà jouée ?
	if ( CurrentGame->CarteJouee[IndexCarte] >= 0) return;		//	Si positif, déjà carte jouée
	//	Multiplie proba initiale
	if ( fabs(ProbaInitiale - 1.0) < 1e-6 )	//	Si trop proche de 1, ne fait rien...
		return;
    pJeu->ProbCarte[Joueur][IndexCarte] *= (1.0 - ProbRegle);
    if ( ProbaInitiale < 1e-6 ) return;
#if DEBUG > 0
	//	S'assure que tout est OK
	assert(pJeu->ProbCarte[Joueur][IndexCarte] >= 0);
	assert(pJeu->ProbCarte[Joueur][IndexCarte] <= 1.0000000001);
#endif // DEBUG
	//	Facteur multiplicatif pour les autres
	Facteur = (1.0 - pJeu->ProbCarte[Joueur][IndexCarte])/(1.0 - ProbaInitiale);
	if ( Facteur < 1e-12 ) Facteur = 1e-12;
	//	Modifie les probas des autres
	for ( j = 0; j <= 4; j++ )
    {
        if ( j == Joueur ) continue;
        pJeu->ProbCarte[j][IndexCarte] *= Facteur;
        //	Tests aux limites, s'assure que les probas restent inférieures à 1
        if (pJeu->ProbCarte[j][IndexCarte] > 1.0) pJeu->ProbCarte[j][IndexCarte] = 1.0;
    }
    //	Pour "enlever" l'effet des erreurs, normalise la somme des probas pour cette carte à 1...
	Facteur = pJeu->ProbCarte[0][IndexCarte] + pJeu->ProbCarte[1][IndexCarte] + pJeu->ProbCarte[2][IndexCarte]
			+ pJeu->ProbCarte[3][IndexCarte] + pJeu->ProbCarte[4][IndexCarte];
	for ( i = 0; i <= 4; i++)
		pJeu->ProbCarte[i][IndexCarte] /= Facteur;
#if DEBUG > 0
	//	Teste que tout est OK
	for ( i = 0; i <= 4; i++)
    {
        assert(pJeu->ProbCarte[i][IndexCarte] <= 1.0000000001);
        assert(pJeu->ProbCarte[i][IndexCarte] >= 0.0);
    }
#endif
	//	Mise à jour Confiance
	if ( pJeu->ConfianceProb[Joueur][IndexCarte] < ProbRegle )
		pJeu->ConfianceProb[Joueur][IndexCarte] = ProbRegle;
    //  Compte le nombre de joueurs pouvant avoir cette carte
	for ( loc = Joueur, nb = 0, i = 0; i < 5; i++)
	{
		if ( i == Joueur ) continue;
		if ( pJeu->ProbCarte[i][IndexCarte] > 0 )
		{
			nb++;
			loc = i;
		}
	}
	if ( nb == 1 )
	{
		//	Si plus que deux à en avoir (Joueur + autre), hérite de la confiance
		pJeu->ConfianceProb[loc][IndexCarte] = pJeu->ConfianceProb[Joueur][IndexCarte];
	}
	RenormaliseMontant(pJeu, Joueur, CurrentGame->NbCarteJoueur[Position]);
#if DEBUG_CHECK_PROBA > 0
    CheckDebugProba(CurrentGame, pJeu, IndexCarte);
#endif // DEBUG_CHECK_PROBA
}

//	Normalise les probas dans le cas général
//  Pour une question d'éfficacité, plutôt utiliser RenormaliseMontant ou RenormaliseDescendant
//	Assure que Somme(Proba(Joueur)) = Nombre de cartes attendu
//  Le fait vu pour le joueur dont la struct _Jeu pJeu est passée en paramètre

#if DEBUG > 0
#define DEBUG_NORMALISE_PROBA 0
#else
#define DEBUG_NORMALISE_PROBA 0
#endif  // DEBUG

void NormaliseProba(TarotGame CurrentGame, struct _Jeu *pJeu)
{
double Erreur[5];
double SommeErr;
int Boucle = 0;
double MaxErr;
int ColonneErr;
double SConfiance, MaxValErr;
double TmpConf, Delta;
double Diviseur;
int NoConfiance;
int i;

	CheckProbaCartes(CurrentGame, pJeu);
	CalcDeviation(CurrentGame, pJeu, Erreur);		//	Calcule les erreurs sur chaque joueur
	SommeErr = fabs(Erreur[0]) + fabs(Erreur[1]) + fabs(Erreur[2]) + fabs(Erreur[3]) + fabs(Erreur[4]);
	//  Boucle tant que l'erreur est assez conséquente
	while ( SommeErr > DELTA_NORMALISATION + PRECISION_NORMALISATION*pJeu->NbCarte)
	{
		Boucle++;
		if ( Boucle > LIMITE_BOUCLE )
        {
            printf("ERREUR : NormaliseProba ne converge pas après %d boucles\n", Boucle);
            break;
        }
#if DEBUG > 0
        if ( Boucle > LIMITE_BOUCLE - 20 )
        {
            OutDebug("Joueur %d: Normalise Proba NbCarte=%d Bcl %d, Pos %d, SommeErr = %.3e, Err[x] = %.3e %.3e %.3e %.3e %.3e\n",
                pJeu->PositionJoueur, pJeu->NbCarte, Boucle, pJeu->PositionJoueur, SommeErr, Erreur[0], Erreur[1], Erreur[2], Erreur[3], Erreur[4]);
        }
#endif // DEBUG
#if DEBUG_NORMALISE_PROBA > 0
		OutDebug("Joueur %d: Normalise Proba NbCarte=%d Bcl %d, Pos %d, SommeErr = %.3e, Err[x] = %.3e %.3e %.3e %.3e %.3e\n",
			pJeu->PositionJoueur, pJeu->NbCarte, Boucle, pJeu->PositionJoueur, SommeErr, Erreur[0], Erreur[1], Erreur[2], Erreur[3], Erreur[4]);
#endif // DEBUG_NORMALISE_PROBA
		MaxErr = 0.0;
		ColonneErr = 0;
		for ( i = 0; i <= 4; i++)
		{	//	Recherche colonne (Joueur) avec plus grande erreur
			if ( fabs(Erreur[i]) > MaxErr )
			{
				MaxErr = fabs(Erreur[i]);
				ColonneErr = i;
			}
		}
		//	ColonneErr représente le joueur avec l'erreur la plus grande.
		//	L'idée est d'arriver à une erreur à 0 sur ce joueur pour converger
		SConfiance = 0.0;
		MaxValErr = 0.0;
		if ( Boucle >= 990 )
            i = 0;
		for ( i = 0; i < 78; i++)
		{
			if ( pJeu->ProbCarte[ColonneErr][i] > 0 )
			{
				//	Attention, impossible de descendre en dessous de 0 ou de monter au dessus de 1
				//	limite donc Confiance via TmpConf
				//  Par défaut : TmpConf d'autant plus grand que la confiance est faible
				//  On bougera moins les cartes dont on est "sûr"
				TmpConf = 1.0 - pJeu->ConfianceProb[ColonneErr][i];
				if ( Erreur[ColonneErr] < 0 )	//	Doit remonter les probas
				{
					MaxValErr += 1.0 - pJeu->ProbCarte[ColonneErr][i];        //  Maximum atteignable pour cette carte
					if ( pJeu->ConfianceProb[ColonneErr][i] < pJeu->ProbCarte[ColonneErr][i] )
                    {
                        //  Si confiance plus faible que proba, utilise plutôt la probabilité comme borne
                        //  En effet ne pourra pas bouger plus que cela...
						TmpConf = 1.0 - pJeu->ProbCarte[ColonneErr][i];
                    }
				}
				else                            //  Doit baisser les probas
				{
					MaxValErr += pJeu->ProbCarte[ColonneErr][i];            //  Maximum atteignable pour cette carte
					if ( TmpConf > pJeu->ProbCarte[ColonneErr][i] )
						TmpConf = pJeu->ProbCarte[ColonneErr][i];           //  Si trop de confiance, utilise la proba comme borne
				}
				SConfiance += TmpConf;
			}
		}
        //	Répartit l'erreur
        //	Pour cela utilise le terme "confiance" : Règle = Change d'autant plus que confiance faible
		Diviseur = SConfiance;
		NoConfiance = 0;
		if ( SConfiance < MaxErr )	//	Pas assez pour corriger, erreur dans Confiance
		{
			NoConfiance = 1;
			printf("Erreur dans Confiance (Trop confiant) pour nomalisation, Calculé=%.2f, Erreur à rattaper=%.2f\n", SConfiance, MaxErr);
			Diviseur = MaxValErr;
		}
		for ( i = 0; i < 78; i++ )
		{
			if ( pJeu->ProbCarte[ColonneErr][i] > 0 )
			{
				TmpConf = 1.0 - pJeu->ConfianceProb[ColonneErr][i];
				if ( Erreur[ColonneErr] < 0 )	//	Doit remonter les probas
				{
					if ( NoConfiance || pJeu->ConfianceProb[ColonneErr][i] < pJeu->ProbCarte[ColonneErr][i] )
						TmpConf = 1.0 - pJeu->ProbCarte[ColonneErr][i];
				}
				else
				{
					if ( NoConfiance || TmpConf > pJeu->ProbCarte[ColonneErr][i] )
						TmpConf = pJeu->ProbCarte[ColonneErr][i];
				}
				Delta = -1.0* Erreur[ColonneErr] / Diviseur * TmpConf;
				assert(!isnan(Delta));
				if ( Delta + pJeu->ProbCarte[ColonneErr][i] < 0.0 )	//	Limite valeur basse --> Doit passer a valeur à 0
				{
					for ( int j = 0; j <= 4; j++)
					{
						if ( j == ColonneErr ) continue;
						pJeu->ProbCarte[j][i] *= 1.0 / (1.0 - pJeu->ProbCarte[ColonneErr][i]);
						assert(!isnan(pJeu->ProbCarte[j][i]));
						assert(pJeu->ProbCarte[j][i] < 1.0000001 );
					}
					pJeu->ProbCarte[ColonneErr][i] = 0.0;
				}
				else if ( Delta > 1.0 - pJeu->ProbCarte[ColonneErr][i] )	//	Limite valeur haute --> Doit passer à 1, donc les autres à 0
				{
					for ( int j = 0; j <= 4; j++)
					{
						if ( j == ColonneErr ) continue;
						pJeu->ProbCarte[j][i] = 0.0;
					}
					pJeu->ProbCarte[ColonneErr][i] = 1.0;
				}
				else
				{
					//	Change les autres
					if ( pJeu->ProbCarte[ColonneErr][i] < 0.99999999 )
					{
						for ( int j = 0; j <= 4; j++)
						{
							if ( j == ColonneErr ) continue;
							pJeu->ProbCarte[j][i] *= (1.0 - pJeu->ProbCarte[ColonneErr][i] - Delta) / (1.0 - pJeu->ProbCarte[ColonneErr][i]);
							assert(!isnan(pJeu->ProbCarte[j][i]));
                            assert(pJeu->ProbCarte[j][i] < 1.0000001 );
						}
						pJeu->ProbCarte[ColonneErr][i] += Delta;
						assert(!isnan(pJeu->ProbCarte[ColonneErr][i]));
						assert(pJeu->ProbCarte[ColonneErr][i] < 1.0000001 );
					}
				}
			}
		}
		CalcDeviation(CurrentGame, pJeu, Erreur);		//	Recalcule les erreurs sur chaque joueur
		SommeErr = fabs(Erreur[0]) + fabs(Erreur[1]) + fabs(Erreur[2]) + fabs(Erreur[3]) + fabs(Erreur[4]);
	}
	CheckProbaJeu(CurrentGame, pJeu);
}


//	Après modification la somme des probas n'est plus égale à NbCarte... Il faut donc corriger
//	L'erreur va être positive, car on a baissé une proba...
//  Normalise dans ce cas pour atteindre NbCarteAttendues
//  Le fait pour le Joueur passé en paramètre
//  Le fait vu de la struct _Jeu pJeu passée en paramètre

void RenormaliseMontant(struct _Jeu *pJeu, int Joueur, int NbCarteAttendues)
{
double Somme = 0.0;
double SConfiance = 0.0;
double Erreur;
double Delta;
double TmpConf;
double MaxValErr = 0.0;
int NoConfiance = 0;
double Diviseur;
int i, j;
int jr;

    if ( Joueur == CHIEN ) NbCarteAttendues = 6;
	if ( NbCarteAttendues == 0 ) return;
	for ( int i = 0; i < 78; i++)
	{
		if ( pJeu->ProbCarte[Joueur][i] > 0 )
		{
			Somme += pJeu->ProbCarte[Joueur][i];
			//	Attention, impossible de monter plus que 1.0 - ProbCarte, limite donc Confiance
			TmpConf = 1.0 - pJeu->ConfianceProb[Joueur][i];
			MaxValErr += 1.0 - pJeu->ProbCarte[Joueur][i];
			if ( pJeu->ConfianceProb[Joueur][i] < pJeu->ProbCarte[Joueur][i] )
				TmpConf = 1.0 - pJeu->ProbCarte[Joueur][i];
			SConfiance += TmpConf;
		}
	}
	//	L'erreur est de NbCarte - Somme. Il faut la répartir...
	//	Pour cela utilise le terme "confiance" : Règle = Change d'autant plus que confiance faible
	Erreur = NbCarteAttendues - Somme;
	Diviseur = SConfiance;
	if ( SConfiance < fabs(Erreur) )	//	Pas assez pour corriger, erreur dans Confiance
	{
		NoConfiance = 1;
		Diviseur = MaxValErr;
		printf("RenormaliseMontant, Confiance surestimée, SConfiance=%.2f, Erreur à rattraper %.2f\n", SConfiance, Erreur);
	}
	if (Erreur < 1.0e-7) return;
	if ( Erreur < 0 )
    {
        assert(Erreur > -DELTA_NORMALISATION);
        return;
    }
	for ( i = 0; i < 78; i++ )
	{
		if ( pJeu->ProbCarte[Joueur][i] > 0 )
		{
			TmpConf = 1.0 - pJeu->ConfianceProb[Joueur][i];     //  Delta max pour cette carte
			if ( NoConfiance || pJeu->ConfianceProb[Joueur][i] < pJeu->ProbCarte[Joueur][i] )
				TmpConf = 1.0 - pJeu->ProbCarte[Joueur][i];     //  Pas sûr, limite à 1 - proba actuelle pour ne pas dépasser 1
			Delta = Erreur / Diviseur * TmpConf;
			assert(!isnan(Delta));
			assert(Delta >= 0.0);                             //  Delta DOIT être positif !
			if ( Delta > 1.0 - pJeu->ProbCarte[Joueur][i] )	 //	Limite valeur haute, limite proba de la carte à 1, donc 0 pour les autres joueurs
			{
			    for ( jr = 0; jr <= 4; jr++ )
                {
                    pJeu->ProbCarte[jr][i] = 0.0;
                }
				pJeu->ProbCarte[Joueur][i] = 1.0;
			}
			else
			{
				//	Diminue les autres pour que la somme fasse toujours 1.0
				//  Le fait en multipliant chaque proba par (1.0 - pJeu->ProbCarte[Joueur][i] - Delta) / (1.0 - pJeu->ProbCarte[Joueur][i])
				//  Cela donne la bonne somme au final
				if ( pJeu->ProbCarte[Joueur][i] < 0.99999999 )
				{
                    for ( jr = 0; jr <= 4; jr++ )
                    {
                        if ( jr == Joueur ) continue;
                        pJeu->ProbCarte[jr][i] *= (1.0 - pJeu->ProbCarte[Joueur][i] - Delta) / (1.0 - pJeu->ProbCarte[Joueur][i]);
                    }
					pJeu->ProbCarte[Joueur][i] += Delta;
				}
			}
#if DEBUG > 0
			for ( j = 0; j < 5; j++ )
            {
                assert(pJeu->ProbCarte[j][i] < 1.0000001);
                assert(pJeu->ProbCarte[j][i] >= 0);
            }
#endif // DEBUG
		}
	}
}


//	Après modification la somme des probas n'est plus égale à NbCarte... Il faut donc corriger
//	L'erreur va être négative, car on a monté une proba...
//  Normalise dans ce cas pour atteindre NbCarteAttendues
//  Le fait pour le Joueur passé en paramètre
//  Le fait vu de la struct _Jeu pJeu passée en paramètre

void RenormaliseDescendant(struct _Jeu *pJeu, int Joueur, int NbCarteAttendues)
{
double Somme= 0.0;
double SConfiance = 0.0;
double Erreur;
double Delta;
double TmpConf;
double MaxValErr = 0.0;
int NoConfiance = 0;
double Diviseur;
int i, j, jr;

    if ( Joueur == CHIEN ) NbCarteAttendues = 6;
	if ( NbCarteAttendues == 0 ) return;
	for ( int i = 0; i < 78; i++)
	{
		if ( pJeu->ProbCarte[Joueur][i] > 0 )
		{
			Somme += fabs(pJeu->ProbCarte[Joueur][i]);
			//	Attention, impossible de descendre en dessous de 0, limite donc Confiance
			TmpConf = 1.0 - pJeu->ConfianceProb[Joueur][i];
			MaxValErr += pJeu->ProbCarte[Joueur][i];
			if ( TmpConf > pJeu->ProbCarte[Joueur][i] )
				TmpConf = pJeu->ProbCarte[Joueur][i];
			SConfiance += TmpConf;
		}
	}
	//	L'erreur est de NbCarte - Somme. Il faut la répartir...
	//	Pour cela utilise le terme "confiance" : Règle = Change d'autant plus que confiance faible
	Erreur = NbCarteAttendues - Somme;
	Diviseur = SConfiance;
	if ( SConfiance < fabs(Erreur) )	//	Pas assez pour corriger, erreur dans Confiance
	{
		NoConfiance = 1;
		Diviseur = MaxValErr;
		printf("RenormaliseDescendant, Confiance surestimée, SConfiance=%.2f, Erreur à rattraper %.2f\n", SConfiance, Erreur);
	}
	if (fabs(Erreur) < 1.0e-7) return;
	if ( Erreur > 0 )
    {
        assert(Erreur < DELTA_NORMALISATION);
        return;
    }
	for ( i = 0; i < 78; i++ )
	{
		if ( pJeu->ProbCarte[Joueur][i] > 0 )
		{
			TmpConf = 1.0 - pJeu->ConfianceProb[Joueur][i];
			if ( NoConfiance || TmpConf > pJeu->ProbCarte[Joueur][i] )
				TmpConf = pJeu->ProbCarte[Joueur][i];
			Delta = Erreur / Diviseur * TmpConf;
			assert(!isnan(Delta));
			assert(Delta <= 0.0);                             //  Delta DOIT être négatif !
			if ( Delta + pJeu->ProbCarte[Joueur][i] < 0.0 )	//	Limite valeur basse, passe à 0 et monte les autres
			{
			    for ( jr = 0; jr <= 4; jr++ )
                {
                    if ( jr == Joueur ) continue;
                    pJeu->ProbCarte[jr][i] *= 1.0 / (1.0 - pJeu->ProbCarte[Joueur][i]);
                }
				pJeu->ProbCarte[Joueur][i] = 0.0;
			}
			else
			{
				//	Augmente les autres (Delta est négatif)
				if ( pJeu->ProbCarte[Joueur][i] < 0.99999999 )
				{
                    for ( jr = 0; jr <= 4; jr++ )
                    {
                        if ( jr == Joueur ) continue;
                        pJeu->ProbCarte[jr][i] *= (1.0 - pJeu->ProbCarte[Joueur][i] - Delta) / (1.0 - pJeu->ProbCarte[Joueur][i]);
                    }
					pJeu->ProbCarte[Joueur][i] += Delta;
				}
			}
#if DEBUG > 0
			for ( j = 0; j < 5; j++ )
            {
                assert(pJeu->ProbCarte[j][i] < 1.0000001);
                assert(pJeu->ProbCarte[j][i] >= 0);
            }
#endif // DEBUG
		}
	}
}

//  Modifie la proba des cartes du preneur en tenant compte du contrat et des statistiques de distributions
//  Le fait pour le joueur Position

//  Probabilités des cartes suivant le contrat
//  Probabilités établies en distribuant 5 000 000 de parties avec le programme
//  Les probas des autres cartes seront "normalisées"

static const double ProbaExcuse[NB_TYPE_PARTIE] = {0.0, 0.41, 0.56, 0.75, 0.90};
static const double ProbaPetit[NB_TYPE_PARTIE] = {0.0, 0.37, 0.54, 0.77, 0.91};
static const double Proba21[NB_TYPE_PARTIE] = {0.0, 0.55, 0.73, 0.90, 0.97};
static const double ProbaAtoutNonMaj[NB_TYPE_PARTIE] = {0.0, 0.38, 0.42, 0.53, 0.55};
static const double ProbaAtouMaj[NB_TYPE_PARTIE] = {0.0, 0.39, 0.44, 0.55, 0.60};
static const double ProbaRoi[NB_TYPE_PARTIE] = {0.0, 0.22, 0.28, 0.30, 0.32};

//  Nombre d'atouts moyens Preneur par contrat en fonction du nombre de bouts
const  double NbAtoutMoyen[4][4] = {
	{10.0,	7.5,	6.5,	5.5},		//	Prise
	{11.0,	8.0,	7.0,	6.0},		//	Garde
	{12.0,	9.0,	7.5,	6.5},		//	Garde sans
	{13.0,	9.0,	8.0,	7.0}
};

static void InitProbaPreneur(TarotGame CurrentGame, int Position)
{
int Contrat = CurrentGame->TypePartie;
int Preneur = CurrentGame->JoueurPreneur;
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[Position];
double Facteur;
int i;

    //  Change Proba Excuse
    if ( pJeu->ConfianceProb[Preneur][0] == 0.0 )
    {
        //  Pas sûr pour excuse, change proba
        Facteur = ProbaExcuse[Contrat] / pJeu->ProbCarte[Preneur][0];
        if ( Facteur > 1.0 )
            MonteProba(CurrentGame, Position, Preneur, 0, Facteur, 0);
    }
    //  Change Proba Petit
    if ( pJeu->ConfianceProb[Preneur][1] == 0.0 )
    {
        //  Pas sûr pour Petit, change proba
        Facteur = ProbaPetit[Contrat] / pJeu->ProbCarte[Preneur][1];
        if ( Facteur > 1.0 )
            MonteProba(CurrentGame, Position, Preneur, 1, Facteur, 0);
    }
    //  Change Proba 21
    if ( pJeu->ConfianceProb[Preneur][21] == 0.0 )
    {
        //  Pas sûr pour 21, change proba
        Facteur = Proba21[Contrat] / pJeu->ProbCarte[Preneur][21];
        if ( Facteur > 1.0 )
            MonteProba(CurrentGame, Position, Preneur, 21, Facteur, 0);
    }
    //  Change proba atouts non majeurs
    for ( i = 2; i < 16; i++ )
    {
        //  Pas sûr pour cette carte, change proba
        Facteur = ProbaAtoutNonMaj[Contrat] / pJeu->ProbCarte[Preneur][i];
        if ( Facteur > 1.0 )
            MonteProba(CurrentGame, Position, Preneur, i, Facteur, 0);
    }
    //  Change proba atouts majeurs
    for ( i = 16; i <= 20; i++ )
    {
        //  Pas sûr pour cette carte, change proba
        Facteur = ProbaAtouMaj[Contrat] / pJeu->ProbCarte[Preneur][i];
        if ( Facteur > 1.0 )
            MonteProba(CurrentGame, Position, Preneur, i, Facteur, 0);
    }
    //  Et enfin les rois...
    for ( i = 35; i < 78; i += 14)
    {
        //  Pas sûr pour cette carte, change proba
        Facteur = ProbaRoi[Contrat] / pJeu->ProbCarte[Preneur][i];
        if ( Facteur > 1.0 )
            MonteProba(CurrentGame, Position, Preneur, i, Facteur, 0);
    }
    NormaliseProba(CurrentGame, pJeu);
}


//	Retourne la Proba que le joueur possède exactement N cartes de la couleur supérieures à l'Index et inférieure à la borne

double ProbaExactementN(struct _Jeu *pJeu, int Joueur, int N, int Index, int Borne)
{
double p0 = 0.0;
double p1 = 1.0;
int i;

	if ( N == 0 )	//	Arrêt recursivité... Calcule la proba de ne plus avoir de carte de la couleur
	{
		p0 = 1.0;
		for ( i = Index + 1; i < Borne; i++ )
		{
			p0 *= 1.0 - pJeu->TmpProbCarte[Joueur][i];
		}
		return(p0);
	}
	if ( Borne - Index - 1 < N ) return(0.0);		//	Pour gagner du temps... on ne peut plus en avoir N entre les deux
	for ( i = Index + 1; i < Borne; i++ )
	{
		p0 += p1* pJeu->TmpProbCarte[Joueur][i] * ProbaExactementN(pJeu, Joueur, N-1, i, Borne);
		p1 *= 1 - pJeu->TmpProbCarte[Joueur][i];
	}
	return(p0);
}

//	Retourne la probabilité que le joueur ait de l'atout

double GetProbAtout(struct _Jeu *pJeu, int joueur)
{
double val = 1.0;

	for ( int i = 1; i < 22; i++ )
	{
		val *= 1.0 - pJeu->TmpProbCarte[joueur][i];
	}
	return(1.0 - val);
}


//	Retourne la proba qu'au moins un des défenseurs soit sans ATOUT

double ProbUnDefenseurSansAtout(struct _Jeu *pJeu)
{
double val = 1.0;
int i;

	for ( i = 0; i < 4; i++ )
	{
		if ( i == pJeu->PositionPreneur ) continue;
		val *= GetProbAtout(pJeu, i);
	}
	return(1.0 - val);
}

//	Recalcule les probas de coupe en fonction des cartes tombées sur la table

void CalcTmpProbCoupe(TarotGame CurrentGame, struct _Jeu *pJeu, int IndexTable)
{
double v;
int c;
int posPreneur = (CurrentGame->JoueurPreneur - CurrentGame->JoueurEntame) & 3;

	for ( c = TREFLE; c < NB_COULEUR; c++)	//	Pour chaque couleur
	{
	    //  Calcule tout d'abord avec les probas de chaque carte
		v = ProbCoupeJoueur(pJeu, pJeu->PositionPreneur, c);
		//  Si proba coupe > ancienne proba, met à jour.
		if ( v > pJeu->TmpProbCoupe[c] )
            pJeu->TmpProbCoupe[c] = v;
        if ( v == 1.0 )
        {
            pJeu->GuessProbCoupe[c] = 1.0;      //  Change GuessProbCoupe si vérifié.
        }
        if ( posPreneur > IndexTable )
            pJeu->TmpProbCoupe[c] = fmax(pJeu->TmpProbCoupe[c], pJeu->GuessProbCoupe[c]);
		assert(!isnan(pJeu->TmpProbCoupe[c]));
	}
}

#if DEBUG > 0
#define DEBUG_REGLE_TMP 1
#else
#define DEBUG_REGLE_TMP 0
#endif  // DEBUG
//	Change la probabilité d'une carte. La proba initiale est multipliée par 1 - Probregle.
//	S'emploie quand on est sur a x% que le joueur ne possède pas la carte

void BaisseTmpProb(struct _Jeu *pJeu, int posJoueur, int IndexCarte, double ProbRegle, double NumRegle)
{
double Initial = pJeu->TmpProbCarte[posJoueur][IndexCarte];
double Facteur;
double Somme;

	if ( Initial <= 0.0 ) return;
#if DEBUG > 0
	assert(IndexCarte >= 0);
	assert(IndexCarte < 78);
	if ( isnan(Initial) ) assert(0);
#endif // DEBUG
	if ( fabs(Initial - 1.0) > 1e-8 )
		pJeu->TmpProbCarte[posJoueur][IndexCarte] *= (1.0 - ProbRegle);
    else
    {   //  On de doit pas changer une proba sûre !!!
#if DEBUG
        //assert(0);
#endif // DEBUG
        return;
    }
	assert(pJeu->TmpProbCarte[posJoueur][IndexCarte] >= 0);
	assert(pJeu->TmpProbCarte[posJoueur][IndexCarte] <= 1.0000000001);
	Facteur = (1.0 - pJeu->TmpProbCarte[posJoueur][IndexCarte])/(1.0 - Initial);
	if ( Facteur < 1e-12 ) Facteur = 1e-12;
	pJeu->TmpProbCarte[posJoueur^1][IndexCarte] *= Facteur;
	pJeu->TmpProbCarte[posJoueur^2][IndexCarte] *= Facteur;
	pJeu->TmpProbCarte[posJoueur^3][IndexCarte] *= Facteur;
	pJeu->TmpProbCarte[4][IndexCarte] *= Facteur;
#if DEBUG_REGLE_TMP > 0
    OutDebug("Règle Tmp %.1f, BaisseTmpProb(%d, %d) Initial = %.3f, New %.3f, Facteur pour autres %.3f\n",
             NumRegle, posJoueur, IndexCarte, Initial, pJeu->TmpProbCarte[posJoueur][IndexCarte], Facteur);
    OutDebug("  Nouvelles valeurs TmpProbCarte[%d][%d] = %.3f TmpProbCarte[%d][%d] = %.3f TmpProbCarte[%d][%d] = %.3f TmpProbCarte[4][%d] = %.3f\n"
             , posJoueur^1, IndexCarte, pJeu->TmpProbCarte[posJoueur^1][IndexCarte]
             , posJoueur^2, IndexCarte, pJeu->TmpProbCarte[posJoueur^2][IndexCarte]
             , posJoueur^3, IndexCarte, pJeu->TmpProbCarte[posJoueur^3][IndexCarte]
             , IndexCarte, pJeu->TmpProbCarte[4][IndexCarte]);
#endif // DEBUG_REGLE_TMP
#if DEBUG > 0
	assert(pJeu->TmpProbCarte[posJoueur^1][IndexCarte] >= 0);
	assert(pJeu->TmpProbCarte[posJoueur^1][IndexCarte] <= 1.0000001);
	if ( pJeu->TmpProbCarte[posJoueur^1][IndexCarte] > 1.0 ) pJeu->TmpProbCarte[posJoueur^1][IndexCarte] = 1.0;
	assert(pJeu->TmpProbCarte[posJoueur^2][IndexCarte] >= 0);
	assert(pJeu->TmpProbCarte[posJoueur^2][IndexCarte] <= 1.0000001);
	if ( pJeu->TmpProbCarte[posJoueur^2][IndexCarte] > 1.0 ) pJeu->TmpProbCarte[posJoueur^2][IndexCarte] = 1.0;
	assert(pJeu->TmpProbCarte[posJoueur^3][IndexCarte] >= 0);
	assert(pJeu->TmpProbCarte[posJoueur^3][IndexCarte] <= 1.0000001);
	if ( pJeu->TmpProbCarte[posJoueur^3][IndexCarte] > 1.0 ) pJeu->TmpProbCarte[posJoueur^3][IndexCarte] = 1.0;
	Somme = pJeu->TmpProbCarte[0][IndexCarte] + pJeu->TmpProbCarte[1][IndexCarte] + pJeu->TmpProbCarte[2][IndexCarte]
			+ pJeu->TmpProbCarte[3][IndexCarte] + pJeu->TmpProbCarte[4][IndexCarte];
	assert(fabs(Somme - 1.0) < 1e-5 );
#endif // DEBUG
}


//	Monte la proba de la carte passée en paramètre
//	Divise la proba des autres joueurs par mul, celle du joueur passé en paramètre se trouve donc augmentée
//

void MonteTmpProb(struct _Jeu *pJeu, int posJoueur, int IndexCarte, double mul, double NumRegle)
{
double Initial = pJeu->TmpProbCarte[posJoueur][IndexCarte];

	if ( Initial <= 1e-7 )
    {   //  On ne change pas une proba sûre !!!
#if DEBUG
        //assert(0);
#endif // DEBUG
        return;
    }
	assert(mul > 1.0 );
	pJeu->TmpProbCarte[posJoueur^1][IndexCarte] /= mul;
	pJeu->TmpProbCarte[posJoueur^2][IndexCarte] /= mul;
	pJeu->TmpProbCarte[posJoueur^3][IndexCarte] /= mul;
	pJeu->TmpProbCarte[4][IndexCarte] /= mul;
	pJeu->TmpProbCarte[posJoueur][IndexCarte] = 1.0 - pJeu->TmpProbCarte[posJoueur^1][IndexCarte]
		- pJeu->TmpProbCarte[posJoueur^2][IndexCarte] - pJeu->TmpProbCarte[posJoueur^3][IndexCarte]
		- pJeu->TmpProbCarte[4][IndexCarte];
#if DEBUG_REGLE_TMP > 0
    OutDebug("Règle Tmp %.1f, MonteTmpProb(%d, %d) Initial = %.3f, New %.3f, Facteur pour autres %.3f\n",
             NumRegle, posJoueur, IndexCarte, Initial, pJeu->TmpProbCarte[posJoueur][IndexCarte], mul);
    OutDebug("  Nouvelles valeurs TmpProbCarte[%d][%d] = %.3f TmpProbCarte[%d][%d] = %.3f TmpProbCarte[%d][%d] = %.3f TmpProbCarte[4][%d] = %.3f\n"
             , posJoueur^1, IndexCarte, pJeu->TmpProbCarte[posJoueur^1][IndexCarte]
             , posJoueur^2, IndexCarte, pJeu->TmpProbCarte[posJoueur^2][IndexCarte]
             , posJoueur^3, IndexCarte, pJeu->TmpProbCarte[posJoueur^3][IndexCarte]
             , IndexCarte, pJeu->TmpProbCarte[4][IndexCarte]);
#endif // DEBUG_REGLE_TMP
}


//	Normalise les probas temporaires
//	D'abord assure que Somme(Proba(Joueur)) = Nombre de cartes
//  indexJoueur donne la position sur la table (donc nombre de cartes - 1)
//	Ensuite si une proba est a 1, met les autres a 0

void NormaliseTmpProba(TarotGame CurrentGame, struct _Jeu *pJeu, int indexJoueur)
{
int i, j, nb;
double val;
double ligne;
double ligm;
int calcul = 0;
double mul;
double sq = 0.0;
int NbC;
int iPos;

	for ( nb = 0; nb < 10; nb++)
	{
		for ( i = 0; i < 4; i++)		//	Joueur
		{
			iPos = (CurrentGame->JoueurEntame+i) & 3;
			val = 0.0;
			NbC = pJeu->NbCarte;
			if ( i < indexJoueur ) NbC--;                   //  Les joueurs avant ont une carte de moins
			for ( j = 0; j < 78; j++)
			{
				if ( isnan(pJeu->TmpProbCarte[iPos][j]) ) assert(0);
				val += pJeu->TmpProbCarte[iPos][j];         //  val va donner le nombre de cartes du joueur iPos
			}
			if ( fabs((val / NbC) - 1.0) > 0.02 )	//	Déviation importante ?
			{
				calcul++;
				for ( j = 0; j < 78; j++)
				{
					assert(pJeu->TmpProbCarte[0][j] >= 0 && pJeu->TmpProbCarte[0][j] < 1.000001);
					assert(pJeu->TmpProbCarte[1][j] >= 0 && pJeu->TmpProbCarte[1][j] < 1.000001);
					assert(pJeu->TmpProbCarte[2][j] >= 0 && pJeu->TmpProbCarte[2][j] < 1.000001);
					assert(pJeu->TmpProbCarte[3][j] >= 0 && pJeu->TmpProbCarte[3][j] < 1.000001);
					assert(pJeu->TmpProbCarte[4][j] >= 0 && pJeu->TmpProbCarte[4][j] < 1.000001);
					if ( pJeu->TmpProbCarte[iPos][j] <= 1e-8 ) continue;
					if ( fabs(pJeu->TmpProbCarte[iPos][j] - 1.0) < 1e-8 ) continue;
					mul = NbC / val;				//	Rapport entre calculé (val) et réel (Nbcarte)
					pJeu->TmpProbCarte[iPos][j] *= mul;				//	Normalise...
					sq = mul;
					while ( pJeu->TmpProbCarte[iPos][j] >= 1.0 )	//	Si trop grand, il faut faire quelque chose
					{
						sq = sqrt(sq) + 0.01;
						pJeu->TmpProbCarte[iPos][j] /= sq;
					}
					ligne = pJeu->TmpProbCarte[0][j] + pJeu->TmpProbCarte[1][j] + pJeu->TmpProbCarte[2][j] + pJeu->TmpProbCarte[3][j] + pJeu->TmpProbCarte[4][j];
					if ( fabs(ligne - pJeu->TmpProbCarte[iPos][j]) < 1e-5 )
					{
						pJeu->TmpProbCarte[iPos][j] = 1.0;
						pJeu->TmpProbCarte[iPos^1][j] = 0;
						pJeu->TmpProbCarte[iPos^2][j] = 0;
						pJeu->TmpProbCarte[iPos^3][j] = 0;
						pJeu->TmpProbCarte[4][j] = 0;
					}
					else
					{
						ligm= (1.0 - pJeu->TmpProbCarte[iPos][j]) / (ligne - pJeu->TmpProbCarte[iPos][j]);
						if ( isnan(ligm) ) assert(0);
						assert(ligm > 0);
						pJeu->TmpProbCarte[iPos^1][j] *= ligm;
						pJeu->TmpProbCarte[iPos^2][j] *= ligm;
						pJeu->TmpProbCarte[iPos^3][j] *= ligm;
						pJeu->TmpProbCarte[4][j] *= ligm;
					}
					if ( isnan(pJeu->TmpProbCarte[0][j]) ) assert(0);
					assert(pJeu->TmpProbCarte[0][j] >= 0);
					if ( isnan(pJeu->TmpProbCarte[1][j]) ) assert(0);
					assert(pJeu->TmpProbCarte[1][j] >= 0);
					if ( isnan(pJeu->TmpProbCarte[2][j]) ) assert(0);
					assert(pJeu->TmpProbCarte[2][j] >= 0);
					if ( isnan(pJeu->TmpProbCarte[3][j]) ) assert(0);
					assert(pJeu->TmpProbCarte[3][j] >= 0);
					if ( isnan(pJeu->TmpProbCarte[4][j]) ) assert(0);
					assert(pJeu->TmpProbCarte[4][j] >= 0);
				}
			}
		}
	}
	assert(calcul < 40);
	//  Si une proba est presque à 1, met également la probabilité temporaire à 0
	for ( i = 0; i < 4; i++)
	{
		for ( j= 0; j < 78; j++)
		{
			if ( fabs(pJeu->ProbCarte[i][j] - 1.0) < 1e-9 )
			{
				pJeu->TmpProbCarte[i][j] = 1.0;
				pJeu->TmpProbCarte[i^1][j] = 0.0;
				pJeu->TmpProbCarte[i^2][j] = 0.0;
				pJeu->TmpProbCarte[i^3][j] = 0.0;
				pJeu->TmpProbCarte[4][j] = 0.0;
			}
		}
	}
}

