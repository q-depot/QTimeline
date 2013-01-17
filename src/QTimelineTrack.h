/*
 *  QTimelineTrack.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QTIMELINE_TRACK
#define QTIMELINE_TRACK

#pragma once

#include "QTimelineWidget.h"

typedef std::shared_ptr<class QTimelineTrack>       QTimelineTrackRef;
typedef std::shared_ptr<class QTimelineModule>      QTimelineModuleRef;
typedef std::shared_ptr<class QTimelineItem>        QTimelineItemRef;
typedef std::shared_ptr<class QTimelineModuleItem>  QTimelineModuleItemRef;

class QTimeline;
class QTimelineItem;

class QTimelineTrack : public QTimelineWidget, public std::enable_shared_from_this<QTimelineTrack>
{
    
    friend class QTimeline;
    friend class QTimelineItem;
  
public:
    
    QTimelineTrack( std::string name );
    
    ~QTimelineTrack();
    
    void clear();
    
    QTimelineItemRef addModuleItem( float startTime, float duration, std::string name = "" );
    
    QTimelineItemRef addAudioItem( float startTime, float duration, std::string filename );
    
    ci::Rectf render( ci::Rectf rect, ci::Vec2f timeWindow, double currentTime );
    
    bool mouseDown( ci::app::MouseEvent event );
    
    bool mouseUp( ci::app::MouseEvent event );
    
    bool mouseMove( ci::app::MouseEvent event );
    
    bool mouseDrag( ci::app::MouseEvent event );
    
    void open() {  mIsTrackOpen = true; }
    
    void close() {  mIsTrackOpen = false; }
    
    void toggle() { mIsTrackOpen = !mIsTrackOpen; }
    
    bool isOpen() { return mIsTrackOpen; }
    
    ci::XmlTree getXmlNode();
    
    void loadXmlNode( ci::XmlTree node );
    
    void menuEventHandler( QTimelineMenuItemRef item );
    
    QTimelineTrackRef getRef() { return shared_from_this(); }
    
    bool isMouseOnModule() { return ( mMouseOnItem ) ? true : false; }
    
    QTimelineItemRef getMouseOnModule() { return mMouseOnItem; }
    
    void eraseModule( QTimelineItemRef itemRef );
    
    std::vector<QTimelineItemRef>   getItems() { return mItems; }
    
    void findModuleBoundaries( QTimelineItemRef itemRef, float *prevEndTime, float *nextStartTime );
    
    QTimelineItemRef getActiveItem() { return mActiveItem; }
    
    void setActiveItem( QTimelineItemRef ref )  { mActiveItem = ref; }
    
    void releaseActiveItem( QTimelineItemRef ref )      // QTimelineItem can only release itself
    {
        if ( mActiveItem == ref )
            mActiveItem.reset();
    }
    
    
private:
    
    ci::Rectf makeRect( ci::Rectf trackRect, ci::Vec2f window, double startTime, double endTime )
    {
        ci::Vec2f 	timeVec( startTime, endTime );
        ci::Vec2f   trackPos = ci::Vec2f( timeVec.x - window.x, timeVec.y - window.x ) / ( window.y - window.x );
        trackPos.x  = ci::math<float>::clamp( trackPos.x, 0.0f, 1.0f );
        trackPos.y  = ci::math<float>::clamp( trackPos.y, 0.0f, 1.0f );
        trackPos    *= trackRect.getWidth();
        ci::Rectf   r( trackPos.x, trackRect.getY1(), trackPos.y, trackRect.getY2() );
        
        return r;
    }
    
    void initMenu();
    
     
private:
    
    std::vector<QTimelineItemRef>   mItems;
  
    QTimelineItemRef                mActiveItem;
    QTimelineItemRef                mMouseOnItem;
    QTimelineItemRef                mSelectedItem;
    
    bool                            mIsMouseOnTrack;
    
    bool                            mIsTrackOpen;
    
    double                          mMouseDownAt;
};


#endif