#include <stdlib.h>
#include <gtk/gtk.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include "Tarot_Ui_Objects.h"

int FlagSignalisation = 1;

//  Efface le contrat pour ceux qui n'ont pas pris

void EffaceContratsJoueurs(TarotGame CurrentGame)
{
int i;

    if ( CurrentGame->TypePartie > PASSE  )
    {
        for (i = 0; i < MAX_JOUEURS; i++ )
        {
            if ( CurrentGame->JoueurPreneur != i)
                CurrentGame->AnnonceJoueur[i] = -1;     //  Efface annonce après choix final
        }
    }

}

//  Retourne 1 si le joueur Joueur a déjà joué la couleur Couleur, et sinon 0

int HasJoueCouleur(TarotGame CurrentGame, int Joueur, int Couleur )
{
int i;

    for ( i = Startof[Couleur]; i < Endof[Couleur]; i++ )
    {
        if ( CurrentGame->CarteJouee[i] == Joueur ) return 1;
    }
    return 0;
}

//	Retourne 1 si la carte passée en paramètre est maître

int IsMaitre(TarotGame CurrentGame, struct _Jeu *pJeu, struct _Carte_Jeu *pCarte)
{
int i;
int c = pCarte->Couleur;

	if ( c == EXCUSE ) return(0);             //  Excuse jamais maîtresse
	//  Pour les autres,
	for ( i = pCarte->Index +1 ; i < Endof[c]; i++)
	{
	    if ( CurrentGame->Carte2Joueur[i] == pJeu->PositionJoueur ) continue;       //  Le joueur a cette carte, ne compte pas et passe au suivant
	    if ( CurrentGame->CarteJouee[i] < 0 ) return 0;     //  Pas encore jouée, n'est donc pas maître
	}
	return(1);
}

//  Compte le nombre de cartes maîtresses du joueur dont la struct _Jeu est passée en paramètre dans la couleur Couleur

int NbMaitreCouleur(TarotGame CurrentGame, struct _Jeu *pJeu, int Color)
{
int i;
int nb = 0;

	for ( i = pJeu->NbCarte-1; i >=0 ; i--)
	{
		if ( pJeu->MyCarte[i].Couleur != Color ) continue;
		//  on est dans la bonne couleur
		if ( IsMaitre(CurrentGame, pJeu, &pJeu->MyCarte[i]) ) nb++;
	}
	return nb;
}

//	Retourne 1 si le joueur dont la struct _Jeu est passée en paramètre est maître dans la couleur passée en paramètre

int IsMaitreCouleur(TarotGame CurrentGame, struct _Jeu *pJeu, int Color)
{
int i;

	for ( i = pJeu->NbCarte-1; i >=0 ; i--)
	{
		if ( pJeu->MyCarte[i].Couleur != Color ) continue;
		//  Trouvé la première carte de la bonne couleur, plus forte du joueur
		return(IsMaitre(CurrentGame, pJeu, &pJeu->MyCarte[i]));
	}
	return(0);
}

//  Retourne un pointeur la carte la plus faible pour le joueur dans la couleur c
//  Attention ne doit pas être appelé si pas de carte dans la couleur

struct _Carte_Jeu *GetCartePlusFaible(struct _Jeu *pJeu, int c)
{
int i;

	for ( i = 0; i < pJeu->NbCarte; i++)
	{
		if ( pJeu->MyCarte[i].Couleur == c ) return(&pJeu->MyCarte[i]);
	}
	assert(0);
	return(NULL);
}

//  Retourne l'index (dans les cartes du joueur) de la carte la plus faible pour le joueur dans la couleur c
//  Attention ne doit pas être appelé si pas de carte dans la couleur

int GetPlusFaible(struct _Jeu *pJeu, int c)
{
int i;

	for ( i = 0; i < pJeu->NbCarte; i++)
	{
		if ( pJeu->MyCarte[i].Couleur == c ) return(i);
	}
	assert(0);
	return(-1);
}

//  Retourne l'index (dans les cartes du joueur) de la carte la plus faible pour le joueur dans la couleur c
//  Attention ne doit pas être appelé si pas de carte dans la couleur

int GetPlusForte(struct _Jeu *pJeu, int c)
{
int i;

	for ( i = pJeu->NbCarte-1; i >= 0; i--)
	{
		if ( pJeu->MyCarte[i].Couleur == c ) return(i);
	}
	assert(0);
	return(-1);
}

//  Retourne l'index (dans les cartes du joueur) de la carte la plus faible pour le joueur dans la couleur c
//  Attention ne doit pas être appelé si pas de carte dans la couleur

struct _Carte_Jeu *GetCartePlusForte(struct _Jeu *pJeu, int c)
{
int i;

	for ( i = pJeu->NbCarte-1; i >= 0; i--)
	{
		if ( pJeu->MyCarte[i].Couleur == c ) return(&pJeu->MyCarte[i]);
	}
	assert(0);
	return(NULL);
}

//	Retourne le nombre d'atouts maîtres du joueur dont la Position est passée en paramètre
//  Il ne s'agit pas de probas, ce sont les cartes de son jeu

int NbAtoutMaitre(TarotGame CurrentGame, int Position)
{
int nb = 0;
int i;

	for ( i = 21; i > 0; i--)
	{
		if ( CurrentGame->CarteJouee[i] >= 0 ) continue;        //  Déjà jouée, passe au suivant
		if ( CurrentGame->Carte2Joueur[i] == Position )
            nb++;           //  Une de plus
		else
			break;          //  Fin de la série
	}
	return(nb);
}


//	Retourne Vrai si la signalisation peut être OK quand le joueur avec structure pJeu joue la couleur Couleur
//  Teste
//      ATOUT impair pour signaler plus de 7 atouts
//      Honneur (Roi/Dame) signalé par carte du 1 au 5

int IsSigNOK(TarotGame CurrentGame, struct _Jeu *pJeu, int Couleur)
{
int Position = pJeu->PositionJoueur;

	if ( Couleur == ATOUT )
	{
	    //  Atout vérifie si plus de 7 atout IMPAIR
		if ( pJeu->NbAtout < 7 )
		{
			//	Recherche ATOUT pair
			for ( int i = 2; i <= 20; i += 2 )
			{
				if ( CurrentGame->Carte2Joueur[i] == Position ) return(0);  //  Oui, retourne 0
			}
			return(1);                                                      //  Pas d'atout pair, pb sig
		}
		//	Recherche impair comme le joueur a plus de 7 atouts
		for ( int i = 1; i <= 21; i += 2 )
		{
            if ( CurrentGame->Carte2Joueur[i] == Position ) return(0);  //  Oui, retourne 0
		}
		return(1);
	}
	//  Maintenant couleur : Si possède ROI ou Dame doit avoir du 1 au 5
	if ( GetCartePlusForte(pJeu, Couleur)->Hauteur >= DAME )
    {
        if ( GetCartePlusFaible(pJeu, Couleur)->Hauteur > 5 ) return(1);
    }
	else
	{
        //  N'a ni Roi ni Dame, doit posséder du 6 au 10
		if ( CurrentGame->Carte2Joueur[Startof[Couleur]+5] == Position ) return(0);     //  Possède le 6 : OK
		if ( CurrentGame->Carte2Joueur[Startof[Couleur]+6] == Position ) return(0);     //  Possède le 7 : OK
		if ( CurrentGame->Carte2Joueur[Startof[Couleur]+7] == Position ) return(0);     //  Possède le 8 : OK
		if ( CurrentGame->Carte2Joueur[Startof[Couleur]+8] == Position ) return(0);     //  Possède le 9 : OK
		if ( CurrentGame->Carte2Joueur[Startof[Couleur]+9] == Position ) return(0);     //  Possède le 10 : OK
		return(1);
	}
	return(0);
}

//	Retourne le nombre de cartes non Honneur (1 au 10) du joueur Position dans la couleur Couleur

int NbNonHonneur(TarotGame CurrentGame, int Position, int Couleur)
{
int nb = 0;
int i;

	for ( i = Startof[Couleur]; i < Startof[Couleur]+10; i++)
	{
	    if ( CurrentGame->CarteJouee[i] >= 0 ) continue;        //  Déjà jouée passe
		if ( CurrentGame->Carte2Joueur[i] == Position )         //  Oui, appartient au joueur
			nb++;
	}
	return(nb);
}

//	Retourne le nombre de cartes non jouées dans la couleur Couleur en possession des autres joueurs que le joueur Position

int NbReste(TarotGame CurrentGame, int Position, int Couleur)
{
int nb = 0;
int i;

	for ( i = Startof[Couleur]; i < Endof[Couleur]; i++)
	{
	    if ( CurrentGame->CarteJouee[i] >= 0 ) continue;        //  Déjà jouée, passe à la suivante
	    if ( CurrentGame->Carte2Joueur[i] != Position )
            nb++;
	}
	return(nb);
}

//	Compte les points du joueur dont la struct _Jeu est passée en paramètre

int ComptePoints(struct _Jeu *pJeu)
{
int compte = 0;
int i;

	for ( i = pJeu->NbAtout; i < pJeu->NbCarte; i++)        //  Commence après les atouts
	{
		if ( pJeu->MyCarte[i].Valeur > 1 ) compte += pJeu->MyCarte[i].Valeur ;
	}
	return(compte);
}


//  Lève les cartes possibles pour le pli à ce stade pour SUD

void LeveCartesPossiblesSUD(TarotGame CurrentGame)
{
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[SUD];
int i;

    DeltaCarteLevee = Card_Height/6;
    for ( i = 0; i < pJeu->NbCarte; i++ )
    {
        if ( CarteValide(pJeu, i, CurrentGame->JoueurEntame) )
        {
            CurrentGame->isCarteLevee[i] = 1;       //  Lève la carte
        }
    }
}

void BaisseCartesSUD(TarotGame CurrentGame)
{
int i;

    DeltaCarteLevee = 0;
    for ( i = 0; i < MAX_CARTE_JOUEUR; i++ )
    {
        CurrentGame->isCarteLevee[i] = 0;       //  Toutes les cartes baissées
    }
    CurrentGame->NbCarteLevee = 0;
}

//  Retourne 1 si la carte 2 est plus forte que la carte 1 dans un pli.
//  L'ordre est celui du jeu
int isSup(struct _Carte_Jeu *pCarte1, struct _Carte_Jeu *pCarte2)
{

	if ( pCarte1->Couleur == EXCUSE ) return(1);        //  L'excuse ne fait jamais le pli
	if ( pCarte2->Couleur == EXCUSE ) return(0);
	if ( pCarte2->Couleur == ATOUT && pCarte1->Couleur != ATOUT ) return(1);
	if ( pCarte2->Couleur == ATOUT && pCarte2->Hauteur > pCarte1->Hauteur ) return(1);
	if ( pCarte1->Couleur == ATOUT ) return(0);
	if ( pCarte2->Couleur != pCarte1->Couleur ) return(0);
	if ( pCarte2->Hauteur > pCarte1->Hauteur ) return(1);
	return(0);
}

//	Retourne la longueur moyenne de la couleur du joueur
//	Cette longueur est la somme des probabilités de possession des cartes de la couleur.

double AvgLongueur(struct _Jeu *pJeu, int Joueur, int Couleur)
{
double l = 0;

	for ( int i = Startof[Couleur]; i < Endof[Couleur]; i++)
	{
		if ( pJeu->TmpProbCarte[Joueur][i] >= 0) l += pJeu->TmpProbCarte[Joueur][i];
	}
	return(l);

}

//  Retourne le nombre de points à faire en fonction des bouts dans les plis du preneur
int NombrePointsContrat(TarotGame CurrentGame)
{
    switch ( CurrentGame->NumBoutsPreneur )
    {
    case 0:
        return 56;
    case 1:
        return 51;
    case 2:
        return 41;
    case 3:
        return 36;
    default:
        assert(0);
    }
    return 0;
}

//	Retourne le nombre de couleurs jouées par le joueur Joueur

int CompteCouleurJoueur(struct _Jeu *pJeu, int Joueur)
{
int nb = 0;
int c;

	for (c = TREFLE; c < NB_COULEUR; c++)
	{
		if ( pJeu->NbJoue[Joueur][c] > 0 ) nb++;
	}
	return(nb);
}


//	Retourne la probabilité de coupe de la défense avant le preneur dans la couleur c
//	Pour cela calcule la proba que chaque joueur coupe la couleur
//	ProbCoupeAvantPreneur = 1.0 - Prod[ Proba_ne_coupe_PAS(joueur) ]

double ProbCoupeAvantPreneur(struct _Jeu *pJeu, int c)
{
int Pos = pJeu->PositionJoueur;
double p0, p1;
int j;

	p0 = 1.0;
	for ( j = (Pos + 1) & 3; j != Pos; j = (j+1)&3 )
	{
		if ( j == pJeu->PositionPreneur) break;
		p1 = ProbCoupeJoueur(pJeu, j, c);		//	Proba coupe par le joueur j
		p0 *= 1.0 - p1;
	}
	assert(j != Pos);
	return(1- p0);
}

//	Retourne la probabilité de coupe de la défense après le preneur
//	Pour cela calcule la proba que chaque joueur coupe la couleur
//	ProbCoupeApresPreneur = 1.0 - Prod[ Proba_ne_coupe_PAS(joueur) ]

double ProbCoupeApresPreneur(struct _Jeu *pJeu, int c)
{
double p0, p1;
int j;
int Pos = pJeu->PositionJoueur;

	p0 = 1.0;
	for ( j = (pJeu->PositionPreneur + 1) & 3; j != Pos; j = (j+1)&3 )
	{
		p1 = ProbCoupeJoueur(pJeu, j, c);		//	Proba coupe par le joueur j
		p0 *= 1.0 - p1;
	}
	return(1.0 - p0);
}
//	Retourne la probabilité que le joueur Joueur coupe la couleur Couleur

double ProbCoupeJoueur(struct _Jeu *pJeu, int Joueur, int Couleur)
{
double p = 1.0;
int i;

	for ( i = Startof[Couleur]; i < Endof[Couleur]; i++)
	{
		p *= 1.0 - pJeu->TmpProbCarte[Joueur][i];
	}
	return(p);
}

//	Retourne l'index de la carte maîtresse dans la couleur Couleur
//	Cherche pour cela dans tous les jeux.
//  Bloque si appelé alors qu'il ne reste plus de cartes dans la couleur

int CarteMaitreNonJouee(TarotGame CurrentGame, int Couleur)
{
int i;

	for ( i = Endof[Couleur]-1; i >= Startof[Couleur]; i--)
	{
	    if ( CurrentGame->CarteJouee[i] < 0 ) return(i);
	}
	assert(0);
	return(-1);
}

//	Retourne l'index de la carte maîtresse dans la couleur Couleur
//	Cherche pour cela dans tous les jeux.
//  Retourne -1 si plus de carte dans cette couleur

int CarteMaitreNonJoueeNoErr(TarotGame CurrentGame, int Couleur)
{
int i;

	for ( i = Endof[Couleur]-1; i >= Startof[Couleur]; i--)
	{
	    if ( CurrentGame->CarteJouee[i] < 0 ) return(i);
	}
	return(-1);
}

//	Retourne la taille minimum de la couleur Couleur du joueur Joueur

int MinLongueur(struct _Jeu *pJeu, int Joueur, int Couleur)
{
int lg = 0;
int i;

	for (  i = Startof[Couleur]; i < Endof[Couleur]; i++)
	{
		if ( pJeu->TmpProbCarte[Joueur][i] >= 0.999 ) lg++;
	}
	return(lg);
}


//	Retourne la taille maximum de la Couleur du joueur Joueur

int MaxLongueur(struct _Jeu *pJeu, int Joueur, int Couleur)
{
int lg = 0;
int i;

	for ( i = Startof[Couleur]; i < Endof[Couleur]; i++)
	{
		if ( pJeu->TmpProbCarte[Joueur][i] >= 0.000001 ) lg++;
	}
	return(lg);
}


//	Retourne une force du preneur dans une couleur
//	La formule est Force = SOMME(Proba(carte i)*Proba(Cartei+1 au chien))

double ForcePreneurCouleur(TarotGame CurrentGame, struct _Jeu *pJeu, int Couleur)
{
double p = 0.0;             //  Proba
double pc = 1.0;
int i;
int Preneur = pJeu->PositionPreneur;

	for ( i = Endof[Couleur] - 1; i >= Startof[Couleur]; i--)
	{
		if ( IsJoue(CurrentGame, i) ) continue;
		p += pJeu->TmpProbCarte[Preneur][i]*pc;
		pc *= pJeu->TmpProbCarte[CHIEN][i];
	}
	return(p);
}


//	Retourne le nombre de bouts de la défense.
//  Ne compte que ceux dans les cartes au départ

int NbBoutDefense(TarotGame CurrentGame, int Position)
{
int nb = 0;
int Preneur = CurrentGame->JoueurPreneur;

    //	Compte les bouts joués de la défense et ses propres bouts non joués
    if ( CurrentGame->CarteJouee[0] >= 0 )
    {
        if ( CurrentGame->CarteJouee[0] != Preneur ) nb++;       //  Excuse jouée par la défense
    }
    else
    {
        if ( CurrentGame->Carte2Joueur[0] == Position )         //  Excuse dans son jeu
            nb++;
    }
    if ( CurrentGame->CarteJouee[1] >= 0 )
    {
        if ( CurrentGame->CarteJouee[1] != Preneur ) nb++;       //  Petit joué par la défense
    }
    else
    {
        if ( CurrentGame->Carte2Joueur[1] == Position )         // Petit dans son jeu
            nb++;
    }
    if ( CurrentGame->CarteJouee[21] >= 0 )
    {
        if ( CurrentGame->CarteJouee[21] != Preneur ) nb++;       //  21 joué par la défense
    }
    else
    {
        if ( CurrentGame->Carte2Joueur[21] == Position )         //  21 dans son jeu
            nb++;
    }
	return(nb);
}

//	Retourne le nombre de couleurs ouvertes

int NbCouleurOuverte(struct _Jeu *pJeu)
{
int nb = 0;

	for (int c = TREFLE; c < NB_COULEUR; c++)
	{
		if ( pJeu->JoueurCouleur[c] >= 0 ) nb++;
	}
	return(nb);
}


//  Gestion du fichier DEBUG

static FILE *DebugFile = NULL;

void OpenDebugFile(const char *Name)
{
#if DEBUG > 0
    DebugFile = fopen(Name, "w");
#endif // DEBUG
}

void OutDebug(char *fmt, ...)
{
#if DEBUG > 0
va_list ap;
    if ( DebugFile == NULL ) return;
    va_start(ap, fmt);
    vfprintf(DebugFile, fmt, ap);
    fflush(DebugFile);
    va_end(ap);
#endif // DEBUG
}

void CloseDebugFile()
{
#if DEBUG > 0
    if ( DebugFile!= NULL )
    {
        fclose(DebugFile);
        DebugFile = NULL;
    }
#endif // DEBUG
}

//  Imprime le nom de la carte dans le fichier DEBUG

void ImprimeCarte(struct _Carte_Jeu *Carte)
{
    if ( DebugFile == NULL ) return;
	switch ( Carte->Couleur )
	{
	case EXCUSE:
	    OutDebug("Exc ");
		return;
	case ATOUT:
		OutDebug("%dA ", Carte->Hauteur);
		return;
	case TREFLE:
		if ( Carte->Hauteur <= 10 )
			OutDebug("%dT ", Carte->Hauteur);
		if ( Carte->Hauteur == VALET )
			OutDebug( "VT ", Carte->Hauteur);
		if ( Carte->Hauteur == CAVALIER )
			OutDebug( "CT ", Carte->Hauteur);
		if ( Carte->Hauteur == DAME )
			OutDebug( "DT ", Carte->Hauteur);
		if ( Carte->Hauteur == ROI )
			OutDebug( "RT ", Carte->Hauteur);
		return;
	case CARREAU:
		if ( Carte->Hauteur <= 10 )
			OutDebug( "%dK ", Carte->Hauteur);
		if ( Carte->Hauteur == VALET )
			OutDebug( "VK ", Carte->Hauteur);
		if ( Carte->Hauteur == CAVALIER )
			OutDebug( "CK ", Carte->Hauteur);
		if ( Carte->Hauteur == DAME )
			OutDebug( "DK ", Carte->Hauteur);
		if ( Carte->Hauteur == ROI )
			OutDebug( "RK ", Carte->Hauteur);
		return;
	case COEUR:
		if ( Carte->Hauteur <= 10 )
			OutDebug("%dC ", Carte->Hauteur);
		if ( Carte->Hauteur == VALET )
			OutDebug("VC ", Carte->Hauteur);
		if ( Carte->Hauteur == CAVALIER )
			OutDebug( "CC ", Carte->Hauteur);
		if ( Carte->Hauteur == DAME )
			OutDebug( "DC ", Carte->Hauteur);
		if ( Carte->Hauteur == ROI )
			OutDebug("RC ", Carte->Hauteur);
		return;
	case PIQUE:
		if ( Carte->Hauteur <= 10 )
			OutDebug("%dP ", Carte->Hauteur);
		if ( Carte->Hauteur == VALET )
			OutDebug( "VP ", Carte->Hauteur);
		if ( Carte->Hauteur == CAVALIER )
			OutDebug( "CP ", Carte->Hauteur);
		if ( Carte->Hauteur == DAME )
			OutDebug( "DP ", Carte->Hauteur);
		if ( Carte->Hauteur == ROI )
			OutDebug( "RP ", Carte->Hauteur);
		return;
	}
}


//	"fabrique" une carte a partir de son index

void MakeCarte(struct _Carte_Jeu *Carte, int Index)
{
	Carte->Index = Index;
	if ( Index == 0 )
	{
		Carte->Couleur = EXCUSE;
		Carte->Hauteur = 0;
		Carte->Valeur = 10;
		return;
	}
	if ( Index < 22 )
	{
		Carte->Couleur = ATOUT;
		Carte->Hauteur = Index;
		if ( Index == 1 || Index == 21 )
			Carte->Valeur = 9;
		else
			Carte->Valeur = 1;
		return;
	}
	if ( Index < 36 )
	{
		Carte->Couleur = TREFLE;
		Carte->Hauteur = Index - 21;
		if ( Carte->Hauteur > 10 )
			Carte->Valeur = (Carte->Hauteur - 10)*2 + 1;
		else
			Carte->Valeur = 1;
		return;
	}
	if ( Index < 50 )
	{
		Carte->Couleur = CARREAU;
		Carte->Hauteur = Index - 35;
		if ( Carte->Hauteur > 10 )
			Carte->Valeur = (Carte->Hauteur - 10)*2 + 1;
		else
			Carte->Valeur = 1;
		return;
	}
	if ( Index < 64 )
	{
		Carte->Couleur = PIQUE;
		Carte->Hauteur = Index - 49;
		if ( Carte->Hauteur > 10 )
			Carte->Valeur = (Carte->Hauteur - 10)*2 + 1;
		else
			Carte->Valeur = 1;
		return;
	}
	Carte->Couleur = COEUR;
	Carte->Hauteur = Index - 63;
	if ( Carte->Hauteur > 10 )
		Carte->Valeur = (Carte->Hauteur - 10)*2 + 1;
	else
		Carte->Valeur = 1;
}


//	Retourne l'index dans le jeu du joueur de la carte demandée

int Carte2Index(struct _Jeu *pJeu, int Couleur, int Hauteur)
{
	for ( int i = 0; i < pJeu->NbCarte; i++)
	{
		if ( pJeu->MyCarte[i].Couleur == Couleur && pJeu->MyCarte[i].Hauteur == Hauteur ) return(i);
	}
	assert(0);
	return(-1);
}

//	Compte le nombre de cartes non jouées de la couleur c plus grandes que h
//  Tient compte également des cartes sur la table de jeu

int ComptePlusGrand(TarotGame CurrentGame, int c, int h)
{
int nb = 0;
int i;

	for ( i = Startof[c] + h; i < Endof[c]; i++)
	{
		if ( CurrentGame->CarteJouee[i] >= 0 ) continue;
		if ( Table[0].Index == i ) continue;
		if ( Table[1].Index == i ) continue;
		if ( Table[2].Index == i ) continue;
		if ( Table[3].Index == i ) continue;
		nb++;
	}
	return(nb);
}


//	Retourne les points restants du joueur Joueur dans la couleur Couleur
//  Tient compte des cartes posées sur la table de jeu

double PointsRestants(struct _Jeu *pJeu, int Joueur, int Couleur)
{
double val = 0;
int i;

	for ( i = Startof[Couleur]+10; i < Endof[Couleur]; i++)
	{
		if ( Table[0].Index == i ) continue;
		if ( Table[1].Index == i ) continue;
		if ( Table[2].Index == i ) continue;
		if ( Table[3].Index == i ) continue;
		if ( pJeu->TmpProbCarte[Joueur][i] > 1e-7 )
			val += pJeu->TmpProbCarte[Joueur][i] * (2.0 * ( i - Startof[Couleur] - 9 ) + 1.0);
	}
	return(val);
}


//	Retourne le nombre de points du plus petit honneur du joueur dans la couleur

double GetPointMinHonneur(struct _Jeu *pJeu, int c)
{
int i;

	for ( i = Startof[c]+10; i < Endof[c]; i++)
	{
		if ( HasCarte(pJeu, pJeu->PositionJoueur, i) )
			return(2.0*(i - Startof[c] - 9));
	}
	return(0.0);
}

//  Retourne Vrai si le joueur avec la structure pJeu possède le petit

int HasPetit(struct _Jeu *pJeu)
{
    return pJeu->ProbCarte[pJeu->PositionJoueur][1] > 0.99;
}

//  Retourne vrai si le joueur avec la structure pJeu possède l'excuse

int HasExcuse(struct _Jeu *pJeu)
{
    return pJeu->ProbCarte[pJeu->PositionJoueur][0] > 0.99;
}


//	Retourne le nombre de plis possibles dans une couleur c par le joueur Joueur en défense
//	Tient compte des cartes déjà jouées et de la longueur possible du preneur
//  Mise à jour de la variable TousUtiles si toutes les cartes peuvent faire des plis

double PlisPossibles(TarotGame CurrentGame, struct _Jeu *pJeu, int Couleur,  double *TousUtiles)
{
int i;
double pp = 0;
double AvgLgP = 0;
double nb = pJeu->NbCouleur[Couleur];
int LastUtile = 0;
double val = 1.0;

    //	Calcule tout d'abord AvgLgP = le nombre de cartes du preneur dans la couleur
    //  Utilise une proba modifiée pour augmenter la longueur pour les probabilités faibles
	for ( i = Startof[Couleur]; i < Endof[Couleur]; i++)
	{
        AvgLgP += sqrt(pJeu->TmpProbCarte[pJeu->PositionPreneur][i]);		//	Augmente longueur du preneur pour cette fonction
	}
    //	Pour chaque carte, en partant du ROI, regarde si le joueur la possède
    //	Donne les plis possibles si possède La xieme carte dans la couleur protégée
	for ( i = Startof[Couleur] + 13; i >= Startof[Couleur]; i--)
	{
		if ( CurrentGame->CarteJouee[i] >= 0 ) continue;
		nb -= 1.0;				//	Une de passée
		if ( HasCarte(pJeu, pJeu->PositionJoueur, i ) )
		{
			if ( nb >= 0 && AvgLgP > 0)
			{
				pp += 1.0;			//	Un pli possible de plus
				if ( nb == 0 )		//	Dernière carte dans la couleur ?
				{
					LastUtile = 1;	//	oui, toutes sont utiles
					val = i - Startof[Couleur] - 9.0;	//	Valeur associée
					if ( val < 2.0 ) val = 1.0;
				}
			}
		}
		AvgLgP -= 1.0;
	}
	//	Correction pour la valeur du dernier.
	val = 1.0 - (4.0 - val)*(4.0 - val)/13.0;
	*TousUtiles = LastUtile * val;
	return(pp);
}

//	Compte les honneurs du joueur
//	Retourne le nombre de cartes et remplit le tableau par couleur

int CompteHonneur(TarotGame CurrentGame, int Joueur, int ResParCouleur[])
{
int i;
	for ( int c = 0; c < 4; c++)
	{
		ResParCouleur[c] = 0;
		for ( i = Startof[c] + 10; i < Endof[c]; i++)       //  Regarde les honneurs de chaque couleur
		{
			if ( CurrentGame->Carte2Joueur[i] == Joueur ) ResParCouleur[c]++;
		}
	}
	return(ResParCouleur[0]+ResParCouleur[1]+ResParCouleur[2]+ResParCouleur[3]);
}

//	Retourne la probabilité qu'un joueur soit maître dans une couleur
//	La formule est Prob = SOMME(Proba(carte i)*Proba(Cartei+1 au chien))

double ProbaMaitreCouleur(TarotGame CurrentGame, struct _Jeu *pJeu, int Joueur, int Couleur)
{
double p = 0.0;
double pc = 1.0;

	for ( int i = Endof[Couleur] - 1; i >= Startof[Couleur]; i--)
	{
		if ( CurrentGame->CarteJouee[i] >= 0 ) continue;    //  Déjà jouée ?
		p += pJeu->TmpProbCarte[Joueur][i]*pc;
		pc *= pJeu->TmpProbCarte[4][i];                     //  Si au chien, ne sera jamais maître...
	}
	return(p);
}

//  Regarde si petit au bout en fin de partie
//  Retourne 0 si pas petit au bout, 10 si petit au bout gagnant et -10 si petit perdu au bout

int PointsPetitAuBout(TarotGame CurrentGame)
{
int ExcuseAuBout = isCarteInPli(CurrentGame, 17, 0);
int nPli = 17;
int i;
int PetitAuBout;
int gagnant;

    //  Si Chelem et excuse au bout (preneur), le petit au bout est à l'avant dernier pli
    if ( ExcuseAuBout == CurrentGame->JoueurPreneur && CurrentGame->NumPlisPreneur == 18 )
    {
        nPli = 16;
    }
    PetitAuBout = isCarteInPli(CurrentGame, nPli, 1);
    if ( PetitAuBout < 0 ) return 0;            //  pas de petit au bout.
    //  Maintenant regarde qui a fait le pli
    for ( i = 0; i < 4; i++ )
    {
        MakeCarte(&Table[i], CurrentGame->MemPlis[nPli][i]);
    }
    gagnant = Gagnant(3);
    if ( PetitAuBout == CurrentGame->JoueurPreneur && gagnant == PetitAuBout ) return 10;
    if ( PetitAuBout == CurrentGame->JoueurPreneur ) return -10;
    //  Petit au bout à la défense
    if ( gagnant == CurrentGame->JoueurPreneur ) return -10;
    return 10;
}

//  Compte les points à la fin de la partie

void ComptePointsFinPartie(TarotGame CurrentGame)
{
int i;
int Multiplicateur;
int pp = 0;
int BasePoints = 0;

    switch ( CurrentGame->TypePartie )
    {
    case PETITE:
    case GARDE:
    case GARDE_SANS:
        CurrentGame->NumPointsPreneur = 91 - CurrentGame->NumPointsDefense;      //  Les points au chien sont pour le preneur
        CurrentGame->NumBoutsPreneur = 3 - CurrentGame->NumBoutsDefense;
        Multiplicateur = 1 << (CurrentGame->TypePartie - 1);
        break;
    case GARDE_CONTRE:
        CurrentGame->NumPointsDefense = 91 - CurrentGame->NumPointsPreneur;
        CurrentGame->NumBoutsDefense = 3 - CurrentGame->NumBoutsPreneur;
        Multiplicateur = 6;
        break;
    default:
        assert(0);
    }
    CurrentGame->PointsMinPreneur = NombrePointsContrat(CurrentGame);
    CurrentGame->PartieGagnee = CurrentGame->NumPointsPreneur >= CurrentGame->PointsMinPreneur;
    //  Petit au bout
    pp = PointsPetitAuBout(CurrentGame);
    if ( pp )
    {
        if ( CurrentGame->Carte2Joueur[1] == CurrentGame->JoueurPreneur )
            CurrentGame->PointPetitAuBoutAttaque = pp * Multiplicateur;
        else
            CurrentGame->PointPetitAuBoutDefense = pp * Multiplicateur;
    }
    //  Poignées...
    for ( i = 0; i < 4; i++ )
    {
        if ( i == CurrentGame->JoueurPreneur )
        {
            pp = 0;
            if ( CurrentGame->PoigneeMontree[i] == POIGNEE_SIMPLE )
                pp = 20;
            else if ( CurrentGame->PoigneeMontree[i] == POIGNEE_DOUBLE )
                pp = 30;
            else if ( CurrentGame->PoigneeMontree[i] == POIGNEE_TRIPLE )
                pp = 40;
            if ( CurrentGame->PartieGagnee )
                CurrentGame->PointsPoigneeAttaque += pp;
            else
                CurrentGame->PointsPoigneeAttaque -= pp;
        }
        else
        {
            pp = 0;
            if ( CurrentGame->NumAtoutPoignee[i] == 10 )
                pp = 20;
            else if ( CurrentGame->NumAtoutPoignee[i] == 13 )
                pp = 30;
            else if ( CurrentGame->NumAtoutPoignee[i] == 15 )
                pp = 40;
            if ( CurrentGame->PartieGagnee )
                CurrentGame->PointsPoigneeDefense -= pp;
            else
                CurrentGame->PointsPoigneeDefense += pp;
        }
    }
    //  Points du Chelem
    if ( CurrentGame->NumPlisPreneur == 18 )
    {
        if ( CurrentGame->ChelemDemande )
            CurrentGame->PointsChelemAttaque = 400;
        else
            CurrentGame->PointsChelemAttaque = 200;
    }
    else if ( CurrentGame->NumPlisPreneur == 0 )
    {
        CurrentGame->PointsChelemDefense = 200;
    }
    else if ( CurrentGame->ChelemDemande )
        CurrentGame->PointsChelemAttaque = -200;
    if ( CurrentGame->PartieGagnee )
        BasePoints = 25;
    else
        BasePoints = -25;
    BasePoints += CurrentGame->NumPointsPreneur - CurrentGame->PointsMinPreneur;
    BasePoints *= Multiplicateur;
    BasePoints += CurrentGame->PointPetitAuBoutAttaque;
    BasePoints -= CurrentGame->PointPetitAuBoutDefense;
    BasePoints += CurrentGame->PointsPoigneeAttaque;
    BasePoints -= CurrentGame->PointsPoigneeDefense;
    BasePoints += CurrentGame->PointsChelemAttaque;
    BasePoints -= CurrentGame->PointsChelemDefense;
    CurrentGame->PointsAttaque = 3 * BasePoints;
    CurrentGame->PointsDefense = -BasePoints;
    if ( NumDonnes < NOMBRE_DONNES_PARTIE )
    {
        ResPartie[NumDonnes].TypeEnregistre = CurrentGame->NumPartieEnregistree >= 0;
        ResPartie[NumDonnes].Preneur = CurrentGame->JoueurPreneur;
        if ( ResPartie[NumDonnes].TypeEnregistre )
        {
            ResPartie[NumDonnes].ScoreAttaque = (int) (ResultatPartieMaitre(CurrentGame)*100.0);
            ResPartie[NumDonnes].ScoreDefense = 100 - ResPartie[NumDonnes].ScoreAttaque;
        }
        else
        {
            ResPartie[NumDonnes].ScoreAttaque = CurrentGame->PointsAttaque;
            ResPartie[NumDonnes].ScoreDefense = CurrentGame->PointsDefense;
        }
    }
    NumDonnes++;
    SaveScores();
}

//  Calcule les scores à la fin de la partie
//  Retourne le score max
int CalcScoresPartie(double ScoreJoueurs[MAX_JOUEURS], int *JoueurGagnant)
{
int n, i;
int ScoreGagnant = -10000000;
int jg = -1;

    //  Calcul des scores
    for ( i = 0; i < MAX_JOUEURS; i++ )
        ScoreJoueurs[i] = 0;
    for ( n = 0; n < NumDonnes; n++ )
    {
        for ( i = 0; i < 4; i++ )
        {
            if ( i == ResPartie[n].Preneur )
                ScoreJoueurs[i] += ResPartie[n].ScoreAttaque;
            else
                ScoreJoueurs[i] += ResPartie[n].ScoreDefense;
        }
    }
    if ( ResPartie[0].TypeEnregistre && NumDonnes > 0 )
    {
        for ( i = 0; i < 4; i++ )
        {
            ScoreJoueurs[i] /= NumDonnes;       //  Divise par NumDonnes pour avoir un pourcentage
        }
    }
    //  Calcul le (ou les gagnants)
    for ( i = 0; i < 4; i++ )
    {
        if ( ScoreJoueurs[i] > ScoreGagnant )
        {
            ScoreGagnant = ScoreJoueurs[i];
            if ( jg < 0 )
                jg = i;
            else
                jg = MAX_JOUEURS+1;         //  Plusieurs gagants
        }
    }
    if ( JoueurGagnant != NULL )
        *JoueurGagnant = jg;
    return ScoreGagnant;
}


//	Retourne la somme des proba que les joueurs après Idx possèdent un atout plus élevé que h

double ProbRestantAuDessus(struct _Jeu *pJeu, int h, int pos, int Idx)
{
double p = 0;
int i, j;

	//	Commence à partir de h+1
	for ( i = h+1; i < 22; i++)
	{
		//	Pour les joueurs suivants
		for ( j = Idx+1; j < 4; j++)
		{
			if ( pJeu->TmpProbCarte[(pos+j)&3][i] > 0 )	p += pJeu->TmpProbCarte[(pos+j)&3][i];
		}
	}
	return(p);
}

//  Retourne l'atout non joué immédiatement supérieur à l'atout Index

int NextAtout(TarotGame CurrentGame, int Index)
{
int h;

    for ( h = Index + 1; h < 22; h++ )
    {
        if ( CurrentGame->CarteJouee[h] < 0 ) break;
    }
    if ( h == 22 )
        return 0;
    return h;
}

//  Retourne le plus gros atout non joué, 0 si ils ont tous été joués

int AtoutMaitre(TarotGame CurrentGame)
{
int h;

    for ( h = 21; h > 0; h-- )
    {
        if ( CurrentGame->CarteJouee[h] < 0 ) break;
    }
    return h;
}

//  Calcule n!

double factorielle(int n)
{
double res = 1.0;
int i;

    assert(n>=0);
    if ( n == 0 ) return 1;
    for ( i = n; i > 1; i--)
    {
        res *= i;
    }
    return res;
}

//  Calcule le nombre de combinaisons de p éléments parmi n
//  c_n_p = n!/(n-p!*p
//  retourne un double car peut facilement dépasser la capacité d'un entier 32 bit voire 64 bit

double C_n_p(int n, int p)
{
double res = 1.0;
int i;

    assert(n >= p);
    //  Calcule n*(n-1)*(n-2)*...(n-p+1)
    for (i = n-p+1; i <= n; i++)
    {
        res *= i;
    }
    //  Puis divise par p!
    res /= factorielle(p);
    return res;
}

//  Calcule le nombre de distributions possibles vu du joueur avec la structure pJeu passée en paramètre
//  Calcule pour chaque joueur combien de cartes sont possibles

double nbDistrib(TarotGame CurrentGame, struct _Jeu *pJeu)
{
int NbTotal = 0;
int TotalSur = 0;
int pos = pJeu->PositionJoueur;
int NbPoss[MAX_JOUEURS+1];
int NbSur[MAX_JOUEURS+1];
int i, j, nb, nbs;
double Res;
int nbNonJoue = pJeu->NbCarte;
int nbplacees;

    //  Tout d'abord calcule le nombre de cartes non jouées qui ne sont pas dans le jeu du joueur
    for ( i = 0; i < 78; i++ )
    {
        if ( CurrentGame->CarteJouee[i] >= 0 ) continue;
        if ( pJeu->ProbCarte[pos][i] < 1.0 ) NbTotal++;
    }
    for ( j = 0; j <= CHIEN; j++ )
    {
        nb = 0;
        nbs = 0;
        if ( j == pos )
        {
            NbPoss[j] = 0;
            NbSur[j] = nbNonJoue;
            continue;
        }
        for ( i = 0; i < 78; i++ )
        {
            if ( pJeu->ProbCarte[j][i] >= 1.0 )
            {
                nbs++;
                continue;
            }
            if ( pJeu->ProbCarte[j][i] > 0.0 ) nb++;
        }
        NbPoss[j] = nb;
        NbSur[j] = nbs;
        TotalSur += nbs;
    }
    //  commence par le joueur avec le nombre de cartes possibles le plus faible, pour cela trie les jeux
    for ( i = 0; i < 3; i++ )
    {
        for ( j = i+1; j < 4; j++ )
        {
            if ( NbPoss[j] < NbPoss[j-1] )
            {
                nb = NbPoss[j-1];
                NbPoss[j-1] = NbPoss[j];
                NbPoss[j] = nb;
                nb = NbSur[j-1];
                NbSur[j-1] = NbSur[j];
                NbSur[j] = nb;
            }
        }
    }
    //  Le premier joueur est celui qui fait le test, une seule possibilité, le passe
    // Pour le second joueur prend le min entre NBTotal et NbPoss[1]
    Res = C_n_p(NbTotal - TotalSur, nbNonJoue - NbSur[1]);
    nbplacees = nbNonJoue - NbSur[1];
    // Pour le troisième joueur prend le min entre NBTotal - nbNonJoue et NbPoss[1]
    nb = NbTotal - TotalSur - nbplacees;
    if ( NbPoss[2] < nb ) nb = NbPoss[2];
    Res *= C_n_p(nb, nbNonJoue - NbSur[2]);
    nbplacees += nbNonJoue - NbSur[2];
    // Pour le quatrième joueur prend le min entre NBTotal - 2*nbNonJoue et NbPoss[1]
    nb = NbTotal - TotalSur - nbplacees;
    if ( NbPoss[3] < nb ) nb = NbPoss[3];
    Res *= C_n_p(nb, nbNonJoue - NbSur[3]);
    //  Les cartes restantes sont forcément au chien, pas de multiplication supplémentaire
    return Res;
}

//  Retourne le nombre d'atouts joués
//  Compte sur 21 atouts

int NbAtoutJoues(TarotGame CurrentGame)
{
int nb = 0;
int i;

    for ( i = 1; i < 22; i++ )
    {
        if ( CurrentGame->CarteJouee[i] >= 0 ) nb++;
    }
    return nb;
}
