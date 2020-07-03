#include <stdlib.h>
#include <gtk/gtk.h>
#include "Tarot_Ui_Objects.h"
#include <assert.h>
#include <math.h>


//  Initialise la structure pJeu pour le joueur au début de la partie.
//  Certains champs dont les probabilités sont déja remplis (appelé après InitProba)
//  Ne traite que les champs qui doivent être non nuls, les autres sont déjà mis à 0.

void InitStructJeu(TarotGame CurrentGame, struct _Jeu *pJeu)
{
int c;
int i;

    pJeu->NbCarte = 18;                 //  Nombre de cartes en main pour le joueur
    pJeu->CouleurTenue = -1;            //  Ne tient rien par défaut
    pJeu->Flanc = -1;                   //  Ne sait pas encore
    pJeu->PositionPreneur = CurrentGame->JoueurPreneur;
    for ( c = 0; c < NB_COULEUR; c++)
    {
        pJeu->JoueurCouleur[c] = -1;
        //  Init couleurs vues au chien
        for ( i = Startof[c]; i < Endof[c]; i++ )
            if ( CurrentGame->CarteAuChien[i] ) pJeu->NbCouleurChien[c]++;
    }
	pJeu->JoueurMainForte = -1;
	pJeu->ResteAtout = 22 - pJeu->NbAtout;
	CopieProba2Tmp(pJeu);
	pJeu->AtoutPreneur = AvgLongueur(pJeu, pJeu->PositionPreneur, ATOUT);
}

//	Retourne la longueur moyenne de la couleur c du joueur Joueur (cartes restantes)
//	Cette longueur est la somme des probabilités de possession des cartes de la couleur.
//  Calcul vu du joueur dont la struct _Jeu est passée en paramètre

double LongueurMoyenneCouleur(struct _Jeu *pJeu, int Joueur, int c)
{
double l = 0;
int i;

	for ( i = Startof[c]; i < Endof[c]; i++)
	{
		l += pJeu->TmpProbCarte[Joueur][i];
	}
	return(l);
}

//	Répond 1 si la carte du numéro passe en parametre est jouee

int IsJoue(TarotGame CurrentGame, int i)
{
int j;

    if ( CurrentGame->CarteJouee[i] >= 0 ) return 1;     //  Oui dans les plis passés
	for ( j = 0; j < 4; j++)
	{
		if ( CurrentGame->CartePli[j] == i ) return(1);     //  Oui dans le plis en cours
	}
	return(0);                                      //  Non pas encore joué
}

//	Retourne 1 si le Joueur a encore la carte IndexCarte passée en paramètre dans son jeu (proba sûre ou quasiment)
//  Répond vu du joueur dont la structure _Jeu est passée en paramètre

int HasCarte(struct _Jeu *pJeu, int joueur, int IndexCarte)
{
	return(fabs(pJeu->ProbCarte[joueur][IndexCarte] - 1.0) < ERREUR_PROBA_SURE);
}

//  Retourne le possesseur le plus probable du petit, vu du joueur dont la structure _Jeu est passée en paramètre
//  Utilise les probas temporaires
//  Si le petit a déjà été joué retourne -1


int JoueurAvecPetit(TarotGame CurrentGame, struct _Jeu *pJeu)
{
int j;
double pMax = 0;
int pos = -1;

    if ( IsJoue(CurrentGame, 1) ) return -1;
    for ( j = 0; j < 4; j++ )
    {
        if ( pJeu->TmpProbCarte[j][1] > pMax )
        {
            pMax = pJeu->TmpProbCarte[j][1];
            pos = j;
        }
    }
    return pos;
}


//	Si Preneur : Retourne le nombre d'adversaires possédant des cartes de la couleur c
//	Si Défense : Retourne le nombre de partenaires possédant des cartes de la couleur c
//	Si le pointeur est non nul, met à jour le joueur qui en possède

#define SEUIL_PROBA 1e-7

int NbPossCouleur(struct _Jeu *pJeu, int Couleur)
{
int nb = 0;
int nb1;
int i;

	for ( i = 0; i < 4; i++)
	{
		if ( i == pJeu->PositionJoueur ) continue;
		if ( i == pJeu->PositionPreneur ) continue;
		nb1 = 0;
		for ( int j = Startof[Couleur]; j < Endof[Couleur]; j++)
		{
			if ( pJeu->TmpProbCarte[i][j] > SEUIL_PROBA )
			{
				nb1++;
			}
		}
		if ( nb1 ) nb++;
	}
	return(nb);
}

//	Retourne le nombre d'atouts du joueur Position parmi les N plus gros atouts non joués
//  Ne tient compte que des probabilités élevées (> 0.95)

int NbGrosAtout(TarotGame CurrentGame, struct _Jeu *pJeu, int Position, int N)
{
int h = 22;
int nb = 0;


    while ( --h > 0 && N > 0 )
    {
        if ( CurrentGame->CarteJouee[h] >= 0 ) continue;        //  Déjà jouée, passe au suivant
        if ( pJeu->TmpProbCarte[Position][h] > 0.95 )
            nb++;                                               //  Oui, un de plus
        N--;
    }
	return(nb);
}

//	Retourne N si le joueur possède un barrage dans la couleur.
//	Pour cela il faut qu'il possède la carte maîtresse (retourne 1)
//  ou que sa plus forte soit maître après N coups. retourne 1/N

double HasBarrage(TarotGame CurrentGame, struct _Jeu *pJeu, int Couleur)
{
int N = 0;
int idxCarte;
int j;

	idxCarte = GetPlusForte(pJeu, Couleur);
	if ( IsMaitre(CurrentGame, pJeu, &pJeu->MyCarte[idxCarte]) ) return(1);
	for ( j = pJeu->MyCarte[idxCarte].Index+1; j < Endof[Couleur]; j++)
	{
		if ( IsJoue(CurrentGame, j) == 0 ) N++;
	}
	if ( N <= pJeu->NbCouleur[Couleur] ) return(1.0/N);
	return(0.0);
}

//  Retourne les points restant à jouer dans la couleur Couleur

double CalcPointRestantAJouer(TarotGame CurrentGame, int Couleur)
{
double val = 0;
int i;

    //  Compte tous les points restant à jouer
    for ( i = Startof[Couleur]+10; i < Startof[Couleur] + ROI; i++)
    {
        if ( CurrentGame->CarteJouee[i] < 0 )       //  Pas encore jouée ?
            val += (2.0 * ( i - Startof[Couleur] - 9 ) + 1.0);
    }
    return(val);
}

//	Retourne les points restants chez les joueurs différents du gagnant inférieurs à la carte gagnante dans une couleur
//	Si le gagnant est le preneur, retourne tous les points de la défense
//	Si le gagnant est défenseur, retourne les points du preneur et la moitié des points de la défense.
//  PosP donne la position du preneur sur la table (par rapport à l'entame)
//  IdxGagnant donne la position relative sur la table (par rapport à Entame)

double CalcPointRestant(TarotGame CurrentGame, struct _Jeu *pJeu, int Couleur, int idxGagnant, int PosP)
{
double val = 0;
int i;
int PosGagnant = (CurrentGame->JoueurEntame + idxGagnant)&3;
int PosPreneur =  (CurrentGame->JoueurEntame + PosP)&3;


	if ( idxGagnant == PosP )
	{
	//	Le gagnant est le preneur...
		for ( i = Startof[Couleur]+10; i < Startof[Couleur] + Table[idxGagnant].Hauteur - 1; i++)
		{
		    if ( i == Table[0].Index ) continue;        //  Ne compte pas les cartes jouées
		    if ( i == Table[1].Index ) continue;
		    if ( i == Table[2].Index ) continue;
		    if ( i == Table[3].Index ) continue;
            val += pJeu->ProbCarte[PosPreneur^1][i] * (2.0 * ( i - Startof[Couleur] - 9 ) + 1.0);
            val += pJeu->ProbCarte[PosPreneur^2][i] * (2.0 * ( i - Startof[Couleur] - 9 ) + 1.0);
            val += pJeu->ProbCarte[PosPreneur^3][i] * (2.0 * ( i - Startof[Couleur] - 9 ) + 1.0);
		}

		return(val);
	}
	//	Le gagnant est la défense
	for ( i = Startof[Couleur]+10; i < Startof[Couleur] + Table[idxGagnant].Hauteur - 1; i++)
	{
		if ( Table[0].Index == i ) continue;
		if ( Table[1].Index == i ) continue;
		if ( Table[2].Index == i ) continue;
		if ( Table[3].Index == i ) continue;
        val += pJeu->ProbCarte[PosGagnant^1][i] * (2.0 * ( i - Startof[Couleur] - 9 ) + 1.0) * ( 1.0 - 0.5 * ((PosGagnant^1) != PosPreneur));
        val += pJeu->ProbCarte[PosGagnant^2][i] * (2.0 * ( i - Startof[Couleur] - 9 ) + 1.0) * ( 1.0 - 0.5 * ((PosGagnant^2) != PosPreneur));
        val += pJeu->ProbCarte[PosGagnant^3][i] * (2.0 * ( i - Startof[Couleur] - 9 ) + 1.0) * ( 1.0 - 0.5 * ((PosGagnant^3) != PosPreneur));
	}
	//	Si le gagnant met la carte juste au dessus du preneur, retire des points restants
	if ( Table[idxGagnant].Index == Table[PosP].Index + 1) return(val * 0.3);
	return(val);
}




//	Calcule le nombre de plis minimal restant à faire au preneur
//  Calcule vraiment le minimum, suppose tous les forts dans un même jeu défense.

int MinPlisPreneur(TarotGame CurrentGame, struct _Jeu *pJeu)
{
int c, nb;
int i;
int MinPlisAtout = 0;
int MinPlisC;
int MinPlis = 0;
int MaxAtoutPartenaire = 0;
int Position = pJeu->PositionJoueur;
int Preneur = pJeu->PositionPreneur;

    //	Regarde les plis maîtres à l'atout
    //  Calcule d'abord la longueur max de la défense à l'atout (MaxAtoutPartenaire)
	if ( (Position^1) != Preneur && (nb = MaxLongueur(pJeu, Position^1, ATOUT)) > MaxAtoutPartenaire) MaxAtoutPartenaire = nb;
	if ( (Position^2) != Preneur && (nb = MaxLongueur(pJeu, Position^2, ATOUT)) > MaxAtoutPartenaire) MaxAtoutPartenaire = nb;
	if ( (Position^3) != Preneur && (nb = MaxLongueur(pJeu, Position^3, ATOUT)) > MaxAtoutPartenaire) MaxAtoutPartenaire = nb;
    //	Joue les atouts du preneur, plus gros en tête et compte le nombre de plis faits
    //	Si le preneur ne possède pas un atout maître, enlève un pour la suite...
	nb = 0;         //  nb compte les atouts maîtres de la défense
	for ( int i = 21; i > 0; i--)
	{
		if ( IsJoue(CurrentGame, i) ) continue;
		if ( pJeu->TmpProbCarte[Preneur][i] >= 0.97)
		{
			if ( nb == 0 )
				MinPlisAtout++;         //  Pli sûr pour le preneur
			else
				nb--;                   //  Enlève un atout maître défense (pour faire ce pli)
		}
		else if ( MaxAtoutPartenaire > 0 )
		{
			nb++;                       //  Atout maître défense
		}
		MaxAtoutPartenaire--;           //  Un de moins en défense
	}
	MinPlis += MinPlisAtout;
    //	Fait la même chose pour chaque couleur
	for ( c = TREFLE; c < NB_COULEUR; c++)
	{
		MinPlisC = 0;
		nb = 0;
		for ( i = Startof[c] + 13; i >= Startof[c]; i--)
		{
			if ( IsJoue(CurrentGame, i) ) continue;
			if ( pJeu->TmpProbCarte[Preneur][i] >= 0.97 )
			{
				if ( nb == 0 )
					MinPlisC++;
				else
					nb--;
			}
			else if ( pJeu->TmpProbCarte[Preneur][i] + pJeu->TmpProbCarte[CHIEN][i] < 0.98 )
			{
				nb++;
			}
		}
		MinPlis += MinPlisC;
	}
	return(MinPlis);
}


void DebutPartie(TarotGame CurrentGame)
{
    InitProbaJoueurs(CurrentGame);              //  Init probas
    CurrentGame->JoueurCourant = (CurrentGame->JoueurDistrib + 1) & 3;
    CurrentGame->JoueurEntame = CurrentGame->JoueurCourant;
#if DEBUG > 0
    OutDebug("Début de partie : Distrib %d, Preneur %d, Contrat %d, Joueur Entame %d\n",
                CurrentGame->JoueurDistrib, CurrentGame->JoueurPreneur, CurrentGame->TypePartie, CurrentGame->JoueurEntame);
    OutDebug("Force du jeu de SUD : %.0f / %.0f\n", CurrentGame->JeuJoueur[SUD].NbPoint, CurrentGame->JeuJoueur[SUD].NbPointDH);
    OutDebug("Force du jeu de EST : %.0f / %.0f\n", CurrentGame->JeuJoueur[EST].NbPoint, CurrentGame->JeuJoueur[EST].NbPointDH);
    OutDebug("Force du jeu de NORD : %.0f / %.0f\n", CurrentGame->JeuJoueur[NORD].NbPoint, CurrentGame->JeuJoueur[NORD].NbPointDH);
    OutDebug("Force du jeu de OUEST : %.0f / %.0f\n", CurrentGame->JeuJoueur[OUEST].NbPoint, CurrentGame->JeuJoueur[OUEST].NbPointDH);
    CheckProbaJeu(CurrentGame, &CurrentGame->JeuJoueur[SUD]);
    CheckProbaJeu(CurrentGame, &CurrentGame->JeuJoueur[EST]);
    CheckProbaJeu(CurrentGame, &CurrentGame->JeuJoueur[NORD]);
    CheckProbaJeu(CurrentGame, &CurrentGame->JeuJoueur[OUEST]);
#endif // DEBUG
    RegardeChelemPreneur(CurrentGame);
}


#if DEBUG > 0
#define DEBUG_EVAL_SECOND 0
#else
#define DEBUG_EVAL_SECOND 0
#endif  // DEBUG


//	Evalue le coup joue par le Second
//	Evalue pour cela les possibilités du 3ème joueur

double EvalCoupSecond(TarotGame CurrentGame, struct _Jeu *pJeu)
{
int posJoueur3 = (CurrentGame->JoueurEntame + 2) & 3;	//	3eme joueur
int i, j;
double ResultatCoup[78];
int IndexCoup[78];
double ProbPasMeilleurCoup = 1.0;
double ValeurCoup = 0.0;
double Tmp;
int iTmp;
double Signe = 1.0;
int CouleurDemandee;
int StartAtout;
double ValeurExcuse = -1e10;
double ValC;


	if ( posJoueur3 == pJeu->PositionPreneur ) Signe = -1.0;    //  Juste avant le preneur. Le joueur 3 sera donc en attaque.
	if ( pJeu->PositionJoueur == pJeu->PositionPreneur ) Signe = -1.0;    //  Joueur 2 = Preneur, le joueur 3 sera dans l'autre camp
    //	Recherche parmi les 78 cartes celles qui sont possibles pour le joueur n° 3
	for ( i = 0; i < 78; i++)
	{
		if ( Table[0].Index == i || Table[1].Index == i )
		{
			ResultatCoup[i] = -1e6;		//	Déjà sur la table, impossible met donc un score très mauvais
		}
		else if ( CurrentGame->CarteJouee[i] >= 0 || pJeu->TmpProbCarte[posJoueur3][i] == 0.0 )		//	Déjà jouée ou impossible ?
		{
			ResultatCoup[i] = -1e6;		//	Déjà joué ou carte pas chez joueur 3, impossible met donc un score très mauvais
		}
		else
		{
			MakeCarte(&Table[2], i);        //  "Joue" virtuellement la carte i
			ResultatCoup[i] = EvalCoupTroisieme(CurrentGame, pJeu);
		}
	}
    //	Trie les coups par groupe en partant du meilleur
	for ( i = 0; i < 78; i++) IndexCoup[i] = i;
	CouleurDemandee = Table[0].Couleur;
	if ( CouleurDemandee == EXCUSE ) CouleurDemandee = Table[1].Couleur;
    //	Groupe 1 : Cas spécial, l'Excuse
	if ( Table[0].Index != 0 && Table[1].Index != 0 && pJeu->TmpProbCarte[posJoueur3][0] > 1e-7 )
	{
		if ( posJoueur3 == pJeu->PositionJoueur && ResultatCoup[0] > -3.0 )		//	Doit se débarasser... de l'excuse
		{	//	Pour cela, augmente valeur du coup si ne perd pas trop...
			//	Augmente encore plus si "économise un atout"
			ResultatCoup[0] += 3.0;
			if ( CouleurDemandee == ATOUT || pJeu->ProbCoupe[CouleurDemandee] > 0.95 )
				ResultatCoup[0] += 2.0;
		}
		ValeurExcuse = pJeu->TmpProbCarte[posJoueur3][0] * ResultatCoup[0];
#if DEBUG_EVAL_SECOND
        OutDebug("EvalCoupSecond : ");
        MakeCarte(&Table[2], 0);
        ImprimeCarte(&Table[0]);
        ImprimeCarte(&Table[1]);
        ImprimeCarte(&Table[2]);
        OutDebug(", ResCoup Exc = %.2f ValeurExc = %.2f   ProbReste = %.2f\n", ResultatCoup[0], ValeurExcuse, ProbPasMeilleurCoup);
#endif // DEBUG_EVAL_SECOND
	}
    //	Groupe 2 : Couleur demandée
	if ( CouleurDemandee > ATOUT )	//	Couleur normale
	{
	    //  Trie les coups dans cette couleur
        //  Les meilleures valeurs vont être au début après le tri
		for ( i = Startof[CouleurDemandee]; i < Endof[CouleurDemandee]; i++)
		{
			for ( j = i+1; j < Endof[CouleurDemandee]; j++)
			{
				if ( ResultatCoup[i] < ResultatCoup[j] )
				{
					Tmp = ResultatCoup[i];
					ResultatCoup[i] = ResultatCoup[j];
					ResultatCoup[j] = Tmp;
					iTmp = IndexCoup[i];
					IndexCoup[i] = IndexCoup[j];
					IndexCoup[j] = iTmp;
				}
			}
		}
        //	Calcule maintenant la valeur du coup en partant du meilleur
        //	ValeurCoup = Somme(TmpProbCarte*ResultatCoup[i]*ProbPasMeilleurCoup)
		if ( posJoueur3 == pJeu->PositionPreneur && ProbPasMeilleurCoup > 0 && CouleurDemandee > ATOUT)
		{
			ProbPasMeilleurCoup = 1.0 - pJeu->TmpProbCoupe[CouleurDemandee];
		}
		if ( ProbPasMeilleurCoup > 0.0 )
		{
			for ( i = Startof[CouleurDemandee]; i < Endof[CouleurDemandee]; i++)
			{
				if ( ProbPasMeilleurCoup <= 0.0 ) break;
				if ( Table[0].Index == IndexCoup[i] || Table[1].Index == IndexCoup[i] ) continue;
				if ( pJeu->TmpProbCarte[posJoueur3][IndexCoup[i]] <= 1e-7 ) continue;
				ValC = ResultatCoup[i] * pJeu->TmpProbCarte[posJoueur3][IndexCoup[i]];
				if ( ValC <= ValeurExcuse )         //  Si moins bon que l'excuse, peut toujours s'excuser à la place. Mais ne marche qu'une fois
				{
					ValeurCoup += ValeurExcuse * ProbPasMeilleurCoup;
					ValeurExcuse = -1e10;
					ProbPasMeilleurCoup *= 1.0 - pJeu->TmpProbCarte[posJoueur3][0];
				}
				ValeurCoup += ValC  * ProbPasMeilleurCoup;
#if DEBUG_EVAL_SECOND
                OutDebug("EvalCoupSecond : ");
                MakeCarte(&Table[2], IndexCoup[i]);
                ImprimeCarte(&Table[0]);
                ImprimeCarte(&Table[1]);
                ImprimeCarte(&Table[2]);
                OutDebug(", ResCoup = %.2f Valeur = %.2f   ProbReste = %.2f\n", ResultatCoup[i], ValeurCoup, ProbPasMeilleurCoup);
#endif // DEBUG_EVAL_SECOND
				ProbPasMeilleurCoup *= 1.0 - pJeu->TmpProbCarte[posJoueur3][IndexCoup[i]];     //  Baisse ProbPasMeilleurCoup (coups les meilleurs joués d'abord)
			}
			if ( posJoueur3 == pJeu->PositionPreneur && ProbPasMeilleurCoup > 0 )
			{
				if (ProbPasMeilleurCoup <= pJeu->TmpProbCoupe[CouleurDemandee])
					ProbPasMeilleurCoup = pJeu->TmpProbCoupe[CouleurDemandee];
			}
		}
		else
		{
			ProbPasMeilleurCoup = 1.0;
		}
	}
    //	Groupe 3 : Atouts plus grands que le MAX (joué par les joueurs 0, 1 et 2)
	StartAtout = PlusFortAtoutJoue(3) + 1;
	for ( i = StartAtout; i < 22; i++)
	{
		for ( j = i+1; j < 22; j++)
		{
			if ( ResultatCoup[i] < ResultatCoup[j] )
			{
				Tmp = ResultatCoup[i];
				ResultatCoup[i] = ResultatCoup[j];
				ResultatCoup[j] = Tmp;
				iTmp = IndexCoup[i];
				IndexCoup[i] = IndexCoup[j];
				IndexCoup[j] = iTmp;
			}
		}
	}
    //	Calcule maintenant la valeur du coup en partant du meilleur
    //	ValeurCoup = Somme(TmpProbCarte*ResultatCoup[i]*ProbPasMeilleurCoup)
	for ( i = StartAtout; i < 22; i++)
	{
		if ( ProbPasMeilleurCoup <= 0.0 ) break;
		if ( Table[0].Index == IndexCoup[i] || Table[1].Index == IndexCoup[i] ) continue;
		if ( pJeu->TmpProbCarte[posJoueur3][IndexCoup[i]] <= 1e-7 ) continue;
		ValC = ResultatCoup[i] * pJeu->TmpProbCarte[posJoueur3][IndexCoup[i]];
		if ( ValC <= ValeurExcuse )
		{
			ValeurCoup += ValeurExcuse * ProbPasMeilleurCoup;
			ValeurExcuse = -1e10;
			ProbPasMeilleurCoup *= 1.0 - pJeu->TmpProbCarte[posJoueur3][0];
		}
		ValeurCoup += ValC  * ProbPasMeilleurCoup;
#if DEBUG_EVAL_SECOND
                OutDebug("EvalCoupSecond : ");
                MakeCarte(&Table[2], IndexCoup[i]);
                ImprimeCarte(&Table[0]);
                ImprimeCarte(&Table[1]);
                ImprimeCarte(&Table[2]);
                OutDebug(", ResCoup = %.2f Valeur = %.2f   ProbReste = %.2f\n", ResultatCoup[i], ValeurCoup, ProbPasMeilleurCoup);
#endif // DEBUG_EVAL_SECOND
		ProbPasMeilleurCoup *= 1.0 - pJeu->TmpProbCarte[posJoueur3][IndexCoup[i]];
	}
    //	Groupe 4 : Autres atouts
	for ( i = 1; i < StartAtout; i++)
	{
		for ( j = i+1; j < StartAtout; j++)
		{
			if ( ResultatCoup[i] < ResultatCoup[j] )
			{
				Tmp = ResultatCoup[i];
				ResultatCoup[i] = ResultatCoup[j];
				ResultatCoup[j] = Tmp;
				iTmp = IndexCoup[i];
				IndexCoup[i] = IndexCoup[j];
				IndexCoup[j] = iTmp;
			}
		}
	}
	for ( i = 1; i < StartAtout; i++)
	{
		if ( ProbPasMeilleurCoup <= 0.0 ) break;
		if ( Table[0].Index == IndexCoup[i] || Table[1].Index == IndexCoup[i] ) continue;
		if ( pJeu->TmpProbCarte[posJoueur3][IndexCoup[i]] <= 1e-7 ) continue;
		ValC = ResultatCoup[i] *pJeu-> TmpProbCarte[posJoueur3][IndexCoup[i]];
		if ( ValC <= ValeurExcuse )
		{
			ValeurCoup += ValeurExcuse * ProbPasMeilleurCoup;
			ValeurExcuse = -1e10;
			ProbPasMeilleurCoup *= 1.0 - pJeu->TmpProbCarte[posJoueur3][0];
		}
		ValeurCoup += ValC  * ProbPasMeilleurCoup;
#if DEBUG_EVAL_SECOND
                OutDebug("EvalCoupSecond : ");
                MakeCarte(&Table[2], IndexCoup[i]);
                ImprimeCarte(&Table[0]);
                ImprimeCarte(&Table[1]);
                ImprimeCarte(&Table[2]);
                OutDebug(", ResCoup = %.2f Valeur = %.2f   ProbReste = %.2f\n", ResultatCoup[i], ValeurCoup, ProbPasMeilleurCoup);
#endif // DEBUG_EVAL_SECOND
		ProbPasMeilleurCoup *= 1.0 - pJeu->TmpProbCarte[posJoueur3][IndexCoup[i]];
	}
//	Groupe 5 : Autres cartes
	for ( i = 22; i < 78; i++)
	{
		if ( CouleurDemandee < ATOUT && i >= Startof[CouleurDemandee] && i < Endof[CouleurDemandee]) continue;
		for ( j = i+1; j < 78; j++)
		{
			if ( CouleurDemandee < ATOUT && j >= Startof[CouleurDemandee] && j < Endof[CouleurDemandee]) continue;
			if ( ResultatCoup[i] < ResultatCoup[j] )
			{
				Tmp = ResultatCoup[i];
				ResultatCoup[i] = ResultatCoup[j];
				ResultatCoup[j] = Tmp;
				iTmp = IndexCoup[i];
				IndexCoup[i] = IndexCoup[j];
				IndexCoup[j] = iTmp;
			}
		}
	}
	for ( i = 22; i < 78; i++)
	{
		if ( ProbPasMeilleurCoup <= 0.0 ) break;
		if ( Table[0].Index == IndexCoup[i] || Table[1].Index == IndexCoup[i] ) continue;
		if ( pJeu->TmpProbCarte[posJoueur3][IndexCoup[i]] <= 1e-7 ) continue;
		if ( CouleurDemandee < ATOUT && i >= Startof[CouleurDemandee] && i < Endof[CouleurDemandee]) continue;
		ValC = ResultatCoup[i] * pJeu->TmpProbCarte[posJoueur3][IndexCoup[i]];
		if ( ValC <= ValeurExcuse )
		{
			ValeurCoup += ValeurExcuse * ProbPasMeilleurCoup;
			ValeurExcuse = -1e10;
			ProbPasMeilleurCoup *= 1.0 - pJeu->TmpProbCarte[posJoueur3][0];
		}
		ValeurCoup += ValC  * ProbPasMeilleurCoup;
#if DEBUG_EVAL_SECOND
                OutDebug("EvalCoupSecond : ");
                MakeCarte(&Table[2], IndexCoup[i]);
                ImprimeCarte(&Table[0]);
                ImprimeCarte(&Table[1]);
                ImprimeCarte(&Table[2]);
                OutDebug(", ResCoup = %.2f Valeur = %.2f   ProbReste = %.2f\n", ResultatCoup[i], ValeurCoup, ProbPasMeilleurCoup);
#endif // DEBUG_EVAL_SECOND
		ProbPasMeilleurCoup *= 1.0 - pJeu->TmpProbCarte[posJoueur3][IndexCoup[i]];
	}
	return(Signe*ValeurCoup);
}


#if DEBUG > 0
#define DEBUG_EVAL_TROISIEME 0
#else
#define DEBUG_EVAL_TROISIEME 0
#endif  // DEBUG

//	Evalue le coup joue par le troisième joueur
//	Pour cela "joue" toutes les cartes possibles pour le quatrième joueur
//  Les trie ensuite par valeur et pondère par la probabilité d'avoir la carte en question.

double EvalCoupTroisieme(TarotGame CurrentGame, struct _Jeu *pJeu)
{
int posJoueur4 = (CurrentGame->JoueurEntame + 3) & 3;	//	position 4eme joueur
int posJoueur3 = (CurrentGame->JoueurEntame + 2) & 3;	//	position 3eme joueur
int i, j;
double ResultatCoup[78];
int IndexCoup[78];
int iGagnant[78];
double ProbPasMeilleurCoup = 1.0;
double ValeurCoup = 0.0;
double Tmp;
int iTmp;
double Signe = 1.0;
int CouleurDemandee;
int StartAtout;
int AtoutMaitre;
double CorrectionMaitre;
double ValeurExcuse = -1e10;
double ValC;

    //  Détermine déjà si le joueur 4 est dans la même équipe que le joueur 3
	if ( posJoueur4 == CurrentGame->JoueurPreneur ) Signe = -1.0;       //  Joueur 4 = Preneur, pas la même équipe que 4
	if ( posJoueur3 == CurrentGame->JoueurPreneur ) Signe = -1.0;       //  Joueur 3 = Preneur, pas la même équipe que 4
    //	Recherche parmi les 78 cartes celles qui sont possibles
	for ( i = 0; i < 78; i++)
	{
		if ( Table[0].Index == i || Table[1].Index == i || Table[2].Index == i )
		{
			ResultatCoup[i] = -1e6;		//	Déjà sur la table, donc impossible. Met un score très mauvais, sera ignoré
		}
		else if ( CurrentGame->CarteJouee[i] >= 0  ||  pJeu->TmpProbCarte[posJoueur4][i] == 0.0 )
		{
			ResultatCoup[i] = -1e6;     //	Déjà jouée ou impossible. Met un score très mauvais, sera ignoré
		}
		else
		{
			MakeCarte(&Table[3], i);
			ResultatCoup[i] = EvalCoup(CurrentGame, pJeu, &iGagnant[i]);
		}
	}
    //	Trie les coups par groupe en partant du meilleur vers le moins bon.
    //  L'idée est que le joueur 4 va jouer les meilleurs coups en priorité
	for ( i = 0; i < 78; i++) IndexCoup[i] = i;
	CouleurDemandee = Table[0].Couleur;
	if ( CouleurDemandee == EXCUSE ) CouleurDemandee = Table[1].Couleur;
    //	Groupe 1 : Excuse qui est un cas spécial.
    //  Si le joueur possède l'excuse, il pourra toujours la jouer pour remplacer un coup moins bon
	if ( Table[0].Index != 0 && Table[1].Index != 0 && Table[2].Index != 0 && pJeu->TmpProbCarte[posJoueur4][0] > 1e-7 )
	{
		if ( posJoueur4 == pJeu->PositionPreneur && ResultatCoup[0] > -3.0 )		//	Doit se débarrasser... de l'excuse
		{	//	Pour cela, augmente valeur du coup si ne perd pas trop...
			//	Augmente encore plus si "économise un atout"
			ResultatCoup[0] += 3.0;
			if ( CouleurDemandee == ATOUT || pJeu->ProbCoupe[CouleurDemandee] > 0.95 )
				ResultatCoup[0] += 2.0;
		}
		ValeurExcuse = pJeu->TmpProbCarte[posJoueur4][0] * ResultatCoup[0];
#if DEBUG_EVAL_TROISIEME > 0
        OutDebug("EvalCoupTroisième : ");
        MakeCarte(&Table[3], 0);
        ImprimeCarte(&Table[0]);
        ImprimeCarte(&Table[1]);
        ImprimeCarte(&Table[2]);
        ImprimeCarte(&Table[3]);
        OutDebug(", ResCoup Exc = %.2f ValeurExc = %.2f   ProbReste = %.2f\n", ResultatCoup[0], ValeurExcuse, ProbPasMeilleurCoup);
#endif // DEBUG_EVAL_TROISIEME
	}
    //	Groupe 2 : Couleur demandée
	if ( CouleurDemandee > ATOUT )	//	Couleur normale
	{
		for ( i = Startof[CouleurDemandee]; i < Endof[CouleurDemandee]; i++)
		{
			for ( j = i+1; j < Endof[CouleurDemandee]; j++)
			{
				if ( ResultatCoup[i] < ResultatCoup[j] )
				{
					Tmp = ResultatCoup[i];
					ResultatCoup[i] = ResultatCoup[j];
					ResultatCoup[j] = Tmp;
					iTmp = IndexCoup[i];
					IndexCoup[i] = IndexCoup[j];
					IndexCoup[j] = iTmp;
					iTmp = iGagnant[i];
					iGagnant[i] = iGagnant[j];
					iGagnant[j] = iTmp;;
				}
			}
		}
//	Calcule maintenant la valeur du coup en partant du meilleur
//	ValeurCoup = Somme(TmpProbCarte*ResultatCoup[i]*ProbPasMeilleurCoup)
		if ( posJoueur4 == pJeu->PositionPreneur && ProbPasMeilleurCoup > 0 && CouleurDemandee > ATOUT)
		{
		    //  Pour le preneur, change proba pas meilleur coup, lui donne la probabilité de coupe
			ProbPasMeilleurCoup = 1.0 - pJeu->TmpProbCoupe[CouleurDemandee];
		}
		if ( ProbPasMeilleurCoup > 0.0 )
		{
			for ( i = Startof[CouleurDemandee]; i < Endof[CouleurDemandee]; i++)
			{
				if ( ProbPasMeilleurCoup <= 1e-7 ) break;
				if ( Table[0].Index == IndexCoup[i] || Table[1].Index == IndexCoup[i]|| Table[2].Index == IndexCoup[i] )
					continue;
				if ( pJeu->TmpProbCarte[posJoueur4][IndexCoup[i]] <= 1e-7 ) continue;
				ValC = ResultatCoup[i] * pJeu->TmpProbCarte[posJoueur4][IndexCoup[i]];
				if ( ValC <= ValeurExcuse )
				{
					ValeurCoup += ValeurExcuse * ProbPasMeilleurCoup;
					ValeurExcuse = -1e10;
					ProbPasMeilleurCoup *= 1.0 - pJeu->TmpProbCarte[posJoueur4][0];
				}
				if ( iGagnant[i] == 3 )
				{
                    //	Si gagnant en position 4, limite proba carte à ProbaPasMeilleurCoup
					if ( pJeu->TmpProbCarte[posJoueur4][IndexCoup[i]] > ProbPasMeilleurCoup )
						ValeurCoup += ResultatCoup[i] * ProbPasMeilleurCoup;
					else
						ValeurCoup += ValC;
				}
				else
				{
					ValeurCoup += ValC  * ProbPasMeilleurCoup;
				}
#if DEBUG_EVAL_TROISIEME > 0
                OutDebug("EvalCoupTroisième : ");
                MakeCarte(&Table[3], IndexCoup[i]);
                ImprimeCarte(&Table[0]);
                ImprimeCarte(&Table[1]);
                ImprimeCarte(&Table[2]);
                ImprimeCarte(&Table[3]);
                OutDebug(", ResCoup = %.2f Valeur = %.2f   ProbReste = %.2f\n", ResultatCoup[i], ValeurCoup, ProbPasMeilleurCoup);
#endif // DEBUG_EVAL_TROISIEME
				ProbPasMeilleurCoup *= 1.0 - pJeu->TmpProbCarte[posJoueur4][IndexCoup[i]];
			}
			if ( posJoueur4 == pJeu->PositionPreneur && ProbPasMeilleurCoup > 0 && CouleurDemandee > ATOUT)
			{
			    //  Pour le preneur, met la probabilité ProbPasMeilleurCoup à la probabilité de coupe de la couleur
				if (ProbPasMeilleurCoup <= pJeu->TmpProbCoupe[CouleurDemandee])
					ProbPasMeilleurCoup = pJeu->TmpProbCoupe[CouleurDemandee];
			}
		}
		else
		{
			ProbPasMeilleurCoup = 1.0;
		}
	}
    //	Groupe 3 : Atouts plus grands que le MAX
	//	Correction de proba : si 4eme est le preneur, joue ATOUT et beaucoup de points
	//	sur la table, monte la proba de possession de l'atout maître
	AtoutMaitre = CarteMaitreNonJoueeNoErr(CurrentGame, ATOUT);
	if ( posJoueur4 == pJeu->PositionPreneur && (Table[0].Couleur == ATOUT || pJeu->TmpProbCoupe[Table[0].Couleur] > 0.9)
		&& Table[0].Index != AtoutMaitre && Table[1].Index != AtoutMaitre && Table[2].Index != AtoutMaitre && AtoutMaitre > 0
		&& pJeu->ProbCarte[posJoueur4][AtoutMaitre] > 0)
    {
	    //  CorrectionMaitre sera d'autant plus grand que la probabilité que le preneur ait l'atout maître est faible
		CorrectionMaitre = 0.95 / sqrt(pJeu->ProbCarte[posJoueur4][AtoutMaitre]);
	}
	else
		CorrectionMaitre = 1.0;
	StartAtout = PlusFortAtoutJoue(3) + 1;
	for ( i = StartAtout; i < 22; i++)
	{
		for ( j = i+1; j < 22; j++)
		{
			if ( ResultatCoup[i] < ResultatCoup[j] )
			{
				Tmp = ResultatCoup[i];
				ResultatCoup[i] = ResultatCoup[j];
				ResultatCoup[j] = Tmp;
				iTmp = IndexCoup[i];
				IndexCoup[i] = IndexCoup[j];
				IndexCoup[j] = iTmp;
			}
		}
	}
    //	Calcule maintenant la valeur du coup en partant du meilleur
    //	ValeurCoup = Somme(TmpProbCarte*ResultatCoup[i]*ProbPasMeilleurCoup)
	for ( i = StartAtout; i < 22; i++)
	{
		if ( ProbPasMeilleurCoup <= 0.0 ) break;
		if ( Table[0].Index == IndexCoup[i] || Table[1].Index == IndexCoup[i]|| Table[2].Index == IndexCoup[i] )
			continue;
		if ( pJeu->TmpProbCarte[posJoueur4][IndexCoup[i]] <= 1e-7 ) continue;
		if ( IndexCoup[i] == AtoutMaitre )
			ValC = ResultatCoup[i] * pJeu->TmpProbCarte[posJoueur4][IndexCoup[i]] * CorrectionMaitre;
		else
			ValC = ResultatCoup[i] * pJeu->TmpProbCarte[posJoueur4][IndexCoup[i]];
		if ( ValC <= ValeurExcuse )
		{
			ValeurCoup += ValeurExcuse * ProbPasMeilleurCoup;
			ValeurExcuse = -1e10;
			ProbPasMeilleurCoup *= 1.0 - pJeu->TmpProbCarte[posJoueur4][0];
		}
		ValeurCoup += ValC  * ProbPasMeilleurCoup;
#if DEBUG_EVAL_TROISIEME > 0
        OutDebug("EvalCoupTroisième : ");
        MakeCarte(&Table[3], IndexCoup[i]);
        ImprimeCarte(&Table[0]);
        ImprimeCarte(&Table[1]);
        ImprimeCarte(&Table[2]);
        ImprimeCarte(&Table[3]);
        OutDebug(", ResCoup = %.2f Valeur = %.2f   ProbReste = %.2f\n", ResultatCoup[i], ValeurCoup, ProbPasMeilleurCoup);
#endif // DEBUG_EVAL_TROISIEME
		ProbPasMeilleurCoup *= 1.0 - pJeu->TmpProbCarte[posJoueur4][IndexCoup[i]];
	}
//	Groupe 4 : Autres atouts
	for ( i = 1; i < StartAtout; i++)
	{
		for ( j = i+1; j < StartAtout; j++)
		{
			if ( ResultatCoup[i] < ResultatCoup[j] )
			{
				Tmp = ResultatCoup[i];
				ResultatCoup[i] = ResultatCoup[j];
				ResultatCoup[j] = Tmp;
				iTmp = IndexCoup[i];
				IndexCoup[i] = IndexCoup[j];
				IndexCoup[j] = iTmp;
			}
		}
	}
	for ( i = 1; i < StartAtout; i++)
	{
		if ( ProbPasMeilleurCoup <= 0.0 ) break;
		if ( Table[0].Index == IndexCoup[i] || Table[1].Index == IndexCoup[i]|| Table[2].Index == IndexCoup[i] )
			continue;
		if ( pJeu->TmpProbCarte[posJoueur4][IndexCoup[i]] <= 1e-7 ) continue;
		ValC = ResultatCoup[i] * pJeu->TmpProbCarte[posJoueur4][IndexCoup[i]];
		if ( ValC <= ValeurExcuse )
		{
			ValeurCoup += ValeurExcuse * ProbPasMeilleurCoup;
			ValeurExcuse = -1e10;
			ProbPasMeilleurCoup *= 1.0 - pJeu->TmpProbCarte[posJoueur4][0];
		}
		ValeurCoup += ValC  * ProbPasMeilleurCoup;
#if DEBUG_EVAL_TROISIEME > 0
        OutDebug("EvalCoupTroisième : ");
        MakeCarte(&Table[3], IndexCoup[i]);
        ImprimeCarte(&Table[0]);
        ImprimeCarte(&Table[1]);
        ImprimeCarte(&Table[2]);
        ImprimeCarte(&Table[3]);
        OutDebug(", ResCoup = %.2f Valeur = %.2f   ProbReste = %.2f\n", ResultatCoup[i], ValeurCoup, ProbPasMeilleurCoup);
#endif // DEBUG_EVAL_TROISIEME
		ProbPasMeilleurCoup *= 1.0 - pJeu->TmpProbCarte[posJoueur4][IndexCoup[i]];
	}
//	Groupe 5 : Autres cartes
	for ( i = 22; i < 78; i++)
	{
		if ( CouleurDemandee < ATOUT && i >= Startof[CouleurDemandee] && i < Endof[CouleurDemandee]) continue;
		for ( j = i+1; j < 78; j++)
		{
			if ( CouleurDemandee < ATOUT && j >= Startof[CouleurDemandee] && j < Endof[CouleurDemandee]) continue;
			if ( ResultatCoup[i] < ResultatCoup[j] )
			{
				Tmp = ResultatCoup[i];
				ResultatCoup[i] = ResultatCoup[j];
				ResultatCoup[j] = Tmp;
				iTmp = IndexCoup[i];
				IndexCoup[i] = IndexCoup[j];
				IndexCoup[j] = iTmp;
			}
		}
	}
	for ( i = 22; i < 78; i++)
	{
		if ( ProbPasMeilleurCoup <= 0.0 ) break;
		if ( Table[0].Index == IndexCoup[i] || Table[1].Index == IndexCoup[i]|| Table[2].Index == IndexCoup[i] )
			continue;
		if ( pJeu->TmpProbCarte[posJoueur4][IndexCoup[i]] <= 1e-7 ) continue;
		if ( CouleurDemandee < ATOUT && i >= Startof[CouleurDemandee] && i < Endof[CouleurDemandee]) continue;
		ValC = ResultatCoup[i] * pJeu->TmpProbCarte[posJoueur4][IndexCoup[i]];
		if ( ValC <= ValeurExcuse )
		{
			ValeurCoup += ValeurExcuse * ProbPasMeilleurCoup;
			ValeurExcuse = -1e10;
			ProbPasMeilleurCoup *= 1.0 - pJeu->TmpProbCarte[posJoueur4][0];
		}
		ValeurCoup += ValC  * ProbPasMeilleurCoup;
#if DEBUG_EVAL_TROISIEME > 0
        OutDebug("EvalCoupTroisième : ");
        MakeCarte(&Table[3], IndexCoup[i]);
        ImprimeCarte(&Table[0]);
        ImprimeCarte(&Table[1]);
        ImprimeCarte(&Table[2]);
        ImprimeCarte(&Table[3]);
        OutDebug(", ResCoup = %.2f Valeur = %.2f   ProbReste = %.2f\n", ResultatCoup[i], ValeurCoup, ProbPasMeilleurCoup);
#endif // DEBUG_EVAL_TROISIEME
		ProbPasMeilleurCoup *= 1.0 - pJeu->TmpProbCarte[posJoueur4][IndexCoup[i]];
	}
	return(Signe*ValeurCoup);
}


//	Evalue un coup "joué".
//		iGagnant  : Pointeur sur joueur gagnant (à partir de JoueurEntame)
//	Retourne la valeur du coup pour le dernier joueur.

double EvalCoup(TarotGame CurrentGame, struct _Jeu *pJeu, int *iGagnant)
{
int igagnant;
int GainAttaque;
int j, j1, j2;
double pts[2];
int couleur;
int PosGagnant;
int IndexPreneur = (CurrentGame->JoueurPreneur - CurrentGame->JoueurEntame) & 3;
int IndexJoueur = (pJeu->PositionJoueur - CurrentGame->JoueurEntame) & 3;
int PosC;
double PtRestant, v;
int nb;
double PtDefausse = 0.0;
int HauteurAtout = 0;
int CouleurDemandee = -1;
int OldHauteurAtout = 0;

	HauteurAtout = 0;
    //	Phase 1 : Regarde qui a le pli
    igagnant = Gagnant(4);
    *iGagnant = igagnant;
    PosGagnant = (igagnant + CurrentGame->JoueurEntame)&3;
	pts[0] = 0;
	pts[1] = 0;
	if ( PosGagnant == pJeu->PositionPreneur )	//	Gagnant = Attaque
	{
		GainAttaque = 1;
	}
	else										//	Gagnant = Défense
	{											//	Points en plus si gagnant avant preneur
		GainAttaque = 0;
		//	Valeurs : 4, 1, 0 si dernier = défense, sinon : 3, 0, -1
		//  L'idée est qu'il est meilleur d'avoir la main devant le preneur
		pts[0] = (((PosGagnant - pJeu->PositionPreneur) & 3) - 1);
		pts[0] = pts[0] * pts[0];
		if ( IndexPreneur == 3 ) pts[0]--;
	}
    //	Phase 2 : Pour chaque carte sur la table, détermine sa "valeur"
    //	La valeur retournée sera la somme des valeurs individuelles de chaque carte
	for ( j = 0; j < 4; j++ )
	{
		couleur = Table[j].Couleur;
		if ( j == 0 ) CouleurDemandee = couleur;
		if ( j == 1 && CouleurDemandee == EXCUSE ) CouleurDemandee = couleur;
		PosC = (CurrentGame->JoueurEntame + j) & 3;			//	Position du joueur courant
		if ( couleur == EXCUSE )
		{
        //	Valeur de l'excuse.
        //	Si chelem, Ok pour la jouer en dernier.
        //	Sinon, la donne à l'autre, c'est très mauvais
        //	Force à jouer l'excuse avant en augmentant sa valeur dans les derniers coups (5)
        //	Sinon, ne vaut rien (0 point).
        //	Ceci pour ne pas mettre l'excuse a la place d'une petite carte.
        //	Si un défenseur possède l'excuse, retire des points s'il a le petit.
			if ( pJeu->NbCarte == 1 )
			{
			//	Excuse joue en dernier
			//	De toute manière, pas trop le choix, donc la valeur importe peu...
				if ( CurrentGame->ChelemDemande && (CurrentGame->JoueurEntame == pJeu->PositionPreneur) && j == 0 )
				{
					pts[1] += 9.0;          //  OK seulement dans ce cas
					GainAttaque = 1;
					*iGagnant = 0;
				}
				else
					pts[GainAttaque] += 9.0;
			}
			else if ( pJeu->NbCarte < 5)
			{
			//	Il est grand temps d'y penser...
			//	Donne des points supplémentaires à l'excuse quand on approche de la fin
			//	Points excuse : 19/(5-NbCarte**2)/9
			//	Si 4 cartes : 2.1  3 cartes : 8.4  2 cartes : 18.9
				if ( j == IndexPreneur )
				{
                    //	Le preneur possède l'excuse
                    //	Si calcul pour le preneur, tient compte des bouts
					if ( pJeu->PositionJoueur == pJeu->PositionPreneur )
						pts[1] += (19 + 20*(pJeu->NbBout == 2)) * (5 - pJeu->NbCarte)*(5 - pJeu->NbCarte)/9.0;
					else
						pts[1] += 19.0 * (5 - pJeu->NbCarte)*(5 - pJeu->NbCarte)/9.0;
					pts[GainAttaque]++;
				}
				else
				//	Même répartition de points que pour le preneur
				{
					pts[0] +=  19.0 * (5 - pJeu->NbCarte)*(5 - pJeu->NbCarte)/9.0;
					pts[GainAttaque]++;
				}
			}
			else if ( pJeu->PositionJoueur != pJeu->PositionPreneur && Table[0].Couleur == ATOUT
                    && pJeu->JoueurCouleur[ATOUT] < 0 && !HasPetit(pJeu) )
			{
				pts[0] -= 30.0;		//	Ne fait pas croire qu'il a le petit en jouant l'excuse sur le premier tour d'atout
			}
			else if ( FlagSignalisation && CurrentGame->JoueurEntame == pJeu->PositionPreneur
                    && pJeu->JoueurCouleur[CouleurDemandee] < 0 && pJeu->PositionJoueur != pJeu->PositionPreneur )
			{
				if ( pJeu->NbCouleur[CouleurDemandee] >= 4 && ( Table[0].Hauteur < VALET || Table[0].Hauteur == ROI )
                && (HasCarte(pJeu, pJeu->PositionJoueur, Startof[CouleurDemandee]+ROI-1) || HasCarte(pJeu, pJeu->PositionJoueur, Startof[CouleurDemandee]+DAME-1)) )
				{
					pts[0] += 10.0;		//	Signale Tenue par l'excuse, c'est bon pour la défense
				}
				else
				{
					pts[0] -= 7.0;		//	Ne Signale pas fausse Tenue par l'excuse
				}
			}
			else if ( Table[0].Couleur > ATOUT && j != IndexPreneur)
			{	//	Joue excuse pour ne pas prendre la main et arrêter la chasse
				//	Seulement si hauteur ATOUT suffisante pour prendre la main
				nb = NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT);
				if ( nb > 2 && pJeu->StatusChasse == CHASSE_DEFENSE_IMPAIR && pJeu->NbAtout == 2 &&
					PosC == pJeu->PositionJoueur &&  pJeu->MyCarte[1].Hauteur > Table[igagnant].Hauteur)
				//	Si la défense chasse et qu'il ne reste qu'un seul atout en plus de l'excuse
				//	Ajoute jusqu'a 7 points
				{
					pts[0] += 2.0 + 7.0/(nb - 2);
				}
				//	Ne joue pas l'excuse s'il a le petit
				if ( pJeu->AtoutPreneur > 0.01 ) pts[0] -= pJeu->ProbCarte[PosC][1] * 3;
				//  Peut-être intéressant de jouer l'excuse pour allonger (surtout si points).
				//  Mais pas forcément au début
				if ( PosC == pJeu->PositionJoueur && pJeu->NbCouleur[CouleurDemandee] > 0)
                {
                    if ( IndexPreneur == 0 && pJeu->NbCouleur[CouleurDemandee] >= 5 && GetCartePlusForte(pJeu, CouleurDemandee)->Hauteur >= DAME
                        && pJeu->JoueurCouleur[CouleurDemandee] < 0)
                    {
                        //  Intéressant de jouer l'excuse si ouverture preneur et couleur longue et forte
                        pts[0] += 4;
                    }
                    else if ( AvgLongueur(pJeu, pJeu->PositionPreneur, CouleurDemandee) > 3 )
                        pts[0] -= 3.0 / pJeu->NbCouleur[CouleurDemandee];
                    else
                        pts[0] -= 3;

                }

			}
			else if ( Table[0].Couleur == ATOUT && j != IndexPreneur && PosC == pJeu->PositionJoueur)
			{
				//	Evite de mettre l'excuse pour laisser le pli au preneur si chasse lancée
				if ( pJeu->StatusChasse == CHASSE_DEFENSE_IMPAIR || (Table[0].Index & 1) )
				{
					if ( GainAttaque == 1 && PosC > IndexPreneur && pJeu->NbAtout > 0
                        && GetCartePlusForte(pJeu, ATOUT)->Index > Table[IndexPreneur].Index )
						pts[0] -= 5.0;
				}
			}
			else if ( j == IndexPreneur )
			//	Plus c'est tard, plus il est difficile de la placer
			//	Ajoute un peu de points quand on se rapproche des 5 cartes avant la fin
			{
				pts[1] += exp(0.25*(5.0 - pJeu->NbCarte + MinPlisPreneur(CurrentGame, pJeu)));
				if ( pJeu->NbCouleur[CouleurDemandee] == 0 ) pts[1] += 1.0 + HauteurAtout/10.0;	//	Ajoute des points si ATOUT

			}
			//	Par défaut : 0 point pour l'excuse
		}
		else if ( couleur == ATOUT )
		{
			//	Calcule déjà le plus haut atout sur la table jusqu'a présent
			if ( Table[j].Hauteur > HauteurAtout )
			{
				OldHauteurAtout = HauteurAtout;
				HauteurAtout = Table[j].Hauteur;
			}
			if ( Table[IndexPreneur].Couleur != ATOUT && pJeu->AtoutPreneur < 0.01 )
				pts[GainAttaque] -= 10.0;		//	Essaie de ne pas couper si c'est possible...
			v = NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT) / (NbPossCouleur(pJeu, ATOUT) + 2);
			v = v - pJeu->NbAtout - 1;
			if ( Table[j].Hauteur == 1  )	//	Petit
			{
            //	Le petit vaut 19 points sauf si l'on estime pouvoir le mener au bout.
            //	Dans ce cas, on retire des points
				if ( PosC == pJeu->PositionJoueur && j == igagnant && GainAttaque )
				{	//	Preneur gagnant : vaut-il mieux le mener au bout ?
					pts[GainAttaque] += 19.0 - pJeu->ValPetitAuBout * (1 + 2.0*StyleJoueur[PosC][dStyleAttPetit]);
				}
				else if ( PosC == pJeu->PositionJoueur && j == igagnant && !GainAttaque  )
				{	//	Défense gagnante
					pts[GainAttaque] += 19.0 - pJeu->ValPetitAuBout ;
				}
				else if ( !GainAttaque  && pJeu->PositionJoueur == pJeu->PositionPreneur )
				{	//	Preneur perd le petit
					pts[GainAttaque] += 29;		//	Prime si perd le petit !!!
				}
				else if ( GainAttaque  && pJeu->PositionJoueur != pJeu->PositionPreneur )
				{	//	Preneur prend le petit
					pts[GainAttaque] += 29;		//	Prime si perd le petit !!!
				}
				else if ( !GainAttaque && PosC == pJeu->PositionJoueur )
				{
					pts[GainAttaque] += 19.0 - pJeu->ValPetitAuBout ;
				}
				else
					pts[GainAttaque] += 19;
			}
			else if ( j == igagnant && pJeu->ResteAtout > 0 && PosC == pJeu->PositionJoueur && PosC != pJeu->PositionPreneur
                    && HasCarte(pJeu, pJeu->PositionJoueur,  1) && v > 0 )
			{	//	Possède le petit, joue les gros si après le preneur (et n'en a pas trop...)
				pts[GainAttaque] += (((PosGagnant - pJeu->PositionPreneur) & 3) - 1) + 0.25*(HauteurAtout - OldHauteurAtout) + v;
			}
			else if ( PosC == pJeu->PositionJoueur && PosC != pJeu->PositionPreneur && PosC == ((pJeu->PositionPreneur-1) & 3) && j < 3
				&& pJeu->JoueurCouleur[4] < 0 && !HasPetit(pJeu) )
			{	//	Ne monte pas trop le premier tour d'atout si n'a pas le petit
				pts[0] -= (Table[j].Hauteur - 7.0)*0.2;
			}
			else if ( j == igagnant  && pJeu->ResteAtout > 0)		//	Sort les gros a bon escient
			{
                //	Pour les gros atouts, retire des points pour ne pas "assurer" un simple valet
				//	Première correction : Valeur retirée variant linéairement de 0 pour le 2 à 4.0 pour le 21
				pts[GainAttaque] -= (Table[j].Hauteur - 2.0)*4.0/19.0;
				//	Seconde correction : Valeur retirée variant linéairement de 0 pour le 14 à 1.0 pour le 21
				if ( Table[j].Hauteur >= 15 )
					pts[GainAttaque] -= (Table[j].Hauteur - 14.0) * 1.0 / 7.0;
				//	Retire en outre 2.0 points si joue l'atout maitre
				if ( NbPossCouleur(pJeu, ATOUT) > 0 && Table[j].Index == CarteMaitreNonJouee(CurrentGame, ATOUT) )
					pts[GainAttaque] -= 2.0;
			}
			else if ( PosC == pJeu->PositionJoueur && PosC != pJeu->PositionPreneur && JoueurAvecPetit(CurrentGame, pJeu) == pJeu->PositionPreneur
				&& (pJeu->StatusChasse == CHASSE_DEFENSE_IMPAIR || (pJeu->JoueurCouleur[ATOUT] < 0 && IndexPreneur != 0 && CouleurDemandee == ATOUT))
				&&  ((pJeu->NbAtout == 2 && !HasCarte(pJeu, pJeu->PositionJoueur, 0)) || (pJeu->NbAtout == 3 && HasCarte(pJeu, pJeu->PositionJoueur, 0))))
			{
				//	Joue la plus grosse des deux dernières pour ne pas bloquer la chasse
				if ( !HasCarte(pJeu, pJeu->PositionJoueur, 0) && Table[j].Hauteur == pJeu->MyCarte[1].Hauteur )
					pts[0] += 3.0;
				if ( HasCarte(pJeu, pJeu->PositionJoueur, 0) && Table[j].Hauteur == pJeu->MyCarte[2].Hauteur )
					pts[0] += 3.0;
			}
			else if ( PosC == pJeu->PositionJoueur && PosC != pJeu->PositionPreneur && !GainAttaque && igagnant > j )
			{	//	Force le suivant a mettre un gros Atout, pas très bon pour sauver le petit
				pts[GainAttaque] -= (Table[j].Hauteur - 2.0)*4.0/19.0;
				//	Seconde correction : Valeur retirée variant linéairement de 0 pour le 14 à 5.0 pour le 21
				if ( Table[j].Hauteur >= 15 )
					pts[GainAttaque] -= (Table[j].Hauteur - 14.0) * 5.0 / 7.0;
			}
            //	Si juste avant le preneur, prime aux gros pour "uppercut"
			else if ( PosC == ((pJeu->PositionPreneur-1) & 3) && j < 3 && Table[IndexPreneur].Couleur == ATOUT
				&& Table[j].Hauteur == HauteurAtout && Table[IndexPreneur].Hauteur > HauteurAtout
				&& !IsMaitre(CurrentGame, pJeu, &Table[j]) )
			{
				pts[0] += (Table[j].Hauteur - 2.0) * 3.0/19.0;
			}
			else if ( pJeu->ResteAtout > 0 && Table[j].Hauteur > 12 && PosC != pJeu->PositionPreneur)	//	Sort les gros a bon escient
			{
            //	Pour les gros atouts, retire des points pour ne pas "assurer" un simple valet
				if ( pJeu->PositionJoueur == PosC && HasPetit(pJeu) )	//	Si petit, joue gros...
				{
				//	Ajoute une correction linéaire de 0 à 4.0 points si pas dernier et de 0 à 8.0 points si dernier
					v = 4;
					if ( j == 3 )
						v = 8;
					else if ( PosC == ((pJeu->PositionPreneur-1) & 3) )
						v = 2.0*j + 4.0;
					pts[0] += (Table[j].Hauteur - 2.0)*v/19.0;
				}
				else
					pts[0] -= (Table[j].Hauteur - 2.0)*4.0/19.0;
			}
			else if ( Table[j].Hauteur > 12 && PosC == pJeu->PositionPreneur && pJeu->ResteAtout > 0 )	//	Sort les gros a bon escient
			{
                //	Pour les gros atouts, retire des points pour ne pas "assurer" un simple valet
				pts[1] -= (Table[j].Hauteur - 2.0)*4.0/19.0;
			}
			else
				pts[GainAttaque]++;
		}
        //	Autres couleurs.
		else
		{
            //	Si Preneur, force a ne pas jouer trop les gros de suite
            //	Si défausse preneur, ne défausse pas les couleurs ou il est maître...
			if ( PosC == pJeu->PositionPreneur )
			{
				if ( j == igagnant )		//	Preneur Gagnant ?
				{
					pts[GainAttaque] += Table[j].Valeur;		//	Ajoute les points de la carte
					if ( Table[j].Valeur > 1 && IndexPreneur > 0 )				//	Si honneur...
					{
						PtRestant = CalcPointRestant(CurrentGame, pJeu, couleur, igagnant, IndexPreneur);
						//	Ne garde pas un honneur non maître ! Une dame ne prend pas un roi !
						if ( pJeu->NbCouleur[couleur] == 2 && !IsMaitreCouleur(CurrentGame, pJeu, couleur) )
							PtRestant = 0;
						pts[GainAttaque] -= PtRestant * ( 1 - 2.0 * StyleJoueur[pJeu->PositionPreneur][dStyleAttPoints] );
					}
				}
				else
				{
					pts[GainAttaque] += Table[j].Valeur;
					if ( couleur != CouleurDemandee && couleur != Table[igagnant].Couleur )		//	Plus d'atout chez le preneur ?
					{
						if ( NbReste(CurrentGame, pJeu->PositionPreneur, couleur) > 0 && Table[j].Index == CarteMaitreNonJouee(CurrentGame, couleur) )
							pts[1] -= 10.0;		//	Le preneur ne doit pas jeter pas une carte maîtresse.
					}
				}
			}
            //	La défense joue.
			else if ( !GainAttaque )	//	Défense gagne
			{
				if ( PosC == pJeu->PositionJoueur && pJeu->NbCouleur[couleur] == 2 && !IsMaitreCouleur(CurrentGame, pJeu, couleur) && Table[j].Valeur > 1)
				{
					//	Ne garde pas une carte non maîtresse s'il n'en reste que deux dans la couleur
					if ( j > IndexPreneur )
						pts[GainAttaque] += 0.5 * Table[j].Valeur * pJeu->TmpProbCarte[pJeu->PositionPreneur][CarteMaitreNonJouee(CurrentGame, couleur)];		//	Pour être sur de le mettre !
					else
						pts[GainAttaque] += 0.1 * Table[j].Valeur * pJeu->TmpProbCarte[pJeu->PositionPreneur][CarteMaitreNonJouee(CurrentGame, couleur)];;		//	Pour être sur de le mettre !
					if ( Table[j].Hauteur == CAVALIER && CurrentGame->CarteJouee[Startof[couleur]+ROI-1] < 0 && CurrentGame->CarteJouee[Startof[couleur]+DAME-1] < 0)
						pts[GainAttaque] += 0.5 * Table[j].Valeur;
				}
                //	Si gagnant et ProbCoupe
				//	nb = nombre de cartes jouées dans la couleur sur la table après le joueur
				nb = 0;
				for ( int c1 = IndexJoueur+1; c1 < 4; c1++ )
				{
					if ( c1 == j ) continue;
					if ( Table[c1].Couleur == couleur ) nb++;
				}
				//	nb = nombre de cartes restantes dans la couleur après ce coup
				nb = NbReste(CurrentGame, pJeu->PositionJoueur, couleur) - nb;
				if ( j == igagnant )
				{
					if ( nb == 0 || (Table[j].Index == Table[IndexPreneur].Index + 1))	//	Ne garde pas des cartes inutiles...
					{
						pts[GainAttaque] += Table[j].Valeur;
					}
					else if ( PosC == pJeu->PositionJoueur )
					{
                    //	le joueur est le gagnant
                    //	Force à ne pas mettre les honneurs de suite. Pour cela retire ce qu'il reste comme points
                    //	Si c'est la couleur du preneur, retire également des points...
						PtRestant = CalcPointRestant(CurrentGame, pJeu, couleur, igagnant, IndexPreneur);
						if ( pJeu->NbCouleur[couleur] == 2 && !IsMaitreCouleur(CurrentGame, pJeu, couleur) )
						{		//	Ne garde pas un honneur non maître ! Une dame ne prend pas un roi !
							PtRestant = 0;
							nb = 0;
						}
						//	Si possède doubleton R x, joue le ROI pour être sur.
						if ( pJeu->NbCouleur[couleur] == 2 && Table[j].Hauteur == ROI && pJeu->JoueurCouleur[couleur] < 0
                            && CurrentGame->JoueurEntame != pJeu->PositionPreneur)
						{
							if ( pJeu->ProbCarte[pJeu->PositionPreneur][Table[j].Index ] < 0.25 )	//	Pas la dame
								PtRestant -= 7.0;
							if ( j > IndexPreneur && GetCartePlusFaible(pJeu, couleur)->Valeur > 2)
								PtRestant /= 2.0;
						}
						pts[GainAttaque] -= PtRestant * ( 1 - 2.0 * StyleJoueur[pJeu->PositionJoueur][dStyleDefPoints] );
						//	Correction sur nb. Ne pas pas retarder plus que le nombre de cartes dans la couleur
						if ( nb > pJeu->NbCouleur[couleur] ) nb = pJeu->NbCouleur[couleur];
						//	Ne sert à rien si le preneur n'en a plus...
						if ( Table[IndexPreneur].Couleur != couleur )
							nb = 0;
						//	Si le preneur sort ses honneurs, il n'en a presque plus...
						else if ( Table[IndexPreneur].Hauteur >= VALET && nb > 1)
							nb = 1;
						//	Calcul des points de défausse, la fonction tient compte du nombre de possesseurs d'atout (en racine...)
						//	du nombre d'atouts restants (linéaire décroissant entre 0 (2.0) et 20 (0.0)), du nombre de cartes nb
						//	et du style de jeu
						if ( nb >= 1)
						{
							PtDefausse -= nb * sqrt(3.0 - NbPossCouleur(pJeu, ATOUT)) * (2.0 - NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT)/10.0)
								* (2.0 - 3.0*StyleJoueur[pJeu->PositionJoueur][dStyleDefPoints]);	//	Possibilite de defausse...
						}
						//	Seconde correction sur PtRestant
						if ( PtRestant < 1 )
							pts[GainAttaque] += Table[j].Valeur;	//	Plus rien à gratter, compte tous les points
						else if ( IndexPreneur > j )			//	Joue avant Preneur, Ok pour jouer gros
							pts[GainAttaque] += 1.0* Table[j].Valeur / PtRestant;
						else
							pts[GainAttaque] += 0.25 * Table[j].Valeur / PtRestant;		//	Après le preneur, divise par 4 la valeur de la carte
						if ( pJeu->TmpProbCarte[PosGagnant][Table[j].Index-1] >= 0.99 && Table[j].Valeur > 1)
						{	//	Ajoute des points si possède carte du dessous
							pts[GainAttaque] += Table[j].Valeur;
						}
						j2 = GetPlusForte(pJeu, couleur);
						j1 = ComptePlusGrand(CurrentGame, couleur, pJeu->MyCarte[j2].Hauteur);
						//	Si pas de chance de faire le pli, pas de défausse en vue...
						if ( pJeu->NbCouleur[couleur] <= j1)
							PtDefausse = 0;
						else
						//	Rectifie Pt Défausse si ne joue pas le plus gros
							PtDefausse *= 1.0 / (j2 - Carte2Index(pJeu, Table[j].Couleur, Table[j].Hauteur) + 1.0);
					}
					else	// Position != PosC
					{
						//	Pas le joueur courant, compte simplement les points de la carte
						pts[GainAttaque] += Table[j].Valeur;
					}
				}
                //	L'équipe gagne
				else
				{
					if ( couleur == CouleurDemandee )
					{
					//	Cas sans défausse
						if ( nb > 0 && (pJeu->JoueurCouleur[couleur] == pJeu->PositionPreneur || CurrentGame->JoueurEntame == pJeu->PositionPreneur )
                                && PosC == pJeu->PositionJoueur)
						{
							//	Cas où nb >0 et la couleur a été ouverte par le preneur (couleur du preneur à priori)
							j2 = GetPlusForte(pJeu, couleur);		//	Carte la plus forte du joueur dans la couleur
							j1 = ComptePlusGrand(CurrentGame, couleur, pJeu->MyCarte[j2].Hauteur);	//	Nombre cartes non jouées au dessus
							if ( pJeu->NbCouleur[couleur] <= j1 || nb == j1 )
							{	//	Pas de chance d'être maître...
								pts[GainAttaque] += Table[j].Valeur;
								if ( pJeu->NbCouleur[couleur] >= 2 && pJeu->MyCarte[j2].Index != Table[j].Index && pJeu->MyCarte[j2].Valeur > 1)	//	Pas la plus forte !
								{
									pts[GainAttaque] -= 1.0 * pJeu->MyCarte[j2].Valeur / pJeu->NbCouleur[couleur];		//	Attention a ne pas perdre un honneur bêtement ...
								}
							}
							else if ( GetCartePlusFaible(pJeu, couleur)->Index != Table[j].Index )
							{
								//	Donne plus d'importance aux points sur la fin...
								pts[GainAttaque] += Table[j].Valeur * (9 - pJeu->NbCarte)/8.0;
								if ( pJeu->NbCouleur[couleur] - nb/3 > 0 				//	pas mal de cartes (plus que la moyenne)
									&& (j2 - Carte2Index(pJeu, couleur, Table[j].Hauteur)) < pJeu->NbCouleur[couleur] - nb/3 )	//	si joue un peu haut pour exploiter les défausses
								{
									//	v est nombre de points estimés en défausse
									v = (pJeu->NbCouleur[couleur] * 3 - nb - 3) * 4;
									if ( v < 0 ) v = 0;
									//	j2 taille minimum couleur preneur
									j2 = MinLongueur(pJeu, pJeu->PositionPreneur, couleur);
									//	Les partenaires peuvent se défausser si 3*MinLongueur est supérieur à nb.
									if ( 3*j2 > nb )
									{
									    //  Premier Cas : Garde des points pour faire les plis plus tard avec défausse
										if ( AvgLongueur(pJeu, pJeu->PositionPreneur, couleur) >= j1 )
											v = v / sqrt( nb/3.0 - j2);
										else
										{
											//	divise par le nombre de plis sans défausse
											v = v / ( nb/3.0 - j2);
										}
										PtDefausse -= v;        //  Retire des points au coup pour ne pas jouer la plus forte
									}
									else if ( 3*AvgLongueur(pJeu, pJeu->PositionPreneur, couleur) > nb )
									{
									    //  Second cas : moins sûr car les preneur n'en a peut-être plus ?
										if ( AvgLongueur(pJeu, pJeu->PositionPreneur, couleur) >= j1 )
											v = 0.25 * v / sqrt( nb/3.0 - j2);
										else
										{
											//	divise par le nombre de plis sans défausse
											v = 0.25 * v / ( nb/3.0 - j2);
										}
										PtDefausse -= v;        //  Retire des points au coup pour ne pas jouer la plus forte
									}
									else if  ( IsMaitreCouleur(CurrentGame, pJeu, couleur) && nb <= 2 && pJeu->MyCarte[j2].Hauteur <= CAVALIER )
                                    {
                                        //  Pas trop fort, cela vaut le coup d'attendre
                                        PtDefausse -= 2.5 * (DAME - pJeu->MyCarte[j2].Hauteur);
                                    }
								}
							}
						}
						else
						{
							pts[GainAttaque] += Table[j].Valeur;
						}
					}
					else
					{
                    //	Défausse défense
                    //	OK sauf si probabilité de coupe dans la couleur est faible
						if ( pJeu->JoueurCouleur[couleur] != pJeu->PositionPreneur || pJeu->TmpProbCoupe[couleur] > 0.9 )
							pts[GainAttaque] += Table[j].Valeur * pJeu->TmpProbCoupe[couleur] * (1.0 + StyleJoueur[pJeu->PositionJoueur][dStyleDefDefausse]) * exp((CurrentGame->TypePartie-1.0)/3.0);
						else
							pts[GainAttaque] += 0.5 * Table[j].Valeur * pJeu->TmpProbCoupe[couleur] * (1.0 + StyleJoueur[pJeu->PositionJoueur][dStyleDefDefausse]) * exp((CurrentGame->TypePartie-1.0)/3.0);
					}
				}
			}
            //	Le preneur gagne.
			else if ( couleur != CouleurDemandee )
			{
				//	Cas des défausses de la défense.
				pts[GainAttaque] += Table[j].Valeur * (0.5 + 0.5*pJeu->ProbCouleur[Table[j].Couleur]);
				j1 = CarteMaitreNonJoueeNoErr(CurrentGame, couleur);
				if ( Table[j].Index == j1 )
					pts[GainAttaque] += 10.0 * pJeu->ProbCouleur[couleur];	//	Ne jette pas maitre
				nb = NbNonHonneur(CurrentGame, pJeu->PositionJoueur, couleur);
				if ( couleur != Table[igagnant].Couleur && PosC == pJeu->PositionJoueur
					&& pJeu->ProbCouleur[couleur] > 0.01 && !HasCarte(pJeu, pJeu->PositionJoueur, j1)
					&& PointsRestants(pJeu, pJeu->PositionJoueur, couleur) > 0	&& nb <= 2 && nb > 0)
				{
					pts[GainAttaque] += GetPointMinHonneur(pJeu, couleur) / NbNonHonneur(CurrentGame, pJeu->PositionJoueur, couleur);
				}
			}
			else
			{
				pts[GainAttaque] += Table[j].Valeur;
			}
		}
	}
	// Pour le moment retourne seulement la différence de points
	if ( IndexPreneur == 3 )
	{
		return(1.0*(pts[1] - pts[0]));
	}
	return(PtDefausse + pts[0] - pts[1]);
}

//	Calcule l'intérêt de mener le petit au bout
//  Mise à jour de la variable ValPetitAuBout dans la structure pJeu
//  Appelé avant de jouer chaque carte attaque et défense

void GainPetitAuBout(TarotGame CurrentGame, struct _Jeu *pJeu)
{
double p0;

	if ( pJeu->ProbCarte[pJeu->PositionJoueur][1] <= 0.0 )      //  Pas le petit ou déjà joué
	{
		pJeu->ValPetitAuBout = 0;                         //  Valeur 0
	}
	else if (pJeu->PositionJoueur == CurrentGame->JoueurPreneur )
	{
		p0 = ProbaPetitPrenable(pJeu);
		pJeu->ValPetitAuBout = pJeu->ValPetitAuBout0 * (1.0 - p0);
		if ( p0 < 0.001 )
		{
			pJeu->ValPetitAuBout *= 2;
		}
	}
	else if ( (AvgLongueur(pJeu, CurrentGame->JoueurPreneur, ATOUT) < pJeu->NbAtout - 1.2) || AvgLongueur(pJeu, CurrentGame->JoueurPreneur, ATOUT) < 0.001)
	{
		pJeu->ValPetitAuBout = 20.0;
	}
	else
	{
		pJeu->ValPetitAuBout = 0.0;
	}
}



