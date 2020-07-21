#include <gtk/gtk.h>
#include "Tarot_ui.h"
#include "Tarot_Game.h"
#include "Tarot_Ui_Objects.h"
#include "cairo_util.h"

//  Affiche le résultat de la partie
#define COULEUR_RESULTAT_TOUR   0.5, 0.5, 0.5           //  Gris moyen
#define COULEUR_RESULTAT_FOND1  0.7, 0.7, 0.7           //  Gris clair
#define COULEUR_RESULTAT_FOND2  0.5, 0.5, 0.5           //  Gris moyen
#define COULEUR_RESULTAT_TEXTE  0.0, 0.0, 0.0           //  Noir
#define COULEUR_BOUTON_RESULTAT 0.06, 0.47, 0.14        //  Vert "tapis"
#define COULEUR_TEXTE_BOUTON_RESULTAT   1.0, 1.0, 1.0   //  Blanc
#define COULEUR_OK  0.14, 0.68, 0.14
#define COULEUR_KO  0.94, 0.12, 0.08

void AfficheResultatPartie(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame)
{
PangoContext *pangocontext;
PangoLayout *pangolayoutTitre = NULL;
PangoLayout *pangolayoutResultat = NULL;
PangoLayout *pangolayoutNoms = NULL;
PangoFontDescription *fontTitre;
PangoFontDescription *fontNoms;
PangoFontDescription *fontResultat;
PangoRectangle inkRectTitre, logicalRectTitre;
PangoRectangle inkRectBouton, logicalRectBouton;
PangoRectangle inkRectNoms, logicalRectNoms;
PangoRectangle Bulle;
PangoRectangle RectResultat;
char strFonte[64];
int wBouton, hBouton;
double wNomAttaque, wNomDefense;
double xb, yb, wb;
int x_col1, x_col2, x_col3;
double yLigne;
char strTexte[64];
int nLignes = CurrentGame->NumPartieEnregistree>=0 ? 9 : 8;
double Result;

    pangocontext = gtk_widget_create_pango_context (widget);            //  Contexte pango utilisé dans la suite
    //  Crée structures pour les deux fontes qui vont être utilisées
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(20));
    fontTitre = pango_font_description_from_string (strFonte);
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(16));
    fontResultat = pango_font_description_from_string (strFonte);
    snprintf(strFonte, 60, "Sans Bold %d", ComputeSizeFont(16));
    fontNoms = pango_font_description_from_string (strFonte);
    pangolayoutTitre = pango_layout_new (pangocontext);
    pango_layout_set_font_description (pangolayoutTitre, fontTitre);   //   Commence avec le titre
    pango_layout_set_text (pangolayoutTitre, "Résultat", -1);
    //  Récupère taille titre
    pango_layout_get_pixel_extents(pangolayoutTitre, &inkRectTitre, &logicalRectTitre);
    //  Puis avec texte résultat au moins pour les boutons
    pangolayoutResultat = pango_layout_new (pangocontext);
    pango_layout_set_font_description (pangolayoutResultat, fontResultat);   //   Pour texte général et boutons
    pango_layout_set_text (pangolayoutResultat, "Rejouer la donne", -1);
    pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectBouton, &logicalRectBouton);
    //  Puis avec les noms
    pangolayoutNoms = pango_layout_new (pangocontext);
    pango_layout_set_font_description (pangolayoutNoms, fontNoms);   //   Puis les noms
    pango_layout_set_text (pangolayoutNoms, NomJoueurs[CurrentGame->JoueurPreneur], -1);
    pango_layout_get_pixel_extents(pangolayoutNoms, &inkRectNoms, &logicalRectNoms);
    wNomAttaque = logicalRectNoms.width*1.4;
    pango_layout_set_text (pangolayoutNoms, "Défense", -1);
    pango_layout_get_pixel_extents(pangolayoutNoms, &inkRectNoms, &logicalRectNoms);
    wNomDefense = logicalRectNoms.width*1.4;
    if ( wNomAttaque < wNomDefense )
        wNomAttaque = wNomDefense;
    wBouton = logicalRectBouton.width * 1.4;
    hBouton = logicalRectBouton.height * 2;
    Bulle.width = wBouton * 1.2;
    if ( wNomAttaque < logicalRectTitre.width)
        Bulle.width = 2 * Bulle.width + logicalRectTitre.width * 1.5;
    else
        Bulle.width = 2 * Bulle.width + wNomAttaque;
    Bulle.height = hBouton * (nLignes + 2);
    //  Tout d'abord dessine la bulle globale

    printf("Bulle résultats w=%d, h=%d\n", Bulle.width, Bulle.height);
    Bulle.x = (GameZoneArea_width - Bulle.width)/2;
    Bulle.y = (GameZoneArea_height - Bulle.height)/2;
    cairo_set_source_rgb(cr, COULEUR_RESULTAT_TOUR);      //  Couleur pour le tour de résultat
    CairoBulle(cr, Bulle.x, Bulle.y, Bulle.width, Bulle.height, 0.1, Bulle.height*0.05, 1, CurrentGame->JoueurPreneur);
    //  Maintenant le rectangle arrondi dans la bulle
    RectResultat.height = Bulle.height - 2 * hBouton;
    RectResultat.width = Bulle.width * 0.9;
    RectResultat.x = Bulle.x + (Bulle.width - RectResultat.width)/2;
    RectResultat.y = Bulle.y + hBouton * 1.5;
    cairo_set_source_rgb(cr, COULEUR_RESULTAT_FOND1);      //  Couleur pour la zone résultat
    CairoRoundedRectangle(cr, RectResultat.x, RectResultat.y, RectResultat.width, RectResultat.height, 0.1, 1);

    //  Affiche le texte titre centré
    cairo_set_source_rgb(cr, COULEUR_RESULTAT_TEXTE);
    cairo_move_to(cr, RectResultat.x + (RectResultat.width-logicalRectTitre.width)/2
                    , Bulle.y + (1.5*hBouton - logicalRectTitre.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutTitre);
    //  Puis les boutons
    NbBoutons = 2;
    yb = Bulle.y + 0.25*hBouton;
    xb = RectResultat.x + RectResultat.width - wBouton;     //  Aligné à droite sur RectResultat
    cairo_set_source_rgb(cr, COULEUR_BOUTON_RESULTAT);
    CairoRoundedRectangle(cr, xb, yb, wBouton, hBouton, 0.45, 1);
    BoutonsTarot[0].Xmin = xb;
    BoutonsTarot[0].Xmax = xb+wBouton;
    BoutonsTarot[0].Ymin = yb;
    BoutonsTarot[0].Ymax = yb+hBouton;
    BoutonsTarot[0].ResponseCode = 1;
    BoutonsTarot[0].radius = 0.45;
    BoutonsTarot[0].isOK = 1;
    //  Texte du bouton
    cairo_set_source_rgb(cr, COULEUR_TEXTE_BOUTON_RESULTAT);
    cairo_move_to(cr, xb + (wBouton-logicalRectBouton.width)/2
                    , yb + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutResultat);
    xb = RectResultat.x;     //  Aligné à gauche sur RectResultat
    BoutonsTarot[1].Xmin = xb;
    BoutonsTarot[1].Xmax = xb+wBouton;
    BoutonsTarot[1].Ymin = yb;
    BoutonsTarot[1].Ymax = yb+hBouton;
    BoutonsTarot[1].ResponseCode = 2;
    BoutonsTarot[1].radius = 0.45;
    BoutonsTarot[1].HasFocus = 1;
    cairo_set_source_rgb(cr, COULEUR_BOUTON_RESULTAT);
    CairoRoundedRectangle(cr, xb, yb, wBouton, hBouton, 0.45, 1);
    //  Texte du bouton
    pango_layout_set_text (pangolayoutResultat, "Nouvelle partie", -1);
    pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectBouton, &logicalRectBouton);
    cairo_set_source_rgb(cr, COULEUR_TEXTE_BOUTON_RESULTAT);
    cairo_move_to(cr, xb + (wBouton-logicalRectBouton.width)/2
                    , yb + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutResultat);

    //  Pour le texte du résultat calcule la position des colonnes
    x_col3 = RectResultat.x + RectResultat.width - wNomDefense;
    x_col2 = x_col3 - wNomAttaque;
    x_col1 = RectResultat.x + (x_col2 - RectResultat.x)*0.15;
    if ( x_col2 > RectResultat.x + RectResultat.width/2 )
    {
        x_col2 = RectResultat.x + RectResultat.width/2;
        if ( wNomAttaque < RectResultat.width/4)
            x_col3 = x_col2 + RectResultat.width/4;
         else
            x_col3 = (x_col2 + wNomAttaque + x_col3)/2;
    }
    printf("x_col1=%d, x_col2=%d, x_col3 = %d\n", x_col1, x_col2, x_col3);
    //  Affiche les noms des joueurs, défense en premier (déjà calculé)
    yLigne = RectResultat.y;
    cairo_set_source_rgb(cr, COULEUR_RESULTAT_TEXTE);
    cairo_move_to(cr, x_col3 + (wNomDefense - logicalRectNoms.width)/2
                    , yLigne + (hBouton - logicalRectNoms.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutNoms);

    pango_layout_set_text (pangolayoutNoms, NomJoueurs[CurrentGame->JoueurPreneur], -1);
    pango_layout_get_pixel_extents(pangolayoutNoms, &inkRectNoms, &logicalRectNoms);
    cairo_move_to(cr, x_col2 + (wNomAttaque - logicalRectNoms.width)/2
                    , yLigne + (hBouton - logicalRectNoms.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutNoms);
    //  Maintenant chaque ligne
    //  Ligne contrat
    yLigne += hBouton;
    pango_layout_set_text (pangolayoutResultat, "Contrat", -1);
    cairo_move_to(cr, x_col1, yLigne + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutResultat);
    pango_layout_set_text (pangolayoutResultat, strContrat[CurrentGame->TypePartie], -1);
    pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectBouton, &logicalRectBouton);
    cairo_move_to(cr, x_col2 + (x_col3-x_col2-logicalRectBouton.width)/2, yLigne + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutResultat);
    //  Ligne points réalisés
    yLigne += hBouton;
    pango_layout_set_text (pangolayoutResultat, "Points faits", -1);
    cairo_move_to(cr, x_col1, yLigne + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutResultat);
    snprintf(strTexte, 60, "%d (%d)", CurrentGame->NumPointsPreneur, CurrentGame->PointsMinPreneur);
    pango_layout_set_text (pangolayoutResultat, strTexte, -1);
    pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectBouton, &logicalRectBouton);
    cairo_move_to(cr, x_col2 + (x_col3-x_col2-logicalRectBouton.width)/2, yLigne + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutResultat);
    snprintf(strTexte, 60, "%d", CurrentGame->NumPointsDefense);
    pango_layout_set_text (pangolayoutResultat, strTexte, -1);
    pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectBouton, &logicalRectBouton);
    cairo_move_to(cr, x_col3 + (RectResultat.x+RectResultat.width-x_col3-logicalRectBouton.width)/2, yLigne + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutResultat);
    //  Ligne résultat
    yLigne += hBouton;
    pango_layout_set_text (pangolayoutResultat, "Résultat", -1);
    cairo_move_to(cr, x_col1, yLigne + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutResultat);
    if ( CurrentGame->PartieGagnee )
    {
        pango_layout_set_text (pangolayoutResultat, "Gagné", -1);
        pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectBouton, &logicalRectBouton);
        wb = logicalRectBouton.width * 1.4;
        cairo_set_source_rgb(cr, COULEUR_OK);
        xb = x_col2 + (x_col3-x_col2-wb)/2;
        CairoRoundedRectangle(cr, xb, yLigne, wb, hBouton, 0.1, 1);
        cairo_set_source_rgb(cr, COULEUR_TEXTE_BOUTON_RESULTAT);
        cairo_move_to(cr, xb + (wb-logicalRectBouton.width)/2, yLigne + (hBouton - logicalRectBouton.height)/2 );
        pango_cairo_show_layout(cr, pangolayoutResultat);
    }
    else
    {
        pango_layout_set_text (pangolayoutResultat, "Perdu", -1);
        pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectBouton, &logicalRectBouton);
        wb = logicalRectBouton.width * 1.4;
        cairo_set_source_rgb(cr, COULEUR_KO);
        xb = x_col2 + (x_col3-x_col2-wb)/2;
        CairoRoundedRectangle(cr, xb, yLigne, wb, hBouton, 0.2, 1);
        cairo_set_source_rgb(cr, COULEUR_TEXTE_BOUTON_RESULTAT);
        cairo_move_to(cr, xb + (wb-logicalRectBouton.width)/2, yLigne + (hBouton - logicalRectBouton.height)/2 );
        pango_cairo_show_layout(cr, pangolayoutResultat);
    }
    //  Ligne petit au bout
    yLigne += hBouton;
    pango_layout_set_text (pangolayoutResultat, "Petit au bout", -1);
    cairo_set_source_rgb(cr, COULEUR_RESULTAT_TEXTE);
    cairo_move_to(cr, x_col1, yLigne + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutResultat);
    snprintf(strTexte, 60, "%d", CurrentGame->PointPetitAuBoutAttaque);
    pango_layout_set_text (pangolayoutResultat, strTexte, -1);
    pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectBouton, &logicalRectBouton);
    cairo_move_to(cr, x_col2 + (x_col3-x_col2-logicalRectBouton.width)/2, yLigne + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutResultat);
    snprintf(strTexte, 60, "%d", CurrentGame->PointPetitAuBoutDefense);
    pango_layout_set_text (pangolayoutResultat, strTexte, -1);
    pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectBouton, &logicalRectBouton);
    cairo_move_to(cr, x_col3 + (RectResultat.x+RectResultat.width-x_col3-logicalRectBouton.width)/2, yLigne + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutResultat);
    //  Ligne poignée
    yLigne += hBouton;
    pango_layout_set_text (pangolayoutResultat, "Poignée", -1);
    cairo_move_to(cr, x_col1, yLigne + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutResultat);
    snprintf(strTexte, 60, "%d", CurrentGame->PointsPoigneeAttaque);
    pango_layout_set_text (pangolayoutResultat, strTexte, -1);
    pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectBouton, &logicalRectBouton);
    cairo_move_to(cr, x_col2 + (x_col3-x_col2-logicalRectBouton.width)/2, yLigne + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutResultat);
    snprintf(strTexte, 60, "%d", CurrentGame->PointsPoigneeDefense);
    pango_layout_set_text (pangolayoutResultat, strTexte, -1);
    pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectBouton, &logicalRectBouton);
    cairo_move_to(cr, x_col3 + (RectResultat.x+RectResultat.width-x_col3-logicalRectBouton.width)/2, yLigne + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutResultat);
    //  Ligne Chelem
    yLigne += hBouton;
    pango_layout_set_text (pangolayoutResultat, "Chelem", -1);
    cairo_move_to(cr, x_col1, yLigne + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutResultat);
    snprintf(strTexte, 60, "%d", CurrentGame->PointsChelemAttaque);
    pango_layout_set_text (pangolayoutResultat, strTexte, -1);
    pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectBouton, &logicalRectBouton);
    cairo_move_to(cr, x_col2 + (x_col3-x_col2-logicalRectBouton.width)/2, yLigne + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutResultat);
    snprintf(strTexte, 60, "%d", CurrentGame->PointsChelemDefense);
    pango_layout_set_text (pangolayoutResultat, strTexte, -1);
    pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectBouton, &logicalRectBouton);
    cairo_move_to(cr, x_col3 + (RectResultat.x+RectResultat.width-x_col3-logicalRectBouton.width)/2, yLigne + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutResultat);
    //  Ligne Total (en gras)
    yLigne += hBouton;
    pango_layout_set_text (pangolayoutNoms, "Total", -1);
    cairo_move_to(cr, x_col1, yLigne + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutNoms);
    snprintf(strTexte, 60, "%d", CurrentGame->PointsAttaque);
    pango_layout_set_text (pangolayoutNoms, strTexte, -1);
    pango_layout_get_pixel_extents(pangolayoutNoms, &inkRectBouton, &logicalRectBouton);
    wb = logicalRectBouton.width * 1.4;
    if ( CurrentGame->PartieGagnee > 0 )
        cairo_set_source_rgb(cr, COULEUR_OK);
    else
        cairo_set_source_rgb(cr, COULEUR_KO);
    xb = x_col2 + (x_col3-x_col2-wb)/2;
    CairoRoundedRectangle(cr, xb, yLigne, wb, hBouton, 0.1, 1);
    cairo_set_source_rgb(cr, COULEUR_TEXTE_BOUTON_RESULTAT);
    cairo_move_to(cr, x_col2 + (x_col3-x_col2-logicalRectBouton.width)/2, yLigne + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutNoms);
    snprintf(strTexte, 60, "%d", CurrentGame->PointsDefense);
    pango_layout_set_text (pangolayoutNoms, strTexte, -1);
    pango_layout_get_pixel_extents(pangolayoutNoms, &inkRectBouton, &logicalRectBouton);
    wb = logicalRectBouton.width * 1.4;
    if ( CurrentGame->PartieGagnee > 0 )
        cairo_set_source_rgb(cr, COULEUR_KO);
    else
        cairo_set_source_rgb(cr, COULEUR_OK);
    xb = x_col3 + (RectResultat.x+RectResultat.width-x_col3-wb)/2;
    CairoRoundedRectangle(cr, xb, yLigne, wb, hBouton, 0.1, 1);
    cairo_set_source_rgb(cr, COULEUR_TEXTE_BOUTON_RESULTAT);
    cairo_move_to(cr, x_col3 + (RectResultat.x+RectResultat.width-x_col3-logicalRectBouton.width)/2, yLigne + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutNoms);
    //  Ligne supplémentaire résultat (si mode enregistré)
    if ( CurrentGame->NumPartieEnregistree >= 0 )
    {
        yLigne += hBouton;
        Result = ResultatPartieMaitre(CurrentGame)*100.0;
        pango_layout_set_text (pangolayoutResultat, "Classement", -1);
        cairo_move_to(cr, x_col1, yLigne + (hBouton - logicalRectBouton.height)/2 );
        pango_cairo_show_layout(cr, pangolayoutResultat);
        snprintf(strTexte, 60, "%.0f%%", Result);
        pango_layout_set_text (pangolayoutResultat, strTexte, -1);
        pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectBouton, &logicalRectBouton);
        cairo_move_to(cr, x_col2 + (x_col3-x_col2-logicalRectBouton.width)/2, yLigne + (hBouton - logicalRectBouton.height)/2 );
        pango_cairo_show_layout(cr, pangolayoutResultat);
        snprintf(strTexte, 60, "%.0f%%", 100.0 - Result);
        pango_layout_set_text (pangolayoutResultat, strTexte, -1);
        pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectBouton, &logicalRectBouton);
        cairo_move_to(cr, x_col3 + (RectResultat.x+RectResultat.width-x_col3-logicalRectBouton.width)/2, yLigne + (hBouton - logicalRectBouton.height)/2 );
        pango_cairo_show_layout(cr, pangolayoutResultat);

    }
    pango_font_description_free (fontNoms);
    pango_font_description_free (fontTitre);
    pango_font_description_free (fontResultat);
    g_object_unref (pangolayoutResultat);
    g_object_unref (pangolayoutNoms);
    g_object_unref (pangolayoutTitre);
    g_object_unref (pangocontext);

}
