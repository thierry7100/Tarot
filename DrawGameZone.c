#include <gtk/gtk.h>
#include "Tarot_Ui_Objects.h"
#include "cairo_util.h"
#include <math.h>

#define COULEUR_TAPIS           0.06, 0.47, 0.14
#define COULEUR_ENCADREMENT     0.14, 0.27, 0.18


//  Dessine la table de jeu

void DrawGameZone(GtkWidget *widget, cairo_t *cr, TarotGame CurrentGame)
{
GtkStyleContext *context;
GdkRGBA color;
double margin;

    context = gtk_widget_get_style_context (widget);
    // Get its size
    GameZoneArea_width = gtk_widget_get_allocated_width (GameZoneArea);
    GameZoneArea_height = gtk_widget_get_allocated_height (GameZoneArea);
    //printf("Initial Game zone size : Width:%d, Height:%d\n", GameZoneArea_width, GameZoneArea_height);

    //  First draw the background matrix
    gtk_render_background (context, cr, 0, 0, GameZoneArea_width, GameZoneArea_height);
    //  Dessine le tapis de jeu complet
    gtk_style_context_get_color (context, gtk_style_context_get_state (context), &color);
    cairo_set_source_rgb(cr, COULEUR_TAPIS);      //  en vert
    cairo_rectangle(cr, 0, 0, GameZoneArea_width, GameZoneArea_height );       //  Draw rectangle
    //  Then fill this rectangle
    cairo_fill(cr);
    //  Then draw a rounder rectangle
    cairo_set_source_rgb(cr, COULEUR_ENCADREMENT);      //  en gris vert
    margin = fmin(GameZoneArea_width, GameZoneArea_height) * 0.02;
    cairo_set_line_width(cr, 3);
    CairoRoundedRectangle(cr, margin, margin, GameZoneArea_width - 2 * margin, GameZoneArea_height - 2*margin, 0.02, 0);

    AfficheCarteJoueurs(cr, CurrentGame);
    AfficheNomJoueurs(widget, cr);
    AfficheContrat(widget, cr, CurrentGame);
    AfficheDistrib(widget, cr, CurrentGame);

    if ( CurrentGame->AffChoixJoueur )
        DrawDialogContract(widget, cr, CurrentGame);
    if ( CurrentGame->StateJeu == JEU_MONTRE_CHIEN )
        AffCartesChien(widget, cr, CurrentGame);
    if ( CurrentGame->StateJeu == JEU_AFFICHE_ATOUT_ECART )
        AffAtoutsEcart(widget, cr, CurrentGame);
    if ( CurrentGame->StateJeu == JEU_MESSAGE )
        AfficheInfoMessageTarot(widget, cr, CurrentGame);
    if (  CurrentGame->StateJeu == JEU_CHOIX_CARTES_CHIEN )
        AfficheInfoMessageTarotGreyed(widget, cr, CurrentGame, CurrentGame->NbCarteLevee == 6);
    if (  CurrentGame->StateJeu == JEU_MESSAGE_NO_BUTTON )
        AfficheInfoMessageSansBouton(widget, cr, CurrentGame);
    if (  CurrentGame->StateJeu == JEU_CHOIX_POIGNEE )
        AfficheInfoMessagePoigneeSUD(widget, cr, CurrentGame);
    if (  CurrentGame->StateJeu == JEU_CHOIX_CARTE_POIGNEE )
        AfficheInfoMessageTarotGreyed(widget, cr, CurrentGame, CurrentGame->NbCarteLevee == CurrentGame->NumAtoutPoignee[SUD]);
    if (  CurrentGame->StateJeu == JEU_AFFICHE_POIGNEE )
        AfficheInfoMessagePoigneeAutres(widget, cr, CurrentGame);
    if (  CurrentGame->StateJeu == JEU_TERMINE )
        AfficheResultatPartie(widget, cr, CurrentGame);
    if ( CurrentGame->StateJeu == JEU_AFFICHE_PLI_PRECEDENT )
        AffichePliPrecedent(widget, cr, CurrentGame);
    if ( CurrentGame->StateJeu == JEU_AFFICHE_SCORES )
        AfficheScores(widget, cr, CurrentGame);
    if ( CurrentGame->StateJeu == JEU_FIN_PARTIE )
        AfficheFinPartie(widget, cr, CurrentGame);
    if ( CurrentGame->StateJeu == JEU_DECLARE_CHELEM )
        AfficheInfoMessageChelem(widget, cr, CurrentGame);
    if ( CurrentGame->StateJeu == JEU_POSE_QUESTION_CHELEM )
        AfficheInfoMessageOuiNon(widget, cr, CurrentGame);
    if ( CurrentGame->StateJeu == JEU_NOUVELLE_PARTIE )
        AfficheInfoMessageOuiNon(widget, cr , CurrentGame);
}
