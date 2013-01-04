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

class QTimeline;
class QTimelineItem;

class QTimelineTrack : public QTimelineWidget, public std::enable_shared_from_this<QTimelineTrack>
{
    
    friend class QTimeline;
    
public:
    
    QTimelineTrack( std::string name );
    
    ~QTimelineTrack();
    
    void clear();
    
    void addModuleItem( float startTime, float duration, QTimelineModuleRef ref );
    
    void addAudioItem( float startTime, float duration, std::string audioTrackFilename = "" );
    
    ci::Rectf render( ci::Rectf rect, ci::Vec2f timeWindow, double currentTime );
    
    bool mouseDown( ci::app::MouseEvent event );
    
    bool mouseUp( ci::app::MouseEvent event );
    
    bool mouseMove( ci::app::MouseEvent event );
    
    bool mouseDrag( ci::app::MouseEvent event );
    
    void open() {  mIsTrackOpen = true; }
    
    void close() {  mIsTrackOpen = false; }
    
    void toggle() { mIsTrackOpen = !mIsTrackOpen; }
    
    bool isOpen() { return mIsTrackOpen; }
    
    void markModuleForRemoval( QTimelineItemRef moduleItemRef );
    
    ci::XmlTree getXmlNode();
    
    void loadXmlNode( ci::XmlTree node );
    
    void menuEventHandler( QTimelineMenuItemRef item );
    
    QTimelineTrackRef getRef() { return shared_from_this(); }
    
    bool isMouseOnModule() { return ( mMouseOnItem ) ? true : false; }
    
    QTimelineItemRef getMouseOnModule() { return mMouseOnItem; }
    
    void eraseModule( QTimelineItemRef itemRef );
    
    std::vector<QTimelineItemRef>   getModules() { return mModules; }
    
    void findModuleBoundaries( QTimelineItemRef itemRef, float *prevEndTime, float *nextStartTime );
    
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
    
    std::vector<QTimelineItemRef>   mModules;
    
    QTimelineItemRef                mMouseOnItem;
    QTimelineItemRef                mSelectedItem;
    
    bool                            mIsMouseOnTrack;
    
    bool                            mIsTrackOpen;
    
    double                          mMouseDownAt;
};


#endif