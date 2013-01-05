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
    gl::color( mBgColor );
    gl::vertex( mRect.getUpperLeft() );
    gl::vertex( mRect.getUpperRight() );
    gl::vertex( mRect.getLowerRight() );
    gl::vertex( mRect.getLowerLeft() );
    glEnd();
    
    // render handles
//    if ( mouseOver )
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
}


void QTimelineAudioItem::initMenu()
{
    mMenu->init( "AUDIO ITEM" );
    mMenu->addColorPalette( this, &QTimelineAudioItem::menuEventHandler );
    mMenu->addSeparator();
    mMenu->addButton( "Load track", "load_track", this, &QTimelineAudioItem::menuEventHandler );
}


void QTimelineAudioItem::loadAudioTrack() {}


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

