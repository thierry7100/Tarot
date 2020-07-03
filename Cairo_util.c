#include <gtk/gtk.h>
#include "cairo_util.h"

//  Draw a rounded rectangle starting at x,y with size Sx, Sy.
//  The round factor gives the radius of the rounded rectangle by radius = min(Sx, Sy)*RoundFactor
void CairoRoundedRectangle(cairo_t *cr, double x, double y, double Sx, double Sy, double RoundFactor, int Filled)
{
double Radius = Sx < Sy ? Sx : Sy;

    Radius *=  RoundFactor;
    cairo_new_sub_path (cr);

    cairo_arc (cr, x + Sx - Radius, y + Radius, Radius, -G_PI/2, 0.0);
    cairo_arc (cr, x + Sx - Radius, y + Sy - Radius, Radius, 0.0, G_PI/2);
    cairo_arc (cr, x + Radius, y + Sy - Radius, Radius, G_PI/2, G_PI);
    cairo_arc (cr, x + Radius, y + Radius, Radius, G_PI, 3*G_PI/2);

    cairo_close_path (cr);
    if ( Filled )
        cairo_fill(cr);
    else
        cairo_stroke(cr);
}


//  Affiche une bulle : Rectangle arrondi avec un "bec"
//  La taille du rectangle de base est Sx, Sy, l'arrondi fait Sy*0.45, le bec est au centre de la droite du bas, les angles sont de 45°
//  Si PositionBec = 0 : en dessous

void CairoBulle(cairo_t *cr, double x, double y, double Sx, double Sy, double RoundFactor, double SizeBec, int Filled, int PositionBec)
{
double Radius = Sx < Sy ? Sx : Sy;

    Radius *= RoundFactor;
    cairo_new_sub_path (cr);
    switch ( PositionBec )
    {
    case BEC_DESSOUS:
        //  Commence en haut à gauche
        cairo_move_to(cr, x+Radius, y);             //  Haut gauche
        cairo_line_to(cr, x+Sx-Radius, y);          //  Ligne du haut
        cairo_arc (cr, x + Sx - Radius, y + Radius, Radius, -G_PI/2, 0.0);
        cairo_line_to(cr, x+Sx, y + Sy - Radius);
        cairo_arc (cr, x + Sx - Radius, y + Sy - Radius, Radius, 0.0, G_PI/2);
        //  Droite avec Bec dessous
        cairo_line_to(cr, x+Sx/2.0+SizeBec, y+Sy);
        cairo_line_to(cr, x+Sx/2.0, y+Sy+SizeBec);
        cairo_line_to(cr, x+Sx/2.0-SizeBec, y+Sy);
        cairo_line_to(cr, x+Radius, y+Sy);
        cairo_arc (cr, x + Radius, y + Sy - Radius, Radius, G_PI/2, G_PI);
        cairo_line_to(cr, x, y+Radius);
        cairo_arc (cr, x + Radius, y + Radius, Radius, G_PI, 3*G_PI/2);
        break;
    case BEC_DESSUS:
        //  Commence en haut à gauche, droite jusqu'au bec
        cairo_move_to(cr, x+Radius, y);             //  Haut gauche
        cairo_line_to(cr, x+Sx/2.0-SizeBec, y);
        cairo_line_to(cr, x+Sx/2.0, y-SizeBec);
        cairo_line_to(cr, x+Sx/2.0+SizeBec, y);
        cairo_line_to(cr, x+Sx-Radius, y);      //   Fin de la ligne du dessus
        cairo_arc (cr, x + Sx - Radius, y + Radius, Radius, -G_PI/2, 0.0);
        cairo_line_to(cr, x+Sx, y + Sy - Radius);
        cairo_arc (cr, x + Sx - Radius, y + Sy - Radius, Radius, 0.0, G_PI/2);
        cairo_line_to(cr, x+Radius, y+Sy);
        cairo_arc (cr, x + Radius, y + Sy - Radius, Radius, G_PI/2, G_PI);
        cairo_line_to(cr, x, y+Radius);
        cairo_arc (cr, x + Radius, y + Radius, Radius, G_PI, 3*G_PI/2);
        break;
    case BEC_DROITE:
        //  Commence en haut à gauche
        cairo_move_to(cr, x+Radius, y);             //  Haut gauche
        cairo_line_to(cr, x+Sx-Radius, y);          //  Ligne du haut complète
        cairo_arc (cr, x + Sx - Radius, y + Radius, Radius, -G_PI/2, 0.0);
        //  Coté droit avec bec
        cairo_line_to(cr, x+Sx, y+Sy/2.0-SizeBec);
        cairo_line_to(cr, x+Sx+SizeBec, y+Sy/2);
        cairo_line_to(cr, x+Sx, y+Sy/2.0+SizeBec);
        cairo_line_to(cr, x+Sx, y + Sy - Radius);
        //  Arrondi bas droite
        cairo_arc (cr, x + Sx - Radius, y + Sy - Radius, Radius, 0.0, G_PI/2);
        cairo_line_to(cr, x+Radius, y+Sy);
        cairo_arc (cr, x + Radius, y + Sy - Radius, Radius, G_PI/2, G_PI);
        cairo_line_to(cr, x, y+Radius);
        cairo_arc (cr, x + Radius, y + Radius, Radius, G_PI, 3*G_PI/2);
        break;
    case BEC_GAUCHE:
        //  Commence en haut à gauche
        cairo_move_to(cr, x+Radius, y);             //  Haut gauche
        cairo_line_to(cr, x+Sx-Radius, y);          //  Ligne du haut complète
        cairo_arc (cr, x + Sx - Radius, y + Radius, Radius, -G_PI/2, 0.0);
        //  Coté droit
        cairo_line_to(cr, x+Sx, y + Sy - Radius);
        //  Arrondi bas droite
        cairo_arc (cr, x + Sx - Radius, y + Sy - Radius, Radius, 0.0, G_PI/2);
        cairo_line_to(cr, x+Radius, y+Sy);
        cairo_arc (cr, x + Radius, y + Sy - Radius, Radius, G_PI/2, G_PI);
        //  Coté gauche avec bec
        cairo_line_to(cr, x, y+Sy/2.0+SizeBec);
        cairo_line_to(cr, x-SizeBec, y+Sy/2);
        cairo_line_to(cr, x, y+Sy/2.0-SizeBec);
        cairo_line_to(cr, x, y+Radius);
        cairo_arc (cr, x + Radius, y + Radius, Radius, G_PI, 3*G_PI/2);
        break;

    }
    cairo_close_path (cr);
    if ( Filled )
        cairo_fill(cr);
    else
        cairo_stroke(cr);
}
