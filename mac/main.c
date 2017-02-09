//
//  main.c
//  wacom9va
//
//  Created by user on 17/02/01.
//  Copyright __EVA*Project__ 2017. All rights reserved.
//

#include <Carbon/Carbon.h>

#define kCSkDocViewClassID	CFSTR( "com.9vae.wacomtest.view" )


static OSStatus        HandleNewWindow();
static OSStatus        WindowEventHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
static IBNibRef        sNibRef;

enum { 
    kDocumentViewSignature	= 'CskV',
    kDocumentBoundsParam	= 'Boun'
};


#define Max(x,y)        ((x)>=(y) ? (x) : (y))
#define Abs(x)          ((x)>=0   ? (x) : -(x))

struct CGrgba {
    CGFloat   r;
    CGFloat   g;
    CGFloat   b;
    CGFloat   a;
};
typedef struct CGrgba CGrgba;

struct CanvasData {
    HIViewRef		theView;
};
typedef struct CanvasData   CanvasData;

HIViewRef			theScrollView;


Rect 	m_bounds;			// Bounding rectangle of window
int		m_index = 0;		// Number of input strokes
int		m_press;			// Pressure
int		m_scrollY = 0;		// Scroll Offset (Changed by mouse wheel)
int		isDown = 0;			// 1= mouse button down 0= up

// Stroke data
#define AnLINEMAX  1000  	//Maximum strokes number 
#define AnLPNTMAX  250  	//Maximum points number in one stroke

int		m_points[AnLINEMAX];				// Number of points
int		m_pointX[AnLINEMAX][AnLPNTMAX];		// x
int		m_pointY[AnLINEMAX][AnLPNTMAX];		// y
int		m_pressure[AnLINEMAX][AnLPNTMAX];	// pressure

CGMutablePathRef	m_path;


//--------------------------------------------------------------------
void	 asSetPoint(int pnt, int x, int y)
{
	if(!pnt) CGPathMoveToPoint(m_path , NULL, x, y +m_scrollY);
	else     CGPathAddLineToPoint(m_path , NULL, x, y +m_scrollY);
}

//--------------------------------------------------------------------
void	 MakeBrushLine(int in, int pnt)
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
		y1 = m_pointY[in][pnt-1]; if(Abs(y1-y2)>m_bounds.bottom/2) break;
		w1 = m_pressure[in][pnt-1];
		w1 = Max(1,w1);
		v1 = (w1*1732/2000);u1=Max(1,w1/2);
		qq = atan2((double)y2-y1,(double)x2-x1);
		for(;qq<0;) qq+=2*aPI;
		dir = (int)((qq+pi8) / pi8/2);
		switch(dir){ //no break in following lines
			case 0: asSetPoint(kk, x1   , y1+w1); if(++kk==7) break;
			case 1: asSetPoint(kk, x1-u1, y1+v1); if(++kk==7) break;
			case 2: asSetPoint(kk, x1-v1, y1+u1); if(++kk==7) break;
			case 3: asSetPoint(kk, x1-w1, y1   ); if(++kk==7) break;
			case 4: asSetPoint(kk, x1-v1, y1-u1); if(++kk==7) break;
			case 5: asSetPoint(kk, x1-u1, y1-v1); if(++kk==7) break;
			case 6: asSetPoint(kk, x1   , y1-w1); if(++kk==7) break;
			case 7: asSetPoint(kk, x1+u1, y1-v1); if(++kk==7) break;
			case 8: asSetPoint(kk, x1+v1, y1-u1); if(++kk==7) break;
			case 9: asSetPoint(kk, x1+w1, y1   ); if(++kk==7) break;
			case 10:asSetPoint(kk, x1+v1, y1+u1); if(++kk==7) break;
			default:asSetPoint(kk, x1+u1, y1+v1); if(++kk==7) break;
				asSetPoint(kk, x1   , y1+w1); if(++kk==7) break;
				asSetPoint(kk, x1-u1, y1+v1); if(++kk==7) break;
				asSetPoint(kk, x1-v1, y1+u1); if(++kk==7) break;
				asSetPoint(kk, x1-w1, y1   ); if(++kk==7) break;
				asSetPoint(kk, x1-v1, y1-u1); if(++kk==7) break;
				asSetPoint(kk, x1-u1, y1-v1); if(++kk==7) break;
				asSetPoint(kk, x1   , y1-w1); if(++kk==7) break;
		}
		break;
	}
	switch(dir){  //no break in following lines
			case 0: asSetPoint(kk, x2,    y2-w2); if(++kk==14) break;
			case 1: asSetPoint(kk, x2+u2, y2-v2); if(++kk==14) break;
			case 2: asSetPoint(kk, x2+v2, y2-u2); if(++kk==14) break;
			case 3: asSetPoint(kk, x2+w2, y2)   ; if(++kk==14) break;
			case 4: asSetPoint(kk, x2+v2, y2+u2); if(++kk==14) break;
			case 5: asSetPoint(kk, x2+u2, y2+v2); if(++kk==14) break;
			case 6: asSetPoint(kk, x2,    y2+w2); if(++kk==14) break;
			case 7: asSetPoint(kk, x2-u2, y2+v2); if(++kk==14) break;
			case 8: asSetPoint(kk, x2-v2, y2+u2); if(++kk==14) break;
			case 9: asSetPoint(kk, x2-w2, y2)   ; if(++kk==14) break;
			case 10:asSetPoint(kk, x2-v2, y2-u2); if(++kk==14) break;
			default:asSetPoint(kk, x2-u2, y2-v2); if(++kk==14) break;
				asSetPoint(kk, x2,    y2-w2); if(++kk==14) break;
				asSetPoint(kk, x2+u2, y2-v2); if(++kk==14) break;
				asSetPoint(kk, x2+v2, y2-u2); if(++kk==14) break;
				asSetPoint(kk, x2+w2, y2)   ; if(++kk==14) break;
				asSetPoint(kk, x2+v2, y2+u2); if(++kk==14) break;
				asSetPoint(kk, x2+u2, y2+v2); if(++kk==14) break;
				asSetPoint(kk, x2,    y2+w2); if(++kk==14) break;
	}
}

//--------------------------------------------------------------------
CGColorSpaceRef GetGenericRGBColorSpace(void)
{
    static CGColorSpaceRef genericRGBColorSpace = NULL;
    if (genericRGBColorSpace == NULL){
		genericRGBColorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    }
    return genericRGBColorSpace;
}


//--------------------------------------------------------------------
void DrawPage(CGContextRef ctx)
{    
   	int in,pnt;
	const CGrgba blackColor	    = { 0.0, 0.0, 0.0, 1.0 };
    CGColorSpaceRef genericColorSpace  = GetGenericRGBColorSpace();
	
	CGContextSaveGState(ctx);	// because SetContextStateForDrawObject is doing what it says it will
	CGContextSetFillColorSpace(ctx, genericColorSpace); 
	CGContextSetStrokeColor( ctx, (CGFloat*)&blackColor);		
	CGContextSetFillColor( ctx, (CGFloat*)&blackColor);		
	CGContextBeginPath(ctx);			// reset current path to empty
	for(in=0; in < m_index; in++){
		m_path = CGPathCreateMutable();
		for(pnt=0; pnt< m_points[in]; pnt++){ 
			MakeBrushLine(in, pnt);
		}
		CGContextAddPath(ctx, m_path);
	}
	CGContextDrawPath(ctx, kCGPathFillStroke);
	CGContextRestoreGState(ctx);	// undo the changes for the specific obj drawing
}

//--------------------------------------------------------------------
void AddPolygonPoint(int in, int x, int y, int prs)
{    
	if( in < AnLINEMAX ){
		int pnt = m_points[in];
		if( pnt < AnLPNTMAX ){
			m_pointX[in][pnt] = x;
			m_pointY[in][pnt] = y;
			m_pressure[in][pnt] = prs;
			m_points[in]++;
		}
	}
}

//--------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    OSStatus                    err;
    err = CreateNibReference( CFSTR("main"), &sNibRef );
    require_noerr( err, CantGetNibRef );
    
    // Create a new window. 
    HandleNewWindow();
    
    // Run the event loop
    RunApplicationEventLoop();

CantGetNibRef:
    return err;
}


//--------------------------------------------------------------------------------------------
DEFINE_ONE_SHOT_HANDLER_GETTER( WindowEventHandler )

//--------------------------------------------------------------------------------------------
static OSStatus
HandleNewWindow()
{
    OSStatus     err;
    WindowRef    window;
    static HIObjectClassRef sMyViewClassRef = NULL;
    EventRef	event;
    HIViewRef	theView;
    HIViewRef contentView;
    OptionBits options = kHIScrollViewOptionsVertScroll | kHIScrollViewOptionsHorizScroll | kHIScrollViewOptionsAllowGrow;
    const HIViewID viewID = { kDocumentViewSignature, 0 };
	
	
    static const EventTypeSpec    kWindowEvents[] =
    {
        { kEventClassCommand, kEventCommandProcess },

		{ kEventClassHIObject, kEventHIObjectConstruct },
		{ kEventClassHIObject, kEventHIObjectInitialize },
		{ kEventClassHIObject, kEventHIObjectDestruct },
        { kEventClassControl, kEventControlDraw },
		
		{ kEventClassMouse,		kEventMouseWheelMoved },
		{ kEventClassMouse,		kEventMouseDown },
		{ kEventClassMouse,		kEventMouseDragged },
		{ kEventClassMouse,		kEventMouseMoved },
		{ kEventClassMouse,		kEventMouseUp }							
    };
    
    // Create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib( sNibRef, CFSTR("MainWindow"), &window );
    require_noerr( err, CantCreateWindow );

    // Install a command handler on the window. We don't use this handler yet, but nearly all
    // Carbon apps will need to handle commands, so this saves everyone a little typing.
    InstallWindowEventHandler( window, GetWindowEventHandlerUPP(),
                               GetEventTypeCount( kWindowEvents ), kWindowEvents,
                               window, NULL );
    // Position new windows in a staggered arrangement on the main screen
    RepositionWindow( window, NULL, kWindowCascadeOnMainScreen );
	GetWindowBounds(window,kWindowContentRgn,&m_bounds);			
	
	
	
	//Create HIView
    // Make scroll view
    err = HIScrollViewCreate(options, &theScrollView);
    require(err == noErr, CantCreateScrollView);
	
    // Bind it to the window's contentView
    HIRect bounds;
    HIViewFindByID(HIViewGetRoot(window), kHIViewWindowContentID, &contentView);
    HIViewAddSubview(contentView, theScrollView);
    HIViewGetBounds(contentView, &bounds);
    HIViewSetFrame(theScrollView, &bounds);
    HIViewSetVisible(theScrollView, true);
	
    err = HIObjectRegisterSubclass( kCSkDocViewClassID,
								   kHIViewClassID,			// base class ID
								   0,					// option bits
								   WindowEventHandler,		// construct proc
								   GetEventTypeCount( kWindowEvents ),
								   kWindowEvents,
								   NULL,				// construct data,
								   &sMyViewClassRef );

    // Make an initialization event
    err = CreateEvent( NULL, kEventClassHIObject, kEventHIObjectInitialize, GetCurrentEventTime(), 0, &event ); 
    require_noerr( err, CantCreateEvent );
	
    err = SetEventParameter(event, kDocumentBoundsParam, typeQDRectangle, sizeof(Rect), &m_bounds);
    require_noerr( err, CantSetParameter );

    err = HIObjectCreate(kCSkDocViewClassID, event, (HIObjectRef*)&theView);
    require_noerr(err, CantCreate);
    err = HIViewAddSubview(theScrollView, theView);
	
	SetControlID(theView, &viewID);
	HIViewSetVisible(theView, true);

    // The window was created hidden, so show it
    ShowWindow( window );
    
CantCreate:
CantSetParameter:
CantCreateEvent:
    ReleaseEvent( event );	
	
CantCreateScrollView:
CantCreateWindow:
    return err;
}

//--------------------------------------------------------------------------------------------
static OSStatus
WindowEventHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inUserData )
{
    OSStatus    err = eventNotHandledErr;
    unsigned long	ekind;
	SInt32 		whDelta = 0;
	EventRecord eve;
	WindowRef	wptr;
	Point   	where;
	UInt32  	modifiers;
	CGContextRef ctx;
	CanvasData	*data = (CanvasData	*)inUserData;
	
    ekind = GetEventKind(inEvent);
	
    switch ( GetEventClass( inEvent ) ){	
			
	case kEventClassHIObject:   // the boilerplate HIObject business
		switch ( ekind ){
		case kEventHIObjectConstruct:
			data = (CanvasData*)malloc(sizeof(CanvasData));	
			err = GetEventParameter( inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL, sizeof(HIObjectRef*), NULL, &data->theView );
			require_noerr( err, ParameterMissing );
			SetEventParameter( inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof(CanvasData), &data ); 
			break;
				
		case kEventHIObjectDestruct:
			free(inUserData);
			break;
		}
		break;	// kEventClassHIObject
			
			
	case kEventClassControl:    // draw, hit test and track
		switch ( ekind ){
		case kEventControlDraw:
			GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(CGContextRef), NULL, &ctx);
			//CallNextEventHandler(inCallRef, inEvent);	    // Erase old content
			DrawPage(ctx);
			err = noErr;
			break;
		}			
		break; //kEventClassControl
		
			
    case kEventClassMouse:
    	{ // Get Wacom Event	 
			UInt32 eventType;
			struct TabletPointRec tabletPnt; 
			struct TabletProximityRec tabletTyp;
			
			err = GetEventParameter(inEvent, 
							kEventParamTabletEventType, typeUInt32, NULL,
							sizeof(eventType), NULL, &eventType);
			if(err==noErr){ 
				switch( eventType ){
				case kEventTabletPoint:
					err = GetEventParameter(inEvent, 
							kEventParamTabletPointRec, typeTabletPointRec, NULL,
							sizeof(tabletPnt), NULL, &tabletPnt);
					if(err==noErr){
							m_press = tabletPnt.pressure;
					}
					break;
				case kEventTabletProximity:
					err = GetEventParameter(inEvent, 
							kEventParamTabletProximityRec, typeTabletProximityRec, NULL,
							sizeof(tabletTyp), NULL, &tabletTyp);
					if(err==noErr){
					}
					break;
				}
			}
		}//Wacom Event
		switch( ekind ){
		case kEventMouseWheelMoved:
			err = GetEventParameter(inEvent, kEventParamMouseWheelDelta, typeSInt32, NULL, sizeof(SInt32), NULL, &whDelta);
			m_scrollY += whDelta*4;
			/*no break*/
		case kEventMouseUp:
		case kEventMouseDown:
		case kEventMouseDragged:
		case kEventMouseMoved:
			if(ekind==kEventMouseDown){
				isDown = 1;
				m_points[m_index] = m_pointX[m_index][0] = m_pointY[m_index][0] = 0;
				m_index++;
			}
			if(ekind==kEventMouseUp){
				isDown = 0;
				m_press = 0;
			}
			ConvertEventRefToEventRecord(inEvent, &eve);

			if(FindWindow(eve.where, &wptr)!=inContent && ekind!=kEventMouseUp){
				break;
			}
			if(wptr != ActiveNonFloatingWindow()&& ekind!=kEventMouseUp){
				break;
			}
			GetEventParameter(inEvent, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &where);
			GetEventParameter(inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &modifiers);
			if(isDown){
				AddPolygonPoint(m_index-1, where.h, 
								where.v - m_scrollY -m_bounds.top, 15*m_press/0x10000);
			}
			if(whDelta || isDown){
				whDelta=0;
				HIViewSetNeedsDisplay(theScrollView, true);				
			}
			break;
		}		   
		break; //kEventClassMouse
   	default:
    	break;
    }
ParameterMissing:	
    return err;
}
