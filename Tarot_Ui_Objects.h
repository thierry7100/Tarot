//  GTK User interface objects that will be shared among files dealing with UI.
//  Also include functions prototypes

#ifndef TAROT_UI_OBJECTS_H_INCLUDED
#define TAROT_UI_OBJECTS_H_INCLUDED

#include "Tarot_Game.h"

struct RectangleNom {
    double x0, y0;
    int w, h;
    int align;
};



extern GtkWindow *MainWindow;                  //  The top level window
extern GtkCheckMenuItem *JoueSignalisation;    //  If checked, play with signaling (advanced player)
extern GtkWidget *GameZoneArea;                //  Game Zone, will use this zone to draw current game
extern GtkCssProvider *Tarot_css;

extern int GameZoneArea_width;                  //  Size of game zone
extern int GameZoneArea_height;

//  Noms des joueurs
void InitNomJoueurs();

//  Affichage du jeu lui-même

void DrawGameZone(GtkWidget *widget, cairo_t *cr, TarotGame CurrentGame);
gboolean ButtonPressGameZone(GtkWidget      *widget, GdkEventButton *event, TarotGame CurrentGame);
gboolean MouseMoveGameZone(GtkWidget      *widget, GdkEventButton *event, TarotGame CurrentGame);

void AfficheCarte(cairo_t *cr, int idxCarte, double X, double Y, int Visible);
void AfficheCarteJoueurs(cairo_t *cr, TarotGame CurrentGame);
void ComputeCardPosition(TarotGame CurrentGame, int Joueur, int Index, int FlagPli, double *XPos, double *YPos);

void AfficheDistrib(GtkWidget *widget, cairo_t *cr, TarotGame CurrentGame);
void AfficheNomJoueurs(GtkWidget *widget, cairo_t *cr);
void AfficheContrat(GtkWidget *widget, cairo_t *cr, TarotGame CurrentGame);
void DrawDialogContract(GtkWidget *widget, cairo_t *cr, TarotGame CurrentGame);
int ChoixContrat(TarotGame CurrentGame, int Joueur);
int NextStepContrat(gpointer data);
int NextStepJeu(gpointer data);
void AffCartesChien(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame);
void AffAtoutsEcart(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame);
void AfficheInfoMessageTarot(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame);
void AfficheInfoMessageSansBouton(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame);
void AfficheInfoMessagePoigneeSUD(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame);
void AfficheInfoMessagePoigneeAutres(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame);
void AfficheResultatPartie(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame);
void AffichePliPrecedent(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame);
void AfficheScores(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame);
void AfficheFinPartie(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame);
void AfficheInfoMessageChelem(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame);
void AfficheInfoMessageOuiNon(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame);
int ComputeSizeFont(int BaseSize);
void EffaceContratsJoueurs(TarotGame CurrentGame);
void GenereParties(TarotGame CurrentGame);
int EvalGames(TarotGame CurrentGame);
void AfficheMsgStatusBarGen(const char *Message);
void AfficheMsgStatusBarAtout(const char *Message);
void AfficheMsgStatusBarPtDef(const char *Message);
void AfficheMsgStatusBarPtAtt(const char *Message);
void SaveFile(const char *Name, TarotGame CurrentGame);
int LoadFile(const char *Name, TarotGame CurrentGame);
void ReadUserPreferences();
void SaveConfFile();
void ChangePartieMaitreConf();
void ChangeUserConfigSignalisation(void);
void ChangeUserConfigModeEnregistre(void);
void ChangeUserConfigAffichage(void);
void ChangeContraintesDistribution(void);
void ChangeNomsJoueurs(void);
void SaveScores(void);
void SaveStyleJoueurs(void);
void AffichagePoints(TarotGame CurrentGame);
void AffichageAtouts(TarotGame CurrentGame);
void ResizeStatusBarGen();

//  Position des différents éléments
extern double LeftXJoueur[MAX_JOUEURS];     //  Position Gauche des cartes du joueur
extern double TopYJoueur[MAX_JOUEURS];      //  Position Haut des cartes du joueur en Y
extern double RightXJoueur[MAX_JOUEURS];    // Position droite des cartes du joueurs
extern double BottomYJoueur[MAX_JOUEURS];   // Position Bas des cartes du joueur
extern struct RectangleNom PosNomJoueur[MAX_JOUEURS];    //  Position nom des joueurs
extern const char *TexteBoutonPoignee[4];

extern gchar *FileConfName;
extern gchar *AppConfDir;
//  Contraintes de distribution

struct _ContrainteDistrib {
    int MinBout;            //  Nombre minimum de bout pour SUD
    int MinAtout;           //  Nombre minimum d'atouts
    int MinRoi;             //  Nombre minimum de rois
    int MinTrefle;
    int MinCarreau;
    int MinCoeur;
    int MinPique;
    int MinPoints;
    int MaxPoints;
    int JeuFixeContrat;
} ContraintesDistribution;

#define LIMITE_ATTENTE_SUD  5       //  Après 2.5s affichage message : à vous de jouer
#define DECOMPTE_FIN_PLI    10      //  Soit 5 s pour voir le pli

//  "Boutons" utilisés pour le chien, les contrats...
#define MAX_BOUTONS 10

struct BoutonTarot {
    double Xmin, Ymin;
    double Xmax, Ymax;
    double radius;
    int ResponseCode;
    int HasFocus;
};

extern int NbBoutons;
extern struct BoutonTarot BoutonsTarot[MAX_BOUTONS];

extern int DeltaCarteLevee;

extern const int DurationStepContrat;
extern const int DurationStepJeu;

extern int Card_Width;
extern int Card_Height;
#define CARD_WIDTH  348
#define CARD_HEIGHT 662

// L'image associée à chaque carte
extern GdkPixbuf *Image_Cartes[78];      //  78 cartes de tarot
extern GdkPixbuf *Image_Dos;             //  Dos unique pour le moment
#endif // TAROT_UI_OBJECTS_H_INCLUDED
