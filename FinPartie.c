#include <stdlib.h>
#include <gtk/gtk.h>
#include "Tarot_Ui_Objects.h"
#include <assert.h>
#include <math.h>

//  Les distributions avec une probabilité plus faible que cette limite ne seront pas traitées.
#define PROB_MIN_DISTRIB    1e-4

#if DEBUG > 0
#define DEBUG_FIN_PARTIE 2
#else
#define DEBUG_FIN_PARTIE 0
#endif  // DEBUG

//  Prépare les structures des autres joueurs après avoir créé une distribution pour le joueur Courant

void PrepareStructJoueur(TarotGame CurrentGame)
{
int i, j, idxCarte, c;
struct _Jeu *pJeuRef, *pJeu;

    pJeuRef = &CurrentGame->JeuJoueur[CurrentGame->JoueurCourant];      //  Jeu de référence
    for ( j = 0; j < MAX_JOUEURS; j++ )
    {
        if ( j == CurrentGame->JoueurCourant ) continue;     //  Rien si joueur courant
        pJeu = &CurrentGame->JeuJoueur[j];
        //  Tout d'abord recopie la partie probcarte
        memcpy(pJeu->ProbCarte, pJeuRef->ProbCarte, sizeof(pJeu->ProbCarte));
        CopieProba2Tmp(pJeu);       //  Idem probas temporaires
        pJeu->NbAtout = 0;
        for ( c = 0; c < NB_COULEUR; c++ )
        {
            pJeu->NbCouleur[c] = 0;
        }
        //  Puis la structure carte et StatusCartes
        idxCarte = 0;
        for ( i = 0; i < 78; i++ )
        {
            if ( pJeu->ProbCarte[j][i] > 0.9999 )
            {
                pJeu->StatusCartes[i] = 1;      //  Possède cette carte
                MakeCarte(&pJeu->MyCarte[idxCarte], i);     //  Crée la carte dans la structure carte
                CurrentGame->IdxCartesJoueur[j][idxCarte] = i;  //  Mise à jour index des cartes du joueur
                if ( i < 22 )
                    pJeu->NbAtout++;
                else
                    pJeu->NbCouleur[pJeu->MyCarte[idxCarte].Couleur]++;
                idxCarte++;
            }
            else if ( pJeu->StatusCartes [i] >= 0 ) //  Ne change pas les cartes déjà jouées
            {
                pJeu->StatusCartes[i] = 0;      //  Ne l'a pas
            }
        }
    }
}

//  Joue la carte index Index du joueur courant et va jusqu'à la fin de la partie avec la distribution courante
//  Retourne le score fait par l'attaque

int JoueFinPartie(TarotGame CurrentGame, int Index)
{
int isAttaque;
int Score;
int BestScore = -100000;
TarotGame locGame = (TarotGame) malloc(sizeof(struct _Tarot_Partie));
int Signe;
int i;


    if ( CurrentGame->JoueurCourant == CurrentGame->JoueurPreneur )
        isAttaque = 1;
    else
        isAttaque = -1;   //  Attaque --> 1, Défense -1;
#if DEBUG_FIN_PARTIE > 1
    OutDebug("Entrée JoueFinPartie, Joueur Courant = %d, NbCarte=%d, isAttaque = %d, IndexCarte=%d\n",
                CurrentGame->JoueurCourant, CurrentGame->NbCarteJoueur[CurrentGame->JoueurCourant], isAttaque, Index);
#endif // DEBUG_FIN_PARTIE
    //  Efface table de jeu
    PoseCarte(CurrentGame, Index);      //  Pose première carte
    MakeCarte(&Table[1], 0);            //  Efface les autres
    MakeCarte(&Table[2], 0);
    MakeCarte(&Table[3], 0);
    CurrentGame->JoueurCourant++;       //  Passe au suivant
    CurrentGame->JoueurCourant &= 3;
    //  Joue pour le second
    if ( CurrentGame->JoueurCourant == CurrentGame->JoueurPreneur )
        JoueAttaqueSecond(CurrentGame);
    else
        JoueDefenseSecond(CurrentGame);
    CurrentGame->JoueurCourant++;       //  Passe au suivant
    CurrentGame->JoueurCourant &= 3;
    //  Joue pour le troisième
    if ( CurrentGame->JoueurCourant == CurrentGame->JoueurPreneur )
        JoueAttaqueTroisieme(CurrentGame);
    else
        JoueDefenseTroisieme(CurrentGame);
    CurrentGame->JoueurCourant++;       //  Passe au suivant
    CurrentGame->JoueurCourant &= 3;
    //  Joue pour le dernier
    if ( CurrentGame->JoueurCourant == CurrentGame->JoueurPreneur )
        JoueAttaqueDernier(CurrentGame);
    else
        JoueDefenseDernier(CurrentGame);
    //  Ramasse le pli
    RamassePli(CurrentGame, 1);
    CurrentGame->NumPli++;                  //  Incrémente n° de pli car pas fait dans RamassePli
    if ( CurrentGame->NumPli != 18 )        //  Fin de partie ?
    {
        //  Joue le pli suivant en testant toutes les cartes du joueur entame
        if (CurrentGame->JoueurCourant == CurrentGame->JoueurPreneur)
            Signe = 1;
        else
            Signe = -1;
        for ( i = 0; i < CurrentGame->NbCarteJoueur[CurrentGame->JoueurEntame]; i++)
        {
            *locGame = *CurrentGame;        //  copie structure courante car va être modifiée à chaque carte jouée
            Score = JoueFinPartie(locGame, i) * Signe;
            if ( Score > BestScore )
            {
#if DEBUG_FIN_PARTIE > 1
                OutDebug("JoueFinPartie[NbCarte =%d, i=%d), Nouveau meilleur Score = %d\n", CurrentGame->NbCarteJoueur[CurrentGame->JoueurCourant], i, Score);
#endif // DEBUG_FIN_PARTIE
                BestScore = Score;
            }
        }
        free(locGame);
        //  Doit retourner score pour attaque.
        //  Si Signe = 1 et isAttaque = 1, conserve BestScore
        //  Si Signe = -1 et isAttaque = 1, inverse Bestscore
        //  Si Signe = -1 et isAttaque = -1, conserve BestScore
        //  Si Signe = 1 et isAttaque = -1, conserve BestScore
        if ( Signe < 0 && isAttaque > 0 ) BestScore = -BestScore;
#if DEBUG_FIN_PARTIE > 1
        OutDebug("::::::::::::::::Sortie JoueFinPartie(NbCarte=%d), isAttaque=%d, Score = %d\n", CurrentGame->NbCarteJoueur[CurrentGame->JoueurCourant], isAttaque, BestScore);
#endif // DEBUG_FIN_PARTIE
        return BestScore;
    }
    free(locGame);
    ComptePointsFinPartie(CurrentGame, 1);
#if DEBUG_FIN_PARTIE > 1
    OutDebug("::::::::::::::::Sortie JoueFinPartie, Score = %d\n", (CurrentGame->PointsAttaque - CurrentGame->PointsDefense));
#endif // DEBUG_FIN_PARTIE
    //  Retourne points pour Attaque
    return(CurrentGame->PointsAttaque - CurrentGame->PointsDefense);
}

//  Affiche la distribution générée pour Debug

void AfficheDistributionGeneree(TarotGame CurrentGame, int nDistrib)
{
int i, j;
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[CurrentGame->JoueurCourant];
char str[16];

    OutDebug("Distribution trouvée (%d): ");
    for ( i = 0; i < 78; i++ )
    {
        if ( CurrentGame->CarteJouee[i] >= 0 ) continue;
        for ( j = 0; j <= CHIEN; j++ )
        {
            if ( pJeu->ProbCarte[j][i] < 0.000001 ) continue;
            if ( pJeu->ProbCarte[j][i] > 0.999999)
            {
                OutDebug("%s --> %d ", strNomCarte(str, i), j);
            }
            else
            {
                OutDebug("Problème, Probcarte[%d][%s] avec probabilité %.3f\n", j, strNomCarte(str, i), pJeu->ProbCarte[j][i]);
            }
        }
    }
    OutDebug("\n");
}

//  Génère la prochaine distribution possible avec les probabilités courantes
//  La proba de la distribution est calculée
//  Cette fonction est récursive, elle sort quand toutes les cartes ont été attribuées
//  Pour cela, crée un nouveau contexte de jeu chaque fois qu'une carte est choisie (soit avec proba à 0 ou à 1).
//  Pour gagner du temps, commence la recherche au joueur Joueur et à la carte Index
//  Retourne la carte la meilleure pour le moment si le score est meilleur lastBestcore

int GenereDistrib(TarotGame CurrentGame, int *Joueur, int *Index, int Level, int LastBest, int *LastBestScore)
{
struct _Jeu *pJeu = &CurrentGame->JeuJoueur[CurrentGame->JoueurCourant];
struct _Jeu *pTmpJeu;
int i, j;
int StartIndex = *Index;
TarotGame tmpGame = NULL;
int ChangeCarte = 0;
static int nDistrib;
static double SommeProbaDistrib;
static double Score[16];
int res;
int iBest;
double BestScore;
#if DEBUG_FIN_PARTIE > 0
char str[16];
#endif // DEBUG_FIN_PARTIE

#if DEBUG_FIN_PARTIE > 1
    OutDebug("Entrée GenereDistrib Level = %d, Joueur = %d, Index=%d (%s)\n", Level, *Joueur, *Index, strNomCarte(str, *Index));
    if ( Level == 6 )
        i = 0;
#endif // DEBUG_FIN_PARTIE
    if ( Level == 0 )
    {
        nDistrib = 0;     //  Initialise pour niveau 0
        SommeProbaDistrib = 0;
        for ( i = 0; i < 16; i++ )
            Score[i] = 0;
    }
    //  Parcourt toutes les cartes jusqu'à trouver une avec une proba entre 0 et 1 (bornes exclues)
    for ( j = *Joueur; j < MAX_JOUEURS; j++ )
    {
        if ( j == CurrentGame->JoueurCourant ) continue;        //  Passe au suivant si joueur courant
#if DEBUG > 0
        if ( nDistrib == 1490 && j == 3)
            i = 0;
#endif // DEBUG
        for ( i = StartIndex; i < 78; i++ )
        {
            if ( pJeu->ProbCarte[j][i] < 0.000001 ) continue;   //  Saute les cartes avec proba trop faibles (sûr de ne pas avoir)
            if ( pJeu->ProbCarte[j][i] > 0.999999 ) continue;   //  Idem, saute les cartes certaines
            //  Trouvé une carte qui peut être fixée à 0 ou 1
            if ( tmpGame == NULL )
                tmpGame = (TarotGame) malloc(sizeof(struct _Tarot_Partie));     //  Alloue la mémoire pour la structure temporaire
            ChangeCarte = 1;
            //  Commence avec 0
            *tmpGame = *CurrentGame;            //  Recopie structure courante
            tmpGame->ProbaDistrib *= (1.0 - pJeu->ProbCarte[j][i]);
#if DEBUG_FIN_PARTIE > 1
            OutDebug("  GenereDistrib(%d) ProbaCarte[%d][%s] = %.3f --> Force à 0, prob Distrib = %.5f\n", Level, j, strNomCarte(str, i), pJeu->ProbCarte[j][i], tmpGame->ProbaDistrib);
#endif // DEBUG_FIN_PARTIE
            pTmpJeu =  &tmpGame->JeuJoueur[CurrentGame->JoueurCourant];
            BaisseProba(tmpGame, tmpGame->JoueurCourant, j, i, 1.0);        //  Force proba à 0, recalcule les autres probas
            NormaliseProba(tmpGame, pTmpJeu);
            Attract_Proba(tmpGame, pTmpJeu);
            //  Ensuite continue avec cette carte fixée à 0
            iBest = GenereDistrib(tmpGame, &j, &i, Level+1, LastBest, LastBestScore);
            //  Puis avec 1
            *tmpGame = *CurrentGame;            //  Recopie structure courante
            tmpGame->ProbaDistrib *= pJeu->ProbCarte[j][i];
            pTmpJeu =  &tmpGame->JeuJoueur[CurrentGame->JoueurCourant];
            tmpGame->Carte2Joueur[i] = j;
#if DEBUG_FIN_PARTIE > 1
            OutDebug("  GenereDistrib(%d) ProbaCarte[%d][%s] = %.3f --> Force à 1, prob Distrib = %.5f\n", Level, j, strNomCarte(str, i), pJeu->ProbCarte[j][i], tmpGame->ProbaDistrib);
#endif // DEBUG_FIN_PARTIE
            SetCarte2Position(pTmpJeu, j, i);       //  Fixe la carte i au joueur j
            RenormaliseDescendant(pTmpJeu, j, CurrentGame->NbCarteJoueur[j]);          //  On a monté la proba du Joueur, normalise donc vers le bas (sa somme est trop grande).
            NormaliseProba(tmpGame, pTmpJeu);
            Attract_Proba(tmpGame, pTmpJeu);
            //  Ensuite continue avec cette carte fixée à 1 maintenant
            iBest = GenereDistrib(tmpGame, &j, &i, Level+1, iBest, LastBestScore);
            if ( tmpGame != NULL ) free(tmpGame);
            return iBest;
        }
        StartIndex = 0;         //  Pour les joueurs suivants, part à 0
    }
    if ( !ChangeCarte )
    {
        //  Distribution OK sans avoir rien changé !
        nDistrib++;
        SommeProbaDistrib += CurrentGame->ProbaDistrib;
#if DEBUG_FIN_PARTIE > 0
        OutDebug("##########  GenereDistrib, Distrib %d avec probabilité %.5f, Cumul = %.3f  ########\n", nDistrib, CurrentGame->ProbaDistrib, SommeProbaDistrib);
        AfficheDistributionGeneree(CurrentGame, nDistrib);
        if ( CurrentGame->ProbaDistrib < PROB_MIN_DISTRIB  )
        {
            OutDebug("Saute cette distribution\n");
        }
#endif // DEBUG_FIN_PARTIE
        if ( CurrentGame->ProbaDistrib < PROB_MIN_DISTRIB )
        {
            if ( tmpGame != NULL ) free(tmpGame);
            return LastBest;
        }
        if ( tmpGame == NULL )
            tmpGame = (TarotGame) malloc(sizeof(struct _Tarot_Partie));     //  Alloue la mémoire pour la structure temporaire
        CopieProba2Tmp(pJeu);                           //  Recopie probas temporaires comme les probas ont changé
        PrepareStructJoueur(CurrentGame);               //  Crée des structures joueurs OK pour les autres joueurs.
        //  Essaie chaque carte
        for ( i = 0; i < pJeu->NbCarte; i++ )
        {
            *tmpGame = *CurrentGame;                    //  Copie structure jeu qui va être modifiée à chaque carte
#if DEBUG_FIN_PARTIE > 0
            OutDebug("Essai de la carte %d, %s\n", i, strNomCarte(str, pJeu->MyCarte[i].Index));
#endif // DEBUG_FIN_PARTIE
            res = JoueFinPartie(tmpGame, i);            //  Joue la carte index i pour la fin de partie, récupère score
            Score[i] += res * CurrentGame->ProbaDistrib;
                //  Copie structure jeu qui va être modifiée à chaque carte
#if DEBUG_FIN_PARTIE > 0
            OutDebug("Fin essai de la carte %d, %s, score =%d, Cumul %.3f\n", i, strNomCarte(str, pJeu->MyCarte[i].Index), res, Score[i]);
            OutDebug("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
#endif // DEBUG_FIN_PARTIE
        }
        iBest = -1;
        BestScore = -10000;
        //  Cherche la carte avec le meilleur score
        for ( i = 0; i < pJeu->NbCarte; i++ )
        {
            if ( Score[i] > BestScore )
            {
                BestScore = Score[i];
                iBest = i;
            }
        }
        //  Retourne la meilleure carte
        if ( tmpGame != NULL ) free(tmpGame);
        if ( BestScore > *LastBestScore )
        {
            *LastBestScore = BestScore;
            return iBest;
        }
        return LastBest;        //  Pas mieux que l'appel précédent, ne change rien.
    }
    assert(0);
    if ( tmpGame != NULL ) free(tmpGame);
    return -1;      //  Ne devrait Jamais venir ici
}

//  Génère toutes les distributions possibles à la fin de la partie chez les autres joueurs, chacune avec une probabilité
//  Pour chaque distribution joue toutes les cartes du joueur, à la fin calcule la carte avec le meilleur score
//  Les scores sont pondérés par les probabilités.
//  Retourne la carte choisie ou -1 si pas de carte choisie

int FinPartie(TarotGame CurrentGame)
{
TarotGame locGame;      //  Sert à recopier la partie initiale à chaque distribution
int JoueurDebut;
int IndexDebut = 0;
int Res;
int LastBest = -100000;
double nDis = nbDistrib(CurrentGame, &CurrentGame->JeuJoueur[CurrentGame->JoueurCourant]);

#if DEBUG_FIN_PARTIE > 0
    OutDebug("$$$$$$$$$$$$$$$$   Entrée Fin de partie, nbDistrib = %.0f\n", nDis);
#endif // DEBUG_FIN_PARTIE
    //  Alloue une structure jeu pour ne pas perturber la structure globale
    locGame = (TarotGame) malloc(sizeof(struct _Tarot_Partie));     //  Alloue la mémoire pour la structure temporaire
    *locGame = *CurrentGame;
    if ( CurrentGame->JoueurCourant != SUD )
        JoueurDebut = SUD;          //  Commence au joueur SUD si pas SUD
    else
        JoueurDebut = EST;
    locGame->ProbaDistrib = 1.0;        //  Initialise à 1, sera baissée en cours de calcul
    Res = GenereDistrib(locGame, &JoueurDebut, &IndexDebut, 0,-1,  &LastBest);
    free(locGame);
    return Res;
}

