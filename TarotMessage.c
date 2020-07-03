#include <gtk/gtk.h>
#include "Tarot_ui.h"
#include "Tarot_Game.h"
#include "Tarot_Ui_Objects.h"
#include "cairo_util.h"
#include <math.h>
//  Affiche un message d'information dans une bulle avec un bouton OK
//  Le massage est affiché sur la table de jeu, un peu au dessus du jeu de SUD
//  Quand le bouton OK est cliqué, le nouvel état du Jeu est MessageNextStep
//

#define COULEUR_BULLE_MESSAGE   0.0, 0.0, 0.0
#define COULEUR_BOUTON_MESSAGE  1.0, 1.0, 1.0
#define COULEUR_TEXTE_MESSAGE   1.0, 1.0, 1.0
#define COULEUR_TEXTE_BOUTON_MESSAGE    0.0, 0.0, 0.0

void AfficheInfoMessageTarot(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame)
{
double wRect, hRect;
double xRect, yRect;
double hBouton, wBouton;
char strFonte[64];
PangoContext *pangocontext;
PangoLayout *pangolayoutTexte = NULL;
PangoLayout *pangolayoutBouton = NULL;
PangoFontDescription *fontMessage;
PangoFontDescription *fontBouton;
PangoRectangle inkRect, logicalRect;
double XBouton, YBouton;
double xtexte, ytexte;

    pangocontext = gtk_widget_create_pango_context (widget);
    //  Calcule taille fonte
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(14));
    fontMessage = pango_font_description_from_string (strFonte);
    pangolayoutTexte = pango_layout_new (pangocontext);
    //  Taille  max de la bulle 0.3* écran en largeur
    wRect = GameZoneArea_width * 0.3;
    //  Calcule la taille du texte à afficher
    pango_layout_set_font_description (pangolayoutTexte, fontMessage);
    pango_layout_set_text (pangolayoutTexte, CurrentGame->InfoMessage, -1);
    pango_layout_set_alignment(pangolayoutTexte, PANGO_ALIGN_CENTER);
    pango_layout_set_width(pangolayoutTexte, pango_units_from_double(wRect));
    pango_layout_get_pixel_extents(pangolayoutTexte, &inkRect, &logicalRect);
    if ( logicalRect.width < wRect*0.8 )        //  Si texte trop petit par rapport à bulle max, réduit la taille de la bulle
        wRect = logicalRect.width * 1.25;
    hBouton = ComputeSizeFont(14)*3.5;          //  Hauteur max bouton
    //  Dessine la bulle
    hRect = logicalRect.height*1.8 + hBouton + GameZoneArea_height*0.03; //  Hauteur bulle
    yRect = TopYJoueur[SUD] - GameZoneArea_height*0.03 - 10 - hRect - CurrentGame->DeltaYMessage;
    xRect = (GameZoneArea_width - wRect) / 2;
    //  Trace la bulle
    cairo_set_source_rgb(cr, COULEUR_BULLE_MESSAGE);      //  Couleur pour la bulle (Noir)
    CairoBulle(cr, xRect, yRect, wRect, hRect, 0.1, GameZoneArea_height*0.015, 1, BEC_DESSOUS);

    YBouton = yRect + hRect - hBouton - GameZoneArea_height*0.015;

    //  Trace le texte du message. Centré au dessus du bouton
    ytexte = yRect + (YBouton-yRect-logicalRect.height)/2;
    xtexte = GameZoneArea_width*0.35;
    cairo_set_source_rgb(cr, COULEUR_TEXTE_MESSAGE);      //  Couleur pour le texte (Blanc)
    cairo_move_to(cr, xtexte, ytexte);
    pango_cairo_show_layout(cr, pangolayoutTexte);
    //  Et maintenant le bouton
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(16));
    pangolayoutBouton = pango_layout_new (pangocontext);
    fontBouton = pango_font_description_from_string (strFonte);
    pango_layout_set_font_description (pangolayoutBouton, fontBouton);
    pango_layout_set_text (pangolayoutBouton, "OK", -1);
    pango_layout_get_pixel_extents(pangolayoutBouton, &inkRect, &logicalRect);
    wBouton = logicalRect.width * 2.5;
    XBouton = (GameZoneArea_width - wBouton)/2.0;
    cairo_set_source_rgb(cr, COULEUR_BOUTON_MESSAGE);      //  Couleur pour le bouton (Blanc)
    CairoRoundedRectangle(cr, XBouton, YBouton, wBouton, hBouton, 0.45, 1);
    ytexte = YBouton + (hBouton - logicalRect.height)/2.0;
    xtexte = XBouton + (wBouton - logicalRect.width)/2.0;
    cairo_set_source_rgb(cr, COULEUR_TEXTE_BOUTON_MESSAGE);      //  Couleur pour le texte (Blanc)
    cairo_move_to(cr, xtexte, ytexte);
    pango_cairo_show_layout(cr, pangolayoutBouton);

    NbBoutons = 1;
    BoutonsTarot[0].Xmin = xRect;
    BoutonsTarot[0].Xmax = xRect+wRect;
    BoutonsTarot[0].Ymin = yRect;
    BoutonsTarot[0].Ymax = yRect+hRect;
    BoutonsTarot[0].ResponseCode = 1;
    BoutonsTarot[0].radius = 0.45;
    BoutonsTarot[0].HasFocus = 1;

    pango_font_description_free (fontMessage);
    pango_font_description_free (fontBouton);
    g_object_unref (pangolayoutBouton);
    g_object_unref (pangolayoutTexte);
    g_object_unref (pangocontext);

}

//  Affiche message demande de Chelem

void AfficheInfoMessageChelem(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame)
{
double wRect, hRect;
double xRect, yRect;
double hBouton, wBouton;
char strFonte[64];
PangoContext *pangocontext;
PangoLayout *pangolayoutTexte = NULL;
PangoLayout *pangolayoutBouton = NULL;
PangoFontDescription *fontMessage;
PangoFontDescription *fontBouton;
PangoRectangle inkRect, logicalRect;
double XBouton, YBouton;
double xtexte, ytexte;

    pangocontext = gtk_widget_create_pango_context (widget);
    //  Calcule taille fonte
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(18));
    fontMessage = pango_font_description_from_string (strFonte);
    pangolayoutTexte = pango_layout_new (pangocontext);
    //  Taille  max de la bulle 0.3* écran en largeur
    wRect = GameZoneArea_width * 0.3;
    //  Calcule la taille du texte à afficher
    pango_layout_set_font_description (pangolayoutTexte, fontMessage);
    pango_layout_set_text (pangolayoutTexte, CurrentGame->InfoMessage, -1);
    pango_layout_set_alignment(pangolayoutTexte, PANGO_ALIGN_CENTER);
    pango_layout_set_width(pangolayoutTexte, pango_units_from_double(wRect));
    pango_layout_get_pixel_extents(pangolayoutTexte, &inkRect, &logicalRect);
    if ( logicalRect.width < wRect*0.8 )        //  Si texte trop petit par rapport à bulle max, réduit la taille de la bulle
        wRect = logicalRect.width * 1.25;
    hBouton = ComputeSizeFont(14)*3.5;          //  Hauteur max bouton
    //  Dessine la bulle
    hRect = logicalRect.height*1.8 + hBouton + GameZoneArea_height*0.03; //  Hauteur bulle
    if ( CurrentGame->JoueurPreneur != NORD )
        yRect = TopYJoueur[OUEST];
    else
        yRect = TopYJoueur[NORD] + Card_Height*1.3;
    xRect = (GameZoneArea_width - wRect) / 2;
    //  Trace la bulle
    cairo_set_source_rgb(cr, COULEUR_BULLE_MESSAGE);      //  Couleur pour la bulle (Noir)
    CairoBulle(cr, xRect, yRect, wRect, hRect, 0.1, GameZoneArea_height*0.015, 1, CurrentGame->JoueurPreneur);

    YBouton = yRect + hRect - hBouton - GameZoneArea_height*0.015;

    //  Trace le texte du message. Centré au dessus du bouton
    ytexte = yRect + (YBouton-yRect-logicalRect.height)/2;
    xtexte = GameZoneArea_width*0.35;
    cairo_set_source_rgb(cr, COULEUR_TEXTE_MESSAGE);      //  Couleur pour le texte (Blanc)
    cairo_move_to(cr, xtexte, ytexte);
    pango_cairo_show_layout(cr, pangolayoutTexte);
    //  Et maintenant le bouton
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(16));
    pangolayoutBouton = pango_layout_new (pangocontext);
    fontBouton = pango_font_description_from_string (strFonte);
    pango_layout_set_font_description (pangolayoutBouton, fontBouton);
    pango_layout_set_text (pangolayoutBouton, "OK", -1);
    pango_layout_get_pixel_extents(pangolayoutBouton, &inkRect, &logicalRect);
    wBouton = logicalRect.width * 2.5;
    XBouton = (GameZoneArea_width - wBouton)/2.0;
    cairo_set_source_rgb(cr, COULEUR_BOUTON_MESSAGE);      //  Couleur pour le bouton (Blanc)
    CairoRoundedRectangle(cr, XBouton, YBouton, wBouton, hBouton, 0.45, 1);
    ytexte = YBouton + (hBouton - logicalRect.height)/2.0;
    xtexte = XBouton + (wBouton - logicalRect.width)/2.0;
    cairo_set_source_rgb(cr, COULEUR_TEXTE_BOUTON_MESSAGE);      //  Couleur pour le texte (Blanc)
    cairo_move_to(cr, xtexte, ytexte);
    pango_cairo_show_layout(cr, pangolayoutBouton);

    NbBoutons = 1;
    BoutonsTarot[0].Xmin = xRect;
    BoutonsTarot[0].Xmax = xRect+wRect;
    BoutonsTarot[0].Ymin = yRect;
    BoutonsTarot[0].Ymax = yRect+hRect;
    BoutonsTarot[0].ResponseCode = 1;
    BoutonsTarot[0].radius = 0.45;
    BoutonsTarot[0].HasFocus = 1;

    pango_font_description_free (fontMessage);
    pango_font_description_free (fontBouton);
    g_object_unref (pangolayoutBouton);
    g_object_unref (pangolayoutTexte);
    g_object_unref (pangocontext);

}

//  Pose question avec 2 réponses Oui / Non

void AfficheInfoMessageOuiNon(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame)
{
double wRect, hRect;
double xRect, yRect;
double hBouton, wBouton, width_all_buttons;
char strFonte[64];
PangoContext *pangocontext;
PangoLayout *pangolayoutTexte = NULL;
PangoLayout *pangolayoutBouton = NULL;
PangoFontDescription *fontMessage;
PangoFontDescription *fontBouton;
PangoRectangle inkRect, logicalRect;
PangoRectangle inkRectBouton, logicalRectBouton;
double XBouton, YBouton;
double xtexte, ytexte;
double espace_inter_boutons;
int i;

    NbBoutons = 2;
    pangocontext = gtk_widget_create_pango_context (widget);
    //  Calcule d'abord la taille des boutons
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(16));
    pangolayoutBouton = pango_layout_new (pangocontext);
    fontBouton = pango_font_description_from_string (strFonte);
    pango_layout_set_font_description (pangolayoutBouton, fontBouton);
    pango_layout_set_text (pangolayoutBouton, "Oui", -1);
    pango_layout_get_pixel_extents(pangolayoutBouton, &inkRectBouton, &logicalRectBouton);
    wBouton = logicalRectBouton.width * 2;
    espace_inter_boutons = wBouton / 3;
    width_all_buttons = NbBoutons * wBouton + (NbBoutons-1) * wBouton/3;
    //  Calcule taille fonte message
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(14));
    fontMessage = pango_font_description_from_string (strFonte);
    pangolayoutTexte = pango_layout_new (pangocontext);
    //  Taille  max de la bulle 0.8* écran en largeur
    wRect = GameZoneArea_width * 0.8;
    //  Calcule la taille du texte à afficher
    pango_layout_set_font_description (pangolayoutTexte, fontMessage);
    pango_layout_set_text (pangolayoutTexte, CurrentGame->InfoMessage, -1);
    pango_layout_set_alignment(pangolayoutTexte, PANGO_ALIGN_LEFT);
    pango_layout_get_pixel_extents(pangolayoutTexte, &inkRect, &logicalRect);
    //  Calcule la taille de la bulle pour être OK avec boutons et texte message
    if ( logicalRect.width < wRect*0.8 && width_all_buttons < wRect*0.8)        //  Si texte trop petit par rapport à bulle max, réduit la taille de la bulle
        wRect = fmax(logicalRect.width, width_all_buttons)  * 1.25;
    hBouton = ComputeSizeFont(14)*3.5;          //  Hauteur max bouton
    //  Dessine la bulle
    hRect = logicalRect.height*1.8 + hBouton + GameZoneArea_height*0.03; //  Hauteur bulle
    yRect = TopYJoueur[SUD] - GameZoneArea_height*0.03 - 10 - hRect - CurrentGame->DeltaYMessage;
    xRect = (GameZoneArea_width - wRect) / 2;
    //  Trace la bulle
    cairo_set_source_rgb(cr, COULEUR_BULLE_MESSAGE);      //  Couleur pour la bulle (Noir)
    CairoBulle(cr, xRect, yRect, wRect, hRect, 0.1, GameZoneArea_height*0.015, 1, BEC_DESSOUS);

    YBouton = yRect + hRect - hBouton - GameZoneArea_height*0.015;

    //  Trace le texte du message. Centré au dessus du bouton
    ytexte = yRect + (YBouton-yRect-logicalRect.height)/2;
    xtexte = xRect + (wRect-logicalRect.width)/2;
    cairo_set_source_rgb(cr, COULEUR_TEXTE_MESSAGE);      //  Couleur pour le texte (Blanc)
    cairo_move_to(cr, xtexte, ytexte);
    pango_cairo_show_layout(cr, pangolayoutTexte);

    //  Et maintenant les boutons
    for ( i = 0; i < NbBoutons; i++ )
    {
        XBouton = (GameZoneArea_width - width_all_buttons)/2.0 + i*(wBouton + espace_inter_boutons);
        cairo_set_source_rgb(cr, COULEUR_BOUTON_MESSAGE);      //  Couleur pour le bouton (Blanc)
        CairoRoundedRectangle(cr, XBouton, YBouton, wBouton, hBouton, 0.45, 1);
        cairo_set_source_rgb(cr, COULEUR_TEXTE_BOUTON_MESSAGE);      //  Couleur pour le texte (Blanc)
        if ( i == 0 )
            pango_layout_set_text (pangolayoutBouton, "Non", -1);
        else
            pango_layout_set_text (pangolayoutBouton, "Oui", -1);
        pango_layout_set_font_description (pangolayoutBouton, fontBouton);
        pango_layout_get_pixel_extents(pangolayoutBouton, &inkRectBouton, &logicalRectBouton);
        ytexte = YBouton + (hBouton - logicalRectBouton.height)/2.0;
        xtexte = XBouton + (wBouton - logicalRectBouton.width)/2.0;
        cairo_move_to(cr, xtexte, ytexte);
        pango_cairo_show_layout(cr, pangolayoutBouton);
        //  Positionne le bouton pour souris
        BoutonsTarot[i].Xmin = XBouton;
        BoutonsTarot[i].Xmax = XBouton+wBouton;
        BoutonsTarot[i].Ymin = YBouton;
        BoutonsTarot[i].Ymax = YBouton+hBouton;
        BoutonsTarot[i].ResponseCode = i;
        BoutonsTarot[i].radius = 0.45;
        BoutonsTarot[i].HasFocus = 1;
    }
    pango_font_description_free (fontMessage);
    pango_font_description_free (fontBouton);
    g_object_unref (pangolayoutBouton);
    g_object_unref (pangolayoutTexte);
    g_object_unref (pangocontext);

}

//  Affiche un message sans bouton

void AfficheInfoMessageSansBouton(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame)
{
double wRect, hRect;
double xRect, yRect;
char strFonte[64];
PangoContext *pangocontext;
PangoLayout *pangolayoutTexte = NULL;
PangoFontDescription *fontMessage;
PangoRectangle inkRect, logicalRect;
double xtexte, ytexte;

    pangocontext = gtk_widget_create_pango_context (widget);
    //  Calcule taille fonte
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(14));
    fontMessage = pango_font_description_from_string (strFonte);
    pangolayoutTexte = pango_layout_new (pangocontext);
    //  Taille  max de la bulle 0.3* écran en largeur
    wRect = GameZoneArea_width * 0.3;
    //  Calcule la taille du texte à afficher
    pango_layout_set_font_description (pangolayoutTexte, fontMessage);
    pango_layout_set_text (pangolayoutTexte, CurrentGame->InfoMessage, -1);
    pango_layout_set_alignment(pangolayoutTexte, PANGO_ALIGN_CENTER);
    pango_layout_set_width(pangolayoutTexte, pango_units_from_double(wRect));
    pango_layout_get_pixel_extents(pangolayoutTexte, &inkRect, &logicalRect);
    if ( logicalRect.width < wRect*0.8 )        //  Si texte trop petit par rapport à bulle max, réduit la taille de la bulle
        wRect = logicalRect.width * 1.25;
    //  Dessine la bulle
    hRect = logicalRect.height*2.0 + GameZoneArea_height*0.03; //  Hauteur bulle
    yRect = TopYJoueur[SUD] - GameZoneArea_height*0.03 - 10 - hRect - CurrentGame->DeltaYMessage;
    xRect = (GameZoneArea_width - wRect) / 2;
    //  Trace la bulle
    cairo_set_source_rgb(cr, COULEUR_BULLE_MESSAGE);      //  Couleur pour la bulle (Noir)
    CairoBulle(cr, xRect, yRect, wRect, hRect, 0.1, GameZoneArea_height*0.015, 1, BEC_DESSOUS);

    //  Trace le texte du message. Centré au dessus du bouton
    ytexte = yRect + (hRect-logicalRect.height)/2;
    xtexte = GameZoneArea_width*0.35;
    cairo_set_source_rgb(cr, COULEUR_TEXTE_MESSAGE);      //  Couleur pour le texte (Blanc)
    cairo_move_to(cr, xtexte, ytexte);
    pango_cairo_show_layout(cr, pangolayoutTexte);
    //  Et maintenant le bouton
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(16));
    cairo_set_source_rgb(cr, COULEUR_TEXTE_BOUTON_MESSAGE);      //  Couleur pour le texte (Blanc)
    cairo_move_to(cr, xtexte, ytexte);

    NbBoutons = 0;
    BoutonsTarot[0].Xmin = xRect;
    BoutonsTarot[0].Xmax = xRect+wRect;
    BoutonsTarot[0].Ymin = yRect;
    BoutonsTarot[0].Ymax = yRect+hRect;
    BoutonsTarot[0].ResponseCode = 1;
    BoutonsTarot[0].radius = 0.45;
    BoutonsTarot[0].HasFocus = 1;

    pango_font_description_free (fontMessage);
    g_object_unref (pangolayoutTexte);
    g_object_unref (pangocontext);

}

//  Affiche message pour indiquer au joueur SUD de choisir la poignée à montrer
//  Les boutons dépendent de la poignée

const char *TexteBoutonPoignee[4] = {"Non", "Simple", "Double", "Triple"};

void AfficheInfoMessagePoigneeSUD(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame)
{
double wRect, hRect;
double xRect, yRect;
double hBouton, wBouton, width_all_buttons;
char strFonte[64];
PangoContext *pangocontext;
PangoLayout *pangolayoutTexte = NULL;
PangoLayout *pangolayoutBouton = NULL;
PangoFontDescription *fontMessage;
PangoFontDescription *fontBouton;
PangoRectangle inkRect, logicalRect;
PangoRectangle inkRectBouton, logicalRectBouton;
double XBouton, YBouton;
double xtexte, ytexte;
double espace_inter_boutons;
int i;

    if ( CurrentGame->NumAtoutPoignee[SUD] == 10 )
        NbBoutons = 2;
    else if ( CurrentGame->NumAtoutPoignee[SUD] == 13 )
        NbBoutons = 3;
    else
        NbBoutons = 4;
    CurrentGame->InfoMessage = "Voulez-vous annoncer une poignée ?";
    pangocontext = gtk_widget_create_pango_context (widget);
    //  Calcule d'abord la taille des boutons
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(16));
    pangolayoutBouton = pango_layout_new (pangocontext);
    fontBouton = pango_font_description_from_string (strFonte);
    pango_layout_set_font_description (pangolayoutBouton, fontBouton);
    pango_layout_set_text (pangolayoutBouton, "Simple", -1);
    pango_layout_get_pixel_extents(pangolayoutBouton, &inkRectBouton, &logicalRectBouton);
    wBouton = logicalRectBouton.width * 2;
    espace_inter_boutons = wBouton / 3;
    width_all_buttons = NbBoutons * wBouton + (NbBoutons-1) * wBouton/3;
    //  Calcule taille fonte message
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(14));
    fontMessage = pango_font_description_from_string (strFonte);
    pangolayoutTexte = pango_layout_new (pangocontext);
    //  Taille  max de la bulle 0.8* écran en largeur
    wRect = GameZoneArea_width * 0.8;
    //  Calcule la taille du texte à afficher
    pango_layout_set_font_description (pangolayoutTexte, fontMessage);
    pango_layout_set_text (pangolayoutTexte, CurrentGame->InfoMessage, -1);
    pango_layout_set_alignment(pangolayoutTexte, PANGO_ALIGN_LEFT);
    pango_layout_get_pixel_extents(pangolayoutTexte, &inkRect, &logicalRect);
    //  Calcule la taille de la bulle pour être OK avec boutons et texte message
    if ( logicalRect.width < wRect*0.8 && width_all_buttons < wRect*0.8)        //  Si texte trop petit par rapport à bulle max, réduit la taille de la bulle
        wRect = fmax(logicalRect.width, width_all_buttons)  * 1.25;
    hBouton = ComputeSizeFont(14)*3.5;          //  Hauteur max bouton
    //  Dessine la bulle
    hRect = logicalRect.height*1.8 + hBouton + GameZoneArea_height*0.03; //  Hauteur bulle
    yRect = TopYJoueur[SUD] - GameZoneArea_height*0.03 - 10 - hRect - CurrentGame->DeltaYMessage;
    xRect = (GameZoneArea_width - wRect) / 2;
    //  Trace la bulle
    cairo_set_source_rgb(cr, COULEUR_BULLE_MESSAGE);      //  Couleur pour la bulle (Noir)
    CairoBulle(cr, xRect, yRect, wRect, hRect, 0.1, GameZoneArea_height*0.015, 1, BEC_DESSOUS);

    YBouton = yRect + hRect - hBouton - GameZoneArea_height*0.015;

    //  Trace le texte du message. Centré au dessus du bouton
    ytexte = yRect + (YBouton-yRect-logicalRect.height)/2;
    xtexte = xRect + (wRect-logicalRect.width)/2;
    cairo_set_source_rgb(cr, COULEUR_TEXTE_MESSAGE);      //  Couleur pour le texte (Blanc)
    cairo_move_to(cr, xtexte, ytexte);
    pango_cairo_show_layout(cr, pangolayoutTexte);

    //  Et maintenant les boutons
    for ( i = 0; i < NbBoutons; i++ )
    {
        XBouton = (GameZoneArea_width - width_all_buttons)/2.0 + i*(wBouton + espace_inter_boutons);
        cairo_set_source_rgb(cr, COULEUR_BOUTON_MESSAGE);      //  Couleur pour le bouton (Blanc)
        CairoRoundedRectangle(cr, XBouton, YBouton, wBouton, hBouton, 0.45, 1);
        cairo_set_source_rgb(cr, COULEUR_TEXTE_BOUTON_MESSAGE);      //  Couleur pour le texte (Blanc)
        pango_layout_set_text (pangolayoutBouton, TexteBoutonPoignee[i], -1);
        pango_layout_set_font_description (pangolayoutBouton, fontBouton);
        pango_layout_get_pixel_extents(pangolayoutBouton, &inkRectBouton, &logicalRectBouton);
        ytexte = YBouton + (hBouton - logicalRectBouton.height)/2.0;
        xtexte = XBouton + (wBouton - logicalRectBouton.width)/2.0;
        cairo_move_to(cr, xtexte, ytexte);
        pango_cairo_show_layout(cr, pangolayoutBouton);
        //  Positionne le bouton pour souris
        BoutonsTarot[i].Xmin = XBouton;
        BoutonsTarot[i].Xmax = XBouton+wBouton;
        BoutonsTarot[i].Ymin = YBouton;
        BoutonsTarot[i].Ymax = YBouton+hBouton;
        BoutonsTarot[i].ResponseCode = i;
        BoutonsTarot[i].radius = 0.45;
        BoutonsTarot[i].HasFocus = 1;
    }
    pango_font_description_free (fontMessage);
    pango_font_description_free (fontBouton);
    g_object_unref (pangolayoutBouton);
    g_object_unref (pangolayoutTexte);
    g_object_unref (pangocontext);
}

//  Affiche message fin de partie
//

void AfficheFinPartie(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame)
{
double wRect, hRect;
double xRect, yRect;
double hBouton, wBouton, width_all_buttons;
char strFonte[64];
PangoContext *pangocontext;
PangoLayout *pangolayoutTexte = NULL;
PangoLayout *pangolayoutBouton = NULL;
PangoFontDescription *fontMessage;
PangoFontDescription *fontBouton;
PangoRectangle inkRect, logicalRect;
PangoRectangle inkRectBouton, logicalRectBouton;
double XBouton, YBouton;
double xtexte, ytexte;
double espace_inter_boutons;
int i;
char Message[128];
double ScoreJoueurs[MAX_JOUEURS];
int JoueurGagnant;

    NbBoutons = 2;
    CalcScoresPartie(ScoreJoueurs, &JoueurGagnant);
    if ( JoueurGagnant >= 0 && JoueurGagnant < MAX_JOUEURS )
    {
        //  Un seul gagnant
        if ( JoueurGagnant == SUD )
            snprintf(Message, 120, "Bravo, vous avez gagné la partie");
        else
            snprintf(Message, 120, "%s a gagné cette partie", NomJoueurs[JoueurGagnant]);
    }
    else
        snprintf(Message, 120, "Fin de la partie");
    CurrentGame->InfoMessage = Message;
    pangocontext = gtk_widget_create_pango_context (widget);
    //  Calcule d'abord la taille des boutons
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(16));
    pangolayoutBouton = pango_layout_new (pangocontext);
    fontBouton = pango_font_description_from_string (strFonte);
    pango_layout_set_font_description (pangolayoutBouton, fontBouton);
    pango_layout_set_text (pangolayoutBouton, "Nouvelle partie", -1);
    pango_layout_get_pixel_extents(pangolayoutBouton, &inkRectBouton, &logicalRectBouton);
    wBouton = logicalRectBouton.width * 2;
    espace_inter_boutons = wBouton / 3;
    width_all_buttons = NbBoutons * wBouton + (NbBoutons-1) * wBouton/3;
    //  Calcule taille fonte message
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(14));
    fontMessage = pango_font_description_from_string (strFonte);
    pangolayoutTexte = pango_layout_new (pangocontext);
    //  Taille  max de la bulle 0.8* écran en largeur
    wRect = GameZoneArea_width * 0.8;
    //  Calcule la taille du texte à afficher
    pango_layout_set_font_description (pangolayoutTexte, fontMessage);
    pango_layout_set_text (pangolayoutTexte, CurrentGame->InfoMessage, -1);
    pango_layout_set_alignment(pangolayoutTexte, PANGO_ALIGN_LEFT);
    pango_layout_get_pixel_extents(pangolayoutTexte, &inkRect, &logicalRect);
    //  Calcule la taille de la bulle pour être OK avec boutons et texte message
    if ( logicalRect.width < wRect*0.8 && width_all_buttons < wRect*0.8)        //  Si texte trop petit par rapport à bulle max, réduit la taille de la bulle
        wRect = fmax(logicalRect.width, width_all_buttons)  * 1.25;
    hBouton = ComputeSizeFont(14)*3.5;          //  Hauteur max bouton
    //  Dessine la bulle
    hRect = logicalRect.height*1.8 + hBouton + GameZoneArea_height*0.03; //  Hauteur bulle
    yRect = TopYJoueur[SUD] - GameZoneArea_height*0.03 - 10 - hRect - CurrentGame->DeltaYMessage;
    xRect = (GameZoneArea_width - wRect) / 2;
    //  Trace la bulle
    cairo_set_source_rgb(cr, COULEUR_BULLE_MESSAGE);      //  Couleur pour la bulle (Noir)
    CairoBulle(cr, xRect, yRect, wRect, hRect, 0.1, GameZoneArea_height*0.015, 1, BEC_DESSOUS);

    YBouton = yRect + hRect - hBouton - GameZoneArea_height*0.015;

    //  Trace le texte du message. Centré au dessus du bouton
    ytexte = yRect + (YBouton-yRect-logicalRect.height)/2;
    xtexte = xRect + (wRect-logicalRect.width)/2;
    cairo_set_source_rgb(cr, COULEUR_TEXTE_MESSAGE);      //  Couleur pour le texte (Blanc)
    cairo_move_to(cr, xtexte, ytexte);
    pango_cairo_show_layout(cr, pangolayoutTexte);

    //  Et maintenant les boutons
    for ( i = 0; i < NbBoutons; i++ )
    {
        XBouton = (GameZoneArea_width - width_all_buttons)/2.0 + i*(wBouton + espace_inter_boutons);
        cairo_set_source_rgb(cr, COULEUR_BOUTON_MESSAGE);      //  Couleur pour le bouton (Blanc)
        CairoRoundedRectangle(cr, XBouton, YBouton, wBouton, hBouton, 0.45, 1);
        cairo_set_source_rgb(cr, COULEUR_TEXTE_BOUTON_MESSAGE);      //  Couleur pour le texte (Blanc)
        if ( i == 0 )
            pango_layout_set_text (pangolayoutBouton, "Voir les scores", -1);
        else
            pango_layout_set_text (pangolayoutBouton, "Nouvelle partie", -1);
        pango_layout_set_font_description (pangolayoutBouton, fontBouton);
        pango_layout_get_pixel_extents(pangolayoutBouton, &inkRectBouton, &logicalRectBouton);
        ytexte = YBouton + (hBouton - logicalRectBouton.height)/2.0;
        xtexte = XBouton + (wBouton - logicalRectBouton.width)/2.0;
        cairo_move_to(cr, xtexte, ytexte);
        pango_cairo_show_layout(cr, pangolayoutBouton);
        //  Positionne le bouton pour souris
        BoutonsTarot[i].Xmin = XBouton;
        BoutonsTarot[i].Xmax = XBouton+wBouton;
        BoutonsTarot[i].Ymin = YBouton;
        BoutonsTarot[i].Ymax = YBouton+hBouton;
        BoutonsTarot[i].ResponseCode = i;
        BoutonsTarot[i].radius = 0.45;
        BoutonsTarot[i].HasFocus = 1;
    }
    pango_font_description_free (fontMessage);
    pango_font_description_free (fontBouton);
    g_object_unref (pangolayoutBouton);
    g_object_unref (pangolayoutTexte);
    g_object_unref (pangocontext);
}

