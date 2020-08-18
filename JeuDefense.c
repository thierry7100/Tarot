#include <stdlib.h>
#include <gtk/gtk.h>
#include "Tarot_Ui_Objects.h"
#include <assert.h>
#include <math.h>

#if DEBUG > 0
#define DEBUG_ENTAME_DEFENSE 1
#else
#define DEBUG_ENTAME_DEFENSE 0
#endif  // DEBUG

//  Calcul de la force des longues

static double ForceLongueur[3] = {0.2, 0.5, 1.0};
static double val_longueur[3][15] = {
	{0.0, 0.5, 3.5, 4.0, 5.0, 3.0, 1.5, 1.2, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5, 0.5},	//	Force = 0.2
	{0.0, 0.5, 2.0, 3.5, 5.0, 5.0, 4.0, 2.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5, 0.5},	//	Force = 0.5
	{0.0, 0.5, 1.0, 2.0, 5.5, 9.5,11.5,12.0,11.5, 8.0, 7.0, 7.0, 7.0, 6.5, 5.5}	    //	Force = 1.0
};

//	Nombre de coupe moyen du preneur, fonction du contrat et du nombre de bouts
static const  double NbCoupeMoyen[4][4] = {
	{2.0, 1.7, 1.5, 1.5},
	{2.3, 2.0, 1.5, 1.5},
	{2.0, 1.5, 1.0, 1.0},
	{2.0, 1.5, 1.0, 1.0}
};

enum _Type_Force_Jeu {
    JEU_FAIBLE,
    JEU_MOYEN_FAIBLE,
    JEU_MOYEN_FORT,
    JEU_FORT
};

//	Calcule les proba de coupe du preneur a priori  par couleurs
//	L'idée est de ne pas baser les probas de coupe sur le calcul direct
//	car les cartes ne sont pas évidemment écartées au hasard par le preneur, mais
//	avec l'idée de se faire une ou plusieurs coupes.

//  Correction de la proba de coupe preneur en fonction de la longueur de la couleur chez le joueur
//  Plus la couleur est longue plus la proba de coupe sera grande

static const double FirstCorrection[15] =
{	1.0,	1.0,	1.0,	0.95,	0.8,
	0.5,	0.4,	0.3,	0.2,	0.2,
	0.2,	0.2,	0.2,	0.2,	0.1
};

void InitProbaCoupes(TarotGame CurrentGame, struct _Jeu *pJeu)
{
int c, j;
int NbRoi = 0;
double val;
int nb;
int GuessBoutPreneur = 1;

	for ( c = TREFLE; c < NB_COULEUR; c++)
		pJeu->GuessProbCoupe[c] = 1.0;	//	Initialise tout a 1
    //	Commence par calculer le nombre de roi du chien
    //	On est sur qu'il n'y aura pas de coupe dans un roi au chien
	if ( CurrentGame->TypePartie <= GARDE )
	{
		for ( c = TREFLE; c < NB_COULEUR; c++)
		{
			if (CurrentGame->CarteAuChien[Endof[c]-1] )
			{
				pJeu->GuessProbCoupe[c] = 0.0;
				NbRoi++;
			}
		}
	}
    if ( CurrentGame->TypePartie >= GARDE ) GuessBoutPreneur = 2;
	if ( CurrentGame->CarteAuChien[0] ) GuessBoutPreneur++;
	if ( CurrentGame->CarteAuChien[1] ) GuessBoutPreneur++;
	if ( CurrentGame->CarteAuChien[21] ) GuessBoutPreneur++;
	if ( GuessBoutPreneur + pJeu->NbBout > 3 ) GuessBoutPreneur = 3 - pJeu->NbBout;
    //	Environ 1,5 coupe par jeu. La proba de coupe est donc de 1,5/4-NbRoi au chien
    //	La proba exacte dépend du contrat et du nombre de bouts.
    //	On applique ensuite une correction fonction du nombre de cartes dans la
    //	couleur du joueur. Plus sa couleur est longue plus la proba de coupe du
    //	preneur est importante.
	if ( NbRoi < 4 )
	{
		val = 1.0 - (NbCoupeMoyen[CurrentGame->TypePartie-1][GuessBoutPreneur] / (4.0 - NbRoi));
		for ( c = TREFLE; c < NB_COULEUR; c++)
		{
			if ( pJeu->GuessProbCoupe[c] != 0.0 )
			{
				nb = pJeu->NbCouleur[c];
				if ( HasCarte(pJeu, pJeu->PositionJoueur, Endof[c]-1) )	//	Possède le ROI ?
				{	//	Allonge artificiellement les couleurs si possède ROI ou DAME
				    //  Avec Roi Lg + 1, avec Roi et Dame Lg+2
					nb++;
					if ( HasCarte(pJeu, pJeu->PositionJoueur, Endof[c]-2) )	//	Possède le ROI et la DAME ?
						nb++;
				}
				if ( nb > 14 ) nb = 14;     //  Reste dans les valeurs possibles !
				pJeu->GuessProbCoupe[c] = 1.0 - val * FirstCorrection[nb];
			}
		}
	}
	for ( c = TREFLE; c < NB_COULEUR; c++)
	{
		pJeu->ProbCoupe[c] = pJeu->GuessProbCoupe[c];
		pJeu->ProbCouleur[c] = 1.0 - pJeu->ProbCoupe[c];
		pJeu->CalProbCoupe[c] = 1.0;
		for ( j = Startof[c]; j < Endof[c]; j++)
		{
			if ( pJeu->ProbCarte[CurrentGame->JoueurPreneur][j] < 1e-7 ) continue;
			pJeu->CalProbCoupe[c] *= (1.0 - pJeu->ProbCarte[CurrentGame->JoueurPreneur][j]);
		}
	}
	pJeu->NbCoupeInit = 0;		//	Nombre de coupes initiales du preneur
}


//	Calcule les probabilités de coupe des différentes couleurs par le Preneur
//	Utilise des heuristiques.
//  Maintient 3 valeurs par couleur
//  ProbCoupe qui est effectivement utilisée
//  GuessProbCoupe qui vient d'heuristiques comme en moyenne 1 à 2 coupes chez le preneur
//  CalProbCoupe qui calcule effectivement la probabilité de coupe d'une couleur
//  EN fonction de l'état du jeu utilise pour ProbCoupe CalProbCoupe ou GuessProbCoupe
//  Calcule également ProbCouleur qui donne la probabilité d'avoir une carte de cette couleur

void CalcProbCoupe(TarotGame CurrentGame, struct _Jeu *pJeu, int JoueurEntame)
{
int c, j;
int Nbc;
int single = 0;
int nblongue = 0;
int csingle = -1;

#if DEBUG > 0
    if ( pJeu->PositionJoueur == 1 )
        c = 0;
#endif // DEBUG
	if ( AvgLongueur(pJeu, CurrentGame->JoueurPreneur, ATOUT) < 0.001)
	{
	    //  Plus d'atout chez le preneur, passe la proba de coupe à 0
		for ( c = TREFLE; c < NB_COULEUR; c++ )
        {
            pJeu->ProbCoupe[c] = 0;
            pJeu->GuessProbCoupe[c] = 0;
            pJeu->CalProbCoupe[c] = 0;
            //  Pas d'atout : calcule directement les probas
            pJeu->ProbCouleur[c] = 1.0;
			for ( j = Startof[c]; j < Endof[c]; j++)
			{
				pJeu->ProbCouleur[c] *= 1.0 - pJeu->ProbCarte[CurrentGame->JoueurPreneur][j];
			}
			pJeu->ProbCouleur[c] = 1.0 - pJeu->ProbCouleur[c];
        }
		return;
	}
    //	Calcule les proba de coupe : CalProbCoupe
    //	Pour cela, accumule les probas de chaque carte dans la couleur

	for ( c = TREFLE; c < NB_COULEUR; c++)	//	Pour chaque couleur
	{
		assert(pJeu->GuessProbCoupe[c] >= 0.0 && pJeu->GuessProbCoupe[c] <= 1.00001);
		pJeu->CalProbCoupe[c] = 1.0;
		for ( j = Startof[c]; j < Endof[c]; j++)
		{
			if ( pJeu->ProbCarte[CurrentGame->JoueurPreneur][j] < 1e-7 ) continue;
			pJeu->CalProbCoupe[c] *= (1.0 - pJeu->ProbCarte[CurrentGame->JoueurPreneur][j]);
		}
		assert(!isnan(pJeu->CalProbCoupe[c]));
	}

    //	Essaie de détecter les "singlettes"
    //	Pour cela, si deux longues et une singlette, monte la proba de coupe de la singlette
	for ( c = TREFLE; c < NB_COULEUR; c++)
	{
		if ( pJeu->NbJoue[CurrentGame->JoueurPreneur][c] == 1 )
		{
			single++;               //  Compte les singlettes potentielles
			csingle = c;
		}
		if ( pJeu->NbJoue[CurrentGame->JoueurPreneur][c] >= 3 ) nblongue++;    //   Une longue potentielle de plus
	}
	if ( nblongue >= 2 )
	{
		if ( single )
		{
			for ( c = TREFLE; c < NB_COULEUR; c++ )
			{
			    //  Multiplie par 2 la proba de coupe pour toutes les couleurs où une seule carte a été jouée
			    //  Le fait seulement si la proba devinée est basse (< 0.25)
				if ( pJeu->NbJoue[CurrentGame->JoueurPreneur][c] == 1 && pJeu->GuessProbCoupe[c] <= 0.25 )
					pJeu->GuessProbCoupe[c] *= 2.0;
			}
		}
	}
    //	Egalement si ouvre d'une couleur, puis joue dans une autre peut-être une singlette
	for ( c = TREFLE; c < NB_COULEUR; c++ )
	{
		if ( pJeu->JoueurCouleur[c] == CurrentGame->JoueurPreneur )
		{
			if ( pJeu->PositionJoueur != CurrentGame->JoueurPreneur && pJeu->NbJoue[CurrentGame->JoueurPreneur][c] == 1
                && CurrentGame->JoueurEntame == CurrentGame->JoueurPreneur && Table[0].Couleur != c && pJeu->GuessProbCoupe[c] <= 0.3)
			{
				pJeu->GuessProbCoupe[c] *= 2.0;
				if ( pJeu->ValFlanc[c] < 0 ) pJeu->ValFlanc[c] = 0.5*pJeu->GuessProbCoupe[c];
			}
		}
	}
    //	Affecte les probas de coupe maintenant
    //	Algorithme général : Prend sup(GuessProbCoupe, CalProbCoupe) pour chaque couleur
	Nbc = 0;
	for ( c = TREFLE; c < NB_COULEUR ; c++)
	{
		if ( pJeu->PositionJoueur == CurrentGame->JoueurPreneur )
            pJeu->ProbCoupe[c] = pJeu->CalProbCoupe[c];         //  Sûr pour le preneur
        else
            pJeu->ProbCoupe[c] = fmax(pJeu->GuessProbCoupe[c], pJeu->CalProbCoupe[c]);
		if ( pJeu->CalProbCoupe[c] >= 0.999 ) Nbc++;
	}
    //	Autre cas de détection de singlette. Si singlette au roi, monte proba...
	if ( nblongue == 1 && single == 1 && Nbc > 0)
	{
		if ( CurrentGame->CarteJouee[Endof[csingle]-1] == CurrentGame->JoueurPreneur  )	//	A joué le ROI
		{
			assert(csingle >= TREFLE && csingle < NB_COULEUR);
			pJeu->GuessProbCoupe[csingle] = sqrt(pJeu->GuessProbCoupe[csingle]);
			assert(!isnan(pJeu->GuessProbCoupe[csingle]));
		}
	}
	if ( pJeu->PositionJoueur == CurrentGame->JoueurPreneur ) return;
//	Si 3 coupes, met la proba a 0 si il reste moins d'atout que de cartes
	if ( Nbc == 3 )		//	Au moins 3 coupes
	{
		for ( c = TREFLE; c < NB_COULEUR ; c++)
		{
			if ( pJeu->CalProbCoupe[c] < 0.999 )
            {
                pJeu->ProbCoupe[c] = pJeu->CalProbCoupe[c];         //  Passe à la probabilité calculée avec les cartes
                if ( MaxLongueur(pJeu, CurrentGame->JoueurPreneur, ATOUT) < pJeu->NbCarte)      //  Reste forcément d'autres cartes que des atouts ?
                {
                    pJeu->ProbCoupe[c] = 0;                         //  Oui, alors ne peut couper dans cette couleur
                }
            }
		}
	}
	Nbc = 0;
	for ( c = TREFLE; c < NB_COULEUR; c++)
	{
		if ( pJeu->CoupeCouleur[c] > 0 ) Nbc++;		//	Compte les coupes franches
	}
	pJeu->NbCoupeInit = Nbc;
    //	Si 2 coupes, compte seulement les probas via les cartes
	if ( Nbc == 2 )		//	Au moins 2 coupes
	{
		for ( c = TREFLE; c < NB_COULEUR ; c++)
		{
		    pJeu->GuessProbCoupe[c] = pJeu->CalProbCoupe[c];
			pJeu->ProbCoupe[c] = pJeu->CalProbCoupe[c];
		}
	}
    //	Si que des atouts, mise a jour ProbCoupe
	if ( MinLongueur(pJeu, CurrentGame->JoueurPreneur, ATOUT) == pJeu->NbCarte )
	{
		for ( c = TREFLE; c < NB_COULEUR ; c++)
		{
			pJeu->ProbCoupe[c] = 1.0;
		}
		for ( j = 22; j < 78; j++)
        {
            if ( CurrentGame->CarteJouee[j] < 0 )
                BaisseProba(CurrentGame, pJeu->PositionJoueur, CurrentGame->JoueurPreneur, j, 1.0);        //  Passe proba à 0 pour les autres cartes
        }
	}
	for ( c = TREFLE; c < NB_COULEUR; c++)
		pJeu->ProbCouleur[c] = 1.0 - pJeu->ProbCoupe[c];
}

//  Retourne la force du jeu (FAIBLE, MOYEN, FORT) du joueur dont la struct _Jeu est passée en paramètre

int GetForceJeuDefense(struct _Jeu *pJeu)
{
double force = pJeu->ForceMain[pJeu->PositionJoueur];       //  Force du jeu

    if ( force < ForceLongueur[0] ) return JEU_FAIBLE;
    if ( force > ForceLongueur[2] ) return JEU_FORT;
    if ( force <  ForceLongueur[1] ) return JEU_MOYEN_FAIBLE;
    return JEU_MOYEN_FORT;
}


//  Calcule la valeur d'une couleur comme ouverture pour la défense
//  Tient compte du type de jeu : faible.. fort
//  Bien sûr de la longueur de la couleur
double GetValeurCouleurOuverture(TarotGame CurrentGame, struct _Jeu *pJeu, int Couleur, double SansAtoutPreneur)
{
int ForceJeu = GetForceJeuDefense(pJeu);
double val;
double pourcentage = 0.0;
int i0;
int pos = (pJeu->PositionPreneur - pJeu->PositionJoueur) & 3;


    if ( ForceJeu == JEU_FAIBLE )
    {
        val = val_longueur[0][pJeu->NbCouleur[Couleur]];
    }
    else if ( ForceJeu == JEU_FORT )
    {
        val = val_longueur[2][pJeu->NbCouleur[Couleur]];
    }
    else
    {
        //  Position entre i-1 et i, calcule le pourcentage entre les deux
        pourcentage = (pJeu->ForceMain[pJeu->PositionJoueur] - ForceLongueur[ForceJeu-1]) / (ForceLongueur[ForceJeu] - ForceLongueur[ForceJeu-1]);
        //  Cas jeu moyen, part de la valeur du dessous
        val = val_longueur[ForceJeu-1][pJeu->NbCouleur[Couleur]];
        //  Et ajoute un facteur linéaire par rapport au pourcentage
        val +=  (val_longueur[ForceJeu][pJeu->NbCouleur[Couleur]] - val_longueur[ForceJeu-1][pJeu->NbCouleur[Couleur]])*pourcentage;
    }
#if DEBUG_ENTAME_DEFENSE > 0
    OutDebug("Entrée GetValeurCouleurOuverture(%d) : ForceJeu=%d, val_longueur = %.2f, pourcentage=%.2f, SansAtoutPreneur=%.2f\n", Couleur, ForceJeu, val, pourcentage, SansAtoutPreneur);
#endif // DEBUG_ENTAME_DEFENCE
    //	Calcule maintenant les points liés à la force de la couleur
    i0 = GetPlusForte(pJeu, Couleur);
    //  Au moins le cavalier : +2.5 points
    if ( pJeu->MyCarte[i0].Hauteur >= CAVALIER ) val += 2.5;
    //  Au moins le 9 second +1 point
    if ( pJeu->NbCouleur[Couleur] > 3 && pJeu->MyCarte[i0-1].Hauteur >= 9) val += 1.0;
        //	Si toutes les cartes sont faibles enlève un peu de points
    if ( pJeu->MyCarte[i0].Hauteur <= 10 && pJeu->NbCouleur[Couleur] >= 4 ) val -= 1.5;
        //	Enlève des points si DAME seconde : -1 si possède petit, -3 sinon
    if ( pJeu->NbCouleur[Couleur] == 2 && pJeu->MyCarte[i0].Hauteur == DAME )
        val -= 1.0 + 2.0*(HasPetit(pJeu) == 0);
        //	Enlève des points si DAME ou CAVAL TROISIEME
    if ( pJeu->NbCouleur[Couleur] == 3 && pJeu->MyCarte[i0].Hauteur == CAVALIER )
        val -= 0.75;
    if ( pJeu->NbCouleur[Couleur] == 3 && pJeu->MyCarte[i0].Hauteur == DAME )
        val -= 0.75;
	//	Cas où le joueur possède le ROI, dépend de la force du jeu (pas trop intérêt à ouvrir sous un Roi)
    if ( pJeu->MyCarte[i0].Hauteur == ROI && pJeu->ForceMain[pJeu->PositionJoueur] <= 0.8 )
    {
        if ( pJeu->NbCouleur[Couleur] == 1 ) val -= 7.0;        //  Pas d'intérêt à lance un roi sec
        if ( pJeu->NbCouleur[Couleur] == 2 ) val -= 5.0;
        if ( pJeu->NbCouleur[Couleur] == 3 ) val -= 4.0;
        if ( pJeu->NbCouleur[Couleur] == 4 ) val -= 1.5;        //  à partir de 4, pas trop mal
        if ( pJeu->NbCouleur[Couleur] == 5 ) val -= 0.5;        //  à partir de 5, plus de souci
    }
    if ( pJeu->AtoutPreneur < 4 )
    {
        val += pJeu->GuessProbCoupe[Couleur]*5;                 //  Essaie de faire couper le preneur à la fin
    }

#if DEBUG_ENTAME_DEFENSE > 0
    OutDebug("GetValeurCouleurOuverture(%d) après honneurs: val=%.2f\n", Couleur, val);
#endif // DEBUG_ENTAME_DEFENCE
    //	Ajoute des points si permet de montrer main forte : ROI ou petit mariage
    if ( FlagSignalisation && pJeu->MyCarte[i0].Hauteur == ROI && pJeu->ForceMain[pJeu->PositionJoueur] >= 1.0 )
        val += 3.0;
    if ( FlagSignalisation && pJeu->MyCarte[i0].Hauteur == DAME
        && pJeu->NbCouleur[Couleur] > 1 && pJeu->MyCarte[i0-1].Hauteur == CAVALIER && pJeu->ForceMain[pJeu->PositionJoueur] >= 1.0 )
        val += 3.0;
	//	Retire des points si gêne la signalisation
    if ( FlagSignalisation && IsSigNOK(CurrentGame, pJeu, Couleur) )
        val -= 1.5;
    //  Si permet de montrer un roi ou une dame, ajoute un peu pour la première entame
    else if ( FlagSignalisation && pJeu->MyCarte[i0].Hauteur >= DAME && pJeu->NbEntame[pJeu->PositionJoueur] == 0 )
        val += 1.0;
#if DEBUG_ENTAME_DEFENSE > 0
    OutDebug("GetValeurCouleurOuverture(%d) après signalisation: val=%.2f\n", Couleur, val);
#endif // DEBUG_ENTAME_DEFENCE
	//	Retire des points si possède fourchette ROI / Cavalier sans la DAME surtout du fond
    if ( HasCarte(pJeu, pJeu->PositionJoueur, Startof[Couleur]+13) && HasCarte(pJeu, pJeu->PositionJoueur, Startof[Couleur]+11)
        && !HasCarte(pJeu, pJeu->PositionJoueur, Startof[Couleur]+12) )
    {
        val -= pos*pos * 0.5;       //  -0.5 devant, -2 milieu, -4.5 du fond
    }
	//	Pas d'entame dans un ROI du chien sauf si très fort ou petit court a signaler
    if ( HasCarte(pJeu, pJeu->PositionPreneur, Startof[Couleur]+13) )
    {
        if ( HasPetit(pJeu) && pJeu->NbCouleur[Couleur] >= 5 )
        {
            val -= 0.2 * pJeu->NbAtout;
        }
        else
        {
            val -= 4.0 / sqrt(pJeu->ForceMain[pJeu->PositionJoueur]);
        }
#if DEBUG_ENTAME_DEFENSE > 0
        OutDebug("GetValeurCouleurOuverture(%d) couleur Roi du chien: val=%.2f\n", Couleur, val);
#endif // DEBUG_ENTAME_DEFENCE
    }
    //	Correction liée aux cartes au chien si joué moins de 2 couleurs
    //  Idée : pas très bon de jouer dans ce type de couleur du fond
    if ( CompteCouleurJoueur(pJeu, pJeu->PositionPreneur) < 2 )
        val -= pJeu->NbCouleurChien[Couleur] * pJeu->NbCouleurChien[Couleur] / (pos+1);
    //	Privilégie la couleur où pas mal de non honneurs...
    if ( NbNonHonneur(CurrentGame, pJeu->PositionJoueur, Couleur) >= 3 && pJeu->ForceMain[pJeu->PositionJoueur] < 1.0)
        val += 0.25 * NbNonHonneur(CurrentGame, pJeu->PositionJoueur, Couleur) * (1.5 - pJeu->ForceMain[pJeu->PositionJoueur]);
    //	A priori n'ouvre pas la dernière couleur
    if ( NbCouleurOuverte(pJeu) == 3 )
    {
        val += -10.0 + 5.0 * CompteCouleurJoueur(pJeu, pJeu->PositionPreneur);
    }
    else if ( pos == 1 )
    {
        val += pJeu->GuessProbCoupe[Couleur]*5;
    }
    else if ( pos == 2 )
    {
        val += pJeu->GuessProbCoupe[Couleur]*2;
    }
    val *= 1.0 - SansAtoutPreneur;
    //	Si pas d'Atout au preneur règles différentes
    //	Favorise toujours les couleurs longues, mais avec le ROI...
    val += SansAtoutPreneur *
        (5.0 - 3.0 * pos + 0.5*pJeu->NbCouleur[Couleur]
        - 4.0 * pJeu->TmpProbCarte[pJeu->PositionPreneur][Startof[Couleur]+ROI-1]
        + (3.0 + 2.0*pos) * HasCarte(pJeu, pJeu->PositionJoueur, Startof[Couleur]+ROI-1)
        + 2.0 * pJeu->CouleurVoulue[Couleur]);


    return val;
}

//	"Coups" spécifiques défense, joue avant les règles générales
//	Retourne la carte à jouer si trouve quelque chose

static int CoupsEntameDefense(TarotGame CurrentGame, struct _Jeu *pJeu, double ValCouleur[6])
{
int PosPreneur = (pJeu->PositionPreneur - pJeu->PositionJoueur) & 3;        //  Position relative par rapport au preneur
int PositionPetit = JoueurAvecPetit(CurrentGame, pJeu);                     //  Position la plus probable du petit
int c;
int i0;


#if DEBUG_ENTAME_DEFENSE > 0
        OutDebug("Entrée CoupsEntameDefense : NbCarte=%d, Joueur=%d, ValCouleur=[%.2f, %.2f, %.2f, %.2f, %.2f, %.2f]\n",
                    pJeu->NbCarte, pJeu->PositionJoueur, ValCouleur[0], ValCouleur[1], ValCouleur[2], ValCouleur[3], ValCouleur[4], ValCouleur[5]);
#endif // DEBUG_ENTAME_DEFENCE
    //	Coup 1 : Si à deux cartes de la fin, possède un atout (mais pas l'excuse) et une couleur et que le preneur
    //  possède deux atouts dont le petit, joue couleur pour éviter petit au bout
	if ( pJeu->NbCarte == 2 && pJeu->NbAtout == 1 && HasCarte(pJeu, pJeu->PositionJoueur, 0) == 0
		&& pJeu->AtoutPreneur >= 1.95 && pJeu->TmpProbCarte[pJeu->PositionPreneur][1] > 0.99 )
	{
#if DEBUG_ENTAME_DEFENSE > 0
        OutDebug("CoupsEntameDefense n° 1 : Joue Couleur pour éviter petit au bout\n");
        OutDebug("NbCarte=%d, NbAtout=%d, HasExcuse=%d, AtoutPreneur=%.2f, ProbPetitPreneur = %.2f\n",
                    pJeu->NbCarte, pJeu->NbAtout, HasExcuse(pJeu), pJeu->AtoutPreneur, pJeu->TmpProbCarte[pJeu->PositionPreneur][1]);
#endif // DEBUG_ENTAME_DEFENCE
		return(1);		//	La carte n° 1 est celle de la couleur
	}
    //	Coup 2 : Si possède ATOUT maître et petit pas encore joué, le joue.
    //	Pas vrai si après le preneur ni si possède deux atouts maîtres
	if ( pJeu->AtoutPreneur > 0.5 && !HasPetit(pJeu) && pJeu->NbAtout > 0 && IsMaitreCouleur(CurrentGame, pJeu, ATOUT)
		&& IsJoue(CurrentGame, 1) == 0
        && (pJeu->TmpProbCarte[pJeu->PositionPreneur][1] < 0.9 - PosPreneur*0.1 - 0.1*pow(NbAtoutMaitre(CurrentGame, pJeu->PositionJoueur),2))
        && PosPreneur != 3)
	{
#if DEBUG_ENTAME_DEFENSE > 0
        OutDebug("CoupsEntameDefense n° 2 : Joue ATOUT Maître petit pas encore joué\n");
        OutDebug("AtoutPreneur=%.2f, HasPetit=%d,  IsJoue(Petit)=%d, NbAtout=%d, IsMaitre(ATOUT)=%d, PosPreneur=%d, ProbaPetitPreneur=%.2f\n",
                    pJeu->AtoutPreneur, HasPetit(pJeu), IsJoue(CurrentGame, 1),
                    pJeu->NbAtout, IsMaitreCouleur(CurrentGame, pJeu, ATOUT), PosPreneur,
                    pJeu->TmpProbCarte[pJeu->PositionPreneur][1]);
#endif // DEBUG_ENTAME_DEFENCE
		return(pJeu->NbAtout-1);        //  C'est l'atout le plus fort
	}
    //	Coup 3 :
    //	Si possède petit (court) et que le preneur ne possède pas le plus gros atout, joue le petit
	if ( HasPetit(pJeu) && pJeu->NbAtout < pJeu->AtoutPreneur - 3 )
	{
		for ( int i = 21; i > 0; i--)
		{
			if ( HasCarte(pJeu, pJeu->PositionJoueur, i) ) continue;
			if ( IsJoue(CurrentGame, i)) continue;
			if ( pJeu->TmpProbCarte[pJeu->PositionPreneur][i] == 0 )
			{	//	Joue le petit dans ce cas.
#if DEBUG_ENTAME_DEFENSE > 0
                OutDebug("CoupsEntameDefense n° 3 : Joue Petit quand preneur n'a pas les atouts maîtres\n");
                OutDebug("AtoutPreneur=%.2f, HasPetit=%d,  IsJoue(Petit)=%d, NbAtout=%d, ProbaAtoutMaitre[%d]Preneur=%.2f\n",
                            pJeu->AtoutPreneur, HasPetit(pJeu), IsJoue(CurrentGame, 1),
                            pJeu->NbAtout, i, pJeu->TmpProbCarte[pJeu->PositionPreneur][i]);
#endif // DEBUG_ENTAME_DEFENCE
				return(GetPlusFaible(pJeu, ATOUT));
			}
			break;
		}
	}
#ifdef UNDEF
	//	Coup 4 :
//	Si possède une coupe dans ROI au chien et couleur non ouverte, et pas mal de points, joue ATOUT pour forcer
//	le preneur à ouvrir cette couleur.
	for ( int i = 0; i < 4; i++ )		//	Pour chaque couleur
	{
		if ( NbCouleur[i] != 0 ) continue;		//	Cherche une coupe...
		if ( HasCarte(Preneur, Endof[i]-1) == 0 ) continue;		//	Le preneur doit avoir le ROI
		if ( JoueurCouleur[i] >= 0 ) continue;	//	La couleur ne doit pas être ouverte.
		val = ComptePoints()*1.0 + PosPreneur*PosPreneur*1.0;
		if ( val > 20 && JoueurCouleur[ATOUT] < 0 && !HasCarte(Position, 1) && NbAtout >= 4)
		//	OK et ATOUT non ouvert ?
		{
			for ( int j = GetPlusFaible(ATOUT); MyCarte[j].Couleur == ATOUT; j++)
			{
				if ( (MyCarte[j].Hauteur & 1) == 0 ) return(j);
			}
		}
	}
#endif
    //	Coup 5 : Si peut prendre petit au preneur, joue ATOUT
    //  Preneur avec peu d'atout et pas excuse chez le preneur
	if ( PositionPetit == pJeu->PositionPreneur  && pJeu->AtoutPreneur < 1.1 && pJeu->NbAtout > HasExcuse(pJeu) && !HasCarte(pJeu, pJeu->PositionPreneur, 0))
	{
#if DEBUG_ENTAME_DEFENSE > 0
        OutDebug("CoupsEntameDefense n° 5.1 : Joue ATOUT pour prendre petit au preneur\n");
        OutDebug("AtoutPreneur=%.2f, PositionPetit=%d,  IsJoue(Petit)=%d, NbAtout=%d, ExcusePreneur=%.2f\n",
                    pJeu->AtoutPreneur, PositionPetit, IsJoue(CurrentGame, 1),
                    pJeu->NbAtout, pJeu->TmpProbCarte[pJeu->PositionPreneur][0]);
#endif // DEBUG_ENTAME_DEFENCE
		return(GetPlusForte(pJeu, ATOUT));
	}
	if ( PositionPetit == pJeu->PositionPreneur && pJeu->AtoutPreneur < pJeu->NbAtout + 0.01
        && NbAtoutMaitre(CurrentGame, pJeu->PositionJoueur) >= pJeu->AtoutPreneur - 1.05 && pJeu->NbAtout < HasExcuse(pJeu) )
	{
#if DEBUG_ENTAME_DEFENSE > 0
        OutDebug("CoupsEntameDefense n° 5.2 : Joue ATOUT pour prendre petit au preneur\n");
        OutDebug("AtoutPreneur=%.2f, PositionPetit=%d,  IsJoue(Petit)=%d, NbAtout=%d, NbAtoutMaitre=%d\n",
                    pJeu->AtoutPreneur, PositionPetit, IsJoue(CurrentGame, 1),
                    pJeu->NbAtout, NbAtoutMaitre(CurrentGame, pJeu->PositionJoueur));
#endif // DEBUG_ENTAME_DEFENCE
		return(GetPlusForte(pJeu, ATOUT));
	}
    //	Coup 6 : Si possède deux ATOUTS dont le maître et en reste 2 dehors à deux cartes de la fin, joue le maître
	if ( NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT) == 2 && pJeu->NbAtout == 2 && IsMaitreCouleur(CurrentGame, pJeu, ATOUT) )
	{
#if DEBUG_ENTAME_DEFENSE > 0
        OutDebug("CoupsEntameDefense n° 6 : Joue ATOUT maître à 2 coups de la fin\n");
        OutDebug("AtoutPreneur=%.2f, IsMaitre(ATOUT)=%d, NbAtout=%d, Reste(ATOUT)=%d\n",
                    pJeu->AtoutPreneur, IsMaitreCouleur(CurrentGame, pJeu, ATOUT),
                    pJeu->NbAtout, NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT));
#endif // DEBUG_ENTAME_DEFENCE
		return(GetPlusForte(pJeu, ATOUT));
	}
    //	Coup 7 : Si entame (NbCarte = 18) essaie Entame Singleton
	if ( pJeu->NbCarte == 18 && pJeu->NbAtout <= 3 && ComptePoints(pJeu) >= 15)
	{
		for ( c = TREFLE; c < NB_COULEUR; c++ )		//	Pour chaque couleur
		{
			if ( pJeu->NbCouleur[c] != 1 ) continue;		//	Cherche une singlette...
			if ( pJeu->NbCouleurChien[c] == 0 ) 	//	0 cartes de cette couleur au chien, sinon abandonne
            {
#if DEBUG_ENTAME_DEFENSE > 0
                OutDebug("CoupsEntameDefense n° 7.1 : Joue Entame singleton si peu d'atouts et beaucoup de points\n");
                OutDebug("NbAtout=%d, NbCarte=%d, ComptePoints(Jeu)=%d, NbCouleurChien[%d]=%d\n",
                            pJeu->NbAtout, pJeu->NbCarte, ComptePoints(pJeu), c, pJeu->NbCouleurChien[c]);
#endif // DEBUG_ENTAME_DEFENCE
				return(GetPlusForte(pJeu, c));
            }
			if ( pJeu->NbCouleurChien[c] == 1 && ComptePoints(pJeu) >= 20 ) 	//	1 carte et beaucoup de points
            {
#if DEBUG_ENTAME_DEFENSE > 0
                OutDebug("CoupsEntameDefense n° 7.2 : Joue Entame singleton si peu d'atouts et beaucoup de points\n");
                OutDebug("NbAtout=%d, NbCarte=%d, ComptePoints(Jeu)=%d, NbCouleurChien[%d]=%d\n",
                            pJeu->NbAtout, pJeu->NbCarte, ComptePoints(pJeu), c, pJeu->NbCouleurChien[c]);
#endif // DEBUG_ENTAME_DEFENCE
				return(GetPlusForte(pJeu, c));
            }
		}
	}
    //	Coup 8 : Si possède un cavalier ou une dame sèche avec le petit court, joue l'honneur
    //  Ne le fait que pour la première entame !
	if ( pJeu->NbEntame[pJeu->PositionJoueur] == 0 && HasPetit(pJeu) && pJeu->NbAtout <= 5)
	{
		for ( c = TREFLE; c < NB_COULEUR; c++ )		//	Pour chaque couleur
		{
			if ( pJeu->NbCouleur[c] != 1 ) continue;		//	Cherche une singlette...
			i0 = GetPlusFaible(pJeu, c);                    //  Carte de la couleur
			if ( pJeu->MyCarte[i0].Hauteur >= CAVALIER && pJeu->MyCarte[i0].Hauteur <= DAME )
			{
#if DEBUG_ENTAME_DEFENSE > 0
                OutDebug("CoupsEntameDefense n° 8 : Joue Cavalier ou Dame sèche avec petit court\n");
                OutDebug("NbAtout=%d, NbEntame=%d, NbCouleur[%d]=%d, Hauteur=%d\n",
                            pJeu->NbAtout, pJeu->NbEntame[pJeu->PositionJoueur], c, pJeu->NbCouleur[c], pJeu->MyCarte[i0].Hauteur);
#endif // DEBUG_ENTAME_DEFENCE
				return(i0);
			}
		}
	}
    //	Coup 9 : Si entame (NbCarte = 18) essaie Entame Doubleton..., ne joue pas directement mais change ValCouleur
	if ( pJeu->NbCarte == 18 && pJeu->NbAtout <= 3 && ComptePoints(pJeu) >= 15)
	{
		for ( c = TREFLE; c < NB_COULEUR; c++ )		//	Pour chaque couleur
		{
			if ( pJeu->NbCouleur[c] != 2 ) continue;		//	Cherche un doubleton..
			if ( pJeu->NbCouleurChien[c] <= 1 )             //	Max 1 carte de cette couleur au chien, sinon abandonne
            {
				ValCouleur[c] += 5.0 - pJeu->NbCouleurChien[c] - pJeu->NbAtout + pJeu->NbAtoutMaj;
#if DEBUG_ENTAME_DEFENSE > 0
                OutDebug("CoupsEntameDefense n° 9 : Favorise entame doubleton\n");
                OutDebug("NbAtout=%d, NbCarte=%d, ComptePoints(Jeu)=%d, NbCouleur[%d]=%d, NbCouleurChien[%d]=%d, ValCouleur[c] = %.2f\n",
                            pJeu->NbAtout, pJeu->NbCarte, ComptePoints(pJeu), c, pJeu->NbCouleur[c], c, pJeu->NbCouleurChien[c], ValCouleur[c]);
#endif // DEBUG_ENTAME_DEFENCE
				ValCouleur[c] += 5.0 - pJeu->NbCouleurChien[c] - pJeu->NbAtout + pJeu->NbAtoutMaj;
            }
		}
	}
    //	Coup 10 : si reste un atout à deux cartes de la fin et idem coté preneur, joue la couleur
	if ( pJeu->NbCarte == 2 && pJeu->NbAtout == 1 && HasExcuse(pJeu) == 0 && pJeu->AtoutPreneur >= 0.75 )
	{
#if DEBUG_ENTAME_DEFENSE > 0
        OutDebug("CoupsEntameDefense n° 10 : si reste un atout à deux cartes de la fin et idem coté preneur, joue la couleur\n");
        OutDebug("NbCarte=%d, NbAtout=%d, NbAtoutPreneur=%.2f\n",
                    pJeu->NbCarte, pJeu->NbAtout, pJeu->AtoutPreneur);
#endif // DEBUG_ENTAME_DEFENCE
		return(1);		//	La carte n° 1 est celle de la couleur, le n°0 est l'atout
	}
    //	Coup 11 : Si entame (NbCarte = 18) essaie Entame Singleton (avec un peu plus d'atouts)
	if ( pJeu->NbCarte == 18 && pJeu->NbAtout <= 4 && ComptePoints(pJeu) >= 15)
	{
		for ( c = TREFLE; c < NB_COULEUR; c++ )		//	Pour chaque couleur
		{
			if ( pJeu->NbCouleur[c] != 1 ) continue;		//	Cherche une singlette...
			if ( pJeu->NbCouleurChien[c] <= 1 )
            {
                ValCouleur[c] += 8.0 - pJeu->NbCouleurChien[c] - 0.75*pJeu->NbAtout + pJeu->NbAtoutMaj;
#if DEBUG_ENTAME_DEFENSE > 0
                OutDebug("CoupsEntameDefense n° 11 : Favorise entame singleton\n");
                OutDebug("NbCarte=%d, NbAtout=%d, NbCouleurChien[%d]=%d, ValCouleur[c]=%.2f\n",
                            pJeu->NbCarte, pJeu->NbAtout, c, pJeu->NbCouleurChien[c], ValCouleur[c]);
#endif // DEBUG_ENTAME_DEFENSE
            }
		}
	}
	//  Coup 12 : Si première entame du joueur avec petit court, essaie entame singleton, surtout si pas mal de points
	//  Ajoute quelques points si ROI au chien, cela montrera le petit court
	if ( pJeu->NbEntame[pJeu->PositionJoueur] == 0 && pJeu->NbAtout <= 4 && HasPetit(pJeu) + ComptePoints(pJeu) >= 15 )
    {
        for ( c = TREFLE; c < NB_COULEUR; c++)
        {
            //  Cherche singleton mais pas au ROI sauf si possède Petit
            if ( pJeu->NbCouleur[c] == 1 && (pJeu->ProbCarte[pJeu->PositionJoueur][Endof[c]-1] == 0 || HasPetit(pJeu)))
            {
                ValCouleur[c] +=  (5 - pJeu->NbAtout)*(pJeu->NbCouleurChien[c] * 2.0 + 5.0*HasPetit(pJeu)*HasCarte(pJeu, pJeu->PositionPreneur, Endof[c]-1));
#if DEBUG_ENTAME_DEFENSE > 0
                OutDebug("CoupsEntameDefense n° 12 : Entame singleton si petit court\n");
                OutDebug("NbCarte=%d, NbAtout=%d, NbCouleurChien[%d]=%d, ComptesPoints=%d, ValCouleur[c]=%.2f\n",
                            pJeu->NbCarte, pJeu->NbAtout, ComptePoints(pJeu), c, pJeu->NbCouleurChien[c], ValCouleur[c]);
#endif // DEBUG_ENTAME_DEFENSE
                if ( ValCouleur[c] > 10.0 )
                {
                    //  Bon coup, joue directement cette carte
                    i0 = GetPlusFaible(pJeu, c);
                    return i0;
                }
            }
        }
    }
#if DEBUG_ENTAME_DEFENSE > 0
    OutDebug("Sortie CoupsEntameDefense : Pas de coup sûr\n");
    OutDebug("ValCouleur TREFLE=%.3f CARREAU=%.3f PIQUE=%.3f COEUR=%.3f\n",
             ValCouleur[TREFLE], ValCouleur[CARREAU], ValCouleur[PIQUE], ValCouleur[COEUR] );
#endif // DEBUG_ENTAME_DEFENSE

	return(-1);
}

//  Joue l'entame (première carte du jeu) quand c'est à la défense d'ouvrir
//  Choisit la meilleure ouverture en fonction du jeu du joueur et du chien
//	Recherche la couleur à ouvrir
//	"Règles" utilisées :
//	1)	Met des points aux couleurs longues (au moins 5eme) et tenues.
//	2)	Met des points à l'atout si fort (7 ou 6eme avec 2 > 15)
//	3)	Si coupe un roi du preneur, ajoute des points à l'ATOUT, surtout du fond.
//		Ceci est vrai si possède au moins deux atouts
//	4)	Si main forte, le montre avec un ROI par exemple.
//		Ajoute des points si permet de montrer sa force.
//	5)	Retire des points à la longue du chien (surtout en premier)
//	6)	Si main assez faible, privilégie les couleurs où beaucoup de cartes inférieures au valet
//		pour ne pas perdre de points.
//	7)	Si couleur trop longue et jeu assez faible, enlève des points (gêne les partenaires)
//	8)	Enlève un peu de points si gêne la signalisation
//	9)	Si joue atout avec plus de deux atouts au chien, cela veut dire que le joueur tient
//		la couleur du chien. Ajoute quelques points à cette solution.
//	10)	Si possède ROI dans une couleur enlève un peu de points
//	11)	Si Possède ROI + CAVALIER enlève plus de points pour que le preneur ne fasse pas sa dame
//		(surtout du fond).
//	12)	Si très court dans la longue du chien, ne joue pas forcément ATOUT impair
//		(sauf si possède les très gros).
//	13)	Entame singleton dans des cas bien précis..
//	14)	Ok pour entame singleton si possède un cavalier sec, surtout si du fond pour surcouper
//		le preneur (76)
//	15)	Pas d'entame dans un roi du chien, sauf si jeu fort.
//	16)	Ajoute des points à l'atout en fonction des atouts au chien
//	17)	Retire des points à l'ATOUT si possède petit
//	18)	Joue plus atout du fond. Si ne sait pas quoi ouvrir (pb style pas de longues évidente,
//		ROI assez court), préfère ATOUT pair dans ce cas.


void EntameDefense(TarotGame CurrentGame)
{
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[CurrentGame->JoueurEntame];
int pos = (pJeu->PositionPreneur - pJeu->PositionJoueur) & 3;
double ValCouleur[6];
double Best = -10000000.0;
int BestC = 0;
int GoChasse = 0;
int c;
int i0;
double Val;

	for ( c = 0; c < NB_COULEUR; c++) ValCouleur[c] = 0;
		OutDebug("ERROR : Recherche plus faible couleur %d pour Joueur %d, ne trouve pas de carte de cette couleur\n", c, pJeu->PositionJoueur);
    //	Coups spécifiques d'entame défense en contradiction avec les règles générales
    //	Si retour positif ou nul, joue la carte correspondante.
	if ( (i0 = CoupsEntameDefense(CurrentGame, pJeu, ValCouleur)) >= 0 )
	{
		PoseCarte(CurrentGame, i0);
		return;
	}
#if DEBUG_ENTAME_DEFENSE > 0
    OutDebug("Entrée EntameDéfense, NbCarte=%d, NbAtout=%d, ForceJeu=%.2f\n", pJeu->NbCarte, pJeu->NbAtout, pJeu->ForceMain[pJeu->PositionJoueur]);
    OutDebug("ProbCoupe[T]=%.3f, ProbCoupe[K]=%.3f, ProbCoupe[P]=%.3f, ProbCoupe[C]=%.3f\n",
          pJeu->ProbCoupe[TREFLE], pJeu->ProbCoupe[CARREAU], pJeu->ProbCoupe[PIQUE], pJeu->ProbCoupe[COEUR]);
    OutDebug("GuessProbCoupe[T]=%.3f, GuessProbCoupe[K]=%.3f, GuessProbCoupe[P]=%.3f, GuessProbCoupe[C]=%.3f\n",
          pJeu->GuessProbCoupe[TREFLE], pJeu->GuessProbCoupe[CARREAU], pJeu->GuessProbCoupe[PIQUE], pJeu->GuessProbCoupe[COEUR]);
    OutDebug("CalProbCoupe[T]=%.3f, CalProbCoupe[K]=%.3f, CalProbCoupe[P]=%.3f, CalProbCoupe[C]=%.3f\n",
          pJeu->CalProbCoupe[TREFLE], pJeu->CalProbCoupe[CARREAU], pJeu->CalProbCoupe[PIQUE], pJeu->CalProbCoupe[COEUR]);
    OutDebug("EntameDefense, recherche meilleure couleur\n");
#endif // DEBUG_ENTAME_DEFENCE
    //	Pour chaque couleur, calcule valeur de la couleur.
	for ( c = TREFLE; c < NB_COULEUR; c++)
	{
		if ( pJeu->NbCouleur[c] == 0 )
		{
			ValCouleur[c] = -100000;
			continue;
		}
        //	Valeur liée à la longueur de la couleur...
        //  Sans AtoutPreneur = 0 ici !
        ValCouleur[c] += GetValeurCouleurOuverture(CurrentGame, pJeu, c, 0.0);
#if DEBUG_ENTAME_DEFENSE > 0
        OutDebug("GetValeurCouleurOuverture(%d) = %.2f\n", c, ValCouleur[c]);
#endif // DEBUG_ENTAME_DEFENSE
	}
	//	Pour l'atout...
	if ( pJeu->NbAtout > HasExcuse(pJeu) )
	{
		Val = 0.0;
		//  Plus de 7 atouts ou 6 avec deux au dessus du 16 et pas d'atout au chien
		//  OK pour jouer atout, on va à la chasse !
		if ( !HasPetit(pJeu) && (pJeu->NbAtout >= 7 || (pJeu->NbAtout >= 6 && pJeu->NbAtoutMaj >= 2 && pJeu->NbCouleurChien[ATOUT] == 0)) )
		{
			Val = 8.0 + 1.5*(pJeu->NbAtout - 6) + 1.0*(pJeu->NbAtoutMaj-1);
			GoChasse = 1;
		}
		//	Ajoute des points à l'atout si coupe un roi dont on est sûr qu'il est chez le preneur (au chien)
		//  Il faut bien sûr avoir pas mal d'atouts
		//  Et c'est intéressant du fond
		for ( c = TREFLE; c < NB_COULEUR; c++)
		{
			if ( HasCarte(pJeu, pJeu->PositionPreneur, Startof[c]+13) && pJeu->NbCouleur[c] == 0 )
				Val += 3.0 * pos * pos * (1.0 - 1.0/sqrt(pJeu->NbAtout));
		}
		//	Points en fonction des atouts au chien, retire des points si beaucoup d'atouts au chien
		//  Ajoute 1.7 si 0 atouts, 0.08 si 1 atout, retire
		Val -= pow(pJeu->NbCouleurChien[ATOUT]-1.2,3);
		//	Points en fonction de la position. Mieux du fond, devant les entrées sont préférables
		Val += (pos-1) * (pos-1);
		//	Points en fonction du nombre d'atouts
		Val += sqrt(pJeu->NbAtout);
		//	Recherche longue au chien : si oui, et la tient, ajoute des points à l'atout
		for ( c = TREFLE; c < NB_COULEUR; c++)
		{
			if ( pJeu->NbCouleurChien[c] >= 3 )
				Val += 2.0 + 2.0*(pJeu->NbCouleur[c] - 5);
		}
		//	Si très peu d'atouts et pas au chien, joue éventuellement pair, mais pas devant
		if ( pJeu->NbCouleurChien[ATOUT] == 0 && pJeu->NbAtout <= 3 && IsSigNOK(CurrentGame, pJeu, ATOUT) == 0 )
			Val += 2.0*(pos-1) * (pos-1);
		//	Si Pb signalisation enlève des points
		if ( IsSigNOK(CurrentGame, pJeu, ATOUT) )
			Val -= 2.0;
		//	Enlève des points si possède PETIT... Sauf si énormément d'atouts...
		Val -= 15.0*HasPetit(pJeu) * (pJeu->NbAtout < 7);
		if ( Val < 10.0 )
			GoChasse = 0;
		ValCouleur[ATOUT] += Val;
#if DEBUG_ENTAME_DEFENSE > 0
        OutDebug("EntameDefense, Valeur[ATOUT] = %.2f, GoChasse=%d\n", ValCouleur[ATOUT], GoChasse);
#endif // DEBUG_ENTAME_DEFENSE
	}
	//  Cas excuse : pas très bon en général mais à éviter absolument devant le preneur
	if ( pJeu->MyCarte[0].Couleur == EXCUSE )
    {
        ValCouleur[EXCUSE] = -8.0;
        if ( pos == 1 && pJeu->NbCarte > 2 )
            ValCouleur[EXCUSE] = -20;
        if ( pJeu->NbCarte == 2 )
            ValCouleur[EXCUSE] = 50;        //  Joue l'excuse avant de la perdre...
    }
    else
    {
        ValCouleur[EXCUSE] = -1000.0;
    }
	//	Recherche la couleur la meilleure
	for ( c = 0; c < NB_COULEUR; c++)
	{
		if ( ValCouleur[c] > Best )
		{
			Best = ValCouleur[c];
			BestC = c;
		}
	}
	//	Recherche maintenant la meilleure carte
	if ( BestC == EXCUSE )
    {
        i0 = 0;
        assert(pJeu->MyCarte[i0].Couleur == EXCUSE );
    }
	else if ( BestC != ATOUT )
	{
		//	Recherche la meilleure carte à la couleur
		if ( HasCarte(pJeu, pJeu->PositionJoueur, Startof[BestC]+13) && pJeu->ForceMain[pJeu->PositionJoueur] > 0.85 )
		{
			//	Jeu Fort, joue le ROI
			i0 = GetPlusForte(pJeu, BestC);
		}
		// Idem si possède DAME + CAVALIER et jeu assez fort
		else if ( HasCarte(pJeu, pJeu->PositionJoueur, Startof[BestC]+DAME-1) &&
                HasCarte(pJeu, pJeu->PositionJoueur, Startof[BestC]+CAVALIER-1) && pJeu->ForceMain[pJeu->PositionJoueur] >= 0.85)
		{
			i0 = GetPlusForte(pJeu, BestC) - 1;		//	Pointe sur Cavalier
		}
        //  Jeu assez faible avec ROI / DAME et moins de 4 dans la couleur
        //  Choisit la dame pour ne pas montrer jeu fort
		else if ( pJeu->ForceMain[pJeu->PositionJoueur]  < 0.5 && pJeu->NbCouleur[BestC] <= 4
			&& HasCarte(pJeu, pJeu->PositionJoueur, Startof[BestC]+ROI-1)
            && HasCarte(pJeu, pJeu->PositionJoueur, Startof[BestC]+DAME-1) )
		{
			i0 = GetPlusForte(pJeu, BestC) - 1;		//	Pointe sur la DAME
		}
		else
        {
            //  Joue une faible
            //	Si ne possède pas ROI ni DAME joue entre 5 et 10 sinon entre 1 et 5
            i0 = GetPlusFaible(pJeu, BestC);
            if ( FlagSignalisation )
            {
                if ( HasCarte(pJeu, pJeu->PositionJoueur, Startof[BestC]+ROI-1) == 0
                    &&  HasCarte(pJeu, pJeu->PositionJoueur, Startof[BestC]+DAME-1) == 0 && pJeu->MyCarte[i0].Hauteur <= 5)
                {
                    //  Monte tant que carte inférieure au 5
                    while ( i0 < (pJeu->NbCarte - 1) && pJeu->MyCarte[i0+1].Couleur == BestC && pJeu->MyCarte[i0].Hauteur <= 5 ) i0++;
                }
            }
        }
	}
	else
	{
	    //  Choix de la carte pour l'atout
		if ( GoChasse )
			pJeu->StatusChasse = CHASSE_DEFENSE_IMPAIR;
        i0 = GetPlusFaible(pJeu, ATOUT);
        if ( pJeu->MyCarte[i0].Hauteur == 1 ) i0++;      //  Ne joue pas le petit !!!
		if ( FlagSignalisation )
		{
		    //  Choisit un atout impair
			if ( GoChasse )
			{
				while ( pJeu->MyCarte[i0+1].Couleur == ATOUT && (pJeu->MyCarte[i0+1].Hauteur & 1) == 0 )
				{
					i0++;
				}
			}
			else
			{
				while ( pJeu->MyCarte[i0+1].Couleur == ATOUT && (pJeu->MyCarte[i0+1].Hauteur & 1) == 1 )
				{
					i0++;
				}
			}
		}
	}
    PoseCarte(CurrentGame, i0);
}

//	Calcule le coût probable d'une coupe du preneur dans la couleur Couleur
//  Ne tient pas compte de ses propres cartes

double CoutCoupe(struct _Jeu *pJeu, int Couleur)
{
int nb_avec_couleur = NbPossCouleur(pJeu, Couleur);
int Position = pJeu->PositionJoueur;
int Preneur = pJeu->PositionPreneur;
int j_avec_couleur = -1;
int joueur1, joueur2;
int i;
double val, probreste;

	if ( nb_avec_couleur == 1 )		//	Un seul possesseur de cartes de cette couleur
	{
		//	Recherche le joueur qui en possède
		for ( i = Startof[Couleur]; i < Endof[Couleur]; i++)
		{
			if ( pJeu->TmpProbCarte[Position^1][i] > 1e-7 ) j_avec_couleur = (Position^1);
			if ( pJeu->TmpProbCarte[Position^2][i] > 1e-7 ) j_avec_couleur = (Position^2);
			if ( pJeu->TmpProbCarte[Position^3][i] > 1e-7 ) j_avec_couleur = (Position^3);
		}
		val = 0;
		probreste = 1;          //  Proba qu'il n'ait aucune des cartes en dessous
		for ( i = Startof[Couleur]; i < Endof[Couleur]; i++)
		{
			if ( i - Startof[Couleur] < 10 )
				val += probreste * pJeu->TmpProbCarte[j_avec_couleur][i];        //  Pas honneur, 1 point
			else
				val += probreste * pJeu->TmpProbCarte[j_avec_couleur][i] * (2.0*(i - Startof[Couleur] - 9) + 1.0);   //  Compte les honneurs
			probreste *= 1.0 - pJeu->TmpProbCarte[j_avec_couleur][i];
		}
	}
	else
	{
        //	les deux partenaires possèdent des cartes de la couleur Couleur
		val = 0;
		joueur1 = Position ^ 1;
		if ( joueur1 == Preneur )
		{
			joueur1 = Position ^ 2;
			joueur2 = Position ^ 3;
		}
		else
		{
			joueur2 = Position ^ 2;
			if ( joueur2 == Preneur ) joueur2 = Position ^ 3;
		}
        //	Calcule "perte" du premier partenaire
		probreste = 1;
		for ( i = Startof[Couleur]; i < Endof[Couleur]; i++)
		{
			if ( pJeu->TmpProbCarte[joueur1][i] < 1e-7 ) continue;
			if ( i- Startof[Couleur] < 10 )
				val += probreste* pJeu->TmpProbCarte[joueur1][i];
			else
				val += probreste * pJeu->TmpProbCarte[joueur1][i] * (2.0*(i - Startof[Couleur] - 9) + 1.0);
			probreste *= 1.0 - pJeu->TmpProbCarte[joueur1][i];
		}
        //	Calcule la "perte" du second partenaire
		probreste = 1;
		for ( i = Startof[Couleur]; i < Endof[Couleur]; i++)
		{
			if ( pJeu->TmpProbCarte[joueur2][i] < 1e-7 ) continue;
			if ( i- Startof[Couleur] < 10 )
				val += probreste* pJeu->TmpProbCarte[joueur2][i];
			else
				val += probreste * pJeu->TmpProbCarte[joueur2][i] * (2.0*(i - Startof[Couleur] - 9) + 1.0);
			probreste *= 1.0 - pJeu->TmpProbCarte[joueur2][i];
		}
		//  Algo surestime la perte, réduit donc la valeur de perte
		if ( val > 2 )
			val -= 2.0;
		else
			val = 0.0;
	}
    //	Calcule la perte du joueur lui même
	for ( i = Startof[Couleur]; i < Endof[Couleur]; i++)
	{
		if ( pJeu->ProbCarte[Position][i] > 1e-7 )
		{
			if ( i- Startof[Couleur] >= 10 ) val += 2.0*(i - Startof[Couleur] - 9) + 1.0;
			return(val);
		}
	}
	return(val);
}

//  Calcule intérêt deux pour un (jouer petit ATOUT avant le preneur) pour revenir ensuite dans la coupe du preneur;

double DeuxPourUnOK(TarotGame CurrentGame, struct _Jeu *pJeu)
{
double va, vc;
int posJP = (pJeu->PositionPreneur - pJeu->PositionJoueur) & 3;     //  Position joueur par rapport au preneur
int c;

    if ( pJeu->NbAtout <= HasExcuse(pJeu) ) return -1000;       //  Pas d'atout !
    //  Si le preneur possède ATOUT maître, pas bon il peut le jouer et bloquer le coup
    if ( pJeu->TmpProbCarte[pJeu->PositionPreneur][CarteMaitreNonJouee(CurrentGame, ATOUT)] >= 0.8 || (pJeu->ResteAtout - pJeu->AtoutPreneur) < 0.5 )
            return -1000;
    va = -10000;
    for ( c = TREFLE; c < NB_COULEUR; c++ )
    {
        vc = 0;
        if ( pJeu->GuessProbCoupe[c] == 1.0 )
        {
            //  Sur de couper cette couleur ?
            vc = -CoutCoupe(pJeu, c);       //  Calcule coût de la coupe
            if ( vc > va )
                va = vc;
        }
    }
    va += pJeu->NbCoupeInit * 10.0 + 20.0 - 6*posJP - GetCartePlusFaible(pJeu, ATOUT)->Hauteur
        - 10.0 * pJeu->TmpProbCarte[pJeu->PositionPreneur][CarteMaitreNonJouee(CurrentGame, ATOUT)] - 4.0 * pJeu->NbCouleurChien[ATOUT];
    return va;
}

//  Jeu de la défense quand entame, mais pas pour la première carte du jeu.
//  Il y a déjà eu au moins un pli
//
//	La défense joue en premier...
//	"Règles" utilisées
//	1) Suit sauf motif valable la première couleur ouverte par la défense :
//		plus dans cette couleur,
//		petit en danger,
//		autre main forte,
//		peu de succès au premier coup.
//		La dernière possibilité n'est autorisée que si le premier ouvreur n'a pas signalé de main forte.
//	2)	Si désobéissance main forte, mieux vaut le signaler lors de l'ouverture pour ne pas
//		induire les partenaires en erreur (petit).
//		Sinon, les mêmes règles que pour la première ouverture s'appliquent.
//	3)	Si désobéissance car n'en possède plus, joue suivant les mêmes principes que pour l'ouverture.
//	4)	Désobéissance petit uniquement si danger pour ce petit, surtout si ouvreur signale main forte.
//	5)	Si changement de couleur pour cause de non coupe au premier tour,
//		mêmes règles que pour l'ouverture.
//	6)	Ne pas ouvrir la 4eme couleur si déjà deux coupes du preneur.
//	7)	Joue ATOUT si un partenaire a signalé la tenue de la couleur du preneur
//	8)	Joue COULEUR si signalée par un doubleton partenaire.
//	9)	Joue 2-pour-un si juste devant le preneur (surtout si petit tombé) ou du milieu si le
//		joueur du fond a signalé main forte.
//		Ne pas jouer si le preneur a montré une poignée avec beaucoup d'atouts forts.
//		Il faut également avoir trouvé une coupe du preneur, et que le joueur du fond en possède
//		encore.
//		Le 2-pour-un est favorisé si possède des points et peu d'atouts.
//		Si c'est le premier tour d'atout, mieux vaut jouer atout pair.
//	10)	Si juste avant le preneur, peut être intéressant d'ouvrir une couleur (sauf si déjà deux
//		coupes) pour faire la main du preneur
//	11)	D'autant moins désobéissance que peu de plis pour la défense (suite très fort au début,
//		peut changer de couleur par la suite).
//	12)	Ne joue pas dans une couleur ouverte par le preneur (ou où le preneur revient).
//

#if DEBUG > 0
#define DEBUG_DEFENSE_PREMIER    0
#else
#define DEBUG_DEFENSE_PREMIER    0
#endif // DEBUG

void JoueEnPremierDefense(TarotGame CurrentGame)
{
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[CurrentGame->JoueurEntame];
//  Position par rapport au preneur : -1 devant, 1 après, 2 du fond
int PosPreneurM1 = ((CurrentGame->JoueurPreneur - pJeu->PositionJoueur) & 3) - 1;
double p0;
int c, i, i0, i1;
double Chasse = 5.0;
int NbCoupe = 0;
double BestScore = -1e6;
int NbAtoutSurPreneur = 0;
double ValCouleur[6];
int CarteCouleur[6];
double SansAtoutPreneur = 0.0;
double va;
int GoChasse = 0;
double x;
double ProbCoupeAvant, ProbCoupeApres;
double ValAtoutFort = - 100000.0;
double nDis = nbDistrib(CurrentGame, pJeu);

	pJeu->AtoutPreneur = AvgLongueur(pJeu, CurrentGame->JoueurPreneur, ATOUT);
	pJeu->ResteAtout = NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT);
	GainPetitAuBout(CurrentGame, pJeu);
    //  Init valeur couleur et meilleure carte dans la couleur
	for ( i1 = 0; i1 < NB_COULEUR; i1++) CarteCouleur[i1] = -1;
	for ( i1 = 0; i1 < NB_COULEUR; i1++) ValCouleur[i1] = 0;
	CarteCouleur[EXCUSE] = 0;
    //	Teste si le preneur a de l'atout.
    //	Si non, les algorithmes sont différents
	if ( AvgLongueur(pJeu, pJeu->PositionPreneur, ATOUT) < 0.001 )
	{
		SansAtoutPreneur = 1.0;
		pJeu->StatusChasse = -1;			//	Plus d'affolement...
		if ( pJeu->Flanc == ATOUT ) pJeu->Flanc = -1;	//	On ne joue plus ATOUT
	}
	else
	{
    //  Convertit Sans AtoutPreneur pour être plus "tranchant" que la probabilité seule
    //	Modèle : SansAtoutPreneur = 0.6 - 1/PI*Atan(10*(AtoutPreneur - 0.25))
    //	Si AtoutPreneur = 0, SansAtoutPreneur = 0,978
    //	Si AtoutPreneur = 0.05, SansAtoutPreneur = 0,95
    //	Si AtoutPreneur = 0.5, SansAtoutPreneur = 0,22

		SansAtoutPreneur = 0.6 - 1.0/M_PI*atan(10.0*(pJeu->AtoutPreneur-0.25));
		if ( SansAtoutPreneur < 0.15 ) SansAtoutPreneur = 0.0;
	}
#if DEBUG_DEFENSE_PREMIER > 0
#if DEBUG > 0
    if ( CurrentGame->NumPli == 7 || CurrentGame->NumPli == 9 )
        c = 0;
#endif // DEBUG
    OutDebug("---------------------------------------- Pli %d ---------------------------------\n", CurrentGame->NumPli+1);
    OutDebug("Entrée JoueEnPremierDefense, Position %d, Flanc = %d, StatusChasse=%d, AvgLgPreneur=%.2f, SansAtoutPreneur=%.3f\n",
                pJeu->PositionJoueur, pJeu->Flanc, pJeu->StatusChasse,  AvgLongueur(pJeu, pJeu->PositionPreneur, ATOUT), SansAtoutPreneur);
    OutDebug("Nombre de distributions %.0f\n", nDis);
    OutDebug("ProbCoupe[T]=%.3f, ProbCoupe[K]=%.3f, ProbCoupe[P]=%.3f, ProbCoupe[C]=%.3f\n",
          pJeu->ProbCoupe[TREFLE], pJeu->ProbCoupe[CARREAU], pJeu->ProbCoupe[PIQUE], pJeu->ProbCoupe[COEUR]);
    OutDebug("GuessProbCoupe[T]=%.3f, GuessProbCoupe[K]=%.3f, GuessProbCoupe[P]=%.3f, GuessProbCoupe[C]=%.3f\n",
          pJeu->GuessProbCoupe[TREFLE], pJeu->GuessProbCoupe[CARREAU], pJeu->GuessProbCoupe[PIQUE], pJeu->GuessProbCoupe[COEUR]);
    OutDebug("CalProbCoupe[T]=%.3f, CalProbCoupe[K]=%.3f, CalProbCoupe[P]=%.3f, CalProbCoupe[C]=%.3f\n",
          pJeu->CalProbCoupe[TREFLE], pJeu->CalProbCoupe[CARREAU], pJeu->CalProbCoupe[PIQUE], pJeu->CalProbCoupe[COEUR]);
    OutDebug("Proba Petit = %d:%.3f  %d:%.3f  %d:%.3f  Chien:%.3f\n",
             pJeu->PositionJoueur^1, pJeu->TmpProbCarte[pJeu->PositionJoueur^1][1],
             pJeu->PositionJoueur^2, pJeu->TmpProbCarte[pJeu->PositionJoueur^2][1],
             pJeu->PositionJoueur^3, pJeu->TmpProbCarte[pJeu->PositionJoueur^3][1],
             pJeu->TmpProbCarte[4][1]);
    OutDebug("Proba 21 = %d:%.3f  %d:%.3f  %d:%.3f  Chien:%.3f\n",
             pJeu->PositionJoueur^1, pJeu->TmpProbCarte[pJeu->PositionJoueur^1][21],
             pJeu->PositionJoueur^2, pJeu->TmpProbCarte[pJeu->PositionJoueur^2][21],
             pJeu->PositionJoueur^3, pJeu->TmpProbCarte[pJeu->PositionJoueur^3][21],
             pJeu->TmpProbCarte[4][1]);
    OutDebug("Flanc = %d, ValFlanc = %.3f, CouleurTenue = %d\n", pJeu->Flanc, pJeu->Flanc > 0 ? pJeu->ValFlanc[pJeu->Flanc]:0.0, pJeu->CouleurTenue);
#endif // DEBUG
    if ( pJeu->NbCarte > 1 && pJeu->NbCarte <= MAX_CARTE_FIN_PARTIE_DEFENSE && nDis <= NOMBRE_DISTRIB_FIN_PARTIE_DEFENSE )
    {
        i0 = FinPartie(CurrentGame);
        if ( i0 >= 0 )
        {
            PoseCarte(CurrentGame, i0);
            return;
        }
    }
    //	Coups spécifiques d'entame défense en contradiction avec les règles générales
    //	Si retour positif ou nul, joue la carte correspondante.
	if ( (i0 = CoupsEntameDefense(CurrentGame, pJeu, ValCouleur)) >= 0 )
	{
		PoseCarte(CurrentGame, i0);
		return;
	}
    //	Pour chaque couleur, donne une valeur à cette couleur (ValCouleur) et détermine si possible
    //	la meilleure carte à jouer (CarteCouleur)
    //	Ensuite la couleur avec la plus forte valeur sera jouée. Si une carte a été trouvée elle sera
    //	également jouée.
    //	Pour information, la gamme des valeurs possibles se situe en gros en -20 et +20.

	for ( c = TREFLE; c < NB_COULEUR; c++)
	{
	    ProbCoupeAvant = ProbCoupeAvantPreneur(pJeu, c);
	    ProbCoupeApres = ProbCoupeApresPreneur(pJeu, c);
        //	Si n'en a pas, pas de problème, ne pourra pas en jouer
		if ( pJeu->NbCouleur[c] == 0 )
		{
			ValCouleur[c] = -100000;
			continue;
		}
        //	Couleur pas encore jouée : Applique les règles de l'ouverture
		else if ( pJeu->JoueurCouleur[c] < 0 )	        //	Pas encore ouverte
		{
            ValCouleur[c] = GetValeurCouleurOuverture(CurrentGame, pJeu, c, SansAtoutPreneur);
#if DEBUG_DEFENSE_PREMIER > 0
            OutDebug("Valeur ouverture %d : NbCouleur = %d, Val=%.3f, ProbCoupeAvant=%.3f, ProbCoupeApres=%.3f\n",
                    c, pJeu->NbCouleur[c], ValCouleur[c], ProbCoupeAvant, ProbCoupeApres);
#endif // DEBUG
            //	Carte a jouer dans cette couleur ?
			if ( SansAtoutPreneur > 0.75 && HasCarte(pJeu, pJeu->PositionJoueur, Startof[c]+ROI-1) )
				CarteCouleur[c] = GetPlusForte(pJeu, c);
			if ( FlagSignalisation && pJeu->NbEntame[pJeu->PositionJoueur] == 0 )	//	Première Entame du joueur
			{
				if ( (pJeu->JoueurMainForte < 0 && pJeu->ForceMain[pJeu->PositionJoueur] >= 0.85)
                    || (pJeu->JoueurMainForte >= 0 && pJeu->ForceMain[pJeu->PositionJoueur] > pJeu->ForceMain[pJeu->JoueurMainForte]))
				{
					if ( HasCarte(pJeu, pJeu->PositionJoueur, Startof[c]+ROI-1) )
					{
						CarteCouleur[c] = GetPlusForte(pJeu, c);
					}
					else if ( HasCarte(pJeu, pJeu->PositionJoueur, Startof[c]+DAME-1) && pJeu->TmpProbCarte[pJeu->PositionJoueur][Startof[c]+CAVALIER-1] > 0.99 )
					{
						CarteCouleur[c] = GetPlusForte(pJeu, c) - 1;		//	Pointe sur Cavalier
					}
				}
				else if ( pJeu->ForceMain[pJeu->PositionJoueur]  < 0.5 && pJeu->NbCouleur[c] <= 4)
				{
					if ( HasCarte(pJeu, pJeu->PositionJoueur, Startof[c]+ROI-1) && HasCarte(pJeu, pJeu->PositionJoueur, Startof[c]+DAME-1) )
					{
						CarteCouleur[c] = GetPlusForte(pJeu, c) - 1;		//	Pointe sur la DAME
					}
				}
			}
		}
        // Couleur ouverte par le possesseur du petit : c'est bon sauf si Preneur coupe et partenaire avec Petit aussi
		else if ( pJeu->JoueurCouleur[c] == JoueurAvecPetit(CurrentGame, pJeu) && pJeu->JoueurCouleur[c] != pJeu->PositionPreneur
			&& pJeu->TmpProbCarte[pJeu->JoueurCouleur[c]][1] > 0.75)
		{
			ValCouleur[c] += (1.0-SansAtoutPreneur)*(14.0 + 10.0*pJeu->TmpProbCarte[JoueurAvecPetit(CurrentGame, pJeu)][1]
                        - 1.0 * exp((1.0 + 4.0 *ProbCoupeJoueur(pJeu, pJeu->JoueurCouleur[c], c)) * pJeu->ProbCoupe[c] ))
                        + SansAtoutPreneur*(20.0 + pJeu->NbCouleur[c] + 6.0*IsMaitreCouleur(CurrentGame, pJeu, c));
			if ( SansAtoutPreneur > 0.5 && IsMaitreCouleur(CurrentGame, pJeu, c) )
			{
                //  Dans ce cas (peu d'atout preneur et maître) joue la plus forte
				CarteCouleur[c] = GetPlusForte(pJeu, c);
			}
			else if ( pJeu->ProbCoupe[c] < 0.7 )
            {
                //  Idem si Proba de coupe preneur raisonnable
				CarteCouleur[c] = GetPlusForte(pJeu, c);
            }
			else
				CarteCouleur[c] = GetPlusFaible(pJeu, c);
		}
        //	Si couleur ouverte par partenaire. Bien, mais pas trop pour ne pas faire tomber
        //	tous les points.
        //	Suit à priori, surtout si couleur ouverte par main forte
		else if ( pJeu->JoueurCouleur[c] != pJeu->PositionPreneur )
		{
            //	Si Coupe du preneur
            //
            //	Val = K6 + K7*CoutCoupe + K8*CoupeAvantPreneur + K9*CoupeApresPreneur + K10*ValPlusFaible
			if ( pJeu->ProbCouleur[c] == 0.0 )
			{
				//  Si Atout côté preneur, plutôt bon, sauf si cela coûte cher. A priori obéit à la main forte
				ValCouleur[c] += (1.0 -SansAtoutPreneur) * ( 9.0 + - 2.5 * CoutCoupe(pJeu, c) + 10.0*pJeu->Obeissance[pJeu->JoueurCouleur[c]]*pJeu->ValFlanc[c] );
				//  Si Atout preneur, pas mauvais si coupe avant le preneur (qui devra monter)
				ValCouleur[c] += (1.0 -SansAtoutPreneur)* 2.0 * ProbCoupeAvant;
				//  Et encore meilleur si coupe après le preneur
				ValCouleur[c] += (1.0 -SansAtoutPreneur)* 4.0 * ProbCoupeApres;
				//  Si pas 'd'atout chez le preneur, la stratégie change. Il faut jouer les longues avec cartes maîtresses
				ValCouleur[c] += SansAtoutPreneur * (20.0 + pJeu->NbCouleur[c] + 6.0 *IsMaitreCouleur(CurrentGame, pJeu, c));
				ValCouleur[c] += SansAtoutPreneur * 4.0 * HasBarrage(CurrentGame, pJeu, c);
				//  Si petit pas tombé et en défense, fait très attention à ne pas jouer cette couleur si risque important de coupe du défenseur
				if ( IsJoue(CurrentGame, 1) == 0 && JoueurAvecPetit(CurrentGame, pJeu) != pJeu->PositionPreneur
                    && ProbCoupeJoueur(pJeu, JoueurAvecPetit(CurrentGame, pJeu), c) > 0.9 )
					ValCouleur[c] -= (1 - SansAtoutPreneur) * 25.0 * ProbCoupeJoueur(pJeu, JoueurAvecPetit(CurrentGame, pJeu), c);
				CarteCouleur[c] = GetPlusFaible(pJeu, c);
				//	Si plus de cartes dans cette couleur, plutôt bon si le preneur a le petit (moins long que le joueur) et que les autres ont de l'atout
				if ( NbReste(CurrentGame, pJeu->PositionJoueur, c) == 0 && ProbCoupeAvant * ProbCoupeApres > 0.75 )
				{
					ValCouleur[c] = (pJeu->NbAtout - pJeu->AtoutPreneur) * ( 1 - (JoueurAvecPetit(CurrentGame, pJeu) == pJeu->PositionPreneur));
				}
				//	Ajoute des points si seule couleur ouverte par la défense.
				i1 = 0;
				for ( i0 = TREFLE; i0 < NB_COULEUR; i0++)
				{
					if ( pJeu->JoueurCouleur[i0] < 0 ) continue;
					if ( pJeu->JoueurCouleur[i0] == pJeu->PositionPreneur ) continue;
					i1++;
				}
				if ( i1 == 1 )
				{
					ValCouleur[c] += 3.0 + 8.0*(pJeu->JoueurCouleur[c] == pJeu->PositionJoueur) + 0.75*PosPreneurM1*PosPreneurM1;
				}
			}
            //	Sinon (pas de coupe du preneur)
            //	Val = K11 + K12*PointsRestants(i) + K13*ValPlusFaible(i)
			else
			{
			    //  Tout d'abord obéit quand même si ouvreur main forte
				ValCouleur[c] += (1.0 - SansAtoutPreneur ) * ( -4.0 + 20.0 * pJeu->Obeissance[pJeu->JoueurCouleur[c]]*pJeu->ValFlanc[c]);
				//  Retire des points si le preneur possède la carte maîtresse et qu'il reste des points à la défense
				ValCouleur[c] += (1.0 - SansAtoutPreneur ) * (0.5 - pJeu->TmpProbCarte[pJeu->PositionPreneur][CarteMaitreNonJouee(CurrentGame, c)])*CalcPointRestantAJouer(CurrentGame, c);
                // Retire des points si plus de cartes en dessous du valet
                ValCouleur[c] += (1.0 - SansAtoutPreneur ) * (-2.0 * GetCartePlusFaible(pJeu, c)->Valeur - 2.0 * (pJeu->NbCoupeInit >= 2)  );
                //  Si le preneur n'a plus d'atout, favorise si maître
				ValCouleur[c] += SansAtoutPreneur * ( -8.0 + pJeu->NbCouleur[c] + 14.0 * IsMaitreCouleur(CurrentGame, pJeu, c)
								- AvgLongueur(pJeu, pJeu->PositionPreneur, c) + 10.0 * HasBarrage(CurrentGame, pJeu, c));
			}
		}
        //	Ouverte par le preneur. Ne joue pas dedans, sauf si possède petit
		else
		{
            //	Si Coupe du preneur
			if ( pJeu->ProbCouleur[c] == 0.0 )
			{
			    //  Retire des points si coût coupe élevé, ajoute si coupe avant / après par la défense
				ValCouleur[c] += (1.0 - SansAtoutPreneur ) * (-3.0 - 2.5 * CoutCoupe(pJeu, c) + 0.8 * ProbCoupeAvant + 2.0 * ProbCoupeApres);
				//  Si pas d'atout preneur, favorise les longues
				ValCouleur[c] += SansAtoutPreneur * ( 20.0 + pJeu->NbCouleur[c] + 6.0*IsMaitreCouleur(CurrentGame, pJeu, c)
                                         + 4.0 * HasBarrage(CurrentGame, pJeu, c) );
			}
            //	Si Petit à la défense (et pas coupe preneur)
			else if ( IsJoue(CurrentGame, 1) == 0 && JoueurAvecPetit(CurrentGame, pJeu) != pJeu->PositionPreneur )
			{
			    //  Atout preneur : bon si avant le preneur, pas bon du fond (risque de coupe/surcoupe quand même)
				ValCouleur[c] += (1.0 - SansAtoutPreneur ) * (-6.0 - 3.0 * PosPreneurM1 + 0.2 * CalcPointRestantAJouer(CurrentGame, c));
				//  Et retire des points si honneur
				ValCouleur[c] += (1.0 - SansAtoutPreneur ) * (-2.0 * GetCartePlusFaible(pJeu, c)->Valeur );
				//  Plus d'atout preneur : bon si maître
				ValCouleur[c] += SansAtoutPreneur * ( -8.0 + pJeu->NbCouleur[c] + 14.0 * IsMaitreCouleur(CurrentGame, pJeu, c)
						- AvgLongueur(pJeu, pJeu->PositionPreneur, c) - 10.0 +  10.0 * HasBarrage(CurrentGame, pJeu, c));
			}
            //	Sinon, pas coupe preneur, petit côté preneur
			else
			{
			    //  Si atout preneur, enlève des points du fond si cela risque de coûter assez cher
				ValCouleur[c] += (1.0 - SansAtoutPreneur ) * (-15.0 - 4.0 * PosPreneurM1 + 0.2 * CalcPointRestantAJouer(CurrentGame, c) );
				//  Retire des points si reste que des honneurs
				ValCouleur[c] += (1.0 - SansAtoutPreneur ) * (-2.0 * GetCartePlusFaible(pJeu, c)->Valeur );
				//  Plus d'atout preneur, OK si maître et long...
				ValCouleur[c] += SansAtoutPreneur * ( -8.0 + pJeu->NbCouleur[c] + 14.0 * IsMaitreCouleur(CurrentGame, pJeu, c)
						- AvgLongueur(pJeu, pJeu->PositionPreneur, c) - 10.0 +  10.0 * HasBarrage(CurrentGame, pJeu, c) );
			}
		}
        //  Si le joueur possède le petit, change les règles
        //  Joue dans une courte si cela a du sens (peu d'atouts, proba de coupe faible par le preneur)
        if ( HasPetit(pJeu) )		//	Possède le Petit
        {
            if ( pJeu->NbCouleur[c] <= 2 )	//	Courte ?
            {	//	Attention, il faut qu'il en reste suffisamment en jeu pour marcher...
                if ( pJeu->AtoutPreneur > pJeu->NbAtout && pJeu->NbAtout <= 4 && NbReste(CurrentGame, pJeu->PositionJoueur, c) >= 2*(pJeu->NbCouleur[c]+1))
                {	//	Danger PETIT ? oui, joue plutôt la courte pour couper ensuite
                    ValCouleur[c] += (10.0  + 2.0 * pJeu->AtoutPreneur - 1.25*pJeu->NbCouleur[c] + 0.5*NbReste(CurrentGame, pJeu->PositionJoueur, c)) * ( 1.0 - pJeu->ProbCoupe[c]);
                }
            }
            //	Longue ou coupe "sûre" du preneur.
            if ( (pJeu->NbCouleur[c] >= 4 || pJeu->ProbCoupe[c] >= 0.8) && (pJeu->AtoutPreneur >= pJeu->NbAtout) )
                ValCouleur[c] += 25.0 - 1.0 * pJeu->ResteAtout + 10.0 * pJeu->ProbCoupe[c];
        }
#if DEBUG_DEFENSE_PREMIER > 0
        OutDebug("Valeur couleur %d: %.2f\n", c, ValCouleur[c]);
#endif // DEBUG_DEFENSE_PREMIER
	}
    //	Cas de l'atout maintenant
    //	Si Chasse lancée et possède de l'atout, en joue.
	if ( pJeu->StatusChasse >= CHASSE_DEFENSE_JOUE_21 )
	{
		i0 = MinLongueur(pJeu, pJeu->PositionPreneur, ATOUT);
		if ( pJeu->NbAtout < i0 && NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT) < 2*i0 && pJeu->StatusChasse == CHASSE_DEFENSE_IMPAIR)
			pJeu->StatusChasse = CHASSE_DEFENSE_PAIR;     //  Plus trop de chances de le prendre, baisse valeur
		if ( NbBoutDefense(CurrentGame, pJeu->PositionJoueur) == 2 ) i0 += 3;
		if ( pJeu->StatusChasse == CHASSE_DEFENSE_IMPAIR )  //  Continue la chasse a priori si fort
			ValCouleur[ATOUT] = 25.0;
		if ( pJeu->StatusChasse == CHASSE_DEFENSE_PAIR )
		{
			ValCouleur[ATOUT] = 8.0;
			if ( i0  <= pJeu->NbAtout || NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT) >= 2*i0 + pJeu->NbAtout)
				ValCouleur[ATOUT] = 14.0;
		}
		//  Baisse valeur si pas sur que le preneur possède le petit...
		ValCouleur[ATOUT] *= pJeu->TmpProbCarte[pJeu->PositionPreneur][1];
#if DEBUG_DEFENSE_PREMIER > 0
        OutDebug("Valeur ATOUT chasse lancée: %.2f\n", ValCouleur[ATOUT]);
#endif // DEBUG_DEFENSE_PREMIER
	}
	else if ( pJeu->StatusChasse == 0 )
	{
    //	Si Chasse pas encore lancée, et possède de l'atout lance éventuellement
    //	Condition lancement chasse :
    //	Le preneur n'a pas trop d'atout au chien (ou dans sa poignée)
    //	Le contrat est faible
    //	Le défenseur n'a pas de coupe
    //	Le défenseur possède beaucoup d'atouts
    //	Le défenseur est loin du preneur

		ValCouleur[ATOUT] = 0;
		if ( pJeu->TmpProbCarte[pJeu->PositionPreneur][1] >= 0.3 && pJeu->NbAtout > 0 )
		{
			if ( HasCarte(pJeu, pJeu->PositionPreneur, 1))
			{
				Chasse += 5.0;	//	Sur qu'il a le petit ?
			}
			else
			{
			    //  Si juste devant preneur avec 19 et 20 joue le 20
				if ( HasCarte(pJeu, pJeu->PositionJoueur, 20) && HasCarte(pJeu, pJeu->PositionJoueur, 19) && PosPreneurM1 == 0 )
				{
					pJeu->StatusChasse = CHASSE_DEFENSE_PAIR;
					PoseCarte(CurrentGame, GetPlusForte(pJeu, ATOUT));
					return;
				}
			}
			i1 = JoueurAvecPetit(CurrentGame, pJeu);
			if ( i1 != pJeu->PositionPreneur )
				Chasse += (pJeu->TmpProbCarte[pJeu->PositionPreneur][1] - pJeu->TmpProbCarte[i1][1]) * 40;
            //  Compte les atouts surs du preneur (vus au chien ou poignée...)
			for ( i1 = 1; i1 < 22; i1++)
			{
				if ( HasCarte(pJeu, pJeu->PositionPreneur, i1) ) NbAtoutSurPreneur++;
			}
			//  Compte ensuite les coupes du joueur. Idée va moins à la chasse si coupe, proba prendre le petit assez faible
			for ( i1 = TREFLE; i1 < NB_COULEUR; i1++)
			{
				if ( pJeu->NbCouleur[i1] == 0 ) NbCoupe++;
			}
			Chasse -= NbAtoutSurPreneur*5.0;
			Chasse -= CurrentGame->TypePartie*3;
			Chasse -= NbCoupe * 5;
			if ( NbCoupe && pJeu->NbAtout <= 5 ) Chasse -= 4.0 + 2.0*(6 - pJeu->NbAtout) ;
			if ( pJeu->NbAtout >= 7 ) Chasse += 10 + (pJeu->NbAtout - 7) * 5;
			Chasse += PosPreneurM1;           //  Meilleur du fond si pas trop sûr
			Chasse += StyleJoueur[pJeu->PositionJoueur][dStyleDefChasse] * 10;
			if ( Chasse > 0 )		//	Essaie la chasse
			{
				ValCouleur[ATOUT] = 5.0 * Chasse + 10.0;
				GoChasse = 1;
			}
		}
#if DEBUG_DEFENSE_PREMIER > 0
        OutDebug("Valeur ATOUT chasse non lancée: %.2f\n", ValCouleur[ATOUT]);
#endif // DEBUG_DEFENSE_PREMIER
	}
	if ( IsMaitreCouleur(CurrentGame, pJeu, ATOUT) )		//	Possède ATOUT Maître
	{
    //	Joue surtout si les partenaires ne possèdent pas d'atout...
		va = 0.0;
		for ( i = 0; i < 4; i++)
		{
			if ( i == pJeu->PositionPreneur ) continue;
			if ( i == pJeu->PositionJoueur ) continue;
			if ( pJeu->NbDefausse[i] < 5 )		//	Ajoute les possibilités de défausse
			{
				x = 100.0*pow(0.5 - GetProbAtout(pJeu, i), 3.0) * (5.0 - pJeu->NbDefausse[i]);
				if ( x > 0.0 ) x = sqrt(x);
				if ( x < 0.0 ) x *= 2.0 * pJeu->NbAtout / (pJeu->ResteAtout - pJeu->AtoutPreneur);
				va += x;
			}
		}
		if ( ((pJeu->PositionJoueur - pJeu->PositionPreneur)&3) == 1 )	//	Juste après le preneur, retire des points
			va -= 10.0;
		ValAtoutFort = va;
	}
	if ( pJeu->NbAtout == 0 || (pJeu->NbAtout == 1 && HasCarte(pJeu, pJeu->PositionJoueur, 0)))
	{
		ValCouleur[ATOUT] = -100000;
	}
	else if ( SansAtoutPreneur > 0.75 )
	{
//	Si le preneur n'a plus d'atout, pas très bon d'en jouer...
//	A FAIRE : JOUE ATOUT A LA FIN POUR NE PAS AVOIR LA MAIN SI COULEUR DU PRENEUR
		i0 = 0;
		i1 = 0;
		for ( c = TREFLE; c < NB_COULEUR; c++ )
		{
			if ( pJeu->NbCouleur[c] == 0 ) continue;
			if ( pJeu->ProbCouleur[c] < 1e-5 ) i0++;
			if ( AvgLongueur(pJeu, pJeu->PositionPreneur, c) > 1.0 ) i1++;
		}
		if ( i1 > i0 && i1 > 1 )	//	Joue atout si possède longue preneur.
			ValCouleur[ATOUT] = 10.0 * 2.0/pJeu->NbCarte;
		else
			ValCouleur[ATOUT] = -10.0;
	}
	else
	{
		if ( HasCarte(pJeu, pJeu->PositionJoueur, 1 ) && (pJeu->AtoutPreneur >= pJeu->NbAtout - 1 || pJeu->ResteAtout >= pJeu->NbAtout ))
			ValCouleur[ATOUT] = -100;
		else if ( HasCarte(pJeu, pJeu->PositionJoueur, 1 ) )
			ValCouleur[ATOUT] = -10;
		else if ( IsJoue(CurrentGame, 1) == 0 )
		{
        //	Pas de chasse au petit a priori. Très mauvais si le petit est à la défense.
			ValCouleur[ATOUT] += -30.0 + 25.0 * pJeu->TmpProbCarte[pJeu->PositionPreneur][1] + GoChasse * 10.0 + 2.0*PosPreneurM1*PosPreneurM1;
		}
//	Si possède plus d'atout que le preneur, joue plutôt atout
		else if ( pJeu->NbAtout > pJeu->AtoutPreneur + 0.75 )
		{
			ValCouleur[ATOUT] += 15.0 / sqrt(pJeu->NbAtout);
			if ( IsMaitreCouleur(CurrentGame, pJeu, ATOUT) )		//	Possède ATOUT Maître
				ValCouleur[ATOUT] += 5.0;
			i0 = 0;
			i1 = 0;
			for ( c = TREFLE; c < NB_COULEUR; c++ )
			{
				if ( pJeu->NbCouleur[c] == 0 ) continue;
				if ( pJeu->ProbCouleur[c] < 1e-5 ) i0++;			//	Coupe preneur
				if ( AvgLongueur(pJeu, pJeu->PositionPreneur, c) > 1.0 ) i1++;	//	Longue preneur
			}
			if ( i1 > i0 && i1 > 1 )
			{
			    //  Si longues preneur > coupes preneur : ajoute des points par coupe et retire par longue
				if ( NbPossCouleur(pJeu, ATOUT) > 1 )
					ValCouleur[ATOUT] -= 3.0*i1 + 5*i0;
			}
		}
		else if ( PosPreneurM1 == 0 )		//	Juste avant preneur, essaie 2 pour un
		{
		    va = DeuxPourUnOK(CurrentGame, pJeu);
			if ( va > -10 )
			{
				CarteCouleur[ATOUT] = GetPlusFaible(pJeu, ATOUT);
				ValCouleur[ATOUT] = va;

			}
		}
		else if ( PosPreneurM1 == 1 )		//	Milieu, essaie également 2 pour un
		{
		    va = DeuxPourUnOK(CurrentGame, pJeu);
			if ( va > -10 )
			{
				CarteCouleur[ATOUT] = GetPlusFaible(pJeu, ATOUT);
				ValCouleur[ATOUT] = va;
				ValCouleur[ATOUT] -= 10.0 * (1.0 - pJeu->ForceMain[(pJeu->PositionJoueur+3)&3]);
			}
		}
		else if ( pJeu->NbAtout <= 2 && pJeu->ResteAtout > 10 && MinLongueur(pJeu, pJeu->PositionPreneur, ATOUT) < 6)
		{
            //	Joue atout pour se débarrasser, aide à sauver des points après
			ValCouleur[ATOUT] += -5.0 + 0.33 * pJeu->ResteAtout - pJeu->NbAtout
				+ (ComptePoints(pJeu) - pJeu->NbCarte)*0.3;		//	Aide a sauver les points
		}
		else if ( pJeu->ResteAtout > 11 && pJeu->AtoutPreneur < pJeu->ResteAtout/2.5)
		{
			va = pJeu->AtoutPreneur - pJeu->NbAtout + 2.0 * PosPreneurM1;
			if ( pJeu->JoueurMainForte >= 0 )
				va += PosPreneurM1;
			ValCouleur[ATOUT] += va;
		}
		else if ( pJeu->ValFlanc[ATOUT] > 0.25 )
		{
			ValCouleur[ATOUT] += pJeu->ValFlanc[ATOUT] * 5.0;
		}
		else
		{
			va = pow(pJeu->NbAtout - pJeu->AtoutPreneur - 0.75, 3.0);	//	Premier facteur tient compte du fait que plus d'atout que preneur
			p0 = pJeu->ResteAtout - (NbPossCouleur(pJeu, ATOUT)+1.0)*pJeu->AtoutPreneur;	//	Aide des autres partenaires ?
			va += 3.0 * atan(2.0*p0);
			if ( va < 0.0 )
				va += 8.0*(0.5-pJeu->TmpProbCarte[pJeu->PositionPreneur][CarteMaitreNonJouee(CurrentGame, ATOUT)]);
			if ( fabs(pJeu->AtoutPreneur - pJeu->NbAtout) < 0.1 && pJeu->TmpProbCarte[pJeu->PositionPreneur][CarteMaitreNonJouee(CurrentGame, ATOUT)] > 0.6 )
			{
			    //  Compte le nombre de pli minimum à faire par le preneur
				i0 = MinPlisPreneur(CurrentGame, pJeu) - 1;
				//  Si positif, pas intérêt à jouer ATOUT, cela renforce plutôt le preneur
				va -= i0 * 10.0;
				for ( i1 = TREFLE; i1 < NB_COULEUR; i1++)
				{
					va -= (15.0 + 5.0/pJeu->NbAtout) * ForcePreneurCouleur(CurrentGame, pJeu, i1);
				}
			}
			ValCouleur[ATOUT] += va;
		}
		if ( ValCouleur[ATOUT] > -50.0 && ValAtoutFort > ValCouleur[ATOUT] )
		{
			ValCouleur[ATOUT] = ValAtoutFort;
			CarteCouleur[ATOUT] = GetPlusForte(pJeu, ATOUT);
		}
	}
	if ( pJeu->NbAtout > 0 && pJeu->Flanc == ATOUT
        && ValCouleur[ATOUT] > -100.0 - 100.0*pJeu->ProbCarte[pJeu->PositionPreneur][1]/pJeu->NbAtout
                                - 50.0*NbGrosAtout(CurrentGame, pJeu, pJeu->PositionPreneur,4)*pJeu->AtoutPreneur)
	{
		if ( ! HasPetit(pJeu) )	//	Possède Petit ??
			ValCouleur[ATOUT] += 15.0 + fabs(ValCouleur[ATOUT]);
		if ( IsMaitreCouleur(CurrentGame, pJeu, ATOUT) )
		{
			ValCouleur[ATOUT] += 5.0;
			if ( IsJoue(CurrentGame, 1) == 0 )
				CarteCouleur[ATOUT] = GetPlusForte(pJeu, ATOUT);
		}
	}
#if DEBUG_DEFENSE_PREMIER > 0
    OutDebug("Valeur finale ATOUT : %.2f\n", ValCouleur[ATOUT]);
#endif // DEBUG_DEFENSE_PREMIER
//	Possibilité de jouer l'Excuse...
	if ( !HasCarte(pJeu, pJeu->PositionJoueur, 0) )
		ValCouleur[EXCUSE] = -100000;
	else if ( pJeu->NbCarte == 3 )
		ValCouleur[EXCUSE] = 10;
	else if ( pJeu->NbCarte == 2 )
		ValCouleur[EXCUSE] = 50;
	else
		ValCouleur[EXCUSE] = -50.0;
//	Choix de la couleur : on prend celle pour qui ValeurCouleur est maximale
	i0 = 0;
	BestScore = -1000000;
	for ( c = 0; c < NB_COULEUR; c++)
	{
		if ( ValCouleur[c] > BestScore )
		{
			i0 = c;
			BestScore = ValCouleur[c];
		}
	}
    //	Choix de la carte à jouer dans la couleur
	//	Si Excuse, pas de problème...
	if ( i0 == EXCUSE )
	{
		PoseCarte(CurrentGame, 0);
		return;
	}
	//	Si Atout, deux cas
	if ( i0 == ATOUT && CarteCouleur[ATOUT] >= 0 )
	{
		PoseCarte(CurrentGame, CarteCouleur[ATOUT]);
		return;
	}
	//	Si va a la chasse démarre atout impair
	if ( GoChasse && pJeu->NbAtout >= 6)
	{
		pJeu->StatusChasse = CHASSE_DEFENSE_IMPAIR;
		if ( FlagSignalisation )
		{
			i = GetPlusFaible(pJeu, ATOUT);
			while ( pJeu->MyCarte[i].Couleur == ATOUT && (pJeu->MyCarte[i].Hauteur & 1) == 0 )	//	Pair
			{
				i++;
			}
			if ( pJeu->MyCarte[i].Couleur == ATOUT )
			{
				PoseCarte(CurrentGame, i);
				return;
			}
		}
		PoseCarte(CurrentGame, GetPlusFaible(pJeu, ATOUT));
		return;
	}
	//	1)	Juste avant le preneur ou possède Atout maître : joue le plus gros
	//	2)	Autre position : joue le plus petit.
	if ( i0 == ATOUT && (PosPreneurM1 == 0 || IsMaitreCouleur(CurrentGame, pJeu, ATOUT)) )
	{
		if ( pJeu->ResteAtout - pJeu->AtoutPreneur > NbPossCouleur(pJeu, ATOUT) 	//	Ne joue pas le plus gros si il n'en reste qu'un (gros ?) aux partenaires
			&& ( ValAtoutFort == ValCouleur[ATOUT] ))
		{
			PoseCarte(CurrentGame, GetPlusForte(pJeu, ATOUT));
			return;
		}
	}
	if ( i0 == ATOUT )
	{
		i0 = GetPlusForte(pJeu, ATOUT);
		if ( pJeu->NbAtout == 2 && pJeu->MyCarte[i0].Hauteur > 17 && !HasCarte(pJeu, pJeu->PositionPreneur, 0) && pJeu->ResteAtout - pJeu->AtoutPreneur > NbPossCouleur(pJeu, ATOUT) )
		{
			PoseCarte(CurrentGame, GetPlusForte(pJeu, ATOUT));
			return;
		}
		if ( pJeu->NbAtout > pJeu->AtoutPreneur + 0.75 && NbAtoutMaitre(CurrentGame, pJeu->PositionJoueur) > 1 )
		{
			PoseCarte(CurrentGame, i0);
			return;
		}
		i0 = GetPlusFaible(pJeu, ATOUT);
		if ( pJeu->MyCarte[i0].Index == 1 && pJeu->NbCarte > 1)
			i0++;
		PoseCarte(CurrentGame, i0);
		return;
	}
	//	Autre couleur
	//	Si maitre dans la couleur : si Proba coupe assez faible, joue la carte maîtresse
	//  Cas particulier : plus d'atout preneur, et un seul dans la couleur joue son dernier atout pour ne pas bloquer
	if ( pJeu->NbAtout == 1 && !HasExcuse(pJeu) && SansAtoutPreneur > 0.8 && pJeu->NbCouleur[i0] == 1 )	//	Joue d'abord ATOUT...
	{
		i0 = GetPlusForte(pJeu, ATOUT);
		PoseCarte(CurrentGame, i0);
		return;
	}
	if( IsMaitreCouleur(CurrentGame, pJeu, i0) && (FlagSignalisation  || pJeu->NbEntame[pJeu->PositionJoueur] != 1 || pJeu->JoueurCouleur[i0] >= 0))
	{
		i1 = GetPlusForte(pJeu, i0);		//	Plus forte carte dans la couleur
		if ( SansAtoutPreneur > 0.5 || pJeu->ProbCoupe[i0]*pJeu->MyCarte[i1].Valeur + PosPreneurM1*0.1 + pJeu->NbCouleur[i0]*0.1 < 3.6 )
		{
			PoseCarte(CurrentGame, i1);
			return;
		}
		i = CarteMaitreNonJouee(CurrentGame, ATOUT);
		for ( int p1 = PosPreneurM1+2; p1 < 4; p1++)
		{
			if ( pJeu->TmpProbCarte[(p1+pJeu->PositionJoueur)&3][i] > 0.9 && pJeu->MyCarte[i1].Valeur >= 8 )
			{
				PoseCarte(CurrentGame, i1);
				return;
			}
		}
	}
	if ( CarteCouleur[i0] >= 0 )
	{
		PoseCarte(CurrentGame, CarteCouleur[i0]);
		return;
	}
	//	Si ouverture et Signalisation, cas particulier
	if ( FlagSignalisation && pJeu->JoueurCouleur[i0] < 0 )
	{
		i1 = GetPlusFaible(pJeu, i0);
		//  Si ni ROI ni DAME, jour du 6 au 10
		if ( HasCarte(pJeu, pJeu->PositionJoueur, Startof[i0]+ROI-1) == 0 &&  HasCarte(pJeu, pJeu->PositionJoueur, Startof[i0]+DAME-1) == 0 && pJeu->MyCarte[i1].Hauteur <= 5)
		{
			while ( i1 < (pJeu->NbCarte - 1) && pJeu->MyCarte[i1].Hauteur <= 5 && pJeu->MyCarte[i1].Couleur == i0) i1++;
			if ( pJeu->MyCarte[i1].Hauteur >= VALET && i1 > 0 ) i1--;
			if ( pJeu->MyCarte[i1].Couleur != i0 && i1 > 0 ) i1--;
		}
		if ( pJeu->MyCarte[i1].Hauteur > 5 )
		{
			PoseCarte(CurrentGame, i1);
			return;
		}
	}
	//	Sinon joue plus petite dans la couleur
	i1 = GetPlusFaible(pJeu, i0);
	i = GetPlusForte(pJeu, i0);
		//	Cas maître
	if ( pJeu->MyCarte[i].Hauteur >= VALET && IsMaitreCouleur(CurrentGame, pJeu, i0) && pJeu->ProbCoupe[i0] < 0.9 - FlagSignalisation*0.3 )
	{
		PoseCarte(CurrentGame, i);
		return;
	}
		//	Cas où ne possède que des grosses, les joue...
	if ( pJeu->MyCarte[i1].Hauteur >= VALET && pJeu->ProbCarte[pJeu->PositionPreneur][CarteMaitreNonJouee(CurrentGame, i0)] < 0.2 && PosPreneurM1 <= 1)
	{
		PoseCarte(CurrentGame, i);
		return;
	}
	PoseCarte(CurrentGame, i1);
}

//	Jeu à l'atout de la défense
//	Retourne l'index de la carte a jouer ou -1 si ne sait pas

int JeuDefenseAtout(TarotGame CurrentGame, struct _Jeu *pJeu)
{
int j;
int IndexTable = (pJeu->PositionJoueur - CurrentGame->JoueurEntame) & 3;
int h;
int petit;
int i;

    if ( pJeu->NbAtout <= HasExcuse(pJeu) ) return(-1);     //  Pas d'atout !
	if ( CurrentGame->JoueurEntame == pJeu->PositionPreneur && HasPetit(pJeu) )
	{	//	Possède le petit et le preneur joue atout
		j = 0;      //  Compte les atouts non joués
		for ( i = Table[0].Hauteur+1; i < 22; i++)
		{
			if ( CurrentGame->CarteJouee[i] < 0 ) j++;
		}
		if ( j >= pJeu->NbAtout )
		{
			if ( Table[0].Hauteur == CarteMaitreNonJouee(CurrentGame, ATOUT) )
                return(-1);	//	Rien a espérer, l'atout joué est maître, le programme "normal" donnera une solution
			if ( Gagnant(IndexTable-1) != 0 ) return(-1);		//	Preneur pas maître revient au cas normal
			if ( IndexTable == 3) return(-1);	                //	Joue en dernier, cas normal
			j = GetPlusFaible(pJeu, ATOUT);	                    //	Espère que cela sera bon
			if ( CarteValide(pJeu, j, CurrentGame->JoueurEntame) )
                return(j);          //  Si cette carte est valide (Petit), la joue
		}
		if ( MinLongueur(pJeu, pJeu->PositionPreneur, ATOUT) > pJeu->NbAtout )
		{   //  Le preneur a encore pas mal d'atout, ce sera dur de sauver le petit
		    //  Essaie de voir combien d'atouts maîtres peuvent être chez le preneur
			j = pJeu->NbAtout;
			for ( i = 0; i < j; i++)
			{
				if ( CurrentGame->CarteJouee[21-i] < 0 || HasCarte(pJeu, pJeu->PositionJoueur, 21-i) )
				{
					j++;
					continue;
				}
				if ( pJeu->ProbCarte[pJeu->PositionPreneur][21-i] < 0.8 ) break;
			}
			if ( i >= j )	//	Choisit de résister...
			{
				if ( Gagnant(IndexTable-1) != 0 ) return(-1);		//	Preneur pas maître...
				if ( HasExcuse(pJeu) ) return(0);			    //	Joue l'excuse si la possède
				if ( CarteValide(pJeu, 1, CurrentGame->JoueurEntame) && pJeu->NbAtout > 3)	//	Peut jouer cette carte
					return(1);
				j = 0;
				for ( i = 0; i < 4; i++ )
				{	//	Teste si les partenaires ont de l'atout
					if ( i == pJeu->PositionPreneur ) continue;
					if ( i == pJeu->PositionJoueur) continue;
					if ( AvgLongueur(pJeu, i, ATOUT) < 0.001) j++;
				}
				if ( pJeu->NbAtout == 2 && j <= 1 && CarteValide(pJeu, 1, CurrentGame->JoueurEntame) )
					return(1);
				if ( NbAtoutMaitre(CurrentGame, pJeu->PositionJoueur) > 1 ) return(GetPlusForte(pJeu, ATOUT));
				return(-1);
			}
		}
	}
	//	Si possède l'excuse et le petit, la met pour arrêter la chasse...
	if ( HasExcuse(pJeu) && HasPetit(pJeu) )
	{
		return(0);
	}
	//	Si possède très peu d'atout, joue les gros en premier
	if ( CurrentGame->JoueurEntame != pJeu->PositionPreneur && pJeu->NbAtout <= 3
        && (NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT) >= 15 - IndexTable)
        && CurrentGame->CarteJouee[1] < 0 && !HasPetit(pJeu) )
	{
		return(GetPlusForte(pJeu, ATOUT));
	}
	//	Si chasse lancée, joue les gros pour ne pas la bloquer
	if ( pJeu->StatusChasse == CHASSE_DEFENSE_IMPAIR && pJeu->NbAtout == 2 )
	{
		return(GetPlusForte(pJeu, ATOUT));
	}
	//	Si le preneur attaque ATOUT et possède le 21 juste derrière, ne le met pas...
	if ( CurrentGame->JoueurEntame == pJeu->PositionPreneur && HasCarte(pJeu, pJeu->PositionJoueur, 21) && !HasPetit(pJeu) && IndexTable == 1 )
	{
		h = GetPlusForte(pJeu, ATOUT);
        //  monte "à la pointure", cherche le premier atout qui convient
		for ( j = h; pJeu->MyCarte[j].Couleur == ATOUT; j--)
		{
			if ( CarteValide(pJeu, j, CurrentGame->JoueurEntame) ) continue;
			break;
		}
		return(j+1);
	}
	//	Si le petit est sur la table, met le plus fort...
	h = 0;
	petit = 0;
	for ( j = 0; j < IndexTable; j++)
	{
		if ( Table[j].Index == 1 ) petit = 1;
		if ( Table[j].Couleur == ATOUT  && Table[j].Index > h ) h = Table[j].Index;
	}
	if ( petit )
	{
		j = GetPlusForte(pJeu, ATOUT);
		if ( j >= 0 && pJeu->MyCarte[j].Index > h ) return(j);
	}
	return(-1);
}


//  Cas spécial coupe défense
//  Si défenseur court en atout et coupe dans la longue du preneur, joue les forts d'abord pour éviter de faire monter les partenaires
//  Retourne la carte à jouer si cas spécial, sinon retourne -1, le cas général s'applique alors

int JoueCoupeFortEnTete(TarotGame CurrentGame, struct _Jeu *pJeu)
{
int j;
double pnc, pf;
double val = 1.0;
int idx;

    if ( HasPetit(pJeu) ) return -1;        //  Pas avec le petit !
    if ( pJeu->NbAtout <= HasExcuse(pJeu) ) return -1;      //  Pas d'atout !
    if ( CurrentGame->JoueurEntame != CurrentGame->JoueurPreneur ) return -1;       //  Seulement si entame preneur
    if ( pJeu->NbAtout > 4 ) return -1;         //  Ne doit pas avoir beaucoup d'atouts
    if ( JoueurAvecPetit(CurrentGame, pJeu) != pJeu->PositionPreneur )  //  Si petit pas chez preneur, ne le fait pas
    if ( AvgLongueur(pJeu, pJeu->PositionPreneur, Table[0].Couleur) < 2 )
        return -1;                  //  Seulement si longue preneur
    idx = GetPlusForte(pJeu, ATOUT);
    //  L'idée est de jouer les gros si pas de coupe derrière ET proba d'avoir plus gros importante
    pnc = 1.0;                   //  Proba NON coupe
    pf = 1.0;                   //  Proba d'avoir plus fort
    j = (pJeu->PositionJoueur + 1) & 3;
    if ( j == CurrentGame->JoueurPreneur ) return -1;       //  Pas si juste avant le preneur (fond)
    //  Calcule proba de coupe des partenaires derrière
    while ( j != CurrentGame->JoueurPreneur )
    {
        pnc *= (1.0 - ProbCoupeJoueur(pJeu, j, Table[0].Couleur));
        pf *= ProbaCarteSup(pJeu, j, ATOUT, pJeu->MyCarte[idx].Index);
        val *= pf * pnc;        //  Si val devient trop faible cela ne vaut pas la peine
        j = (j + 1) & 3;
    }
    if ( val < 0.5 ) return -1;      //  Si val trop faible ne la fait pas
    return idx;
}

//  Traite les cas spéciaux quand un défenseur doit couper
//  Retourne la carte à jouer si cas spécial, sinon retourne -1, le cas général s'applique alors

int JoueCoupeDefense(TarotGame CurrentGame, struct _Jeu *pJeu)
{
int CBest;

     if ( (CBest = JoueCoupeFortEnTete(CurrentGame, pJeu)) > 0 )
        return CBest;
    return -1;
}

#if DEBUG > 0
#define DEBUG_DEFENSE_SECOND 0
#else
#define DEBUG_DEFENSE_SECOND 0
#endif  // DEBUG



void JoueDefenseSecond(TarotGame CurrentGame)
{
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[CurrentGame->JoueurCourant];
int pos = (pJeu->PositionPreneur - pJeu->PositionJoueur) & 3;
int i0;
int BestCarte = -1;
double BestScore = -1e6;
double Score;
int couleur;
int CBest;

	assert(pJeu->NbCarte > 0);
    CalcTmpProbCarte(CurrentGame);      //  Apprend de la carte jouée auparavant
	pJeu->AtoutPreneur = AvgLongueur(pJeu, CurrentGame->JoueurPreneur, ATOUT);
	pJeu->ResteAtout = NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT);
	GainPetitAuBout(CurrentGame, pJeu);
#if DEBUG_DEFENSE_SECOND > 0
    OutDebug("--\nPosition %d, JoueDefenseSecond après ", pJeu->PositionJoueur);
    ImprimeCarte(&Table[0]);
    OutDebug("\n");
    OutDebug("Proba Petit = %d:%.3f  %d:%.3f  %d:%.3f  Chien:%.3f\n",
             pJeu->PositionJoueur^1, pJeu->TmpProbCarte[pJeu->PositionJoueur^1][1],
             pJeu->PositionJoueur^2, pJeu->TmpProbCarte[pJeu->PositionJoueur^2][1],
             pJeu->PositionJoueur^3, pJeu->TmpProbCarte[pJeu->PositionJoueur^3][1],
             pJeu->TmpProbCarte[4][1]);
    OutDebug("Proba 21 = %d:%.3f  %d:%.3f  %d:%.3f  Chien:%.3f\n",
             pJeu->PositionJoueur^1, pJeu->TmpProbCarte[pJeu->PositionJoueur^1][21],
             pJeu->PositionJoueur^2, pJeu->TmpProbCarte[pJeu->PositionJoueur^2][21],
             pJeu->PositionJoueur^3, pJeu->TmpProbCarte[pJeu->PositionJoueur^3][21],
             pJeu->TmpProbCarte[4][1]);
    OutDebug("ProbCoupe[T]=%.3f, ProbCoupe[K]=%.3f, ProbCoupe[P]=%.3f, ProbCoupe[C]=%.3f\n",
              pJeu->ProbCoupe[TREFLE], pJeu->ProbCoupe[CARREAU], pJeu->ProbCoupe[PIQUE], pJeu->ProbCoupe[COEUR]);
    OutDebug("GuessProbCoupe[T]=%.3f, GuessProbCoupe[K]=%.3f, GuessProbCoupe[P]=%.3f, GuessProbCoupe[C]=%.3f\n",
          pJeu->GuessProbCoupe[TREFLE], pJeu->GuessProbCoupe[CARREAU], pJeu->GuessProbCoupe[PIQUE], pJeu->GuessProbCoupe[COEUR]);
    OutDebug("CalProbCoupe[T]=%.3f, CalProbCoupe[K]=%.3f, CalProbCoupe[P]=%.3f, CalProbCoupe[C]=%.3f\n",
          pJeu->CalProbCoupe[TREFLE], pJeu->CalProbCoupe[CARREAU], pJeu->CalProbCoupe[PIQUE], pJeu->CalProbCoupe[COEUR]);
    OutDebug("TmpProbCoupe[T]=%.3f, TmpProbCoupe[K]=%.3f, TmpProbCoupe[P]=%.3f, TmpProbCoupe[C]=%.3f\n",
              pJeu->TmpProbCoupe[TREFLE], pJeu->TmpProbCoupe[CARREAU], pJeu->TmpProbCoupe[PIQUE], pJeu->TmpProbCoupe[COEUR]);
    OutDebug("Longueur ATOUT Preneur =%.2f\n", pJeu->AtoutPreneur);
    OutDebug("Status Chasse = %d\n", pJeu->StatusChasse);
    OutDebug("MinPlisPreneur = %d\n", MinPlisPreneur(CurrentGame, pJeu));
    OutDebug("Flanc = %d, ValFlanc = %.3f, CouleurTenue = %d\n", pJeu->Flanc, pJeu->Flanc > 0 ? pJeu->ValFlanc[pJeu->Flanc]:0.0, pJeu->CouleurTenue);
    OutDebug("ProbGain = %.3f\n", ProbGainCoup(CurrentGame, pJeu, 1, 0));
    if ( HasCarte(pJeu, pJeu->PositionJoueur, 1) )
        OutDebug("Valeur petit au bout = %.3f\n", pJeu->ValPetitAuBout);
    if ( Table[0].Couleur == COEUR )
        i0 = 0;
#endif // DEBUG_DEFENSE_SECOND
    MakeCarte(&Table[2], 0);             //  Simule excuse après pour être sûr de ne pas faire le pli.
    MakeCarte(&Table[3], 0);

    //  Atout demandé. Si le joueur en a, décide quel est l'atout le meilleur à jouer
	if (  Table[0].Couleur == ATOUT && pJeu->NbAtout > 0 )
	{
		if ( (CBest = JeuDefenseAtout(CurrentGame, pJeu)) >= 0 )
		{
			PoseCarte(CurrentGame, CBest);
			return;
		}
	}
	//  Parmi les cartes du joueur, détermine celles qui sont valides
	for ( i0 = 0; i0 < pJeu->NbCarte; i0++)
	{
		if ( CarteValide(pJeu, i0, CurrentGame->JoueurEntame) )		//	Peut jouer cette carte ?
		{
			if ( pJeu->MyCarte[i0].Couleur != Table[0].Couleur && Table[0].Couleur != EXCUSE && pJeu->MyCarte[i0].Couleur > ATOUT )
			{
			    //  Défausse du joueur, applique algorithme particulier
				if ( (CBest = JoueDefausseEnSecond(CurrentGame, pJeu)) >= 0 )
				{
                    PoseCarte(CurrentGame, CBest);
					return;
				}
			}
			Table[1] = pJeu->MyCarte[i0];       //  Essaie la carte
			if ( Gagnant(1) == 1 && Table[1].Couleur > ATOUT)
			{
			    //  La carte posée gagne le pli pour le moment
			    //  La fonction JoueDefenseGagnant ne retourne pas forcément une carte, cela peut être -1 si ne sait pas.
				if ( (CBest = JoueDefenseGagnant(CurrentGame, pJeu)) >= 0)
				{
                    PoseCarte(CurrentGame, CBest);
					return;
				}
			}
			//  Cas spéciaux de coupe de la défense
			if ( Table[0].Couleur > ATOUT && Table[1].Couleur == ATOUT )
            {
				if ( (CBest = JoueCoupeDefense(CurrentGame, pJeu)) >= 0)
				{
                    PoseCarte(CurrentGame, CBest);
					return;
				}
            }
			Table[1] = pJeu->MyCarte[i0];       //  Essaie la carte ( a été modifiée par JoueDefenseGagnant)
			//  Cas général, évalue la valeur du coup.
			Score = EvalCoupSecond(CurrentGame, pJeu);
			if ( Table[1].Couleur == ATOUT && Table[1].Index == GetCartePlusForte(pJeu, ATOUT)->Index && HasPetit(pJeu) )
			{		//	Ajoute des points si joue gros ATOUT et possède le PETIT
				switch ( pos )
				{
				case 1:
					Score += 4.0;           //  Juste avant le preneur, OK
					break;
				case 2:
					if ( Table[0].Hauteur > Table[1].Hauteur )
						Score += 4.0;       //  OK si joue en dessous
					else
						Score += 2.0;		//	2 avant le Preneur, pas très bon, le preneur finit le pli
					break;
				case 3:
					if ( Table[0].Hauteur > Table[1].Hauteur && Table[1].Index > 1)
						Score -= 100.0;       //  Ne doit jamais arriver ! Le preneur joue atout, on ne peut monter DOIT jouer le petit
					break;
				}
			}
			//	Correction si coupe avant le preneur sur une entame de la défense
			if ( Table[1].Couleur == ATOUT && Table[0].Couleur > ATOUT && pJeu->JoueurCouleur[Table[0].Couleur ] < 0
				&& CurrentGame->JoueurEntame != CurrentGame->JoueurPreneur )
			{
				if ( IsMaitre(CurrentGame, pJeu, &Table[1]) ) Score -= 5.0;		//	Ne joue pas atout maître tout de suite
			}
#if DEBUG_DEFENSE_SECOND > 0
            OutDebug("Essai carte ");
            ImprimeCarte(&Table[1]);
            OutDebug(" Score = %.2f\n", Score);
#endif // DEBUG_DEFENSE_SECOND
			if ( FlagSignalisation )	//	Signalisation des doubletons ou des tenues en descendant
			{
				couleur = Table[0].Couleur;
				//  Joue la bonne couleur (non Atout), l'entame n'est pas au preneur et c'est l'ouverture de cette couleur
				//  Deux cartes dans la couleur, prime à la plus forte pour la jouer en premier
				if ( couleur > ATOUT && Table[1].Couleur == couleur && pJeu->NbCouleur[couleur] == 2
					&& CurrentGame->JoueurEntame != CurrentGame->JoueurPreneur && pJeu->JoueurCouleur[couleur] < 0
					&& i0 > 0 && pJeu->MyCarte[i0-1].Couleur == couleur )
				{
                    Score += 1.0;       //  Bonus pour cette carte, si pas trop mauvais sera choisie
				}
				//  Si pas doubleton, favorise la plus petite
				if ( couleur > ATOUT && Table[1].Couleur == couleur && pJeu->NbCouleur[couleur] > 2
					&& CurrentGame->JoueurEntame != CurrentGame->JoueurPreneur && pJeu->JoueurCouleur[couleur] < 0
					&& i0 == GetPlusFaible(pJeu, couleur) )
				{
                    Score += 1.0;       //  Bonus pour cette carte, pour ne pas tromper la défense
				}
				//  Si tenue, joue également en descendant
				if ( couleur > ATOUT && Table[1].Couleur == couleur && CurrentGame->JoueurEntame == CurrentGame->JoueurPreneur
					&& pJeu->JoueurCouleur[couleur] < 0 && HasPetit(pJeu) == 0
                    && pJeu->NbCouleur[couleur] >= 5 && GetCartePlusForte(pJeu, couleur)->Hauteur >= CAVALIER
                    && (i0==0 || pJeu->MyCarte[i0-1].Couleur != couleur) )
				{
				    //  Dans ce cas, enlève un point à la première qui ne sera donc pas choisie
					Score -= 1.0;
				}
			}
			if ( Score > BestScore )
			{
				BestScore = Score;
				BestCarte = i0;
			}
		}
	}
	if ( BestCarte >= 0 )
	{
		PoseCarte(CurrentGame, BestCarte);
		return;
	}
	//  "Efface" les cartes jouées virtuellement
	Table[2].Index = -1;
	Table[3].Index = -1;
	assert(0);
}

//	Joue en second mais n'a pas de la couleur demandée
//	Applique un algorithme spécifique pour se défausser
//	Retourne l'index de la carte a joueur si a trouvé une défausse, -1 dans le cas contraire
//	Dans ce cas, le joueur joue "comme avant"

int JoueDefausseEnSecond(TarotGame CurrentGame, struct _Jeu *pJeu)
{
double ProbGain;
int MinPliP;
int i, c;
int Best = -1;
int BestC = -1;
int BestH = -1;
double Bval;
double val;
double x;
double TousUtiles;
double pp = 0;
int HonneurParCouleur[NB_COULEUR];
int TotHonneur;
int JoueAvantPreneur = ((pJeu->PositionPreneur - CurrentGame->JoueurEntame)&3) > 2;
int NbPoint = Table[0].Valeur;


	if ( HasExcuse(pJeu) && pJeu->NbCarte == 2 )
	{	//	Joue l'excuse en avant-dernier !
		return(0);
	}
	MakeCarte(&Table[1], 0);        //  Pose virtuellement l'excuse sur la table
	ProbGain = ProbGainCoup(CurrentGame, pJeu, 1, 0);
	MinPliP = MinPlisPreneur(CurrentGame, pJeu);
	TotHonneur = CompteHonneur(CurrentGame, pJeu->PositionJoueur, HonneurParCouleur);
#if DEBUG_DEFENSE_SECOND > 0
    OutDebug("Défausse 2nd, ProbGain = %.2f, MinPliPreneur = %d\n", ProbGain, MinPliP);
#endif // DEBUG_DEFENSE_SECOND
	//	Cas 1 : défausse quand le pli est perdu
	//	Joue plutôt les petites cartes dans les couleurs coupées par le preneur
	//	Evite de trop défausser dans une couleur pour protéger ses honneurs.
	//	Ne défausse pas dans les couleurs où susceptible de faire des plis.
	//	Défausse éventuellement dans des courtes même non coupées par le preneur pour
	//	pouvoir défausser des grosses ensuite.
	if ( ProbGain < 1e-3 )		//	Pas trop de chances de gagner...
	{
		Bval = -1000000.0;
		if ( JoueAvantPreneur && CurrentGame->CarteJouee[0] < 0 && pJeu->TmpProbCarte[pJeu->PositionPreneur][0] >= 0.5 && TotHonneur >= 1)
		{	//	Le preneur a peut-être l'excuse, met au moins un valet...
			for ( c = 0; c < 4; c++)
			{
				for ( i = 10; i < 13; i++)
				{
					if ( HasCarte(pJeu, pJeu->PositionJoueur, Startof[c] + i) )
					{
						val = 10.0 * pJeu->ProbCoupe[c]
							- (NbPoint + 2.0*(i - 9)*(i-9.0))*(1.0 - pJeu->TmpProbCarte[pJeu->PositionPreneur][0]*( 10.0*pow(1.0/pJeu->NbCarte,3.0) ));
						if ( val > Bval )
						{
							BestC = c ;
							BestH = i + 1;
							Bval = val;
						}
					}
				}
			}
			if ((TotHonneur - pJeu->NbCarte - MinPliP) >= 0)
				Bval += (TotHonneur - pJeu->NbCarte - MinPliP);
			if ( Bval > 0)
			{
				return(Carte2Index(pJeu, BestC, BestH));
			}
		}
		return(DefPerdante(CurrentGame, pJeu));
	}
	// Cas 2 : défausse quand la défense a le pli.
	//	Défausse plutôt des grosses cartes dans les couleurs coupées par le preneur.
	//	Ne défausse pas dans les couleurs où susceptible de faire des plis.
	if ( ProbGain > 0.99 )
	{
		return(DefGagnant(CurrentGame, pJeu));
	}
	//	Cas 3 : Le preneur est sur de faire tous les plis
	if ( pJeu->NbCarte == MinPliP )
		return(-1);
	//	Si fait le pli, ajoute un a MinPliP
	if ( ProbGain < 0.25 )
		MinPliP++;
	//	Cas particulier si joue après le preneur et reste seulement deux cartes...
	if ( CurrentGame->JoueurEntame == pJeu->PositionPreneur && NbReste(CurrentGame, pJeu->PositionJoueur, Table[0].Couleur) == 2 )
	{
		if ( NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT) - pJeu->AtoutPreneur < 0.25 ) MinPliP++;
	}
	//	Cas général, pas trop sur des probabilités
	if ( TotHonneur >= pJeu->NbCarte - MinPliP )		//	Pas trop le choix, il faut en sauver...
	{
	//	Défausse plutôt des grosses cartes dans les couleurs coupées par le preneur.
	//	La valeur des cartes dépend de la probabilité de gain...
	//	Ne défausse pas dans les couleurs où susceptible de faire des plis.
		Bval = -1000000.0;
		if ( pJeu->NbCarte > MinPliP )
			x = 1.0/(pJeu->NbCarte - MinPliP);
		else
			x = 10.0;
		for ( i = 0; i < pJeu->NbCarte; i++)
		{
			c = pJeu->MyCarte[i].Couleur;
			pp = PlisPossibles(CurrentGame, pJeu, c, &TousUtiles);
			if ( IsMaitre(CurrentGame, pJeu, &pJeu->MyCarte[i])) TousUtiles = 1;
			if ( pJeu->ProbCoupe[c] > 0.25 && pp > 1.0) pp = 1.0;
			val = (2*ProbGain - 1.0 + 0.5*(TotHonneur - pJeu->NbCarte + MinPliP) + x - 0.5) * pJeu->MyCarte[i].Valeur
				+ 0.75/pJeu->NbCouleur[c] -  0.5*NbNonHonneur(CurrentGame, pJeu->PositionJoueur, c) + 2.0*pJeu->ProbCoupe[c]
				- (1.0 + 6.0*TousUtiles)*(1.0-pJeu->ProbCoupe[c])*sqrt(pp);
#if DEBUG_DEFENSE_SECOND > 0
            OutDebug("Défausse 2nd : ");
            ImprimeCarte(&Table[0]);
            ImprimeCarte(&Table[1]);
            OutDebug(", val : %.2f = (2.0* %.2f -1.0 + %.2f) * %d + 0.75/%d -  0.5*%d + 2.0*%.2f - (1.0 + 6*%d) * (1.0 - %.2f) * %.2f\n"
                , val, ProbGain, 0.5*(TotHonneur - pJeu->NbCarte + MinPliP), pJeu->MyCarte[i].Valeur, pJeu->NbCouleur[c]
                 , NbNonHonneur(CurrentGame, pJeu->PositionJoueur, c), pJeu->ProbCoupe[c], TousUtiles, pJeu->ProbCoupe[c], pp);
#endif // DEBUG_DEFENSE_SECOND
			if ( val > Bval )
			{
				Best = i;
				Bval = val;
			}
		}
		assert(Best >= 0);
		return(Best);
	}
	return(DefGeneral(CurrentGame, pJeu, ProbGain, MinPliP));
}

#if DEBUG > 0
#define DEBUG_DEFENSE_3EME 0
#else
#define DEBUG_DEFENSE_3EME 0
#endif  // DEBUG


//	Joue en troisième mais n'a pas de la couleur demandée
//	Applique un algorithme spécifique pour se défausser
//	Retourne l'index de la carte a joueur si a trouvé une défausse, -1 dans le cas contraire
//	Dans ce cas, le joueur joue "comme avant"

int JoueDefausseEnTroisieme(TarotGame CurrentGame, struct _Jeu *pJeu)
{
double ProbGain;
int MinPliP;
int i, c;
int Best = -1;
double Bval;
double val;
double TousUtiles;
double pp = 0;
int HonneurParCouleur[4];
int TotHonneur;
int JoueAvantPreneur = 0;
int NbPoint = Table[0].Valeur + Table[1].Valeur;
int CouleurDemandee;
int BestC = -1;
int BestH = -1;

	CouleurDemandee = Table[0].Couleur;
	if ( CouleurDemandee == EXCUSE )
		CouleurDemandee = Table[1].Couleur;
	if ( HasExcuse(pJeu) && pJeu->NbCarte == 2 )
	{	//	Joue l'excuse en avant-dernier !
		return(0);
	}
#if DEBUG > 0
    if ( Table[0].Couleur == COEUR && Table[0].Hauteur == 1 )
        BestC = -1;
#endif // DEBUG
	MakeCarte(&Table[2], 0);
	ProbGain = ProbGainCoup(CurrentGame, pJeu, 2, 0);
	MinPliP = MinPlisPreneur(CurrentGame, pJeu);
	if ( ((pJeu->PositionPreneur - CurrentGame->JoueurEntame)&3) > 2 ) JoueAvantPreneur = 1;
	TotHonneur = CompteHonneur(CurrentGame, pJeu->PositionJoueur, HonneurParCouleur);
#if DEBUG_DEFENSE_3EME > 0
    OutDebug("JoueDefausseEnTroisieme, ProbGain = %.2f, MinPliP = %d\n", ProbGain, MinPliP);
#endif // DEBUG_DEFENSE_3EME
	//	Cas 1 : défausse quand le pli est perdu
	//	Joue plutôt les petites cartes dans les couleurs coupées par le preneur
	//	Evite de trop défausser dans une couleur pour protéger ses honneurs.
	//	Ne défausse pas dans les couleurs où susceptible de faire des plis.
	//	Défausse éventuellement dans des courtes même non coupées par le preneur pour
	//	pouvoir défausser des grosses ensuite.
	if ( ProbGain < 1e-3 )		//	Pas trop de chances de gagner...
	{
		Bval = -1000000.0;
		if ( JoueAvantPreneur && IsJoue(CurrentGame, 0) == 0 && pJeu->TmpProbCarte[pJeu->PositionPreneur][0] >= 0.5 && TotHonneur >= 1)
		{	//	Le preneur a peut-être l'excuse, met au moins un valet...
			for ( c = 0; c < 4; c++)
			{
				for ( i = 10; i < 13; i++)
				{
					if ( HasCarte(pJeu, pJeu->PositionJoueur, Startof[c] + i) )
					{
						val = 10.0 * pJeu->ProbCoupe[c]
							- (NbPoint + 2.0*(i - 9)*(i-9))*(1.0 - pJeu->TmpProbCarte[pJeu->PositionPreneur][0]*( 10.0*pow(1.0/pJeu->NbCarte,3.0) ));
						if ( val > Bval )
						{
							BestC = c ;
							BestH = i+1;
							Bval = val;
						}
					}
				}
			}
			if ((TotHonneur - pJeu->NbCarte - MinPliP) >= 0)
				Bval += (TotHonneur - pJeu->NbCarte - MinPliP);
			if ( Bval > 0)
			{
				return(Carte2Index(pJeu, BestC, BestH));
			}
		}
		return(DefPerdante(CurrentGame, pJeu));
	}
	// Cas 2 : défausse quand la défense a le pli.
	//	Défausse plutôt des grosses cartes dans les couleurs coupées par le preneur.
	//	Ne défausse pas dans les couleurs où susceptible de faire des plis.
	if ( ProbGain > 0.99 )
	{
		return(DefGagnant(CurrentGame, pJeu));
	}
	//	Cas 3 : Le preneur est sur de faire tous les plis
	if ( pJeu->NbCarte == MinPliP )
		return(-1);
	//	Cas général, pas trop sur des probabilités
	if ( TotHonneur >= pJeu->NbCarte - MinPliP )		//	Pas trop le choix, il faut en sauver...
	{
	//	Défausse plutôt des grosses cartes dans les couleurs coupees par le preneur.
	//	La valeur des cartes dépend de la probabilité de gain...
	//	Ne défausse pas dans les couleurs où susceptible de faire des plis.
		Bval = -1000000.0;
		//	Si il reste un seul pli à faire, cas spécial
		//	Si joue dans coupe preneur et preneur possède atout ne change rien
		//	Si joue dans couleur preneur, fait comme si le gain est possible
		if ( pJeu->NbCarte - MinPliP == 1 && (NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT) - pJeu->AtoutPreneur) >= 0.2
			&&	(JoueAvantPreneur == 0 || pJeu->ProbCoupe[CouleurDemandee] < 0.9) )
			ProbGain = 1.0;
		for ( i = 0; i < pJeu->NbCarte; i++)
		{
			c = pJeu->MyCarte[i].Couleur;
			pp = PlisPossibles(CurrentGame, pJeu, c, &TousUtiles);
			if ( IsMaitre(CurrentGame, pJeu, &pJeu->MyCarte[i])) TousUtiles = 1;
			if ( pJeu->ProbCoupe[c] > 0.25 && pp > 1.0) pp = 1.0;
			val = (2*ProbGain - 1.0 + 0.5*(TotHonneur - pJeu->NbCarte + MinPliP)) * pJeu->MyCarte[i].Valeur
                + 0.75/pJeu->NbCouleur[c] -  0.5*NbNonHonneur(CurrentGame, pJeu->PositionJoueur, c) + 2.0*pJeu->ProbCoupe[c] - (1.0 + 6.0*TousUtiles)*(1.0-pJeu->ProbCoupe[c])*sqrt(pp);
#if DEBUG_DEFENSE_3EME > 0
            OutDebug("Defausse3eme : ");
            ImprimeCarte(&Table[0]);
            ImprimeCarte(&Table[1]);
            ImprimeCarte(&pJeu->MyCarte[i]);
            OutDebug(", val : %.2f =  %.2f * %d + 0.75/%d -  0.5*%d + 2.0*%.2f - (1.0 + 6*%d) * (1.0 - %.2f) *  %.2f\n"
                    , val, (2*ProbGain - 1.0 + 0.5*(TotHonneur - pJeu->NbCarte + MinPliP)), pJeu->MyCarte[i].Valeur, pJeu->NbCouleur[c]
                    , NbNonHonneur(CurrentGame, pJeu->PositionJoueur, c), pJeu->ProbCoupe[c], TousUtiles, pJeu->ProbCoupe[c], pp);
#endif // DEBUG_DEFENSE_3EME
			if ( val > Bval )
			{
				Best = i;
				Bval = val;
			}
		}
		assert(Best >= 0);
		return(Best);
	}
	return(DefGeneral(CurrentGame, pJeu, ProbGain, MinPliP));
}


//	Sélectionne la carte a jouer en 3ème pour la défense

void JoueDefenseTroisieme(TarotGame CurrentGame)
{
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[CurrentGame->JoueurCourant];
int i;
int Best = -1;
double BestScore = -1e6;
double Score;
int FirstInC = 0;
int FirstV = 1;
int CouleurDemandee;
int BestC;

	assert(pJeu->NbCarte > 0);
    CalcTmpProbCarte(CurrentGame);      //  Apprend des cartes jouées auparavant
	pJeu->AtoutPreneur = AvgLongueur(pJeu, CurrentGame->JoueurPreneur, ATOUT);
	pJeu->ResteAtout = NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT);
	GainPetitAuBout(CurrentGame, pJeu);
#if DEBUG_DEFENSE_3EME
    OutDebug("---\nPosition %d, JoueEnTroisieme après ", pJeu->PositionJoueur);
    ImprimeCarte(&Table[0]);
    ImprimeCarte(&Table[1]);
    OutDebug("\n");
    OutDebug("Proba Petit = %d:%.3f  %d:%.3f  %d:%.3f  Chien:%.3f\n",
             pJeu->PositionJoueur^1, pJeu->TmpProbCarte[pJeu->PositionJoueur^1][1],
             pJeu->PositionJoueur^2, pJeu->TmpProbCarte[pJeu->PositionJoueur^2][1],
             pJeu->PositionJoueur^3, pJeu->TmpProbCarte[pJeu->PositionJoueur^3][1],
             pJeu->TmpProbCarte[4][1]);
    OutDebug("Proba 21 = %d:%.3f  %d:%.3f  %d:%.3f  Chien:%.3f\n",
             pJeu->PositionJoueur^1, pJeu->TmpProbCarte[pJeu->PositionJoueur^1][21],
             pJeu->PositionJoueur^2, pJeu->TmpProbCarte[pJeu->PositionJoueur^2][21],
             pJeu->PositionJoueur^3, pJeu->TmpProbCarte[pJeu->PositionJoueur^3][21],
             pJeu->TmpProbCarte[4][1]);
    OutDebug("ProbCoupe[T]=%.3f, ProbCoupe[K]=%.3f, ProbCoupe[P]=%.3f, ProbCoupe[C]=%.3f\n",
              pJeu->ProbCoupe[TREFLE], pJeu->ProbCoupe[CARREAU], pJeu->ProbCoupe[PIQUE], pJeu->ProbCoupe[COEUR]);
    OutDebug("GuessProbCoupe[T]=%.3f, GuessProbCoupe[K]=%.3f, GuessProbCoupe[P]=%.3f, GuessProbCoupe[C]=%.3f\n",
          pJeu->GuessProbCoupe[TREFLE], pJeu->GuessProbCoupe[CARREAU], pJeu->GuessProbCoupe[PIQUE], pJeu->GuessProbCoupe[COEUR]);
    OutDebug("CalProbCoupe[T]=%.3f, CalProbCoupe[K]=%.3f, CalProbCoupe[P]=%.3f, CalProbCoupe[C]=%.3f\n",
          pJeu->CalProbCoupe[TREFLE], pJeu->CalProbCoupe[CARREAU], pJeu->CalProbCoupe[PIQUE], pJeu->CalProbCoupe[COEUR]);
    OutDebug("TmpProbCoupe[T]=%.3f, TmpProbCoupe[K]=%.3f, TmpProbCoupe[P]=%.3f, TmpProbCoupe[C]=%.3f\n",
          pJeu->TmpProbCoupe[TREFLE], pJeu->TmpProbCoupe[CARREAU], pJeu->TmpProbCoupe[PIQUE], pJeu->TmpProbCoupe[COEUR]);
    OutDebug("Longueur ATOUT Preneur =%.2f\n", pJeu->AtoutPreneur);
    OutDebug("Status Chasse = %d\n", pJeu->StatusChasse);
    OutDebug("MinPlisPreneur = %d\n", MinPlisPreneur(CurrentGame, pJeu));
    OutDebug("Flanc = %d, ValFlanc = %.3f, CouleurTenur = %d\n", pJeu->Flanc, pJeu->Flanc > 0 ? pJeu->ValFlanc[pJeu->Flanc]:0.0, pJeu->CouleurTenue);
    OutDebug("ProbGain = %.3f\n", ProbGainCoup(CurrentGame, pJeu, 2, 0));
    if ( HasCarte(pJeu, pJeu->PositionJoueur, 1) )
        OutDebug("Valeur petit au bout = %.3f\n", pJeu->ValPetitAuBout);
#if DEBUG > 0
    if ( Table[0].Couleur == PIQUE )
        i = 0;
#endif
#endif // DEBUG_DEFENSE_3EME
    MakeCarte(&Table[3], 0);             //  Simule excuse après pour être sûr de ne pas faire le pli.
	CouleurDemandee = Table[0].Couleur;
	if ( CouleurDemandee == EXCUSE )
		CouleurDemandee = Table[1].Couleur;
	if ( Table[0].Couleur == ATOUT && pJeu->NbAtout > 0 )
	{
		if ( (BestC = JeuDefenseAtout(CurrentGame, pJeu)) >= 0 )
		{
			PoseCarte(CurrentGame, BestC);
			return;
		}
	}
	for ( i = 0; i < pJeu->NbCarte; i++)
	{
		if ( CarteValide(pJeu, i, CurrentGame->JoueurEntame) )		//	Peut jouer cette carte
		{
			if ( FirstV && pJeu->MyCarte[i].Couleur != CouleurDemandee && pJeu->MyCarte[i].Couleur > ATOUT )
			{
				if ( (BestC = JoueDefausseEnTroisieme(CurrentGame, pJeu)) >= 0 )
				{
                    PoseCarte(CurrentGame, BestC);
					return;
				}
			}
			Table[2] = pJeu->MyCarte[i];
			if ( Gagnant(2) == 2 && Table[2].Couleur > ATOUT )
			{
				if ( (BestC = JoueDefenseGagnant(CurrentGame, pJeu)) >= 0)
				{
                    PoseCarte(CurrentGame, BestC);
					return;
				}
			}
			Table[2] = pJeu->MyCarte[i];
			if ( GainPli(CurrentGame, 2) && ((pJeu->PositionPreneur - CurrentGame->JoueurEntame)&3) < 2
				&& pJeu->MyCarte[i].Couleur == CouleurDemandee && CouleurDemandee > ATOUT )
			{
			    //  La défense est sûre de gagner le pli (preneur avant le joueur)
				if ( (BestC = JoueDefenseGagnant(CurrentGame, pJeu)) >= 0)
				{
                    PoseCarte(CurrentGame, BestC);
					return;
				}
			}
			//  Cas spéciaux de coupe de la défense
			if ( Table[0].Couleur > ATOUT && Table[2].Couleur == ATOUT )
            {
				if ( (BestC = JoueCoupeDefense(CurrentGame, pJeu)) >= 0)
				{
                    PoseCarte(CurrentGame, BestC);
					return;
				}
            }
#if DEBUG > 0
            if (CouleurDemandee == PIQUE && Table[0].Hauteur == VALET )
                Score = 0;
#endif // DEBUG
			Table[2] = pJeu->MyCarte[i];
			if ( Table[2].Couleur != EXCUSE )
				FirstV = 0;
			Score = EvalCoupTroisieme(CurrentGame, pJeu);
			if ( Table[2].Couleur == ATOUT && Table[2].Index == GetCartePlusForte(pJeu, ATOUT)->Index && HasPetit(pJeu) )
				Score += 3.0 + (pJeu->PositionPreneur == ((pJeu->PositionJoueur+1) & 3)) ;      //  Ajoute des points si joue son gros devant preneur
			if ( FlagSignalisation )
			{
				if ( CouleurDemandee > ATOUT && Table[2].Couleur == CouleurDemandee && pJeu->NbCouleur[CouleurDemandee] == 2
					&& CurrentGame->JoueurEntame != pJeu->PositionPreneur && pJeu->JoueurCouleur[CouleurDemandee] < 0 )
				{
				    //  Retire un point à la première carte pour jouer doubleton descendant
					if ( FirstInC == 0 )
					{
						Score -= 1.0;
						FirstInC = 1;
					}
				}
				if ( CouleurDemandee > ATOUT && Table[2].Couleur == CouleurDemandee && CurrentGame->JoueurEntame == pJeu->PositionPreneur
					&& pJeu->JoueurCouleur[CouleurDemandee] < 0 && !HasPetit(pJeu) &&
					((pJeu->NbCouleur[CouleurDemandee] >= 5 && GetCartePlusForte(pJeu, CouleurDemandee)->Hauteur >= CAVALIER ) ))
				{   //  Longue dans l'ouverture du preneur, retire 1 point à la plus faible pour jouer descendant
					if ( FirstInC == 0 )
					{
						Score -= 1.0;
						FirstInC = 1;
					}
				}
				if ( CouleurDemandee > ATOUT && Table[2].Couleur == EXCUSE && CurrentGame->JoueurEntame == pJeu->PositionPreneur
                    && pJeu->JoueurCouleur[CouleurDemandee] < 0 && !HasPetit(pJeu) )
                {
                    if (pJeu->NbCouleur[CouleurDemandee] >= 5 && GetCartePlusForte(pJeu, CouleurDemandee)->Hauteur >= CAVALIER )
                    {
                        //  Longue et forte dans couleur preneur, OK pour mettre excuse
                        Score += 3.0;
                    }
                    else if ( pJeu->NbCouleur[CouleurDemandee] == 1 && Table[0].Hauteur == ROI && GetCartePlusForte(pJeu, CouleurDemandee)->Hauteur >= DAME )
                    {
                        Score += 1.0;       //  Ok pour sauver la dame
                    }
                    else if (Table[0].Hauteur == ROI )
                    {
                        Score -= 5.0;           //  Pas bon de tromper les partenaires
                    }
                }

			}
#if DEBUG_DEFENSE_3EME > 0
            OutDebug("Essai Carte ");
            ImprimeCarte(&Table[2]);
            OutDebug(" Score = %.2f\n", Score);
#endif // DEBUG_DEFENSE_3EME
			if ( Score > BestScore )
			{
				BestScore = Score;
				Best = i;
			}
		}
	}
	if ( Best >= 0 )
	{
        PoseCarte(CurrentGame, Best);
		return;
	}
	assert(0);
}

#if DEBUG > 0
#define DEBUG_DEFENSE_DERNIER 0
#else
#define DEBUG_DEFENSE_DERNIER 0
#endif  // DEBUG

//	Joue en dernier mais n'a pas de la couleur demandée
//	Applique un algorithme spécifique pour se défausser
//	Retourne l'index de la carte a joueur si a trouvé une défausse, -1 dans le cas contraire
//	Dans ce cas, le joueur joue "comme avant"

int JoueDefausseEnDernier(TarotGame CurrentGame, struct _Jeu *pJeu)
{
double ProbGain;
int MinPliP;
int i;
int Best = -1;
double Bval;
double val;


	if ( HasExcuse(pJeu) && pJeu->NbCarte == 2 )
	{	//	Joue l'excuse en avant-dernier !
		return(0);
	}
	MakeCarte(&Table[3], 0);
	ProbGain = ProbGainCoup(CurrentGame, pJeu, 3, 0);
	MinPliP = MinPlisPreneur(CurrentGame, pJeu);
	//	Cas 1 : dernier pli possible de la défense, met la plus grosse...
	if ( MinPliP == pJeu->NbCarte - 1 && ProbGain > 0.5 )		//	Met la plus grosse
	{
		Bval = 0;
		for (i = 0; i < pJeu->NbCarte; i++)
		{
			val = pJeu->MyCarte[i].Valeur;
			if ( val > Bval )
			{
				Best = i;
				Bval = val;
			}
		}
		assert(Best >= 0);
		return(Best);
	}
	if ( ProbGain < 1e-7 )
	{
		return(DefPerdante(CurrentGame, pJeu));
	}
	// Cas 3 : défausse en dernier quand la défense a le pli.
	return(DefGagnant(CurrentGame, pJeu));
}

//	Sélectionne la meilleure carte à jouer pour le joueur en dernier

void JoueDefenseDernier(TarotGame CurrentGame)
{
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[CurrentGame->JoueurCourant];

int i;
int Best = -1;
double BestScore = -1e6;
double Score;
int FirstInC = 0;
int FirstV = 1;
int CouleurDemandee;
int CBest;
int iGagnant;


	assert(pJeu->NbCarte > 0);
    CalcTmpProbCarte(CurrentGame);      //  Apprend des cartes jouées auparavant
	pJeu->AtoutPreneur = AvgLongueur(pJeu, CurrentGame->JoueurPreneur, ATOUT);
	pJeu->ResteAtout = NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT);
	GainPetitAuBout(CurrentGame, pJeu);
#if DEBUG_DEFENSE_DERNIER > 0
    OutDebug("----\nPosition %d, JoueDefenseDernier après ", pJeu->PositionJoueur);
    ImprimeCarte(&Table[0]);
    ImprimeCarte(&Table[1]);
    ImprimeCarte(&Table[2]);
    OutDebug("\n");
    OutDebug("Proba Petit = %d:%.3f  %d:%.3f  %d:%.3f  Chien:%.3f\n",
             pJeu->PositionJoueur^1, pJeu->TmpProbCarte[pJeu->PositionJoueur^1][1],
             pJeu->PositionJoueur^2, pJeu->TmpProbCarte[pJeu->PositionJoueur^2][1],
             pJeu->PositionJoueur^3, pJeu->TmpProbCarte[pJeu->PositionJoueur^3][1],
             pJeu->TmpProbCarte[4][1]);
    OutDebug("Proba 21 = %d:%.3f  %d:%.3f  %d:%.3f  Chien:%.3f\n",
             pJeu->PositionJoueur^1, pJeu->TmpProbCarte[pJeu->PositionJoueur^1][21],
             pJeu->PositionJoueur^2, pJeu->TmpProbCarte[pJeu->PositionJoueur^2][21],
             pJeu->PositionJoueur^3, pJeu->TmpProbCarte[pJeu->PositionJoueur^3][21],
             pJeu->TmpProbCarte[4][1]);
    OutDebug("ProbCoupe[T]=%.3f, ProbCoupe[K]=%.3f, ProbCoupe[P]=%.3f, ProbCoupe[C]=%.3f\n",
          pJeu->ProbCoupe[TREFLE], pJeu->ProbCoupe[CARREAU], pJeu->ProbCoupe[PIQUE], pJeu->ProbCoupe[COEUR]);
    OutDebug("GuessProbCoupe[T]=%.3f, GuessProbCoupe[K]=%.3f, GuessProbCoupe[P]=%.3f, GuessProbCoupe[C]=%.3f\n",
          pJeu->GuessProbCoupe[TREFLE], pJeu->GuessProbCoupe[CARREAU], pJeu->GuessProbCoupe[PIQUE], pJeu->GuessProbCoupe[COEUR]);
    OutDebug("CalProbCoupe[T]=%.3f, CalProbCoupe[K]=%.3f, CalProbCoupe[P]=%.3f, CalProbCoupe[C]=%.3f\n",
          pJeu->CalProbCoupe[TREFLE], pJeu->CalProbCoupe[CARREAU], pJeu->CalProbCoupe[PIQUE], pJeu->CalProbCoupe[COEUR]);
    OutDebug("TmpProbCoupe[T]=%.3f, TmpProbCoupe[K]=%.3f, TmpProbCoupe[P]=%.3f, TmpProbCoupe[C]=%.3f\n",
          pJeu->TmpProbCoupe[TREFLE], pJeu->TmpProbCoupe[CARREAU], pJeu->TmpProbCoupe[PIQUE], pJeu->TmpProbCoupe[COEUR]);
    OutDebug("Longueur ATOUT Preneur =%.2f\n", pJeu->AtoutPreneur);
    OutDebug("Status Chasse = %d\n", pJeu->StatusChasse);
    OutDebug("MinPlisPreneur = %d\n", MinPlisPreneur(CurrentGame, pJeu));
    OutDebug("Flanc = %d, ValFlanc = %.3f, CouleurTenur = %d\n", pJeu->Flanc, pJeu->Flanc > 0 ? pJeu->ValFlanc[pJeu->Flanc]:0.0, pJeu->CouleurTenue);
    OutDebug("ProbGain = %.3f\n", ProbGainCoup(CurrentGame, pJeu, 3, 0));
    if ( HasCarte(pJeu, pJeu->PositionJoueur, 1) )
        OutDebug("Valeur petit au bout = %.3f\n", pJeu->ValPetitAuBout);
#endif // DEBUG_DEFENSE_DERNIER
	CouleurDemandee = Table[0].Couleur;
	if ( CouleurDemandee == EXCUSE )
		CouleurDemandee = Table[1].Couleur;
	if ( Table[0].Couleur == ATOUT && pJeu->NbAtout > 0 )
	{
		if ( (CBest = JeuDefenseAtout(CurrentGame, pJeu)) >= 0 )
		{
			PoseCarte(CurrentGame, CBest);
			return;
		}
	}
	for ( i = 0; i < pJeu->NbCarte; i++)
	{
		if ( CarteValide(pJeu, i, CurrentGame->JoueurEntame) )		//	Peut jouer cette carte
		{
			if ( FirstV && pJeu->MyCarte[i].Couleur != CouleurDemandee && pJeu->MyCarte[i].Couleur > ATOUT )
			{
				if ( (CBest = JoueDefausseEnDernier(CurrentGame, pJeu)) >= 0 )
				{
                    PoseCarte(CurrentGame, CBest);
					return;
				}
			}
			Table[3] = pJeu->MyCarte[i];
			if ( Gagnant(3) == 3 && Table[3].Couleur > ATOUT)
			{
				if ( (CBest = JoueDefenseGagnant(CurrentGame, pJeu)) >= 0)
				{
                    PoseCarte(CurrentGame, CBest);
					return;
				}
			}
			if ( GainPli(CurrentGame, 3) && pJeu->MyCarte[i].Couleur == CouleurDemandee && CouleurDemandee > ATOUT )
			{
				if ( (CBest = JoueDefenseGagnant(CurrentGame, pJeu)) >= 0)
				{
                    PoseCarte(CurrentGame, CBest);
					return;
				}
			}
			Table[3] = pJeu->MyCarte[i];
			if ( Table[3].Couleur != EXCUSE )
				FirstV = 0;
			Score = EvalCoup(CurrentGame, pJeu, &iGagnant);
			//  Si joueur à la petit et le preneur n'a plus d'atout, OK pour jouer gros atout
			if ( pJeu->AtoutPreneur < 0.15 && Table[3].Couleur == ATOUT && Table[3].Index == GetCartePlusForte(pJeu, ATOUT)->Index && HasPetit(pJeu) )
				Score += 4.0;
			if ( FlagSignalisation )
			{
				if ( CouleurDemandee > ATOUT && Table[3].Couleur == CouleurDemandee && pJeu->NbCouleur[CouleurDemandee] == 2
					&& CurrentGame->JoueurEntame != pJeu->PositionPreneur && pJeu->JoueurCouleur[CouleurDemandee] < 0 )
				{	//	Doubleton, joue à l'envers...
					if ( FirstInC == 0 )
					{
						Score -= 1.0;
						FirstInC = 1;
					}
				}
				if ( CouleurDemandee > ATOUT && Table[3].Couleur == CouleurDemandee && CurrentGame->JoueurEntame == pJeu->PositionPreneur
					&& pJeu->JoueurCouleur[CouleurDemandee] < 0 && HasPetit(pJeu) == 0 &&
					((pJeu->NbCouleur[CouleurDemandee] >= 5 && GetCartePlusForte(pJeu, CouleurDemandee)->Hauteur >= CAVALIER ) ))
				{	//	Tient la couleur, joue descendant également
					if ( FirstInC == 0 )
					{
						Score -= 1.0;
						FirstInC = 1;
					}
				}
			}
#if DEBUG_DEFENSE_DERNIER > 0
            OutDebug("Essai Carte ");
            ImprimeCarte(&pJeu->MyCarte[i]);
            OutDebug(" Score = %.2f\n", Score);
#endif // DEBUG_DEFENSE_DERNIER
			if ( Score > BestScore )
			{
				BestScore = Score;
				Best = i;
			}
		}
	}
	if ( Best >= 0 )
	{
        PoseCarte(CurrentGame, Best);
		return;
	}
    assert(0);
}


//	Calcule la probabilité de gain du coup pour la défense.
//  indexTable est la position sur la table (0 à 3) et CarteDef est la carte jouée

double ProbGainCoup(TarotGame CurrentGame, struct _Jeu *pJeu, int indexTable, int CarteDef)
{
int gagnant;
int i, k;
int posj, posk, posP;
double Prob4C = 1.0;        //  Proba joueur 4 gagne à la couleur
double Prob4A = 1.0;        //  Proba Joueur 4 gagne pas à l'atout
double Prob3C = 0.0;
double ProbP;
double ProbC = 1.0;
double ProbC1 = 1.0;
int CouleurDemandee;
double probGC[21], Tmp;
int IndexCoup[21], iTmp;

    //  Fabrique la carte
    MakeCarte(&Table[indexTable], CarteDef);
    //  Regarde qui a le pli jusqu'à la position courante
    gagnant = Gagnant(indexTable);
    gagnant = (gagnant + CurrentGame->JoueurEntame) & 3;
	switch ( indexTable )
	{
	case 3:
	//	Regarde qui a le pli jusqu'à la dernière carte. Ici on est sûr ce ne sont pas des probabilités
		if ( gagnant == pJeu->PositionPreneur )
			return(0.0);
		return(1.0);
	case 2:
        //	Si le preneur est avant le joueur, regarde qui a le pli
        //	Si c'est la défense retourne 1.0. Sinon calcule en fonction du dernier
		if ( ((pJeu->PositionPreneur - CurrentGame->JoueurEntame) & 3) <= 1 )
		{
			if ( gagnant != pJeu->PositionPreneur )
				return(1.0);
		}
        //	Calcule la proba en fonction de ce que pourrait jouer le dernier
		posj = (CurrentGame->JoueurEntame + 3) & 3;
		CouleurDemandee = Table[0].Couleur;
		if ( CouleurDemandee == EXCUSE ) CouleurDemandee = Table[1].Couleur;
		if ( CouleurDemandee > ATOUT )
		{
			for ( i = Startof[CouleurDemandee]; i < Endof[CouleurDemandee]; i++)
			{
				if ( Table[0].Index == i || Table[1].Index == i ) continue;
				if ( pJeu->TmpProbCarte[posj][i] < 1e-7 ) continue;
				MakeCarte(&Table[3], i);
				gagnant = Gagnant(3);
				if ( gagnant == 3 )
					Prob4C *= 1.0 - pJeu->TmpProbCarte[posj][i];        //  Prob4C = Proba(Joueur 4 n'ait pas de carte qui gagne)
                ProbC *= 1.0 - pJeu->TmpProbCarte[posj][i];             //  ProbC = Proba(Joueur 4 n'ait pas de cartes dans la couleur)
			}
		}
		for ( i = 1; i < 22; i++)
		{
			if ( Table[0].Index == i || Table[1].Index == i ) continue;
			if ( pJeu->TmpProbCarte[posj][i] < 1e-7 ) continue;
			MakeCarte(&Table[3], i);
			gagnant = Gagnant(3);
			if ( gagnant == 3 )
				Prob4A *= 1.0 - pJeu->TmpProbCarte[posj][i];            //  Prob4A = Proba(Joueur 4 n'ait pas d'atouts gagnants)
		}
		//  Proba gain par le joueur 4. Deux cas
		//  1) Possède des cartes gagnantes dans la couleur
		//  2) Pas de cartes perdantes à la couleur et des atouts gagnants
		Prob4C = (1-ProbC)*(1.0 - Prob4C) + ProbC*(1.0 - Prob4A);
		if ( ((pJeu->PositionPreneur - CurrentGame->JoueurEntame) & 3) == 3 )
			return(1.0 - Prob4C);
		return(Prob4C);
	case 1:
		if ( pJeu->PositionPreneur == CurrentGame->JoueurEntame )
		{
		    //  Le preneur a l'entame
		    gagnant = Gagnant(1);
		    if ( gagnant > 0 )      //  Le joueur 1 (qui n'est pas le preneur gagne)
				return(1.0);
		}
        //	Calcule la proba de gain en fonction du jeu des joueurs 3 et 4
        //	Pour chaque carte possible jouée par 3, calcule la proba de gain par le joueur 4
        //	Calcule ensuite la proba de 3 en fonction de ces résultats
        //	Accumule les probas en partant du fait que 3 joue sa carte la plus forte en premier
		posk = (CurrentGame->JoueurEntame + 2) & 3;
		posj = (CurrentGame->JoueurEntame + 3) & 3;
		posP = (CurrentGame->JoueurPreneur - CurrentGame->JoueurEntame) & 3;
		CouleurDemandee = Table[0].Couleur;
		if ( CouleurDemandee == EXCUSE ) CouleurDemandee = Table[1].Couleur;
		if ( CouleurDemandee > ATOUT )
		{
			for ( k = Startof[CouleurDemandee]; k < Endof[CouleurDemandee]; k++)
			{
				probGC[k-Startof[CouleurDemandee]] = 0.0;
				IndexCoup[k-Startof[CouleurDemandee]] = k;
				if ( Table[0].Index == k || Table[1].Index == k ) continue;
				if ( pJeu->TmpProbCarte[posk][k] < 1e-7 ) continue;
				MakeCarte(&Table[2], k);
				Prob4C = 1.0;		//	Proba que le joueur 4 ne soit pas maître
				ProbC = 1.0;		//	Proba que le joueur 4 n'ait pas de la couleur
				Prob4A = 1.0;		//	Proba que 4 ne soit pas maître à l'atout
				if ( CouleurDemandee > ATOUT )
				{
					for ( i = Startof[CouleurDemandee]; i < Endof[CouleurDemandee]; i++)
					{
						if ( Table[0].Index == i || Table[1].Index == i || Table[2].Index == i ) continue;
						if ( pJeu->TmpProbCarte[posj][i] < 1e-7 ) continue;
						MakeCarte(&Table[3], i);
						gagnant = Gagnant(3);
						if ( gagnant == 3 )
							Prob4C *= 1.0 - pJeu->TmpProbCarte[posj][i];	//	Accumule proba pas maître
						ProbC *= 1.0 - pJeu->TmpProbCarte[posj][i];		    //	Accumule proba couleur
					}
				}
				for ( i = 1; i < 22; i++)
				{
					if ( Table[0].Index == i || Table[1].Index == i || Table[2].Index == i ) continue;
					if ( pJeu->TmpProbCarte[posj][i] < 1e-7 ) continue;
					MakeCarte(&Table[3], i);
					gagnant = Gagnant(3);
					if ( gagnant == 3 )
						Prob4A *= 1.0 - pJeu->TmpProbCarte[posj][i];	//	Accumule proba pas maître atout
				}
				//	Proba que le joueur 4 soit maître
				Prob4C = (1.0-ProbC)*(1.0 - Prob4C) + ProbC*(1.0 - Prob4A);
				//	Calcule maintenant la proba que l'équipe de 3 soit maître
				if ( posP == 3 )	//	Preneur en 4
                {
                    //  Dans ce cas équipe de 3 = défense
					probGC[k-Startof[CouleurDemandee]] = 1.0 - Prob4C;
                }
				else if ( posP == 2 )	//	Preneur en 3
				{
				    //  Equipe de 3 = attaque
					if ( Gagnant(2) == 2 )  //  Oui, preneur maître quand il a joué, sinon proba = 0
						probGC[k-Startof[CouleurDemandee]] = 1.0 - Prob4C;
				}
				else
				{					        //	Preneur entame;
					if ( Gagnant(2) )
						probGC[k-Startof[CouleurDemandee]] = 1.0;       //  Si Joueur 3 Maître Proba = 1.0
					else
						probGC[k-Startof[CouleurDemandee]] = Prob4C;    //  Sinon, proba = Proba calculée Prob4C
				}
			}
            //	Trie maintenant avec le plus fort en tête
			for ( i = 0; i < 14; i++)
			{
				for ( k = i+1; k < 14; k++)
				{
					if ( probGC[i] < probGC[k] )
					{
						Tmp = probGC[i];
						probGC[i] = probGC[k];
						probGC[k] = Tmp;
						iTmp = IndexCoup[i];
						IndexCoup[i] = IndexCoup[k];
						IndexCoup[k] = iTmp;
					}
				}
			}
			ProbC1 = 1.0;
			Prob3C = 0.0;
			for ( i = 0; i < 14; i++ )
			{
				Prob3C += pJeu->TmpProbCarte[posk][IndexCoup[i]] * probGC[i] * ProbC1;
				if ( pJeu->TmpProbCarte[posk][IndexCoup[i]] > 1e-7 )
					ProbC1 *= 1.0 - pJeu->TmpProbCarte[posk][IndexCoup[i]];
			}
		}
		//	Si 3 n'a pas de la couleur, il peut jouer ATOUT...
		for ( k = 1; k < 22; k++)
		{
			probGC[k-1] = 0.0;
			IndexCoup[k-1] = k;
			if ( Table[0].Index == k ) continue;
			if ( pJeu->TmpProbCarte[posk][k] < 1e-7 ) continue;
			MakeCarte(&Table[2], k);
			Prob4C = 1.0;
			ProbC = 1.0;
			Prob4A = 1.0;
			if ( CouleurDemandee > ATOUT )
			{
				for ( i = Startof[CouleurDemandee]; i < Endof[CouleurDemandee]; i++)
				{
					if ( Table[0].Index == i || Table[1].Index == i || Table[2].Index == i ) continue;
					if ( pJeu->TmpProbCarte[posj][i] < 1e-7 ) continue;
					MakeCarte(&Table[3], i);
					gagnant = Gagnant(3);
					if ( gagnant == 3 )
						Prob4C *= 1.0 - pJeu->TmpProbCarte[posj][i];
					else
						ProbC *= 1.0 - pJeu->TmpProbCarte[posj][i];
				}
			}
			for ( i = 1; i < 22; i++)
			{
				if ( Table[0].Index == i || Table[1].Index == i || Table[2].Index == i ) continue;
				if ( pJeu->TmpProbCarte[posj][i] < 1e-7 ) continue;
				MakeCarte(&Table[3], i);
				gagnant = Gagnant(3);
				if ( gagnant == 3 )
					Prob4A *= 1.0 - pJeu->TmpProbCarte[posj][i];
			}
			//	Proba que le joueur 4 soit maître
			Prob4C = (1.0 - ProbC) * (1.0 - Prob4C) + ProbC*(1.0 - Prob4A);
			if ( posP == 3 )	//	Preneur en 4
				probGC[k-1] = 1.0 - Prob4C;
            else if ( posP == 2 )	//	Preneur en 3
            {
                //  Equipe de 3 = attaque
                if ( Gagnant(2) == 2 )  //  Oui, preneur maître quand il a joué, sinon proba = 0
                    probGC[k-1] = 1.0 - Prob4C;
            }
            else
            {					        //	Preneur entame;
                if ( Gagnant(2) )
                    probGC[k-1] = 1.0;
                else
                    probGC[k-1] = Prob4C;
            }
		}
        //	Trie maintenant avec le plus fort en tête
		for ( i = 0; i < 21; i++)
		{
			for ( k = i+1; k < 21; k++)
			{
				if ( probGC[i] < probGC[k] )
				{
					Tmp = probGC[i];
					probGC[i] = probGC[k];
					probGC[k] = Tmp;
					iTmp = IndexCoup[i];
					IndexCoup[i] = IndexCoup[k];
					IndexCoup[k] = iTmp;
				}
			}
		}
        //	Accumule les probas en tenant compte du passé (ProbC1)
        //			ProbC1 = 1.0;
        //			Prob3C = 0.0;
		for ( i = 0; i < 21; i++ )
		{
			Prob3C += pJeu->TmpProbCarte[posk][IndexCoup[i]] * probGC[i] * ProbC1;
            ProbC1 *= 1.0 - pJeu->TmpProbCarte[posk][IndexCoup[i]];
		}
        //	Cas où le dernier sera tout seul, simule excuse en 3ème place
		MakeCarte(&Table[2], 0);
		Prob4C = 1.0;
		ProbC = 1.0;
		if ( CouleurDemandee > ATOUT )
		{
			for ( i = Startof[CouleurDemandee]; i < Endof[CouleurDemandee]; i++)
			{
				if ( Table[0].Index == i ) continue;
				if ( pJeu->TmpProbCarte[posj][i] < 1e-7 ) continue;
				MakeCarte(&Table[3], i);
				gagnant = Gagnant(3);
				ProbC *= 1.0 - pJeu->TmpProbCarte[posj][i];
				if ( gagnant == 3 )
					Prob4C *= 1.0 - pJeu->TmpProbCarte[posj][i];
			}
		}
		for ( i = 1; i < 22; i++)
		{
			if ( Table[0].Index == i ) continue;
			if ( pJeu->TmpProbCarte[posj][i] < 1e-7 ) continue;
			MakeCarte(&Table[3], i);
			gagnant = Gagnant(3);
			if ( gagnant == 3 )
				Prob4A *= 1.0 - pJeu->TmpProbCarte[posj][i];
		}
		//	Proba que le joueur 4 soit maître
		Prob4C = (1.0 - Prob4C) + ProbC*(1.0 - Prob4A);
		//	Prob3B donne la probabilite que le preneur ne soit pas maitre
		if ( posP == 3 )		//	Preneur en dernier => prob idem Prob4C
			Prob3C += ProbC1 * (1.0 - Prob4C);
		else if ( posP == 0 )		//	Preneur en premier => Le dernier ne doit pas être maître
		{
			Prob3C += ProbC1 * Prob4C;
		}
		if (posP == 2 )
			ProbP = 1.0 - Prob3C;
		else
			ProbP = Prob3C;
		return(ProbP);
	default:
	    assert(0);
		return(0.0);
	}
}



//	Retourne le nombre de points possibles sur un pli en défausse chez les partenaires.

double EvalPointsDefausse(TarotGame CurrentGame, struct _Jeu *pJeu)
{
int i, j, c;
double val, Bval;
double PtRestant = 0;

	for ( i = 0; i < 4; i++)
	{
		if ( i == pJeu->PositionJoueur ) continue;
		if ( i == pJeu->PositionPreneur) continue;
		Bval = -1000.0;
		for ( c = TREFLE; c < NB_COULEUR; c++)
		{
			for ( j= Startof[c] + 10; j < Startof[c]+ ROI; j++)
			{
				if ( pJeu->TmpProbCarte[i][j] > 0.01 )
				{
					val = sqrt(pJeu->TmpProbCarte[i][j]) * (j - Startof[c] - 9.0)*2.0 + 1.0;
					if ( val > Bval ) Bval = val;
				}
			}
		}
		PtRestant += Bval;
	}
	return(PtRestant);
}



#if DEBUG > 0
#define DEBUG_DEFENSE_GAGNANT 0
#else
#define DEBUG_DEFENSE_GAGNANT 0
#endif  // DEBUG

//	Jeu de la défense quand le pli est possible.
//	Décide en particulier de mettre les "gros" a bon escient
//  Retourne l'index de la carte à jouer ou -1 si ne sais pas

int JoueDefenseGagnant(TarotGame CurrentGame, struct _Jeu *pJeu)
{
double ProbGain;
int MinPliP;
int i, j;
int Best = -1;
double Bval = -10000;
double val;
double Avgl;
double pd;
int idxPos = (pJeu->PositionJoueur - CurrentGame->JoueurEntame) & 3;
int PosPreneur = (pJeu->PositionPreneur - CurrentGame->JoueurEntame) & 3;
int CouleurDemandee;
double PtRestant;
int nb;
int Maitre;

	CouleurDemandee = Table[0].Couleur;
	if ( CouleurDemandee == EXCUSE )
	{
        if ( idxPos > 1 )
            CouleurDemandee = Table[1].Couleur;         //  Si excuse en premier, le second fixe la couleur
	    else
            return -1;                                  //  Ne sait pas dans ce cas, le traitement général fera l'affaire
	}
	ProbGain = ProbGainCoup(CurrentGame, pJeu, idxPos, 0);
	MinPliP = MinPlisPreneur(CurrentGame, pJeu);
	if ( ProbGain > 0.001 && MinPliP >= pJeu->NbCarte - 1 )	//	Doit faire tous les autres ...
    {
        //	Le preneur doit faire tous les autres plis, sauve qui peut et joue la carte la plus grosse...
#if DEBUG_DEFENSE_GAGNANT > 0
        switch ( idxPos )
        {
        case 1:
            OutDebug("JoueDefenseGagnant (second)  : ");
            ImprimeCarte(&Table[0]);
            break;
        case 2:
            OutDebug("JoueDefenseGagnant (troisième)  : ");
            ImprimeCarte(&Table[0]);
            ImprimeCarte(&Table[1]);
            break;
        case 3:
            OutDebug("JoueDefenseGagnant (dernier)  : ");
            ImprimeCarte(&Table[0]);
            ImprimeCarte(&Table[1]);
            ImprimeCarte(&Table[2]);
            break;
        }
        OutDebug(" JoueGros : MinPliP = %d, NbCarte = %d\n", MinPliP, pJeu->NbCarte);
#endif // DEBUG_DEFENSE_GAGNANT
        for ( i = 0; i < pJeu->NbCarte; i++)
		{
			if ( CarteValide(pJeu, i, CurrentGame->JoueurEntame) )		//	Peut jouer cette carte
			{
				val = pJeu->MyCarte[i].Valeur;
				if ( val > Bval )
				{
					Bval = val;
					Best = i;
				}
			}
		}
		return(Best);
	}
	//	Cas où il reste un seul atout (le petit) au preneur. Prend la main pour rejouer
	if ( pJeu->NbAtout > 0 && pJeu->NbAtout >= NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT) && CurrentGame->CarteJouee[1] < 0 )
	{
		Maitre = GetPlusForte(pJeu, CouleurDemandee);
		return(Maitre);
	}
	//	Cas où il reste un seul ATOUT sur 2 cartes. Joue la plus grosse de toute manière
	if ( NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT) == 1 && pJeu->NbCarte == 2 && CouleurDemandee != EXCUSE )
	{
		Maitre = GetPlusForte(pJeu, CouleurDemandee);
		return(Maitre);
	}
	//	Cas où le preneur doit encore en avoir...
	//  On joue après le preneur. Si possède 2 cartes maîtresses mais pas de longues, joue la plus grosse.
	Avgl =  AvgLongueur(pJeu, pJeu->PositionPreneur, CouleurDemandee) ;
	if ( idxPos > PosPreneur && CouleurDemandee > ATOUT && Avgl > 0.5 && IsMaitreCouleur(CurrentGame, pJeu, CouleurDemandee) )		//	Possède la carte maitresse
	{
		Maitre = GetPlusForte(pJeu, CouleurDemandee);
		if ( pJeu->MyCarte[Maitre-1].Index == pJeu->MyCarte[Maitre].Index - 1 )		//	Possède les deux cartes maîtresses
		{
			if ( pJeu->NbCouleur[CouleurDemandee] + pJeu->NbJoue[pJeu->PositionJoueur][CouleurDemandee] < 5 )
			{	//	Pas de longue, ne finasse pas...
				return(Maitre);
			}
		}
	}
	//  On joue après le preneur qui doit encore en avoir et a déjà 2 coupes
	if ( idxPos > PosPreneur && CouleurDemandee > ATOUT && Avgl > 0.5
		&& (pJeu->JoueurCouleur[CouleurDemandee] >= 0 || pJeu->NbCoupeInit >= 2) )
	{
		if ( IsMaitreCouleur(CurrentGame, pJeu, CouleurDemandee) )		//	Possède la carte maîtresse
		{
			Maitre = GetPlusForte(pJeu, CouleurDemandee);
			if ( pJeu->MyCarte[Maitre-1].Index == pJeu->MyCarte[Maitre].Index - 1 )		//	Possède les deux cartes maîtresses
			{
                //  Si possède 2 cartes maîtresses, joue la plus grosse si pas de longue
				if ( pJeu->NbCouleur[CouleurDemandee] + pJeu->NbJoue[pJeu->PositionJoueur][CouleurDemandee] < 5 )
				{	//	Pas de longue, ne finasse pas...
					return(Maitre);
				}
			}
			//  "Joue" virtuellement la carte maîtresse, pour regarder si fait le pli
			MakeCarte(&Table[idxPos], pJeu->MyCarte[Maitre].Index);
			j = Gagnant(idxPos);
			if ( j != idxPos && Table[j].Couleur == CouleurDemandee ) return -1; // Ne fait pas le pli, n'insiste pas et garde la carte
			//  Repasse à l'excuse
			MakeCarte(&Table[idxPos], 0 );
			if ( Gagnant(idxPos) != PosPreneur )	//	Gain du coup ?
			{	//	La défense gagne déjà..., peut mettre un "petit"
				if ( Table[PosPreneur].Valeur > 1 )
				{
				    //  Le preneur a mis un honneur, regarde combien il peut en avoir au dessus
					Avgl = 0;
					for ( i = Table[PosPreneur].Index + 1; i < Endof[CouleurDemandee]; i++)
					{
						if ( pJeu->TmpProbCarte[pJeu->PositionPreneur][i] > 0 )
							Avgl += pJeu->TmpProbCarte[pJeu->PositionPreneur][i];
					}
				}
				//	Si x plis possibles dans la couleur, cherche à mettre la xieme moins grosse
				Avgl -= 0.5*(1.0 - 2.0* StyleJoueur[pJeu->PositionJoueur][dStyleDefPoints]);
				PtRestant = PointsRestants(pJeu, pJeu->PositionPreneur, CouleurDemandee);
				if ( PtRestant == 0 )
					Avgl -= 0.5;
                Best = GetPlusForte(pJeu, CouleurDemandee);
				i = Best - 1;
				for ( j = 0; i >= 0; i--)
				{
					if ( j > Avgl ) break;
					if ( pJeu->MyCarte[i].Couleur == CouleurDemandee )
					{
						if ( pJeu->MyCarte[i].Valeur > 1 )
							j++;
						Best = i;
					}
				}
				return(Best);
			}
			else
			{	//	Essaie de gagner le pli, sans trop en faire...
				Maitre = GetPlusForte(pJeu, CouleurDemandee);
				if ( pJeu->NbCouleur[CouleurDemandee] == 1 )            //	Si n'en a qu'une seule...
					return(Maitre);
				//	Regarde si possède toutes les cartes maîtresses
				for ( i = 1; i < pJeu->NbCarte - MinPliP; i++)
				{
					j = Maitre - i;
					for ( j = pJeu->MyCarte[j].Index; j < pJeu->MyCarte[Maitre].Index; j++)
					{
						if ( pJeu->TmpProbCarte[pJeu->PositionPreneur][j] > 0.001 ) break;
					}
					if ( j < pJeu->MyCarte[Maitre].Index ) break;   //  Si vrai, le preneur possède peut-être une carte entre celle-ci et la carte maîtresse
				}
				if ( i >= pJeu->NbCarte - MinPliP )
					return(Maitre);
				//	Joue au plus juste, ou au moins essaie de le faire
				val = 0;
				//	Compte les points "sur la table"
				for ( i = 0; i < idxPos; i++)
					val += Table[i].Valeur;
				val -= 3;
				for ( i = (pJeu->PositionJoueur+1)&3; i != CurrentGame->JoueurEntame; i = (i+1) & 3)
				{	//	Calcule la proba de défausse
					pd = 1.0;
					for ( j = Startof[CouleurDemandee]; j < Endof[CouleurDemandee]; j++)
					{
						pd *= 1.0 - pJeu->TmpProbCarte[i][j];
					}
					for ( j = 1; j < 22; j++)	//	Idem a l'atout
					{
						pd *= 1.0 - pJeu->TmpProbCarte[i][j];
					}
					val += 0.5 * pd * ( 9.0 - 2.0*pJeu->NbDefausse[i] );
				}
				//	Compte les cartes restantes dans la couleur
				nb = 0;
				for ( int c1 = 0; c1 < idxPos; c1++ )
				{
					if ( Table[c1].Couleur == CouleurDemandee ) nb++;
				}
				nb = NbReste(CurrentGame, pJeu->PositionJoueur, CouleurDemandee) - nb;
				PtRestant = CalcPointRestant(CurrentGame, pJeu, CouleurDemandee, idxPos, PosPreneur);
				Bval = PtRestant * ( 1 - 2.0 * StyleJoueur[pJeu->PositionJoueur][dStyleDefPoints] );
				if ( nb > pJeu->NbCouleur[CouleurDemandee] ) nb = pJeu->NbCouleur[CouleurDemandee];
				if ( Table[PosPreneur].Couleur != CouleurDemandee )
					nb = 0;
				else if ( Table[PosPreneur].Hauteur >= VALET && nb > 1)
					nb = 1;
				if ( nb >= 1)
				{
					Bval += EvalPointsDefausse(CurrentGame, pJeu);
				}
				for ( i = pJeu->NbCarte-1; i >= 0; i--)
				{
					if ( pJeu->MyCarte[i].Couleur == CouleurDemandee ) break;
				}
				MakeCarte(&Table[idxPos], pJeu->MyCarte[i].Index);
				if ( Gagnant(idxPos) == idxPos && val > Bval)
					Best = i;
				i--;
				Bval /= 2.0;
				for ( j = 1; i >= 0; i--)
				{
					if ( pJeu->MyCarte[i].Couleur == CouleurDemandee )
					{
						MakeCarte(&Table[idxPos], pJeu->MyCarte[i].Index);
						if ( Gagnant(idxPos) == idxPos && val > Bval)
							Best = i;
						if ( Gagnant(idxPos) == idxPos && (i == 0 || pJeu->MyCarte[i-1].Couleur != CouleurDemandee))
							Best = i;
						if ( pJeu->MyCarte[i].Valeur == 1 ) continue;
						j++;
						Bval /= 2.0;
					}
					if ( j > Avgl ) break;
				}
				return(Best);
			}
		}
		return(-1);
	}
	//	Joue avant le preneur en possédant la carte maîtresse
	if ( idxPos < PosPreneur && CouleurDemandee > ATOUT && Avgl > 0.5
		&& (pJeu->JoueurCouleur[CouleurDemandee] >= 0 || pJeu->NbCoupeInit >= 2) )
	{
		if ( IsMaitreCouleur(CurrentGame, pJeu, CouleurDemandee) )		//	Possède la carte maîtresse
		{
			for ( i = Startof[CouleurDemandee] + 10; i < Endof[CouleurDemandee]; i++)
			{
				if ( pJeu->TmpProbCarte[pJeu->PositionPreneur][i] > 0.1 )
				{
					return(GetPlusForte(pJeu, CouleurDemandee));
				}
			}
			i = GetPlusForte(pJeu, CouleurDemandee);
			if (i > 0)	//	Recherche si possède celle d'en dessous...
			{
				for ( j = pJeu->MyCarte[i-1].Index+1; j < pJeu->MyCarte[i].Index; j++)
				{
					if ( CurrentGame->CarteJouee[j] >= 0 ) continue;
					break;
				}
				if ( j == pJeu->MyCarte[i].Index ) //	Possède les deux
				{
					return(i);
				}
			}
		}
	}
	return(-1);
}


#if DEBUG > 0
#define DEBUG_DEFAUSSE_DEFENSE 0
#else
#define DEBUG_DEFAUSSE_DEFENSE 0
#endif  // DEBUG


//	Défausse quand le pli est sur d'être perdu
//	Joue plutôt les petites cartes dans les couleurs coupées par le preneur
//	Evite de trop défausser dans une couleur pour protéger ses honneurs.
//	Ne défausse pas dans les couleurs où susceptible de faire des plis.
//	Défausse éventuellement dans des courtes même non coupées par le preneur pour
//	pouvoir défausser des grosses ensuite.

int DefPerdante(TarotGame CurrentGame, struct _Jeu *pJeu)
{
int i;
int c;
double Bval;
double TousUtiles;
double pp = 0;
double val;
int nbnh;
#if DEBUG_DEFAUSSE_DEFENSE > 0
int idxPos = (pJeu->PositionJoueur - CurrentGame->JoueurEntame)&3;
#endif
int Best = -1;

	Bval = -1000000.0;
	for ( i = 0; i < pJeu->NbCarte; i++)
	{
		c = pJeu->MyCarte[i].Couleur;
#if DEBUG_DEFAUSSE_DEFENSE > 0
        switch ( idxPos )
        {
        case 1:
            OutDebug("Défausse Perdant second : ");
            ImprimeCarte(&Table[0]);
            break;
        case 2:
            OutDebug("Défausse Perdant troisième : ");
            ImprimeCarte(&Table[0]);
            ImprimeCarte(&Table[1]);
            break;
        case 3:
            OutDebug("Défausse Perdant dernier : ");
            ImprimeCarte(&Table[0]);
            ImprimeCarte(&Table[1]);
            ImprimeCarte(&Table[2]);
            break;
        }
        ImprimeCarte(&pJeu->MyCarte[i]);
#endif // DEBUG_DEFAUSSE_DEFENSE
		if ( c == EXCUSE )
		{   //  Cas particulier excuse. Valeur 0, ne rapporte rien mais pas mauvais non plus...
			val = 0.0;
#if DEBUG_DEFAUSSE_DEFENSE > 0
            OutDebug(", val : %.2f \n", val);
#endif // DEBUG_DEFAUSSE_DEFENSE
		}
		else
		{
			pp = PlisPossibles(CurrentGame, pJeu, c, &TousUtiles);
			if ( IsMaitre(CurrentGame, pJeu, &pJeu->MyCarte[i])) TousUtiles = 1;
			nbnh = NbNonHonneur(CurrentGame, pJeu->PositionJoueur, c);
			if ( nbnh == 0 ) nbnh = 1;
			if ( pJeu->ProbCoupe[c] > 0.25 && pp > 1.0) pp = 1.0;
			val = (-1.0 + 1.0/(pJeu->NbCarte+1.0)) * pJeu->MyCarte[i].Valeur + 0.75/pJeu->NbCouleur[c]
                +  pJeu->ProbCoupe[c]*2.0/pJeu->NbCarte*nbnh + 2.5*pJeu->ProbCoupe[c]
				- (2.0 + 8.0*TousUtiles)*(1.0-pJeu->ProbCoupe[c])*sqrt(pp)
				- 0.2/pJeu->NbCouleur[c]*pJeu->ProbCouleur[c]*GetCartePlusForte(pJeu, c)->Valeur*ProbaMaitreCouleur(CurrentGame, pJeu, pJeu->PositionPreneur, c);
			//	Si aucune chance de faire des plis dans cette couleur, se défausse...
			if ( pJeu->NbCouleur[c] <= 2 && ComptePlusGrand(CurrentGame, c, GetCartePlusForte(pJeu, c)->Hauteur) >= 3*(pJeu->NbCouleur[c] - 1)+1 )
				val += (5.0/pJeu->NbCouleur[c])/sqrt(pJeu->MyCarte[i].Valeur);
			//	Ajoute des points si seule carte maitre dans la couleur
			if ( pJeu->NbCouleur[c] == 1 && pJeu->ProbCoupe[c] < 0.1 && IsMaitre(CurrentGame, pJeu, &pJeu->MyCarte[i]) )
				val -= 15.0 * pJeu->ProbCouleur[c];
			//	Si possède beaucoup plus de cartes que le preneur dans cette couleur, peut en sacrifier
			if ( pJeu->NbCouleur[c] > AvgLongueur(pJeu, pJeu->PositionPreneur, c)*1.25 + 1.0 )
			{
				if ( i == GetPlusFaible(pJeu, c) ) val += 5.0;
			}
			//	Signalisation des defausses. Jette les couleurs dont ne veut pas...
			if ( FlagSignalisation )
			{
				if ( pJeu->MyCarte[i].Valeur == 1 && pp == 0 && (pJeu->DefausseCouleurParJoueur[pJeu->PositionJoueur] & (1 << c)) == 0 )
					//	Pas encore de défausse dans cette couleur où ne fera rien...
				{
					val += 4.0;		//	Aide à la défausse...
				}
			}
#if DEBUG_DEFAUSSE_DEFENSE > 0
            OutDebug(", val : %.2f = (-1 + 1.0/%d)*%d + 0.75/%d +  2.0*%.2f/%d*%d + 2.5*%.2f \n    - 2.0 * (1.0 - %.2f) * %.2f - 8.0* %.2f * (1.0 - %.2f) * %.2f - 0.2/%d*%.2f*%d*%.2f\n"
                , val, pJeu->NbCarte+1, pJeu->MyCarte[i].Valeur, pJeu->NbCouleur[c], pJeu->ProbCoupe[c], pJeu->NbCarte, nbnh, pJeu->ProbCoupe[c]
                , pJeu->ProbCoupe[c], pp, TousUtiles, pJeu->ProbCoupe[c], pp
                , pJeu->NbCouleur[c], pJeu->ProbCouleur[c], GetCartePlusForte(pJeu, c)->Valeur, ProbaMaitreCouleur(CurrentGame, pJeu, pJeu->PositionPreneur, c));
#endif // DEBUG_DEFAUSSE_DEFENSE
		}
		if ( val > Bval )
		{
			Best = i;
			Bval = val;
		}
	}
	assert(Best >= 0);
	return(Best);
}

//	Joue la défausse quand la défense est sure de faire le pli
//	Défausse plutôt des grosses cartes dans les couleurs coupées par le preneur.
//	Ne défausse pas dans les couleurs où susceptible de faire des plis.

int DefGagnant(TarotGame CurrentGame, struct _Jeu *pJeu)
{
int i;
int c;
double Bval;
double TousUtiles;
double pp = 0;
double pc;
double val;
int Best = -1;
double Maitre = 0;
double NbA;
int idxPos = (pJeu->PositionJoueur - CurrentGame->JoueurEntame)&3;
int nbnh;
double v;
int d;
int MaxPlisDefense = pJeu->NbCarte - MinPlisPreneur(CurrentGame, pJeu);

	Bval = -1000000.0;
	for ( i = 0; i < pJeu->NbCarte; i++)
	{
		c = pJeu->MyCarte[i].Couleur;
#if DEBUG_DEFAUSSE_DEFENSE > 0
        switch ( idxPos )
        {
        case 1:
            OutDebug("DéfausseGagnant_second : ");
            ImprimeCarte(&Table[0]);
            break;
        case 2:
            OutDebug("DéfausseGagnant_troisieme : ");
            ImprimeCarte(&Table[0]);
            ImprimeCarte(&Table[1]);
            break;
        case 3:
            OutDebug("DéfausseGagnant_dernier : ");
            ImprimeCarte(&Table[0]);
            ImprimeCarte(&Table[1]);
            ImprimeCarte(&Table[2]);
            break;
        }
        ImprimeCarte(&pJeu->MyCarte[i]);
#endif // DEBUG_DEFAUSSE_DEFENSE
		if ( c == EXCUSE )
		{
			val = 0.0;
#if DEBUG_DEFAUSSE_DEFENSE > 0
            OutDebug(", val : %.2f \n", val);
#endif // DEBUG_DEFAUSSE_DEFENSE
		}
		else
		{
			pp = PlisPossibles(CurrentGame, pJeu, c, &TousUtiles);
			Maitre = 0.0;
			if ( IsMaitre(CurrentGame, pJeu, &pJeu->MyCarte[i]))
			{
				TousUtiles = 1;
				Maitre = 1.0;
			}
			if ( NbMaitreCouleur(CurrentGame, pJeu, c) > pp )
                TousUtiles = 0;         //  Plus de cartes que de plis possibles ?
			if ( AvgLongueur(pJeu, pJeu->PositionPreneur, ATOUT) < 0.001 && Maitre > 0 )
				Maitre = (1.0 + 3.0*pJeu->ProbCarte[pJeu->PositionPreneur][pJeu->MyCarte[i].Index-1]) ;
            //  Calcul pc : proba coupe
			pc = pJeu->ProbCoupe[c];
			if ( pJeu->JoueurCouleur[c] < 0 && pc > 0.15)		//	Pas encore ouvert
				pc += (1.0 - pc)/pJeu->NbCarte;
            //  Modifie pc avec proba faire des plis. Pas la peine de garder un honneur si ne peut faire de plis
            if ( pc < 1 )
            {
                v = AvgLongueur(pJeu, pJeu->PositionPreneur, c);
                d = ROI - GetCartePlusForte(pJeu, c)->Hauteur;      //  Nombre de cartes avant de faire un pli
                if ( v < d + 1 )
                {
                    pc += d + 1 - v;
                    if ( pc > 1 ) pc = 1;
                }
            }
			NbA = AvgLongueur(pJeu, pJeu->PositionPreneur, ATOUT);			//	Nombre d'atouts du preneur
			if ( ((pJeu->PositionPreneur-CurrentGame->JoueurEntame) & 3) > idxPos )	//	Avant le preneur
			{
				if ( Table[0].Couleur <= ATOUT )
					NbA--;
				else
					NbA -= pJeu->ProbCoupe[Table[0].Couleur];
			}
			if ( NbA < 0 ) NbA = 0.0;
			if ( pJeu->ProbCoupe[c] > 0.25 && pp > 1.0)
			{
				if ( NbA > pp - 1)
					pp = 1.0;
				else
					pp -= NbA;
			}
			nbnh = NbNonHonneur(CurrentGame, pJeu->PositionJoueur, c);
			if ( nbnh > 3 ) nbnh = 3;       //  Limite importance du facteur
			if ( nbnh > MaxPlisDefense ) nbnh = MaxPlisDefense;
			val = 1.0 * pJeu->MyCarte[i].Valeur + 0.75/pJeu->NbCouleur[c] -  0.5*nbnh - 0.1*pJeu->NbCouleurChien[c]
				+ 2.0*pc - (1.0 + 6.0*TousUtiles)*(1.0-pc)*sqrt(pp) - Maitre*PointsRestants(pJeu, pJeu->PositionPreneur, c)*(1.0 - StyleJoueur[pJeu->PositionJoueur][dStyleDefDefausse])
				- Maitre* pJeu->MyCarte[i].Valeur *(1-pc)/(1.0 + NbA)*AvgLongueur(pJeu, pJeu->PositionPreneur, c);
			//	Signalisation des défausses. Jette les couleurs dont ne veut pas...
			if ( FlagSignalisation )
			{
				if ( pJeu->MyCarte[i].Valeur == 1 && pp == 0 && (pJeu->DefausseCouleurParJoueur[pJeu->PositionJoueur] & (1 << c)) == 0 )
					//	Pas encore de défausse dans cette couleur où ne fera rien...
				{
                    if ( MaxPlisDefense > 3 )
                        val += 1.5;		//	Aide à la défausse... La valeur doit être inférieure à la différence avec un valet cependant
				}
			}
#if DEBUG_DEFAUSSE_DEFENSE > 0
            OutDebug(", val =  %.2f\n", val);
            OutDebug("    = pJeu->MyCarte[i].Valeur : %d\n", pJeu->MyCarte[i].Valeur);
            OutDebug("    + 0.75/pJeu->NbCouleur[c] : %.2f\n", 0.75/pJeu->NbCouleur[c]);
            OutDebug("    - 0.5*min(NbNonHonneur(CurrentGame, pJeu->PositionJoueur, c),3) : %.2f\n", 0.5*nbnh);
            OutDebug("    - 0.1*pJeu->NbCouleurChien[c] : %.2f\n", 0.1*pJeu->NbCouleurChien[c]);
            OutDebug("    + 2.0*pc : %.2f\n", 2.0*pc);
            OutDebug("    - (1.0 + 6.0*TousUtiles)*(1.0-pc)*sqrt(pp) : (1.0 + 6.0*%.2f)*(1-0-%.2f)*sqrt(%.2f) -> %.3f\n", TousUtiles, pc, pp, (1.0 + 6.0*TousUtiles)*(1.0-pc)*sqrt(pp));
            OutDebug("    - Maitre*PointsRestants(pJeu, pJeu->PositionPreneur, c) : %.2f*%.2f-> %.3f\n", Maitre, PointsRestants(pJeu, pJeu->PositionPreneur, c), Maitre*PointsRestants(pJeu, pJeu->PositionPreneur, c));
            OutDebug("    - Maitre* pJeu->MyCarte[i].Valeur *(1-pc)/(1.0 + NbA)*AvgLongueur(pJeu, pJeu->PositionPreneur, c) : %.2f*%d*(1-%.2f)/(1.0+%.2f)*%.2f-> %.3f\n",
                                Maitre, pJeu->MyCarte[i].Valeur, pc, NbA, AvgLongueur(pJeu, pJeu->PositionPreneur, c), Maitre* pJeu->MyCarte[i].Valeur *(1-pc)/(1.0 + NbA)*AvgLongueur(pJeu, pJeu->PositionPreneur, c));
#endif // DEBUG_DEFAUSSE_DEFENSE
		}
		if ( val > Bval )
		{
			Best = i;
			Bval = val;
		}
	}
	assert(Best >= 0);
	return(Best);
}

//	Défausse plutôt des grosses cartes dans les couleurs coupées par le preneur.
//	La valeur des cartes dépend de la probabilité de gain...
//	Ne défausse pas dans les couleurs où susceptible de faire des plis.

int DefGeneral(TarotGame CurrentGame, struct _Jeu *pJeu, double ProbGain, int MinPliP)
{
int i;
int c;
double Bval;
double TousUtiles;
double pp = 0;
double val;
int Best = -1;
int TotHonneur;
int HonneurParCouleur[4];
int MinPliD = 0;
int NbPA;
double NbA;
double x;
int idxPos = (pJeu->PositionJoueur - CurrentGame->JoueurEntame)&3;

	Bval = -1000000.0;
	TotHonneur = CompteHonneur(CurrentGame, pJeu->PositionJoueur, HonneurParCouleur);
	if ( AvgLongueur(pJeu, pJeu->PositionPreneur, ATOUT) < 0.001 )
	{
		NbPA = NbPossCouleur(pJeu, ATOUT);
		if ( NbPA )
			MinPliD = (NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT) + NbPA - 1) / NbPA;
		if ( MinPliD >= TotHonneur )
			ProbGain *= 0.5;
	}
	for ( i = 0; i < pJeu->NbCarte; i++)
	{
		c = pJeu->MyCarte[i].Couleur;
#if DEBUG_DEFAUSSE_DEFENSE > 0
        switch ( idxPos )
        {
        case 1:
            OutDebug("Défausse Général second : ");
            ImprimeCarte(&Table[0]);
            break;
        case 2:
            OutDebug("Défausse Général troisième : ");
            ImprimeCarte(&Table[0]);
            ImprimeCarte(&Table[1]);
            break;
        case 3:
            OutDebug("Défausse Général dernier : ");
            ImprimeCarte(&Table[0]);
            ImprimeCarte(&Table[1]);
            ImprimeCarte(&Table[2]);
            break;
        }
        ImprimeCarte(&pJeu->MyCarte[i]);
#endif // DEBUG_DEFAUSSE_DEFENSE
		if ( c == EXCUSE )
		{
			val = 0.0;
		}
		else
		{
			pp = PlisPossibles(CurrentGame, pJeu, c, &TousUtiles);
			if ( IsMaitre(CurrentGame, pJeu, &pJeu->MyCarte[i])) TousUtiles = 1;
			NbA = AvgLongueur(pJeu, pJeu->PositionPreneur, ATOUT);			//	Nombre d'atouts du preneur
			if ( ((pJeu->PositionPreneur-CurrentGame->JoueurEntame) & 3) > idxPos )	//	Avant le preneur
			{
				if ( Table[0].Couleur <= ATOUT )
					NbA--;
				else
					NbA -= pJeu->ProbCoupe[Table[0].Couleur];
			}
			if ( pJeu->ProbCoupe[c] > 0.25 && pp > 1.0)
			{
				if ( NbA > pp - 1)
					pp = 1.0;
				else
					pp -= NbA;
			}
			x = pJeu->MyCarte[i].Valeur*((sqrt(1.0*pJeu->NbCarte)-2)*(1.0-ProbGain)+1.0)  - 0.25 - 9.0*ProbGain - 2.0/(pJeu->NbCarte - MinPliP - TotHonneur);
			val = -0.2*pow(x, 2.0)
				+ 5.0 + 0.75/pJeu->NbCouleur[c] -  (ProbGain - 0.5)*NbNonHonneur(CurrentGame, pJeu->PositionJoueur, c)
				+ 2.0*pJeu->ProbCoupe[c] - (1.0 + 10.0*TousUtiles)*(1.0-pJeu->ProbCoupe[c])*sqrt(pp);
			//	Signalisation des defausses. Jette les couleurs dont ne veut pas...
			if ( FlagSignalisation )
			{
				if ( pJeu->MyCarte[i].Valeur == 1 && pp == 0 && (pJeu->DefausseCouleurParJoueur[pJeu->PositionJoueur] & (1 << c)) == 0 )
					//	Pas encore de défausse dans cette couleur où ne fera rien...
				{
					val += 4.0;		//	Aide à la défausse...
				}
			}
#if DEBUG_DEFAUSSE_DEFENSE
            OutDebug(", val : %.2f = -0.2*(%d*((sqrt(%d)-2)*(1.0-%.2f)+1) - 0.25 - 9*%.2f - 2/%d[=%.2f])**2 + 5 +  0.75/%d - (%.2f- 0.5)*%d + 2.0*%.2f - (1.0 + 6*%.2f)*(1.0-%.2f)*%.2f\n"
                , val, pJeu->MyCarte[i].Valeur, pJeu->NbCarte, ProbGain, ProbGain, (pJeu->NbCarte - MinPliP - TotHonneur), x, pJeu->NbCouleur[c]
                 , ProbGain, NbNonHonneur(CurrentGame, pJeu->PositionJoueur, c), pJeu->ProbCoupe[c], TousUtiles, pJeu->ProbCoupe[c], pp);
#endif // DEBUG_DEFAUSSE_DEFENSE
		}
		if ( val > Bval )
		{
			Best = i;
			Bval = val;
		}
	}
	return(Best);
}



