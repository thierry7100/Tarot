#include <stdlib.h>
#include <gtk/gtk.h>
#include "Tarot_Ui_Objects.h"
#include <assert.h>
#include <math.h>


//	Paramètres de jeu de l'attaque
//	Pas ouvert : Val = K0 + K1 * ProbCoupe + K2*IsMaitre + K3*GetPlusFaible + K4*PetitPrenable
const double K0 = -2.0;
const double K1 = 8.0;
const double K2 = 0.5;
const double K3 = -2.0;
const double K4 = 6.0;
//	OuvertPreneur : Val = K5 + K6*ProbCoupe + K7*IsMaitre + K8*ValPlusFaible + K9*ProbDefausse + K10*PetitPrenable
const double K5 = 0.0;
const double K6 = 8.0;
const double K7 = 0.5;
const double K8 = -2.0;
const double K9 = -0.5;
const double K10 = 6.0;
//	OuvertPossPetit : Val = K11 + K12*ProbCoupe + K13*IsMaitre + K14*ValPlusFaible + K15*ProbDefausse
const double K11 = -3.0;
const double K12 = 8.0;
const double K13 = 0.5;
const double K14 = -2.0;
const double K15 = -0.5;
//	OuvertAutre : Val = K16 + K17*ProbCoupe + K18*IsMaitre + K19*ValPlusFaible + K20*ProbDefausse + K10*PetitPrenable
const double K16 = -1.5;
const double K17 = 8.0;
const double K18 = 0.5;
const double K19 = -2.0;
const double K20 = -0.5;
//	AtoutMaitreEtRestant : Val = K21 + K22*AtoutRestant + K23 * (NbAtout-AtoutRestant) + K24*PetitPrenable
const double K21 = 6.0;
const double K22 = -0.5;
const double K23 = -4.0;
const double K24 = -30.0;
//	AtoutNonMaitreEtRestant : Val = K25 + K26*AtoutRestant + K27 * (NbAtout-AtoutRestant) + K28*PetitPrenable
const double K25 = -1.0;
const double K26 = -0.5;
const double K27 = -4.0;
const double K28 = -30.0;
//	AtoutNonMaitreEtNonRestant : Val = K29 + K30*HasPetit
const double K29 = -35.0;
const double K30 = -20.0;
//	Excuse : Val = K31 + K32 * NbCarte==2
const double K31 = -30.0;
const double K32 = 150.0;

//  Regarde si le petit est prenable directement
//  Les atouts du preneur sont classés par ordre décroissant (joue les forts en tête)
//  nb1 donne le nombre d'atout du défenseur 1 et Max1 son plus gros atout
//  idem pour les 2 autres joueurs avec nb2, Max2, nb3, Max3
//  JoueurPeit donne le n0 du défenseur avec le petit
//  Retourne 1 si OK, 0 sinon

static int PriseDirectePetitOK(struct _Jeu *pJeu, int nb1, int Max1, int nb2, int Max2, int nb3, int Max3, int JoueurPetit)
{
int IndexDefense;  //  Index dans les atouts défense
int IndexPreneur;  //  Index dans les atouts du preneur
int Hauteur;

    for ( IndexDefense = 0, IndexPreneur = pJeu->NbAtout-1; IndexPreneur >= 0; IndexPreneur--,  IndexDefense++)
    {
        //  Le preneur joue les gros en tête
        Hauteur = pJeu->MyCarte[IndexPreneur].Hauteur;
        if ( IndexDefense >= nb1 ) Max1 = -1;
        if ( IndexDefense >= nb2 ) Max2 = -1;
        if ( IndexDefense >= nb3 ) Max3 = -1;
        if ( Hauteur < Max1 ) return 0;
        if ( Hauteur < Max2 ) return 0;
        if ( Hauteur < Max3 ) return 0;
        if ( JoueurPetit == 0 && IndexDefense == nb1 - 1 ) return 1;
        if ( JoueurPetit == 1 && IndexDefense == nb2 - 1 ) return 1;
        if ( JoueurPetit == 2 && IndexDefense == nb3 - 1 ) return 1;
    }
    return 0;
}

//  Retourne 1 si le preneur est capable de faire tous les plis dans la couleur
static int ToutesCartesMaitresses(struct _Jeu *pJeu, int nb1, int Max1, int nb2, int Max2, int nb3, int Max3, int Couleur)
{
int IndexDefense;  //  Index dans les cartes défense
int IndexPreneur;  //  Index dans les cartes du preneur
int Hauteur;
int Nb = Couleur == ATOUT ? pJeu->NbAtout : pJeu->NbCouleur[Couleur];

    IndexPreneur = GetPlusForte(pJeu, Couleur);
    //  Calcule les hauteurs en fonction de l'index.
    if ( nb1 > 0 ) Max1 -= Startof[Couleur] - 1;
    if ( nb2 > 0 ) Max2 -= Startof[Couleur] - 1;
    if ( nb3 > 0 ) Max3 -= Startof[Couleur] - 1;
    for ( IndexDefense = 0; --Nb >= 0; IndexPreneur--,  IndexDefense++)
    {
        //  Le preneur joue les gros en tête
        Hauteur = pJeu->MyCarte[IndexPreneur].Hauteur;
        if ( IndexDefense >= nb1 ) Max1 = -1;       //  Si le joueur n'en a plus, ne compte pas
        if ( IndexDefense >= nb2 ) Max2 = -1;
        if ( IndexDefense >= nb3 ) Max3 = -1;
        if ( Hauteur < Max1 ) return 0;             //  Si la défense a plus gros, retourne 0
        if ( Hauteur < Max2 ) return 0;
        if ( Hauteur < Max3 ) return 0;
    }
    //  Pour la couleur, tout est maître retourne 1
    //  Pour les atouts, il ne faut pas qu'un défenseur en ait plus
    if ( Couleur == ATOUT )
    {
        if ( IndexDefense < nb1 ) return 0;
        if ( IndexDefense < nb2 ) return 0;
        if ( IndexDefense < nb3 ) return 0;
    }
    return 1;
}

//  Génère toutes les répartitions de cartes dans une Couleur pour calculer la probabilité de faire tous les plis dans cette couleur

static void GenereRepartitionCouleur(TarotGame CurrentGame, struct _Jeu *pJeu, int Preneur, int Couleur, int DefenseCouleur[3][22], int nb1, int nb2, int nb3, double *SommeProb, double *SommeOK, int index)
{
int i;
double p;
int j1, j2, j3;
int Max1, Max2, Max3;

    //  Cherche première carte non affectée dans cette couleur
    for ( ; index < Endof[Couleur]; index++ )
    {
        if ( pJeu->StatusCartes[index] == 0 )       //  N'a pas la carte et pas encore jouée
            break;
    }
    if ( index >= Endof[Couleur] )        //  Plus besoin de continuer ?
    {
        //  Plus grosse carte de chaque joueur
        Max1 = -1;
        if ( nb1 > 0 ) Max1 = DefenseCouleur[0][nb1-1];
        Max2 = -1;
        if ( nb2 > 0 ) Max2 = DefenseCouleur[1][nb2-1];
        Max3 = -1;
        if ( nb3 > 0 ) Max3 = DefenseCouleur[2][nb3-1];
        p = 1.0;                //  Proba de cette distribution
        j1 = Preneur ^ 1;
        j2 = Preneur ^ 2;
        j3 = Preneur ^ 3;
        for ( i = 0; i < nb1; i++ )
        {
            p *= pJeu->ProbCarte[j1][DefenseCouleur[0][i]];
            if ( p < 1e-20) return;     //  Abandonne trop faible...
        }
        for ( i = 0; i < nb2; i++ )
        {
            p *= pJeu->ProbCarte[j2][DefenseCouleur[1][i]];
            if ( p < 1e-20) return;     //  Abandonne trop faible...
        }
        for ( i = 0; i < nb3; i++ )
        {
            p *= pJeu->ProbCarte[j3][DefenseCouleur[2][i]];
            if ( p < 1e-20) return;     //  Abandonne trop faible...
        }
        //  p donne la probabilité de cette distribution
        *SommeProb += p;
        if ( ToutesCartesMaitresses(pJeu, nb1, Max1, nb2, Max2, nb3, Max3, Couleur) )
            *SommeOK += p;
        return;
    }
    //  Affecte la carte index au joueur 1
    DefenseCouleur[0][nb1] = index;
    GenereRepartitionCouleur(CurrentGame, pJeu, Preneur, Couleur, DefenseCouleur, nb1+1, nb2, nb3, SommeProb, SommeOK, index+1);
    for ( i = nb1; i < 22; i++ ) DefenseCouleur[0][i] = -1;        // Efface fin du tableau
    for ( i = nb2; i < 22; i++ ) DefenseCouleur[1][i] = -1;        // Efface fin du tableau
    for ( i = nb3; i < 22; i++ ) DefenseCouleur[2][i] = -1;        // Efface fin du tableau
    //  Affecte la carte index au joueur 2
    DefenseCouleur[1][nb2] = index;
    GenereRepartitionCouleur(CurrentGame, pJeu, Preneur, Couleur, DefenseCouleur, nb1, nb2+1, nb3, SommeProb, SommeOK, index+1);
    for ( i = nb1; i < 22; i++ ) DefenseCouleur[0][i] = -1;        // Efface fin du tableau
    for ( i = nb2; i < 22; i++ ) DefenseCouleur[1][i] = -1;        // Efface fin du tableau
    for ( i = nb3; i < 22; i++ ) DefenseCouleur[2][i] = -1;        // Efface fin du tableau
    //  Affecte la carte index au joueur 3
    DefenseCouleur[2][nb3] = index;
    GenereRepartitionCouleur(CurrentGame, pJeu, Preneur, Couleur, DefenseCouleur, nb1, nb2, nb3+1, SommeProb, SommeOK, index+1);
}
//  Génère toutes les répartitions possibles d'atout dans la défense pour savoir si le petit est prenable

static void GenereRepartitionAtout(TarotGame CurrentGame, struct _Jeu *pJeu, int Preneur, int DefenseAtouts[3][22], int nb1, int nb2, int nb3, double *SommeProb, double *SommeOK, int index)
{
int i;
double p;
int j1, j2, j3;
int Max1, Max2, Max3;
static int JoueurPetit;
int Delta;

    //  Cherche premier atout non affecté
    for ( ; index < 22; index++ )
    {
        if ( pJeu->StatusCartes[index] == 0 )       //  N'a pas la carte et pas encore jouée
            break;
    }
    if ( index >= 22 )        //  Plus besoin de continuer ?
    {
        //  Plus gros atout de chaque joueur
        Max1 = -1;
        if ( nb1 > 0 ) Max1 = DefenseAtouts[0][nb1-1];
        Max2 = -1;
        if ( nb2 > 0 ) Max2 = DefenseAtouts[1][nb2-1];
        Max3 = -1;
        if ( nb3 > 0 ) Max3 = DefenseAtouts[2][nb3-1];
        //  Fin, lance le calcul, mais avant stocke petit en fin de tableau
        if ( JoueurPetit ==  0 )
        {
            if ( HasJoueCouleur(CurrentGame, JoueurPetit, ATOUT) == 0 && nb1 == 1 ) return;     //  Impossible, petit sec
            DefenseAtouts[0][nb1] = 1;
            nb1++;
        }
        else if ( JoueurPetit == 1 )
        {
            if ( HasJoueCouleur(CurrentGame, JoueurPetit, ATOUT) == 0 && nb2 == 1 ) return;     //  Impossible, petit sec
            DefenseAtouts[1][nb2] = 1;
            nb2++;
        }
        else
        {
            if ( HasJoueCouleur(CurrentGame, JoueurPetit, ATOUT) == 0 && nb3 == 1 ) return;     //  Impossible, petit sec
            DefenseAtouts[2][nb3] = 1;
            nb3++;
        }
        p = 1.0;                //  Proba de cette distribution
        j1 = Preneur ^ 1;
        j2 = Preneur ^ 2;
        j3 = Preneur ^ 3;
        for ( i = 0; i < nb1; i++ )
        {
            p *= pJeu->ProbCarte[j1][DefenseAtouts[0][i]];
            if ( p < 1e-20) return;     //  Abandonne trop faible...
        }
        for ( i = 0; i < nb2; i++ )
        {
            p *= pJeu->ProbCarte[j2][DefenseAtouts[1][i]];
            if ( p < 1e-20) return;     //  Abandonne trop faible...
        }
        for ( i = 0; i < nb3; i++ )
        {
            p *= pJeu->ProbCarte[j3][DefenseAtouts[2][i]];
            if ( p < 1e-20) return;     //  Abandonne trop faible...
        }
        //  p donne la probabilité de cette distribution
        *SommeProb += p;
        if ( PriseDirectePetitOK(pJeu, nb1, Max1, nb2, Max2, nb3, Max3, JoueurPetit) )
            *SommeOK += p;
        return;
    }
    //  Affecte l'atout index au joueur 1
    Delta = 1 - (index == 1);     //  Si petit, n'avance pas dans le tableau des atouts
    if ( index == 1 )
        JoueurPetit = 0;
    else
        DefenseAtouts[0][nb1] = index;
    GenereRepartitionAtout(CurrentGame, pJeu, Preneur, DefenseAtouts, nb1+Delta, nb2, nb3, SommeProb, SommeOK, index+1);
    for ( i = nb1; i < 22; i++ ) DefenseAtouts[0][i] = -1;        // Efface fin du tableau
    for ( i = nb2; i < 22; i++ ) DefenseAtouts[1][i] = -1;        // Efface fin du tableau
    for ( i = nb3; i < 22; i++ ) DefenseAtouts[2][i] = -1;        // Efface fin du tableau
    //  Affecte l'atout index au joueur 2
    if ( index == 1 )
        JoueurPetit = 1;
    else
        DefenseAtouts[1][nb2] = index;
    GenereRepartitionAtout(CurrentGame, pJeu, Preneur, DefenseAtouts, nb1, nb2+Delta, nb3, SommeProb, SommeOK, index+1);
    for ( i = nb1; i < 22; i++ ) DefenseAtouts[0][i] = -1;        // Efface fin du tableau
    for ( i = nb2; i < 22; i++ ) DefenseAtouts[1][i] = -1;        // Efface fin du tableau
    for ( i = nb3; i < 22; i++ ) DefenseAtouts[2][i] = -1;        // Efface fin du tableau
    //  Affecte l'atout index au joueur 2
    if ( index == 1 )
        JoueurPetit = 2;
    else
        DefenseAtouts[2][nb3] = index;
    GenereRepartitionAtout(CurrentGame, pJeu, Preneur, DefenseAtouts, nb1, nb2, nb3+Delta, SommeProb, SommeOK, index+1);
}

//  Calcule la probabilité de prendre le petit en jouant les gros atouts d'abord.
//  Fait ce travail pour le preneur
//  Pour cela génère toutes les possibilités de positionnement des atouts au niveau défense

static double CalcProbPriseDirectePetit(TarotGame CurrentGame, struct _Jeu *pJeu, int Preneur)
{
int DefenseAtouts[3][22];
double SommeProb = 0;
double SommeOK = 0;
double val;

    //  Pas la peine si as le 21 (et pas déjà jouée)!
    if ( CurrentGame->CarteJouee[21] < 0 && pJeu->ProbCarte[pJeu->PositionJoueur][21] < 1.0 ) return 0;
    //  Pas la peine si pas le 20 (et pas déjà jouée)!
    if ( pJeu->ResteAtout > 10 && CurrentGame->CarteJouee[20] < 0 && pJeu->ProbCarte[pJeu->PositionJoueur][20] < 1.0 ) return 0;
    //  Pas la peine si pas le 19 (et pas déjà jouée)!
    if ( pJeu->ResteAtout > 10 && CurrentGame->CarteJouee[19] < 0 && pJeu->ProbCarte[pJeu->PositionJoueur][19] < 1.0 ) return 0;

    memset(DefenseAtouts, -1, sizeof(DefenseAtouts));
    GenereRepartitionAtout(CurrentGame, pJeu, Preneur, DefenseAtouts, 0, 0, 0, &SommeProb, &SommeOK, 0);
    val = SommeOK / SommeProb;
    assert(!isnan(val));
    return (val);
}

//  Calcule la probabilité de faire un chelem par le preneur
double CalcProbaChelem(TarotGame CurrentGame)
{
int DefenseCouleur[3][22];
double SommeProb = 0;
double SommeOK = 0;
double val;
int c;
double ProbChelem = 1.0;
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[CurrentGame->JoueurPreneur];
int StartIdx;

    for ( c = ATOUT; c < NB_COULEUR; c++)
    {
        SommeOK = 0;
        SommeProb = 0;
        if ( c == ATOUT && pJeu->NbAtout == 0 ) return 0.0;
        if ( c > ATOUT && pJeu->NbCouleur[c] == 0 ) continue;
        if ( !IsMaitreCouleur(CurrentGame, pJeu, c) ) return 0.0;
        if ( c == ATOUT && pJeu->ProbCarte[CurrentGame->JoueurPreneur][20] == 0 && pJeu->NbAtout < 16 ) return 0.0;
        if ( c == ATOUT && pJeu->ProbCarte[CurrentGame->JoueurPreneur][19] == 0 && pJeu->NbAtout < 13 ) return 0.0;
        if ( c > ATOUT && NbMaitreCouleur(CurrentGame, pJeu, c) == pJeu->NbCouleur[c] ) continue;
        memset(DefenseCouleur, -1, sizeof(DefenseCouleur));
        StartIdx =  Startof[c];
        if ( !HasExcuse(pJeu) )
            StartIdx--;             //  Rallonge le joueur avec l'excuse
        GenereRepartitionCouleur(CurrentGame, pJeu, CurrentGame->JoueurPreneur, c, DefenseCouleur, 0, 0, 0, &SommeProb, &SommeOK, StartIdx);
        assert(SommeProb > 0);
        val = SommeOK / SommeProb;
        ProbChelem *= val;
    }
#if DEBUG > 0
    OutDebug("Proba Chelem = %.2f%%\n", ProbChelem*100.0);
#endif // DEBUG
    return ProbChelem;
}

//	Décide au début de l'intérêt de mener le petit au bout
void InitPetitAuBout(TarotGame CurrentGame)
{
int c;
int nbCoupe = 0;
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[CurrentGame->JoueurPreneur];

	if ( !HasPetit(pJeu))
	{
		pJeu->ValPetitAuBout0 = 0;
		return;
	}
	for ( c = TREFLE; c < NB_COULEUR; c++)
	{
		if ( pJeu->NbCouleur[c] == 0 ) nbCoupe++;		//	Nombre de coupes
	}
	//  30 pour petit au bout, retire 4 points par coupe
	pJeu->ValPetitAuBout0 = 30.0 - nbCoupe * 4.0;
#if DEBUG > 0
    OutDebug("Valeur initiale petit au bout = %.3f, nbCoupe=%d, NbAtout=%d\n", pJeu->ValPetitAuBout0, nbCoupe, pJeu->NbAtout);
#endif // DEBUG
}



//  Retourne la "force" de la couleur la plus forte du Preneur

const int Honneur2Force[16] = { 0, 2, 3, 4, 5, 6, 8, 10, 8, 10, 10, 13, 14, 16, 18, 20};

double GetForceCouleurs(struct _Jeu *pJeu)
{
double f = 0;
int r;
int i;
int force = 0;

	for ( int c = TREFLE; c < NB_COULEUR; c++)
	{
		if ( pJeu->NbCouleur[c] <= 3 ) continue;
		force = 0;
		for ( i = 0; i < pJeu->NbCarte; i++)
		{
			if ( pJeu->MyCarte[i].Couleur != c ) continue;
			if ( pJeu->MyCarte[i].Hauteur >= VALET )
				force |= 1 << (pJeu->MyCarte[i].Hauteur - VALET);
		}
		assert(force >= 0 && force < 16);
		r = pJeu->NbCouleur[c] + Honneur2Force[force];
		if ( r > f )
		{
			f = r;
		}
	}
	return(f);
}

//	Retourne la probabilité de coupe de la couleur c par les autres joueurs
//	Appelée uniquement par le preneur.

static double GetProbCoupe(struct _Jeu *pJeu, int c)
{
double p, p0;
int i, j;

	p0 = 1.0;
	for ( j = 0; j < 4; j++)		//	Pour chaque joueur
	{
		p = 1.0;
		if ( j == pJeu->PositionJoueur ) continue;
        //	Regarde si le joueur a de l'atout. Sinon, il ne pourra pas couper.
		if ( GetProbAtout(pJeu, j) < 0.001 ) continue;
        //	Calcule la probabilité de coupe dans la couleur c pour le joueur j
        //  Multiplie les probabilités de NE PAS avoir les cartes de la couleur
		for ( i = Startof[c] ; i < Endof[c]; i++)
		{
			p *= 1.0 - pJeu->TmpProbCarte[j][i];
		}
        //	Accumule p(NoCoupe)
		p0 *= 1.0 - p;
	}
	//	Retourne la proba de coupe = 1 - Prod(Proba(NoCoupe))
	return(1.0 - p0);
}

//	évalue les pertes possibles suite a défausse des défenseurs si le preneur joue la couleur Couleur
//  Retourne cette valeur

double CoutDefausse(struct _Jeu *pJeu, int Couleur)
{
double val = 0;
double prob_coupe, proba_sans_atout;
int k;
int i;

	for ( i = 0; i < 4; i++ )       //  Pour chaque joueur
	{
		if ( i == pJeu->PositionJoueur ) continue;
		prob_coupe = 1.0;		        //	Calcule la proba de coupe dans la couleur c
		for ( k = Startof[Couleur] ; k < Endof[Couleur]; k++)
		{
			prob_coupe *= 1.0 - pJeu->TmpProbCarte[i][k];
		}
		proba_sans_atout = 1.0;		//	Calcule la proba sans Atout
		for ( k = 1; k < 22; k++)
		{
			proba_sans_atout *= 1.0 - pJeu->TmpProbCarte[i][k];
		}
		//  Coût comptabilisé : 5 points - le nombre de défausse déjà réalisées
		val += prob_coupe * proba_sans_atout * (5 - pJeu->NbDefausse[i]);
	}
	return(val);
}

//	Calcule la probabilité que le petit soit prenable c'est à dire qu'un joueur possède au moins autant d'atouts que le preneur
//	Toujours appelé par le preneur.

double ProbaPetitPrenable(struct _Jeu *pJeu)
{
int NbAtoutRestant = pJeu->ResteAtout;
int Nb = pJeu->NbAtout;
int n;
double NewProb;
int pos;
int Position = pJeu->PositionJoueur;

	if ( pJeu->ProbCarte[Position][1] < 1.0 ) return(0.0);		//	Ne l'a pas, on ne peut pas lui prendre...
	NewProb = 0.0;
	for ( pos = 0; pos < 4; pos++)
	{
		if ( pos == Position ) continue;
		for ( n = Nb; n <= NbAtoutRestant; n++)
			NewProb += ProbaExactementN(pJeu, pos, n, 1, 22);
	}
	return(NewProb);
}


//  Intérêt de chasser suivant le nombre de bouts du preneur
//  Pas d'intérêt si déjà 2 bouts. Très intéressant si 1 bout (delta petit = 15 points)

static const int InteretChasse[4] = { 8, 10, 0, 0};

//  Décide si le preneur peut ou doit aller à la chasse
//  Mise à jour au fur et à mesure des plis réalisés
//  Retourne la valeur de StatusChasse

int StrategieChasseAttaque(TarotGame CurrentGame, struct _Jeu *pJeu, int Position)
{
double Chasse = 5.0;
int NbCoupe;
double NAtoutChasse;
double facteurChasse;
int i;
double p;
double ForceCouleur;

    if ( HasCarte(pJeu, Position, 1) )
    {
        pJeu->StatusChasse = CHASSE_POSSEDE_PETIT;
        return CHASSE_POSSEDE_PETIT;
    }
    if ( CurrentGame->CarteJouee[1] >= 0 )   //  Petit déjà joué ?
    {
        pJeu->StatusChasse = CHASSE_TERMINEE;		//	Plus d'atout, pas (plus) la peine
        return CHASSE_TERMINEE;
    }
	if ( pJeu->NbAtout == 0 || (pJeu->NbAtout == 1 &&  HasCarte(pJeu, Position, 0)) )
    {
		pJeu->StatusChasse = CHASSE_TERMINEE;		//	Plus d'atout, pas (plus) la peine
        return CHASSE_TERMINEE;
    }
    //  Si le possesseur du petit (le plus probable) a plus d'atout que le preneur, passe en ATTENTE
	if ( LongueurMoyenneCouleur(pJeu, JoueurAvecPetit(CurrentGame, pJeu), ATOUT) > pJeu->NbAtout )
    {
		pJeu->StatusChasse = CHASSE_ATTENTE;
        return CHASSE_ATTENTE;
    }
    //  Idem si le preneur commence à avoir beaucoup moins d'atouts que la défense
    //  L'excuse ne sert à rien pour la chasse !
    NAtoutChasse = pJeu->NbAtout - HasCarte(pJeu, Position, 0);
    if ( (NAtoutChasse / pJeu->ResteAtout) < 0.4 )
    {
        pJeu->StatusChasse = CHASSE_ATTENTE;
        return CHASSE_ATTENTE;
    }
    //	Décide d'entamer ou non la chasse...
	if ( pJeu->StatusChasse == CHASSE_ATTENTE  )
	{
        //  Intérêt de la chasse fonction du nombre de bouts du preneur.
		Chasse = InteretChasse[pJeu->NbBout];
		//  Compte les coupes du preneur
		NbCoupe = 0;
		for ( i = TREFLE; i < NB_COULEUR; i++)
		{
			if ( pJeu->NbCouleur[i] == 0 ) NbCoupe++;
		}
		//	Pour chasser, mieux vaut avoir plein d'atouts...
		//  Estimation succès chasse fonction du rapport entre le nombre d'atout du preneur et les atouts restants
		//  Facteur pris en compte 4*(NAtoutChasse / ResteAtout) au carré.
		facteurChasse = NAtoutChasse / pJeu->ResteAtout;
		//  Si preneur a autant d'atout que les autres réunis + 28 points (environ 10 atouts chez preneur)
		//  Si preneur moitié moins d'atouts que les autres  +1.75 points (environ 7 atouts chez le preneur en début de partie)
		//  Si NAtoutChasse / pJeu->ResteAtout < 0.4 ne va PAS à la chasse
		if ( facteurChasse < 0.4 )
		{
            facteurChasse = -4.0 / (facteurChasse * facteurChasse);
		}
		else
        {
            facteurChasse *= 4.0*facteurChasse;
        }
		Chasse += facteurChasse;
		//	et surtout des atouts majeurs --> + 2 points par atout majeur
		Chasse += pJeu->NbAtoutMaj * 2;
		//	On retire des points pour chaque coupe qui consomment des atouts (-4 par coupe)
		Chasse -= NbCoupe * (4.0 - facteurChasse) ;
		//	Si longue faible, il faut éviter de chasser
		//	Mais si couleur forte, il faut faire tomber les atouts...
		ForceCouleur = GetForceCouleurs(pJeu);
		if ( ForceCouleur < 10 ) Chasse -= 5.0*(10.0-ForceCouleur);
		if ( ForceCouleur >= 12 && pJeu->NbAtout >= 9 ) Chasse += (2.0 + facteurChasse)*(ForceCouleur-12.0);
		Chasse += StyleJoueur[Position][dStyleAttChasse] * 5;
		if ( Chasse >= 20 )		//	Va a la chasse au petit
		{
			pJeu->StatusChasse = CHASSE_PRENEUR_DESSOUS;	//	Chasse lancée, par en dessous par défaut
            p = CalcProbPriseDirectePetit(CurrentGame, pJeu, Position);
			if ( p > 0.85 || (p > 0.75 && pJeu->NbBout < 2) )
			{
				pJeu->StatusChasse = CHASSE_PRENEUR_GROS;
			}
		}
	}
    return pJeu->StatusChasse;
}


//  Choix de la carte à jouer dans une couleur (entame) par le preneur
//  Idée de base : si proba de coupe faible, joue les cartes maîtresses (si c'est le cas)
//  Sinon, joue plutôt faible
//  Si petit prenable, joue les forts même si risqué.
//  hMax va donner la hauteur maximale que l'on pourra avoir
//  Retourne l'index de la carte choisie

int ChoixCarteAttaque(TarotGame CurrentGame, struct _Jeu *pJeu, int Couleur, double p_coupe, double proba_petit_prenable, double *ValCouleur)
{
int hMax;
int i1;

    if ( p_coupe < 0.25 ) hMax = ROI;
    else if ( p_coupe < 0.40 ) hMax = DAME;
    else if ( p_coupe < 0.7 ) hMax = CAVALIER;
    else if ( p_coupe < 0.9 ) hMax = VALET;
    else hMax = 10;
    hMax += proba_petit_prenable*10.0;      //  remonte valeur si petit prenable

    if ( IsMaitreCouleur(CurrentGame, pJeu, Couleur) && GetCartePlusForte(pJeu, Couleur)->Hauteur <= hMax )
    {
        i1 = GetPlusForte(pJeu, Couleur);
        if ( proba_petit_prenable > 0.5 ) *ValCouleur += 2.0*p_coupe;
    }
    else
    {
        i1 = GetPlusFaible(pJeu, Couleur);
        if ( pJeu->MyCarte[i1].Hauteur > hMax ) *ValCouleur -= 2.0;       //  Retire des points si trop fort !
    }
    return i1;
}


//	Entame en position d'attaque
//  Le Preneur est le premier à jouer

#if DEBUG > 0
#define DEBUG_ATTAQUE_PREMIER 0
#else
#define DEBUG_ATTAQUE_PREMIER 0
#endif  // DEBUG

void JoueEnPremierAttaque(TarotGame CurrentGame)
{
int Position = CurrentGame->JoueurPreneur;
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[Position];
int PossPetit = JoueurAvecPetit(CurrentGame, pJeu);                 //  Position de celui qui a le petit
double proba_petit_prenable = ProbaPetitPrenable(pJeu);             //  Proba petit prenable par la défense
int AtoutVrais = pJeu->NbAtout - HasCarte(pJeu, Position, 0);       //  Atouts sans excuse
int pos;
int c;
int i0;
double p_coupe;
double p0;
double BestScore = -1e6;
int BestCouleur = -1;
double ValCouleur[6];
int CarteCouleur[6];
double nDis = nbDistrib(CurrentGame, pJeu);

    StrategieChasseAttaque(CurrentGame, pJeu, Position);
	pJeu->ResteAtout = NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT);
	GainPetitAuBout(CurrentGame, pJeu);
#if DEBUG_ATTAQUE_PREMIER > 0
    OutDebug("--------------Pli %d--------------------------------\n", CurrentGame->NumPli+1);
    OutDebug("JoueEnPremierAttaque : NbCarte = %d\n", pJeu->NbCarte);
    OutDebug("JoueEnPremierAttaque: StrategieChasse --> pJeu->StatusChasse ==%d\n", pJeu->StatusChasse);
    OutDebug("JoueEnPremierAttaque : ResteAtout = %d\n", pJeu->ResteAtout);
    OutDebug("Nombre de distributions %.0f\n", nDis);
#endif // DEBUG_ATTAQUE_PREMIER
    if ( pJeu->NbCarte > 1 && pJeu->NbCarte <= MAX_CARTE_FIN_PARTIE_ATTAQUE && nDis <= NOMBRE_DISTRIB_FIN_PARTIE_ATTAQUE )
    {
        i0 = FinPartie(CurrentGame);
        if ( i0 >= 0 )
        {
            PoseCarte(CurrentGame, i0);
            return;
        }
    }
    if ( pJeu->StatusChasse == CHASSE_PRENEUR_GROS )
    {
		if ( IsMaitreCouleur(CurrentGame, pJeu, ATOUT) )	//	Vérifie si maître...
        {
            //  Joue le plus fort des atouts : c'est forcément le dernier (ils sont classés).
            PoseCarte(CurrentGame, pJeu->NbAtout-1);
            return;
        }
        pJeu->StatusChasse = CHASSE_PRENEUR_DESSOUS;    //  Repasse dans le mode "par dessous"
    }
    if ( pJeu->StatusChasse == CHASSE_PRENEUR_DESSOUS )
    {
        //  Vérifie quand même que ce n'est pas le moment de mettre les gros
        p0 = CalcProbPriseDirectePetit(CurrentGame, pJeu, Position);
#if DEBUG_ATTAQUE_PREMIER > 0
        OutDebug("CalcProbPriseDirectePetit = %.3f\n", p0);
#endif // DEBUG_ATTAQUE_PREMIER
        if ( p0 > 0.80 || (p0 > 0.70 && pJeu->NbBout < 2) )
        {
            pJeu->StatusChasse = CHASSE_PRENEUR_GROS;       //  Repasse dans ce mode
            //  Joue le plus fort des atouts : c'est forcément le dernier (ils sont classés).
            PoseCarte(CurrentGame, pJeu->NbAtout-1);
            return;
        }
        //  Regarde où est le Petit
        PossPetit = JoueurAvecPetit(CurrentGame, pJeu);
        pos = (PossPetit - Position) & 3;		//	h : distance entre preneur et petit.
		if ( pos <= 2 || pJeu->TmpProbCarte[PossPetit][1] < 0.5 )	    //	Si pas en dernier ou pas vraiment sur de la position essaie en dessous
		{
            //  Dans ce cas joue un petit atout (son plus petit en fait) :
            i0 = HasCarte(pJeu, Position, 0);   //  1 si possède l'excuse, sinon 0 --> Plus petit atout
            if ( pJeu->MyCarte[i0].Hauteur >= 12 && NbMaitreCouleur(CurrentGame,pJeu, ATOUT) >= 2)  //  Petit trop gros, joue les gros
                i0 = pJeu->NbAtout - 1;
            PoseCarte(CurrentGame, i0);
            return;
		}
        //  Trop risqué ne joue pas atout, repasse sans la chasse pour le moment
    }
//	Ne joue pas atout à priori. Calcule la meilleure couleur a jouer
//	Pour chaque couleur calcule la valeur à priori
//	Commence par les couleurs normales
	for ( c = TREFLE; c < NB_COULEUR; c++ )
	{
		if ( pJeu->NbCouleur[c] == 0 )
		{
			ValCouleur[c] = -100000.0;          //  Pas de carte, ne va pas jouer cette couleur !
			CarteCouleur[c] = -1;               //  Initialise quand même
			continue;
		}
        p_coupe = GetProbCoupe(pJeu, c);
        if ( pJeu->JoueurCouleur[c] < 0 )	//	Pas encore jouée, doit-on ouvrir dans cette couleur
        {
            //	Pas ouvert : Val = K0 + K1 * ProbCoupe + K2*IsMaitre + K3*GetPlusFaible + K4*PetitPrenable

            ValCouleur[c] = K0 + K1*p_coupe + K2*IsMaitreCouleur(CurrentGame, pJeu, c) + K3*GetCartePlusFaible(pJeu, c)->Valeur + K4*proba_petit_prenable;
            CarteCouleur[c] = ChoixCarteAttaque(CurrentGame, pJeu, c, p_coupe, proba_petit_prenable, &ValCouleur[c]);
        }
        //	Si Couleur ouverte par preneur
        //	Val = K5 + K6*ProbCoupe + K7*IsMaitreCouleur + K8*ValPlusFaible + K9*ProbDefausse + K10*PetitPrenable
		else if ( pJeu->JoueurCouleur[c] == Position )
		{
			ValCouleur[c] = K5 + K6*p_coupe + K7*IsMaitreCouleur(CurrentGame, pJeu, c) + K8*GetCartePlusFaible(pJeu, c)->Valeur
			             + K9*CoutDefausse(pJeu, c) + K10*proba_petit_prenable;
            CarteCouleur[c] = ChoixCarteAttaque(CurrentGame, pJeu, c, p_coupe, proba_petit_prenable, &ValCouleur[c]);
		}
        //	Couleur Ouverte par possesseur du petit
        //	Val = K11 + K12*ProbCoupe + K13*IsMaitreCouleur + K14*ValPlusFaible + K15*ProbDefausse
		else if ( pJeu->JoueurCouleur[c] == PossPetit )
		{
			ValCouleur[c] = K11 + K12*p_coupe + K13*IsMaitreCouleur(CurrentGame, pJeu, c) + K14*GetCartePlusFaible(pJeu, c)->Valeur
                          + K15*CoutDefausse(pJeu, c);
            CarteCouleur[c] = ChoixCarteAttaque(CurrentGame, pJeu, c, p_coupe, 0.0, &ValCouleur[c]);
		}
		else
        {
            //  Dernier cas, ouvert par un défenseur qui n'a pas le petit a priori
            //	OuvertAutre : Val = K16 + K17*ProbCoupe + K18*IsMaitre + K19*ValPlusFaible + K20*ProbDefausse + K10*PetitPrenable
            ValCouleur[c] = K16 + K17*p_coupe + K18*IsMaitreCouleur(CurrentGame, pJeu, c) + K19*GetCartePlusFaible(pJeu, c)->Valeur
                            + K20*CoutDefausse(pJeu, c) + K21*proba_petit_prenable;
            CarteCouleur[c] = ChoixCarteAttaque(CurrentGame, pJeu, c, p_coupe, proba_petit_prenable, &ValCouleur[c]);
        }
        if ( BestScore < ValCouleur[c] )
        {
            BestScore = ValCouleur[c];
            BestCouleur = c;
        }
#if DEBUG_ATTAQUE_PREMIER > 0
    OutDebug("JoueEnPremierAttaque: ValCouleur[%d]=%.3f\n", c, ValCouleur[c]);
#endif // DEBUG_ATTAQUE_PREMIER
	}
	//  Maintenant valeur ATOUT (même si ne chasse pas)
	//  Premier cas : simple : pas d'atout ou seulement l'excuse !
	if ( AtoutVrais == 0 )
	{
		ValCouleur[ATOUT] = -100000;
	}
	else
	{
#if DEBUG_ATTAQUE_PREMIER > 0
        OutDebug("JoueEnPremierAttaque: Cas ATOUT, AtoutVrais=%d, ResteAtout=%d\n", AtoutVrais, pJeu->ResteAtout);
#endif // DEBUG_ATTAQUE_PREMIER
		if ( IsMaitreCouleur(CurrentGame, pJeu, ATOUT) && pJeu->ResteAtout > 0)
		{
        //	Possède l'atout maître., il reste de l'atout à la défense
        //	AtoutMaitreEtRestant : Val = K21 + K22*AtoutRestant + K23 * (NbAtout-AtoutRestant) + K24*PetitPrenable
        //  TODO : A MODIFIER, ne joue pas assez ATOUT, au moins en début de partie
			ValCouleur[ATOUT] = K21 + K22*pJeu->ResteAtout + K23*exp((pJeu->ResteAtout-AtoutVrais)*2.0/sqrt(AtoutVrais)) + K24*proba_petit_prenable;
        //	Cas ou possède plusieurs gros atouts
			if ( AtoutVrais >= 4 && NbPossCouleur(pJeu, ATOUT) >= 2 && pJeu->ResteAtout-AtoutVrais < 3 && NbGrosAtout(CurrentGame, pJeu, Position, 5) >= 3 )
			{
				ValCouleur[ATOUT] += 2.0 * NbGrosAtout(CurrentGame, pJeu, Position, 5);
			}
            //	Cas ou manque plusieurs gros atouts ( 4 parmi les 6 ), joue un ATOUT moyen pour faire tomber les gros (au début seulement)
			else if ( AtoutVrais >= 7 && NbGrosAtout(CurrentGame, pJeu, Position, 6) <= 2 && pJeu->NbCarte >= 15 )
			{
				ValCouleur[ATOUT] += 10.0 - 2.0 * NbGrosAtout(CurrentGame, pJeu, Position, 5);
			}
#if DEBUG_ATTAQUE_PREMIER > 0
            OutDebug("JoueEnPremierAttaque: Cas ATOUT, ValCouleur[ATOUT]=%.3f\n", ValCouleur[ATOUT]);
#endif // DEBUG_ATTAQUE_PREMIER
			if ( pJeu->ResteAtout == 1 && AtoutVrais == 1 )	//	Possède un seul atout, la défense aussi
			{	//	Reste un seul atout. Le joue si possibilité de plis derrière...
			    for ( c = TREFLE; c < NB_COULEUR; c++ )
                {
                    //  Compte les plis et points possibles dans les autres couleurs
                    if ( pJeu->NbCouleur[c] == 0 ) continue;        //  Il faut en avoir...
                    ValCouleur[ATOUT] += NbMaitreCouleur(CurrentGame, pJeu, c) * 10.0;
#if DEBUG_ATTAQUE_PREMIER > 0
                    OutDebug("JoueEnPremierAttaque: Cas 1 ATOUT Maitre regarde couleur %d, ValCouleur[ATOUT]=%.3f\n", c, ValCouleur[ATOUT]);
#endif // DEBUG_ATTAQUE_PREMIER
                }
			}
		}
		else if ( pJeu->ResteAtout > 0 )
		{
            //	Ne possède pas l'atout maître
            //	AtoutNonMaitreEtRestant : Val = K25 + K26*AtoutRestant + K27 * (NbAtout-AtoutRestant) + K28*PetitPrenable
			ValCouleur[ATOUT] = K25 + K26*pJeu->ResteAtout + K27*exp((pJeu->ResteAtout-AtoutVrais)*2.0/sqrt(AtoutVrais)) + K28*proba_petit_prenable;
            //	Cas ou possède plusieurs gros atouts ( 3 parmi les 5 )
			if ( AtoutVrais >= 4 && NbPossCouleur(pJeu, ATOUT) >= 2 && pJeu->ResteAtout-AtoutVrais < 3 && NbGrosAtout(CurrentGame, pJeu, Position, 5) >= 3 )
			{
				ValCouleur[ATOUT] += 3.0 + 2.0 * NbGrosAtout(CurrentGame, pJeu, Position, 5);
			}
            //	Cas ou manque plusieurs gros atouts ( 4 parmi les 6 ), joue un ATOUT moyen pour faire tomber les gros (au début seulement)
			else if ( AtoutVrais >= 7 && NbGrosAtout(CurrentGame, pJeu, Position, 6) <= 2 && pJeu->NbCarte >= 15 )
			{
				ValCouleur[ATOUT] += 10.0 - 2.0 * NbGrosAtout(CurrentGame, pJeu, Position, 5);
			}
			if ( pJeu->ResteAtout == 1 && AtoutVrais == 1 )	//	Ne joue pas le dernier non maître, corrige donc à la baisse (K27 < 0)
			{
				ValCouleur[ATOUT] += 3.5 * ( 2.0 / pJeu->NbCarte) * K27;
			}
		}
		else
		{
		    //  Plus d'atout côté défense, pas d'intérêt à jouer ATOUT.
			ValCouleur[ATOUT] = K29 + K30*HasCarte(pJeu, Position, 1);
		}
		//  Maintenant choisit la carte à jouer si joue ATOUT
		if ( IsMaitreCouleur(CurrentGame, pJeu, ATOUT) )			//	Possède ATOUT Maître
		{
			if ( ProbUnDefenseurSansAtout(pJeu) < 0.1 )	            //	Les défenseurs possèdent de l''ATOUT ?
				CarteCouleur[ATOUT] = pJeu->NbAtout/2 + 1;		    //	Oui, joue moyen
			else
				CarteCouleur[ATOUT] = GetPlusForte(pJeu, ATOUT);	//	non, joue gros pour ne pas perdre de points
		}
		else
		{
			if ( ProbUnDefenseurSansAtout(pJeu) < 0.2 )
				CarteCouleur[ATOUT] = pJeu->NbAtout/2 + 1;
			else
				CarteCouleur[ATOUT] = GetPlusForte(pJeu, ATOUT);
		}
	}
	if ( BestScore < ValCouleur[ATOUT] )
    {
        BestScore = ValCouleur[ATOUT];
        BestCouleur = ATOUT;
    }
	//  Dernière "Couleur" joue l'excuse. C'est très rarement un bon choix pour le preneur !
	//  Seule possibilité, quand il reste 2 cartes pour ne pas la mener au bout
	//  TODO TODO : cas de chelem à regarder
	if ( !HasCarte(pJeu, Position, 0) )
		ValCouleur[EXCUSE] = -100000;
	else
		ValCouleur[EXCUSE] = K31 + K32*(pJeu->NbCarte==2);
	CarteCouleur[EXCUSE] = 0;
    if ( BestScore < ValCouleur[EXCUSE] )
    {
        BestScore = ValCouleur[EXCUSE];
        BestCouleur = EXCUSE;
        CarteCouleur[EXCUSE] = 0;       //  Toujours index 0
    }
	PoseCarte(CurrentGame, CarteCouleur[BestCouleur]);
}

#if DEBUG > 0
#define DEBUG_ATTAQUE_SECOND 0
#else
#define DEBUG_DEFENSE_SECOND 0
#endif  // DEBUG

void JoueAttaqueSecond(TarotGame CurrentGame)
{
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[CurrentGame->JoueurCourant];
//int pos = (pJeu->PositionPreneur - pJeu->PositionJoueur) & 3;
int i0;
int BestCarte = -1;
double BestScore = -1e6;
double Score;

	assert(pJeu->NbCarte > 0);
    CalcTmpProbCarte(CurrentGame);      //  Apprend de la carte jouée auparavant
	pJeu->ResteAtout = NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT);
	GainPetitAuBout(CurrentGame, pJeu);
#if DEBUG_ATTAQUE_SECOND > 0
    OutDebug("--\nPosition %d, JoueAttaqueSecond après ", pJeu->PositionJoueur);
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
    OutDebug("Status Chasse = %d\n", pJeu->StatusChasse);
    OutDebug("Flanc = %d, ValFlanc = %.3f, CouleurTenue = %d\n", pJeu->Flanc, pJeu->Flanc > 0 ? pJeu->ValFlanc[pJeu->Flanc]:0.0, pJeu->CouleurTenue);
    OutDebug("ResteAtout = %d\n", pJeu->ResteAtout);
    OutDebug("Valeur Petit au Bout=%.3f\n", pJeu->ValPetitAuBout);
    OutDebug("ProbGain = %.3f\n", ProbGainCoup(CurrentGame, pJeu, 1, 0));
    if ( Table[0].Couleur == COEUR && Table[0].Hauteur == VALET )
        i0 = 0;
#endif // DEBUG_DEFENSE_SECOND
    MakeCarte(&Table[2], 0);             //  Simule excuse après pour être sûr de ne pas faire le pli.
    MakeCarte(&Table[3], 0);             //  Simule excuse après pour être sûr de ne pas faire le pli.
	for ( i0 = 0; i0 < pJeu->NbCarte; i0++)
	{
		if ( CarteValide(pJeu, i0, CurrentGame->JoueurEntame) )		//	Peut jouer cette carte ?
		{
			Table[1] = pJeu->MyCarte[i0];
			Score = EvalCoupSecond(CurrentGame, pJeu);
#if DEBUG_ATTAQUE_SECOND > 0
            OutDebug("Essai carte ");
            ImprimeCarte(&Table[1]);
            OutDebug(" Score = %.2f\n", Score);
#endif // DEBUG_ATTAQUE_SECOND
			if ( Score > BestScore )
			{
				BestScore = Score;
				BestCarte = i0;
			}
		}
	}
	//  "Efface" les cartes jouées virtuellement
	Table[2].Index = -1;
	Table[3].Index = -1;
    if ( BestCarte >= 0 )
	{
		PoseCarte(CurrentGame, BestCarte);
		return;
	}
	assert(0);
}

#if DEBUG > 0
#define DEBUG_ATTAQUE_TROISIEME 0
#else
#define DEBUG_ATTAQUE_TROISIEME 0
#endif  // DEBUG


void JoueAttaqueTroisieme(TarotGame CurrentGame)
{
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[CurrentGame->JoueurCourant];
//int pos = (pJeu->PositionPreneur - pJeu->PositionJoueur) & 3;
int i0;
int BestCarte = -1;
double BestScore = -1e6;
double Score;

	assert(pJeu->NbCarte > 0);
    CalcTmpProbCarte(CurrentGame);      //  Apprend des cartes jouées auparavant
	pJeu->ResteAtout = NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT);
	GainPetitAuBout(CurrentGame, pJeu);
#if DEBUG_ATTAQUE_TROISIEME > 0
    OutDebug("--\nPosition %d, JoueAttaqueTroisieme après ", pJeu->PositionJoueur);
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
    OutDebug("Status Chasse = %d\n", pJeu->StatusChasse);
    OutDebug("ResteAtout = %d\n", pJeu->ResteAtout);
    OutDebug("Valeur Petit au Bout=%.3f\n", pJeu->ValPetitAuBout);
    OutDebug("Flanc = %d, ValFlanc = %.3f, CouleurTenue = %d\n", pJeu->Flanc, pJeu->Flanc > 0 ? pJeu->ValFlanc[pJeu->Flanc]:0.0, pJeu->CouleurTenue);
    OutDebug("ProbGain = %.3f\n", ProbGainCoup(CurrentGame, pJeu, 2, 0));
#endif // DEBUG_ATTAQUE_TROISIEME
    MakeCarte(&Table[3], 0);             //  Simule excuse après pour être sûr de ne pas faire le pli.
	for ( i0 = 0; i0 < pJeu->NbCarte; i0++)
	{
		if ( CarteValide(pJeu, i0, CurrentGame->JoueurEntame) )		//	Peut jouer cette carte ?
		{
			Table[2] = pJeu->MyCarte[i0];
			Score = EvalCoupTroisieme(CurrentGame, pJeu);
#if DEBUG_ATTAQUE_TROISIEME > 0
            OutDebug("Essai carte ");
            ImprimeCarte(&Table[2]);
            OutDebug(" Score = %.2f\n", Score);
#endif // DEBUG_ATTAQUE_TROISIEME
			if ( Score > BestScore )
			{
				BestScore = Score;
				BestCarte = i0;
			}
		}
	}
	//  "Efface" les cartes jouées virtuellement
	Table[3].Index = -1;
    if ( BestCarte >= 0 )
	{
		PoseCarte(CurrentGame, BestCarte);
		return;
	}
	assert(0);
}

#if DEBUG > 0
#define DEBUG_ATTAQUE_DERNIER 0
#else
#define DEBUG_ATTAQUE_DERNIER 0
#endif  // DEBUG

void JoueAttaqueDernier(TarotGame CurrentGame)
{
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[CurrentGame->JoueurCourant];
//int pos = (pJeu->PositionPreneur - pJeu->PositionJoueur) & 3;
int i0;

int BestCarte = -1;
double BestScore = -1e6;
double Score;
int iGagnant;

	assert(pJeu->NbCarte > 0);
    CalcTmpProbCarte(CurrentGame);      //  Apprend des cartes jouées auparavant
	pJeu->ResteAtout = NbReste(CurrentGame, pJeu->PositionJoueur, ATOUT);
	GainPetitAuBout(CurrentGame, pJeu);
#if DEBUG_ATTAQUE_DERNIER > 0
    OutDebug("--\nPosition %d, JoueAttaqueDernier après ", pJeu->PositionJoueur);
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
    OutDebug("Status Chasse = %d\n", pJeu->StatusChasse);
    OutDebug("ResteAtout = %d\n", pJeu->ResteAtout);
    OutDebug("Valeur Petit au Bout=%.3f\n", pJeu->ValPetitAuBout);
    OutDebug("Flanc = %d, ValFlanc = %.3f, CouleurTenue = %d\n", pJeu->Flanc, pJeu->Flanc > 0 ? pJeu->ValFlanc[pJeu->Flanc]:0.0, pJeu->CouleurTenue);
    OutDebug("ProbGain = %.3f\n", ProbGainCoup(CurrentGame, pJeu, 3, 0));
#endif // DEBUG_ATTAQUE_DERNIER
	for ( i0 = 0; i0 < pJeu->NbCarte; i0++)
	{
		if ( CarteValide(pJeu, i0, CurrentGame->JoueurEntame) )		//	Peut jouer cette carte ?
		{
			Table[3] = pJeu->MyCarte[i0];
			Score = EvalCoup(CurrentGame, pJeu, &iGagnant);
#if DEBUG_ATTAQUE_DERNIER > 0
            OutDebug("Essai carte ");
            ImprimeCarte(&Table[3]);
            OutDebug(" Score = %.2f\n", Score);
#endif // DEBUG_ATTAQUE_TROISIEME
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
	assert(0);
}

//  Juste avant le premier pli, regarde possibilité de faire un Chelem
//  Si oui, déclare ce chelem

int RegardeChelemPreneur(TarotGame CurrentGame)
{
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[CurrentGame->JoueurPreneur];
double ProbChelem;
static char strMessage[256];

    if ( pJeu->NbPointDH > 80 )
    {
        ProbChelem = CalcProbaChelem(CurrentGame);
        if ( ProbChelem > 0.85 && CurrentGame->JoueurPreneur != SUD)
        {
            snprintf(strMessage, 60, "%s demande un chelem !", NomJoueurs[CurrentGame->JoueurPreneur]);
            CurrentGame->InfoMessage = strMessage;
            CurrentGame->StateJeu = JEU_DECLARE_CHELEM;
            return 1;
        }
        else if ( ProbChelem > 0.25 && CurrentGame->JoueurPreneur == SUD )
        {
            CurrentGame->StateJeu = JEU_POSE_QUESTION_CHELEM;
            CurrentGame->InfoMessage = "Voulez-vous demander un Chelem ?";
            return 1;
        }
    }
    return 0;
}

