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
: QTimelineItem( startTime, duration, "QTimelineAudioItem", "untitled", trackRef )
{
    setBgColor( QTimeline::mAudioItemBgCol );
    setBgOverColor( QTimeline::mAudioItemBgOverCol );
    setTextColor( QTimeline::mAudioItemTextCol );
    
    setHandleColor( QTimeline::mAudioItemHandleCol );
    setHandleOverColor( QTimeline::mAudioItemHandleOverCol );
    
    // init rect width
    setRect( Rectf( QTimeline::getPtr()->getPosFromTime( getStartTime() ),  0,
                    QTimeline::getPtr()->getPosFromTime( getEndTime() ),    0 ) );
    
    loadAudioTrack();
    
    updateLabel();
    
    initMenu();
}


void QTimelineAudioItem::clear()
{
    QTimelineItem::clear();
    
    if ( !BASS_StreamFree( mAudioHandle ) )
    {
        console() << "ERROR QTimelineAudioItem::clear() failed to free the stream: ";
        console() << BASS_ErrorGetCode() << endl;
    }
}


void QTimelineAudioItem::update( float relativeTime )
{
    DWORD trackState = BASS_ChannelIsActive( mAudioHandle );
    
    double currentTime = QTimeline::getPtr()->getTime();
    
    if ( !QTimeline::getPtr()->isPlaying() || currentTime < getStartTime() || currentTime > getEndTime() )
    {
        if ( trackState == BASS_ACTIVE_PLAYING )                        // stop track
        {
            if ( !BASS_ChannelPause( mAudioHandle ) )
            {
                console() << "ERROR QTimelineAudioItem BASS_ChannelPause failed! ";
                console() << BASS_ErrorGetCode() << endl;
            }
        }
     
        return;
    }
    
    if ( trackState != BASS_ACTIVE_PLAYING )                            // play track
    {
        double time = QTimeline::getPtr()->getTime() - getStartTime();
        time = math<double>::max( 0.0f, time );
        QWORD channelPos = BASS_ChannelSeconds2Bytes( mAudioHandle, time );
        BASS_ChannelSetPosition( mAudioHandle, channelPos, BASS_POS_BYTE );

        if ( !BASS_ChannelPlay( mAudioHandle, false ) )
        {
            console() << "ERROR QTimelineAudioItem BASS_ChannelPlay failed! ";
            console() << BASS_ErrorGetCode() << endl;
        }
    }
    
    updateParams( relativeTime );
}


void QTimelineAudioItem::render( bool mouseOver )
{
    // render bg rect
    glBegin( GL_QUADS );
    gl::color( mBgColor );
    gl::vertex( mRect.getUpperLeft() );
    gl::vertex( mRect.getUpperRight() );
    gl::vertex( mRect.getLowerRight() );
    gl::vertex( mRect.getLowerLeft() );
    glEnd();
    
    // render handles
    renderHandles();
    
    gl::color( ColorA( 1.0f, 1.0f, 1.0f, 0.08f ) );
    glBegin( GL_LINE_STRIP );
    gl::vertex( mRect.getLowerLeft() );
    gl::vertex( mRect.getUpperLeft() );
    gl::vertex( mRect.getUpperRight() );
    gl::vertex( mRect.getLowerRight() );
    glEnd();
    
    // render waveform
    float stepInPx = QTimeline::getPtr()->getPtr()->getPosFromTime( AUDIO_WAVEFORM_PRECISION, true );
    
    Vec2f plot;
    
    gl::color( ColorA( 0.0f, 0.0f, 0.0f, 0.4f ) );
    
    glBegin( GL_LINE_STRIP );
    for( size_t k=0; k < mWaveFormLeft.size(); k++ )
    {
        plot.x = mRect.x1 + stepInPx * k;
        plot.y = mRect.y1 + mRect.getHeight() * 0.5 * ( 1 - mWaveFormLeft[k] );
        
        if ( plot.x > mRect.x2 )
            break;
        
        gl::vertex( plot );
    }
    glEnd();
    
    glBegin( GL_LINE_STRIP );
    for( size_t k=0; k < mWaveFormRight.size(); k++ )
    {
        plot.x = mRect.x1 + stepInPx * k;
        plot.y = mRect.y1 + mRect.getHeight() * 0.5 * ( 1 + mWaveFormRight[k] );
        
        if ( plot.x > mRect.x2 )
            break;
        
        gl::vertex( plot );
    }
    glEnd();
    
    // render name
    gl::color( mTextColor );
    mFont->drawString( getLabel(), mRect.getCenter() + mLabelStrSize * Vec2f( -0.5f, 0.3f ) );

}


void QTimelineAudioItem::menuEventHandler( QTimelineMenuItemRef item )
{
    if ( item->getMeta() == "color_palette" )
    {
        QTimelineMenuColorPalette *palette = (QTimelineMenuColorPalette*)item.get();
        mBgColor = palette->getColor();
    }
    
    else if ( item->getMeta() == "delete" )
    {
        QTimeline::getPtr()->closeMenu( mMenu );
        QTimeline::getPtr()->markItemForRemoval( thisRef() );
    }
    
    else if ( item->getMeta() == "name_text_box" )
    {
        setName( item->getName() );
        QTimeline::getPtr()->closeMenu( mMenu );
    }
}


void QTimelineAudioItem::initMenu()
{
    mMenu->init( "AUDIO ITEM" );
    
    mMenu->addTextBox( getName(), "name_text_box", this, &QTimelineAudioItem::menuEventHandler );
    
    mMenu->addColorPalette( this, &QTimelineAudioItem::menuEventHandler );
    
    mMenu->addSeparator();
   
    mMenu->addButton( "X DELETE", "delete", this, &QTimelineAudioItem::menuEventHandler );
}

void QTimelineAudioItem::loadAudioTrack()
{
    cacheWaveForm();
    fs::path filePath( getAssetPath( "Blank__Kytt_-_08_-_RSPN.mp3" ) );
    
    mAudioHandle = BASS_StreamCreateFile(   FALSE,
                                         filePath.generic_string().c_str(),
                                         0, 0, BASS_SAMPLE_FLOAT | BASS_STREAM_PRESCAN );
    if ( !mAudioHandle )
    {
        console() << "ERROR QTimelineAudioItem::loadAudioTrack(): ";
        console() << BASS_ErrorGetCode() << endl;
    }
}

void QTimelineAudioItem::cacheWaveForm()
{
    mWaveFormLeft.clear();
    mWaveFormRight.clear();
    
    fs::path filePath( getAssetPath( "Blank__Kytt_-_08_-_RSPN.mp3" ) );
    
    mAudioHandle = BASS_StreamCreateFile(   FALSE,
                                            filePath.generic_string().c_str(),
                                            0, 0, BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT | BASS_STREAM_PRESCAN );
    
    QWORD len       = BASS_ChannelGetLength( mAudioHandle, BASS_POS_BYTE );     // the length in bytes
    mTrackDuration  = BASS_ChannelBytes2Seconds( mAudioHandle, len );           // the length in seconds
    int samplesN    = mTrackDuration / AUDIO_WAVEFORM_PRECISION;
    
    if ( !mAudioHandle )
    {
        console() << "ERROR QTimelineAudioItem::cacheWaveForm(): ";
        console() << BASS_ErrorGetCode() << endl;
    }
    
    for( int i = 0; i < samplesN; i++ )
    {
        QWORD pos = BASS_ChannelSeconds2Bytes( mAudioHandle, ( mTrackDuration / (float)samplesN ) * (float)i );
        BASS_ChannelSetPosition( mAudioHandle, pos, BASS_POS_BYTE );
        DWORD level = BASS_ChannelGetLevel( mAudioHandle );
        float left  = (float) LOWORD( level ) / 65535.0f;
        float right = (float) HIWORD( level ) / 65535.0f;
        
        mWaveFormLeft.push_back( left );
        mWaveFormRight.push_back( right );
    }
    
    setName( filePath.filename().generic_string() );
    
    setDuration( mTrackDuration );

    if ( !BASS_StreamFree( mAudioHandle ) )
    {
        console() << "ERROR QTimelineAudioItem::cacheWaveForm() failed to free the stream: ";
        console() << BASS_ErrorGetCode() << endl;
    }
}


XmlTree QTimelineAudioItem::getXmlNode()
{
    XmlTree node = QTimelineItem::getXmlNode();
    
    node.setAttribute( "trackPath", mTrackFilename );
    
    return node;
}


void QTimelineAudioItem::loadXmlNode( XmlTree node )
{
    QTimelineItem::loadXmlNode( node );

    // load track path
}


// TODO : this function should also be called when the time bar is set by using the left and right key
void QTimelineAudioItem::onTimeChange()
{
    DWORD   trackState  = BASS_ChannelIsActive( mAudioHandle );
    double  currentTime = QTimeline::getPtr()->getTime();
    
    //    if ( !QTimeline::getPtr()->isPlaying() || currentTime < getStartTime() || currentTime > getEndTime() )
    if ( currentTime < getStartTime() || currentTime > getEndTime() )
    {
        if ( trackState == BASS_ACTIVE_PLAYING )                                    // track is playing, STOP
        {
            if ( !BASS_ChannelPause( mAudioHandle ) )
            {
                console() << "ERROR QTimelineAudioItem BASS_ChannelPause failed! ";
                console() << BASS_ErrorGetCode() << endl;
            }
        }
        
        return;
    }
    
    double time = QTimeline::getPtr()->getTime() - getStartTime();
    time = math<double>::max( 0.0f, time );
    QWORD channelPos = BASS_ChannelSeconds2Bytes( mAudioHandle, time );
    BASS_ChannelSetPosition( mAudioHandle, channelPos, BASS_POS_BYTE );                 // if the module is playing update the position
  
    // TODO : if the timeline is not playing I still want to playback one frame of the audio track, dunno how!
    if ( !QTimeline::getPtr()->isPlaying()  )
    {
//        if ( !BASS_ChannelPlay( mAudioHandle, false ) )
//        {
//            console() << "ERROR QTimelineAudioItem BASS_ChannelPlay failed! ";
//            console() << BASS_ErrorGetCode() << endl;
//        }
//        sleep( 15.0f );
        
        if ( trackState == BASS_ACTIVE_PLAYING || trackState == BASS_ACTIVE_STALLED )   // track is playing, STOP
        {
            if ( !BASS_ChannelPause( mAudioHandle ) )
            {
                console() << "ERROR QTimelineAudioItem BASS_ChannelPause failed! ";
                console() << BASS_ErrorGetCode() << endl;
            }
        }
        
        return;
    }
    
    if ( trackState != BASS_ACTIVE_PLAYING )                                            // if it's not playing, PLAY
        BASS_ChannelPlay( mAudioHandle, false );
    
}
