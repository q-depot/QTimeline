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


bool QTimelineModuleItem::mouseMove( MouseEvent event )
{
    if ( mMouseOnParam )
        mMouseOnParam->mouseMove(event);
    
    mMouseOnParam.reset();
    
    mMouseOnHandleType  = NO_HANDLE;
    
    if ( contains( event.getPos() ) )
    {
        if ( handlesMouseMove( event.getPos() ) )
            return true;

        return true;
    }
    
    else if ( mParentTrack->isOpen() )
    {
        for( size_t k=0; k < mParams.size(); k++ )
            if ( mParams[k]->mouseMove(event) )
            {
                mMouseOnParam = mParams[k];
                return true;
            }
    }
    
    return false;
}


bool QTimelineModuleItem::mouseDown( MouseEvent event )
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


bool QTimelineModuleItem::mouseUp( MouseEvent event )
{
    if ( mMouseOnParam )
        mMouseOnParam->mouseUp( event );
  
    mouseMove( event );
    
    return false;
}


bool QTimelineModuleItem::mouseDrag( MouseEvent event )
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


bool QTimelineModuleItem::dragHandles( MouseEvent event )
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


void QTimelineModuleItem::dragWidget( MouseEvent event )
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


XmlTree QTimelineModuleItem::getXmlNode()
{
    XmlTree node = QTimelineItem::getXmlNode();
    
    node.setAttribute( "targetModuleType", getTargetType() );
    
    return node;
}


void QTimelineModuleItem::loadXmlNode( ci::XmlTree node )
{
    float   value, time;
    string  fnStr;
    
    for( XmlTree::Iter paramIt = node.begin("param"); paramIt != node.end(); ++paramIt )
    {
        QTimelineParamRef param = findParamByName( paramIt->getAttributeValue<string>( "name" ) );
        
        if ( !param )
            continue;
        
        for( XmlTree::Iter kfIt = paramIt->begin("kf"); kfIt != paramIt->end(); ++kfIt )
        {
            value   = kfIt->getAttributeValue<float>( "value" );
            time    = kfIt->getAttributeValue<float>( "time" );
            fnStr   = kfIt->getAttributeValue<string>( "fn" );
            
            param->addKeyframe( time, value, QTimeline::getEaseFnFromString(fnStr), fnStr );
        }
    }
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
