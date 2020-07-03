#include <gtk/gtk.h>
#include "Tarot_ui.h"
#include "Tarot_Game.h"
#include "Tarot_Ui_Objects.h"
#include "cairo_util.h"

gchar *NomJoueurs[MAX_JOUEURS];

void InitNomJoueurs()
{
    NomJoueurs[SUD] = "Thierry Le grand joueur";
    NomJoueurs[NORD] = "Catherine";
    NomJoueurs[EST] = "Jérôme";
    NomJoueurs[OUEST] = "Marie-Claire";
}



struct RectangleNom PosNomJoueur[MAX_JOUEURS];    //  Position nom des joueurs

//  Positionne le nom à coté des cartes du joueur pour NORD et SUD et au dessus à gauche pour OUEST et au dessus à droite pour EST
const double MargeNomX = 30.0;
const double MargeNomY = 20.0;
const int SizeNomY = 25;

static void ComputeNamePosition(int joueur, struct RectangleNom *RectTexte)
{
    switch ( joueur )
    {
    case NORD:
        //  Nord positionne le nom à droite du joueur, centré sur les cartes
        RectTexte->x0 = RightXJoueur[NORD] + MargeNomX;
        //  Ne va pas trop au centre, laisse le bord gauche au 3/5 de l'écran
        if ( RectTexte->x0 < GameZoneArea_width*0.6 )
            RectTexte->x0 = GameZoneArea_width*0.6;
        RectTexte->y0 = (TopYJoueur[NORD] + BottomYJoueur[NORD] - SizeNomY) / 2.0;
        RectTexte->h = SizeNomY;
        RectTexte->w = GameZoneArea_width - RectTexte->x0;
        RectTexte->align = PANGO_ALIGN_LEFT;
        break;
    case SUD:
        //  Sud positionne le nom à gauche du joueur, centré sur les cartes
        RectTexte->w = LeftXJoueur[SUD] - MargeNomX;
        //  Ne va pas trop au centre, laisse le bord droit au 2/5 de l'écran
        if ( RectTexte->w > GameZoneArea_width*0.4 )
            RectTexte->w = GameZoneArea_width*0.4;
        RectTexte->y0 = (TopYJoueur[SUD] + BottomYJoueur[SUD] - SizeNomY) / 2.0;
        RectTexte->h = SizeNomY;
        RectTexte->x0 = 0;
        RectTexte->align = PANGO_ALIGN_RIGHT;
        break;
    case OUEST:
        //  Ouest positionne le nom au dessus à gauche du joueur
        RectTexte->y0 = TopYJoueur[OUEST] - MargeNomY - SizeNomY;
        RectTexte->h = SizeNomY;
        RectTexte->x0 = LeftXJoueur[OUEST];
        //  Ne va pas trop au centre, laisse le bord gauche au 1/4 de l'écran
        if ( RectTexte->x0 > GameZoneArea_width*0.25 )
            RectTexte->x0 = GameZoneArea_width*0.25;
        //  Ne va pas trop à gauche de l'écran, laisse le bord gauche avec place pour icône distrib
        if ( RectTexte->x0 < GameZoneArea_width*0.05 )
            RectTexte->x0 = GameZoneArea_width*0.05;
        RectTexte->w = GameZoneArea_width - RectTexte->x0;
        RectTexte->align = PANGO_ALIGN_LEFT;
        break;
    case EST:
        //  Est positionne le nom au dessus à droite du joueur
        RectTexte->y0 = TopYJoueur[EST] - MargeNomY - SizeNomY;
        RectTexte->h = SizeNomY;
        RectTexte->w = RightXJoueur[EST];
        //  Ne va pas trop au centre, laisse le bord droit au 3/4 de l'écran
        if ( RectTexte->w < GameZoneArea_width*0.75 )
            RectTexte->w = GameZoneArea_width*0.75;
        //  Ne va pas trop à droite, laisse la place pour icône distrib
        if ( RectTexte->w > GameZoneArea_width*0.95 )
            RectTexte->w = GameZoneArea_width*0.95;

        RectTexte->x0 = 0;
        RectTexte->align = PANGO_ALIGN_RIGHT;
        break;
    }
}
//  Affiche le nom des joueurs
#define FONT_NOM "Sans Bold 18"
#define COULEUR_NOM 0.7, 0.7, 0.7

void AfficheNomJoueurs(GtkWidget *widget, cairo_t *cr)
{
int joueur;
struct RectangleNom Position;
PangoContext *pangocontext;
PangoLayout *pangolayout = NULL;
PangoFontDescription *fontNom;
PangoRectangle inkRect, logicalRect;

    pangocontext = gtk_widget_create_pango_context (widget);
    fontNom = pango_font_description_from_string (FONT_NOM);
    pangolayout = pango_layout_new (pangocontext);
    cairo_set_source_rgb(cr, COULEUR_NOM);      //  Couleur pour le texte joueur
    for ( joueur = 0; joueur < MAX_JOUEURS; joueur++)
    {
        ComputeNamePosition(joueur, &Position);
        pango_layout_set_text (pangolayout, NomJoueurs[joueur], -1);
        pango_layout_set_font_description (pangolayout, fontNom);
        pango_layout_set_alignment(pangolayout, Position.align);
        pango_layout_set_width(pangolayout, pango_units_from_double(Position.w));
        cairo_move_to(cr, Position.x0, Position.y0);
        pango_cairo_show_layout(cr, pangolayout);

        //printf("Joueur %s : X=%.0f,Y=%.0f, W=%d, H=%d, Align=%d\n", NomJoueurs[joueur], Position.x0, Position.y0, Position.w, Position.h, Position.align);
        //printf("Joueur %s in pixels : %d,%d w=%d, h=%d\n", NomJoueurs[joueur], logicalRect.x, logicalRect.y, logicalRect.width, logicalRect.height);
        pango_layout_get_pixel_extents(pangolayout, &inkRect, &logicalRect);
        PosNomJoueur[joueur].x0 = logicalRect.x + Position.x0;
        PosNomJoueur[joueur].y0 = logicalRect.y + Position.y0;
        PosNomJoueur[joueur].w = logicalRect.width;
        PosNomJoueur[joueur].h = logicalRect.height;
        PosNomJoueur[joueur].align = Position.align;

    }
    pango_font_description_free (fontNom);
    g_object_unref (pangolayout);
    g_object_unref (pangocontext);
}


//  Affiche un marqueur montant le joueur qui a distribué. A côté du nom du joueur


#define FONT_DISTRIB "Sans Bold"
#define COULEUR_DISTRIB 1.0, 0.0, 0.0
#define COULEUR_TEXTE_DISTRIB 1.0, 1.0, 1.0

void AfficheDistrib(GtkWidget *widget, cairo_t *cr, TarotGame CurrentGame)
{
int joueur;
struct RectangleNom Position;
PangoContext *pangocontext;
PangoLayout *pangolayout = NULL;
PangoFontDescription *fontDistrib;
PangoRectangle inkRect, logicalRect;
char strFont[64];
double dy;
int TailleFont;

    if ( CurrentGame->StateJeu == JEU_VIDE ) return;        // Nothing yet
    //  Taille texte fonction taille écran
    if ( GameZoneArea_height < 700 )
        TailleFont = 12;
    else if ( GameZoneArea_height < 1000 )
        TailleFont = 14;
    else
        TailleFont = 16;

    snprintf(strFont, 60, "%s %d", FONT_DISTRIB, TailleFont );
    //printf("Font Distrib : %s\n", strFont);
    joueur = CurrentGame->JoueurDistrib;
    pangocontext = gtk_widget_create_pango_context (widget);
    fontDistrib = pango_font_description_from_string (strFont);
    pangolayout = pango_layout_new (pangocontext);

    //  Calcule position rectangle arrondi et sa taille
    Position = PosNomJoueur[joueur];
    switch ( joueur )
    {
    case SUD:
    case EST:
        //  Affiche à gauche
        Position.x0 = Position.x0 - TailleFont*4;
        break;
    case NORD:
    case OUEST:
        //  Affiche à droite
        Position.x0 = Position.x0 + Position.w + TailleFont*1.5;
        break;
    }
    Position.w = TailleFont*2.5;
    Position.y0 = Position.y0 + Position.h/2 - TailleFont;
    Position.h = TailleFont*2;
    cairo_set_source_rgb(cr, COULEUR_DISTRIB);      //  Couleur pour le rectangle Distrib
    CairoRoundedRectangle(cr, Position.x0, Position.y0, Position.w, Position.h, 0.40, 1);
    cairo_set_source_rgb(cr, COULEUR_TEXTE_DISTRIB);      //  Couleur pour le texte "D"
    pango_layout_set_text (pangolayout, "D", -1);
    pango_layout_set_font_description (pangolayout, fontDistrib);
    pango_layout_set_alignment(pangolayout, PANGO_ALIGN_CENTER);
    pango_layout_set_width(pangolayout, pango_units_from_double(Position.w));
    pango_layout_get_pixel_extents(pangolayout, &inkRect, &logicalRect);
    dy = (Position.h - logicalRect.height)/2;
    cairo_move_to(cr, Position.x0, Position.y0+dy);
    pango_cairo_show_layout(cr, pangolayout);
    pango_font_description_free (fontDistrib);
    g_object_unref (pangolayout);
    g_object_unref (pangocontext);
}

