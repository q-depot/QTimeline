/*
 *  QTimelineItem.cpp
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#include "QTimeline.h"
#include "QTimelineItem.h"
#include "QTimelineModule.h"

using namespace ci;
using namespace ci::app;
using namespace std;


QTimelineItem::QTimelineItem( float startTime, float duration, std::string type, std::string name, QTimelineTrackRef trackRef )
: QTimelineWidgetWithHandles( name )
{
    setAutoRemove(false);
    
    mParentTrack        = trackRef;
    mParent             = QTimeline::getPtr()->getTimelineRef().get();
    mType               = type;
    
    setStartTime( startTime );
    setDuration( duration );
}

void QTimelineItem::clear()
{    
    if ( mMenu )
        QTimeline::getPtr()->closeMenu( mMenu );
    
    mParams.clear();
}


void QTimelineItem::checkActive()
{
    bool running = isRunning();
    
    if ( running )
        mParentTrack->setActiveItem( thisRef() );
    else
        mParentTrack->releaseActiveItem( thisRef() );

    if ( mTargetModuleRef )
        mTargetModuleRef->stateChanged( running );
}

bool QTimelineItem::mouseMove( ci::app::MouseEvent event )
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


bool QTimelineItem::mouseDown( ci::app::MouseEvent event )
{
    mMouseDownPos       = event.getPos();
    mMouseDownStartTime = getStartTime();
    mMouseDownEndTime   = getEndTime();

    if ( mMouseOnParam )
        mMouseOnParam->mouseDown( event );

    else if ( event.isRightDown() )
        QTimeline::getPtr()->openMenu( mMenu, event.getPos() );

    else
        handlesMouseDown();

    return false;
}


bool QTimelineItem::mouseUp( ci::app::MouseEvent event )
{
    if ( mMouseOnParam )
        mMouseOnParam->mouseUp( event );

    mouseMove( event );

    return false;
}


bool QTimelineItem::mouseDrag( ci::app::MouseEvent event )
{
    if ( event.isRightDown() )
        return false;
    
    // drag param to update a keyframe
    if ( mMouseOnParam )
        mMouseOnParam->mouseDrag( event );

    else
    {
        if ( !dragHandles( event ) )
            dragWidget( event );
        
        QTimeline::getPtr()->updateCurrentTime();
    }

    mMousePrevPos = event.getPos();

    return false;
}


bool QTimelineItem::dragHandles( ci::app::MouseEvent event )
{
    float diff          = QTimeline::getPtr()->getPxInSeconds( event.getPos().x - mMouseDownPos.x );
    float startTime, endTime, prevEndTime, nextStartTime;
    
    mParentTrack->findModuleBoundaries( thisRef(), &prevEndTime, &nextStartTime );
    
    if ( mSelectedHandleType == LEFT_HANDLE )
    {
        endTime     = getEndTime();
        startTime   = ci::math<float>::clamp( mMouseDownStartTime + diff, prevEndTime, endTime - QTimeline::getPtr()->getPxInSeconds( TIMELINE_MODULE_HANDLE_WIDTH * 2 ) );
        startTime   = QTimeline::getPtr()->snapTime( startTime );
        setStartTime( startTime );
        setDuration( endTime - startTime );
    }
    
    else if ( mSelectedHandleType == RIGHT_HANDLE )
    {
        startTime   = getStartTime();
        endTime     = ci::math<float>::clamp( mMouseDownEndTime + diff, startTime + QTimeline::getPtr()->getPxInSeconds( TIMELINE_MODULE_HANDLE_WIDTH * 2 ), nextStartTime );
        endTime     = QTimeline::getPtr()->snapTime( endTime );
        setDuration( endTime - startTime );
    }
    
    updateLabel();
    
    if ( mSelectedHandleType != NO_HANDLE )
        return true;
    
    return false;
}


void QTimelineItem::dragWidget( ci::app::MouseEvent event )
{
    float diff       = QTimeline::getPtr()->getPxInSeconds( event.getPos().x - mMouseDownPos.x );
    float prevEndTime, nextStartTime;
    
    mParentTrack->findModuleBoundaries( thisRef(), &prevEndTime, &nextStartTime );
    
    float startTime = getStartTime();
    float time      = ci::math<float>::clamp( mMouseDownStartTime + diff, prevEndTime, nextStartTime - getDuration() );
    time            = QTimeline::getPtr()->snapTime( time );
    
    setStartTime( time );
    
    // update params, keyframes move with the module
    for( size_t k=0; k < mParams.size(); k++ )
        mParams[k]->updateKeyframesPos( time - startTime );
}


XmlTree QTimelineItem::getXmlNode()
{
    ColorA col = getColor();
    
    XmlTree node( "item", "" );
    node.setAttribute( "type",      getType() );
    node.setAttribute( "name",      getName() );
    node.setAttribute( "startTime", getStartTime() );
    node.setAttribute( "duration",  getDuration() );
    node.setAttribute( "color_r",  col.r );
    node.setAttribute( "color_g",  col.g );
    node.setAttribute( "color_b",  col.b );
    node.setAttribute( "color_a",  col.a );
    
    for( size_t k=0; k < mParams.size(); k++ )
        node.push_back( mParams[k]->getXmlNode() );
    
    return node;
}


void QTimelineItem::loadXmlNode( XmlTree node )
{
    float   value, time;
    string  fnStr;
    
    ColorA  col;
    
    try {
        col.r               = node.getAttributeValue<float>( "color_r" );
        col.g               = node.getAttributeValue<float>( "color_g" );
        col.b               = node.getAttributeValue<float>( "color_b" );
        col.a               = node.getAttributeValue<float>( "color_a" );
        
        setColor( col );
    }
    catch ( ... )
    {
        ci::app::console() << "QTimelineItem::loadXmlNode() " << getName() << " color not found!" << std::endl;
    }
    
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


string QTimelineItem::getTargetType()
{
    if ( mTargetModuleRef )
        return mTargetModuleRef->getType();
    else
        return "";
}


bool QTimelineItem::isRunning()
{
    double currentTime = QTimeline::getPtr()->getTime();
    
    if ( currentTime < getStartTime() || currentTime > getEndTime() )
        return false;
    
    return true;
}
