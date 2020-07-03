#include <stdlib.h>
#include <gtk/gtk.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include "Tarot_Ui_Objects.h"

struct _Carte_Jeu Table[4];

//	Retourne l'atout le plus fort joue avant Position sur la table de jeu
//  Position = 0 --> Ouvreur
//  Si pas atout joué, retourne 0

int PlusFortAtoutJoue(int Position)
{
int i;
int h = 0;

	for ( i = 0; i < Position; i++)
	{
		if ( Table[i].Couleur == ATOUT && Table[i].Hauteur > h )
			h = Table[i].Hauteur;
	}
	return(h);
}


//  "Pose" la carte passée en index sur la table pour le Joueur courant
//  IndexCarte est l'index est parmi les cartes du joueur (0..NbCarte-1)
//  La met à la bonne position en fonction de l'entame
//  Enlève également la carte du jeu du joueur pour affichage
//  Remplit en même temps le tableau Table de cartes sur la "table"

void PoseCarte(TarotGame CurrentGame, int IndexCarte)
{
int Position = CurrentGame->JoueurCourant;
int idxTable = (Position - CurrentGame->JoueurEntame) & 3;
int posPreneur = (CurrentGame->JoueurPreneur - CurrentGame->JoueurEntame) & 3;
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[Position];
int i, j;
int iCarte;

    iCarte = CurrentGame->IdxCartesJoueur[Position][IndexCarte];    //  iCarte : index carte : 0 .. 77
    Table[idxTable] = pJeu->MyCarte[IndexCarte];            //  Recopie la carte jouée dans le tableau "Table"
    if ( idxTable > posPreneur && Gagnant(idxTable) == idxTable && Table[idxTable].Couleur > ATOUT && Table[idxTable].Hauteur > 9 )
    {
        //  Après le preneur, si prend le pli joue carte du dessus si le joueur la possède
        for ( i = IndexCarte+1; i < pJeu->NbCarte; i++ )
        {
            j = CurrentGame->IdxCartesJoueur[Position][i];
            if ( j == iCarte+1)
            {
                iCarte = j;
                IndexCarte++;
            }
        }
        Table[idxTable] = pJeu->MyCarte[IndexCarte];        //  Cela a peut être changé la carte
    }
    if ( idxTable == 3 && posPreneur == 3 && Gagnant(3) == 3 && IndexCarte < pJeu->NbCarte - 1)
    {
        //  Si preneur dernier gagne le pli met la carte au dessus si la possède
        for ( i = IndexCarte+1; i < pJeu->NbCarte; i++)
        {
            j = CurrentGame->IdxCartesJoueur[Position][i];
            if ( j == iCarte+1)
            {
                iCarte = j;
                IndexCarte++;
            }
        }
        Table[idxTable] = pJeu->MyCarte[IndexCarte];        //  Cela a peut être changé la carte
    }
    CurrentGame->CartePli[Position] = iCarte;               //  La met sur la table
    CurrentGame->CarteJouee[iCarte] = Position;             //  Carte jouée par le joueur position
    //  Maintenant enlève la carte de la structure Jeu du joueur et pour affichage
    for ( i = IndexCarte+1; i < CurrentGame->NbCarteJoueur[Position]; i++ )
    {
        pJeu->MyCarte[i-1] = pJeu->MyCarte[i];
        CurrentGame->IdxCartesJoueur[Position][i-1] = CurrentGame->IdxCartesJoueur[Position][i];
    }
    pJeu->NbCarte--;
    CurrentGame->NbCarteJoueur[Position]--;
    //  Passe probas pour cette carte à 0
    for ( i = 0; i < 4; i++)
    {
        for ( j = 0; j <= CHIEN; j++ )
        {
            CurrentGame->JeuJoueur[i].ProbCarte[j][iCarte] = 0;
            CurrentGame->JeuJoueur[i].ConfianceProb[j][iCarte] = 1.0;
        }
    }
#if DEBUG > 0
    OutDebug("Joueur %d, Position %d, pose Carte ", CurrentGame->JoueurCourant, idxTable);
    ImprimeCarte(&Table[idxTable]);
    OutDebug("Reste NbCarte=%d NbCarteJoueur[%d]=%d\n", pJeu->NbCarte, Position, CurrentGame->NbCarteJoueur[Position]);
    if ( Table[idxTable].Index == 29 )
        i = 0;
#endif // DEBUG
}

//	Retourne 1 si la carte proposée est valide dans le contexte de jeu
//  pJeu : pointeur sur la structure Jeu du joueur proposant cette carte
//  IndexCarte : index dans les cartes de ce joueur (0..NbCarte-1)

int CarteValide(struct _Jeu *pJeu, int IndexCarte, int JoueurEntame)
{
int pos = (pJeu->PositionJoueur - JoueurEntame) & 3;
int CouleurDemandee, HauteurMin;
int i;

	if ( pos == 0 ) return(1);              //  Le premier joueur peut faire ce qu'il veut !
#if DEBUG > 0
	if ( pos == 2 && Table[0].Index == 25)
        i = 0;
#endif // DEBUG
	if ( pJeu->MyCarte[IndexCarte].Couleur == EXCUSE ) return(1);        //  Excuse toujours valide également
	CouleurDemandee = Table[0].Couleur;             //  Couleur demandée
	if ( CouleurDemandee == EXCUSE )
	{
		if ( pos == 1 ) return(1);                  //  Après l'excuse en premier tout est valide
		CouleurDemandee = Table[1].Couleur;         //  Nouvelle couleur demandée
	}
	//  Premier cas : joue de la couleur demandée
	if ( CouleurDemandee == pJeu->MyCarte[IndexCarte].Couleur )
	{
		if ( CouleurDemandee != ATOUT ) return(1);          //  Si pas atout, tout est valide
		//  Il faut monter à l'atout, calcule hauteur minimale
		HauteurMin = -1;
		for ( i = 0; i < pos; i++)
		{
			if ( Table[i].Couleur == ATOUT && Table[i].Hauteur > HauteurMin )
				HauteurMin = Table[i].Hauteur;
		}
		if ( pJeu->MyCarte[IndexCarte].Hauteur > HauteurMin ) return(1);      //  C'est bon on est au dessus
		//  Sinon, vérifie que le joueur n'a pas d'atout plus gros
		for ( i = IndexCarte+1; i < pJeu->NbCarte; i++)
		{
			if ( pJeu->MyCarte[i].Couleur != ATOUT ) break;
			if ( pJeu->MyCarte[i].Hauteur > HauteurMin ) return(0);
		}
		return(1);
	}
	//  Le joueur ne joue pas de la couleur demandée, regarde déjà s'il en a
	if ( pJeu->NbCouleur[CouleurDemandee] != 0 ) return(0);     //  S'il en a, doit en jouer
	if ( pJeu->NbAtout == 0 ) return(1);                        //  Si le joueur n'a pas de la couleur demandée et pas d'atout, tout est valide
	//  Cas avec un seul ATOUT, l'excuse, les autres couleurs sont OK
	if ( pJeu->NbAtout == 1 && pJeu->MyCarte[0].Couleur == EXCUSE ) return 1;
    //  Le joueur a de l'atout, il doit en jouer
    if ( pJeu->MyCarte[IndexCarte].Couleur != ATOUT ) return 0;
    //  Vérifie maintenant que l'atout joué est à la bonne hauteur
    HauteurMin = PlusFortAtoutJoue(pos);
    if ( pJeu->MyCarte[IndexCarte].Hauteur > HauteurMin )       //  Plus fort : OK
        return(1);
    for ( i = IndexCarte+1; i < pJeu->NbCarte; i++)
    {
        if ( pJeu->MyCarte[i].Couleur != ATOUT ) break;
        if ( pJeu->MyCarte[i].Hauteur > HauteurMin ) return(0); //  Possède au dessus, doit jouer cette carte
    }
    return(1);
}

//	Retourne l'index du gagnant du pli en cours, regarde Table jusqu'à idxPosition inclus
//  L'index est par rapport au joueur entame. 0 --> Joueur Entame

int Gagnant(int idxPosition)
{
int gagnant = 0;

	for ( int j = 1; j <= idxPosition; j++)
	{
		if ( isSup(&Table[gagnant], &Table[j]) )
			gagnant = j;
	}
	return(gagnant);
}

//	Retourne 1 si l'équipe du Joueur en position PosTable sur la table remporte le pli, 0 sinon

int GainPli(TarotGame CurrentGame, int PosTable)
{
int GainAttaque;
int j, gagnant;
int Pos = (PosTable + CurrentGame->JoueurEntame) & 3;

	//	Regarde qui a le pli
	gagnant = 0;    //  Par défaut premier à jouer
	for ( j = 1; j < 4; j++)
	{
	    if ( isSup(&Table[gagnant], &Table[j]) )
			gagnant = j;
	}
	//  Traduit vers position absolue
	gagnant = (gagnant + CurrentGame->JoueurEntame) & 3;
	if ( gagnant == CurrentGame->JoueurPreneur )
		GainAttaque = 1;
	else
		GainAttaque = 0;
    if ( Pos == CurrentGame->JoueurPreneur ) return GainAttaque;
    return !GainAttaque;
}


//  Mise à jour des vues de jeu après le pli
//  Position est l'index du Joueur, pas l'index sur la table !

void MiseAJourJeuJoueur(TarotGame CurrentGame, int Position)
{
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[Position];
int i, j;
int c;
int IndexCarte;
int pos;
int Couleur;
double nb;
int Boucle;
int IndexPreneur = (CurrentGame->JoueurPreneur - CurrentGame->JoueurEntame)&3;
double GuessAtoutPreneur;

	pJeu->NbEntame[CurrentGame->JoueurEntame]++;                //  Une entame de plus pour ce joueur
	pJeu->CouleurEntameParJoueur[CurrentGame->JoueurEntame] |= 1 << Table[0].Couleur;
	//  Ensuite parcourt le pli, passe à 0 les probas des cartes jouées
	for ( i = 0; i < 4; i++)
	{
		pos = (i + CurrentGame->JoueurEntame) & 3;
		IndexCarte = Table[i].Index;
		//  Carte jouée, passe proba à 0
		for ( j = 0; j <= CHIEN; j++ )
        {
            pJeu->ProbCarte[j][IndexCarte] = 0;
            pJeu->ConfianceProb[j][IndexCarte] = 1.0;
        }
        Couleur = Table[i].Couleur;
        pJeu->NbJoue[pos][Couleur]++;
        //	Si carte jouée par lui-même, mise à jour des Longueurs et nombre d'atouts
		if ( pos == pJeu->PositionJoueur )
		{
			if ( Couleur <= ATOUT )
                pJeu->NbAtout--;
            else
				pJeu->NbCouleur[Couleur]--;
		}
	}
    NormaliseProba(CurrentGame, pJeu);                  //  Re normalise les probabilités si besoin est.
#if DEBUG > 0
    if ( Table[0].Couleur == COEUR && Position == 1 )
        i = 0;
#endif // DEBUG
    for ( i = 0; i < 4; i++ )                           //  Parcourt de nouveau le pli pour appliquer les règles
    {
		pos = (i + CurrentGame->JoueurEntame) & 3;
		IndexCarte = Table[i].Index;
        Regle2Proba(CurrentGame, pJeu, i);	            //	Mise a jour en fonction des règles
#if DEBUG > 0
        NormaliseProba(CurrentGame, pJeu);              //  Re normalise les probabilités si besoin est.
#endif // DEBUG        //  Puis applique diverses heuristiques
        Heuristique_Petit_Proba(CurrentGame, pJeu, i);
#if DEBUG > 10
        NormaliseProba(CurrentGame, pJeu);              //  Re normalise les probabilités si besoin est.
#endif // DEBUG        //  Puis applique diverses heuristiques
        Heuristique_Chasse(CurrentGame, pJeu, i);
#if DEBUG > 10
        NormaliseProba(CurrentGame, pJeu);              //  Re normalise les probabilités si besoin est.
#endif // DEBUG        //  Puis applique diverses heuristiques
        Heuristique_Atout(CurrentGame, pJeu, i);
#if DEBUG > 10
        NormaliseProba(CurrentGame, pJeu);              //  Re normalise les probabilités si besoin est.
#endif // DEBUG        //  Puis applique diverses heuristiques
        Heuristique_Couleur(CurrentGame, pJeu, i);
#if DEBUG > 0
        NormaliseProba(CurrentGame, pJeu);              //  Re normalise les probabilités si besoin est.
#endif // DEBUG        //  Puis applique diverses heuristiques

        //	Si carte jouée par le preneur, mise a jour des probabilités de coupe
        if ( pos == pJeu->PositionJoueur ) continue;            //  N'apprend rien pour son propre jeu
        if ( i > 0 && pos == CurrentGame->JoueurPreneur )		//	Calcul des probas de coupe Preneur
        {
            if ( Couleur > ATOUT )
            {
                pJeu->GuessProbCoupe[Couleur] /= 2.0;	        //	Couleur jouée par preneur, change Proba coupe !
                if ( pJeu->JoueurCouleur[Couleur] < 0 )         //  Ouverture
                {
                    //  Change les probabilités des couleurs non encore ouvertes
                    for ( c = TREFLE; c < NB_COULEUR; c++)
                    {
                        if ( Couleur == c ) continue;
                        if ( pJeu->JoueurCouleur[c] >= 0 ) continue;
                        if ( pJeu->ProbCoupe[c] == 0 ) continue;        //  Cas sûr (roi au chien par exemple)
                        if ( pJeu->NbCoupeInit < 1 )
                            pJeu->GuessProbCoupe[c] += (1.0 - pJeu->GuessProbCoupe[c]) * 0.5;       //  Change proba coupe des autres couleurs
                    }
                }
            }
		}
	}
	NormaliseProba(CurrentGame, pJeu);
    Couleur = Table[0].Couleur;
    //  En fonction de l'entame, recalcule flanc, obéissance...
    //	Met ValFlanc a jour en fonction des coups joués
    if ( CurrentGame->JoueurEntame != CurrentGame->JoueurPreneur )
    {

        //  Seulement si entame ne venant pas du preneur
        if ( pJeu->Flanc > ATOUT && pJeu->ValFlanc[pJeu->Flanc] >= 0.3 && pJeu->JoueurCouleur[pJeu->Flanc] == JoueurAvecPetit(CurrentGame, pJeu)
            && pJeu->GuessProbCoupe[pJeu->Flanc] == 1.0 && CurrentGame->CarteJouee[1] < 0 )
        {
            pJeu->ValFlanc[pJeu->Flanc] *= 0.75;    //  Baisse valeur flanc
        }
        else if ( pJeu->ForceMain[CurrentGame->JoueurEntame] > 0.8 )
        {   //  Si jeu fort, change le flanc
            for ( c = 0; c < NB_COULEUR; c++)
                pJeu->ValFlanc[c] /= 2.0;
            if ( Table[0].Index > 1 )
                pJeu->ValFlanc[Couleur] = 1.0;
        }
        else if ( pJeu->ValFlanc[Couleur] <= 0.5 && pJeu->ForceMain[pJeu->JoueurMainForte] < 0.8 )
        {   //  Flanc un peu faible et joueur main forte pas très fort...
            for ( c = 0; c < 6; c++)
                pJeu->ValFlanc[c] /= 1.5;   //  Baisse un peu la valeur des autres flancs
            if ( Table[0].Index > 1 )       //  Pas valide si petit ou excuse....
            {
                pJeu->ValFlanc[Couleur] = 0.5;
                if ( Table[IndexPreneur].Couleur == ATOUT && Couleur < ATOUT )
                {
                    pJeu->ValFlanc[Couleur] = 0.8;      //  Le preneur coupe, valeur moyenne au nouveau flanc
                }
                if ( pJeu->ForceMain[CurrentGame->JoueurEntame] == 0 )
                {
                    pJeu->ForceMain[CurrentGame->JoueurEntame] = 0.5;       //  Lui met une force moyenne
                    pJeu->Obeissance[CurrentGame->JoueurEntame] = 0.8;      //  Et on le suit mais pas aveuglement !
                }
            }
        }
    }
    //	Met à jour obéissance pour soi-même
    if ( FlagSignalisation && CurrentGame->JoueurEntame != CurrentGame->JoueurPreneur && pJeu->NbEntame[CurrentGame->JoueurEntame] == 1
            && Position == CurrentGame->JoueurEntame )
    {
        if ( pJeu->JoueurMainForte < 0 )                //  Pas encore d'homme fort ?
        {
            pJeu->Obeissance[Position] = 1.0;           //  Se déclare fort
        }
    }
    //	Si le preneur entame dans une couleur, n'est plus le bon flanc
	if ( Position != CurrentGame->JoueurPreneur && CurrentGame->JoueurEntame == CurrentGame->JoueurPreneur && Table[0].Couleur != ATOUT)
	{
		pJeu->ValFlanc[Couleur] = -0.5;
		if ( pJeu->Flanc == Couleur ) pJeu->Flanc = -1;
	}
    //	Status Chasse
	if ( pJeu->StatusChasse == CHASSE_ATTENTE && Table[0].Couleur == ATOUT && Table[0].Index != 1
		&& NbReste(CurrentGame, Position, ATOUT) + pJeu->NbAtout >= 18 && Position != CurrentGame->JoueurEntame
        && pJeu->ProbCarte[CurrentGame->JoueurEntame][1] < 0.9999)
	{
		if ( CurrentGame->JoueurEntame == CurrentGame->JoueurPreneur )
		{
			pJeu->StatusChasse = CHASSE_PRENEUR_DESSOUS;
            if ( Table[0].Index >= 18 ) pJeu->StatusChasse = CHASSE_PRENEUR_GROS;
			if ( !HasCarte(pJeu, CurrentGame->JoueurPreneur, 1) )
				BaisseProba(CurrentGame, Position, CurrentGame->JoueurPreneur, 1, 0.65);	//	Si lance la chasse ne possède pas le petit
		}
		else if ( Table[0].Index == 21 )
		{
			pJeu->StatusChasse = CHASSE_DEFENSE_JOUE_21;				//	Ouverture du 21, pas vraiment chasse
		}
		else if ( FlagSignalisation )
		{
			if ( Table[0].Hauteur & 1 )
			{
				pJeu->StatusChasse = CHASSE_DEFENSE_IMPAIR;		//	Au moins 6 Atouts...
				for ( i = 1; i < 22; i++)
                {
                    if ( CurrentGame->CarteJouee[i] >= 0 ) continue;
					MonteProba(CurrentGame, Position, CurrentGame->JoueurEntame, i, 1.05, 0);
                }
                //  Le joueur qui entame ATOUT impair est fort, suivons le
				pJeu->ForceMain[CurrentGame->JoueurEntame] = 1.0;
				pJeu->Obeissance[CurrentGame->JoueurEntame] = 1.0;
				pJeu->JoueurMainForte = CurrentGame->JoueurEntame;
				pJeu->Flanc = ATOUT;
				pJeu->ValFlanc[ATOUT] = 1.0;
			}
			else
            {
				pJeu->StatusChasse = CHASSE_DEFENSE_PAIR;
            }
			BaisseProba(CurrentGame, Position, CurrentGame->JoueurEntame, 1, 0.85);	//	Si lance la chasse ne possède pas le petit
		}
		else
		{
			pJeu->StatusChasse = CHASSE_DEFENSE_IMPAIR;
			BaisseProba(CurrentGame, Position, CurrentGame->JoueurEntame, 1, 0.85);	//	Si lance la chasse ne possède pas le petit
		}
	}
	else if ( pJeu->StatusChasse == CHASSE_DEFENSE_PAIR && Position != CurrentGame->JoueurEntame )
	{
		if ( Table[0].Couleur == ATOUT )
			pJeu->StatusChasse = CHASSE_DEFENSE_IMPAIR;
		else if ( Table[0].Couleur != ATOUT )
			pJeu->StatusChasse = CHASSE_TERMINEE;
	}
    //	Calcule les défausses
    for ( i = 1; i < 4; i++ )
    {
		pos = (i + CurrentGame->JoueurEntame) & 3;
        //  Le joueur entame ne peut se défausser, donc i commence à 1
        if ( Table[i].Couleur != Couleur && Table[i].Couleur != ATOUT && GainPli(CurrentGame, i) )
        {
            pJeu->NbDefausse[pos]++;      //  Défausse pour le joueur pos
        }
    }
#if DEBUG > 0
        if ( Position == 2)
            c = 0;
#endif // DEBUG    //	Maintenant compte les atouts du preneur et essaie de se rapprocher de la "moyenne"
	Boucle = 10;
	if ( pJeu->PositionJoueur != pJeu->PositionPreneur && pJeu->NbCarte > 2 )
	{
		i = NbBoutDefense(CurrentGame, pJeu->PositionJoueur);
		//	Compare l'estimation des atouts preneur avec le nombre d'atouts "moyen"
		nb = AvgLongueur(pJeu, pJeu->PositionPreneur, ATOUT) + pJeu->NbJoue[pJeu->PositionPreneur][ATOUT];
		GuessAtoutPreneur = NbAtoutMoyen[CurrentGame->TypePartie][3-i];
		//  Si petit posé rapidement, descend atout moyen preneur.
		if ( pJeu->PetitSurCoupe == 1 )
            GuessAtoutPreneur -= 1.5;
        else if ( pJeu->PetitSurCoupe == 2 )
            GuessAtoutPreneur -= 0.75;
		nb -= GuessAtoutPreneur;
		while ( --Boucle > 0 && nb < (-5.0 )/sqrt(pJeu->NbCarte-1) )	//	Grosse différence (pas assez d'atout) ?
		{
			for ( j = 1; j <= 21; j++)
			{
			    if ( pJeu->ConfianceProb[pJeu->PositionPreneur][j] < 1.0 )
                    MonteProba(CurrentGame, pJeu->PositionJoueur, pJeu->PositionPreneur, j, 1 + 0.05*(1-pJeu->ConfianceProb[pJeu->PositionPreneur][j]), 0);
			}
			nb = pJeu->NbJoue[pJeu->PositionPreneur][ATOUT];
			for ( j = 1; j <= 21; j++)
			{
				if ( pJeu->ProbCarte[pJeu->PositionPreneur][j] > 0 )
					nb += pJeu->ProbCarte[pJeu->PositionPreneur][j];
			}
			nb -= GuessAtoutPreneur;
		}
	}
	NormaliseProba(CurrentGame, pJeu);
	CopieProba2Tmp(pJeu);
	CalcProbCoupe(CurrentGame, pJeu, CurrentGame->JoueurEntame);			//	Mise a jour des proba de coupe...


//	Teste validité du flanc...
	if ( pJeu->PositionJoueur != pJeu->PositionPreneur && pJeu->Flanc > 0)
	{
		if ( pJeu->Flanc == ATOUT  && pJeu->CouleurTenue >= 0)
		{	//	teste que la couleur est toujours tenue
			if ( pJeu->ProbCoupe[pJeu->CouleurTenue] > 0.5 )    //  Le preneur va couper, change le flanc
                pJeu->Flanc = -1;
            else if ( NbReste(CurrentGame, pJeu->PositionJoueur, pJeu->CouleurTenue) <= 1 )
                pJeu->Flanc = -1;
			else if ( AvgLongueur(pJeu, pJeu->PositionPreneur, pJeu->CouleurTenue) < 1
                 && pJeu->ProbCarte[pJeu->PositionPreneur][CarteMaitreNonJouee(CurrentGame, pJeu->CouleurTenue)] > 0.5 )
                pJeu->Flanc = -1;
			else if (  NbReste(CurrentGame, pJeu->PositionJoueur, pJeu->CouleurTenue) <= 3 &&
                    pJeu->ProbCarte[pJeu->PositionPreneur][CarteMaitreNonJouee(CurrentGame, pJeu->CouleurTenue)] > 0.5 )
                pJeu->Flanc = -1;
		}
	}
	if ( pJeu->JoueurCouleur[Table[0].Couleur] < 0 ) pJeu->JoueurCouleur[Table[0].Couleur] = CurrentGame->JoueurEntame;
	pJeu->AtoutPreneur = AvgLongueur(pJeu, pJeu->PositionPreneur, ATOUT);
}

//  Mémorise le pli en cours

void MemorisePli(TarotGame CurrentGame)
{
int i, pos;

    for ( pos = 0; pos < 4; pos++ )
    {
        i = (CurrentGame->JoueurEntame + pos) & 3;
        CurrentGame->MemPlis[CurrentGame->NumPli][i] = Table[pos].Index;
    }
    CurrentGame->Entame[CurrentGame->NumPli] = CurrentGame->JoueurEntame;
    CurrentGame->NumPli++;
}

//  "Ramasse" le pli à fin.
//  Met à jour les différents états

void RamassePli(TarotGame CurrentGame)
{
int pts[2];
int j;
int gagnant;
int PliAttaque;

	//	Regarde qui a le pli
	gagnant = 0;    //  Par défaut premier à jouer
	for ( j = 1; j < 4; j++)
	{
	    if ( isSup(&Table[gagnant], &Table[j]) )
			gagnant = j;
	}
	//  Traduit vers position absolue
	gagnant = (gagnant + CurrentGame->JoueurEntame) & 3;
	if ( gagnant == CurrentGame->JoueurPreneur )
		PliAttaque = 1;
	else
		PliAttaque = 0;
	//	Compte points et atouts
	pts[0] = 0;
	pts[1] = 0;
	for ( j = 0; j < 4; j++)
	{
		if ( Table[j].Couleur == EXCUSE )
		{
		    //  Si jouée en dernier, appartient à celui qui a fait le pli
		    //  Sauf si chelem
			if ( CurrentGame->NumPli == 17 )		//	Excuse joue en dernier
			{
			    if ( CurrentGame->NumPlisPreneur == 17 && j == 0)
                {
                    //  Dit que l'attaquant a le pli si il a joué l'excuse
                    PliAttaque = 1;
                }
			    if ( CurrentGame->NumPlisPreneur == 0 && j == 0)
                {
                    //  Dit que le défenseur a le pli si il a joué l'excuse
                    PliAttaque = 0;
                }
				pts[PliAttaque] += 9;
				if ( PliAttaque == 1 )
					CurrentGame->NumBoutsPreneur++;
				else
					CurrentGame->NumBoutsDefense++;
			}
			else
			{
				if ( ((j + CurrentGame->JoueurEntame) & 3) == CurrentGame->JoueurPreneur )
				{
					pts[1] += 8;
					CurrentGame->NumBoutsPreneur++;
					pts[PliAttaque]++;
				}
				else
				{
					pts[0] += 8;
					CurrentGame->NumBoutsDefense++;
					pts[PliAttaque]++;
				}
			}
		}
		else if ( Table[j].Couleur == ATOUT )
		{
			CurrentGame->NumAtoutsJoues++;
			if ( Table[j].Hauteur == 1 || Table[j].Hauteur == 21 )
			{
				pts[PliAttaque] += 9;
				if ( PliAttaque == 1 )
					CurrentGame->NumBoutsPreneur++;
				else
					CurrentGame->NumBoutsDefense++;
			}
			else
				pts[PliAttaque]++;
		}
		else
		{
			if ( Table[j].Hauteur <= 10 )
				pts[PliAttaque]++;
			else
				pts[PliAttaque] += (Table[j].Hauteur - 10) * 2 + 1;
		}
	}
	CurrentGame->NumPointsDefense += pts[0] >> 1;
	CurrentGame->NumPointsPreneur += pts[1] >> 1;
	if ( PliAttaque == 1 )
		CurrentGame->NumPlisPreneur++;
    for ( j = 0; j < 4; j++ )
    {
        CurrentGame->CartePli[j] = -1;          //  Efface les cartes
    }
    if ( CurrentGame->NumPli < 17 )
    {
        //  Mise à jour des probas sauf si dernier pli
        for ( j = 0; j < MAX_JOUEURS; j++)
        {
            MiseAJourJeuJoueur(CurrentGame, j);
        }
    }
    MemorisePli(CurrentGame);
    CurrentGame->JoueurEntame = gagnant;        //  Prochain à jouer
    CurrentGame->JoueurCourant = CurrentGame->JoueurEntame;
#if DEBUG > 0
    OutDebug("Fin du pli %d\n", CurrentGame->NumPli);
    OutDebug("Points Preneur %d, Défense %d\n", CurrentGame->NumPointsPreneur, CurrentGame->NumPointsDefense);
    OutDebug("--------------------------------------------------------------------------------------------\n");
#endif // DEBUG
}

//	Retourne 1 si signalisation dans la couleur
//	Pour cela, il faut que
//	1) Deuxième carte jouée dans la couleur
//	2) Cela descend...
//	3) Le joueur n'a pas ouvert la couleur...

int LookForSigDescendant(TarotGame CurrentGame, int PosJoueur, int Couleur)
{
int i;
int idxCarte;
int Compte = 0;
int Hauteur = 0;

	for ( i = CurrentGame->NumPli-1; i >= 0; i--)
	{
		idxCarte = CurrentGame->MemPlis[i][PosJoueur];
		if ( idxCarte >= Startof[Couleur] && idxCarte < Endof[Couleur] )
		{
			Compte++;
			Hauteur = idxCarte - Startof[Couleur] + 1;
			if ( CurrentGame->Entame[i] == PosJoueur ) return(0);
			if ( Hauteur >= CAVALIER ) return(0);
		}
	}
	if ( Compte == 1 && Hauteur > Table[(PosJoueur - CurrentGame->JoueurEntame)&3].Hauteur ) return(1);
	return(0);
}

//  retourne l'index du joueur si la carte passée en Index (IndexCarte) est dans le pli N° nPli
//  Sinon retourne -1

int isCarteInPli(TarotGame CurrentGame, int nPli, int IndexCarte )
{
int i;

    for ( i = 0; i < 4; i++ )
    {
        if ( CurrentGame->MemPlis[nPli][i] == IndexCarte )
            return(i);
    }
    return -1;
}

//	Teste si possibilité petit chez le joueur
//	Doit descendre à l'atout alors qu'il pouvait monter.
//	Ne pas être juste devant le preneur.

int LookForPetit(TarotGame CurrentGame, int joueur)
{
int Compte = 0;
int Hauteur = 0;
int Hmin = 0;
int i;
int c = Table[(joueur - CurrentGame->JoueurEntame)&3].Couleur;      //  Couleur jouée par le joueur
int posPreneur = (CurrentGame->JoueurPreneur - CurrentGame->JoueurEntame) & 3;
int posJoueur = (joueur - CurrentGame->JoueurEntame) & 3;
int pos;

	if ( posJoueur == 0 ) return(0);				//	Ne peut rien dire dans ce cas, retourne 0
	if ( posPreneur == posJoueur+1 ) return(0);	            //	Ne peut rien dire si juste avant le preneur.
	if ( c != ATOUT ) return 0;                             //  Seulement si joue ATOUT
	//	Recherche dans les plis la précédente carte jouée à l'atout.
	for ( i = CurrentGame->NumPli-1; i >= 0; i--)
	{
	    Hauteur = CurrentGame->MemPlis[i][joueur];
	    if ( Hauteur == 0 || Hauteur > 21 ) continue;       //  Pas joué atout, passe pli précédent
        Compte++;
        if ( CurrentGame->Entame[i] == joueur ) return(0);      //  Entame ATOUT, pas le petit !
        if ( ((CurrentGame->JoueurPreneur-CurrentGame->Entame[i])& 3) - ((joueur-CurrentGame->Entame[i])&3) == 1 ) return(0);
        Hmin = 0;
        for ( int j = 1; j < 4; j++)
        {
            pos = (CurrentGame->Entame[i] + j) & 3;
            if ( pos == joueur ) break;
            //  Recherche hauteur minimale à mettre pour ce pli
            if ( CurrentGame->MemPlis[i][pos] < 22 && CurrentGame->MemPlis[i][pos] > Hmin )
                Hmin = CurrentGame->MemPlis[i][pos] + 1;
        }
        break;
	}
	//  Si a déjà joué ATOUT plus fort que l'atout actuel et que l'atout actuel était suffisant retourne 1
	if ( Compte == 1 && Hauteur > Table[posJoueur].Hauteur && Hmin <= Table[posJoueur].Hauteur)
		return(1);
	return(0);
}

