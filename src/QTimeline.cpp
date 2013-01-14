/*
 *  QTimeline.cpp
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#include "QTimeline.h"
#include "cinder/Text.h"
#include <fstream>

using namespace ci;
using namespace ci::app;
using namespace std;


ColorA          QTimeline::mTimeBarBgCol            = ColorA( 0.10f, 0.10f, 0.10f, 1.00f );
ColorA          QTimeline::mTimeBarFgCol            = ColorA( 1.00f, 1.00f, 1.00f, 0.20f );
ColorA          QTimeline::mTimeBarTextCol          = ColorA( 1.00f, 1.00f, 1.00f, 1.00f );
ColorA          QTimeline::mTimeBarMouseBarCol     = ColorA( 1.00f, 1.00f, 1.00f, 1.00f );
ColorA          QTimeline::mTimeBarModuleRangeCol  = ColorA( 1.00f, 1.00f, 1.00f, 1.00f );

ColorA          QTimeline::mTransportBgCol          = ColorA( 0.10f, 0.10f, 0.10f, 1.00f );
ColorA          QTimeline::mTransportTextCol        = ColorA( 1.00f, 1.00f, 1.00f, 1.00f );

ColorA          QTimeline::mTracksBgCol             = ColorA( 0.00f, 0.00f, 0.00f, 1.00f );
ColorA          QTimeline::mTracksBgOverCol         = ColorA( 0.00f, 0.00f, 0.00f, 1.00f );

ColorA          QTimeline::mModuleItemBgCol         = ColorA( 0.25f, 0.25f, 0.25f, 1.00f );
ColorA          QTimeline::mModuleItemBgOverCol     = ColorA( 0.25f, 0.25f, 0.25f, 1.00f );
ColorA          QTimeline::mModuleItemTextCol       = ColorA( 1.00f, 1.00f, 1.00f, 1.00f );
ColorA          QTimeline::mModuleItemHandleCol     = ColorA( 1.00f, 1.00f, 1.00f, 0.15f );
ColorA      	QTimeline::mModuleItemHandleOverCol = ColorA( 1.00f, 1.00f, 1.00f, 0.30f );

ColorA          QTimeline::mAudioItemBgCol          = ColorA( 0.25f, 0.25f, 0.25f, 1.00f );
ColorA          QTimeline::mAudioItemBgOverCol      = ColorA( 0.25f, 0.25f, 0.25f, 1.00f );
ColorA          QTimeline::mAudioItemTextCol        = ColorA( 1.00f, 1.00f, 1.00f, 1.00f );
ColorA          QTimeline::mAudioItemHandleCol      = ColorA( 1.00f, 1.00f, 1.00f, 0.15f );
ColorA      	QTimeline::mAudioItemHandleOverCol  = ColorA( 1.00f, 1.00f, 1.00f, 0.30f );
                                                     
ColorA          QTimeline::mParamsBgCol             = ColorA( 0.15f, 0.15f, 0.15f, 1.00f );
ColorA          QTimeline::mParamsBgOverCol         = ColorA( 0.15f, 0.15f, 0.15f, 1.00f );
ColorA          QTimeline::mParamsTextCol           = ColorA( 1.00f, 1.00f, 1.00f, 1.00f );

ColorA          QTimeline::mKeyframesBgCol          = ColorA( 1.00f, 1.00f, 1.00f, 0.40f );
ColorA          QTimeline::mKeyframesBgOverCol      = ColorA( 1.00f, 1.00f, 1.00f, 0.80f );
ColorA          QTimeline::mKeyframesGraphCol       = ColorA( 1.00f, 1.00f, 1.00f, 0.40f );
ColorA          QTimeline::mKeyframesBgSelectedCol  = ColorA( 1.00f, 0.40f, 0.40f, 1.00f );

ColorA          QTimeline::mCueListBgCol            = ColorA( 1.00f, 1.00f, 1.00f, 1.00f );
ColorA          QTimeline::mCueBgCol                = ColorA( 0.25f, 0.25f, 0.25f, 1.00f );
ColorA          QTimeline::mCueBgOverCol            = ColorA( 0.25f, 0.25f, 0.25f, 1.00f );
ColorA          QTimeline::mCueFgCol                = ColorA( 0.25f, 0.25f, 0.25f, 1.00f );
ColorA          QTimeline::mCueTextCol              = ColorA( 1.00f, 1.00f, 1.00f, 1.00f );
ColorA          QTimeline::mCueHandleCol            = ColorA( 1.00f, 1.00f, 1.00f, 0.15f );
ColorA          QTimeline::mCueHandleOverCol        = ColorA( 1.00f, 1.00f, 1.00f, 0.30f );

vector<ColorA>  QTimelineMenuColorPalette::mColors;

QTimeline*      QTimeline::thisRef;    // would be better to use shared_ptr and shared_from_this(), however it seems not working with the static variable.


void QTimeline::init()
{
    QTimeline::thisRef      = this;
    
    mApp                    = ci::app::App::get();
    
    registerCallbacks();
    
    // initialise AT LEAST one color for the menu color palette
    QTimelineMenuColorPalette::mColors.push_back( ci::ColorA( 0.86f, 0.18f, 0.11f, 1.0f ) );
    QTimelineMenuColorPalette::mColors.push_back( ci::ColorA( 1.00f, 0.34f, 0.00f, 1.0f ) );
    QTimelineMenuColorPalette::mColors.push_back( ci::ColorA( 0.86f, 0.62f, 0.00f, 1.0f ) );
    QTimelineMenuColorPalette::mColors.push_back( ci::ColorA( 0.00f, 0.65f, 0.58f, 1.0f ) );
    QTimelineMenuColorPalette::mColors.push_back( ci::ColorA( 0.45f, 0.60f, 0.00f, 1.0f ) );
    QTimelineMenuColorPalette::mColors.push_back( ci::ColorA( 0.50f, 0.50f, 0.50f, 1.0f ) );

    mTimeline = ci::Timeline::create();
    
    mTransportRect  = Rectf( 0, getWindowHeight() - TIMELINE_TRANSPORT_HEIGHT, getWindowWidth(), getWindowHeight() );
    
    mFontSmall              = ci::gl::TextureFont::create( ci::Font( "Helvetica", 12 ) );
    mFontMedium             = ci::gl::TextureFont::create( ci::Font( "Helvetica", 14 ) );
    mFontBig                = ci::gl::TextureFont::create( ci::Font( "Helvetica", 16 ) );
    
    mZoom                   = 1.0f;
    
    mIsVisible              = true;
    
    mMouseDragTimeBar       = false;
    mMouseOnTimeBar         = false;
    
    mSelectedMenu           = NULL;
    
    mCueManager             = new QTimelineCueManager();
    
    // create default track
    mTracks.push_back( QTimelineTrackRef( new QTimelineTrack( "track 0" ) ) );
    
    play( false, FREE_RUN );
    
    mRenderDebug            = true;
    
    mRenderHelp             = false;
    
    TextLayout layout;
	layout.clear( ColorA( 0.0f, 0.0f, 0.0f, 0.8f ) );
    layout.setBorder( 5, 5 );
	layout.setFont( ci::Font( "Helvetica", 12 ) );
	layout.setColor( Color( 0.0f, 0.8f, 0.8f ) );
    layout.setLeadingOffset( 3 );
	layout.addLine( "HELP\n\n" );
	layout.addLine( "? \t\t\t\t toggle help\n" );
	layout.addLine( "space_bar \t\t play/pause FREE RUN mode\n" );
	layout.addLine( "return \t\t\t play/pause CUE LIST mode\n" );
	layout.addLine( "delete \t\t\t set time to 0\n\n" );
    mHelpTex = gl::Texture( layout.render( true ) );
    
    updateTime();
    
    updateTimeWindow();

    // init BASS library
    BASS_Init( -1, 44100, 0, 0, NULL );
}


void QTimeline::update()
{
    eraseMarkedItems();
    
    updateTime();
    
    updateTimeWindow();
    
    double t = getTime();
    mTimeNorm   = ( t - mTimeWindow.x ) / ( mTimeWindow.y - mTimeWindow.x );
    char buff[100];
    sprintf( buff, "%02d:%02d:%02d", (int)( t / 3600.0f ), (int)( t / 60.0f ), (int)t % 60 );
    mTimeStr    = buff;    
}


void QTimeline::render()
{
    if ( !mIsVisible )
        return;
    
    // render transport
    gl::color( mTransportBgCol );
    gl::drawSolidRect( mTransportRect );
  
    gl::color( mTransportTextCol );
    mFontBig->drawString( mTimeStr, mTransportRect.getUpperRight() + Vec2f( -125, 19 ) );
    mFontSmall->drawString( toString( (int)App::get()->getAverageFps() ) + " FPS", mTransportRect.getUpperRight() + Vec2f( -50, 19 ) );

    // render tracks
    mTracksRect = mTransportRect;
    mTracksRect.offset( Vec2f( 0, - TIMELINE_WIDGET_PADDING ) );
    for( int k=mTracks.size()-1; k >= 0; k-- )
        mTracksRect = mTracks[k]->render( mTracksRect, mTimeWindow, getTime() );
    
    // render Time bar
    renderTimeBar();
    
    // render Cue list
    if ( mCueManager )
    {
        mCueManager->setRect( Rectf( mTimeBarRect.x1, mTimeBarRect.y1 - TIMELINE_CUELIST_HEIGHT - 1, mTimeBarRect.x2, mTimeBarRect.y1 - 1 ) );
        mCueManager->render();
    }
    
    if ( mSelectedMenu )
        mSelectedMenu->render();

    gl::color( Color::white() );
    
    // render debug info
    if ( mRenderDebug )
        renderDebugInfo();
    
    if( mRenderHelp )
        gl::draw( mHelpTex, Vec2i( getWindowWidth() - mHelpTex.getWidth() - 100, 15 ) );
}


void QTimeline::registerCallbacks()
{
    mCbMouseDown    = mApp->registerMouseDown(  this, &QTimeline::mouseDown );
    mCbMouseUp      = mApp->registerMouseUp(    this, &QTimeline::mouseUp );
    mCbMouseMove    = mApp->registerMouseMove(  this, &QTimeline::mouseMove );
    mCbMouseDrag    = mApp->registerMouseDrag(  this, &QTimeline::mouseDrag );
    mCbMouseWheel   = mApp->registerMouseWheel( this, &QTimeline::mouseWheel );
    mCbKeyDown      = mApp->registerKeyDown( this, &QTimeline::keyDown );
    mCbResize       = mApp->registerResize(     this, &QTimeline::resize );
}


bool QTimeline::mouseDown( ci::app::MouseEvent event )
{
    if ( mSelectedMenu )
    {
        if ( mSelectedMenu->isVisible() && mSelectedMenu->mouseDown(event) )
            return true;
        else
        {
            mSelectedMenu->close();
            mSelectedMenu = NULL;
            return true;
        }
    }
    
    for( size_t k=0; k < mTracks.size(); k++ )
        mTracks[k]->mouseDown( event );
    
    mCueManager->mouseDown( event );
    
    // time bar handler
    mMouseDragTimeBar = mTimeBarRect.contains( event.getPos() );
    
    mouseDrag( event );
    
    return false;
}


bool QTimeline::mouseUp( ci::app::MouseEvent event )
{
    for( size_t k=0; k < mTracks.size(); k++ )
        mTracks[k]->mouseUp( event );
    
    mCueManager->mouseUp( event );
    
    mMouseDragTimeBar = false;
    
    return false;
}


bool QTimeline::mouseMove( ci::app::MouseEvent event )
{
    mMouseOnTrack.reset();
    
    if ( mSelectedMenu && mSelectedMenu->isVisible() && mSelectedMenu->mouseMove(event) )
        return true;
    
    for( size_t k=0; k < mTracks.size(); k++ )
        if ( mTracks[k]->mouseMove( event ) )
            mMouseOnTrack = mTracks[k];
    
    mCueManager->mouseMove( event );
    
    mMousePrevPos = event.getPos();
    
    mMouseOnTimeBar = mTimeBarRect.contains( event.getPos() );
    
    return false;
}


bool QTimeline::mouseDrag( MouseEvent event )
{
    if ( mSelectedMenu && mSelectedMenu->isVisible() && mSelectedMenu->mouseDrag(event) )
        return true;
    
    for( size_t k=0; k < mTracks.size(); k++ )
        mTracks[k]->mouseDrag( event );
    
    mCueManager->mouseDrag( event );
    
    // time bar handler
    dragTimeBar( event.getPos().x );
    
    mMousePrevPos = event.getPos();
    
    return false;
}


bool QTimeline::keyDown( KeyEvent event )
{
    int code = event.getCode();
    
    if ( code == KeyEvent::KEY_ESCAPE )
    {
        closeMenu();
        return true;
    }
    
    if ( mSelectedMenu && mSelectedMenu->isVisible() && mSelectedMenu->keyDown(event) )
        return true;
    
    return false;
}


bool QTimeline::mouseWheel( MouseEvent event )
{
    if ( !event.isAltDown() )
        return false;
    
    // update zoom
    float incr  = ( event.getWheelIncrement() > 0 ) ? 0.05f : -0.05f;
    mZoom       = (int)( ( mZoom + incr ) * 100.0f + 0.5f ) / 100.0f;   // +0.5f fix floating point madness
    mZoom       = math<float>::clamp( mZoom, 0.25f, 4.0f );
    
    // update items labels
    vector<QTimelineItemRef> items;
    for( size_t k=0; k < mTracks.size(); k++ )
    {
        items = mTracks[k]->getItems();
        for( size_t j=0; j<items.size(); j++ )
            items[j]->updateLabel();
    }
    
    // update cue list labels
    vector<QTimelineCueRef> cueList = mCueManager->getCueList();
    for( size_t k=0; k < cueList.size(); k++ )
        cueList[k]->updateLabel();
    
    return false;
}


bool QTimeline::resize( ci::app::ResizeEvent event )
{
    mTransportRect  = Rectf( 0, getWindowHeight() - TIMELINE_TRANSPORT_HEIGHT, getWindowWidth(), getWindowHeight() );
    return false;
}


void QTimeline::updateTimeWindow()
{
    float   windowInSec = ( TIMELINE_WINDOW_BEATS * 60.0f / TIMELINE_BPM ) / mZoom;
    int     windowN     = mTimeline->getCurrentTime() / windowInSec;
    
    mTimeWindow.x       = windowN * windowInSec;
    mTimeWindow.y       = mTimeWindow.x + windowInSec;
}


void QTimeline::updateTime()
{
    if ( isPlaying() )
    {
        double newTime = mTimeline->getCurrentTime() + getElapsedSeconds() - mPrevElapsedSeconds;
        
        if ( mPlayMode == FREE_RUN )
            mTimeline->stepTo( newTime );
        
        else if ( mPlayMode == CUE_LIST )
        {
            if ( mCueManager->isTimeOnCue() )
                mTimeline->stepTo( min( newTime, mCueManager->getCueEndTime() ) );
            else
                play( false, CUE_LIST );
        }
    }

    mPrevElapsedSeconds   = getElapsedSeconds();
}


void QTimeline::updateCurrentTime()
{
    double current_time = mTimeline->getCurrentTime();
    mTimeline->stepTo(current_time != 0.0 ? 0.0 : 1.0);
    mTimeline->stepTo(current_time);
}


void QTimeline::play( bool play, PlayMode mode )
{
    mPrevElapsedSeconds = getElapsedSeconds();
    mIsPlaying          = play;
    mPlayMode           = mode;
    
    callItemsOnTimeChange();
    
    addOscMessage( OSC_ADDRESS_PLAY, "i" + toString(mPlayMode) + " i" + toString(mIsPlaying) );
}


void QTimeline::playCue( int cueN )
{
    // OR cue != current cue ?
    if ( isPlaying() )
        play( false, CUE_LIST );
    
    else if ( mCueManager->playCue( cueN ) )
    {
        mTimeline->stepTo( mCueManager->getCueStartTime() );
        play( true, CUE_LIST );
    }
}


void QTimeline::renderTimeBar()
{
    // render time bar
    float timelinePosX = mTimeBarRect.getWidth() * mTimeNorm;
    mTimeBarRect  = Rectf( mTracksRect.getUpperLeft() - Vec2f( 0, TIMELINE_BAR_HEIGHT ), mTracksRect.getUpperRight() );

    glBegin( GL_QUADS );
    
    gl::color( QTimeline::mTimeBarBgCol );
    gl::vertex( mTimeBarRect.getUpperLeft() );
    gl::vertex( mTimeBarRect.getUpperRight() );
    gl::vertex( mTimeBarRect.getLowerRight() );
    gl::vertex( mTimeBarRect.getLowerLeft() );
    
    Rectf barOver( Rectf( mTimeBarRect.getUpperLeft(), mTimeBarRect.getLowerLeft() + Vec2f( timelinePosX, 0 ) ) );
    gl::color( QTimeline::mTimeBarFgCol );
    gl::vertex( barOver.getUpperLeft() );
    gl::vertex( barOver.getUpperRight() );
    gl::vertex( barOver.getLowerRight() );
    gl::vertex( barOver.getLowerLeft() );
    
    // render current time bar indicator
    
    gl::color( QTimeline::mTimeBarMouseBarCol );
    gl::vertex( Vec2f( timelinePosX,    mTimeBarRect.getY1() ) );
    gl::vertex( Vec2f( timelinePosX+1,  mTimeBarRect.getY1() ) );
    gl::vertex( Vec2f( timelinePosX+1,  mTransportRect.getY1() ) );
    gl::vertex( Vec2f( timelinePosX,    mTransportRect.getY1() ) );
    
    glEnd();
    
    // render beat and bar labels
    float   windowInSec     = ( TIMELINE_WINDOW_BEATS * 60.0f / TIMELINE_BPM ) / mZoom;
    int     nSecs           = windowInSec + 1;
    float   oneSecInPx      = mTimeBarRect.getWidth() / windowInSec;
    int     incr            = ( oneSecInPx < 20 ) ? 4 : 1;
    ColorA  colMarked       = QTimeline::mTimeBarFgCol; colMarked.a *= 2.5f;
    glBegin( GL_QUADS );
    for( int k=0; k < nSecs; k+=incr )
    {
        if ( k%4 == 0 )
            gl::color( colMarked );
        else
            gl::color( QTimeline::mTimeBarFgCol );
            
        gl::vertex( Vec2f( (int)(k * oneSecInPx),   mTimeBarRect.y1 ) );
        gl::vertex( Vec2f( (int)(k * oneSecInPx)+1, mTimeBarRect.y1 ) );
        gl::vertex( Vec2f( (int)(k * oneSecInPx)+1, mTimeBarRect.y2 ) );
        gl::vertex( Vec2f( (int)(k * oneSecInPx),   mTimeBarRect.y2 ) );
    }
    glEnd();
    
    // render labels
    gl::color( QTimeline::mTimeBarTextCol );
    for( int k=0; k < nSecs; k+=4 )
    mFontSmall->drawString( toString( mTimeWindow.x + k ),  Vec2f( k * oneSecInPx + 3, mTimeBarRect.y2 - 3 ) );
    
    // render mouse bar
    if ( mMouseOnTrack || mMouseOnTimeBar )
    {
        float posX = snapPos( mMousePrevPos.x );
        glBegin( GL_QUADS );
        gl::color( QTimeline::mTimeBarMouseBarCol );
        gl::vertex( Vec2f( posX,        mTimeBarRect.y1 ) );
        gl::vertex( Vec2f( posX + 1,    mTimeBarRect.y1 ) );
        gl::vertex( Vec2f( posX + 1,    mTimeBarRect.y2 ) );
        gl::vertex( Vec2f( posX,        mTimeBarRect.y2 ) );
        glEnd();
    }
    
    // render module range
    if ( mMouseOnTrack && mMouseOnTrack->isMouseOnModule() )
    {
        QTimelineItemRef    ref = mMouseOnTrack->getMouseOnModule();
        Rectf               r   = ref->getRect();
        float               h   = 6;
        
        glBegin( GL_QUADS );

        gl::color( ref->getColor() );

        // left bar
        gl::vertex( Vec2f( r.x1,       mTimeBarRect.y1 ) );
        gl::vertex( Vec2f( r.x1 + 1,   mTimeBarRect.y1 ) );
        gl::vertex( Vec2f( r.x1 + 1,   mTimeBarRect.y2 ) );
        gl::vertex( Vec2f( r.x1,       mTimeBarRect.y2 ) );
        
        // right bar
        gl::vertex( Vec2f( r.x2,       mTimeBarRect.y1 ) );
        gl::vertex( Vec2f( r.x2 - 1,   mTimeBarRect.y1 ) );
        gl::vertex( Vec2f( r.x2 - 1,   mTimeBarRect.y2 ) );
        gl::vertex( Vec2f( r.x2,       mTimeBarRect.y2 ) );

        // line
        gl::vertex( Vec2f( r.x1,       mTimeBarRect.y1 + h ) );
        gl::vertex( Vec2f( r.x2,       mTimeBarRect.y1 + h ) );
        gl::vertex( Vec2f( r.x2,       mTimeBarRect.y1 + h + 1 ) );
        gl::vertex( Vec2f( r.x1,       mTimeBarRect.y1 + h + 1 ) );
        
        glEnd();
    }
    
    gl::color( Color::white() );
}


QTimelineItemRef QTimeline::addModule( float startTime, float duration, string name )
{
    QTimelineTrackRef trackRef( new QTimelineTrack( "Untitled" ) );
    mTracks.push_back( trackRef );
    
    QTimelineItemRef item = trackRef->addModuleItem( startTime, duration, name );
    return item;
}


QTimelineItemRef QTimeline::addModule( QTimelineTrackRef trackRef, float startTime, float duration, string name )
{
    QTimelineItemRef item = trackRef->addModuleItem( startTime, duration, name );
    return item;
}


ci::ColorA QTimeline::getThemeColor( XmlTree tree, string tag )
{
    ColorA col;
    
    if ( tree.hasChild( tag ) )
    {
        tree    = tree.getChild(tag);
        col     = ColorA(   tree.getAttributeValue<float>("r"),
                            tree.getAttributeValue<float>("g"),
                            tree.getAttributeValue<float>("b"),
                            tree.getAttributeValue<float>("a") );
    }
    
    return col;
}


void QTimeline::loadTheme( const fs::path &filepath )
{
    try
    {
        XmlTree theme = XmlTree( loadFile( filepath ) ).getChild("/QTimelineTheme");
        
        // Time bar
        QTimeline::mTimeBarBgCol            = getThemeColor( theme, "TimeBarBgCol" );
        QTimeline::mTimeBarFgCol            = getThemeColor( theme, "TimeBarFgCol" );
        QTimeline::mTimeBarTextCol          = getThemeColor( theme, "TimeBarTextCol" );
        QTimeline::mTimeBarMouseBarCol      = getThemeColor( theme, "TimeBarMouseBarCol" );
        QTimeline::mTimeBarModuleRangeCol   = getThemeColor( theme, "TimeBarModuleRangeCol" );
        
        // Transport
        QTimeline::mTransportBgCol          = getThemeColor( theme, "TransportBgCol" );
        QTimeline::mTransportTextCol        = getThemeColor( theme, "TransportTextCol" );
        
        // Tracks
        QTimeline::mTracksBgCol             = getThemeColor( theme, "TracksBgCol" );
        QTimeline::mTracksBgOverCol         = getThemeColor( theme, "TracksBgOverCol" );
        
        // Module Items
        QTimeline::mModuleItemBgCol         = getThemeColor( theme, "ModuleItemBgCol" );
        QTimeline::mModuleItemBgOverCol     = getThemeColor( theme, "ModuleItemBgOverCol" );
        QTimeline::mModuleItemTextCol       = getThemeColor( theme, "ModuleItemTextCol" );
        QTimeline::mModuleItemHandleCol     = getThemeColor( theme, "ModuleItemHandleCol" );
        QTimeline::mModuleItemHandleOverCol = getThemeColor( theme, "ModuleItemHandleOverCol" );

        // Audio Items
        QTimeline::mAudioItemBgCol          = getThemeColor( theme, "AudioItemBgCol" );
        QTimeline::mAudioItemBgOverCol      = getThemeColor( theme, "AudioItemBgOverCol" );
        QTimeline::mAudioItemTextCol        = getThemeColor( theme, "AudioItemTextCol" );
        QTimeline::mAudioItemHandleCol      = getThemeColor( theme, "AudioItemHandleCol" );
        QTimeline::mAudioItemHandleOverCol  = getThemeColor( theme, "AudioItemHandleOverCol" );

        // Params
        QTimeline::mParamsBgCol             = getThemeColor( theme, "ParamsBgCol" );
        QTimeline::mParamsBgOverCol         = getThemeColor( theme, "ParamsBgOverCol" );
        QTimeline::mParamsTextCol           = getThemeColor( theme, "ParamsTextCol" );
        
        // Keyframes
        QTimeline::mKeyframesBgCol          = getThemeColor( theme, "KeyframesBgCol" );
        QTimeline::mKeyframesBgOverCol      = getThemeColor( theme, "KeyframesBgOverCol" );
        QTimeline::mKeyframesBgSelectedCol  = getThemeColor( theme, "KeyframesBgSelectedCol" );
        QTimeline::mKeyframesGraphCol       = getThemeColor( theme, "KeyframesGraphCol" );
        
        // Cue list
        QTimeline::mCueListBgCol            = getThemeColor( theme, "CueListBgCol" );
        QTimeline::mCueBgCol                = getThemeColor( theme, "CueBgCol" );
        QTimeline::mCueBgOverCol            = getThemeColor( theme, "CueBgOverCol" );
        QTimeline::mCueFgCol                = getThemeColor( theme, "CueFgCol" );
        QTimeline::mCueTextCol              = getThemeColor( theme, "CueTextCol" );
        QTimeline::mCueHandleCol            = getThemeColor( theme, "CueHandleCol" );
        QTimeline::mCueHandleOverCol        = getThemeColor( theme, "CueHandleOverCol" );
        
        // Menu color palette
        QTimelineMenuColorPalette::mColors.clear();
        XmlTree colorPalette = XmlTree( loadFile( filepath ) ).getChild("/QTimelineTheme/MenuColorPalette");
        for( XmlTree::Iter nodeIt = colorPalette.begin("Color"); nodeIt != colorPalette.end(); ++nodeIt )
            QTimelineMenuColorPalette::mColors.push_back( ColorA(   nodeIt->getAttributeValue<float>("r"),
                                                                    nodeIt->getAttributeValue<float>("g"),
                                                                    nodeIt->getAttributeValue<float>("b"),
                                                                    nodeIt->getAttributeValue<float>("a")   ) );
    }
    catch ( ... )
    {
        console() << "ERROR > QTimeline theme NOT loaded!" << endl;
    }
    
    QTimelineTrackRef               trackRef;
    QTimelineItemRef                itemRef;
    QTimelineCueRef                 cueRef;
    std::vector<QTimelineCueRef>    cueList;
    std::vector<QTimelineParamRef>  params;
    
    mCueManager->setBgColor( QTimeline::mCueListBgCol );
    
    for( size_t k=0; k < mTracks.size(); k++ )
    {
        trackRef                = mTracks[k];
        trackRef->mBgColor      = QTimeline::mTracksBgCol;
        trackRef->mBgOverColor  = QTimeline::mTracksBgOverCol;
        
        for( size_t i=0; i < trackRef->mItems.size(); i++ )
        {
            itemRef = trackRef->mItems[i];
            
            if ( itemRef->getType() == "QTimelineModuleItem" )
            {
                itemRef->setBgColor( QTimeline::mModuleItemBgCol );
                itemRef->setBgOverColor( QTimeline::mModuleItemBgOverCol );
                itemRef->setTextColor( QTimeline::mModuleItemTextCol );
                itemRef->setHandleColor( QTimeline::mModuleItemHandleCol );
                itemRef->setHandleOverColor( QTimeline::mModuleItemHandleOverCol );
            }
            
            else if ( itemRef->getType() == "QTimelineAudioItem" )
            {
                itemRef->setBgColor( QTimeline::mAudioItemBgCol );
                itemRef->setBgOverColor( QTimeline::mAudioItemBgOverCol );
                itemRef->setTextColor( QTimeline::mAudioItemTextCol );
                itemRef->setHandleColor( QTimeline::mAudioItemHandleCol );
                itemRef->setHandleOverColor( QTimeline::mAudioItemHandleOverCol );
            }
            
            params = itemRef->getParams();
            
            for( size_t j=0; j < params.size(); j++ )
            {
                params[j]->setBgColor( QTimeline::mParamsBgCol );
                params[j]->setBgOverColor( QTimeline::mParamsBgOverCol );
                params[j]->setTextColor( QTimeline::mParamsTextCol );
                
                params[j]->setKeyframesBgCol( QTimeline::mKeyframesBgCol );
                params[j]->setKeyframesBgOverCol( QTimeline::mKeyframesBgOverCol );
                params[j]->setKeyframesBgSelectedCol( QTimeline::mKeyframesBgSelectedCol );
                params[j]->setKeyframesGraphCol( QTimeline::mKeyframesGraphCol );
            }
        }
    }
    
    cueList = mCueManager->getCueList();
    
    for( size_t k=0; k < cueList.size(); k++ )
    {
        cueRef = cueList[k];
        cueRef->setBgColor( QTimeline::mCueBgCol );
        cueRef->setBgOverColor( QTimeline::mCueBgOverCol );
        cueRef->setFgColor( QTimeline::mCueFgCol );
        cueRef->setTextColor( QTimeline::mCueTextCol );
        cueRef->setHandleColor( QTimeline::mCueHandleCol );
        cueRef->setHandleOverColor( QTimeline::mCueHandleOverCol );
    }
}


void QTimeline::save( const std::string &filename )
{
    XmlTree doc( "QTimeline", "" );

    XmlTree tracks( "tracks", "" );    
    for( size_t k=0; k < mTracks.size(); k++ )
        tracks.push_back( mTracks[k]->getXmlNode() );
    
    doc.push_back( tracks );

    doc.push_back( mCueManager->getXmlNode() );

    doc.write( writeFile( filename ) );
}


void QTimeline::load( const std::string &filename )
{
    clear();
 
    XmlTree doc;
    
    try
    {
        doc = XmlTree( loadFile(filename) );

        
        for( XmlTree::Iter nodeIt = doc.begin("QTimeline/tracks/track"); nodeIt != doc.end(); ++nodeIt )
        {
            string              trackName   = nodeIt->getAttributeValue<string>("name");
            QTimelineTrackRef   trackRef    = QTimelineTrackRef( new QTimelineTrack( trackName ) );
            mTracks.push_back( trackRef );
            trackRef->loadXmlNode( *nodeIt );
        }
        
        mCueManager->loadXmlNode( doc.getChild( "qTimeline/cueList" ) );
    }
    catch ( ... )
    {
        console() << "Error > QTimeline::load(): " << filename << endl;
        return;
    }
    
    updateCurrentTime();
}


void QTimeline::renderDebugInfo()
{    
    Vec2f debugOffset( 15, 50 );
    mFontMedium->drawString( "TIMELINE DEBUG:",                             debugOffset ); debugOffset += Vec2f( 0, 20 );
    mFontMedium->drawString( "Time:\t"      + toString( getTime() ),        debugOffset ); debugOffset += Vec2f( 0, 15 );
    mFontMedium->drawString( "Window:\t"    + toString( mTimeWindow ),      debugOffset ); debugOffset += Vec2f( 0, 15 );
    mFontMedium->drawString( "Zoom:\t"      + toString( mZoom ),            debugOffset ); debugOffset += Vec2f( 0, 15 );
    mFontMedium->drawString( "BPM:\t"       + toString( TIMELINE_BPM ),     debugOffset ); debugOffset += Vec2f( 0, 15 );
    string str = isPlaying() ? "TRUE" : "FALSE";
    mFontMedium->drawString( "PLAY:\t"      + str,                          debugOffset ); debugOffset += Vec2f( 0, 15 );
    str = mPlayMode == FREE_RUN ? "FREE_RUN" : "CUE_LIST";
    mFontMedium->drawString( "MODE:\t"      + str,                          debugOffset ); debugOffset += Vec2f( 0, 15 );
    
    if ( mSelectedMenu )
    {
        str = "MENU OPEN: visible " + toString(mSelectedMenu->mPos) + " " + mSelectedMenu->getName();
        mFontMedium->drawString( str,                                       debugOffset ); debugOffset += Vec2f( 0, 15 );
    }
    
    vector<QTimelineCueRef> cueList = mCueManager->getCueList();
    mFontMedium->drawString( "CUES: " + toString( cueList.size() ),         debugOffset ); debugOffset += Vec2f( 0, 15 );
    
    QTimelineCueRef currentCue = mCueManager->getCurrentCue();
    str = "CUE: ";
    if ( currentCue ) str += currentCue->getName(); else str += "NONE";
    mFontMedium->drawString( str,                                           debugOffset ); debugOffset += Vec2f( 0, 15 );
    
    mFontMedium->drawString( "Timeline modules:\t"   + toString( mTimeline->getNumItems() ),    debugOffset ); debugOffset += Vec2f( 0, 15 );
    mFontMedium->drawString( "Timeline tracks:\t"   + toString( mTracks.size() ),               debugOffset ); debugOffset += Vec2f( 0, 15 );
    
    int j = 0;
    for( size_t k=0; k < mTracks.size();k ++ )
        j += mTracks[k]->mItems.size();
    
    mFontMedium->drawString( "Track modules:\t"   + toString( j ),       debugOffset ); debugOffset += Vec2f( 0, 15 );
}


void QTimeline::eraseMarkedItems()
{
    for( size_t k=0; k < mItemsMarkedForRemoval.size(); k++ )
    {
        QTimelineItemRef    item    = mItemsMarkedForRemoval[k];
        QTimelineTrackRef   track   = item->getParentTrack();
        string              type    = item->getType();
        
        // remove item from the ci::Timeline
        mTimeline->remove( item );                                          // remove() flag the item as erase marked, timeline::stepTo() is in charge to actually delete the item
        updateCurrentTime();                                                // force ci::Timeline to delete all marked items
        
        // remove QTimelineItem from the track
        track->eraseModule( item );
        
        // delete QTimelineModule
        if ( type == "QTimelineModuleItem" )
            callDeleteModuleCb( item );
        
        // destroy params, keyframes and release the QTimelineModule target
        item->clear();
    }
    
    mItemsMarkedForRemoval.clear();
}


void QTimeline::dragTimeBar(float posX )
{
    if ( !mMouseDragTimeBar )
        return;
        
    mTimeline->stepTo( snapTime( getTimeFromPos( posX ) ) );
    
    callItemsOnTimeChange();
}


void QTimeline::callItemsOnTimeChange()
{
    vector<QTimelineItemRef>    items;
    
    for( size_t k=0; k < mTracks.size(); k++ )
    {
        items = mTracks[k]->getItems();
        
        for( size_t i=0; i < items.size(); i++ )
            items[i]->onTimeChange();
    }
}

