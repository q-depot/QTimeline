/*
 *  QTimeline.cpp
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2012 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#include "QTimeline.h"
#include "cinder/Text.h"
#include <fstream>

#define TIMELINE_WINDOW_BEATS       60      // Beats per window / windowInSec = TIMELINE_WINDOW_BEATS * 60.0f / TIMELINE_BPM
#define TIMELINE_BPM                60      // the BMP
#define TIMELINE_BAR_HEIGHT         30
#define TIMELINE_TRANSPORT_HEIGHT   30

using namespace ci;
using namespace ci::app;
using namespace std;


ColorA      QTimeline::mTimeBarBgCol            = ColorA( 0.10f, 0.10f, 0.10f, 1.00f );
ColorA      QTimeline::mTimeBarFgCol            = ColorA( 1.00f, 1.00f, 1.00f, 0.20f );
ColorA      QTimeline::mTimeBarTextCol          = ColorA( 1.00f, 1.00f, 1.00f, 1.00f );

ColorA      QTimeline::mTransportBgCol          = ColorA( 0.10f, 0.10f, 0.10f, 1.00f );
ColorA      QTimeline::mTransportTextCol        = ColorA( 1.00f, 1.00f, 1.00f, 1.00f );

ColorA      QTimeline::mTracksBgCol             = ColorA( 0.00f, 0.00f, 0.00f, 1.00f );
ColorA      QTimeline::mTracksBgOverCol         = ColorA( 0.00f, 0.00f, 0.00f, 1.00f );

ColorA      QTimeline::mModulesBgCol            = ColorA( 0.25f, 0.25f, 0.25f, 1.00f );
ColorA      QTimeline::mModulesBgOverCol        = ColorA( 0.25f, 0.25f, 0.25f, 1.00f );
ColorA      QTimeline::mModulesTextCol          = ColorA( 1.00f, 1.00f, 1.00f, 1.00f );
ColorA      QTimeline::mModulesHandleCol        = ColorA( 1.00f, 1.00f, 1.00f, 0.15f );
ColorA      QTimeline::mModulesHandleOverCol    = ColorA( 1.00f, 1.00f, 1.00f, 0.30f );
                                                     
ColorA      QTimeline::mParamsBgCol             = ColorA( 0.15f, 0.15f, 0.15f, 1.00f );
ColorA      QTimeline::mParamsBgOverCol         = ColorA( 0.15f, 0.15f, 0.15f, 1.00f );
ColorA      QTimeline::mParamsTextCol           = ColorA( 1.00f, 1.00f, 1.00f, 1.00f );

ColorA      QTimeline::mKeyframesBgCol          = ColorA( 1.00f, 1.00f, 1.00f, 0.40f );
ColorA      QTimeline::mKeyframesBgOverCol      = ColorA( 1.00f, 1.00f, 1.00f, 0.80f );
ColorA      QTimeline::mKeyframesGraphCol       = ColorA( 1.00f, 1.00f, 1.00f, 0.40f );
ColorA      QTimeline::mKeyframesBgSelectedCol  = ColorA( 1.00f, 0.40f, 0.40f, 1.00f );

ColorA      QTimeline::mCueListBgCol            = ColorA( 1.00f, 1.00f, 1.00f, 1.00f );
ColorA      QTimeline::mCueBgCol                = ColorA( 0.25f, 0.25f, 0.25f, 1.00f );
ColorA      QTimeline::mCueBgOverCol            = ColorA( 0.25f, 0.25f, 0.25f, 1.00f );
ColorA      QTimeline::mCueFgCol                = ColorA( 0.25f, 0.25f, 0.25f, 1.00f );
ColorA      QTimeline::mCueTextCol              = ColorA( 1.00f, 1.00f, 1.00f, 1.00f );
ColorA      QTimeline::mCueHandleCol            = ColorA( 1.00f, 1.00f, 1.00f, 0.15f );
ColorA      QTimeline::mCueHandleOverCol        = ColorA( 1.00f, 1.00f, 1.00f, 0.30f );


void QTimeline::init()
{
    mApp = ci::app::App::get();
    
    registerCallbacks();
    
    mTimeline = ci::Timeline::create();
    
    mTransportRect  = Rectf( 0, getWindowHeight() - TIMELINE_TRANSPORT_HEIGHT, getWindowWidth(), getWindowHeight() );
    
    mFontSmall              = ci::gl::TextureFont::create( ci::Font( "Helvetica", 12 ) );
    mFontMedium             = ci::gl::TextureFont::create( ci::Font( "Helvetica", 14 ) );
    mFontBig                = ci::gl::TextureFont::create( ci::Font( "Helvetica", 16 ) );
    
    mZoom                   = 1.0f;
    
    mIsVisible              = true;
    
    mIsMouseDragTimeBar     = false;
    
    mSelectedMenu           = NULL;
    
    mCueManager             = new QTimelineCueManager(this);
    
    // create default track
    mTracks.push_back( QTimelineTrackRef( new QTimelineTrack( this, "track 0" ) ) );
    
    play( false, FREE_RUN );
    
    mRenderDebug            = false;
    
    mRenderHelp             = true;
    
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
    
}


void QTimeline::update()
{
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
    mCueManager->setRect( Rectf( mTimeBarRect.x1, mTimeBarRect.y1 - TIMELINE_CUELIST_HEIGHT, mTimeBarRect.x2, mTimeBarRect.y1 ) );
    mCueManager->render();
    
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
    mIsMouseDragTimeBar = mTimeBarRect.contains( event.getPos() );
    
    mouseDrag( event );
    
    return false;
}


bool QTimeline::mouseUp( ci::app::MouseEvent event )
{
    for( size_t k=0; k < mTracks.size(); k++ )
        mTracks[k]->mouseUp( event );
    
    mCueManager->mouseUp( event );
    
    mIsMouseDragTimeBar = false;
    
    return false;
}


bool QTimeline::mouseMove( ci::app::MouseEvent event )
{
    if ( mSelectedMenu && mSelectedMenu->isVisible() && mSelectedMenu->mouseMove(event) )
        return true;
    
    for( size_t k=0; k < mTracks.size(); k++ )
        mTracks[k]->mouseMove( event );

    mCueManager->mouseMove( event );
    
    return false;
}


bool QTimeline::mouseDrag( MouseEvent event )
{
    for( size_t k=0; k < mTracks.size(); k++ )
        mTracks[k]->mouseDrag( event );
    
    mCueManager->mouseDrag( event );
    
    // time bar handler
    if ( mIsMouseDragTimeBar )
        mTimeline->stepTo( getTimeFromPos( event.getPos().x ) );
    
    return false;
}


bool QTimeline::mouseWheel( MouseEvent event )
{
    if ( event.isAltDown() )
        mZoom = math<float>::clamp( mZoom + event.getWheelIncrement(), 0.2f, 3.0f );
    
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


void QTimeline::play( bool play, PlayMode mode )
{
    mPrevElapsedSeconds = getElapsedSeconds();
    mIsPlaying          = play;
    mPlayMode           = mode;
}


void QTimeline::playCue()
{
    play( !isPlaying(), CUE_LIST );
    
    if ( !isPlaying() )
        return;
    
    mCueManager->playCue();
}


void QTimeline::renderTimeBar()
{
    // render time bar
    float timelinePosX = mTimeBarRect.getWidth() * mTimeNorm;
    mTimeBarRect  = Rectf( mTracksRect.getUpperLeft() - Vec2f( 0, TIMELINE_BAR_HEIGHT ), mTracksRect.getUpperRight() );
    gl::color( mTimeBarBgCol );
    gl::drawSolidRect( mTimeBarRect );
    gl::color( mTimeBarFgCol );
    gl::drawSolidRect( Rectf( mTimeBarRect.getUpperLeft(), mTimeBarRect.getLowerLeft() + Vec2f( timelinePosX, 0 ) ) );
    
    // render time bar indicator
    gl::color( mTimeBarFgCol );
    glBegin( GL_QUADS );
    gl::vertex( Vec2f( timelinePosX, mTimeBarRect.getY1() ) );
    gl::vertex( Vec2f( timelinePosX+1, mTimeBarRect.getY1() ) );
    gl::vertex( Vec2f( timelinePosX+1, mTransportRect.getY1() ) );
    gl::vertex( Vec2f( timelinePosX, mTransportRect.getY1() ) );
    glEnd();
    
    // render beat and bar labels
    float   windowInSec     = ( TIMELINE_WINDOW_BEATS * 60.0f / TIMELINE_BPM ) / mZoom;
    int     nSecs           = windowInSec + 1;
    float   oneSecInPx      = mTimeBarRect.getWidth() / windowInSec;
    
    glBegin( GL_QUADS );
    for( int k=0; k < nSecs; k++ )
    {
        gl::vertex( Vec2f( (int)(k * oneSecInPx), mTimeBarRect.y1 ) );
        gl::vertex( Vec2f( (int)(k * oneSecInPx)+1, mTimeBarRect.y1 ) );
        gl::vertex( Vec2f( (int)(k * oneSecInPx)+1, mTimeBarRect.y2 ) );
        gl::vertex( Vec2f( (int)(k * oneSecInPx), mTimeBarRect.y2 ) );
    }
    glEnd();
    
    // render labels
    gl::color( mTimeBarTextCol );
    for( int k=0; k < nSecs; k+=4 )
    mFontSmall->drawString( toString( mTimeWindow.x + k ),  Vec2f( k * oneSecInPx + 3, mTimeBarRect.y2 - 3 ) );
    
    gl::color( Color::white() );
}


void QTimeline::addModule( QTimelineModule *module, float startAt, float duration )
{
    QTimelineTrackRef nullPtr;
    addModule( module, startAt, duration, nullPtr );
}


void QTimeline::addModule( QTimelineModule *module, float startAt, float duration, QTimelineTrackRef trackRef )
{
    // get track, if it doesn't exists or if trackN == -1, create a new one
    if ( !trackRef )
    {
        QTimelineTrackRef ref( new QTimelineTrack( this, "track untitled" ) );
        trackRef.swap( ref );
        mTracks.push_back( trackRef );
    }
    
    // check new module fits in between the others, if not shift the new module to the first slot available
    for( size_t k=0; k < trackRef->mModules.size(); k++ )
        if ( ( startAt >= trackRef->mModules[k]->getStartTime() && startAt <= trackRef->mModules[k]->getEndTime() ) ||
             ( (startAt + duration) >= trackRef->mModules[k]->getStartTime() && ( startAt + duration ) <= trackRef->mModules[k]->getEndTime() ) )
            startAt = trackRef->mModules[k]->getEndTime();
    
    QTimelineModuleItemRef moduleItemRef = QTimelineModuleItem::create( module, trackRef, mTimeline.get() );
    module->setItemRef( moduleItemRef );
    moduleItemRef->setStartTime( startAt );
    moduleItemRef->setDuration( duration );
    mTimeline->insert( moduleItemRef );
    
    trackRef->addModuleItem( moduleItemRef );
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
        
        // Transport
        QTimeline::mTransportBgCol          = getThemeColor( theme, "TransportBgCol" );
        QTimeline::mTransportTextCol        = getThemeColor( theme, "TransportTextCol" );
        
        // Tracks
        QTimeline::mTracksBgCol             = getThemeColor( theme, "TracksBgCol" );
        QTimeline::mTracksBgOverCol         = getThemeColor( theme, "TracksBgOverCol" );
        
        // Modules
        QTimeline::mModulesBgCol            = getThemeColor( theme, "ModulesBgCol" );
        QTimeline::mModulesBgOverCol        = getThemeColor( theme, "ModulesBgOverCol" );
        QTimeline::mModulesTextCol          = getThemeColor( theme, "ModulesTextCol" );
        
        QTimeline::mModulesHandleCol        = getThemeColor( theme, "ModulesHandleCol" );
        QTimeline::mModulesHandleOverCol    = getThemeColor( theme, "ModulesHandleOverCol" );

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
    }
    catch ( ... )
    {
        console() << "ERROR > QTimeline theme NOT loaded!" << endl;
    }
    
    QTimelineTrackRef               trackRef;
    QTimelineModuleItemRef          moduleItemRef;
    QTimelineParamRef               paramRef;
    QTimelineCueRef                 cueRef;
    std::vector<QTimelineCueRef>    cueList;
    
    mCueManager->setBgColor( QTimeline::mCueListBgCol );
    
    for( size_t k=0; k < mTracks.size(); k++ )
    {
        trackRef                = mTracks[k];
        trackRef->mBgColor      = QTimeline::mTracksBgCol;
        trackRef->mBgOverColor  = QTimeline::mTracksBgOverCol;
        
        for( size_t i=0; i < trackRef->mModules.size(); i++ )
        {
            moduleItemRef                       = trackRef->mModules[i];
            moduleItemRef->mBgColor             = QTimeline::mModulesBgCol;
            moduleItemRef->mBgOverColor         = QTimeline::mModulesBgOverCol;
            moduleItemRef->mTextColor           = QTimeline::mModulesTextCol;
            moduleItemRef->mHandleColor         = QTimeline::mModulesHandleCol;
            moduleItemRef->mHandleOverColor     = QTimeline::mModulesHandleOverCol;
            
            for( size_t j=0; j < moduleItemRef->mParams.size(); j++ )
            {
                paramRef                = moduleItemRef->mParams[j];
                paramRef->mBgColor      = QTimeline::mParamsBgCol;
                paramRef->mBgOverColor  = QTimeline::mParamsBgOverCol;
                paramRef->mTextColor    = QTimeline::mParamsTextCol;
                
                paramRef->mKeyframesBgCol           = QTimeline::mKeyframesBgCol;
                paramRef->mKeyframesBgOverCol       = QTimeline::mKeyframesBgOverCol;
                paramRef->mKeyframesBgSelectedCol   = QTimeline::mKeyframesBgSelectedCol;
                paramRef->mKeyframesGraphCol        = QTimeline::mKeyframesGraphCol;
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
    /*
    XmlTree doc( "QTimeline", "" );
    
    for( size_t k=0; k < mTracks.size(); k++ )
        doc.push_back( mTracks[k]->getXmlNode() );

//    for( size_t k=0; k < mCueList.size(); k++ )
//        doc.push_back( mCueList[k]->getXmlNode() );

    doc.write( writeFile( filename ) );
     */
}


void QTimeline::load( const std::string &filename )
{
    /*
    XmlTree doc;
    
    try
    {
        doc = XmlTree( loadFile(filename) );
        
        for( XmlTree::Iter nodeIt = doc.begin("QTimeline/track"); nodeIt != doc.end(); ++nodeIt )
        {
            mTracks.push_back( QTimelineTrackRef( new QTimelineTrack( this, "track untitled", *nodeIt ) ) );
        }

        for( XmlTree::Iter nodeIt = doc.begin("QTimeline/cue"); nodeIt != doc.end(); ++nodeIt )
        {
            
        }

    }
    catch ( ... )
    {
        console() << "Error > QTimeline::load(): " << filename << " not found!" << endl;
        return;
    }
     */
}


void QTimeline::renderDebugInfo()
{    
    Vec2f debugOffset( 15, 50 );
    mFontMedium->drawString( "TIMELINE DEBUG:",                         debugOffset ); debugOffset += Vec2f( 0, 20 );
    mFontMedium->drawString( "Time:\t"   + toString( getTime() ),       debugOffset ); debugOffset += Vec2f( 0, 15 );
    mFontMedium->drawString( "Window:\t" + toString( mTimeWindow ),     debugOffset ); debugOffset += Vec2f( 0, 15 );
    mFontMedium->drawString( "BPM:\t"    + toString( TIMELINE_BPM ),    debugOffset ); debugOffset += Vec2f( 0, 15 );
    string str = isPlaying() ? "TRUE" : "FALSE";
    mFontMedium->drawString( "PLAY:\t"   + str,                         debugOffset ); debugOffset += Vec2f( 0, 15 );
    str = mPlayMode == FREE_RUN ? "FREE_RUN" : "CUE_LIST";
    mFontMedium->drawString( "MODE:\t"   + str,                         debugOffset ); debugOffset += Vec2f( 0, 15 );
    
    if ( mSelectedMenu )
    {
        str = "MENU OPEN: visible " + toString(mSelectedMenu->mPos) + " " + mSelectedMenu->getName();
        mFontMedium->drawString( str,                                   debugOffset ); debugOffset += Vec2f( 0, 15 );
    }
    
    vector<QTimelineCueRef> cueList = mCueManager->getCueList();
    mFontMedium->drawString( "CUES: " + toString( cueList.size() ),     debugOffset ); debugOffset += Vec2f( 0, 15 );
    
    QTimelineCueRef currentCue = mCueManager->getCurrentCue();
    str = "CUE: ";
    if ( currentCue ) str += currentCue->getName(); else str += "NONE";
    mFontMedium->drawString( str,                                       debugOffset ); debugOffset += Vec2f( 0, 15 );
}
