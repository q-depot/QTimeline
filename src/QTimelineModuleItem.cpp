/*
 *  QTimelineModuleItem.cpp
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#include "QTimeline.h"
#include "QTimelineTrack.h"
#include "QTimelineModuleItem.h"
#include "QTimelineModule.h"

using namespace ci;
using namespace ci::app;
using namespace std;


QTimelineModuleItem::QTimelineModuleItem( float startTime, float duration, QTimelineModuleRef targetRef, QTimelineTrackRef trackRef, Timeline *ciTimeline )
: QTimelineItem( startTime, duration, "QTimelineModuleItem", targetRef->getName(), trackRef, ciTimeline )
{
    mTargetModuleRef    = targetRef;
    
    setBgColor( QTimeline::mModuleItemBgCol );
    setBgOverColor( QTimeline::mModuleItemBgOverCol );
    setTextColor( QTimeline::mModuleItemTextCol );

    setHandleColor( QTimeline::mModuleItemHandleCol );
    setHandleOverColor( QTimeline::mModuleItemHandleOverCol );

    // init rect width
    setRect( Rectf( QTimeline::getRef()->getPosFromTime( getStartTime() ), 0,
                    QTimeline::getRef()->getPosFromTime( getEndTime() ), 0 ) );
    
    updateLabel();
    
    initMenu();
}


QTimelineModuleItem::~QTimelineModuleItem()
{
    clear();
}


void QTimelineModuleItem::clear()
{
    if ( mMenu )
        QTimeline::getRef()->closeMenu( mMenu );
    
    mParams.clear();
    
    resetTarget();
}


void QTimelineModuleItem::update( float relativeTime )
{
    if ( isComplete() )
        return;
    
    updateParams( relativeTime );
    
    mTargetModuleRef->update();
}


void QTimelineModuleItem::render( bool mouseOver )
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


XmlTree QTimelineModuleItem::getXmlNode()
{
    XmlTree node = QTimelineItem::getXmlNode();
    
    node.setAttribute( "targetModuleType", getTargetType() );
    
    return node;
}


void QTimelineModuleItem::menuEventHandler( QTimelineMenuItemRef item )
{
    if ( item->getMeta() == "delete" )
    {
        QTimeline::getRef()->closeMenu( mMenu );
        mParentTrack->markModuleForRemoval( thisRef() );
    }
    else if ( item->getMeta() == "color_palette" )
    {
        QTimelineMenuColorPalette *palette = (QTimelineMenuColorPalette*)item.get();
//        mHandleColor = palette->getColor();
        setBgColor( palette->getColor() );
    }
}


void QTimelineModuleItem::initMenu()
{
    mMenu->init( "MODULE MENU" );
    
    mMenu->addColorPalette( this, &QTimelineModuleItem::menuEventHandler );
    
    mMenu->addSeparator();
    
    mMenu->addButton( "X DELETE", "delete", this, &QTimelineModuleItem::menuEventHandler );
}


std::string QTimelineModuleItem::getTargetType()
{
    return mTargetModuleRef->getType();
}
