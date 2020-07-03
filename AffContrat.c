#include <gtk/gtk.h>
#include "Tarot_ui.h"
#include "Tarot_Game.h"
#include "Tarot_Ui_Objects.h"
#include "cairo_util.h"

#define COULEUR_BULLE   0.0, 0.0, 0.0
#define COULEUR_TEXTE_BULLE     1.0, 1.0, 1.0
#define COULEUR_FOND_BOUTON     1.0, 1.0, 1.0
#define COULEUR_FOND_BOUTON_FOCUS   0.3, 0.3, 0.3
#define COULEUR_TEXTE_BOUTON    0.0, 0.0, 0.0

const char *strContrat[5] = { "Passe", "Petite", "Garde", "Garde Sans", "Garde Contre"};
const double HauteurBec = 10;

//  La taille de la fonte utilisée est fonction de la taille des cartes
//  Réduit la font pour les "petits" écrans

int ComputeSizeFont(int BaseSize)
{
    if ( Card_Height < 150 ) return BaseSize-4;
    if ( Card_Height < 220 ) return BaseSize-2;
    return BaseSize;
}

//  Affiche le contrat choisi par les joueurs
void AfficheContrat(GtkWidget *widget, cairo_t *cr, TarotGame CurrentGame)
{
int joueur;
PangoContext *pangocontext;
PangoLayout *pangolayout = NULL;
PangoFontDescription *fontNom;
PangoRectangle inkRect, logicalRect;
PangoRectangle Bulle;
int dx, dy;
char strFonte[64];

    pangocontext = gtk_widget_create_pango_context (widget);
    //  Calcule taille fonte
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(16));
    //printf("Fonte utilisée : %s\n", strFonte);
    fontNom = pango_font_description_from_string (strFonte);
    pangolayout = pango_layout_new (pangocontext);
    for ( joueur = 0; joueur < MAX_JOUEURS; joueur++)
    {
        if ( joueur == SUD && CurrentGame->AffChoixJoueur ) continue;   //  Attente entrée du joueur SUD, ne fait rien
        if ( CurrentGame->AnnonceJoueur[joueur] >= 0 )
        {
            //  Calcule la taille du texte
            pango_layout_set_text (pangolayout, strContrat[CurrentGame->AnnonceJoueur[joueur]], -1);
            pango_layout_set_font_description (pangolayout, fontNom);
            pango_layout_get_pixel_extents(pangolayout, &inkRect, &logicalRect);
            //printf("Joueur %s : Contrat %s, INK X=%d,Y=%d, W=%d, H=%d\n", NomJoueurs[joueur], strContrat[CurrentGame->AnnonceJoueur[joueur]], inkRect.x, inkRect.y, inkRect.width, inkRect.height);
            //printf("Joueur %s : Contrat %s, Logical X=%d,Y=%d, W=%d, H=%d\n", NomJoueurs[joueur], strContrat[CurrentGame->AnnonceJoueur[joueur]], logicalRect.x, logicalRect.y, logicalRect.width, logicalRect.height);

            //  Modifie la taille du texte (bulle plus grande)
            Bulle.height = logicalRect.height*1.4;
            dy = logicalRect.height*0.2;
            Bulle.width = logicalRect.width + Bulle.height;
            dx = (Bulle.width - logicalRect.width) / 2;
            //  Position de la bulle
            Bulle.x = PosNomJoueur[joueur].x0 + (PosNomJoueur[joueur].w - Bulle.width)/2;
            if ( joueur == SUD || joueur == EST )
            {
                //  aligné à droite, la bulle ne doit pas dépasser du nom
                if ( Bulle.x + Bulle.width > PosNomJoueur[joueur].x0 + PosNomJoueur[joueur].w )
                    Bulle.x = PosNomJoueur[joueur].x0 + PosNomJoueur[joueur].w - Bulle.width;
            }
            else
            {
                //  Aligné à gauche, la bulle ne doit pas dépasser du nom
                if ( Bulle.x < PosNomJoueur[joueur].x0 )
                    Bulle.x = PosNomJoueur[joueur].x0 ;
            }
            //  Doit rester dans la zone !
            if ( Bulle.x <= 5 ) Bulle.x = 5;
            if ( Bulle.x + Bulle.width >= GameZoneArea_width - 5 ) Bulle.x = GameZoneArea_width - 5 - Bulle.width;
            Bulle.y = PosNomJoueur[joueur].y0 - Bulle.height - HauteurBec;
            //  Trace la bulle
            cairo_set_source_rgb(cr, COULEUR_BULLE);      //  Couleur pour la bulle (Noir)
            CairoBulle(cr, Bulle.x, Bulle.y, Bulle.width, Bulle.height, 0.45, HauteurBec, 1, BEC_DESSOUS);
            //  Trace le texte
            cairo_set_source_rgb(cr, COULEUR_TEXTE_BULLE);      //  Couleur pour le texte (Blanc)
            cairo_move_to(cr, Bulle.x+dx, Bulle.y+dy);
            pango_cairo_show_layout(cr, pangolayout);
        }
    }
    pango_font_description_free (fontNom);
    g_object_unref (pangolayout);
    g_object_unref (pangocontext);
}


//  Affiche la "boite de dialogue" de choix du contrat

void DrawDialogContract(GtkWidget *widget, cairo_t *cr, TarotGame CurrentGame)
{
int nb;
int nb_row, nb_col;
char strFonte[64];
PangoContext *pangocontext;
PangoLayout *pangolayout = NULL;
PangoFontDescription *fontContrat;
PangoRectangle inkRect, logicalRect;
PangoRectangle Bulle;
double Button_Height, Button_Width, ButtonPasse_Width;
double SpaceBetweenButtonsX;
double SpaceBetweenButtonsY;
double CurY;
double xtexte, ytexte;

    //  Tout d'abord calcule le nombre de boutons
    switch ( CurrentGame->TypePartie )
    {
    case PASSE:
        nb = 5;
        nb_row = 3;
        nb_col = 2;
        break;
    case PETITE:
        nb = 4;
        nb_row = 3;
        nb_col = 2;
        break;
    case GARDE:
        nb = 3;
        nb_row = 2;
        nb_col = 2;
        break;
    case GARDE_SANS:
        nb = 2;
        nb_row = 2;
        nb_col = 1;
        break;
    case GARDE_CONTRE:
        nb = 1;
        nb_row = 1;
        nb_col = 1;
        break;
    default:
        nb = 5;
        nb_row = 3;
        nb_col = 2;
        break;
    }
    pangocontext = gtk_widget_create_pango_context (widget);
    //  Calcule taille fonte
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(16));
    fontContrat = pango_font_description_from_string (strFonte);
    pangolayout = pango_layout_new (pangocontext);
    //  Calcule la taille du texte le plus grand...
    pango_layout_set_text (pangolayout, "Garde Contre", -1);
    pango_layout_set_font_description (pangolayout, fontContrat);
    pango_layout_get_pixel_extents(pangolayout, &inkRect, &logicalRect);
    //  La taille de base des boutons
    Button_Height = logicalRect.height * 2.0;
    Button_Width = logicalRect.width + logicalRect.height;
    SpaceBetweenButtonsX = GameZoneArea_width*0.01;
    SpaceBetweenButtonsY = GameZoneArea_height*0.015;
    ButtonPasse_Width = Button_Width*2 + SpaceBetweenButtonsX;
    //  Taille de la bulle globale
    if ( nb_col == 1 )
    {
        Button_Width = ButtonPasse_Width;       //  Que des grands boutons...
    }
    Bulle.width = ButtonPasse_Width + GameZoneArea_width * 0.02;     //  Marges
    Bulle.height = Button_Height * nb_row + SpaceBetweenButtonsY*(nb_row-1) + GameZoneArea_height * 0.04;     //  Marges
    Bulle.x = (LeftXJoueur[SUD] + RightXJoueur[SUD] - Bulle.width) / 2;
    Bulle.y = TopYJoueur[SUD] - Bulle.height - GameZoneArea_width*0.02;
    //  Trace la bulle
    cairo_set_source_rgb(cr, COULEUR_BULLE);      //  Couleur pour la bulle (Noir)
    CairoBulle(cr, Bulle.x, Bulle.y, Bulle.width, Bulle.height, 0.1, HauteurBec*2, 1, BEC_DESSOUS);
    //  Maintenant dessine les boutons
    //  D'abord le bouton "Passe", toujours présent et de grande taille
    BoutonsTarot[PASSE].Xmin = Bulle.x + GameZoneArea_width*0.01;
    BoutonsTarot[PASSE].Xmax = BoutonsTarot[PASSE].Xmin + ButtonPasse_Width;
    BoutonsTarot[PASSE].Ymax = Bulle.y + Bulle.height - GameZoneArea_height*0.02;
    BoutonsTarot[PASSE].Ymin = BoutonsTarot[PASSE].Ymax - Button_Height;
    CurY = BoutonsTarot[PASSE].Ymin;
    BoutonsTarot[PASSE].radius = Button_Height * 0.25;
    BoutonsTarot[PASSE].ResponseCode = PASSE;
    BoutonsTarot[PASSE].HasFocus = CurrentGame->AnnonceJoueur[SUD] == PASSE;
    //  Maintenant les autres
    if ( nb == 5 )
    {
        //  bouton "Petite", présent seulement si nb==5 et de petite taille
        BoutonsTarot[1].Xmin = Bulle.x + GameZoneArea_width*0.01;
        BoutonsTarot[1].Xmax = BoutonsTarot[1].Xmin + Button_Width;
        BoutonsTarot[1].Ymax = CurY - SpaceBetweenButtonsY;
        BoutonsTarot[1].Ymin = CurY - SpaceBetweenButtonsY - Button_Height;
        BoutonsTarot[1].radius = Button_Height * 0.25;
        BoutonsTarot[1].ResponseCode = PETITE;
        BoutonsTarot[1].HasFocus = CurrentGame->AnnonceJoueur[SUD] == PETITE;
        //  Bouton "Garde" présent et de petite taille
        BoutonsTarot[2].Xmin = BoutonsTarot[1].Xmax + SpaceBetweenButtonsX;
        BoutonsTarot[2].Xmax = BoutonsTarot[2].Xmin + Button_Width;
        BoutonsTarot[2].Ymax = BoutonsTarot[1].Ymax;
        BoutonsTarot[2].Ymin = BoutonsTarot[1].Ymin;
        BoutonsTarot[2].radius = Button_Height * 0.25;
        BoutonsTarot[2].ResponseCode = GARDE;
        BoutonsTarot[2].HasFocus = CurrentGame->AnnonceJoueur[SUD] == GARDE;
        CurY = BoutonsTarot[2].Ymin;
    }
    if ( nb == 4 )
    {
        //  bouton "Garde", présent de grande taille si nb==4
        BoutonsTarot[1].Xmin = Bulle.x + GameZoneArea_width*0.01;
        BoutonsTarot[1].Xmax = BoutonsTarot[1].Xmin + ButtonPasse_Width;
        BoutonsTarot[1].Ymax = CurY - SpaceBetweenButtonsY;
        BoutonsTarot[1].Ymin = CurY - SpaceBetweenButtonsY - Button_Height;
        BoutonsTarot[1].radius = Button_Height * 0.25;
        BoutonsTarot[1].ResponseCode = GARDE;
        BoutonsTarot[1].HasFocus = CurrentGame->AnnonceJoueur[SUD] == GARDE;
        CurY = BoutonsTarot[1].Ymin;
    }
    if ( nb > 2 )
    {
        //  Affiche Garde Sans et Garde Contre de petite Taille
        //  Index Garde Sans : nb - 2, Garde contre : nb - 1
        //  bouton "Garde Sans", présent et de petite taille
        BoutonsTarot[nb-2].Xmin = Bulle.x + GameZoneArea_width*0.01;
        BoutonsTarot[nb-2].Xmax = BoutonsTarot[nb-2].Xmin + Button_Width;
        BoutonsTarot[nb-2].Ymax = CurY - SpaceBetweenButtonsY;
        BoutonsTarot[nb-2].Ymin = CurY - SpaceBetweenButtonsY - Button_Height;
        BoutonsTarot[nb-2].radius = Button_Height * 0.25;
        BoutonsTarot[nb-2].ResponseCode = GARDE_SANS;
        BoutonsTarot[nb-2].HasFocus = CurrentGame->AnnonceJoueur[SUD] == GARDE_SANS;
        //  Bouton "Garde contre" présent et de petite taille
        BoutonsTarot[nb-1].Xmin = BoutonsTarot[nb-2].Xmax + SpaceBetweenButtonsX;
        BoutonsTarot[nb-1].Xmax = BoutonsTarot[nb-1].Xmin + Button_Width;
        BoutonsTarot[nb-1].Ymax = BoutonsTarot[nb-2].Ymax;
        BoutonsTarot[nb-1].Ymin = BoutonsTarot[nb-2].Ymin;
        BoutonsTarot[nb-1].radius = Button_Height * 0.25;
        BoutonsTarot[nb-1].ResponseCode = GARDE_CONTRE;
        BoutonsTarot[nb-1].HasFocus = CurrentGame->AnnonceJoueur[SUD] == GARDE_CONTRE;
        CurY = BoutonsTarot[2].Ymin;
    }
    if ( nb == 2 )
    {
        //  Garde contre de grande taille
        //  Bouton "Garde contre" présent et de grande taille
        BoutonsTarot[nb-1].Xmin = Bulle.x + GameZoneArea_width*0.01;
        BoutonsTarot[nb-1].Xmax = BoutonsTarot[nb-1].Xmin + ButtonPasse_Width;
        BoutonsTarot[nb-1].Ymax = CurY - SpaceBetweenButtonsY;
        BoutonsTarot[nb-1].Ymin = CurY - SpaceBetweenButtonsY - Button_Height;
        BoutonsTarot[nb-1].radius = Button_Height * 0.25;
        BoutonsTarot[nb-1].ResponseCode = GARDE_CONTRE;
        BoutonsTarot[nb-1].HasFocus = CurrentGame->AnnonceJoueur[SUD] == GARDE_CONTRE;
        CurY = BoutonsTarot[2].Ymin;
    }
    NbBoutons = nb;
    //  Affiche les boutons maintenant
    for ( nb = 0; nb < NbBoutons; nb++ )
    {
        cairo_set_source_rgb(cr, COULEUR_FOND_BOUTON);      //  Couleur pour le bouton (Blanc)
        CairoRoundedRectangle(cr, BoutonsTarot[nb].Xmin, BoutonsTarot[nb].Ymin, BoutonsTarot[nb].Xmax-BoutonsTarot[nb].Xmin, BoutonsTarot[nb].Ymax - BoutonsTarot[nb].Ymin, 0.45, 1 );
        //  Puis le texte
        cairo_set_source_rgb(cr, COULEUR_TEXTE_BOUTON);      //  Couleur pour le texte (Noir)
        //  Calcule la taille du texte
        pango_layout_set_text (pangolayout, strContrat[BoutonsTarot[nb].ResponseCode], -1);
        pango_layout_set_font_description (pangolayout, fontContrat);
        pango_layout_get_pixel_extents(pangolayout, &inkRect, &logicalRect);
        ytexte = (BoutonsTarot[nb].Ymax + BoutonsTarot[nb].Ymin - logicalRect.height)/2.0;
        xtexte = (BoutonsTarot[nb].Xmax + BoutonsTarot[nb].Xmin - logicalRect.width)/2.0;
        cairo_move_to(cr, xtexte, ytexte);
        pango_cairo_show_layout(cr, pangolayout);
        //  Si focus, souligne le choix

        if ( BoutonsTarot[nb].HasFocus )
        {
            cairo_set_line_width(cr, 1);
            cairo_move_to(cr, xtexte, ytexte+logicalRect.height);
            cairo_line_to(cr, xtexte+logicalRect.width, ytexte+logicalRect.height);
            cairo_stroke(cr);
        }

    }
    //  Libère les objets pango
    pango_font_description_free (fontContrat);
    g_object_unref (pangolayout);
    g_object_unref (pangocontext);
}
