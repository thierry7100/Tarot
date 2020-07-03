#include <gtk/gtk.h>
#include "Tarot_Ui_Objects.h"
#include "ScreenSize.h"
#include "cairo_util.h"
#include <math.h>
#include <assert.h>
// L'image associée à chaque carte
GdkPixbuf *Image_Cartes[78];      //  78 cartes de tarot
GdkPixbuf *Image_Dos;               //  6 dos pour le moment

int Card_Width;
int Card_Height;
double LeftXJoueur[MAX_JOUEURS];     //  Position Gauche des cartes du joueur
double RightXJoueur[MAX_JOUEURS];    // Position droite des cartes du joueurs
double TopYJoueur[MAX_JOUEURS];      //  Position Haut des cartes du joueur en Y
double BottomYJoueur[MAX_JOUEURS];   // Position Bas des cartes du joueur
int AfficheJeuJoueurs[MAX_JOUEURS];
int DeltaCarteLevee;

static void ComputeCardSize()
{
double Scale, ScaleX, ScaleY;

    //  Calcule l'échelle requise : les cartes font CARD_WIDTH x CARD_HEIGHT
    //  D'abord en Y. Il doit y avoir au moins 5.5 cartes en Y
    ScaleY = (GameZoneArea_height / 4.5) / CARD_HEIGHT;
    //  Il doit y avoir au moins 12 cartes en X
    ScaleX = (GameZoneArea_width / 12.0) / CARD_WIDTH;
    Scale = fmin(ScaleX, ScaleY);
    Card_Height = CARD_HEIGHT * Scale;
    Card_Width = CARD_WIDTH * Scale;
    printf("Taille des cartes sur écran : %d x %d (scale %.3f, en X %.3f en Y %.3f)\n", Card_Width, Card_Height, Scale, ScaleX, ScaleY);
}

static void LoadImageDos()
{
GError *Error = NULL;
GdkPixbuf *lPixbuf;     //  Local copy before scaling

    if ( Card_Height == 0 )
    {
        ComputeCardSize();
    }
    lPixbuf = gdk_pixbuf_new_from_resource("/Tarot/Cartes/Dos/Dos_Defaut.png", &Error);
    Image_Dos = gdk_pixbuf_scale_simple(lPixbuf, Card_Width, Card_Height, GDK_INTERP_BILINEAR);
    g_object_unref(lPixbuf);        //  lPixbuf not needed anymore

}
static void LoadImage(unsigned int Index)
{

char strName[128];
GError *Error = NULL;
GdkPixbuf *lPixbuf;     //  Local copy before scaling


    if ( Card_Height == 0 )
    {
        ComputeCardSize();
    }
    //  Charge les cartes depuis une resource
    if ( Index == 0 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Atout/Atout_Excuse.png");
    }
    else if ( Index <= 21 )     //  Atout
    {
        snprintf(strName, 120, "/Tarot/Cartes/Atout/Atout_%d.png", Index);
    }
    else if ( Index == 22 )     //  As de Trefle
    {
        snprintf(strName, 120, "/Tarot/Cartes/Trefle/Trefle_As.png");
    }
    else if ( Index <= 31 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Trefle/Trefle_%d.png", Index - 21);
    }
    else if ( Index == 32 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Trefle/Trefle_Valet.png");
    }
    else if ( Index == 33 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Trefle/Trefle_Cavalier.png");
    }
    else if ( Index == 34 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Trefle/Trefle_Dame.png");
    }
    else if ( Index == 35 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Trefle/Trefle_Roi.png");
    }
    else if ( Index == 36 )     //  As de Carreau
    {
        snprintf(strName, 120, "/Tarot/Cartes/Carreau/Carreau_As.png");
    }
    else if ( Index <= 45 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Carreau/Carreau_%d.png", Index - 35);
    }
    else if ( Index == 46 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Carreau/Carreau_Valet.png");
    }
    else if ( Index == 47 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Carreau/Carreau_Cavalier.png");
    }
    else if ( Index == 48 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Carreau/Carreau_Dame.png");
    }
    else if ( Index == 49 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Carreau/Carreau_Roi.png");
    }
    else if ( Index == 50 )     //  As de Pique
    {
        snprintf(strName, 120, "/Tarot/Cartes/Pique/Pique_As.png");
    }
    else if ( Index <= 59 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Pique/Pique_%d.png", Index - 49);
    }
    else if ( Index == 60 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Pique/Pique_Valet.png");
    }
    else if ( Index == 61 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Pique/Pique_Cavalier.png");
    }
    else if ( Index == 62 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Pique/Pique_Dame.png");
    }
    else if ( Index == 63 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Pique/Pique_Roi.png");
    }

    else if ( Index == 64 )     //  As de Coeur
    {
        snprintf(strName, 120, "/Tarot/Cartes/Coeur/Coeur_As.png");
    }
    else if ( Index <=  73)
    {
        snprintf(strName, 120, "/Tarot/Cartes/Coeur/Coeur_%d.png", Index - 63);
    }
    else if ( Index == 74 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Coeur/Coeur_Valet.png");
    }
    else if ( Index == 75 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Coeur/Coeur_Cavalier.png");
    }
    else if ( Index == 76 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Coeur/Coeur_Dame.png");
    }
    else if ( Index == 77 )
    {
        snprintf(strName, 120, "/Tarot/Cartes/Coeur/Coeur_Roi.png");
    }
    else
    {
        printf("ERROR : LoadImage : Index cartes illegal = %d\n", Index);
        return;
    }
    lPixbuf = gdk_pixbuf_new_from_resource(strName, &Error);
    Image_Cartes[Index] = gdk_pixbuf_scale_simple(lPixbuf, Card_Width, Card_Height, GDK_INTERP_BILINEAR);
    g_object_unref(lPixbuf);        //  lPixbuf not needed anymore
}

//  Affiche une carte sur l'écran
//  Utilise le contexte cairo cr
//  L'index de la carte est idxCarte
//  La position est X, Y
//  Si le FlagVisible est à 1, affiche la carte, sinon affiche le dos de la carte

void AfficheCarte(cairo_t *cr, int idxCarte, double X, double Y, int Visible)
{
    if ( idxCarte < 0 || idxCarte >= 78 )
    {
        printf("ERROR : Bad index for card : %d\n", idxCarte);
        return;
    }
    if ( Visible > 0 )
    {
        if ( Image_Cartes[idxCarte] == NULL )
        {
            //  Not yet loaded
            LoadImage(idxCarte);
        }
        gdk_cairo_set_source_pixbuf(cr, Image_Cartes[idxCarte], X, Y);
        cairo_paint (cr);
    }
    else
    {
        if ( Image_Dos == NULL )
            LoadImageDos();
        gdk_cairo_set_source_pixbuf(cr, Image_Dos, X, Y);
        cairo_paint (cr);
    }
}

//  Calcule la position d'affichage des cartes
//  Tient compte du Joueur, de l'index dans les cartes du joueur
//  Si FlagPli est vrai, donne la position correspondant au plis
//  Retourne les résultats dans XPos et YPos

const double RecouvrementPliNS = 0.10;          //  Recouvrement vertical des cartes du PLI. 1.0 --> cartes au même endroit
const double RecouvrementPliEO = -0.10;         //  Recouvrement horizontal des cartes du PLI. 1.0 --> Cartes au même endroit. Si < 0 écart plus grand.
const double RecouvrementJeu = 0.35;            //  Taux recouvrement entre les cartes.
const double RecouvrementJeuChien = 0.45;       //  Taux recouvrement entre les cartes pour le chien en ligne
const double RecouvrementAtoutsEcart = 0.75;    //  Taux recouvrement entre les cartes pour le chien en ligne
const double MargeVerticale = 0.03;             //  Marge Haute et basse pour NORD et SUD, fraction taille écran
const double MargeHorizontale = -0.03;          //  Marge droite et gauche pour EST et OUEST, fraction taille écran

void ComputeCardPosition(TarotGame CurrentGame, int Joueur, int Index, int FlagPli, double *XPos, double *YPos)
{
double xc=0, yc=0;      //  Position centre de la carte
double SizeJeuX;    //  Taille du jeu en X
double x=0, y=0;

    if ( FlagPli > 0 )
    {
        //  Position pour pli
        switch ( Joueur )
        {
        case NORD:
            //  La carte de Nord est centrée en X et montée de (1 - RecouvrementPliNS)*SizeY par rapport au centre de l'image
            xc = GameZoneArea_width * 0.5;
            yc = GameZoneArea_height * 0.5 - (1.0 - RecouvrementPliNS)/2.0*Card_Height;
            break;
        case SUD:
            //  La carte de Sud est centrée en X et descendue de (1 - RecouvrementPliNS)*SizeY par rapport au centre de l'image
            xc = GameZoneArea_width * 0.5;
            yc = GameZoneArea_height * 0.5 + (1.0 - RecouvrementPliNS)/2.0*Card_Height;
            break;
        case OUEST:
            //  La carte de Ouest est centrée en Y et décalée vers la gauche de (1 - RecouvrementPliEO)*SizeX par rapport au centre de l'image
            xc = GameZoneArea_width * 0.5 - (1.0 - RecouvrementPliEO)/2.0*Card_Width;
            yc = GameZoneArea_height * 0.5 ;
            break;
        case EST:
            //  La carte de EST est centrée en Y et décalée vers la droite de (1 - RecouvrementPliEO)*SizeX par rapport au centre de l'image
            xc = GameZoneArea_width * 0.5 + (1.0 - RecouvrementPliEO)/2.0*Card_Width;
            yc = GameZoneArea_height * 0.5 ;
            break;
        }
        //  Maintenant donne la position pour affichage
        *XPos = xc - Card_Width/2.0;
        *YPos = yc - Card_Height/2.0;
        return;
    }
    //  Cartes d'un joueur, elles doivent être centrées dans leur zone
    SizeJeuX = (CurrentGame->NbCarteJoueur[Joueur] - 1) * RecouvrementJeu * Card_Width + Card_Width;
    if ( SizeJeuX <= 0 ) SizeJeuX = 0;
    switch ( Joueur )
    {
    case NORD:
        //  Carte centrée en X, position dépend de l'index
        x = (GameZoneArea_width - SizeJeuX)*0.5 + Index * RecouvrementJeu * Card_Width;
        //  Carte au niveau Marge en Y
        y = MargeVerticale * GameZoneArea_height;
        break;
    case SUD:
        //  Carte centrée en X, position dépend de l'index
        x = (GameZoneArea_width - SizeJeuX)*0.5 + Index * RecouvrementJeu * Card_Width;
        //  Carte au niveau Marge basse en Y
        y = GameZoneArea_height * ( 1.0 - MargeVerticale) - Card_Height;
        break;
    case OUEST:
        //  Carte dans moitié gauche écran en X, fin avant zone pli
        x = (GameZoneArea_width*0.5 - SizeJeuX)*0.5 + MargeHorizontale*GameZoneArea_width + Index * RecouvrementJeu * Card_Width;
        //  Carte centrée en Y
        y = (GameZoneArea_height - Card_Height)*0.5;
        break;
    case EST:
        //  Carte dans moitié droite écran en X, fin au niveau de la marge
        x = (GameZoneArea_width*1.5 - SizeJeuX)*0.5 - MargeHorizontale*GameZoneArea_width + Index * RecouvrementJeu * Card_Width;
        //  Carte centrée en Y
        y = (GameZoneArea_height - Card_Height)*0.5;
        break;
    }
    *XPos = x;
    *YPos = y;
    if ( Index == 0 )
    {
        //  Mémorise position à gauche seulement pour la première carte
        LeftXJoueur[Joueur] = x;
        TopYJoueur[Joueur] = y;
        BottomYJoueur[Joueur] = y + Card_Height;
    }
    RightXJoueur[Joueur] = x + Card_Width;      //  Dans tous les cas mémorise position à droite
}

void AfficheCarteJoueurs(cairo_t *cr, TarotGame CurrentGame)
{
int joueur;
int i;
double X, Y;

    if ( Card_Height == 0 )
    {
        ComputeCardSize();
    }
    for ( joueur = 0; joueur < 4; joueur++ )
    {
        if ( CurrentGame->NbCarteJoueur[joueur] == 0 )
        {
            //  Utile pour mettre à jour les positions des noms
            ComputeCardPosition(CurrentGame, joueur, 0, 0, &X, &Y);
        }
        for ( i = 0; i < CurrentGame->NbCarteJoueur[joueur]; i++ )
        {
            ComputeCardPosition(CurrentGame, joueur, i, 0, &X, &Y);
            if ( joueur == SUD && CurrentGame->isCarteLevee[i] )
                Y -= DeltaCarteLevee;
            AfficheCarte(cr, CurrentGame->IdxCartesJoueur[joueur][i], X, Y, CurrentGame->FlagAfficheJoueur[joueur]);
        }
    }
    //  Affiche cartes du pli, mais commence par le joueur avec l'entame
    for ( i = 0; i < 4; i++ )
    {
        joueur = (CurrentGame->JoueurEntame + i)&3;
        if ( CurrentGame->CartePli[joueur] >= 0 )
        {
            ComputeCardPosition(CurrentGame, joueur, 0, 1, &X, &Y);
            AfficheCarte(cr, CurrentGame->CartePli[joueur], X, Y, 1);
        }
    }
}

//  Affiche les cartes du chien dans un rectangle arrondi
//  Affiche également un bouton pour valider

#define COULEUR_FOND_CHIEN   0.03, 0.35, 0.09
#define COULEUR_BOUTON_CHIEN  1.0, 1.0, 1.0
#define COULEUR_TEXTE_CHIEN  0.0, 0.0, 0.0
#define CARTES_EN_LIGNE     1

void AffCartesChien(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame)
{
double wRect, hRect;
double xRect, yRect;
double hBouton, wBouton;
char strFonte[64];
PangoContext *pangocontext;
PangoLayout *pangolayout = NULL;
PangoFontDescription *fontContrat;
PangoRectangle inkRect, logicalRect;
double x, y;
int i;


    pangocontext = gtk_widget_create_pango_context (widget);
    //  Calcule taille fonte
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(14));
    fontContrat = pango_font_description_from_string (strFonte);
    pangolayout = pango_layout_new (pangocontext);
    //  Calcule la taille du texte
    pango_layout_set_text (pangolayout, "OK quand vous avez vu le chien...", -1);
    pango_layout_set_font_description (pangolayout, fontContrat);
    pango_layout_get_pixel_extents(pangolayout, &inkRect, &logicalRect);

#if CARTES_EN_LIGNE > 0
    hRect = Card_Height * 1.7;
#else
    hRect = Card_Height * 2;
#endif // CARTES_EN_LIGNE
    wRect = Card_Width * 4.5;
    if ( wRect < logicalRect.width + 100 )
        wRect = logicalRect.width + 100;
    xRect = (GameZoneArea_width - wRect)/2;     //  Centré dans l'écran
    yRect = (GameZoneArea_height * ( 1.0 - MargeVerticale) - Card_Height - hRect)/2;    //  Centré sur la zone au dessus de SUD
    //  Dessine rectangle arrondi
    cairo_set_source_rgb(cr, COULEUR_FOND_CHIEN);      //  Couleur pour le rectangle (Noir)
    CairoRoundedRectangle(cr, xRect, yRect, wRect, hRect, 0.2, 1);
    //  Dessine le bouton
    NbBoutons = 1;      //  Un seul
    //  Enveloppe du bouton
    wBouton = logicalRect.width + 30;
    BoutonsTarot[0].Xmin = xRect + (wRect - wBouton)/2;
    BoutonsTarot[0].Xmax = BoutonsTarot[0].Xmin + wBouton;
    x = 15;
    hBouton = logicalRect.height*2.0;
    BoutonsTarot[0].Ymax = yRect + (hRect * 0.95);
    BoutonsTarot[0].Ymin = BoutonsTarot[0].Ymax - hBouton;
    y = logicalRect.height*0.5;
    cairo_set_source_rgb(cr, COULEUR_BOUTON_CHIEN);      //  Couleur pour le rectangle (Noir)
    CairoRoundedRectangle(cr, BoutonsTarot[0].Xmin, BoutonsTarot[0].Ymin, wBouton, hBouton, 0.4, 1);
    cairo_set_source_rgb(cr, COULEUR_TEXTE_CHIEN);      //  Couleur pour le texte (Blanc)
    cairo_move_to(cr, BoutonsTarot[0].Xmin+x, BoutonsTarot[0].Ymin+(hBouton-logicalRect.height)/2);
    pango_cairo_show_layout(cr, pangolayout);

    //  Affiche les cartes du chien
#if CARTES_EN_LIGNE > 0
    double SizeJeuX = 5 * RecouvrementJeuChien * Card_Width + Card_Width;
    for ( i = 0; i < 6; i++ )
    {
        x = xRect + (wRect - SizeJeuX)*0.5 + i * RecouvrementJeuChien * Card_Width;
        y = yRect +  (hRect * 0.95 - hBouton - Card_Height)/2;          //  Au milieu de l'espace entre haut de rectangle et bouton
        AfficheCarte(cr, CurrentGame->IdxCartesChien[i], x, y, 1);
    }
#else
    double Rot;
    int idxCarte;
    double Xc, Yc;
    //  Calcule les nouvelles coordonnées pour Cairo
    Xc = xRect + wRect/2;           //  Centre du rectangle en X
    Yc = yRect +  (hRect * 0.95 - hBouton - Card_Height)/2;
    cairo_translate(cr, Xc, Yc);
    for ( i = 0; i < 6; i++ )
    {
        if ( i == 0 )
        {
            Rot = (80 - i*20)*M_PI/180.0;
            cairo_rotate(cr, Rot);
        }
        else
        {
            Rot = -20.0*M_PI/180.0;
            cairo_rotate(cr, Rot);
        }
        idxCarte = CurrentGame->IdxCartesChien[i];
        if ( Image_Cartes[idxCarte] == NULL )
        {
            //  Not yet loaded
            LoadImage(idxCarte);
        }
        gdk_cairo_set_source_pixbuf(cr, Image_Cartes[idxCarte], 0, 0);
        cairo_paint (cr);
    }
    cairo_identity_matrix(cr);      //  Remet valeurs par défaut
#endif // CARTES_EN_LIGNE
}

//  Affiche les atouts qui ont du être mis au chien
//  Affiche également un bouton pour valider

void AffAtoutsEcart(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame)
{
double wRect, hRect;
double xRect, yRect;
double hBouton, wBouton;
char strFonte[64];
PangoContext *pangocontext;
PangoLayout *pangolayout = NULL;
PangoFontDescription *fontContrat;
PangoRectangle inkRect, logicalRect;
double x, y;
int i;
char Texte[64];

    pangocontext = gtk_widget_create_pango_context (widget);
    //  Calcule taille fonte
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(14));
    fontContrat = pango_font_description_from_string (strFonte);
    pangolayout = pango_layout_new (pangocontext);
    //  Calcule la taille du texte
    if ( CurrentGame->NbAtoutEcart > 1 )
        snprintf(Texte, 62, "Obligé de mettre %d atouts à l'écart", CurrentGame->NbAtoutEcart);
    else
        snprintf(Texte, 62, "Obligé de mettre un atout à l'écart");
    pango_layout_set_text (pangolayout, Texte, -1);
    pango_layout_set_font_description (pangolayout, fontContrat);
    pango_layout_get_pixel_extents(pangolayout, &inkRect, &logicalRect);

    hRect = Card_Height * 1.7;
    wRect = Card_Width * CurrentGame->NbAtoutEcart;
    if ( wRect < logicalRect.width + 100 )
        wRect = logicalRect.width + 100;
    xRect = (GameZoneArea_width - wRect)/2;     //  Centré dans l'écran
    yRect = (GameZoneArea_height * ( 1.0 - MargeVerticale) - Card_Height - hRect)/2;    //  Centré sur la zone au dessus de SUD
    //  Dessine rectangle arrondi
    cairo_set_source_rgb(cr, COULEUR_FOND_CHIEN);      //  Couleur pour le rectangle (Noir)
    CairoRoundedRectangle(cr, xRect, yRect, wRect, hRect, 0.2, 1);
    //  Dessine le bouton
    NbBoutons = 1;      //  Un seul
    //  Enveloppe du bouton
    wBouton = logicalRect.width + 30;
    BoutonsTarot[0].Xmin = xRect + (wRect - wBouton)/2;
    BoutonsTarot[0].Xmax = BoutonsTarot[0].Xmin + wBouton;
    x = 15;
    hBouton = logicalRect.height*2;
    BoutonsTarot[0].Ymax = yRect + (hRect * 0.95);
    BoutonsTarot[0].Ymin = BoutonsTarot[0].Ymax - hBouton;
    y = (hBouton-logicalRect.height)/2;
    cairo_set_source_rgb(cr, COULEUR_BOUTON_CHIEN);      //  Couleur pour le rectangle (Noir)
    CairoRoundedRectangle(cr, BoutonsTarot[0].Xmin, BoutonsTarot[0].Ymin, wBouton, hBouton, 0.4, 1);
    cairo_set_source_rgb(cr, COULEUR_TEXTE_CHIEN);      //  Couleur pour le texte (Blanc)
    cairo_move_to(cr, BoutonsTarot[0].Xmin+x, BoutonsTarot[0].Ymin+y);
    pango_cairo_show_layout(cr, pangolayout);

    //  Affiche les cartes du chien
    double SizeJeuX = (CurrentGame->NbAtoutEcart-1) * RecouvrementAtoutsEcart * Card_Width + Card_Width;
    for ( i = 0; i < CurrentGame->NbAtoutEcart; i++ )
    {
        x = xRect + (wRect - SizeJeuX)*0.5 + i * RecouvrementJeuChien * Card_Width;
        y = yRect +  (hRect * 0.95 - hBouton - Card_Height)/2;          //  Au milieu de l'espace entre haut de rectangle et bouton
        AfficheCarte(cr, CurrentGame->IdxCartesChien[i], x, y, 1);
    }
}


//  Affiche les cartes de la poignée d'un Joueur qui n'est PAS SUD dans une bulle
//  Affiche également un bouton pour valider

#define COULEUR_FOND_POIGNEE   0.0, 0.0, 0.0
#define COULEUR_BOUTON_POIGNEE  1.0, 1.0, 1.0
#define COULEUR_TEXTE_POIGNEE  0.0, 0.0, 0.0

void AfficheInfoMessagePoigneeAutres(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame)
{
double wRect, hRect;
double xRect, yRect;
double hBouton, wBouton;
char strFonte[64];
PangoContext *pangocontext;
PangoLayout *pangolayout = NULL;
PangoFontDescription *fontContrat;
PangoRectangle inkRect, logicalRect;
double x, y;
int i;
char Message[256];
int JoueurPoignee = CurrentGame->JoueurAffichePoignee;
int PositionBec;
double SizeJeuX;
int TypePoignee;

    if ( CurrentGame->NumAtoutPoignee[JoueurPoignee] == 10 )
        TypePoignee = POIGNEE_SIMPLE;
    else if ( CurrentGame->NumAtoutPoignee[JoueurPoignee] == 13 )
        TypePoignee = POIGNEE_DOUBLE;
    else
        TypePoignee = POIGNEE_TRIPLE;
    CurrentGame->PoigneeMontree[JoueurPoignee] = TypePoignee;
    pangocontext = gtk_widget_create_pango_context (widget);
    //  Calcule taille fonte
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(14));
    fontContrat = pango_font_description_from_string (strFonte);
    pangolayout = pango_layout_new (pangocontext);
    //  Calcule la taille du texte
    snprintf(Message, 200, "%s montre une %s poignée", NomJoueurs[JoueurPoignee], TexteBoutonPoignee[TypePoignee-1]);
    pango_layout_set_text (pangolayout, Message, -1);
    pango_layout_set_font_description (pangolayout, fontContrat);
    pango_layout_get_pixel_extents(pangolayout, &inkRect, &logicalRect);
    hRect = Card_Height * 1.7;
    SizeJeuX = Card_Width * ( 1 + RecouvrementJeuChien*(CurrentGame->NumAtoutPoignee[JoueurPoignee]-1) );
    wRect = SizeJeuX + 1.5*Card_Width;      //  Un peu de marge autour (une carte et demi )
    if ( wRect < logicalRect.width + 100 )
        wRect = logicalRect.width + 100;
    xRect = (GameZoneArea_width - wRect)/2;     //  Centré dans l'écran
    yRect = (GameZoneArea_height * ( 1.0 - MargeVerticale) - Card_Height - hRect)/2;    //  Centré sur la zone au dessus de SUD
    //  Dessine la bulle
    switch ( JoueurPoignee )
    {
    case NORD:
        PositionBec = BEC_DESSUS;
        break;
    case EST:
        PositionBec = BEC_DROITE;
        break;
    case OUEST:
        PositionBec = BEC_GAUCHE;
        break;
    default:
        PositionBec = BEC_DESSOUS;
        assert(0);
    }
    cairo_set_source_rgb(cr, COULEUR_FOND_POIGNEE);      //  Couleur pour le rectangle (Noir)
    CairoBulle(cr, xRect, yRect, wRect, hRect, 0.1, GameZoneArea_height*0.015, 1, PositionBec);
    //  Dessine le bouton
    NbBoutons = 1;      //  Un seul
    //  Enveloppe du bouton
    wBouton = logicalRect.width + 30;
    BoutonsTarot[0].Xmin = xRect + (wRect - wBouton)/2;
    BoutonsTarot[0].Xmax = BoutonsTarot[0].Xmin + wBouton;
    x = 15;
    hBouton = logicalRect.height*2;
    BoutonsTarot[0].Ymax = yRect + (hRect * 0.95);
    BoutonsTarot[0].Ymin = BoutonsTarot[0].Ymax - hBouton;
    y = (hBouton-logicalRect.height)/2;
    cairo_set_source_rgb(cr, COULEUR_BOUTON_POIGNEE);      //  Couleur pour le rectangle (Noir)
    CairoRoundedRectangle(cr, BoutonsTarot[0].Xmin, BoutonsTarot[0].Ymin, wBouton, hBouton, 0.4, 1);
    cairo_set_source_rgb(cr, COULEUR_TEXTE_POIGNEE);      //  Couleur pour le texte (Blanc)
    cairo_move_to(cr, BoutonsTarot[0].Xmin+x, BoutonsTarot[0].Ymin+y);
    pango_cairo_show_layout(cr, pangolayout);

    //  Affiche les cartes de la poignée
    for ( i = 0; i < CurrentGame->NumAtoutPoignee[JoueurPoignee]; i++ )
    {
        x = xRect + (wRect - SizeJeuX)*0.5 + i * RecouvrementJeuChien * Card_Width;
        y = yRect +  (hRect * 0.95 - hBouton - Card_Height)/2;          //  Au milieu de l'espace entre haut de rectangle et bouton
        AfficheCarte(cr, CurrentGame->ListeAtoutPoignee[JoueurPoignee][i], x, y, 1);
    }
}

//  Affiche les cartes du pli précédent
void AffichePliPrecedent(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame)
{
double wRect, hRect;
double xRect, yRect;
double hBouton, wBouton;
char strFonte[64];
PangoContext *pangocontext;
PangoLayout *pangolayout = NULL;
PangoFontDescription *fontContrat;
PangoRectangle inkRect, logicalRect;
double x, y;
int i;
int joueur;

    assert(CurrentGame->NumPli > 0);
    pangocontext = gtk_widget_create_pango_context (widget);
    //  Calcule taille fonte
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(14));
    fontContrat = pango_font_description_from_string (strFonte);
    pangolayout = pango_layout_new (pangocontext);
    //  Calcule la taille du texte
    pango_layout_set_text (pangolayout, "OK quand vous avez vu le pli...", -1);
    pango_layout_set_font_description (pangolayout, fontContrat);
    pango_layout_get_pixel_extents(pangolayout, &inkRect, &logicalRect);


    hRect = Card_Height * 2.2;
    wRect = Card_Width * 3;
    if ( wRect < logicalRect.width + 100 )
        wRect = logicalRect.width + 100;
    xRect = (GameZoneArea_width - wRect)/2;     //  Centré dans l'écran
    yRect = (GameZoneArea_height * ( 1.0 - MargeVerticale) - Card_Height - hRect)/2;    //  Centré sur la zone au dessus de SUD
    //  Dessine rectangle arrondi
    cairo_set_source_rgb(cr, COULEUR_FOND_CHIEN);      //  Couleur pour le rectangle
    CairoRoundedRectangle(cr, xRect, yRect, wRect, hRect, 0.2, 1);
    //  Dessine le bouton
    NbBoutons = 1;      //  Un seul
    //  Enveloppe du bouton
    wBouton = logicalRect.width + 30;
    BoutonsTarot[0].Xmin = xRect + (wRect - wBouton)/2;
    BoutonsTarot[0].Xmax = BoutonsTarot[0].Xmin + wBouton;
    x = 15;
    hBouton = logicalRect.height*2.0;
    BoutonsTarot[0].Ymax = yRect + (hRect * 0.95);
    BoutonsTarot[0].Ymin = BoutonsTarot[0].Ymax - hBouton;
    y = (hBouton - logicalRect.height)/2;
    cairo_set_source_rgb(cr, COULEUR_BOUTON_CHIEN);      //  Couleur pour le rectangle (Noir)
    CairoRoundedRectangle(cr, BoutonsTarot[0].Xmin, BoutonsTarot[0].Ymin, wBouton, hBouton, 0.4, 1);
    cairo_set_source_rgb(cr, COULEUR_TEXTE_CHIEN);      //  Couleur pour le texte (Blanc)
    cairo_move_to(cr, BoutonsTarot[0].Xmin+x, BoutonsTarot[0].Ymin+y);
    pango_cairo_show_layout(cr, pangolayout);

    //  Affiche cartes du pli, commence par le joueur avec l'entame
    for ( i = 0; i < 4; i++ )
    {
        joueur = (CurrentGame->Entame[CurrentGame->NumPli-1] + i)&3;
        switch ( joueur )
        {
        case SUD:
            x = xRect + (wRect - Card_Width)/2.0;
            y = yRect  + hRect * 0.95 - hBouton - 1.1 * Card_Height;
            break;
        case NORD:
            x = xRect + (wRect - Card_Width)/2.0;
            y = yRect + 0.1 * Card_Height;
            break;
        case OUEST:
            x = xRect + 0.7*Card_Width;
            y = yRect +  (hRect * 0.95 - hBouton - Card_Height)/2;          //  Au milieu de l'espace entre haut de rectangle et bouton
            break;
        case EST:
            x = xRect + wRect - 1.7*Card_Width;
            y = yRect +  (hRect * 0.95 - hBouton - Card_Height)/2;          //  Au milieu de l'espace entre haut de rectangle et bouton
            break;
        }
        AfficheCarte(cr, CurrentGame->MemPlis[CurrentGame->NumPli-1][joueur], x, y, 1);
    }
}
