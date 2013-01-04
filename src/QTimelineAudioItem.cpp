/*
 *  QTimelineAudioItem.cpp
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#include "QTimeline.h"
#include "QTimelineTrack.h"
#include "QTimelineAudioItem.h"

using namespace ci;
using namespace ci::app;
using namespace std;


QTimelineAudioItem::QTimelineAudioItem( float startTime, float duration, QTimelineTrackRef trackRef, ci::Timeline *ciTimeline )
: QTimelineItem( startTime, duration, "QTimelineAudioItem", "track name!", trackRef, ciTimeline )
{
    setBgColor( QTimeline::mAudioItemBgCol );
    setBgOverColor( QTimeline::mAudioItemBgOverCol );
    setTextColor( QTimeline::mAudioItemTextCol );
    
    setHandleColor( QTimeline::mAudioItemHandleCol );
    setHandleOverColor( QTimeline::mAudioItemHandleOverCol );
    
    // init rect width
    setRect( Rectf( QTimeline::getRef()->getPosFromTime( getStartTime() ),  0,
                    QTimeline::getRef()->getPosFromTime( getEndTime() ),    0 ) );
    
    updateLabel();
    
    initMenu();
}


void QTimelineAudioItem::clear()
{
    
}


void QTimelineAudioItem::update( float relativeTime )
{
    if ( isComplete() )
        return;
    
    updateParams( relativeTime );
    
    ci::app::console() << "QTimelineAudioItem::update() " << relativeTime << std::endl;
}


void QTimelineAudioItem::render( bool mouseOver )
{
    // render bg rect
    glBegin( GL_QUADS );
    gl::color( mBgColor );          gl::vertex( mRect.getUpperLeft() );
    gl::color( mBgColor );          gl::vertex( mRect.getUpperRight() );
    gl::color( mBgColor * 0.8f );   gl::vertex( mRect.getLowerRight() );
    gl::color( mBgColor * 0.8f );   gl::vertex( mRect.getLowerLeft() );
    glEnd();
    
    // render handles
    if ( mouseOver )
        renderHandles();
    
    gl::color( ColorA( 1.0f, 1.0f, 1.0f, 0.08f ) );
    glBegin( GL_LINE_STRIP );
    gl::vertex( mRect.getLowerLeft() );
    gl::vertex( mRect.getUpperLeft() );
    gl::vertex( mRect.getUpperRight() );
    gl::vertex( mRect.getLowerRight() );
    glEnd();
    
    // render name
    gl::color( mTextColor );
    mFont->drawString( getLabel(), mRect.getCenter() + mLabelStrSize * Vec2f( -0.5f, 0.3f ) );
}



bool QTimelineAudioItem::mouseMove( MouseEvent event )
{
    if ( mMouseOnParam )
        mMouseOnParam->mouseMove(event);
    
    mMouseOnParam.reset();
    
    mMouseOnHandleType  = NO_HANDLE;
    
    if ( contains( event.getPos() ) )
    {
        if ( handlesMouseMove( event.getPos() ) )
            return true;
        
        else if ( mParentTrack->isOpen() )
        {
            for( size_t k=0; k < mParams.size(); k++ )
                if ( mParams[k]->mouseMove(event) )
                {
                    mMouseOnParam = mParams[k];
                    return true;
                }
        }
        return true;
    }
    
    return false;
}


bool QTimelineAudioItem::mouseDown( MouseEvent event )
{
    mMouseDownPos       = event.getPos();
    mMouseDownStartTime = getStartTime();
    mMouseDownEndTime   = getEndTime();
    
    if ( mMouseOnParam )
        mMouseOnParam->mouseDown( event );
    
    else if ( event.isRightDown() )
        QTimeline::getRef()->openMenu( mMenu, event.getPos() );
    
    else
        handlesMouseDown();
    
    return false;
}


bool QTimelineAudioItem::mouseUp( MouseEvent event )
{
    if ( mMouseOnParam )
        mMouseOnParam->mouseUp( event );
    
    mouseMove( event );
    
    return false;
}


bool QTimelineAudioItem::mouseDrag( MouseEvent event )
{
    // drag param to update a keyframe
    if ( mMouseOnParam )
        mMouseOnParam->mouseDrag( event );
    
    else
    {
        if ( !dragHandles( event ) )
            dragWidget( event );
        
        QTimeline::getRef()->updateCurrentTime();
    }
    
    mMousePrevPos = event.getPos();
    
    return false;
}


bool QTimelineAudioItem::dragHandles( MouseEvent event )
{
    float diff          = QTimeline::getRef()->getPxInSeconds( event.getPos().x - mMouseDownPos.x );
    float startTime, endTime, prevEndTime, nextStartTime;
    
    mParentTrack->findModuleBoundaries( thisRef(), &prevEndTime, &nextStartTime );
    
    if ( mSelectedHandleType == LEFT_HANDLE )
    {
        endTime     = getEndTime();
        startTime   = math<float>::clamp( mMouseDownStartTime + diff, prevEndTime, endTime - QTimeline::getRef()->getPxInSeconds( TIMELINE_MODULE_HANDLE_WIDTH * 2 ) );
        startTime   = QTimeline::getRef()->snapTime( startTime );
        setStartTime( startTime );
        setDuration( endTime - startTime );
    }
    
    else if ( mSelectedHandleType == RIGHT_HANDLE )
    {
        startTime   = getStartTime();
        endTime     = math<float>::clamp( mMouseDownEndTime + diff, startTime + QTimeline::getRef()->getPxInSeconds( TIMELINE_MODULE_HANDLE_WIDTH * 2 ), nextStartTime );
        endTime     = QTimeline::getRef()->snapTime( endTime );
        setDuration( endTime - startTime );
    }
    
    updateLabel();
    
    for( size_t k=0; k < mParams.size(); k++ )
        mParams[k]->updateLabel();
    
    if ( mSelectedHandleType != NO_HANDLE )
        return true;
    
    return false;
}


void QTimelineAudioItem::dragWidget( MouseEvent event )
{
    float diff       = QTimeline::getRef()->getPxInSeconds( event.getPos().x - mMouseDownPos.x );
    float prevEndTime, nextStartTime;
    
    mParentTrack->findModuleBoundaries( thisRef(), &prevEndTime, &nextStartTime );
    
    float startTime = getStartTime();
    float time      = math<float>::clamp( mMouseDownStartTime + diff, prevEndTime, nextStartTime - getDuration() );
    time            = QTimeline::getRef()->snapTime( time );
    
    setStartTime( time );
    
    // update params, keyframes move with the module
    for( size_t k=0; k < mParams.size(); k++ )
        mParams[k]->updateKeyframesPos( time - startTime );
}




void QTimelineAudioItem::menuEventHandler( QTimelineMenuItemRef item )
{
    if ( item->getMeta() == "load_track" )
    {
        loadAudioTrack();
    }
}


void QTimelineAudioItem::initMenu()
{
    mMenu->init( "AUDIO ITEM MENU" );
    mMenu->addSeparator();
    mMenu->addButton( "Load track", "load_track", this, &QTimelineAudioItem::menuEventHandler );
}


void QTimelineAudioItem::loadAudioTrack()
{
//    
//    mTrack = audio::Output::addTrack( audio::load( "/Users/Q/Desktop/drums.mp3" ) );
//    
//	//if the buffer is null, for example if this gets called before any PCM data has been buffered
//	//don't do anything
//	if( ! mPcmBuffer ) {
//		return;
//	}
//	
//	uint32_t bufferLength = mPcmBuffer->getSampleCount();
//	audio::Buffer32fRef leftBuffer = mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT );
//	audio::Buffer32fRef rightBuffer = mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_RIGHT );
//    
//	int displaySize = getWindowWidth();
//	float scale = displaySize / (float)bufferLength;
//	
//	PolyLine<Vec2f>	leftBufferLine;
//	PolyLine<Vec2f>	rightBufferLine;
//	
//	for( int i = 0; i < bufferLength; i++ ) {
//		float x = ( i * scale );
//        
//		//get the PCM value from the left channel buffer
//		float y = ( ( leftBuffer->mData[i] - 1 ) * - 100 );
//		leftBufferLine.push_back( Vec2f( x , y) );
//		
//		y = ( ( rightBuffer->mData[i] - 1 ) * - 100 );
//		rightBufferLine.push_back( Vec2f( x , y) );
//	}
//	gl::color( Color( 1.0f, 0.5f, 0.25f ) );
//	gl::draw( leftBufferLine );
//	gl::draw( rightBufferLine );
//	
}

