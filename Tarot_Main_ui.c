#include "ScreenSize.h"
#include <gtk/gtk.h>
#include "SplashScreen.h"
#include "Tarot_Ui_Objects.h"
#include <glib.h>

//  Global variables for UI, will be accessed in the game
GtkWindow *MainWindow;                  //  The top level window
GtkCheckMenuItem *JoueSignalisation;    //  If checked, play with signaling (advanced player)
GtkCheckMenuItem *ModePartieEnregistreeAttaque;    //  Si choisi, joue des parties enregistrées en attaque
GtkCheckMenuItem *ModePartieEnregistreeDefense;    //  Si choisi, joue des parties enregistrées en attaque
GtkWidget *GameZoneArea;                //  Game Zone, will use this zone to draw current game
GtkStatusbar *StatusBarGen;
GtkStatusbar *StatusBarAtout;
GtkStatusbar *StatusBarPtAtt;
GtkStatusbar *StatusBarPtDef;
int StatusBarGen_width;
GtkCssProvider *Tarot_css;
GtkDialog *DialogAffichage;
GtkDialog *DialogDistrib;
GtkDialog *DialogNomJoueurs;
GtkDialog *DialogStyleJeu;
GtkDialog *DialogChoixPartie;
GtkDialog *DialogUserPref;
GtkBuilder *builder;
GdkPixbuf *IconTarot;
GtkMenuItem * ReloadMenuItem;

static int UserInterfaceReady;

int GameZoneArea_width;
int GameZoneArea_height;
int FlagPartieEnregistreeAttaque;
int FlagPartieEnregistreeDefense;
int FlagAffichagePoints;
int FlagAffichageAtouts;
int FlagSuggestionChien;
int FlagSuggestionCarte;
int FlagSuggestionPoignee;
int PassageAutoPliSuivant;
int DelaiPliAuto;

struct _Tarot_Partie UiGame;
const int DurationStepContrat = 500;
const int DurationStepJeu = 500;

const char *strVersion = "v 0.01.04";

int InitUi(int argc, char **argv)
{
    /* Initialize GTK+ */
    g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, NULL);
    gtk_init (&argc, &argv);
    g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);
    UserInterfaceReady = FALSE;

    GetScreensSize();
    return nMonitor == 0;               //  Error if no Monitor
}

/* Close the splash screen when timer has expired */
int close_screen(gpointer data)
{
    //  Delete Splash window
    gtk_widget_destroy((GtkWidget*)data);
    //  And show the main one.
    gtk_widget_show_all (GTK_WIDGET(MainWindow));
    gtk_window_set_resizable(MainWindow, FALSE);
    return(FALSE);
}

//  Avance pendant les plis


int NextStepJeu(gpointer data)
{
int pos;
static int NbTickTimer;

    if ( UiGame.StateJeu == JEU_TERMINE )
    {
        UiGame.TypeTimer = TIMER_NO;
        return FALSE;
    }
    if ( UiGame.StateJeu < JEU_FIN_CONTRAT )                //  Pas dans la partie jeu, arrête
        return FALSE;
    if ( UiGame.StateJeu == JEU_AFFICHE_PLI_PRECEDENT || UiGame.StateJeu == JEU_AFFICHE_SCORES || UiGame.StateJeu == JEU_NOUVELLE_PARTIE )
    {
        UiGame.TypeTimer = TIMER_PARTIE;
        return FALSE;                           //  Désactive Timer, sera relancé en sortie
    }
    if ( UiGame.StateJeu == JEU_FIN_PLI )
    {
        NbTickTimer = 0;
        if ( UiGame.DecompteFinPli > 0 )
        {
            UiGame.DecompteFinPli--;
            return TRUE;                        //  Reste au même état et garde le timer
        }
        UiGame.DecompteFinPli = 0;
        UiGame.StateJeu = JEU_EN_COURS;         //  Passe au second pli (ou plus)
        RamassePli(&UiGame);                    //  Ramasse le pli en cours, prépare pli suivant
        AffichagePoints(&UiGame);
        if ( UiGame.NumPli == 18 )
        {
            ComptePointsFinPartie(&UiGame);
            UiGame.StateJeu = JEU_TERMINE;
            UiGame.StateAfterMessage = JEU_TERMINE;
            gtk_widget_queue_draw(GameZoneArea);    //  Force réaffichage pour affichage pli ramassé
            UiGame.TypeTimer = TIMER_NO;
            return FALSE;                           //  Cela désactive le timer.
        }
        else if ( UiGame.JoueurCourant == SUD )
            UiGame.StateAfterMessage = JEU_ATTENTE_JEU_SUD;     //  Pour pouvoir jouer sans attendre le timer
        gtk_widget_queue_draw(GameZoneArea);    //  Force réaffichage pour affichage pli ramassé
        return  TRUE;
    }
    //  Regarde où en est-on du pli en cours
    pos = (UiGame.JoueurCourant - UiGame.JoueurEntame)&3;     //  Joueur avec entame Delta par rapport à l'entame
    AffichageAtouts(&UiGame);
    if ( UiGame.JoueurCourant == SUD )
    {
        NbTickTimer++;
        if ( UiGame.NumPli == 0 )
        {
            //  Demande poignée
            if ( UiGame.PoigneeMontree[SUD] == 0 && OkToShowPoignee(&UiGame) )
            {
                BaisseCartesSUD(&UiGame);               //  Laisse les cartes baissées à ce stade
                UiGame.StateJeu = JEU_CHOIX_POIGNEE;
                gtk_widget_queue_draw(GameZoneArea);    //  Force réaffichage pour affichage pli ramassé
                return  FALSE;                          //  Bloque timer le temps de choisir la poignée de SUD
            }
        }
        UiGame.StateAfterMessage = JEU_ATTENTE_JEU_SUD;
        if ( NbTickTimer > LIMITE_ATTENTE_SUD && UiGame.StateJeu != JEU_MESSAGE_NO_BUTTON )
        {
            UiGame.DeltaYMessage = GameZoneArea_height * 0.05;
            UiGame.InfoMessage = "A vous de Jouer";
            UiGame.StateJeu = JEU_MESSAGE_NO_BUTTON;
            gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage pour affichage message
        }
        return TRUE;
    }
    NbTickTimer = 0;
    //  Regarde déjà si montre poignée
    if ( UiGame.NumPli == 0 )
    {
        if ( UiGame.PoigneeMontree[UiGame.JoueurCourant] == 0 && OkToShowPoignee(&UiGame) )
        {
            UiGame.StateJeu = JEU_AFFICHE_POIGNEE;
            UiGame.StateAfterMessage = JEU_PREMIER_PLI;
            UiGame.JoueurAffichePoignee = UiGame.JoueurCourant;
            gtk_widget_queue_draw(GameZoneArea);    //  Force réaffichage pour affichage pli ramassé
            UiGame.TypeTimer = TIMER_NO;
            return  FALSE;                          //  Bloque timer le temps de choisir la poignée de SUD
        }
        else
        {
            UiGame.JoueurAffichePoignee = -1;
            UiGame.StateJeu = JEU_PREMIER_PLI;
        }
        if ( UiGame.JoueurCourant == UiGame.JoueurPreneur )
        {
            InitPetitAuBout(&UiGame);
        }
    }
    //  C'est à un des joueurs (NON SUD) de jouer
    if ( pos == 0 )
    {
        if ( UiGame.JoueurCourant == UiGame.JoueurPreneur )
        {
            JoueEnPremierAttaque(&UiGame);
        }
        else
        {
            if ( UiGame.NbCarteJoueur[UiGame.JoueurCourant] == 18 )
                EntameDefense(&UiGame);
            else
                JoueEnPremierDefense(&UiGame);
        }
    }
    else if ( pos == 1 )
    {
        if ( UiGame.JoueurCourant == UiGame.JoueurPreneur )
        {
            JoueAttaqueSecond(&UiGame);
        }
        else
        {
            JoueDefenseSecond(&UiGame);
        }
    }
    else if ( pos == 2 )
    {
        if ( UiGame.JoueurCourant == UiGame.JoueurPreneur )
        {
            JoueAttaqueTroisieme(&UiGame);
        }
        else
        {
            JoueDefenseTroisieme(&UiGame);
        }
    }
    else if ( pos == 3 )
    {
        if ( UiGame.JoueurCourant == UiGame.JoueurPreneur )
        {
            JoueAttaqueDernier(&UiGame);
        }
        else
        {
            JoueDefenseDernier(&UiGame);
        }
    }
    pos++;
    if ( pos == 4 )
    {
        //  Fin du pli.
        UiGame.StateJeu = JEU_FIN_PLI;
        if ( PassageAutoPliSuivant )
            UiGame.DecompteFinPli = DelaiPliAuto;
        else
            UiGame.DecompteFinPli = 7200;               //  Passe au pli suivant après 1H !
        gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage pour affichage pli
        return TRUE;
    }
    UiGame.JoueurCourant++;                         //  Passe au suivant
    UiGame.JoueurCourant &= 3;
    if ( UiGame.JoueurCourant == SUD )
    {
        if ( UiGame.NumPli == 0 )
        {
            //  Demande poignée (une seule fois !)
            if ( UiGame.PoigneeMontree[SUD] == 0 && OkToShowPoignee(&UiGame) )
            {
                BaisseCartesSUD(&UiGame);               //  Laisse les cartes baissées à ce stade
                UiGame.StateJeu = JEU_CHOIX_POIGNEE;
                gtk_widget_queue_draw(GameZoneArea);    //  Force réaffichage pour affichage pli ramassé
                UiGame.TypeTimer = TIMER_NO;
                return  FALSE;                          //  Bloque timer le temps de choisir la poignée de SUD
            }
        }
        //  Cours "normal" du jeu
        LeveCartesPossiblesSUD(&UiGame);
        UiGame.StateJeu = JEU_ATTENTE_JEU_SUD;
        UiGame.StateAfterMessage = JEU_ATTENTE_JEU_SUD;
    }
    gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage pour affichage pli
    return TRUE;
}
//  Avance l'état des contrats par joueur
int NextStepContrat(gpointer data)
{
int Joueur;
int Contrat;

    //printf("Entrée NextStepContrat, StateJeu = %d\n", UiGame.StateJeu);
    if ( UiGame.StateJeu == JEU_MESSAGE )
    {
        return TRUE;    //  Garde timer mais ne fait rien
    }
    if ( UiGame.StateJeu == JEU_AFFICHE_SCORES || UiGame.StateJeu == JEU_NOUVELLE_PARTIE || UiGame.AffChoixJoueur )
    {
        UiGame.TypeTimer = TIMER_ENCHERES;
        return FALSE;                           //  Désactive Timer, sera relancé en sortie
    }
    if ( UiGame.StateJeu ==  JEU_FIN_CONTRAT )
    {
        if ( UiGame.TypePartie >= GARDE_SANS )              //  Ne montre pas le chien dans ce cas
        {
            UiGame.StateJeu = JEU_PREMIER_PLI;
            gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage pour affichage chien
            EffaceContratsJoueurs(&UiGame);
            DebutPartie(&UiGame);
            if ( UiGame.StateJeu == JEU_PREMIER_PLI )       //  Cela reste vrai si pas de Chelem
            {
                UiGame.TypeTimer = TIMER_PARTIE;
                g_timeout_add (DurationStepJeu, NextStepJeu, NULL); //  Lance Timer Jeu
            }
            gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage
        }
        else if ( UiGame.TypePartie > PASSE && UiGame.TypePartie < GARDE_SANS )
        {
            UiGame.StateJeu = JEU_MONTRE_CHIEN;
            gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage pour affichage chien
        }
        UiGame.TypeTimer = TIMER_NO;
        return FALSE;       //  Détruit timer
    }
    if ( UiGame.StateJeu > JEU_FIN_CONTRAT )                //  Plus dans la partie contrat, arrête
    {
        UiGame.TypeTimer = TIMER_NO;
        return FALSE;
    }
    Joueur = ((UiGame.StateJeu - JEU_ATTENTE_CONTRAT_J1) + UiGame.JoueurDistrib + 1 ) % MAX_JOUEURS;
    //printf("Entrée NextStepContrat, StateJeu = %d, Joueur = %d\n", UiGame.StateJeu, Joueur);
    UiGame.AffChoixJoueur = (Joueur == SUD);      //  Si attente choix joueur
    Contrat = ChoixContrat(&UiGame, Joueur);
    if ( Joueur != SUD )
    {
        if ( Contrat > UiGame.TypePartie )
        {
            UiGame.TypePartie = Contrat;      //  Si pas le joueur et contrat supérieur, c'est le type retenu.
//            printf("%d Contrat : %d\n", Joueur, Contrat);
            UiGame.JoueurPreneur = Joueur;
        }
        UiGame.StateJeu++;
    }
    UiGame.TypeTimer = TIMER_ENCHERES;
    gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage pour affichage contrat
    if ( UiGame.AffChoixJoueur==0 )
    {
        return TRUE;
    }
    return( FALSE );             //  Détruit timer si attente Joueur
}

void AfficheMsgStatusBarGen(const char *Message)
{
static int context_id;

    ResizeStatusBarGen();
    if ( context_id != 0 )
        gtk_statusbar_pop (GTK_STATUSBAR (StatusBarGen), context_id);
    else
        context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR (StatusBarGen), "TarotGame");
    gtk_statusbar_push (GTK_STATUSBAR (StatusBarGen), context_id, Message);
}

void AfficheMsgStatusBarAtout(const char *Message)
{
static int context_id_atout;

    if ( !FlagAffichageAtouts && context_id_atout != 0 )
    {
        gtk_statusbar_remove_all(GTK_STATUSBAR (StatusBarAtout), context_id_atout);     //  Efface tout !
        return;
    }
    if ( context_id_atout != 0 )
        gtk_statusbar_pop (GTK_STATUSBAR (StatusBarAtout), context_id_atout);     //  Efface message précédent
    else
        context_id_atout = gtk_statusbar_get_context_id(GTK_STATUSBAR (StatusBarAtout), "Atouts");
    gtk_statusbar_push (GTK_STATUSBAR (StatusBarAtout), context_id_atout, Message);
}

void AfficheMsgStatusBarPtAtt(const char *Message)
{
static int context_id_att;

    if ( !FlagAffichagePoints && context_id_att != 0 )
    {
        gtk_statusbar_remove_all(GTK_STATUSBAR (StatusBarPtAtt), context_id_att);     //  Efface tout !
        return;
    }
    if ( context_id_att != 0 )
        gtk_statusbar_pop (GTK_STATUSBAR (StatusBarPtAtt), context_id_att);     //  Efface message précédent
    else
        context_id_att = gtk_statusbar_get_context_id(GTK_STATUSBAR (StatusBarPtAtt), "PtAtt");
    gtk_statusbar_push (GTK_STATUSBAR (StatusBarPtAtt), context_id_att, Message);
}

void AfficheMsgStatusBarPtDef(const char *Message)
{
static int context_id_def;

    if ( !FlagAffichagePoints && context_id_def != 0 )
    {
        gtk_statusbar_remove_all(GTK_STATUSBAR (StatusBarPtDef), context_id_def);     //  Efface tout !
        return;
    }
    if ( context_id_def != 0 )
        gtk_statusbar_pop (GTK_STATUSBAR (StatusBarPtDef), context_id_def);     //  Efface message précédent
    else
        context_id_def = gtk_statusbar_get_context_id(GTK_STATUSBAR (StatusBarPtDef), "PtDef");
    gtk_statusbar_push (GTK_STATUSBAR (StatusBarPtDef), context_id_def, Message);
}

//  Menu Call back functions
static void MenuDistribue(GtkWidget *widget, gpointer data)
{    DistribuePartie(&UiGame);
    gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage après distribution
}


void DistribuePartie(TarotGame CurrentGame)
{
gchar *LastFileName;

    if ( NumDonnes >= NOMBRE_DONNES_PARTIE )
    {
        //  Efface et passe à la partie suivante
        NumDonnes = 0;
        memset(ResPartie, 0, sizeof(ResPartie));
    }
    if ( FlagPartieEnregistreeAttaque || FlagPartieEnregistreeDefense )
        DistribuePartieMaitre(&UiGame, 0);
    else
        DistribueJeu(&UiGame);
    LastFileName = g_build_filename(AppConfDir, "LastDistrib.tarot", NULL);
    SaveFile(LastFileName, &UiGame);
    if ( UiGame.StateJeu == JEU_ATTENTE_CONTRAT_J1) //  Lance timer si jeu en attente contrat mais pas si petit imprenable
    {
        g_timeout_add(DurationStepContrat, NextStepContrat, NULL);
        UiGame.TypeTimer = TIMER_ENCHERES;
    }
}

void RejouePartie(TarotGame CurrentGame)
{
    if ( CurrentGame->PartiePetitImprenable ) return;
    RejoueJeu(CurrentGame);
    g_timeout_add (DurationStepContrat, NextStepContrat, NULL);
    UiGame.TypeTimer = TIMER_ENCHERES;
}

//  Affiche la boîte de dialogue pour les préférences utilisateur

static void MenuUserPref(GtkWidget *widget, gpointer data)
{
int Ret;
GtkToggleButton *SuggestionChien, *SuggestionCarte, *SuggestionPoignee;
GtkToggleButton *PliSuivantAuto;
GtkSpinButton *DelaiPli;
double val;

    DelaiPli =  GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "ID_DELAI_PLI_SUIVANT"));
    gtk_spin_button_set_digits(DelaiPli, 1);
    gtk_spin_button_set_range(DelaiPli, 1.0, 10.0);
    gtk_spin_button_set_increments(DelaiPli, 0.5, 2.0);
    gtk_spin_button_set_value(DelaiPli, DelaiPliAuto*0.5);

    PliSuivantAuto =  GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "ID_PLI_SUIVANT_AUTO"));
    gtk_toggle_button_set_active(PliSuivantAuto, PassageAutoPliSuivant);

    SuggestionChien =  GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "ID_SUGGESTION_CHIEN"));
    gtk_toggle_button_set_active(SuggestionChien, FlagSuggestionChien);
    SuggestionCarte =  GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "ID_SUGGESTION_CARTE"));
    gtk_toggle_button_set_active(SuggestionCarte, FlagSuggestionCarte);
    SuggestionPoignee =  GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "ID_SUGGESTION_POIGNEE"));
    gtk_toggle_button_set_active(SuggestionPoignee, FlagSuggestionPoignee);

    Ret = gtk_dialog_run(DialogUserPref);
    gtk_widget_hide(GTK_WIDGET(DialogUserPref));
    if ( Ret == GTK_RESPONSE_OK )
    {
        //  Lecture des champs de la boîte de dialogue;
        val = gtk_spin_button_get_value(DelaiPli);
        DelaiPliAuto = (int) (val * 2.0 + 0.001);
        PassageAutoPliSuivant = gtk_toggle_button_get_active(PliSuivantAuto);
        FlagSuggestionChien = gtk_toggle_button_get_active(SuggestionChien);
        FlagSuggestionCarte = gtk_toggle_button_get_active(SuggestionCarte);
        FlagSuggestionPoignee = gtk_toggle_button_get_active(SuggestionPoignee);
        ChangeUserPref();
    }
}

static void MenuRejoue(GtkWidget *widget, gpointer data)
{
    RejouePartie(&UiGame);
    gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage après distribution
}

static void MenuPliPrecedent(GtkWidget *widget, gpointer data)
{
    //  Cas de test, génère beaucoup de parties pour faire des statistiques
    //GenereParties(&UiGame);
    if ( UiGame.NumPli == 0 ) return;
    if ( UiGame.NumPli == 18 ) return;
    if ( UiGame.StateJeu == JEU_TERMINE ) return;
    if ( UiGame.StateJeu == JEU_AFFICHE_PLI_PRECEDENT ) return;
    UiGame.StateAfterMessage = UiGame.StateJeu;
    UiGame.StateJeu = JEU_AFFICHE_PLI_PRECEDENT;
    gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage
}

static void MenuOuvrir(GtkWidget *widget, gpointer data)
{
GtkWidget *dialog;
GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
gint res;


    dialog = gtk_file_chooser_dialog_new ("Open File", MainWindow,
                                      action, "Cancel", GTK_RESPONSE_CANCEL,
                                      "Open", GTK_RESPONSE_ACCEPT,
                                      NULL);

    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        filename = gtk_file_chooser_get_filename (chooser);
        res = LoadFile(filename, &UiGame);
        g_free (filename);
        gtk_widget_destroy (dialog);
        if ( res < 0 ) return;
        //  Pour le moment
        UiGame.StateJeu = JEU_ATTENTE_CONTRAT_J1;
        gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage après distribution
        if ( UiGame.StateJeu == JEU_ATTENTE_CONTRAT_J1) //  Lance timer si jeu en attente contrat mais pas si petit imprenable
            g_timeout_add (DurationStepContrat, NextStepContrat, NULL);
    }
    else
    {
        gtk_widget_destroy (dialog);
    }
}


static void ReloadLast(GtkWidget *widget, gpointer data)
{
gchar *LastFileName;

    LastFileName = g_build_filename(AppConfDir, "LastDistrib.tarot", NULL);
    if ( LoadFile(LastFileName, &UiGame) < 0 ) return;
    //  Pour le moment
    UiGame.StateJeu = JEU_ATTENTE_CONTRAT_J1;
    gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage après distribution
    if ( UiGame.StateJeu == JEU_ATTENTE_CONTRAT_J1) //  Lance timer si jeu en attente contrat mais pas si petit imprenable
        g_timeout_add (DurationStepContrat, NextStepContrat, NULL);
}

static void MenuSauver(GtkWidget *widget, gpointer data)
{
GtkWidget *dialog;
GtkFileChooser *chooser;
GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
gint res;

    dialog = gtk_file_chooser_dialog_new ("Save File as", MainWindow,
                                      action, "Cancel", GTK_RESPONSE_CANCEL,
                                      "Save", GTK_RESPONSE_ACCEPT,
                                      NULL);
    chooser = GTK_FILE_CHOOSER (dialog);
    gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);
    gtk_file_chooser_set_current_name (chooser, "PartieSansNom.tarot");
    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
    char *filename;

        filename = gtk_file_chooser_get_filename (chooser);
        SaveFile (filename, &UiGame);
        g_free (filename);
    }

    gtk_widget_destroy (dialog);

}

static void MenuScoreAffiche(GtkWidget *widget, gpointer data)
{
    UiGame.StateAfterMessage = UiGame.StateJeu;
    UiGame.StateJeu = JEU_AFFICHE_SCORES;
    gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage
}

const char *ID_style[NB_ENTREE_STYLE_JEU] = {"LONGUE", "HONNEUR", "PRISE_DERNIER", "RISQUE", "BOUTS", "ATOUTS",
                            "CHASSE_ATT", "POINTS_ATT", "PETIT", "CHASSE_DEF", "POINTS_DEF", "DEFAUSSE"};
const char *NameID[4] = {"VOID", "EST", "NORD", "OUEST"};

GtkRange *StyleRange[MAX_JOUEURS][NB_ENTREE_STYLE_JEU];
//  Initialise un slider pour le style de jeu
static void InitSliderStyle(int Joueur, int idx, double InitialValue)
{
char strID[64];
GtkRange *lRangeStyle;
GtkScale *lScaleRange;
double val;
    snprintf(strID, 60,"STYLE_%s_%s", NameID[Joueur], ID_style[idx]);

    lRangeStyle = GTK_RANGE(gtk_builder_get_object(builder, strID));
    if ( lRangeStyle == NULL )
    {
        printf("Error, no range match for %s\n", strID);
        return;
    }
    gtk_range_set_range(lRangeStyle, -0.5, 0.5);    //  Entre -0.5 et +0.5
    gtk_range_set_increments(lRangeStyle, 0.1, 0.3);
    gtk_range_set_round_digits(lRangeStyle, 2);     //  2 digit precision
    gtk_range_set_value(lRangeStyle, InitialValue);
    StyleRange[Joueur][idx] = lRangeStyle;

    lScaleRange = GTK_SCALE(gtk_builder_get_object(builder, strID));
    // Draw ticks
    for ( val = -0.5; val <= 0.5; val += 0.1)
    {
        gtk_scale_add_mark(lScaleRange, val, GTK_POS_BOTTOM, NULL);
    }
}

//  Met les valeurs pour les sliders style jeu du joueur Joueur

static void PositionSlideStyle(int Joueur)
{
int i;

    for ( i = 0;i < NB_ENTREE_STYLE_JEU; i++)
    {
        if ( StyleRange[Joueur][i] == NULL )
            InitSliderStyle(Joueur, i, StyleJoueur[Joueur][i]);
        else
            gtk_range_set_value(StyleRange[Joueur][i], StyleJoueur[Joueur][i]);
    }
}

static void ReadSliderValues(int Joueur)
{
int i;

    for ( i = 0; i < NB_ENTREE_STYLE_JEU; i++)
    {
        StyleJoueur[Joueur][i] = gtk_range_get_value(StyleRange[Joueur][i]);
    }
}

static void ResetSliderValues(int Joueur)
{
int i;

    for ( i = 0; i < NB_ENTREE_STYLE_JEU; i++)
    {
        StyleJoueur[Joueur][i] = 0.0;
    }
}

void ResetSliderEst(GtkWidget *widget, gpointer data)
{
    ResetSliderValues(EST);
    PositionSlideStyle(EST);
}

void ResetSliderOuest(GtkWidget *widget, gpointer data)
{
    ResetSliderValues(OUEST);
    PositionSlideStyle(OUEST);
}

void ResetSliderNord(GtkWidget *widget, gpointer data)
{
    ResetSliderValues(NORD);
    PositionSlideStyle(NORD);
}

static void MenuOptionStyleJeu(GtkWidget *widget, gpointer data)
{
int Ret;
int j;
static GtkButton *ButtonResetOuest, *ButtonResetEst, *ButtonResetNord;

    //  Positionne les sliders pour les 3 joueurs
    for ( j = EST; j < MAX_JOUEURS; j++)
    {
        PositionSlideStyle(j);
    }
    if ( ButtonResetOuest == NULL )
    {
        ButtonResetOuest = GTK_BUTTON(gtk_builder_get_object(builder, "ID_RESET_OUEST"));
        g_signal_connect(ButtonResetOuest, "clicked", G_CALLBACK (ResetSliderOuest), NULL);
    }
    if ( ButtonResetEst == NULL )
    {
        ButtonResetEst = GTK_BUTTON(gtk_builder_get_object(builder, "ID_RESET_EST"));
        g_signal_connect(ButtonResetEst, "clicked", G_CALLBACK (ResetSliderEst), NULL);
    }
    if ( ButtonResetNord == NULL )
    {
        ButtonResetNord = GTK_BUTTON(gtk_builder_get_object(builder, "ID_RESET_NORD"));
        g_signal_connect(ButtonResetNord, "clicked", G_CALLBACK (ResetSliderNord), NULL);
    }
    Ret = gtk_dialog_run(DialogStyleJeu);
    if ( Ret == GTK_RESPONSE_OK )
    {
        for ( j = EST; j < MAX_JOUEURS; j++)
        {
            ReadSliderValues(j);
        }
        SaveStyleJoueurs();
    }
    else if ( Ret == -2)
    {
        //  Réponse Reset ALL
        for ( j = EST; j < MAX_JOUEURS; j++)
        {
            ResetSliderValues(j);
        }
        SaveStyleJoueurs();
    }
    gtk_widget_hide(GTK_WIDGET(DialogStyleJeu));
}

static void MenuOptionAffichage(GtkWidget *widget, gpointer data)
{
int Ret;
GtkToggleButton *button_nord, *button_est, *button_ouest;
GtkToggleButton *button_atouts, *button_points;

    //  Met les éléments graphiques en concordance avec les variables
    button_nord =  GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "AFFICHE_NORD"));
    gtk_toggle_button_set_active(button_nord, AfficheJeuJoueurs[NORD]);
    button_est =  GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "AFFICHE_EST"));
    gtk_toggle_button_set_active(button_est, AfficheJeuJoueurs[EST]);
    button_ouest =  GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "AFFICHE_OUEST"));
    gtk_toggle_button_set_active(button_ouest, AfficheJeuJoueurs[OUEST]);
    button_points =  GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "AFFICHE_POINTS"));
    gtk_toggle_button_set_active(button_points, FlagAffichagePoints);
    button_atouts =  GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "AFFICHE_ATOUTS"));
    gtk_toggle_button_set_active(button_atouts, FlagAffichageAtouts);
    Ret = gtk_dialog_run(DialogAffichage);
    gtk_widget_hide(GTK_WIDGET(DialogAffichage));
    if ( Ret == GTK_RESPONSE_OK )
    {
        //  Lecture des champs de la boîte de dialogue
        FlagAffichageAtouts = gtk_toggle_button_get_active(button_atouts);
        FlagAffichagePoints = gtk_toggle_button_get_active(button_points);
        AfficheJeuJoueurs[NORD] = gtk_toggle_button_get_active(button_nord);
        AfficheJeuJoueurs[EST] = gtk_toggle_button_get_active(button_est);
        AfficheJeuJoueurs[OUEST] = gtk_toggle_button_get_active(button_ouest);
        UiGame.FlagAfficheJoueur[NORD] = AfficheJeuJoueurs[NORD];
        UiGame.FlagAfficheJoueur[EST] = AfficheJeuJoueurs[EST];
        UiGame.FlagAfficheJoueur[OUEST] = AfficheJeuJoueurs[OUEST];
        ChangeUserConfigAffichage();
    }
}

static void MenuOptionDistribution(GtkWidget *widget, gpointer data)
{
int Ret;
GtkEntry *EntryMinBout, *EntryMinAtout, *EntryMinRoi;
GtkEntry *EntryMinTrefle, *EntryMinCarreau, *EntryMinCoeur, *EntryMinPique;
GtkToggleButton *TypeSans, *TypeDefense, *TypePetite, *TypeGarde, *TypeGardeSans, *TypeGardeContre;
char strTemp[128];
int val;

    //  Met les éléments graphiques en concordance avec les variables

    EntryMinBout =  GTK_ENTRY(gtk_builder_get_object(builder, "DISTRIB_MIN_BOUTS"));
    snprintf(strTemp, 16, "%d", ContraintesDistribution.MinBout);
    gtk_entry_set_text(EntryMinBout, strTemp);

    EntryMinAtout =  GTK_ENTRY(gtk_builder_get_object(builder, "DISTRIB_MIN_ATOUTS"));
    snprintf(strTemp, 16, "%d", ContraintesDistribution.MinAtout);
    gtk_entry_set_text(EntryMinAtout, strTemp);

    EntryMinRoi =  GTK_ENTRY(gtk_builder_get_object(builder, "DISTRIB_MIN_ROIS"));
    snprintf(strTemp, 16, "%d", ContraintesDistribution.MinRoi);
    gtk_entry_set_text(EntryMinRoi, strTemp);

    EntryMinTrefle =  GTK_ENTRY(gtk_builder_get_object(builder, "DISTRIB_MIN_TREFLE"));
    snprintf(strTemp, 16, "%d", ContraintesDistribution.MinTrefle);
    gtk_entry_set_text(EntryMinTrefle, strTemp);

    EntryMinCarreau =  GTK_ENTRY(gtk_builder_get_object(builder, "DISTRIB_MIN_CARREAU"));
    snprintf(strTemp, 16, "%d", ContraintesDistribution.MinCarreau);
    gtk_entry_set_text(EntryMinCarreau, strTemp);

    EntryMinCoeur =  GTK_ENTRY(gtk_builder_get_object(builder, "DISTRIB_MIN_COEUR"));
    snprintf(strTemp, 16, "%d", ContraintesDistribution.MinCoeur);
    gtk_entry_set_text(EntryMinCoeur, strTemp);

    EntryMinPique =  GTK_ENTRY(gtk_builder_get_object(builder, "DISTRIB_MIN_PIQUE"));
    snprintf(strTemp, 16, "%d", ContraintesDistribution.MinPique);
    gtk_entry_set_text(EntryMinPique, strTemp);

    TypeSans = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "DISTRIB_SANS"));
    TypeDefense = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "DISTRIB_DEFENSE"));
    TypePetite = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "DISTRIB_PETITE"));
    TypeGarde = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "DISTRIB_GARDE"));
    TypeGardeSans = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "DISTRIB_GARDE_SANS"));
    TypeGardeContre = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "DISTRIB_GARDE_CONTRE"));
    switch ( ContraintesDistribution.JeuFixeContrat )
    {
    case 0:
        gtk_toggle_button_set_active(TypeDefense, 1);
        break;
    case PETITE:
        gtk_toggle_button_set_active(TypePetite, 1);
        break;
    case GARDE:
        gtk_toggle_button_set_active(TypeGarde, 1);
        break;
    case GARDE_SANS:
        gtk_toggle_button_set_active(TypeGardeSans, 1);
        break;
    case GARDE_CONTRE:
        gtk_toggle_button_set_active(TypeGardeContre, 1);
        break;
    default:
        gtk_toggle_button_set_active(TypeSans, 1);
        break;
    }
    /*
    gtk_toggle_button_set_active(TypeSans, ContraintesDistribution.JeuFixeContrat < 0);
    gtk_toggle_button_set_active(TypeDefense, ContraintesDistribution.JeuFixeContrat == 0);
    gtk_toggle_button_set_active(TypePetite, ContraintesDistribution.JeuFixeContrat == PETITE);
    gtk_toggle_button_set_active(TypeGarde, ContraintesDistribution.JeuFixeContrat == GARDE);
    gtk_toggle_button_set_active(TypeGardeSans, ContraintesDistribution.JeuFixeContrat == GARDE_SANS);
    gtk_toggle_button_set_active(TypeGardeContre, ContraintesDistribution.JeuFixeContrat == GARDE_CONTRE);
    */
    Ret = gtk_dialog_run(DialogDistrib);
    gtk_widget_hide(GTK_WIDGET(DialogDistrib));
    if ( Ret == GTK_RESPONSE_OK )
    {
        //  Lecture des champs de la boîte de dialogue
        val = 0;
        sscanf(gtk_entry_get_text(EntryMinBout), "%d", &val);
        if ( val >= 0 && val <= 3 )
            ContraintesDistribution.MinBout = val;

        val = 0;
        sscanf(gtk_entry_get_text(EntryMinAtout), "%d", &val);
        if ( val >= 0 && val <= 18 )
            ContraintesDistribution.MinAtout = val;

        val = 0;
        sscanf(gtk_entry_get_text(EntryMinRoi), "%d", &val);
        if ( val >= 0 && val <= 4 )
            ContraintesDistribution.MinRoi = val;

        val = 0;
        sscanf(gtk_entry_get_text(EntryMinTrefle), "%d", &val);
        if ( val >= 0 && val <= 14 )
            ContraintesDistribution.MinTrefle = val;

        val = 0;
        sscanf(gtk_entry_get_text(EntryMinCarreau), "%d", &val);
        if ( val >= 0 && val <= 14 )
            ContraintesDistribution.MinCarreau = val;

        val = 0;
        sscanf(gtk_entry_get_text(EntryMinCoeur), "%d", &val);
        if ( val >= 0 && val <= 14 )
            ContraintesDistribution.MinCoeur = val;

        val = 0;
        sscanf(gtk_entry_get_text(EntryMinPique), "%d", &val);
        if ( val >= 0 && val <= 14 )
            ContraintesDistribution.MinPique = val;

        val = gtk_toggle_button_get_active(TypeSans);
        if ( val ) ContraintesDistribution.JeuFixeContrat = -1;

        val = gtk_toggle_button_get_active(TypeDefense);
        if ( val ) ContraintesDistribution.JeuFixeContrat = 0;

        val = gtk_toggle_button_get_active(TypePetite);
        if ( val ) ContraintesDistribution.JeuFixeContrat = PETITE;

        val = gtk_toggle_button_get_active(TypeGarde);
        if ( val ) ContraintesDistribution.JeuFixeContrat = GARDE;

        val = gtk_toggle_button_get_active(TypeGardeSans);
        if ( val ) ContraintesDistribution.JeuFixeContrat = GARDE_SANS;

        val = gtk_toggle_button_get_active(TypeGardeContre);
        if ( val ) ContraintesDistribution.JeuFixeContrat = GARDE_CONTRE;

        ChangeContraintesDistribution();
    }
}

static void MenuOptionNomsJoueurs(GtkWidget *widget, gpointer data)
{
int Ret;
GtkEntry *EntrySUD, *EntryEST, *EntryOUEST, *EntryNORD;

    //  Met les éléments graphiques en concordance avec les variables

    EntrySUD =  GTK_ENTRY(gtk_builder_get_object(builder, "NOM_SUD"));
    gtk_entry_set_text(EntrySUD, NomJoueurs[SUD]);

    EntryNORD =  GTK_ENTRY(gtk_builder_get_object(builder, "NOM_NORD"));
    gtk_entry_set_text(EntryNORD, NomJoueurs[NORD]);

    EntryEST =  GTK_ENTRY(gtk_builder_get_object(builder, "NOM_EST"));
    gtk_entry_set_text(EntryEST, NomJoueurs[EST]);

    EntryOUEST =  GTK_ENTRY(gtk_builder_get_object(builder, "NOM_OUEST"));
    gtk_entry_set_text(EntryOUEST, NomJoueurs[OUEST]);

    Ret = gtk_dialog_run(DialogNomJoueurs);
    gtk_widget_hide(GTK_WIDGET(DialogNomJoueurs));
    if ( Ret == GTK_RESPONSE_OK )
    {
        //  Lecture des champs de la boîte de dialogue;
        NomJoueurs[SUD] = (char *) gtk_entry_get_text(EntrySUD);
        NomJoueurs[NORD] = (char *) gtk_entry_get_text(EntryNORD);
        NomJoueurs[EST] = (char *) gtk_entry_get_text(EntryEST);
        NomJoueurs[OUEST] = (char *) gtk_entry_get_text(EntryOUEST);
        ChangeNomsJoueurs();
    }
}

static void MenuChoixPartie(GtkWidget *widget, gpointer data)
{
int Ret;
GtkEntry *NumPartie;
GtkComboBox *ChoixPreneur;
char strNumPartie[64];
int Num;
gchar *LastFileName;

    NumPartie =  GTK_ENTRY(gtk_builder_get_object(builder, "ID_CHOIX_PARTIE_NUM"));
    ChoixPreneur =  GTK_COMBO_BOX(gtk_builder_get_object(builder, "ID_CHOIX_PARTIE_PRENEUR"));

    //  Met les éléments graphiques en concordance avec les variables
    if ( UiGame.NumPartieEnregistree > 0 )
    {
        snprintf(strNumPartie, 60, "%d", UiGame.NumPartieEnregistree+1);
        gtk_combo_box_set_active(ChoixPreneur, UiGame.PreneurPartieEnregistree);
    }
    else
    {
        snprintf(strNumPartie, 60, "%d", rand()%126 + 1);
        gtk_combo_box_set_active(ChoixPreneur, rand()%4);
    }
    gtk_entry_set_text(NumPartie, strNumPartie);

    Ret = gtk_dialog_run(DialogChoixPartie);
    gtk_widget_hide(GTK_WIDGET(DialogChoixPartie));
    if ( Ret == GTK_RESPONSE_OK )
    {
        //  Lecture des champs de la boîte de dialogue;
        Num = 0;
        sscanf(gtk_entry_get_text(NumPartie), "%d", &Num);
        if (Num <= 0 || Num >= 126)
            Num = rand()%126;
        UiGame.NumPartieEnregistree = Num - 1;
        UiGame.PreneurPartieEnregistree = gtk_combo_box_get_active(ChoixPreneur);
        DistribuePartieMaitre(&UiGame, 1);
        LastFileName = g_build_filename(AppConfDir, "LastDistrib.tarot", NULL);
        SaveFile(LastFileName, &UiGame);
        if ( UiGame.StateJeu == JEU_ATTENTE_CONTRAT_J1) //  Lance timer si jeu en attente contrat mais pas si petit imprenable
        {
            g_timeout_add(DurationStepContrat, NextStepContrat, NULL);
            UiGame.TypeTimer = TIMER_ENCHERES;
        }
    }
}

static void on_close_about (GtkDialog *dialog, gint       response_id, gpointer   user_data)
{
  /* This will cause the dialog to be destroyed */
  gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void MenuAbout(GtkWidget *widget, gpointer data)
{
GtkWidget *about_dialog;
const gchar *authors[] = {"Thierry Houdoin", NULL};

    about_dialog = gtk_about_dialog_new ();

    gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG (about_dialog), "Tarot à 4");
    gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (about_dialog), "Copyright \xc2\xa9 2020 Thierry Houdoin");
    gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG (about_dialog), authors);
    gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG (about_dialog), IconTarot);
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG (about_dialog), strVersion);
  /* We do not wish to show the title, which in this case would be
   * "AboutDialog Example". We have to reset the title of the messagedialog
   * window after setting the program name.
   */
    gtk_window_set_title (GTK_WINDOW (about_dialog), "");

  /* To close the aboutdialog when "close" is clicked we connect the response
   * signal to on_close_about
   */
    g_signal_connect (GTK_DIALOG (about_dialog), "response", G_CALLBACK (on_close_about), NULL);

  /* Show the about dialog */
    gtk_widget_show (about_dialog);
}


static void TarotQuit(GtkWidget *widget, gpointer data)
{
    printf("Quitte le programme\n");
    SaveConfFile();
    gtk_main_quit();
}

static void ChangeSignalisation(GtkWidget *widget, gpointer data)
{
    if ( UserInterfaceReady == 0 ) return;
    FlagSignalisation = gtk_check_menu_item_get_active(JoueSignalisation);
    ChangeUserConfigSignalisation();
}

static void ChangeModeEnregistreAttaque(GtkWidget *widget, gpointer data)
{
    if ( UserInterfaceReady == 0 ) return;
    if ( NumDonnes > 0 )
    {
        UiGame.InfoMessage = "Changement mode Donnes Libres/Enregistrées\nVoulez-vous abandonner la partie en cours ?";
        UiGame.StateAfterMessage = UiGame.StateJeu;
        UiGame.StateJeu = JEU_NOUVELLE_PARTIE;
        gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage
    }
    else
    {
        FlagPartieEnregistreeAttaque = gtk_check_menu_item_get_active(ModePartieEnregistreeAttaque);
        ChangeUserConfigModeEnregistre();
    }
}

static void ChangeModeEnregistreDefense(GtkWidget *widget, gpointer data)
{
    if ( UserInterfaceReady == 0 ) return;
    if ( NumDonnes > 0 )
    {
        UiGame.InfoMessage = "Changement mode Donnes Libres/Enregistrées\nVoulez-vous abandonner la partie en cours ?";
        UiGame.StateAfterMessage = UiGame.StateJeu;
        UiGame.StateJeu = JEU_NOUVELLE_PARTIE;
        gtk_widget_queue_draw(GameZoneArea);            //  Force réaffichage
    }
    else
    {
        FlagPartieEnregistreeDefense = gtk_check_menu_item_get_active(ModePartieEnregistreeDefense);
        ChangeUserConfigModeEnregistre();
    }
}
int RunUi(int argc, char **argv)
{
int Ret;
GObject *window;
GtkMenuItem *MenuItem;
//GtkBox *StatusBox;
GError *Error = NULL;
gchar *DebugFileName;

    Ret = InitUi(argc, argv);
    if ( Ret ) return 1;
    //  Get icon from resource
    IconTarot = gdk_pixbuf_new_from_resource("/Tarot/TarotIcon.png", &Error);
    if ( IconTarot == NULL )
    {
        printf("Error %d, %s\n", Error->code, Error->message);
    }

    //  Load splash window
    ShowSplash("/Tarot/Splash.png", 1, 2000);
    //  Load Main User Interface

    //  Use GResource to load the user interface built with glade
    //builder = gtk_builder_new_from_string((const char *)Sudoku_ui, Sudoku_ui_len);
    builder = gtk_builder_new_from_resource("/Tarot/Tarot_ui.glade");

    /* Connect signal handlers to the constructed widgets. */
    window = gtk_builder_get_object (builder, "MAIN_WINDOW");
    MainWindow = GTK_WINDOW(window);
    gtk_window_maximize(MainWindow);
    g_signal_connect (window, "destroy", G_CALLBACK (TarotQuit), NULL);

    //  Process Menus entries
    MenuItem = GTK_MENU_ITEM(gtk_builder_get_object(builder, "MENU_DISTRIBUE"));
    g_signal_connect (MenuItem, "activate", G_CALLBACK(MenuDistribue), NULL);
    ModePartieEnregistreeAttaque = GTK_CHECK_MENU_ITEM(gtk_builder_get_object(builder, "UI_MODE_MAITRE_ATTAQUE"));
    g_signal_connect (ModePartieEnregistreeAttaque, "toggled", G_CALLBACK(ChangeModeEnregistreAttaque), NULL);
    ModePartieEnregistreeDefense = GTK_CHECK_MENU_ITEM(gtk_builder_get_object(builder, "UI_MODE_MAITRE_DEFENSE"));
    g_signal_connect (ModePartieEnregistreeDefense, "toggled", G_CALLBACK(ChangeModeEnregistreDefense), NULL);
    MenuItem = GTK_MENU_ITEM(gtk_builder_get_object(builder, "UI_PREFERENCES_UTILISATEUR"));
    g_signal_connect (MenuItem, "activate", G_CALLBACK(MenuUserPref), NULL);
    MenuItem = GTK_MENU_ITEM(gtk_builder_get_object(builder, "MENU_REJOUE"));
    g_signal_connect (MenuItem, "activate", G_CALLBACK(MenuRejoue), NULL);
    MenuItem = GTK_MENU_ITEM(gtk_builder_get_object(builder, "MENU_PLI_PRECEDENT"));
    g_signal_connect (MenuItem, "activate", G_CALLBACK(MenuPliPrecedent), NULL);
    MenuItem = GTK_MENU_ITEM(gtk_builder_get_object(builder, "MENU_OUVRIR"));
    g_signal_connect (MenuItem, "activate", G_CALLBACK(MenuOuvrir), NULL);
    MenuItem = GTK_MENU_ITEM(gtk_builder_get_object(builder, "MENU_SAUVER"));
    g_signal_connect (MenuItem, "activate", G_CALLBACK(MenuSauver), NULL);
    MenuItem = GTK_MENU_ITEM(gtk_builder_get_object(builder, "MENU_QUIT"));
    g_signal_connect (MenuItem, "activate", G_CALLBACK(TarotQuit), NULL);

    MenuItem = GTK_MENU_ITEM(gtk_builder_get_object(builder, "MENU_SCORE_AFFICHE"));
    g_signal_connect (MenuItem, "activate", G_CALLBACK(MenuScoreAffiche), NULL);

    JoueSignalisation = GTK_CHECK_MENU_ITEM(gtk_builder_get_object(builder, "OPTION_SIGNALISATION"));
    g_signal_connect (JoueSignalisation, "toggled", G_CALLBACK(ChangeSignalisation), NULL);

    MenuItem = GTK_MENU_ITEM(gtk_builder_get_object(builder, "OPTION_NOM_JOUEURS"));
    g_signal_connect (MenuItem, "activate", G_CALLBACK(MenuOptionNomsJoueurs), NULL);
    MenuItem = GTK_MENU_ITEM(gtk_builder_get_object(builder, "OPTION_STYLE_JEU"));
    g_signal_connect (MenuItem, "activate", G_CALLBACK(MenuOptionStyleJeu), NULL);
    MenuItem = GTK_MENU_ITEM(gtk_builder_get_object(builder, "OPTIONS_AFFICHAGE"));
    g_signal_connect (MenuItem, "activate", G_CALLBACK(MenuOptionAffichage), NULL);
    MenuItem = GTK_MENU_ITEM(gtk_builder_get_object(builder, "MENU_OPTION_DISTRIB"));
    g_signal_connect (MenuItem, "activate", G_CALLBACK(MenuOptionDistribution), NULL);

    MenuItem = GTK_MENU_ITEM(gtk_builder_get_object(builder, "MENU_ABOUT"));
    g_signal_connect (MenuItem, "activate", G_CALLBACK(MenuAbout), NULL);

    MenuItem = GTK_MENU_ITEM(gtk_builder_get_object(builder, "MENU_CHOIX_PARTIE"));
    g_signal_connect (MenuItem, "activate", G_CALLBACK(MenuChoixPartie), NULL);

    ReloadMenuItem = GTK_MENU_ITEM(gtk_builder_get_object(builder, "RELOAD_LAST"));
    g_signal_connect (ReloadMenuItem, "activate", G_CALLBACK(ReloadLast), NULL);

    //  Boites de dialogue
    DialogAffichage = GTK_DIALOG(gtk_builder_get_object(builder, "DIALOGUE_AFFICHAGE"));
    gtk_window_set_transient_for(GTK_WINDOW(DialogAffichage), MainWindow);
    gtk_window_set_modal(GTK_WINDOW(DialogAffichage), TRUE);

    DialogDistrib = GTK_DIALOG(gtk_builder_get_object(builder, "DIALOGUE_DISTRIBUTION"));
    gtk_window_set_transient_for(GTK_WINDOW(DialogDistrib), MainWindow);
    gtk_window_set_modal(GTK_WINDOW(DialogDistrib), TRUE);

    DialogNomJoueurs = GTK_DIALOG(gtk_builder_get_object(builder, "DIALOGUE_NOMS_JOUEURS"));
    gtk_window_set_transient_for(GTK_WINDOW(DialogNomJoueurs), MainWindow);
    gtk_window_set_modal(GTK_WINDOW(DialogNomJoueurs), TRUE);

    DialogStyleJeu = GTK_DIALOG(gtk_builder_get_object(builder, "DIALOGUE_STYLE_JEU"));
    gtk_window_set_transient_for(GTK_WINDOW(DialogStyleJeu), MainWindow);
    gtk_window_set_modal(GTK_WINDOW(DialogStyleJeu), TRUE);

    DialogChoixPartie = GTK_DIALOG(gtk_builder_get_object(builder, "DIALOGUE_CHOIX_PARTIE"));
    gtk_window_set_transient_for(GTK_WINDOW(DialogChoixPartie), MainWindow);
    gtk_window_set_modal(GTK_WINDOW(DialogChoixPartie), TRUE);

    DialogUserPref = GTK_DIALOG(gtk_builder_get_object(builder, "DIALOGUE_PREFERENCES"));
    gtk_window_set_transient_for(GTK_WINDOW(DialogUserPref), MainWindow);
    gtk_window_set_modal(GTK_WINDOW(DialogUserPref), TRUE);

    //  Status bars
    StatusBarGen = GTK_STATUSBAR(gtk_builder_get_object(builder, "Status_Bar"));
    StatusBarAtout = GTK_STATUSBAR(gtk_builder_get_object(builder, "STATUS_ATOUTS"));
    StatusBarPtAtt = GTK_STATUSBAR(gtk_builder_get_object(builder, "STATUS_POINT_ATT"));
    StatusBarPtDef = GTK_STATUSBAR(gtk_builder_get_object(builder, "STATUS_POINT_DEF"));

    //  Connect to game area
    GameZoneArea = GTK_WIDGET(gtk_builder_get_object(builder, "TABLE_JEU"));
    //  Install callback functions related to game zone : draw event and button pressed
    g_signal_connect (GameZoneArea, "draw", G_CALLBACK (DrawGameZone), &UiGame);
    //  Puis appui bouton souris
    g_signal_connect (GameZoneArea, "button-press-event", G_CALLBACK (ButtonPressGameZone), &UiGame);
    // Et enfin muvement de souris
    g_signal_connect (GameZoneArea, "motion-notify-event", G_CALLBACK (MouseMoveGameZone), &UiGame);
    //  Ask to receive events the drawing area doesn't normally subscribe to.
    //  In particular, we need to ask for the button press event that want to handle.
    gtk_widget_add_events(GameZoneArea, GDK_BUTTON_PRESS_MASK);

    //

    //  Load css style
    Tarot_css = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(Tarot_css, "/Tarot/Tarot.css");
    if ( IconTarot != NULL )
        gtk_window_set_icon(MainWindow, IconTarot);


    //  Inits...
    ReadUserPreferences();
    gtk_check_menu_item_set_active(JoueSignalisation, FlagSignalisation);
    gtk_check_menu_item_set_active(ModePartieEnregistreeAttaque, FlagPartieEnregistreeAttaque);
    gtk_check_menu_item_set_active(ModePartieEnregistreeDefense, FlagPartieEnregistreeDefense);

    InitDistribue(&UiGame);
#if DEBUG > 0
    DebugFileName = g_build_filename(AppConfDir, "TarotDebug.txt", NULL);
    OpenDebugFile(DebugFileName);
    OutDebug("GTK Version %d.%d.%d\n", gtk_get_major_version(), gtk_get_minor_version(), gtk_get_micro_version());
    OutDebug("Tarot version %s\n\n", strVersion);
#endif // DEBUG
    AfficheMsgStatusBarGen("F2 pour distribuer...");
    UserInterfaceReady = TRUE;
    //  And run the main gtk loop
    gtk_main ();
#if DEBUG > 0
    CloseDebugFile();
#endif // DEBUG
    return 0;
}

void ResizeStatusBarGen()
{
    if ( StatusBarGen_width == 0 )
    {
        StatusBarGen_width = GameZoneArea_width / 2;
        gtk_widget_set_size_request(GTK_WIDGET(StatusBarGen), StatusBarGen_width, -1);
//        printf("Set status bar witdh : %d\n", StatusBarGen_width);
    }
}


void AffichagePoints(TarotGame CurrentGame)
{
char strMessagePtAtt[64];
char strMessagePtDef[64];

    ResizeStatusBarGen();
    //  Si besoin, affiche les points
    strMessagePtAtt[0] = 0;
    strMessagePtDef[0] = 0;
    if ( FlagAffichagePoints )
    {
        snprintf(strMessagePtAtt, 60, "Points Attaque %d", CurrentGame->NumPointsPreneur);
        snprintf(strMessagePtDef, 60, "Points Défense %d", CurrentGame->NumPointsDefense);
    }
    AfficheMsgStatusBarPtAtt(strMessagePtAtt);
    AfficheMsgStatusBarPtDef(strMessagePtDef);
}

void AffichageAtouts(TarotGame CurrentGame)
{
char strMessageAtout[64];

    ResizeStatusBarGen();
    //  Si besoin, affiche les points
    strMessageAtout[0] = 0;
    if ( FlagAffichageAtouts )
    {
        snprintf(strMessageAtout, 60, "Atouts %d", NbAtoutJoues(CurrentGame));
    }
    AfficheMsgStatusBarAtout(strMessageAtout);

}
