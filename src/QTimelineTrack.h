/*
 *  QTimelineTrack.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2012 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QTIMELINE_TRACK
#define QTIMELINE_TRACK

#pragma once

#include "QTimelineWidget.h"

typedef std::shared_ptr<class QTimelineTrack>       QTimelineTrackRef;

class QTimeline;
class QTimelineModuleItem;
class QTimelineModule;

class QTimelineTrack : public QTimelineWidget, public std::enable_shared_from_this<QTimelineTrack>
{
    
    friend class QTimeline;
    friend class QTimelineModuleItem;
    friend class QTimelineParam;
    
public:
    
    QTimelineTrack( QTimeline *timeline, std::string name );
    
//    QTimelineTrack( QTimeline *timeline, ci::XmlTree node );
    
    ~QTimelineTrack();
    
    QTimelineModuleItemRef createModule( ci::Timeline *parent, std::string name, float startAt, float duration  );
    
    void addModule( QTimelineModule *module, float startAt, float duration );
    
    ci::Rectf render( ci::Rectf rect, ci::Vec2f timeWindow, double currentTime );
    
    bool mouseDown( ci::app::MouseEvent event );
    
    bool mouseUp( ci::app::MouseEvent event );
    
    bool mouseMove( ci::app::MouseEvent event );
    
    bool mouseDrag( ci::app::MouseEvent event );
    
    void toggle() { mIsTrackOpen = !mIsTrackOpen; }
    
    bool isOpen() { return mIsTrackOpen; }
    
    ci::Vec2f getTimeWindow();
    
//    void addModuleItem( QTimelineModuleItemRef moduleItemRef );
    
    void deleteModuleItem( QTimelineModuleItemRef moduleItemRef );
    
    ci::XmlTree getXmlNode();
    
    void loadXmlNode( ci::XmlTree node );
    
    void menuEventHandler( QTimelineMenuItem* item );
    
    QTimelineTrackRef getRef() { return shared_from_this(); }
    
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
    
    QTimeline           *mQTimeline;
    
    std::vector<QTimelineModuleItemRef>     mModules;
    
    QTimelineModuleItemRef  mMouseOnModule;
    QTimelineModuleItemRef  mSelectedModule;
    
    bool                mIsMouseOnTrack;
    
    bool                mIsTrackOpen;
    
    double              mMouseDownAt;
};


#endif