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
: QTimelineItem( startTime, duration, "QTimelineAudioItem", "track name!", trackRef )
{
    setBgColor( QTimeline::mAudioItemBgCol );
    setBgOverColor( QTimeline::mAudioItemBgOverCol );
    setTextColor( QTimeline::mAudioItemTextCol );
    
    setHandleColor( QTimeline::mAudioItemHandleCol );
    setHandleOverColor( QTimeline::mAudioItemHandleOverCol );
    
    // init rect width
    setRect( Rectf( QTimeline::getPtr()->getPosFromTime( getStartTime() ),  0,
                    QTimeline::getPtr()->getPosFromTime( getEndTime() ),    0 ) );
    
    updateLabel();
    
    initMenu();
    
    loadAudioTrack();
}


void QTimelineAudioItem::clear()
{
    
}


void QTimelineAudioItem::update( float relativeTime )
{
    if ( isComplete() )
    {
//        if ( mTrack->isPlaying() )
//        {
//            ci::app::console() << "set time" << std::endl;
//            mTrack->setTime( 0.0f );
//            mTrack->stop();
//        }
//        
        return;
    }
    
    updateParams( relativeTime );
    
    
//    if ( !mTrack->isPlaying() )
//    {
//        // set time, getTimeFromPos
//        mTrack->setTime( 0.0f );
//        mTrack->play();
//    }
    
//    ci::app::console() << "QTimelineAudioItem::update() " << relativeTime << " " << mTrack->getTime() << std::endl;
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
    
    // render name
    gl::color( mTextColor );
    mFont->drawString( getLabel(), mRect.getCenter() + mLabelStrSize * Vec2f( -0.5f, 0.3f ) );
    
    
    
	gl::color( Color( 1.0f, 0.5f, 0.0f ) );
	gl::draw( mLeftBufferLine );
//	gl::draw( mRightBufferLine );

    /*
	gl::color( Color( 1.0f, 0.5f, 0.25f ) );
	// grab 512 samples of the wave data
	float waveData[512];
	mSystem->getWaveData( waveData, 512, 0 );
	
	// prep 512 Vec2fs as the positions to render our waveform
	vector<Vec2f> vertices;
	for( int i = 0; i < 512; ++i )
		vertices.push_back( Vec2f( getWindowWidth() / 512.0f * i, getWindowCenter().y + 100 * waveData[i] ) );
    
	// draw the points as a line strip
	glEnableClientState( GL_VERTEX_ARRAY );
	gl::color( Color( 1.0f, 0.5f, 0.25f ) );
    glVertexPointer( 2, GL_FLOAT, 0, &vertices[0] );
    glDrawArrays( GL_LINE_STRIP, 0, vertices.size() );
    */
    
}


void QTimelineAudioItem::menuEventHandler( QTimelineMenuItemRef item )
{
    if ( item->getMeta() == "load_track" )
    {
        loadAudioTrack();
    }
    
    else if ( item->getMeta() == "color_palette" )
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
    
    mMenu->addButton( "Load track", "load_track", this, &QTimelineAudioItem::menuEventHandler );
    
    mMenu->addSeparator();
   
    mMenu->addButton( "X DELETE", "delete", this, &QTimelineAudioItem::menuEventHandler );
}


void QTimelineAudioItem::loadAudioTrack()
{   
    //add the audio track the default audio output
	mTrack = audio::Output::addTrack( audio::load( loadAsset( "booyah.mp3" ) ), true );
	//you must enable enable PCM buffering on the track to be able to call getPcmBuffer on it later
	mTrack->enablePcmBuffering( true );
    
    while( !mPcmBuffer )
    {
        mPcmBuffer = mTrack->getPcmBuffer();
        ci::sleep( 15.0f );
    }
    
    float time = 0;
    int c =0;
    while( mTrack->isPlaying() )
    {
        mTrack->setTime( time );
        
        mPcmBuffer = mTrack->getPcmBuffer();
        
        uint32_t bufferLength = mPcmBuffer->getSampleCount();
        audio::Buffer32fRef leftBuffer = mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT );
        audio::Buffer32fRef rightBuffer = mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_RIGHT );
        
        int displaySize = getWindowWidth();
        float scale = displaySize / (float)bufferLength;
        float val = 0;
        for( int i = 0; i < bufferLength; i++ ) {
            float x = ( i * scale );
            
            //get the PCM value from the left channel buffer
            float y = ( ( leftBuffer->mData[i] - 1 ) * - 100 );
            val += y;
//            mLeftBufferLine.push_back( Vec2f( x , y) );
        }
        
        val /= bufferLength;
        
        mLeftBufferLine.push_back( Vec2f( c++ , val) );
        
        time += 0.1f;
    }
    
    
    
/*
 FMOD::System_Create( &mSystem );
    mSystem->init( 32, FMOD_INIT_NORMAL | FMOD_INIT_ENABLE_PROFILE, NULL );
    
    mSystem->createSound( getAssetPath( "booyah.mp3" ).string().c_str(), FMOD_SOFTWARE, NULL, &mSound );
	mSound->setMode( FMOD_LOOP_NORMAL );
    
    mSystem->playSound( FMOD_CHANNEL_FREE, mSound, true, &mChannel );
    
    
    
    unsigned int length;
    
    mSound->getLength( &length, FMOD_TIMEUNIT_MS );
    
    
    float waveData[512];
    float mean;
    unsigned int pos;
    
    for( unsigned int k=0; k < length; k+=10 )
    {
        mChannel->setPosition( k, FMOD_TIMEUNIT_MS );
        
//        if ( mChannel->getWaveData( waveData, 512, 0 ) != FMOD_OK )
//            console() << k << " no data" << endl;
        
        mSystem->getWaveData( waveData, 512, 0 );
        
        // prep 512 Vec2fs as the positions to render our waveform
        mean = 0;
        for( int i = 0; i < 512; ++i )
            mean += 100 * waveData[i];
        
        mChannel->getPosition( &pos, FMOD_TIMEUNIT_MS );

        console() << k << " " << pos << " " << mean << " " << endl;;
        
        mean /= 512;
        
        mLeftBufferLine.push_back( Vec2f( k, mean ) );

    }
    console() << endl;
//    mChannel->getPosition( &pos, FMOD_TIMEUNIT_MS );
    
//    console() << " pos: " << pos << endl;
 */
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


