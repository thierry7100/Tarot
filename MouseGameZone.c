#include <gtk/gtk.h>
#include "Tarot_Ui_Objects.h"
#include "Tarot_Game.h"
#include <math.h>

int NbBoutons;
struct BoutonTarot BoutonsTarot[MAX_BOUTONS];

extern GtkCheckMenuItem *ModePartieEnregistreeAttaque;    //  Si choisi, joue des parties enregistrées en attaque
extern GtkCheckMenuItem *ModePartieEnregistreeDefense;    //  Si choisi, joue des parties enregistrées en attaque


//  Retourne la distance min d'un des coins du bouton
static double distanceCoinBouton(double x, double y, struct BoutonTarot *pBouton)
{
double d1, d2, d3, d4, dmin;

    //  En haut à gauche
    d1 = (pBouton->Xmin - x)*(pBouton->Xmin - x) + (pBouton->Ymin - y)*(pBouton->Ymin - y);
    dmin = d1;
    //  En haut à doite
    d2 = (pBouton->Xmax - x)*(pBouton->Xmax - x) + (pBouton->Ymin - y)*(pBouton->Ymin - y);
    if ( d2 < dmin ) dmin = d2;
    //  En bas à gauche
    d3 = (pBouton->Xmin - x)*(pBouton->Xmin - x) + (pBouton->Ymax - y)*(pBouton->Ymax - y);
    if ( d3 < dmin ) dmin = d3;
    //  En bas à droite
    d4 = (pBouton->Xmax - x)*(pBouton->Xmax - x) + (pBouton->Ymax - y)*(pBouton->Ymax - y);
    if ( d4 < dmin ) dmin = d4;
    return(sqrt(dmin));
}

//  Si la souris est sur un bouton, retourne l'index de celui-ci, sinon -1

static int LookForBouton(double x, double y)
{
int i;

    for ( i = 0; i < NbBoutons; i++)
    {
        if ( x >= BoutonsTarot[i].Xmin && x <= BoutonsTarot[i].Xmax && y >= BoutonsTarot[i].Ymin && y <= BoutonsTarot[i].Ymax )
        {
            //  Dans la zone...
            //  Verifie maintenant pas trop dans l'arrondi
            if ( distanceCoinBouton(x, y, &BoutonsTarot[i]) > BoutonsTarot[i].radius )
                return i;
        }
    }
    return -1;      //  Pas trouvé...
}
/*
 *  Gestion des évenements "Appui bouton de souris"
 *  Permet de savoir la carte choisie.
 *  The ::button-press signal handler receives a GdkEventButton struct which contains this information.
 */

gboolean ButtonPressGameZone(GtkWidget *widget, GdkEventButton *event, TarotGame CurrentGame)
{
double x, y;
int indexCarte, indexBouton;
extern double RecouvrementJeu;


    printf("Appui Souris bouton %d, X=%.1f, Y=%.1f\n", event->button, event->x, event->y);
    if ( event->button != 1 )   //  Nothing to do, only handle left button click
        return TRUE;
    //  Regarde si la souris est dans la zone des cartes du joueur.
    x = event->x;
    y = event->y;
    if ( CurrentGame->StateJeu == JEU_FIN_PLI ) //  Clic pendant attente vision pli, passe de suite au pli suivant
    {
        CurrentGame->DecompteFinPli = 0;
        CurrentGame->StateJeu = JEU_EN_COURS;         //  Passe au second pli (ou plus)
        CurrentGame->StateAfterMessage = JEU_EN_COURS;//  Ne met pas de suite l'état attente jeu SUD, il faudra attendre un tick avant de pouvoir jouer
        RamassePli(CurrentGame, 0);                      //  Ramasse le pli en cours, prépare pli suivant
        AffichagePoints(CurrentGame);
        if ( CurrentGame->NumPli == 18 )
        {
            ComptePointsFinPartie(CurrentGame, 0);     //
            CurrentGame->StateJeu = JEU_TERMINE;
            CurrentGame->StateAfterMessage = JEU_TERMINE;
            gtk_widget_queue_draw(GameZoneArea);    //  Force réaffichage pour affichage pli ramassé
            return TRUE;
        }
        gtk_widget_queue_draw(GameZoneArea);          //  Force réaffichage pour affichage pli ramassé
        return  TRUE;
    }
    if ( x > LeftXJoueur[SUD] && x < RightXJoueur[SUD] && y > TopYJoueur[SUD] - DeltaCarteLevee && y < BottomYJoueur[SUD] )
    {
        //  Clic dans la zone des cartes du joueurs
        indexCarte = (int)((x - LeftXJoueur[SUD]) / (RecouvrementJeu * Card_Width));
        if ( indexCarte >= CurrentGame->NbCarteJoueur[SUD] )
            indexCarte = CurrentGame->NbCarteJoueur[SUD] - 1;
        if ( CurrentGame->isCarteLevee[indexCarte] && y > BottomYJoueur[SUD]-DeltaCarteLevee )
            return TRUE;
        if ( CurrentGame->isCarteLevee[indexCarte] == 0 && y < TopYJoueur[SUD] )
            return TRUE;
        printf("Clic sur carte %d\n", indexCarte);
        if ( CurrentGame->StateJeu == JEU_CHOIX_CARTES_CHIEN )
        {
            //  Monte la carte choisie
            if ( isEcartable(CurrentGame, CurrentGame->IdxCartesJoueur[SUD][indexCarte]) )
            {
                if ( CurrentGame->isCarteLevee[indexCarte] )
                {
                    CurrentGame->isCarteLevee[indexCarte] = 0;      //  La baisse
                    CurrentGame->NbCarteLevee--;
                    printf("Baisse carte %d, Nb Levee = %d\n", indexCarte, CurrentGame->NbCarteLevee);
                }
                else if  ( CurrentGame->NbCarteLevee < 6 )
                {
                    CurrentGame->isCarteLevee[indexCarte] = 1;      //  La monte
                    CurrentGame->NbCarteLevee++;
                    printf("Monte carte %d, Nb Levee = %d\n", indexCarte, CurrentGame->NbCarteLevee);
                }
                gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage, les cartes ont bougé
            }
        }
        if ( CurrentGame->StateJeu == JEU_CHOIX_CARTE_POIGNEE )
        {
            //  Monte la carte choisie, si c'est possible et si pas plus que le nombre demandé
            if ( isOKPoignee(CurrentGame, indexCarte) )
            {
                if ( CurrentGame->isCarteLevee[indexCarte] )
                {
                    CurrentGame->isCarteLevee[indexCarte] = 0;      //  La baisse
                    CurrentGame->NbCarteLevee--;
                    printf("Baisse carte %d, Nb Levee = %d\n", indexCarte, CurrentGame->NbCarteLevee);
                }
                else if  ( CurrentGame->NbCarteLevee < CurrentGame->NumAtoutPoignee[SUD] )
                {
                    CurrentGame->isCarteLevee[indexCarte] = 1;      //  La monte
                    CurrentGame->NbCarteLevee++;
                    printf("Monte carte %d, Nb Levee = %d\n", indexCarte, CurrentGame->NbCarteLevee);
                }
                gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage, les cartes ont bougé
            }
        }
        if ( CurrentGame->StateAfterMessage == JEU_ATTENTE_JEU_SUD )
        {
            if ( CarteValide(&CurrentGame->JeuJoueur[SUD], indexCarte, CurrentGame->JoueurEntame) )
            {
                //  Possible : joue cette carte
                PoseCarte(CurrentGame, indexCarte);
                BaisseCartesSUD(CurrentGame);
                //  Passe au joueur suivant
                CurrentGame->JoueurCourant++;
                CurrentGame->JoueurCourant &= 3;
                //  Teste fin de pli ?
                if ( CurrentGame->JoueurCourant == CurrentGame->JoueurEntame )
                {
                    CurrentGame->StateJeu = JEU_FIN_PLI;
                    CurrentGame->DecompteFinPli = DECOMPTE_FIN_PLI;
                }
                else
                {
                    if ( CurrentGame->NbCarteJoueur[SUD] == 17 )
                        CurrentGame->StateJeu = JEU_PREMIER_PLI;
                    else
                        CurrentGame->StateJeu = JEU_EN_COURS;
                }
                CurrentGame->StateAfterMessage = CurrentGame->StateJeu;
                gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage, les cartes ont bougé
            }
        }
    }
    if ( NbBoutons > 0 )
    {
        //  Il y a des boutons sur la table de jeu
        //  Regarde si clic au niveau d'un bouton
        indexBouton = LookForBouton(x, y);
        if ( indexBouton >= 0 )
        {
            printf("Clic sur bouton %d\n", indexBouton);
            if ( CurrentGame->StateJeu >= JEU_ATTENTE_CONTRAT_J1 && CurrentGame->StateJeu <= JEU_ATTENTE_CONTRAT_J4 &&  CurrentGame->AffChoixJoueur )
            {
                //  Attente choix joueur
                CurrentGame->AnnonceJoueur[SUD] = BoutonsTarot[indexBouton].ResponseCode;
                if ( CurrentGame->AnnonceJoueur[SUD] > CurrentGame->TypePartie )
                {
                    CurrentGame->JoueurPreneur = SUD;
                    CurrentGame->TypePartie = CurrentGame->AnnonceJoueur[SUD];
                    printf("SUD : nouveau contrat %d\n", CurrentGame->TypePartie);
                }
                CurrentGame->AffChoixJoueur = 0;
                //  Fait repartir le timer
                CurrentGame->StateJeu++;
                g_timeout_add (DurationStepContrat, NextStepContrat, NULL); //  Relance timer qui était arrêté pendant le choix du joueur
                gtk_widget_queue_draw(GameZoneArea);             //  Force réaffichage après choix contrat
            }
            else if ( CurrentGame->StateJeu == JEU_MONTRE_CHIEN )
            {
                if ( CurrentGame->JoueurPreneur == SUD )
                {
                    //  Appel choix carte chien pour preneur....
                    EffaceContratsJoueurs(CurrentGame);
                    AjouteCarteChienPreneur(CurrentGame);                  //  Ajoute les cartes du chien au preneur
                    if ( FlagSuggestionChien )
                        MakeEcart(CurrentGame);                            //  Fait une proposition de cartes à mettre au chien
                    else
                        CurrentGame->NbCarteLevee = 0;                      //  Pas encore de cartes levées
                    CurrentGame->DeltaYMessage = GameZoneArea_height * 0.08;
                    DeltaCarteLevee = Card_Height/5;
                    CurrentGame->InfoMessage = "Choisissez les cartes à écarter";
                    CurrentGame->StateAfterMessage = JEU_PREMIER_PLI;
                    CurrentGame->StateJeu = JEU_CHOIX_CARTES_CHIEN;        //  Passe au suivant... SUD doit choisir des cartes à mettre au chien..
                }
                else
                {
                    AjouteCarteChienPreneur(CurrentGame);       //  Ajoute les cartes du chien au preneur
                    MakeEcart(CurrentGame);                     //  Sélectionne les cartes à mettre au chien pour le preneur
                    MetCartesEcart(CurrentGame);                //  Les met effectivement à l'écart
                    if ( CurrentGame->NbAtoutEcart > 0 )
                    {
                        EffaceContratsJoueurs(CurrentGame);
                        CurrentGame->StateJeu = JEU_AFFICHE_ATOUT_ECART;
                    }
                    else
                    {
                        CurrentGame->StateJeu = JEU_PREMIER_PLI;
                        EffaceContratsJoueurs(CurrentGame);
                        DebutPartie(CurrentGame);
                        if ( CurrentGame->StateJeu == JEU_PREMIER_PLI )
                            g_timeout_add (DurationStepJeu, NextStepJeu, NULL); //  Lance Timer Jeu si pas de chelem
                    }
                }
                gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage après choix contrat
            }
            else if ( CurrentGame->StateJeu == JEU_AFFICHE_ATOUT_ECART )
            {
                CurrentGame->StateJeu = JEU_PREMIER_PLI;
                EffaceContratsJoueurs(CurrentGame);
                DebutPartie(CurrentGame);
                if ( CurrentGame->StateJeu == JEU_PREMIER_PLI )
                    g_timeout_add (DurationStepJeu, NextStepJeu, NULL); //  Lance Timer Jeu si pas modifié état (Chelem...)
                gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage
            }
            else if ( CurrentGame->StateJeu == JEU_AFFICHE_PLI_PRECEDENT || CurrentGame->StateJeu == JEU_AFFICHE_SCORES )
            {
                CurrentGame->StateJeu = CurrentGame->StateAfterMessage;
                if ( CurrentGame->TypeTimer == TIMER_ENCHERES )
                    g_timeout_add (DurationStepContrat, NextStepContrat, NULL); //  (Re)lance Timer encheres
                else if ( CurrentGame->TypeTimer == TIMER_PARTIE )
                    g_timeout_add (DurationStepJeu, NextStepJeu, NULL); //  (Re)lance Timer Jeu
                if ( CurrentGame->StateJeu == JEU_VIDE )
                {
                    CurrentGame->AnnonceJoueur[CurrentGame->JoueurPreneur]= -1;
                    NumDonnes = 0;                              //Nouvelle partie
                }
                gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage
                return TRUE;
            }
            else if ( CurrentGame->StateJeu == JEU_MESSAGE && CurrentGame->StateAfterMessage != JEU_ATTENTE_JEU_SUD )
            {
                CurrentGame->StateJeu = CurrentGame->StateAfterMessage;
                gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage, enlève message
            }
            else if ( CurrentGame->StateJeu == JEU_CHOIX_CARTES_CHIEN )
            {
                if ( CurrentGame->NbCarteLevee == 6 )       //  Choix terminé
                {
                    CurrentGame->StateJeu = JEU_PREMIER_PLI;
                    MetCartesEcart(CurrentGame);
                    DebutPartie(CurrentGame);
                    DeltaCarteLevee = 0;
                    BaisseCartesSUD(CurrentGame);                       //  Baisse les cartes à ce stade
                    if ( CurrentGame->StateJeu == JEU_PREMIER_PLI)
                        g_timeout_add (DurationStepJeu, NextStepJeu, NULL); //  Lance Timer Jeu si état toujours premier pli (pas de chelem)
                    gtk_widget_queue_draw(GameZoneArea);                //  Force réaffichage
                }
            }
            else if  ( CurrentGame->StateJeu == JEU_DECLARE_CHELEM )
            {
                CurrentGame->JoueurCourant = CurrentGame->JoueurPreneur;    //  Le preneur a l'entame pour un chelem
                CurrentGame->JoueurEntame = CurrentGame->JoueurPreneur;
                CurrentGame->ChelemDemande = 1;
                CurrentGame->StateJeu = JEU_PREMIER_PLI;
                CurrentGame->TypeTimer = TIMER_PARTIE;
                g_timeout_add (DurationStepJeu, NextStepJeu, NULL); //  Lance Timer Jeu pour débuter la partie
                gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage
                return TRUE;
            }
            else if ( CurrentGame->StateJeu == JEU_CHOIX_CARTE_POIGNEE )
            {
                if ( CurrentGame->NbCarteLevee == CurrentGame->NumAtoutPoignee[SUD] )       //  Choix terminé ?
                {
                    CurrentGame->StateJeu = JEU_PREMIER_PLI;
                    CurrentGame->StateAfterMessage = JEU_ATTENTE_JEU_SUD;
                    DeltaCarteLevee = 0;
                    SelectAtoutsLeveePoignee(CurrentGame);              //  Sélectionne les atouts levés dans la poignée
                    BaisseCartesSUD(CurrentGame);                       //  Baisse les cartes à ce stade
                    if ( CurrentGame->JoueurEntame != SUD )
                        LeveCartesPossiblesSUD(CurrentGame);                //  Montre les cartes possibles pour le pli en cours
                    g_timeout_add (DurationStepJeu, NextStepJeu, NULL); //  Lance Timer Jeu
                    gtk_widget_queue_draw(GameZoneArea);                //  Force réaffichage
                    return TRUE;
                }
            }
            else if ( CurrentGame->StateJeu == JEU_POSE_QUESTION_CHELEM )
            {
                if ( BoutonsTarot[indexBouton].ResponseCode == 1 )
                {
                    CurrentGame->ChelemDemande = 1;         //  Demande chelem
                    CurrentGame->JoueurCourant = CurrentGame->JoueurPreneur;    //  Le preneur a l'entame pour un chelem
                    CurrentGame->JoueurEntame = CurrentGame->JoueurPreneur;
                }
                //  Puis passe au jeu
                CurrentGame->StateJeu = JEU_PREMIER_PLI;
                CurrentGame->TypeTimer = TIMER_PARTIE;
                g_timeout_add (DurationStepJeu, NextStepJeu, NULL); //  Lance Timer Jeu pour débuter la partie
                gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage
                return TRUE;
            }
            else if ( CurrentGame->StateJeu == JEU_CHOIX_POIGNEE )
            {
                if ( BoutonsTarot[indexBouton].ResponseCode == 0 )
                {
                    //  Ne veut pas la montrer
                    CurrentGame->PoigneeMontree[SUD] = POIGNEE_NON_MONTREE;
                    CurrentGame->NumAtoutPoignee[SUD] = 0;              //  Pas de poignée
                    CurrentGame->StateJeu = JEU_PREMIER_PLI;            //  Retourne au premier pli
                    CurrentGame->StateAfterMessage = JEU_ATTENTE_JEU_SUD;
                    LeveCartesPossiblesSUD(CurrentGame);                //  Montre les cartes possibles pour le pli en cours
                    g_timeout_add (DurationStepJeu, NextStepJeu, NULL); //  Relance Timer Jeu
                    gtk_widget_queue_draw(GameZoneArea);                //  Force réaffichage
                    return TRUE;
                }
                CurrentGame->StateJeu = JEU_CHOIX_CARTE_POIGNEE;
                DeltaCarteLevee = Card_Height/5;
                CurrentGame->NumAtoutPoignee[SUD] = NbAtoutPoignee[BoutonsTarot[indexBouton].ResponseCode];
                CurrentGame->PoigneeMontree[SUD] = BoutonsTarot[indexBouton].ResponseCode + 1;
                CurrentGame->InfoMessage = "Choisissez les cartes de votre poignée";
                CurrentGame->DeltaYMessage = GameZoneArea_height * 0.08;
                MonteAtoutsPoignee(CurrentGame);
                gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage
            }
            else if ( CurrentGame->StateJeu == JEU_AFFICHE_POIGNEE )
            {
                RegardePoigneeJoueurs(CurrentGame, CurrentGame->JoueurCourant);
                CurrentGame->StateJeu = JEU_PREMIER_PLI;        //  Revient à la base...
                g_timeout_add (DurationStepJeu, NextStepJeu, NULL); //  Relance Timer Jeu
                gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage, enlève message
                return TRUE;
            }
            else if ( CurrentGame->StateJeu == JEU_TERMINE )
            {
                CurrentGame->AnnonceJoueur[CurrentGame->JoueurPreneur]= -1;
                if ( BoutonsTarot[indexBouton].ResponseCode == 1 )      //  Rejoue Partie
                {
                    NumDonnes--;                                        //  Décrémente nombre de parties pour ne pas compter deux fois
                    RejouePartie(CurrentGame);
                    gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage
                }
                else if ( BoutonsTarot[indexBouton].ResponseCode == 2 )
                {
                    if ( NumDonnes < NOMBRE_DONNES_PARTIE )
                        DistribuePartie(CurrentGame);                   //  Partie suivante
                    else
                        CurrentGame->StateJeu = JEU_FIN_PARTIE;         //  Fin de la partie
                    gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage
                }
            }
            else if ( CurrentGame->StateJeu == JEU_FIN_PARTIE )
            {
                CurrentGame->AnnonceJoueur[CurrentGame->JoueurPreneur]= -1;
                if ( BoutonsTarot[indexBouton].ResponseCode == 0 )      //  Voir les scores
                {
                    CurrentGame->StateJeu = JEU_AFFICHE_SCORES;
                    CurrentGame->StateAfterMessage = JEU_VIDE;
                    gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage
                }
                else if ( BoutonsTarot[indexBouton].ResponseCode == 1 ) //Partie suivante
                {
                    NumDonnes = 0;                                  //  Recommence une nouvelle partie
                    DistribuePartie(CurrentGame);                   //  Partie suivante
                    gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage
                }
            }
            else if ( CurrentGame->StateJeu == JEU_NOUVELLE_PARTIE )
            {
                if ( BoutonsTarot[indexBouton].ResponseCode == 0 )      //  "Non"
                {
                    CurrentGame->StateJeu = CurrentGame->StateAfterMessage;
                    if ( CurrentGame->TypeTimer == TIMER_ENCHERES )
                        g_timeout_add (DurationStepContrat, NextStepContrat, NULL); //  (Re)lance Timer encheres
                    else if ( CurrentGame->TypeTimer == TIMER_PARTIE )
                        g_timeout_add (DurationStepJeu, NextStepJeu, NULL); //  (Re)lance Timer Jeu
                    gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage
                }
                else if ( BoutonsTarot[indexBouton].ResponseCode == 1 ) //  "Oui"
                {
                    NumDonnes = 0;
                    FlagPartieEnregistreeDefense = gtk_check_menu_item_get_active(ModePartieEnregistreeDefense);
                    FlagPartieEnregistreeAttaque = gtk_check_menu_item_get_active(ModePartieEnregistreeAttaque);
                    ChangeUserConfigModeEnregistre();
                    SaveScores();
                    CurrentGame->StateJeu = JEU_VIDE;
                    DistribuePartie(CurrentGame);                   //  Partie suivante
                    gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage
                }

            }
        }
    }
    // We've handled the event, stop processing
    return TRUE;
}

/*
 *  Traite les évenements mouvements de souris. Utile seulement pour changer la couleur des boutons "demande de prise"
 *  The ::motion-notify signal handler receives a GdkEventMotion struct which contains this information.
 */

gboolean MouseMoveGameZone(GtkWidget *widget, GdkEventButton *event, TarotGame CurrentGame)
{
double x, y;
int indexBouton;

    x = event->x;
    y = event->y;
    if ( NbBoutons > 0 )
    {
        indexBouton = LookForBouton(x, y);
        if ( indexBouton >= 0 )
        {
            printf("Au dessus bouton %d\n", indexBouton);
        }
    }
    /* We've handled the event, stop processing */
    return TRUE;
}
