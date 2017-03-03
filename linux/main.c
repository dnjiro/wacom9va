//  main.c
//  wacom9va
//
//  Created by user on 17/02/26.
//  Copyright __EVA*Project__ 2017. All rights reserved.
//  you need libgtk-3-dev, xserver-xorg-input-wacom

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <math.h>

#define Max(x,y)        ((x)>=(y) ? (x) : (y))
#define Abs(x)          ((x)>=0   ? (x) : -(x))


int		m_index = 0;		// Number of input strokes
int		isDown = 0;			// 1= mouse button down 0= up
int		whDelta = 0;		// Scroll Offset (Changed by mouse wheel)

// Stroke data
#define AnLINEMAX  1000  	//Maximum strokes number 
#define AnLPNTMAX  250  	//Maximum points number in one stroke

int		m_points[AnLINEMAX];				// Number of points
int		m_pointX[AnLINEMAX][AnLPNTMAX];		// x
int		m_pointY[AnLINEMAX][AnLPNTMAX];		// y
int		m_pressure[AnLINEMAX][AnLPNTMAX];	// pressure


GtkWidget *window;
cairo_surface_t *offscreen=NULL;
int offWidth = 0;
int offHeight = 0;
int	offDelta = 0;

//--------------------------------------------------------------------
void	 asSetPoint(cairo_t *qp, int pnt, int x, int y)
{
	if(!pnt) cairo_move_to(qp, x, y - whDelta );
	else     cairo_line_to(qp, x, y - whDelta );
}

//--------------------------------------------------------------------
void	 MakeBrushLine(cairo_t *qp, int in, int pnt)
{
	int x1,y1,w1,u1,x2,y2,w2,u2,v1,v2, dir, kk;
	double aPI = 3.1415926535897932;
	double qq, pi8 = aPI/12;
	
	kk=0;
	x2 = m_pointX[in][pnt];
	y2 = m_pointY[in][pnt];
	w2 = m_pressure[in][pnt];	
	w2 = Max(1,w2);
	v2 = (w2*1732/2000);u2=Max(1,w2/2);
	dir = 0;	
	for(;pnt>0;){
		x1 = m_pointX[in][pnt-1];
		y1 = m_pointY[in][pnt-1];
		w1 = m_pressure[in][pnt-1];
		w1 = Max(1,w1);
		v1 = (w1*1732/2000);u1=Max(1,w1/2);
		qq = atan2((double)y2-y1,(double)x2-x1);
		for(;qq<0;) qq+=2*aPI;
		dir = (int)((qq+pi8) / pi8/2);
		switch(dir){ //no break in following lines
			case 0: asSetPoint(qp, kk, x1   , y1+w1); if(++kk==7) break;
			case 1: asSetPoint(qp, kk, x1-u1, y1+v1); if(++kk==7) break;
			case 2: asSetPoint(qp, kk, x1-v1, y1+u1); if(++kk==7) break;
			case 3: asSetPoint(qp, kk, x1-w1, y1   ); if(++kk==7) break;
			case 4: asSetPoint(qp, kk, x1-v1, y1-u1); if(++kk==7) break;
			case 5: asSetPoint(qp, kk, x1-u1, y1-v1); if(++kk==7) break;
			case 6: asSetPoint(qp, kk, x1   , y1-w1); if(++kk==7) break;
			case 7: asSetPoint(qp, kk, x1+u1, y1-v1); if(++kk==7) break;
			case 8: asSetPoint(qp, kk, x1+v1, y1-u1); if(++kk==7) break;
			case 9: asSetPoint(qp, kk, x1+w1, y1   ); if(++kk==7) break;
			case 10:asSetPoint(qp, kk, x1+v1, y1+u1); if(++kk==7) break;
			default:asSetPoint(qp, kk, x1+u1, y1+v1); if(++kk==7) break;
				asSetPoint(qp, kk, x1   , y1+w1); if(++kk==7) break;
				asSetPoint(qp, kk, x1-u1, y1+v1); if(++kk==7) break;
				asSetPoint(qp, kk, x1-v1, y1+u1); if(++kk==7) break;
				asSetPoint(qp, kk, x1-w1, y1   ); if(++kk==7) break;
				asSetPoint(qp, kk, x1-v1, y1-u1); if(++kk==7) break;
				asSetPoint(qp, kk, x1-u1, y1-v1); if(++kk==7) break;
				asSetPoint(qp, kk, x1   , y1-w1); if(++kk==7) break;
		}
		break;
	}
	switch(dir){  //no break in following lines
			case 0: asSetPoint(qp, kk, x2,    y2-w2); if(++kk==14) break;
			case 1: asSetPoint(qp, kk, x2+u2, y2-v2); if(++kk==14) break;
			case 2: asSetPoint(qp, kk, x2+v2, y2-u2); if(++kk==14) break;
			case 3: asSetPoint(qp, kk, x2+w2, y2)   ; if(++kk==14) break;
			case 4: asSetPoint(qp, kk, x2+v2, y2+u2); if(++kk==14) break;
			case 5: asSetPoint(qp, kk, x2+u2, y2+v2); if(++kk==14) break;
			case 6: asSetPoint(qp, kk, x2,    y2+w2); if(++kk==14) break;
			case 7: asSetPoint(qp, kk, x2-u2, y2+v2); if(++kk==14) break;
			case 8: asSetPoint(qp, kk, x2-v2, y2+u2); if(++kk==14) break;
			case 9: asSetPoint(qp, kk, x2-w2, y2)   ; if(++kk==14) break;
			case 10:asSetPoint(qp, kk, x2-v2, y2-u2); if(++kk==14) break;
			default:asSetPoint(qp, kk, x2-u2, y2-v2); if(++kk==14) break;
				asSetPoint(qp, kk, x2,    y2-w2); if(++kk==14) break;
				asSetPoint(qp, kk, x2+u2, y2-v2); if(++kk==14) break;
				asSetPoint(qp, kk, x2+v2, y2-u2); if(++kk==14) break;
				asSetPoint(qp, kk, x2+w2, y2)   ; if(++kk==14) break;
				asSetPoint(qp, kk, x2+v2, y2+u2); if(++kk==14) break;
				asSetPoint(qp, kk, x2+u2, y2+v2); if(++kk==14) break;
				asSetPoint(qp, kk, x2,    y2+w2); if(++kk==14) break;
	}
}

//--------------------------------------------------------------------
void AddPolygonPoint(int in, int x, int y, int prs)
{    
	if( in < AnLINEMAX ){
		int pnt = m_points[in];
		if( pnt < AnLPNTMAX ){
			m_pointX[in][pnt] = x;
			m_pointY[in][pnt] = y + whDelta;
			m_pressure[in][pnt] = prs;
			m_points[in]++;
		}
	}
}


gboolean cb_expose_event(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	int in, pnt;
	int ww = gdk_window_get_width(gtk_widget_get_window(window));
	int hh = gdk_window_get_height(gtk_widget_get_window(window));
    if(offWidth!=ww || offHeight!=hh || offDelta != whDelta){//Resize offscreen
		if(offscreen != NULL){
			cairo_surface_destroy(offscreen);
		}
		offscreen = cairo_surface_create_similar(cairo_get_target(cr), 
                         CAIRO_CONTENT_COLOR_ALPHA, ww, hh);
		offWidth  = ww;
		offHeight = hh;
		offDelta  = whDelta;
		{   
			cairo_t *qp = cairo_create(offscreen);
			cairo_set_line_width(qp, 0);
			cairo_set_source_rgb (qp, 0., 1., 0.);
			cairo_new_sub_path (qp);		
			for(in=0; in < m_index; in++){
				for(pnt=0; pnt< m_points[in]; pnt++){ 
					MakeBrushLine(qp, in, pnt);
				}
			}
			cairo_fill (qp);
			cairo_destroy(qp);    
		}
	}
	cairo_set_source_surface(cr, offscreen, 0, 0);
	cairo_paint (cr);
    return FALSE;
}

gint cb_motion_notify_event( GtkWidget *widget, GdkEventMotion *event, gpointer user_data )
{
	if(offscreen){
		if(isDown){
			gdouble pressure =0;
			if(!gdk_event_get_axis ((GdkEvent *)event, GDK_AXIS_PRESSURE, &pressure)){
				pressure =0;
			}
			AddPolygonPoint(m_index-1, event->x, event->y, pressure * 16);
		}
	}
    return TRUE;
}


gint cb_button_press_event( GtkWidget *widget, GdkEventMotion *event, gpointer user_data )
{
	isDown = 1;
	m_points[m_index] = m_pointX[m_index][0] = m_pointY[m_index][0] = 0;
	m_index++;
    return TRUE;
}

gint cb_button_release_event( GtkWidget *widget, GdkEventMotion *event, gpointer user_data )
{
	isDown = 0;
	{ int in,pnt; //Draw line on offscreen 
			cairo_t *qp = cairo_create(offscreen);
			cairo_set_line_width(qp, 0.);
			cairo_set_source_rgb (qp, 0., 1., 0.);
			cairo_new_sub_path (qp);
			
			in = m_index-1;
			for(pnt=0; pnt< m_points[in]; pnt++){ 
					MakeBrushLine(qp, in, pnt);
			}
			
			cairo_fill (qp);
			cairo_destroy(qp);    
			gdk_window_invalidate_rect(gtk_widget_get_window(window),NULL,FALSE);
	}
    return TRUE;
}

gint cb_scroll_event( GtkWidget *widget, GdkEventScroll *event, gpointer user_data )
{
	switch ( event->direction ) { 
	case GDK_SCROLL_DOWN:	whDelta += 10; break;	
	case GDK_SCROLL_UP:	    whDelta -= 10; break;	
	}
	gdk_window_invalidate_rect(gtk_widget_get_window(window),NULL,FALSE);
	return TRUE;
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(window, "motion_notify_event", G_CALLBACK( cb_motion_notify_event ), NULL );
    g_signal_connect(window, "button_press_event", G_CALLBACK( cb_button_press_event ), NULL );
    g_signal_connect(window, "button_release_event", G_CALLBACK( cb_button_release_event ), NULL );
	g_signal_connect(window, "draw", G_CALLBACK(cb_expose_event), NULL);
	g_signal_connect(window, "scroll_event",G_CALLBACK( cb_scroll_event ), NULL );
    gtk_widget_set_events( window, GDK_EXPOSURE_MASK 
      | GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
      | GDK_SCROLL_MASK 
      );


    gtk_widget_set_app_paintable(window, TRUE);
    gtk_widget_show(window);
    gtk_main();

	if(offscreen != NULL){
		cairo_surface_destroy(offscreen);
	}
    return 0;
}
