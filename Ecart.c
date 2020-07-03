#include <stdlib.h>
#include <gtk/gtk.h>
#include "Tarot_Ui_Objects.h"
#include <assert.h>

//  Retourne 1 si le preneur peut écarter la carte d'indice idx
//  Pas le droit d'écarter des atouts et des rois

int isEcartable(TarotGame CurrentGame, int idx)
{
    if ( CurrentGame->CartesJeu[idx].Couleur == EXCUSE ) return 0;
    if ( CurrentGame->CartesJeu[idx].Couleur == ATOUT ) return 0;
    if ( CurrentGame->CartesJeu[idx].Hauteur == ROI ) return 0;
    return 1;
}

//  Ajoute les cartes du chien au jeu du preneur
//  Premier calcul de nombre de cartes écartables et donc du nombre d'atouts à mettre à l'écart
//  Met également à jour le tableau CarteAuChien signalant que la carte a été vue au chien

void AjouteCarteChienPreneur(TarotGame CurrentGame)
{
int i, idx;
int NbEcartable = 0;

    memset(CurrentGame->CarteAuChien, 0, sizeof(CurrentGame->CarteAuChien));    //  Init à 0
    for ( i = 0; i < 6; i++ )
    {
        idx = CurrentGame->IdxCartesChien[i];
        CurrentGame->Carte2Joueur[idx] = CurrentGame->JoueurPreneur;    //  Met les cartes dans le jeu du joueur
        CurrentGame->CarteAuChien[idx] = 1;                             //  Vue au chien
    }
    RepartitionCarte(CurrentGame);
    EvalJeu(CurrentGame, CurrentGame->JoueurPreneur);
	for ( i = 0; i < 24; i++)
	{
	    if ( isEcartable(CurrentGame, CurrentGame->IdxCartesJoueur[CurrentGame->JoueurPreneur][i]))
            NbEcartable++;
	}
	CurrentGame->NbAtoutEcart = 0;
	if ( NbEcartable < 6 )  //  Si moins de 6 cartes écartables, il faudra mettre des atouts à l'écart
        CurrentGame->NbAtoutEcart = 6 - NbEcartable;
}

//  Met les cartes levées à l'écart

void MetCartesEcart(TarotGame CurrentGame)
{
int i, j;
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[CurrentGame->JoueurPreneur];


    for ( i = 0, j= 0; i < MAX_CARTE_JOUEUR; i++ )
    {
        if ( CurrentGame->isCarteLevee[i] )
        {
            CurrentGame->Carte2Joueur[CurrentGame->IdxCartesJoueur[CurrentGame->JoueurPreneur][i]] = CHIEN;
            pJeu->ValEcart += CurrentGame->CartesJeu[CurrentGame->IdxCartesJoueur[CurrentGame->JoueurPreneur][i]].Valeur;
            j++;
        }
        CurrentGame->isCarteLevee[i] = 0;       //  Indique carte baissée maintenant
    }
    CurrentGame->NbCarteLevee = 0;
    RepartitionCarte(CurrentGame);
    EvalJeu(CurrentGame, CurrentGame->JoueurPreneur);
    for ( i = 0; i < 78; i++ )
    {
        if ( CurrentGame->Carte2Joueur[i] == CHIEN )
            pJeu->StatusCartes[i] = -1;           //  Pas dans le jeu, considère jouée
    }
}

//	Evalue la force du jeu du preneur en fonction des cartes écartées
//  Retourne la force du jeu

int EvalEcart(TarotGame CurrentGame, struct _Jeu *pJeu)
{
int i;
int PointDH = 0;
int SizeCouleur[NB_COULEUR];
int PointCouleur[NB_COULEUR];
int HasPetit = 0;
int NbCoupe = 0;
int Moinsde7Atouts = pJeu->NbAtout <= 7;

    memset(SizeCouleur, 0, sizeof(SizeCouleur));        //  Init à 0
    memset(PointCouleur, 0, sizeof(PointCouleur));      //  Init à 0
	for ( i = 0; i < pJeu->NbAtout; i++)                //  Les premières cartes sont forcément des atouts
	{
	    if ( pJeu->MyCarte[i].Hauteur == 1 )
            HasPetit = 1;
		if ( CurrentGame->isCarteLevee[i] == 0 )        //  Pas mise au chien ?
		{
			PointDH += pJeu->MyCarte[i].Hauteur;        //  Non, compte la valeur = hauteur des atouts. Impose de mettre les plus petits au chien
		}
	}
	for ( ; i < MAX_CARTE_JOUEUR; i++)                  //  Pour les cartes restantes ( couleur )
	{
		if ( CurrentGame->isCarteLevee[i] == 0 )        //  Garde dans le jeu ?
		{
			SizeCouleur[pJeu->MyCarte[i].Couleur]++;
			if ( pJeu->MyCarte[i].Hauteur == VALET )
			{
				PointDH += 1;
				PointCouleur[pJeu->MyCarte[i].Couleur] |= 1;
			}
			else if ( pJeu->MyCarte[i].Hauteur == CAVALIER )
			{
				PointDH += 2;
				PointCouleur[pJeu->MyCarte[i].Couleur] |= 2;
			}
			else if ( pJeu->MyCarte[i].Hauteur == DAME )
			{
				PointDH += 3;
				PointCouleur[pJeu->MyCarte[i].Couleur] |= 4;
			}
			else if ( pJeu->MyCarte[i].Hauteur == ROI )
			{
				PointDH += 6;
				PointCouleur[pJeu->MyCarte[i].Couleur] |= 8;
			}
		}
		else            //  Valeur des cartes au chien, un de plus que dans le jeu (sûr)
		{
			if ( pJeu->MyCarte[i].Hauteur == VALET )
			{
				PointDH += 2;
			}
			else if ( pJeu->MyCarte[i].Hauteur == CAVALIER )
			{
				PointDH += 3;
			}
			else if ( pJeu->MyCarte[i].Hauteur == DAME )
			{
				PointDH += 4;
			}
			else if ( SizeCouleur[pJeu->MyCarte[i].Couleur] )   //  S'il reste des cartes de cette couleur, enlève un point
				PointDH--;
		}
	}
	//  Valeur des honneurs
	for ( i = TREFLE; i <= PIQUE; i++)
	{
		switch ( PointCouleur[i] )      //  Regarde les honneurs dans chaque couleur
		{
		    //
		case 0:
		case 1:
		case 8:
		case 7:
		case 11:
			break;
			//  Cavalier seul, Roi Valet : enlève un point si moins de 2 cartes dans la couleur
		case 2:
		case 9:
			if ( SizeCouleur[i] <= 2 ) PointDH--;
			break;
			//  Valet-Cavalier, Dame, Dame-Valet, Dame Cavalier, Roi Cavalier
			//  Enlève un point si moins de 2 cartes dans la couleur
		case 3:
		case 4:
		case 5:
		case 6:
		case 10:
			if ( SizeCouleur[i] <= 2 ) PointDH -= 2;
			break;
			//  Roi-Dame, Roi Dame Cavalier, Roi Dame Cavalier Valet
		case 12:
		case 13:
		case 14:
		case 15:
			if ( SizeCouleur[i] <= 6 || Moinsde7Atouts == 0)
                PointDH += 2;		//	2 de plus si mariage, garde mariage évite dame à l'écart si moins de 6 dans la couleur ou plus de 7 atouts
			if ( SizeCouleur[i] <= 3 ) PointDH++;   //  Et un de plus si moins de 3 cartes
			break;
		}
	}
	//  Valeurs des longues
	for ( i = TREFLE; i <= PIQUE; i++)
	{
		if ( SizeCouleur[i] == 5 && pJeu->NbAtout >= 6 )
			PointDH += 5;           //  5 Cartes et au moins 6 atouts : +5
		if ( SizeCouleur[i] >= 6 && pJeu->NbAtout >= 6 )
			PointDH += 7;           //  6 Cartes ou plus et au moins 6 atouts : +7
		if ( SizeCouleur[i] >= 7 && pJeu->NbAtout >= 7 )
			PointDH += 2*(SizeCouleur[i]-6);        //  Plus de 7 cartes et au moins 7 atouts : +2 par cartes au dessus de 6
		if ( SizeCouleur[i] >= 5 && PointCouleur[i] ) PointDH++;    //  Ajoute un point si carte habillée dans la couleur
	}
	//  Valeur des coupes / singlettes
	for ( i = TREFLE; i <= PIQUE; i++)
	{
		if ( SizeCouleur[i] == 0 )
		{
			PointDH += (5-NbCoupe*Moinsde7Atouts) + 3*HasPetit;           //  Une coupe : +5  et 8 si le joueur a le petit
			HasPetit = 0;                       //   Compté une seule fois
			NbCoupe++;
			if ( pJeu->NbAtout >= 7 ) PointDH += 2*(pJeu->NbAtout - 6);       //  Si plus de 7 atouts, ajoute 2 par atout
		}
		if ( SizeCouleur[i] == 1 ) PointDH += 4;        //  Singleton : 4 points
	}
	return(PointDH);
}



//	Sélectionne les cartes a mettre au chien
//  Pour cela essaie tous les groupes de 6 cartes possibles parmi les 24 et détermine le meilleur score
//  Retourne le nombre d'atouts ayant du être mis à l'écart (en général 0)

int MakeEcart(TarotGame CurrentGame)
{
int i0, i1, i2, i3, i4, i5;
int BestScore;
int val;
int Besti0, Besti1, Besti2, Besti3, Besti4, Besti5;
int CurAtoutEcart;
struct _Jeu *pJeu;


    for ( i0 = 0; i0 < MAX_CARTE_JOUEUR; i0++) CurrentGame->isCarteLevee[i0] = 0;
	//	Essaie tous les groupes de 6 cartes possibles parmi les 24 et détermine le meilleur
	BestScore = -1;
	Besti0 = Besti1 = Besti2 = Besti3 = Besti4 = Besti5 = -1;
	CurAtoutEcart = 0;
	pJeu = &CurrentGame->JeuJoueur[CurrentGame->JoueurPreneur];
	for ( i0 = 0; i0 < 19; i0++)
	{
	    //  i0 sera l'index de la première carte mise à l'écart.
	    //  donc i0 varie entre 0 et 18
	    //  Si doit mettre des atouts au chien, sélectionne les atouts les plus faibles pour l'écart, sauf le petit bien sûr
		if ( CurAtoutEcart < CurrentGame->NbAtoutEcart && pJeu->MyCarte[i0].Couleur == ATOUT && pJeu->MyCarte[i0].Valeur == 1 )
		{
			CurAtoutEcart++;
		}
		else if ( isEcartable(CurrentGame, pJeu->MyCarte[i0].Index) == 0 ) continue;        //  Pas écartable : continue
		//  On la sélectionne pour le chien.
		CurrentGame->isCarteLevee[i0] = 1;
		for ( i1 = i0+1; i1 < 20; i1++)
		{
		    //  i1 : index de la seconde carte à mettre à l'écart
            //  donc i1 varie entre i0+1 et 19
            //  Si doit mettre des atouts au chien, sélectionne les atouts les plus faibles pour l'écart, sauf le petit bien sûr
			if ( CurAtoutEcart < CurrentGame->NbAtoutEcart && pJeu->MyCarte[i1].Couleur == ATOUT && pJeu->MyCarte[i1].Valeur == 1 )
			{
				CurAtoutEcart++;
			}
            else if ( isEcartable(CurrentGame, pJeu->MyCarte[i1].Index) == 0 ) continue;     //  Pas écartable : continue
            CurrentGame->isCarteLevee[i1] = 1;
			for ( i2 = i1+1; i2 < 21; i2++)
			{
                //  i2 : index de la 3eme carte à mettre à l'écart
                //  donc i2 varie entre i1+1 et 20
                //  Si doit mettre des atouts au chien, sélectionne les atouts les plus faibles pour l'écart, sauf le petit bien sûr
                if ( CurAtoutEcart < CurrentGame->NbAtoutEcart && pJeu->MyCarte[i2].Couleur == ATOUT && pJeu->MyCarte[i2].Valeur == 1 )
                {
                    CurAtoutEcart++;
                }
                else if ( isEcartable(CurrentGame, pJeu->MyCarte[i2].Index) == 0 ) continue;     //  Pas écartable : continue
                CurrentGame->isCarteLevee[i2] = 1;
				for ( i3 = i2+1; i3 < 22; i3++)
				{
                    //  i3 : index de la 4eme carte à mettre à l'écart
                    //  donc i3 varie entre i2+1 et 21
                    //  Si doit mettre des atouts au chien, sélectionne les atouts les plus faibles pour l'écart, sauf le petit bien sûr
                    if ( CurAtoutEcart < CurrentGame->NbAtoutEcart && pJeu->MyCarte[i3].Couleur == ATOUT && pJeu->MyCarte[i3].Valeur == 1 )
                    {
                        CurAtoutEcart++;
                    }
                    else if ( isEcartable(CurrentGame, pJeu->MyCarte[i3].Index) == 0 ) continue;     //  Pas écartable : continue
                    CurrentGame->isCarteLevee[i3] = 1;
					for ( i4 = i3+1; i4 < 23; i4++)
					{
                        //  i4 : index de la 5eme carte à mettre à l'écart
                        //  donc i4 varie entre i3+1 et 22
                        //  Si doit mettre des atouts au chien, sélectionne les atouts les plus faibles pour l'écart, sauf le petit bien sûr
                        if ( CurAtoutEcart < CurrentGame->NbAtoutEcart && pJeu->MyCarte[i4].Couleur == ATOUT && pJeu->MyCarte[i4].Valeur == 1 )
                        {
                            CurAtoutEcart++;
                        }
                        else if ( isEcartable(CurrentGame, pJeu->MyCarte[i4].Index) == 0 ) continue;     //  Pas écartable : continue
                        CurrentGame->isCarteLevee[i4] = 1;
						for ( i5 = i4+1; i5 < 24; i5++)
						{
                            //  i5 : index de la dernière carte à mettre à l'écart
                            //  donc i5 varie entre i4+1 et 23
                            //  Si doit mettre des atouts au chien, sélectionne les atouts les plus faibles pour l'écart, sauf le petit bien sûr
                            if ( CurAtoutEcart < CurrentGame->NbAtoutEcart && pJeu->MyCarte[i5].Couleur == ATOUT && pJeu->MyCarte[i5].Valeur == 1 )
                            {
                                CurAtoutEcart++;
                            }
                            else if ( isEcartable(CurrentGame, pJeu->MyCarte[i5].Index) == 0 ) continue;     //  Pas écartable : continue
                            CurrentGame->isCarteLevee[i5] = 1;
							val = EvalEcart(CurrentGame, pJeu);
							if ( val > BestScore || (val == BestScore && (rand()%1 == 0)) )
							{
								BestScore = val;
								Besti0 = i0;
								Besti1 = i1;
								Besti2 = i2;
								Besti3 = i3;
								Besti4 = i4;
								Besti5 = i5;
							}
                            CurrentGame->isCarteLevee[i5] = 0;      //  Remet dans le jeu...
						}
                        CurrentGame->isCarteLevee[i4] = 0;      //  Remet dans le jeu...
					}
                    CurrentGame->isCarteLevee[i3] = 0;      //  Remet dans le jeu...
					if ( pJeu->MyCarte[i3].Couleur == ATOUT ) CurAtoutEcart--;
				}
                CurrentGame->isCarteLevee[i2] = 0;      //  Remet dans le jeu...
				if ( pJeu->MyCarte[i2].Couleur == ATOUT ) CurAtoutEcart--;
			}
            CurrentGame->isCarteLevee[i1] = 0;      //  Remet dans le jeu...
			if ( pJeu->MyCarte[i1].Couleur == ATOUT ) CurAtoutEcart--;
		}
        CurrentGame->isCarteLevee[i0] = 0;      //  Remet dans le jeu...
		if ( pJeu->MyCarte[i0].Couleur == ATOUT ) CurAtoutEcart--;
	}
	CurrentGame->isCarteLevee[Besti0] = 1;
	CurrentGame->isCarteLevee[Besti1] = 1;
	CurrentGame->isCarteLevee[Besti2] = 1;
	CurrentGame->isCarteLevee[Besti3] = 1;
	CurrentGame->isCarteLevee[Besti4] = 1;
	CurrentGame->isCarteLevee[Besti5] = 1;
	CurrentGame->NbCarteLevee = 6;
	return(CurrentGame->NbAtoutEcart);
}


//  Action de "voir" la poignée du joueur JoueurAvecPoignee.
//  Mise à jour des connaissances sur le jeu du preneur
//  le fait pour le joueur Position

void RegardeAtoutEcart(TarotGame CurrentGame, struct _Jeu *pJeu)
{
int i;

    //  Mise à jour des probabilités des cartes vues
    for ( i = 0; i < CurrentGame->NbAtoutEcart; i++ )
    {
        assert(CurrentGame->IdxCartesChien[i] < 22);
        //  On est sûr que la carte au chien !
        SetCarte2Position(pJeu, 4, CurrentGame->IdxCartesChien[i]);
        pJeu->StatusCartes[CurrentGame->IdxCartesChien[i]] = -1;        //  Comme si jouée !
        NormaliseProba(CurrentGame, pJeu);
    }
}
