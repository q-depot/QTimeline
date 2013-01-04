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
        QTimeline::getRef()->openMenu( mMenu, event.getPos() );

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


bool QTimelineItem::dragHandles( ci::app::MouseEvent event )
{
    float diff          = QTimeline::getRef()->getPxInSeconds( event.getPos().x - mMouseDownPos.x );
    float startTime, endTime, prevEndTime, nextStartTime;
    
    mParentTrack->findModuleBoundaries( thisRef(), &prevEndTime, &nextStartTime );
    
    if ( mSelectedHandleType == LEFT_HANDLE )
    {
        endTime     = getEndTime();
        startTime   = ci::math<float>::clamp( mMouseDownStartTime + diff, prevEndTime, endTime - QTimeline::getRef()->getPxInSeconds( TIMELINE_MODULE_HANDLE_WIDTH * 2 ) );
        startTime   = QTimeline::getRef()->snapTime( startTime );
        setStartTime( startTime );
        setDuration( endTime - startTime );
    }
    
    else if ( mSelectedHandleType == RIGHT_HANDLE )
    {
        startTime   = getStartTime();
        endTime     = ci::math<float>::clamp( mMouseDownEndTime + diff, startTime + QTimeline::getRef()->getPxInSeconds( TIMELINE_MODULE_HANDLE_WIDTH * 2 ), nextStartTime );
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


void QTimelineItem::dragWidget( ci::app::MouseEvent event )
{
    float diff       = QTimeline::getRef()->getPxInSeconds( event.getPos().x - mMouseDownPos.x );
    float prevEndTime, nextStartTime;
    
    mParentTrack->findModuleBoundaries( thisRef(), &prevEndTime, &nextStartTime );
    
    float startTime = getStartTime();
    float time      = ci::math<float>::clamp( mMouseDownStartTime + diff, prevEndTime, nextStartTime - getDuration() );
    time            = QTimeline::getRef()->snapTime( time );
    
    setStartTime( time );
    
    // update params, keyframes move with the module
    for( size_t k=0; k < mParams.size(); k++ )
        mParams[k]->updateKeyframesPos( time - startTime );
}

