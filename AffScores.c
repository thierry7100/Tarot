#include <gtk/gtk.h>
#include "Tarot_Ui_Objects.h"
#include "ScreenSize.h"
#include "cairo_util.h"
#include <math.h>
#include <assert.h>

#define COULEUR_RESULTAT_TOUR   0.5, 0.5, 0.5           //  Gris moyen
#define COULEUR_RESULTAT_FOND1  0.7, 0.7, 0.7           //  Gris clair
#define COULEUR_RESULTAT_FOND2  0.6, 0.6, 0.6           //  Gris moyen
#define COULEUR_RESULTAT_TEXTE  0.0, 0.0, 0.0           //  Noir
#define COULEUR_BOUTON_RESULTAT 0.06, 0.47, 0.14        //  Vert "tapis"
#define COULEUR_TEXTE_BOUTON_RESULTAT   1.0, 1.0, 1.0   //  Blanc
#define COULEUR_OK  0.14, 0.68, 0.14
#define COULEUR_KO  0.94, 0.12, 0.08

int NumDonnes;
struct _Donne ResPartie[NOMBRE_DONNES_PARTIE];


void AfficheScores(GtkWidget *widget, cairo_t *cr , TarotGame CurrentGame)
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
int wColNoms = 0;
int wCol1;
int hLigne = 0;
int hLigne1;
int YLigne;
int wBouton, hBouton;
int i, n;
int xb, yb;
int CurY;
char strNum[64];
double ScoreGagnant = -100000000;
double ScoreJoueurs[MAX_JOUEURS];
int wZoneColor;

    pangocontext = gtk_widget_create_pango_context (widget);            //  Contexte pango utilisé dans la suite
    //  Crée structures pour les fontes qui vont être utilisées
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(20));
    fontTitre = pango_font_description_from_string (strFonte);
    snprintf(strFonte, 60, "Sans %d", ComputeSizeFont(14));
    fontResultat = pango_font_description_from_string (strFonte);
    snprintf(strFonte, 60, "Sans Bold %d", ComputeSizeFont(16));
    fontNoms = pango_font_description_from_string (strFonte);
    pangolayoutTitre = pango_layout_new (pangocontext);
    pango_layout_set_font_description (pangolayoutTitre, fontTitre);   //   Commence avec le titre
    pango_layout_set_text (pangolayoutTitre, "Scores", -1);
    //  Récupère taille titre
    pango_layout_get_pixel_extents(pangolayoutTitre, &inkRectTitre, &logicalRectTitre);
    //  Ensuite calcule taille nécessaire pour afficher chaque nom
    pangolayoutNoms = pango_layout_new (pangocontext);
    pango_layout_set_font_description (pangolayoutNoms, fontNoms);
    for ( i = 0; i < 4; i++ )
    {
        pango_layout_set_text (pangolayoutNoms, NomJoueurs[i], -1);
        //  Récupère taille noms
        pango_layout_get_pixel_extents(pangolayoutNoms, &inkRectNoms, &logicalRectNoms);
        if ( logicalRectNoms.width + 40 > wColNoms ) wColNoms = logicalRectNoms.width + 40;     //  Largeur colonnes
        if ( logicalRectNoms.height + 12 > hLigne ) hLigne = logicalRectNoms.height + 12;
    }
    pango_layout_set_text (pangolayoutNoms, "99", -1);
    pango_layout_get_pixel_extents(pangolayoutNoms, &inkRectNoms, &logicalRectNoms);
    wCol1 = logicalRectNoms.width + 20;         //  Largeur colonne 1
    hLigne1 = logicalRectNoms.height + 16;
    //  Puis avec texte résultat au moins pour les boutons
    pangolayoutResultat = pango_layout_new (pangocontext);
    pango_layout_set_font_description (pangolayoutResultat, fontResultat);   //   Pour texte général et boutons
    pango_layout_set_text (pangolayoutResultat, "OK", -1);
    pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectBouton, &logicalRectBouton);
    wBouton = logicalRectBouton.width * 4;
    hBouton = logicalRectBouton.height * 2;
    if ( hBouton < logicalRectTitre.height ) hBouton = logicalRectTitre.height;
    //  Hauteur zone avec texte
    RectResultat.height = hLigne*NOMBRE_DONNES_PARTIE + 2*hLigne1;
    //  Largeur bulle
    Bulle.width = (4 * wColNoms + wCol1 + 10)*1.1;
    //  Hauteur bulle
    Bulle.height = RectResultat.height + hBouton * 2;
    Bulle.x = (GameZoneArea_width - Bulle.width)/2;
    Bulle.y = (GameZoneArea_height - Bulle.height)/2;
    //  Trace le rectangle englobant
    cairo_set_source_rgb(cr, COULEUR_RESULTAT_TOUR);      //  Couleur pour le tour de résultat
    CairoRoundedRectangle(cr, Bulle.x, Bulle.y, Bulle.width, Bulle.height, 0.1, 1);
    RectResultat.width = Bulle.width*0.9;
    RectResultat.x = (GameZoneArea_width - RectResultat.width)/2;
    RectResultat.y = Bulle.y + hBouton*1.4;
    cairo_set_source_rgb(cr, COULEUR_RESULTAT_FOND1);      //  Couleur pour le tour de résultat
    CairoRoundedRectangle(cr, RectResultat.x, RectResultat.y, RectResultat.width, RectResultat.height, 0.02, 1);
    //  Titre et bouton
    //  Affiche le texte titre centré
    cairo_set_source_rgb(cr, COULEUR_RESULTAT_TEXTE);
    cairo_move_to(cr, RectResultat.x + (RectResultat.width-logicalRectTitre.width)/2
                    , Bulle.y + hBouton*0.2 + (hBouton - logicalRectTitre.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutTitre);
    //  Puis le boutons
    NbBoutons = 1;
    yb = Bulle.y + hBouton*0.2 ;
    xb = RectResultat.x + RectResultat.width - 1.5*wBouton;     //  Aligné à droite sur RectResultat
    cairo_set_source_rgb(cr, COULEUR_BOUTON_RESULTAT);
    CairoRoundedRectangle(cr, xb, yb, wBouton, hBouton, 0.45, 1);
    BoutonsTarot[0].Xmin = xb;
    BoutonsTarot[0].Xmax = xb+wBouton;
    BoutonsTarot[0].Ymin = yb;
    BoutonsTarot[0].Ymax = yb+hBouton;
    BoutonsTarot[0].ResponseCode = 1;
    BoutonsTarot[0].radius = 0.45;
    BoutonsTarot[0].HasFocus = 1;
    //  Texte du bouton
    cairo_set_source_rgb(cr, COULEUR_TEXTE_BOUTON_RESULTAT);
    cairo_move_to(cr, xb + (wBouton-logicalRectBouton.width)/2
                    , yb + (hBouton - logicalRectBouton.height)/2 );
    pango_cairo_show_layout(cr, pangolayoutResultat);
    //  Puis la partie avec les noms
    cairo_set_source_rgb(cr, COULEUR_RESULTAT_TEXTE);
    pango_layout_set_text (pangolayoutNoms, "N°", -1);
    pango_layout_get_pixel_extents(pangolayoutNoms, &inkRectNoms, &logicalRectNoms);
    CurY = RectResultat.y + (hLigne-logicalRectNoms.height);
    cairo_move_to(cr, RectResultat.x + (wCol1-logicalRectNoms.width)/2, CurY);
    pango_cairo_show_layout(cr, pangolayoutNoms);
    for ( i = 0; i < 4; i++ )
    {
        pango_layout_set_text (pangolayoutNoms, NomJoueurs[i], -1);
        pango_layout_get_pixel_extents(pangolayoutNoms, &inkRectNoms, &logicalRectNoms);
        cairo_move_to(cr, RectResultat.x + wCol1 +  i*wColNoms + (wColNoms-logicalRectNoms.width)/2, CurY);
        pango_cairo_show_layout(cr, pangolayoutNoms);
    }
    pango_layout_set_text (pangolayoutResultat, "99999", -1);
    pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectBouton, &logicalRectBouton);
    wZoneColor = logicalRectBouton.width * 1.2;
    //  Puis les résultats de chaque partie
    for ( n = 0; n < NumDonnes; n++ )
    {
        if ( (n & 1) == 0 )
        {
            cairo_set_source_rgb(cr, COULEUR_RESULTAT_FOND2);
            cairo_rectangle(cr, RectResultat.x, RectResultat.y + (n+1)*hLigne, RectResultat.width, hLigne);
            cairo_fill(cr);
        }
        cairo_set_source_rgb(cr, COULEUR_RESULTAT_TEXTE);
        snprintf(strNum, 10, "%d", n+1);
        pango_layout_set_text (pangolayoutResultat, strNum, -1);
        pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectBouton, &logicalRectBouton);
        YLigne = RectResultat.y + (n+1)*hLigne;
        CurY = YLigne + (hLigne-logicalRectBouton.height)/2;
        cairo_move_to(cr, RectResultat.x + (wCol1-logicalRectBouton.width)/2, CurY);
        pango_cairo_show_layout(cr, pangolayoutResultat);
        for ( i = 0; i < 4; i++ )
        {
            if ( i == ResPartie[n].Preneur )
            {
                if ( ResPartie[n].TypeEnregistre )
                    snprintf(strNum, 10, "%d%%", ResPartie[n].ScoreAttaque);
                else
                    snprintf(strNum, 10, "%d", ResPartie[n].ScoreAttaque);
                pango_layout_set_text (pangolayoutResultat, strNum, -1);
                pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectBouton, &logicalRectBouton);
                if ( ResPartie[n].ScoreAttaque < 0 )
                {
                    cairo_set_source_rgb(cr, COULEUR_KO);
                }
                else
                {
                    cairo_set_source_rgb(cr, COULEUR_OK);
                }
                CairoRoundedRectangle(cr, RectResultat.x + wCol1 +  i*wColNoms + (wColNoms-wZoneColor)/2, YLigne+1,
                                      wZoneColor, hLigne-2, 0.2, 1);
                cairo_set_source_rgb(cr, COULEUR_TEXTE_BOUTON_RESULTAT);
            }
            else
            {
                if ( ResPartie[n].TypeEnregistre )
                    snprintf(strNum, 10, "%d%%", ResPartie[n].ScoreDefense);
                else
                    snprintf(strNum, 10, "%d", ResPartie[n].ScoreDefense);
                pango_layout_set_text (pangolayoutResultat, strNum, -1);
                pango_layout_get_pixel_extents(pangolayoutResultat, &inkRectNoms, &logicalRectBouton);
                cairo_set_source_rgb(cr, COULEUR_RESULTAT_TEXTE);
            }
            cairo_move_to(cr, RectResultat.x + wCol1 +  i*wColNoms + (wColNoms-logicalRectBouton.width)/2, CurY);
            pango_cairo_show_layout(cr, pangolayoutResultat);
        }
    }
    //  Et enfin le total
    YLigne = RectResultat.y + (NOMBRE_DONNES_PARTIE+1)*hLigne;
    ScoreGagnant = CalcScoresPartie(ScoreJoueurs, NULL);
    for ( i = 0; i < MAX_JOUEURS; i++ )
    {
        if ( ResPartie[0].TypeEnregistre )
            snprintf(strNum, 10, "%.2f%%", ScoreJoueurs[i]);
        else
            snprintf(strNum, 10, "%.0f", ScoreJoueurs[i]);
        pango_layout_set_text (pangolayoutNoms, strNum, -1);
        pango_layout_get_pixel_extents(pangolayoutNoms, &inkRectNoms, &logicalRectBouton);
        if ( i== 0)
            CurY = YLigne + (hLigne1-logicalRectBouton.height)/2;
        if ( ScoreJoueurs[i] == ScoreGagnant )
        {
            cairo_set_source_rgb(cr, COULEUR_OK);
            CairoRoundedRectangle(cr, RectResultat.x + wCol1 +  i*wColNoms + (wColNoms-2*logicalRectBouton.width)/2, YLigne+1,
                                  2*logicalRectBouton.width, hLigne1-2, 0.2, 1);
        }
        cairo_set_source_rgb(cr, COULEUR_RESULTAT_TEXTE);
        cairo_move_to(cr, RectResultat.x + wCol1 +  i*wColNoms + (wColNoms-logicalRectBouton.width)/2, CurY);
        pango_cairo_show_layout(cr, pangolayoutNoms);
    }
}
