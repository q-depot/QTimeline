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

using namespace ci;
using namespace ci::app;
using namespace std;


QTimelineModuleItem::QTimelineModuleItem( std::string name, float startTime, float duration, QTimelineTrackRef trackRef )
: QTimelineItem( startTime, duration, "QTimelineModuleItem", name, trackRef )
{
    setBgColor( QTimeline::mModuleItemBgCol );
    setBgOverColor( QTimeline::mModuleItemBgOverCol );
    setTextColor( QTimeline::mModuleItemTextCol );
    
    setHandleColor( QTimeline::mModuleItemHandleCol );
    setHandleOverColor( QTimeline::mModuleItemHandleOverCol );
    
    // set rect width to calculate the label length when the item is created
    Rectf r( QTimeline::getPtr()->getPosFromTime( getStartTime() ), 0.0f,
             QTimeline::getPtr()->getPosFromTime( getEndTime() ),   0.0f   );
    
    setRect( r );
    
    updateLabel();
    
    initMenu();
}

QTimelineModuleItem::~QTimelineModuleItem()
{
    clear();
}


void QTimelineModuleItem::clear()
{
    QTimelineItem::clear();
    
    resetTargetModule();
}


void QTimelineModuleItem::update( float relativeTime )
{
    if ( isComplete() )
        return;
    
    updateParams( relativeTime );
    
    if ( mTargetModuleRef )
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
        QTimeline::getPtr()->closeMenu( mMenu );
        QTimeline::getPtr()->markItemForRemoval( thisRef() );
    }
    
    else if ( item->getMeta() == "color_palette" )
    {
        QTimelineMenuColorPalette *palette = (QTimelineMenuColorPalette*)item.get();
        setColor( palette->getColor() );
    }
    
    else if ( item->getMeta() == "name_text_box" )
    {
        setName( item->getName() );
        QTimeline::getPtr()->closeMenu( mMenu );
    }
}


void QTimelineModuleItem::initMenu()
{
    mMenu->init( "MODULE MENU" );

    mMenu->addTextBox( getName(), "name_text_box", this, &QTimelineModuleItem::menuEventHandler );
    
    mMenu->addColorPalette( this, &QTimelineModuleItem::menuEventHandler );
    
    mMenu->addSeparator();
    
    mMenu->addButton( "X DELETE", "delete", this, &QTimelineModuleItem::menuEventHandler );
}

